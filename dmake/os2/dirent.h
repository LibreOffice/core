/* DIRLIB.H by M. J. Weinstein   Released to public domain 1-Jan-89 */

#ifndef _DIRLIB_h_
#define _DIRLIB_h_

#define INCL_DOSFILEMGR
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdmacs.h"

#define MAXNAMLEN   _MAX_FNAME

struct dirent {
   long              d_ino;
   unsigned short    d_reclen;
   unsigned short    d_namlen;
   char              d_name[MAXNAMLEN+1];
};

typedef struct {
   HDIR           dd_handle;    /* Handle for FindFirst/Next               */
   FILEFINDBUF3   dd_dta;       /* Disk transfer area for this dir.        */
   ULONG          dd_count;     /* Count for FindFirst/Next                */
   APIRET         dd_stat;      /* Status return from last lookup          */
   char           dd_name[1];   /* Full name of file -- struct is extended */
} DIR;

extern DIR           *opendir   ANSI((char *));
extern struct dirent *readdir   ANSI((DIR *));
extern long          telldir    ANSI((DIR *));
extern void          seekdir    ANSI((DIR *, long));
extern void          closedir   ANSI((DIR *));

#define rewinddir(dirp)   seekdir(dirp,0L)
#endif
