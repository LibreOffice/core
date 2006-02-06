#ifndef _MYTHES_HXX_
#define _MYTHES_HXX_

// some maximum sizes for buffers
#define MAX_WD_LEN 200
#define MAX_LN_LEN 16384


// a meaning with definition, count of synonyms and synonym list
struct mentry {
  char*  defn;
  int  count;
  char** psyns;
};


class MyThes
{

       int  nw;                  /* number of entries in thesaurus */
       char**  list;               /* stores word list */
       unsigned int* offst;              /* stores offset list */
       char *  encoding;           /* stores text encoding; */

        FILE  *pdfile;

    // disallow copy-constructor and assignment-operator for now
    MyThes();
    MyThes(const MyThes &);
    MyThes & operator = (const MyThes &);

public:
    MyThes(const char* idxpath, const char* datpath);
    ~MyThes();

        // lookup text in index and return number of meanings
    // each meaning entry has a defintion, synonym count and pointer
        // when complete return the *original* meaning entry and count via
        // CleanUpAfterLookup to properly handle memory deallocation

        int Lookup(const char * pText, int len, mentry** pme);

        void CleanUpAfterLookup(mentry** pme, int nmean);

        char* get_th_encoding();

private:
        // Open index and dat files and load list array
        int thInitialize (const char* indxpath, const char* datpath);

        // internal close and cleanup dat and idx files
        void thCleanup ();

        // read a text line (\n terminated) stripping off line terminator
        int readLine(FILE * pf, char * buf, int nc);

        // binary search on null terminated character strings
        int binsearch(char * wrd, char* list[], int nlst);

        // string duplication routine
        char * mystrdup(const char * p);

        // remove cross-platform text line end characters
        void mychomp(char * s);

        // return index of char in string
        int mystr_indexOfChar(const char * d, int c);

};

#endif





