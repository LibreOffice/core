/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
/*
 * $Id: malloc.h,v 1.2 2006-07-25 10:08:50 rt Exp $
 */
struct mlist
{
    struct mlist    * next;         /* next entry in chain  */
    struct mlist    * prev;         /* prev entry in chain  */
    int       flag;         /* inuse flag       */
    unsigned int      r_size;       /* requested size   */
    union
    {
        unsigned int      size;     /* actual size      */
        double        unused_just_for_alignment;
    } s;
    char          data[4];
};

#define M_SIZE      ((int)(char *)((struct mlist *)0)->data)
#define M_RND       0x08

#define M_INUSE     0x01
#define M_MAGIC     0x03156100

#define M_BLOCKSIZE (1024*8)

#define M_FILL      '\01'
#define M_FREE_FILL '\02'

#define M_ROUNDUP(size) {\
                if( size & (M_RND-1) ) \
                { \
                    size &= ~(M_RND-1); \
                    size += M_RND; \
                } \
            }

/*
 * Malloc warning/fatal error handler defines...
 */
#define M_HANDLE_DUMP   0x80  /* 128 */
#define M_HANDLE_IGNORE 0
#define M_HANDLE_ABORT  1
#define M_HANDLE_EXIT   2
#define M_HANDLE_CORE   3

/*
 * Mallopt commands and defaults
 */

#define MALLOC_WARN 1       /* set malloc warning handling  */
#define MALLOC_FATAL    2       /* set malloc fatal handling    */
#define MALLOC_ERRFILE  3       /* specify malloc error file    */
#define MALLOC_CKCHAIN  4       /* turn on chain checking   */
union malloptarg
{
    int   i;
    char    * str;
};

/*
 * Malloc warning/fatal error codes
 */

#define M_CODE_CHAIN_BROKE  1   /* malloc chain is broken   */
#define M_CODE_NO_END       2   /* chain end != endptr      */
#define M_CODE_BAD_PTR      3   /* pointer not in malloc area   */
#define M_CODE_BAD_MAGIC    4   /* bad magic number in header   */
#define M_CODE_BAD_CONNECT  5   /* chain poingers corrupt   */
#define M_CODE_OVERRUN      6   /* data overrun in malloc seg   */
#define M_CODE_REUSE        7   /* reuse of freed area      */
#define M_CODE_NOT_INUSE    8   /* pointer is not in use    */
#define M_CODE_NOMORE_MEM   9   /* no more memory available */
#define M_CODE_OUTOF_BOUNDS 10  /* gone beyound bounds      */

void malloc_warning();
void malloc_fatal();
void malloc_check_data();
void malloc_check_str();
void malloc_verify();

