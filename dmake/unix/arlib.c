/* RCS  $Id: arlib.c,v 1.1.1.1 2000-09-22 15:33:33 hr Exp $
--
-- SYNOPSIS
--      Unix archive manipulation code.
--
-- DESCRIPTION
--  Originally this code was provided by Eric Gisin of MKS.  I took
--  his code and completely rewrote it adding cacheing of lib members
--  and other various optimizations.  I kept the overal functional
--  idea of the library routines as they are similar to those in GNU
--  make and felt it advantageous to maintain a similar interface.
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

/* Sun unix on 386i's has a broken ar.h that does not assume PORTAR format
 * by default, so we fix it here. */
#if defined(i386) || defined(__DGUX__)
#define PORTAR 1
#endif

#if !defined (COHERENT) && !defined(__COHERENT__)
#include <ar.h>
#else
#include <arcoff.h>
#endif /* COHERENT, __COHERENT__ */
#include "extern.h"
#include "sysintf.h"

/* By defining the defines below it is possible to configure the library
 * code for library cacheing/non-cacheing, ASCII archive headers, and a full
 * decode of the ar_hdr fields in the scan_ar function. */

#ifndef ASCARCH
#define ASCARCH         1   /* ASCII time stored in archive */
#endif

#ifndef LC
#define LC          1   /* Turn on library cacheing */
#endif

#ifndef CHECKELF
#define CHECKELF        1   /* Enable Elf long member names */
#endif

#ifndef DECODE_ALL_AR_FIELDS
#define DECODE_ALL_AR_FIELDS    0   /* decode only fields make needs*/
#endif

#ifndef AR_TRUNCATE_MEMBER_NAMES
#define AR_TRUNCATE_MEMBER_NAMES 0  /* truncate member names for */
#endif                  /* comparison.               */

#if LC
#  define FOUND_MEMBER  FALSE
#else
#  define FOUND_MEMBER  TRUE
#  define _cache_member(a, b, c)
#  define _check_cache(a, b, c, d)  FALSE
#endif

#define MAXFNAME      255       /* Max length of member name    */
#define MAXMNAME    8   /* Max module name < MAXFNAME   */


/* This struct is used to pass the library and member inrmation about the
 * routines that perform the library seeking/cacheing */
struct ar_args {
   char   *lib;
   char   *member;
   time_t time;
};


typedef struct AR {
   char    ar_name[MAXFNAME+1];      /* File name */
   long    ar_size;                  /* Size in bytes */
   time_t  ar_time;                  /* Modification time */

#ifdef DOS
   char    ar_modname[MAXMNAME+1];   /* DOS module name */
#endif

#if DECODE_ALL_AR_FIELDS
   uint16  ar_mode;                  /* File mode */
   uint16  ar_uid;                   /* File owner */
   uint16  ar_gid;                   /* File group owner */
#endif
} AR, *ARPTR;


static int ar_scan  ANSI((FILE *,
              int (*) ANSI((FILE *, struct AR *,struct ar_args *)),
              struct ar_args *));
static int ar_touch ANSI(( FILE *, time_t ));
static int time_function  ANSI(( FILE *, struct AR *, struct ar_args * ));
static int touch_function ANSI(( FILE *, struct AR *, struct ar_args * ));
static int ar_name_equal ANSI((char *, char *));

#if LC
static int _cache_member ANSI((char *, char *, time_t));
static int _check_cache  ANSI((char *, char *, time_t *, int));
#endif

/* decoded archive header */
static AR    _ar;
static off_t arhdroffset;  /* member seek offset */


PUBLIC time_t
seek_arch(name, lib)/*
======================
   Look for module 'name' inside 'lib'.  If compiled with cacheing then first
   check to see if the specified lib is cached.  If so then return that time
   stamp instead of looking into the library. */
char    *name;
char    *lib;
{
   FILE   *f;
   int    rv;
   time_t mtime;
   struct ar_args args;

   /* Check the cache first (if there is a cache) */
   if( _check_cache(name, lib, &mtime, FALSE) )  return( mtime );

   /* Open the lib file and perform the scan of the members, looking
    * for our particular member.  If cacheing is enabled it will be
    * taken care of automatically during the scan. */

   args.lib    = lib;
   args.member = name;
   args.time   = (time_t)0L;

   if( (f = fopen(lib, "r")) == NIL(FILE) ) return( (time_t)0L );
   rv = ar_scan(f, time_function, &args );
   fclose( f );

   if( rv < 0 ) Fatal("(%s): Invalid library format", lib);

   return( args.time );
}


int
touch_arch(name, lib)/*
=======================
   Look for module 'name' inside 'lib'.  If compiled with cacheing then first
   check to see if the specified lib is cached.  If so then set that time
   stamp and write it into the library.  Returns 0 on success, non-zero
   on failure. */
char   *name;
char   *lib;
{
   FILE   *f;
   int    rv;
   struct ar_args args;

   /* Open the lib file and perform the scan of the members, looking
    * for our particular member.  If cacheing is enabled it will be
    * taken care of automatically during the scan. */

   args.lib    = lib;
   args.member = name;
   args.time   = (time_t)0L;

   if( (f = fopen(lib, "r+")) == NIL(FILE) ) return( (time_t)1L );
   rv = ar_scan(f, touch_function, &args );
   fclose( f );

   if( rv < 0 ) Fatal("(%s): Invalid library format", lib);

   return( 0 );
}



static int
time_function(f, arp, argp)/*
=============================
   get library member's time, if it matches than return it in argp, if
   cacheing is enabled than cache the library members also. */
FILE           *f;      /* library file          */
struct AR      *arp;    /* library member header */
struct ar_args *argp;
{
   int rv = _cache_member( arp->ar_name, argp->lib, arp->ar_time );

   if( ar_name_equal (argp->member, arp->ar_name)) {
      argp->time = arp->ar_time;

      if( arp->ar_time == 0 && !(Glob_attr & A_SILENT) )
         Warning( "(%s): Can't extract library member timestamp; using EPOCH",
              argp->member);

      return( rv );  /* 1 => no cacheing, 0 => cacheing */
   }

   return( FALSE ); /* continue scan */
}



static int
touch_function(f, arp, argp)/*
==============================
   Update library member's time stamp, and write new time value into cache
   if required. */
FILE           *f;      /* library file */
struct AR      *arp;    /* library member header */
struct ar_args *argp;
{
   extern time_t time ANSI(( time_t * ));
   time_t now = time((time_t*) NULL);  /* Current time.       */

   if( ar_name_equal(argp->member, arp->ar_name) ) {
      _check_cache( argp->member, argp->lib, &now, TRUE );
      ar_touch(f, now );

      return( TRUE );
   }

   return( FALSE ); /* continue scan */
}


static int
ar_name_equal (char * name1, char * name2)
{
  int equal;

#if AR_TRUNCATE_MEMBER_NAMES
  struct ar_hdr hdr;

  equal = !strncmp (name1, name2, sizeof (hdr.ar_name)-1);
#else
  equal = !strcmp (name1, name2);
#endif

  return equal;
}


static int
ar_scan(f, function, arg)/*
===========================
   Scan the opened archive, and call the given function for each member found.
   The function will be called with the file positioned at the beginning of
   the member and it can read up to arp->ar_size bytes of the archive member.
   If the function returns 1, we stop and return 1.  We return 0 at the end
   of the archive, or -1 if the archive has invalid format.  This interface
   is more general than required by "make", but it can be used by other
   utilities.  */
register FILE *f;
int      (*function) ANSI((FILE *, struct AR *, struct ar_args *));
struct   ar_args *arg;
{
   extern long atol ();
   register char *p;
   struct ar_hdr arhdr;   /* archive member header   */
   long      nsize;   /* size of member name     */
   long          arind=0; /* archive index offset    */
   int           process;
#if defined(_AIX)
   struct fl_hdr flhdr;   /* archive file header     */
   char   magic[SAIAMAG]; /* size of magic string    */
#else
#if ASCARCH
   char magic[SARMAG];
#else
   unsigned short word;
#endif
#endif

   fseek( f, 0L, 0 );   /* Start at the beginning of the archive file */

#if ASCARCH
#if defined(_AIX)
   fread( (char *)&flhdr, sizeof(flhdr), 1, f );
   if( strncmp(flhdr.fl_magic,AIAMAG, SAIAMAG) != 0 ) return(-1);
   fseek(f, atol(flhdr.fl_fstmoff), 0 ); /* postition to first member */
#else
   fread( magic, sizeof(magic), 1, f );
   if( strncmp(magic, ARMAG, SARMAG) != 0 ) return( -1 );
#endif
#else
   fread( (char*)&word, sizeof(word), 1, f );
   if( word != ARMAG ) return( -1 );
#endif

   /* scan the library, calling `function' for each member
    */
   while( 1 ) {
      arhdroffset = ftell(f);
#if defined(_AIX)
      if( fread((char*)&arhdr,sizeof(arhdr)-sizeof(arhdr._ar_name),1,f)!=1)
         break;
      nsize = atoi(arhdr.ar_namlen);
      fseek(f, arhdroffset+(unsigned long)(((struct ar_hdr *)0)->_ar_name.ar_name), 0);
      if( fread((char*)_ar.ar_name,nsize,1,f)!=1)
         break;
      _ar.ar_name[nsize]='\0';
#else
      if( fread((char*) &arhdr, sizeof(arhdr), 1, f) != 1 ) break;
      strncpy(_ar.ar_name, arhdr.ar_name, nsize = sizeof(arhdr.ar_name));
#endif

      for( p = &_ar.ar_name[nsize];
           --p >= _ar.ar_name && *p == ' ';);

      p[1] = '\0';
      if( *p == '/' ) *p = 0;   /* SysV has trailing '/' */

      /* check to see if this is an archive index using SsysV Index scheme.
       * see ar(4) man page for more info */
#if CHECKELF
      if( _ar.ar_name[0] == '/' && _ar.ar_name[1] == '\0' ) {
     arind = arhdroffset+sizeof(arhdr);
     process = 0;
      }
      else
#endif
     process = 1;

#if !defined(_AIX)
#if ASCARCH
      if( strncmp(arhdr.ar_fmag, ARFMAG, sizeof(arhdr.ar_fmag)) != 0 )
     return( -1 );
      _ar.ar_time = atol(arhdr.ar_date);
      _ar.ar_size = atol(arhdr.ar_size);
#else
      _ar.ar_time = arhdr.ar_date;
      _ar.ar_size = arhdr.ar_size;
#endif
#if CHECKELF
      /* check for names of the form /xxxx where xxxx is an offset into the
       * name table pointed at by arind. */
      if(arind && _ar.ar_name[0] == '/') {
     long offset = atol(_ar.ar_name+1);
     long here = ftell(f);
     int  c;

     fseek(f, arind+offset, 0);
     p = _ar.ar_name;
     while((c=fgetc(f)) != EOF) {
        *p++ = c;
        if(c == '/') {
           p[-1] = '\0';
           break;
        }
     }

     if (c==EOF) return(-1);          /* 'c' should never be EOF */
     fseek(f, here, 0);
      }
#endif
#else
#if ASCARCH
      _ar.ar_time = atol(arhdr.ar_date);
      _ar.ar_size = atol(arhdr.ar_nxtmem);
#else
      _ar.ar_time = arhdr.ar_date;
      _ar.ar_size = arhdr.ar_nxtmem;
#endif
#endif


#if DECODE_ALL_AR_FIELDS
#if ASCARCH
      _ar.ar_mode = atoi(arhdr.ar_mode);
      _ar.ar_uid  = atoi(arhdr.ar_uid);
      _ar.ar_gid  = atoi(arhdr.ar_gid);
#else
      _ar.ar_mode = arhdr.ar_mode;
      _ar.ar_uid = arhdr.ar_uid;
      _ar.ar_gid = arhdr.ar_gid;
#endif
#endif
      if( process && (*function)(f, &_ar, arg) ) return( 1 );

#if defined(_AIX)
      if( _ar.ar_size == 0L ) break;
      fseek( f, (long) _ar.ar_size, 0 );
#else
      fseek( f, arhdroffset + sizeof(arhdr) + (_ar.ar_size+1 & ~1L), 0 );
#endif
   }

#if !defined(_AIX)
   if( !feof(f) ) return( -1 );
#endif
   return 0;
}



static int
ar_touch( f, now )/*
====================
   touch module header timestamp. */
FILE   *f;
time_t now;
{
   struct ar_hdr arhdr;                /* external archive header */

   fseek(f, arhdroffset + (unsigned long)(((struct ar_hdr *)0)->ar_date), 0);

#if ASCARCH
   fprintf(f, "%lu", now);
#else
   fwrite((char *)now, sizeof(now), 1, f);
#endif

   return( ferror(f) ? 0 : 1 );
}


#if LC
typedef struct mem {
   time_t   m_time;     /* modify time of member*/
   struct mem   *m_next;    /* next member in lib   */
   char     m_valid;    /* valid cache entry    */
   char     m_name[1];  /* lib member name  */
} MEM, *MEMPTR;

typedef struct lib {
   struct lib   *lb_next;   /* next library in list */
   struct mem   *lb_members;    /* list of lib members  */
   char     lb_valid;   /* valid cache entry    */
   char     *lb_name;   /* library name     */
} LIB, *LIBPTR;

static LIBPTR _cache = NIL(LIB);
static MEMPTR _find_member ANSI(( LIBPTR, char * ));

static int
_check_cache( name, lib, pmtime, touch )/*
==========================================
   Check to see if we have cached member in lib, if so return time in pmtime
   and return TRUE, otherwise return FALSE, if touch is TRUE then touch
   the archive member instead. */
char   *name;
char   *lib;
time_t *pmtime;
int    touch;
{
   register MEMPTR mp;
   register LIBPTR lp;

   for( lp=_cache; lp != NIL(LIB) && lp->lb_name != lib; lp=lp->lb_next );
   if( lp == NIL(LIB) ) return( FALSE );

   mp = _find_member( lp, name );
   if( mp == NIL(MEM) || !mp->m_valid ) return( FALSE );

   if( touch == TRUE )
   {
      mp->m_time = *pmtime;
      mp->m_valid = 1;
   }
   else
      *pmtime = mp->m_time;

   lp->lb_valid   = 1;
   lp->lb_members = mp;

   return( TRUE );
}



static int
_cache_member( name, lib, mtime )/*
===================================
   Cache name in lib along with it's time */
char   *name;
char   *lib;
time_t mtime;
{
   register MEMPTR mp;
   register LIBPTR lp;

   for( lp=_cache;
    lp != NIL(LIB) && lp->lb_name != NIL(char) && lp->lb_name != lib;
    lp=lp->lb_next);

   if( lp == NIL(LIB) )
   {
      lp = (LIBPTR) malloc(sizeof(LIB));
      if( lp == NIL(LIB) ) No_ram();

      lp->lb_name    = lib;
      lp->lb_members = NIL(MEM);
      lp->lb_next    = _cache;
      lp->lb_valid   = 0;
      _cache = lp;
   }

   /* On UNIX ar does not allow multiple copies of the same .o file to live
    * in the same AR file.  If this is not TRUE then use the commented out
    * version to set the value of mp. */

   /*mp = _find_member(lp, name);*/
   mp = NIL(MEM);

   if( mp == NIL(MEM) )
   {
      mp = (MEMPTR) malloc(sizeof(char)*offsetof(MEM,m_name[strlen(name)+1]));
      if( mp == NIL(MEM) ) No_ram();

      strcpy( mp->m_name, name );
      mp->m_time     = mtime;

      if( lp->lb_members == NIL(MEM) ) {
     mp->m_next     = mp;
     lp->lb_members = mp;
      }
      else {
     mp->m_next = lp->lb_members->m_next;
     lp->lb_members->m_next = mp;
     lp->lb_members = mp;
      }
   }
   else
      mp->m_time = mtime;

   mp->m_valid = 1;

   return( lp->lb_valid );
}


static MEMPTR
_find_member( lp, name )
LIBPTR lp;
char   *name;
{
   register MEMPTR mp = lp->lb_members;

   if( mp == NIL(MEM) ) return(mp);

   do {
      if( !strcmp(mp->m_name, name ) ) return( mp );
      mp = mp->m_next;
   }
   while( mp != lp->lb_members );

   return( NIL(MEM) );
}
#endif



void
void_lcache( lib, member )/*
============================
   Void the library cache for lib.  If member is NIL(char) then nuke all
   of the members, if member is NOT NIL(char) then invalidate only that
   member. */
char *lib;
char *member;
{
#if LC
   register LIBPTR lp;
   register MEMPTR mp;
   register MEMPTR tmp;

   for( lp=_cache; lp != NIL(LIB) && lp->lb_name != lib; lp=lp->lb_next );
   if( lp == NIL(LIB) ) return;

   if( member == NIL(char) ) {
      mp = lp->lb_members;
      do {
     tmp = mp->m_next;
     (void) free( mp );
     mp = tmp;
      } while( mp != lp->lb_members );

      lp->lb_valid   = 0;
      lp->lb_members = NIL(MEM);
      lp->lb_name    = NIL(char);
   }
   else {
      mp=lp->lb_members;
      do {
     if( strcmp( member, mp->m_name) == 0 ) {
        lp->lb_members = mp->m_next;
        mp->m_valid = 0;
     }

     mp=mp->m_next;
      } while( mp != lp->lb_members );
   }
#endif
}
