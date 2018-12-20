#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define false 0
#define true (!false)

int* PAGES;
int PAGES_LENGTH;
int NUM_OF_ALGOS = 3;
int MAX_FRAMES;

typedef struct _SCA_FRAME {
    int page;
    int ref_bit;
} SCA_FRAME;

typedef struct _Node {
    int page;
    struct _Node* next;
} Node;

Node* head = NULL;
Node* tail = NULL;
Node* before = NULL;
Node* target = NULL;
int numOfNode = 0;

//Algorithm Functions
float wrapper(int algo, int num_frames);
int FIFO(int num_frames);
int LRU(int num_frames);
int LFU(int num_frames);
int OPTIMAL(int num_frames);
int SCA(int num_frames);

//Helper functions
int in_frames(int to_find, int *f, int n);
void init_array(int *f, int n, int i);
int find_max(int *f, int n);
void map_plus(int *u, int n, int c);
int find_next(int p, int c);
int find_min_count(int*, int*, int );

//Printing functions
void print_frames_state(int, int* frames);
void print_algo_info(char* algo_name);
void print_num_of_misses(int num);

void insert(int page);
Node* getNode(int page);
void removeNode(int page);


int main (int argc, char *argv[])
{
    PAGES = (int*)malloc(sizeof(int) * (argc - 3));
    PAGES_LENGTH = (argc - 3);
    MAX_FRAMES = atoi(argv[2]);

    for(int i=3; i<argc; i++) 
        PAGES[i-3] = atoi(argv[i]);

    wrapper(atoi(argv[1]), MAX_FRAMES);
    return 0;
}

float wrapper(int algo, int num_frames){
  int misses;
  switch (algo){
    case 0:
      misses = FIFO(num_frames);
      break;
    case 1:
      misses = LRU(num_frames);
      break;
    case 2:
      misses = OPTIMAL(num_frames);
      break;
    case 3:
      misses = LFU(num_frames);
      break;
    case 4:
      misses = SCA(num_frames);
      break;
    default:
      misses = 0;
      break;
  }
  return ((float) misses/20);
}

int FIFO(int num_frames) {
    int misses = 0;
    int f_in = 0;
    int *frames = (int *)malloc(sizeof(int)*num_frames);
    
    init_array(frames, num_frames, -1);
    print_algo_info("FIFO");

    int i;
    for (i = 0; i< PAGES_LENGTH; ++i) {
        if (in_frames(PAGES[i], frames, num_frames)!=-1)
        {
        // 암것도안함
        }
        else {
            frames[f_in] = PAGES[i];
            f_in = (f_in + 1) % num_frames;
            ++misses;
        }
        print_frames_state(PAGES[i], frames);
    }
    print_num_of_misses(misses);
    free(frames);
    return misses;
}

int find_victim(SCA_FRAME* sca) {
    Node* temp = head;
    int idx = 0;

    while(temp != NULL) {
        for(int i=0; i<MAX_FRAMES; i++) {
            if(sca[i].page == temp->page) {
                if(sca[i].ref_bit != 1)
                    return temp->page;
                    // return i;
            }
        }
        temp = temp->next;
    }
    // 전부다 1일경우
    printf("WHAT\n");
    for(int i=0; i<MAX_FRAMES; i++) 
        sca[i].ref_bit = 0;
    for(int i=0; i<MAX_FRAMES; i++) {
        if(sca[i].page == head->page) {
            if(sca[i].ref_bit != 1)
                return head->page;
        }
    }
}

void insert(int page) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->page = page;
    node->next = NULL;
    
    if(head == NULL)
        head = tail = node;
    else {
        tail->next = node;
        tail = node;
    }
    numOfNode++;
}
Node* getNode(int page) {
    Node* temp = head;

    while(temp != NULL) {
        if(temp->page == page)
            return temp;
        before = temp;
        temp = temp->next;
    }
    return NULL;
}
void removeNode(int page) {
    Node* node = getNode(page);
    if(node == NULL)
        return;

    if(node == head) {
        head = head->next;
    }
    
    if(before) 
        before->next = node->next;
    before = NULL;
    numOfNode--;
    free(node);
}   
int find_idx(int* frames, int page) {
    for(int i=0; i<MAX_FRAMES; i++) {
        if(frames[i] == page)
            return i;
    }
    return -1;
}

void put(SCA_FRAME* sca, int page, int init_idx){
    int idx, temp;
    int found_page;
    // replacement 해야하는 상황
    if(numOfNode == MAX_FRAMES) {
        // idx = find_victim(sca);
        // removeNode(sca[idx].page);
        found_page = find_victim(sca);
        for(int i=0; i<MAX_FRAMES; i++) {
            if(sca[i].page == found_page) {
                removeNode(sca[i].page);
                idx = i;
                break;
            }
        }

        sca[idx].page = page;    // clear
        insert(page);

        // replacement 발생시 모든참조비트 0으로 리셋
        // for(int i=0; i<MAX_FRAMES; i++)
            // sca[i].ref_bit = 0;
        sca[0].ref_bit = 0;
            
    } else {
        if(init_idx <= MAX_FRAMES)
            sca[init_idx].page = page;
        insert(page);
    }
}

int SCA(int num_frames) { // second chance 알고리즘
    int misses = 0;
    int f_in = 0;
    int isHit = 0;
    int init_idx = 0;
    SCA_FRAME *sca;
    sca = (SCA_FRAME*)malloc(sizeof(SCA_FRAME) * num_frames);
    
    for(int i=0; i<num_frames; i++) {
        sca[i].ref_bit = 0;
        sca[i].page = -1;
    }
    print_algo_info("Second chance");

    int i,idx;
    for (i = 0; i< PAGES_LENGTH; ++i) {
        for(int j=0; j<MAX_FRAMES; j++) {
            if(sca[j].page == PAGES[i]) {
                sca[j].ref_bit = 1;
                isHit = 1;
                break;
            }
        } 
        if(!isHit) {
            put(sca, PAGES[i], init_idx++);
            ++misses;
        }
        printf("Page %d > ", PAGES[i]);

        for(int i=0; i<MAX_FRAMES; i++) {
            if(sca[i].page == -1)
                printf("| F ");
            else 
                printf("| %d ", sca[i].page);
        }
        printf("|\n\n");
        isHit = 0;
    }
    print_num_of_misses(misses);
    free(sca);
    return misses;
}


void print_num_of_misses(int num) {
    printf("Page Fault = %d\n", num);
}

void print_algo_info(char* algo_name) {
    printf("%s, Reference string = ", algo_name);
    for(int i=0; i<PAGES_LENGTH; i++)
        printf("%d ", PAGES[i]);
    printf("\n\n");
}

void print_frames_state(int ref_num_no, int* frames) {
    printf("Page %d > ", ref_num_no);

    for(int i=0; i<MAX_FRAMES; i++) {
        if(frames[i] == -1)
            printf("| F ");
        else 
            printf("| %d ", frames[i]);
    }
    printf("|\n\n");
}

int LRU(int num_frames) {
    int misses = 0;
    int max_index = -1;
    int page_index = 0;
    int *frames = (int *)malloc(sizeof(int)*num_frames);
    int *used = (int *)malloc(sizeof(int)*num_frames);

    init_array(frames, num_frames, -1);
    init_array(used, num_frames, INT_MAX);
    print_algo_info("LRU");

    int i;
    for (i = 0; i< PAGES_LENGTH; ++i) {
        page_index = in_frames(PAGES[i], frames, num_frames);
        if (page_index == -1) {
        ++misses;
        max_index = find_max(used, num_frames);
        frames[max_index] = PAGES[i];
        map_plus(used, num_frames, 1);
        used[max_index] = 0;
        }
        else {
        map_plus(used, num_frames, 1);
        used[page_index] = 0;
        }
        print_frames_state(PAGES[i], frames);
    }
    print_num_of_misses(misses);
    free(frames);
    free(used);
    return misses;
}

int find_min_count(int* frames, int* used, int num_frames) {
    int min = INT_MAX;
    int idx = 0;
    int page_num;
    int a,b;

    for(int i=0; i<num_frames; i++) {
        if(frames[i] == -1)
            return i;
        if(min >= used[frames[i]]) {
            if(min == used[frames[i]]) {
                a = find_next(page_num, -1);
                b = find_next(frames[i],-1);
                if(b < a) { // reference string 인덱스비교
                    idx = i;
                    page_num = frames[i];
                } 
            } else {
                min = used[frames[i]];
                page_num = frames[i];
                idx = i;
            }
        }
    }
    // printf("%d\n", idx);
    return idx;
}

int LFU(int num_frames) {
    int misses = 0;
    int f_in = 0;
    int *frames = (int *)malloc(sizeof(int)*num_frames);
    int *used = (int *)malloc(sizeof(int)*PAGES_LENGTH);

    init_array(frames, num_frames, -1);
    init_array(used, PAGES_LENGTH, 0);
    print_algo_info("LFU");

    int i;
    for (i = 0; i< PAGES_LENGTH; ++i) {
        if (in_frames(PAGES[i], frames, num_frames)!=-1)
        {
            // 암것도안함
        }
        else {
            f_in = find_min_count(frames, used, num_frames);
            frames[f_in] = PAGES[i];
            ++misses;
        }
        print_frames_state(PAGES[i], frames);
        used[PAGES[i]]++;
    }
    print_num_of_misses(misses);
    free(frames);
    return misses;
}

int OPTIMAL(int num_frames) {
    int misses = 0;
    int max_index = -1;
    int page_index = 0;
    int *frames = (int *)malloc(sizeof(int)*num_frames);
    int *until = (int *)malloc(sizeof(int)*num_frames);

    init_array(frames, num_frames, -1);
    init_array(until, num_frames, INT_MAX);
    print_algo_info("OPTIMAL");

    int i;
    for (i = 0; i < PAGES_LENGTH; ++i) {
        page_index = in_frames(PAGES[i], frames, num_frames);
        if (page_index == -1) {
        ++misses;
        max_index = find_max(until, num_frames);
        frames[max_index] = PAGES[i];
        map_plus(until, num_frames, -1);
        until[max_index] = find_next(PAGES[i], i);
        }
        else {
        map_plus(until, num_frames, -1);
        until[page_index] = find_next(PAGES[i],i);  
        }
        print_frames_state(PAGES[i], frames);
    }
    print_num_of_misses(misses);
    free(frames);
    free(until);
    return misses;
}

void init_array(int *frames, int num_frames, int init_value) {
  int i;
  for (i = 0; i< num_frames; ++i) {
    frames[i] = init_value;
  }
}

int in_frames(int to_find, int *frames, int num_frames){
  int i;
  for (i = 0; i< num_frames; ++i) {
    if (frames[i] == to_find) {
      return i;
    }
  }
  return -1;
}

int find_max(int *used, int num_frames) {
  int max_value = INT_MIN;
  int max_index = 0;

  int i;
  for (i = 0; i < num_frames; ++i){ 
    if (used[i] > max_value) {
      max_value = used[i];
      max_index = i;
    }
  }
  return max_index;
}

void map_plus(int *frames, int num_frames, int change_by){
  int i;
  for (i = 0; i< num_frames; ++i) {
    if (frames[i] != INT_MAX) {
      frames[i] = frames[i] + change_by;
    }
  }
}

int find_next(int page, int current) {
  int count = 1;
  int i;
  for (i = current+1; i < PAGES_LENGTH; ++i) {
    if (PAGES[i] == page) {
      return count;
    }
    ++count;
  }
  return INT_MAX;
}
