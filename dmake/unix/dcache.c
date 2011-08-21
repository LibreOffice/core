/* $RCSfile: dcache.c,v $
-- $Revision: 1.9 $
-- last change: $Author: ihi $ $Date: 2007-10-15 15:52:47 $
--
-- SYNOPSIS
--      Directory cache management routines.
--
-- DESCRIPTION
--      This is the code that maintains a directory cache for each directory
--      that dmake visits.  The entire directory is thus only read once and
--      the need for performing costly 'stat' calls when performing target
--      inference is much reduced.  The improvement in performance should be
--      significant for NFS or remote mounted file systems.
--
-- AUTHOR
--      Dennis Vadura, dvadura@dmake.wticorp.com
--
-- WWW
--      http://dmake.wticorp.com/
--
-- COPYRIGHT
--      Copyright (c) 1996,1997 by WTI Corp.  All rights reserved.
--
--      This program is NOT free software; you can redistribute it and/or
--      modify it under the terms of the Software License Agreement Provided
--      in the file <distribution-root>/readme/license.txt.
--
-- LOG
--      Use cvs log to obtain detailed change logs.
*/

/* For Borland 5.00 compile, for some reason they seem to insist on pulling
 * in the winnt.h if __WIN32__ is defined and you include <dirent.h>.  This
 * is, in my opinion, a BUG! on Borland's part.
 */
#if defined(__BORLANDC__) && defined(__WIN32__)
#undef __WIN32__
#endif

#ifdef __APPLE__
#include <sys/types.h>
#endif
#include <dirent.h>
#include "extern.h"
#include "sysintf.h"


typedef struct ent {
   char    *name;
   uint32  hkey;
   time_t  mtime;
   int     isdir;
   struct ent *next;
} Entry, *EntryPtr;


typedef struct mydir {
   char         *path;
   uint32        hkey;
   EntryPtr      entries;
   struct mydir *next;
} DirEntry, *DirEntryPtr;

static DirEntryPtr dtab[HASH_TABLE_SIZE];


/* Stat a path using the directory cache.
 *
 * We build a cannonical representation of the path using either an absolute
 * path name if that is what 'path' is or the relative path name constructed
 * from 'path' and the present value of Pwd.
 *
 * The present value of Pwd then gives a directory path that we search for
 * in our cache using a hash lookup.  If the directory component is located
 * then we search the basename component of the path and return the result of
 * the search:  0L if the component is not in the cache and it's time stamp
 * otherwise.
 *
 * If the directory is not in our cache we insert it into the cache by
 * openning the directory and reading all of the files within.  Once read
 * then we return the result of the above search.
 *
 * Optionally, if force is TRUE, and we did NOT read the directory to provide
 * the result then stat the file anyway and update the internal cache.
 */

PUBLIC time_t
CacheStat(path, force)
char        *path;
int          force;
{
   struct stat stbuf;
   DirEntryPtr dp;
   EntryPtr    ep;
   uint32 hkey;
   uint16 hv;
   char *fpath;
   char *spath;
   char *comp;
   char *dir;
   char *udir; /* Hold the unchanged (DcacheRespCase) directory. */
   int  loaded=FALSE;

   if (If_root_path(path))
      spath = path;
   else
      spath = Build_path(Pwd,path);

   fpath = DmStrDup(spath);

   comp  = Basename(fpath); /* Use before the Filedir() call. */
   dir   = Filedir(fpath);

   /* do caching and comparing lower case if told so. */
   if( !STOBOOL(DcacheRespCase) ) {
      udir = DmStrDup(dir);
      strlwr(comp);
      strlwr(dir);
   } else
      udir = dir;

   hv = Hash(dir,&hkey);

   for(dp=dtab[hv]; dp; dp=dp->next)
      if (hkey == dp->hkey && strcmp(dp->path,dir) == 0)
     break;

   if (!dp) {
      /* Not cached yet, doing it now. */
      DIR *dirp;
      struct dirent *direntp;

      if( Verbose & V_DIR_CACHE )
     printf( "%s:  Caching directory [%s]\n", Pname, dir  );

      /* Load the directory, we have the right hash position already */
      loaded = TRUE;

      TALLOC(dp,1,DirEntry);
      dp->next = dtab[hv];
      dtab[hv] = dp;
      dp->path = DmStrDup(dir);
      dp->hkey = hkey;

      /* We use the unchanged (not potentially lowercased because of
       * DcacheRespCase) directory as this would fail on a case sensitive
       * file system.
       * Note: Using case insensitive directory caching on case sensitive
       * file systems is a *BAD* idea. If in doubt use case sensitive
       * directory caching even on case insensitive file systems as the
       * worst case in this szenario is that /foo/bar/ and /foo/BAR/ are
       * cached separately (with the same content) even though they are
       * the same directory. This would only happen if different targets
       * using different upper/lower case spellings for the same directory
       * and is *never* a good idea. */
      if (Set_dir(udir) == 0) {
     if((dirp=opendir(".")) != NIL(DIR)) {
        while((direntp=readdir(dirp)) != NULL) {
           TALLOC(ep,1,Entry);
           ep->name = DmStrDup(direntp->d_name); /* basename only */
           if( !STOBOOL(DcacheRespCase) )
          strlwr(ep->name);

           Hash(ep->name, &ep->hkey); /* This sets ep->hkey. */

           ep->next = dp->entries;
           dp->entries = ep;
           DMSTAT(direntp->d_name,&stbuf);
           ep->isdir = (stbuf.st_mode & S_IFDIR);
           ep->mtime = stbuf.st_mtime;
        }
        closedir(dirp);
     }
     Set_dir(Pwd);
      }
   }

   Hash(comp, &hkey); /* Calculate hkey. */

   /* search in dp->entries for comp. */
   if (dp) {
      for(ep=dp->entries; ep; ep=ep->next)
     if(hkey == ep->hkey && strcmp(ep->name,comp) == 0)
        break;
   }
   else
      ep = NULL;

   if( force && !loaded) {
      if (strlen(comp) > NameMax || DMSTAT(spath,&stbuf) != 0) {
     /* Either file to long or the stat failed. */
     if (strlen(comp) > NameMax)
        Warning( "File [%s] longer than value of NAMEMAX [%d].\n\
    Assume unix time 0.\n", comp, NameMax );
     if(ep)
        ep->mtime = 0L;
      }
      else {
     if (!ep) {
        TALLOC(ep,1,Entry);
        ep->name = DmStrDup(comp);
        if( !STOBOOL(DcacheRespCase) )
           strlwr(ep->name);
        Hash(ep->name, &ep->hkey);
        ep->next = dp->entries;
        ep->isdir = (stbuf.st_mode & S_IFDIR);
        dp->entries = ep;
     }

     ep->mtime = stbuf.st_mtime;
      }

      if( Verbose & V_DIR_CACHE )
     printf("%s:  Updating dir cache entry for [%s], new time is %ld\n",
            Pname, spath, ep ? ep->mtime : 0L);
   }

   if( udir != dir )
      FREE(udir); /* Keep this before the free of fpath. */

   FREE(fpath);
   return(!ep ? (time_t)0L : ((STOBOOL(Augmake) && ep->isdir)?0L:ep->mtime));
}
