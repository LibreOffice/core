/*************************************************************************
 *
 *  $RCSfile: dcache.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 14:02:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*
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
   int  loaded=FALSE;

#ifdef __APPLE__
   /* On Mac OS X, open, stat, and other system calls are case-insenstive.
      Since this function keeps a case-sensitive cache, we need to force
      a stat of the file if there is no match in the cache just to make sure
      that we don't miss a file when only the case is different */
   force = TRUE;
#endif

   if (If_root_path(path))
      spath = path;
   else
      spath = Build_path(Pwd,path);

   fpath = DmStrDup(spath);
   comp  = Basename(fpath);
   dir   = Filedir(fpath);

   hv = Hash(dir,&hkey);

   for(dp=dtab[hv]; dp; dp=dp->next)
      if (hkey == dp->hkey && strcmp(dp->path,dir) == 0)
     break;

   if (!dp) {
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

      if (Set_dir(dir) == 0) {
     if((dirp=opendir(".")) != NIL(DIR)) {
        while((direntp=readdir(dirp)) != NULL) {
           TALLOC(ep,1,Entry);
           ep->name = DmStrDup(direntp->d_name);

               /* Perform case mapping of name if appropriate */
           DMSTRLWR(ep->name, comp);
               Hash(ep->name, &ep->hkey);

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

   Hash(comp, &hkey);

   if (dp) {
      for(ep=dp->entries; ep; ep=ep->next)
     if(hkey == ep->hkey && strcmp(ep->name,comp) == 0)
        break;
   }
   else
      ep = NULL;

   if( force && !loaded) {
      if (strlen(comp) > NameMax || DMSTAT(spath,&stbuf) != 0) {
     if(ep)
        ep->mtime = 0L;
      }
      else {
     if (!ep) {
        TALLOC(ep,1,Entry);
        ep->name = DmStrDup(comp);
        DMSTRLWR(ep->name, comp);
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

   FREE(fpath);
   return(!ep ? (time_t)0L : ((STOBOOL(Augmake) && ep->isdir)?0L:ep->mtime));
}
