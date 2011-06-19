/* RCS  $Id: sysintf.c,v 1.13 2008-03-05 18:30:58 kz Exp $
--
-- SYNOPSIS
--      System independent interface
--
-- DESCRIPTION
--  These are the routines constituting the system interface.
--  The system is taken to be essentially POSIX conformant.
--  The original code was extensively revised by T J Thompson at MKS,
--  and the library cacheing was added by Eric Gisin at MKS.  I then
--  revised the code yet again, to improve the lib cacheing, and to
--  make it more portable.
--
--  The following is a list of routines that are required by this file
--  in order to work.  These routines are provided as functions by the
--  standard C lib of the target system or as #defines in system/sysintf.h
--  or via appropriate C code in the system/ directory for the given
--  system.
--
--  The first group must be provided by a file in the system/ directory
--  the second group is ideally provided by the C lib.  However, there
--  are instances where the C lib implementation of the specified routine
--  does not exist, or is incorrect.  In these instances the routine
--  must be provided by the the user in the system/ directory of dmake.
--  (For example, the bsd/ dir contains code for putenv(), and tempnam())
--
--  DMAKE SPECIFIC:
--      seek_arch()
--      touch_arch()
--      void_lcache()
--      runargv()
--      DMSTAT()
--      Remove_prq()
--
--  C-LIB SPECIFIC:  (should be present in your C-lib)
--      utime()
--      time()
--      getenv()
--      putenv()
--      getcwd()
--      signal()
--      chdir()
--      tempnam()
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

#include "extern.h"

/* The following definition controls the use of GetModuleFileName() */
#if defined(_MSC_VER) || defined(__MINGW32__)
#   define HAVE_GETMODULEFILENAMEFUNC 1

/* this is needed for the _ftime call below. Only needed here. */
#   include <sys/timeb.h>
#endif

/* for cygwin_conv_to_posix_path() in Prolog() and for cygdospath()*/
#if __CYGWIN__
#   include <sys/cygwin.h>
#endif

#include "sysintf.h"
#if HAVE_ERRNO_H
#  include <errno.h>
#else
   extern  int  errno;
#endif

/*
** Tries to stat the file name.  Returns 0 if the file
** does not exist.  Note that if lib is not null it tries to stat
** the name found inside lib.
**
** If member is NOT nil then look for the library object which defines the
** symbol given by name.  If found DmStrDup the name and return make the
** pointer pointed at by sym point at it.  Not handled for now!
*/
static time_t
really_dostat(name, buf)
char *name;
struct stat *buf;
{
   return( (   DMSTAT(name,buf)==-1
            || (STOBOOL(Augmake) && (buf->st_mode & S_IFDIR)))
       ? (time_t)0L
       : (time_t) buf->st_mtime
     );
}


PUBLIC time_t
Do_stat(name, lib, member, force)
char *name;
char *lib;
char **member;
int  force;
{
   struct stat buf;
   time_t seek_arch();

   if( member != NIL(char *) )
      Fatal("Library symbol names not supported");

   buf.st_mtime = (time_t)0L;
   if( lib != NIL(char) )
      return( seek_arch(Basename(name), lib) );
   else if( strlen(Basename(name)) > NameMax ) {
      Warning( "Filename [%s] longer than value of NAMEMAX [%d].\n\
      Assume unix time 0.\n", Basename(name), NameMax );
      return((time_t)0L);
   }
   else if( STOBOOL(UseDirCache) )
      return(CacheStat(name,force));
   else
      return(really_dostat(name,&buf));
}


/* Touch existing file to force modify time to present.
 */
PUBLIC int
Do_touch(name, lib, member)
char *name;
char *lib;
char **member;
{
   if( member != NIL(char *) )
      Fatal("Library symbol names not supported");

   if (lib != NIL(char))
      return( touch_arch(Basename(name), lib) );
   else if( strlen(Basename(name)) > NameMax ) {
      Warning( "Filename [%s] longer than value of NAMEMAX [%d].\n\
      File timestamp not updated to present time.\n", Basename(name), NameMax );
      return(-1);
   }
   else
#ifdef HAVE_UTIME_NULL
      return( utime(name, NULL) );
#else
#   error "Utime NULL not supported"
#endif
}



PUBLIC void
Void_lib_cache( lib_name, member_name )/*
=========================================
   Void the library cache for lib lib_name, and member member_name. */
char *lib_name;
char *member_name;
{
   VOID_LCACHE( lib_name, member_name );
}



/*
** return the current time
*/
PUBLIC time_t
Do_time()
{
   return (time( NIL(time_t) ));
}



/*
** Print profiling information
*/
PUBLIC void
Do_profile_output( text, mtype, target )
char *text;
uint16 mtype;
CELLPTR target;
{

   time_t time_sec;
   uint32 time_msec;
   char *tstrg;
   char *tname;

#ifdef HAVE_GETTIMEOFDAY
   struct timeval timebuffer;
   gettimeofday(&timebuffer, NULL);
   time_sec = timebuffer.tv_sec;
   time_msec = timebuffer.tv_usec/1000;
#else
#if defined(_MSC_VER) || defined(__MINGW32__)
   struct _timeb timebuffer;
   _ftime( &timebuffer );
   time_sec = timebuffer.time;
   time_msec = timebuffer.millitm;
#   else
   time_sec = time( NIL(time_t) );
   time_msec = 0;
#   endif
#endif

   tname = target->CE_NAME;
   if( mtype & M_TARGET ) {
      tstrg = "target";
      /* Don't print special targets .TARGETS and .ROOT */
      if( tname[0] == '.' && (strcmp(".TARGETS", tname) == 0 || \
                  strcmp(".ROOT", tname) == 0) ) {
     return;
      }
   } else {
      tstrg = "recipe";
   }

   /* Don't print shell escape targets if not especially requested. */
   if( (target->ce_attr & A_SHELLESC) && !(Measure & M_SHELLESC) ) {
      return;
   }

   /* Print absolute path if requested. */
   if( !(target->ce_attr & A_SHELLESC) && (Measure & M_ABSPATH) ) {
      printf("%s %s %lu.%.3u %s%s%s\n",text, tstrg, time_sec, time_msec, Pwd, DirSepStr, tname);
   } else {
      printf("%s %s %lu.%.3u %s\n",text, tstrg, time_sec, time_msec, tname);
   }
}



PUBLIC int
Do_cmnd(cmd, group, do_it, target, cmnd_attr, last)/*
=====================================================
  Execute the string passed in as a command and return
  the return code. The command line arguments are
  assumed to be separated by spaces or tabs.  The first
  such argument is assumed to be the command.

  If group is true then this is a group of commands to be fed to the
  the shell as a single unit.  In this case cmd is of the form
  "file" indicating the file that should be read by the shell
  in order to execute the command group.

  If Wait_for_completion is TRUE add the A_WFC attribute to the new
  process.
*/
char  **cmd; /* Simulate a reference to *cmd. */
int     group;  /* if set cmd contains the filename of a (group-)shell
         * script. */
int     do_it;  /* Only execute cmd if not set to null. */
CELLPTR target;
t_attr  cmnd_attr; /* Attributes for current cmnd. */
int     last;   /* Last recipe line in target. */
{
   int  i;

   DB_ENTER( "Do_cmnd" );

   if( !do_it ) {
      if( last && !Doing_bang ) {
     /* Don't execute, just update the target when using '-t'
      * switch. */
         Update_time_stamp( target );
      }
      DB_RETURN( 0 );
   }

   /* Stop making the rest of the recipies for this target if an error occurred
    * but the Continue (-k) flag is set to build as much as possible. */
   if ( target->ce_attr & A_ERROR ) {
      if ( last ) {
     Update_time_stamp( target );
      }
      DB_RETURN( 0 );
   }

   if( Max_proc == 1 ) Wait_for_completion = TRUE;

   /* Tell runargv() to wait if needed. */
   if( Wait_for_completion ) cmnd_attr |= A_WFC;

   /* remove leading whitespace - This should never trigger! */
   if( iswhite(**cmd) ) {
      char *p;
      if( (p = DmStrSpn(*cmd," \t") ) != *cmd )
     strcpy(*cmd,p);
   }

   /* set shell if shell metas are found */
   if( (cmnd_attr & A_SHELL) || group || (*DmStrPbrk(*cmd, Shell_metas)!='\0') )
      cmnd_attr |= A_SHELL; /* If group is TRUE this doesn't hurt. */

   /* runargv() returns either 0 or 1, 0 ==> command executed, and
    * we waited for it to return, 1 ==> command started and is still
    * running. */
   i = runargv(target, group, last, cmnd_attr, cmd);

   DB_RETURN( i );
}


#define MINARGV 64

PUBLIC char **
Pack_argv( group, shell, cmd )/*
================================
  Take a command and pack it into an argument vector to be executed.
  If group is true cmd holds the group script file.
*/
int    group;
int    shell;
char **cmd; /* Simulate a reference to *cmd. */
{
   static char **av = NIL(char *);
   static int   avs = 0;
   int i = 0;
   char *s; /* Temporary string pointer. */

   if( av == NIL(char *) ) {
      TALLOC(av, MINARGV, char*);
      avs = MINARGV;
   }
   av[0] = NIL(char);

   if (**cmd) {
      if( shell||group ) {
     char* sh = group ? GShell : Shell;

     if( sh != NIL(char) ) {
        av[i++] = sh;
        if( (av[i] = (group?GShell_flags:Shell_flags)) != NIL(char) ) i++;

        if( shell && Shell_quote && *Shell_quote ) {
           /* Enclose the shell command with SHELLCMDQUOTE. */
           s = DmStrJoin(Shell_quote, *cmd, -1, FALSE);
           FREE(*cmd);
           *cmd = DmStrJoin(s, Shell_quote, -1, TRUE);
        }
        av[i++] = *cmd;

#if defined(USE_CREATEPROCESS)
        /* CreateProcess() needs one long command line. */
        av[0] = DmStrAdd(av[0], av[1], FALSE);
        av[1] = NIL(char);
        /* i == 3 means Shell_flags are given. */
        if( i == 3 ) {
           s = av[0];
           av[0] = DmStrAdd(s, av[2], FALSE);
           FREE(s);
           av[2] = NIL(char);
        }
        /* The final free of cmd will free the concated command line. */
        FREE(*cmd);
        *cmd = av[0];
#endif
        av[i]   = NIL(char);
     }
     else
        Fatal("%sSHELL macro not defined", group?"GROUP":"");
      }
      else {
     char *tcmd = *cmd;

#if defined(USE_CREATEPROCESS)
     /* CreateProcess() needs one long command line, fill *cmd
      * into av[0]. */
     while( iswhite(*tcmd) ) ++tcmd;
     if( *tcmd ) av[i++] = tcmd;
#else
     /* All other exec/spawn functions need the parameters separated
      * in the argument vector. */
     do {
        /* Fill *cmd into av[]. Whitespace is converted into '\0' to
         * terminate each av[] member. */
        while( iswhite(*tcmd) ) ++tcmd;
        if( *tcmd ) av[i++] = tcmd;

        while( *tcmd != '\0' && !iswhite(*tcmd) ) ++tcmd;
        if( *tcmd ) *tcmd++ = '\0';

        /* dynamically increase av size. */
        if( i == avs ) {
           avs += MINARGV;
           av = (char **) realloc( av, avs*sizeof(char *) );
        }
     } while( *tcmd );
#endif

     av[i] = NIL(char);
      }
   }

   return(av);
}


/*
** Return the value of ename from the environment
** if ename is not defined in the environment then
** NIL(char) should be returned
*/
PUBLIC char *
Read_env_string(ename)
char *ename;
{
   return( getenv(ename) );
}


/*
** Set the value of the environment string ename to value.
**  Returns 0 if success, non-zero if failure
*/
PUBLIC int
Write_env_string(ename, value)
char *ename;
char *value;
{
#if defined(HAVE_SETENV)

  return( setenv(ename, value, 1) );

#else  /* !HAVE_SETENV */

  char*   p;
  char*   envstr = DmStrAdd(ename, value, FALSE);

  p = envstr+strlen(ename); /* Don't change this code, DmStrAdd does not */
  *p++ = '=';           /* add the space if *value is 0, it does    */
  if( !*value ) *p = '\0';  /* allocate enough memory for one though.   */

  return( putenv(envstr) ); /* Possibly leaking 'envstr' */

#endif /* !HAVE_SETENV */
}


PUBLIC void
ReadEnvironment()
{
   extern char **Rule_tab;
#if !defined(_MSC_VER)
#if defined(__BORLANDC__) && __BORLANDC__ >= 0x500
   extern char ** _RTLENTRY _EXPDATA environ;
#else
   extern char **environ;
#endif
#endif
   char **rsave;

#if !defined(__ZTC__) && !defined(_MPW)
# define make_env()
# define free_env()
#else
   void make_env();
   void free_env();
#endif

   make_env();

   rsave    = Rule_tab;
   Rule_tab = environ;
   Readenv  = TRUE;

   Parse( NIL(FILE) );

   Readenv  = FALSE;
   Rule_tab = rsave;

   free_env();
}



/*
** All we have to catch is SIGINT
*/
PUBLIC void
Catch_signals(fn)
void (*fn)(int);
{
   /* FIXME: Check this and add error handling. */
   if( (void (*)(int)) signal(SIGINT, SIG_IGN) != (void (*)(int))SIG_IGN )
      signal( SIGINT, fn );
   if( (void (*)(int)) signal(SIGQUIT, SIG_IGN) != (void (*)(int))SIG_IGN )
      signal( SIGQUIT, fn );
}



/*
** Clear any previously set signals
*/
PUBLIC void
Clear_signals()
{
   if( (void (*)())signal(SIGINT, SIG_IGN) != (void (*)())SIG_IGN )
      signal( SIGINT, SIG_DFL );
   if( (void (*)())signal(SIGQUIT, SIG_IGN) != (void (*)())SIG_IGN )
      signal( SIGQUIT, SIG_DFL );
}



/*
** Set program name
*/
PUBLIC void
Prolog(argc, argv)
int   argc;
char* argv[];
{
   Pname = (argc == 0) ? DEF_MAKE_PNAME : argv[0];

   /* Only some native Windows compilers provide this functionality. */
#ifdef HAVE_GETMODULEFILENAMEFUNC
   if( (AbsPname = MALLOC( PATH_MAX, char)) == NIL(char) ) No_ram();
   GetModuleFileName(NULL, AbsPname, PATH_MAX*sizeof(char));
#else
   AbsPname = "";
#endif

#if __CYGWIN__
   /* Get the drive letter prefix used by cygwin. */
   if ( (CygDrvPre = MALLOC( PATH_MAX, char)) == NIL(char) )
      No_ram();
   else {
      int err = cygwin_conv_to_posix_path("c:", CygDrvPre);
      if (err)
     Fatal( "error converting \"%s\" - %s\n",
        CygDrvPre, strerror (errno));
      if( (CygDrvPreLen = strlen(CygDrvPre)) == 2 ) {
     /* No prefix */
     *CygDrvPre = '\0';
     CygDrvPreLen = 0;
      } else {
     /* Cut away the directory letter. */
     CygDrvPre[CygDrvPreLen-2] = '\0';
     /* Cut away the leading '/'. We don't free the pointer, i.e. choose
      * the easy way. */
     CygDrvPre++;
     CygDrvPreLen -= 3;
      }
   }
#endif

   /* DirSepStr is used from Clean_path() in Def_cell(). Set it preliminary
    * here, it will be redefined later in Create_macro_vars() in imacs.c. */
   DirSepStr = "/";

   Root = Def_cell( ".ROOT" );
   Targets = Def_cell( ".TARGETS" );
   Add_prerequisite(Root, Targets, FALSE, FALSE);

   Targets->ce_flag = Root->ce_flag = F_RULES|F_TARGET|F_STAT;
   Targets->ce_attr = Root->ce_attr = A_NOSTATE|A_PHONY;

   Root->ce_flag |= F_MAGIC;
   Root->ce_attr |= A_SEQ;

   tzset();
}



/*
** Do any clean up for exit.
*/
PUBLIC void
Epilog(ret_code)
int ret_code;
{
   Write_state();
   Unlink_temp_files(Root);
   Hook_std_writes(NIL(char));      /* For MSDOS tee (-F option) */
   exit( ret_code );
}



/*
** Use the built-in functions of the operating system to get the current
** working directory.
*/
PUBLIC char *
Get_current_dir()
{
   static char buf[PATH_MAX+2];

   if( !getcwd(buf, sizeof(buf)) )
      Fatal("Internal Error: Error when calling getcwd()!");

#ifdef __EMX__
   char *slash;
   slash = buf;
   while( (slash=strchr(slash,'/')) )
      *slash = '\\';
#endif

   return buf;
}



/*
** change working directory
*/
PUBLIC int
Set_dir(path)
char*   path;
{
   return( chdir(path) );
}



/*
** return switch char
*/
PUBLIC char
Get_switch_char()
{
   return( getswitchar() );
}


int Create_temp(tmpdir, path)/*
===============================
  Create a temporary file and open with exclusive access
  Path is updated with the filename and the file descriptor
  is returned.  Note that the new name should be freed when
  the file is removed.
*/
char *tmpdir;
char **path;
{
   int fd; /* file descriptor */

#if defined(HAVE_MKSTEMP)
   mode_t       mask;

   *path = DmStrJoin( tmpdir, DirSepStr, -1, FALSE);
   *path = DmStrJoin( *path, "mkXXXXXX", -1, TRUE );

   mask = umask(0066);
   fd = mkstemp( *path );
   umask(mask);

#elif defined(HAVE_TEMPNAM)
   char pidbuff[32];
#if _MSC_VER >= 1300
   /* Create more unique filename for .NET2003 and newer. */
   long npid;
   long nticks;

   npid = _getpid();
   nticks = GetTickCount() & 0xfff;
   sprintf(pidbuff,"mk%d_%d_",npid,nticks);
#else
   sprintf(pidbuff,"mk");
#endif
   *path = tempnam(tmpdir, pidbuff);
   fd = open(*path, O_CREAT | O_EXCL | O_TRUNC | O_RDWR, 0600);
#else

#error mkstemp() or tempnam() is needed

#endif

   return fd;
}


PUBLIC FILE*
Get_temp(path, mode)/*
======================
  Generate a temporary file name and open the file for writing.
  If a name cannot be generated or the file cannot be opened
  return -1, else return the fileno of the open file.
  and update the source file pointer to point at the new file name.
  Note that the new name should be freed when the file is removed.
  The file stream is opened with the given mode.
*/
char **path;
char *mode;
{
   int          fd;
   FILE         *fp;
   char         *tmpdir;
   int          tries = 20;

   tmpdir = Read_env_string( "TMPDIR" );
   if( tmpdir == NIL(char) )
      tmpdir = "/tmp";

   while( --tries )
   {
      /* This sets path to the name of the created temp file. */
      if( (fd = Create_temp(tmpdir, path)) != -1)
         break;

      free(*path); /* free var if creating temp failed. */
   }

   if( fd != -1)
   {
      Def_macro( "TMPFILE", DO_WINPATH(*path), M_MULTI|M_EXPANDED );
      /* associate stream with file descriptor */
      fp = fdopen(fd, mode);
   }
   else
      fp = NIL(FILE);

   return fp;
}


PUBLIC FILE *
Start_temp( suffix, cp, fname )/*
=================================
  Open a new temporary file and set it up for writing. The file is linked
  to cp and will be removed if once the target is finished.
  If a suffix for the temporary files is requested two temporary files are
  created. This is done because the routines that create a save temporary
  file do not provide a definable suffix. The first (provided by Get_temp())
  is save and unique and the second file is generated by adding the desired
  suffix the the first temporary file. The extra file is also linked to cp
  so that it gets removed later.
  The function returns the FILE pointer to the temporary file (with suffix
  if specified) and leaves the file name in *fname.
*/
char     *suffix;
CELLPTR   cp;
char    **fname;
{
   FILE        *fp, *fp2;
   char        *tmpname;
   char        *name;
   char        *fname_suff;

   name = (cp != NIL(CELL))?cp->CE_NAME:"makefile text";

   /* This sets tmpname to the name that was used. */
   if( (fp = Get_temp(&tmpname, "w")) == NIL(FILE) )
      Open_temp_error( tmpname, name );

   /* Don't free tmpname, it's stored in a FILELIST member in Link_temp(). */
   Link_temp( cp, fp, tmpname );
   *fname = tmpname;

   /* As Get_temp() doesn't provide a definable suffix (anymore) we create an
    * additional temporary file with that suffix. */
   if ( suffix && *suffix ) {

#ifdef HAVE_MKSTEMP
      /* Only use umask if we are also using mkstemp - this basically
       * avoids using the incompatible implementation from MSVC. */
      mode_t mask;

      mask = umask(0066);
#endif

      fname_suff = DmStrJoin( tmpname, suffix, -1, FALSE );

      /* Overwrite macro, Get_temp didn't know of the suffix. */
      Def_macro( "TMPFILE", DO_WINPATH(fname_suff), M_MULTI|M_EXPANDED );

      if( (fp2 = fopen(fname_suff, "w" )) == NIL(FILE) )
         Open_temp_error( fname_suff, name );
#ifdef HAVE_MKSTEMP
      umask(mask);
#endif

      /* Don't free fname_suff. */
      Link_temp( cp, fp2, fname_suff );
      fp = fp2;
      *fname = fname_suff;
   }

   return( fp );
}


/*
** Issue an error on failing to open a temporary file
*/
PUBLIC void
Open_temp_error( tmpname, name )
char *tmpname;
char *name;
{
   Fatal("Cannot open temp file `%s' while processing `%s'", tmpname, name );
}


/*
** Link a temp file onto the list of files.
*/
PUBLIC void
Link_temp( cp, fp, fname )
CELLPTR cp;
FILE   *fp;
char   *fname;
{
   FILELISTPTR new;

   if( cp == NIL(CELL) ) cp = Root;

   TALLOC( new, 1, FILELIST );

   new->fl_next = cp->ce_files;
   new->fl_name = fname;
   new->fl_file = fp;       /* indicates temp file is open */

   cp->ce_files = new;
}


/*
** Close a previously used temporary file.
*/
PUBLIC void
Close_temp(cp, file)
CELLPTR cp;
FILE    *file;
{
   FILELISTPTR fl;
   if( cp == NIL(CELL) ) cp = Root;

   for( fl=cp->ce_files; fl && fl->fl_file != file; fl=fl->fl_next );
   if( fl ) {
      fl->fl_file = NIL(FILE);
      fclose(file);
   }
}


/*
** Clean-up, and close all temporary files associated with a target.
*/
PUBLIC void
Unlink_temp_files( cp )/*
==========================
   Unlink the tempfiles if any exist.  Make sure you close the files first
   though.  This ensures that under DOS there is no disk space lost. */
CELLPTR cp;
{
   FILELISTPTR cur, next;

   if( cp == NIL(CELL) || cp->ce_files == NIL(FILELIST) ) return;

   for( cur=cp->ce_files; cur != NIL(FILELIST); cur=next ) {
      next = cur->fl_next;

      if( cur->fl_file ) fclose( cur->fl_file );

      if( Verbose & V_LEAVE_TMP )
         fprintf( stderr, "%s:  Left temp file [%s]\n", Pname, cur->fl_name );
      else
         (void) Remove_file( cur->fl_name );

      FREE(cur->fl_name);
      FREE(cur);
   }

   cp->ce_files = NIL(FILELIST);
}


PUBLIC void
Handle_result(status, ignore, abort_flg, target)/*
==================================================
  Handle return value of recipe.
*/
int status;
int ignore;
int abort_flg;
CELLPTR target;
{
   status = ((status&0xff)==0 ? status>>8   /* return from exit()      */
         : (status & 0xff)==SIGTERM ? -1 /* terminated from SIGTERM */
         : (status & 0x7f)+128);         /* terminated from signal
                          *         ( =status-128 ) */

   if( status ) {
      if( !abort_flg ) {
     char buf[512];

     sprintf(buf, "%s:  Error code %d, while making '%s'",
         Pname, status, target->ce_fname );

     if( ignore || Continue ) {
        if (!(Glob_attr & A_SILENT)) {
           strcat(buf, " (Ignored" );

           if ( Continue ) {
          /* Continue after error if '-k' was used. */
          strcat(buf,",Continuing");
          target->ce_attr |= A_ERROR;
           }
           strcat(buf,")");
           if (Verbose)
          fprintf(stderr, "%s\n", buf);
        }

        if( target->ce_attr & A_ERRREMOVE
        && Remove_file( target->ce_fname ) == 0
        && !(Glob_attr & A_SILENT))
           fprintf(stderr,"%s:  '%s' removed.\n", Pname, target->ce_fname);
     }
     else {
        fprintf(stderr, "%s\n",buf);

        if(!(target->ce_attr & A_PRECIOUS)||(target->ce_attr & A_ERRREMOVE))
           if( Remove_file( target->ce_fname ) == 0 )
          fprintf(stderr,"%s:  '%s' removed.\n", Pname,
              target->ce_fname);

        Quit(0);
     }
      }
      else if(!(target->ce_attr & A_PRECIOUS)||(target->ce_attr & A_ERRREMOVE))
     Remove_file( target->ce_fname );
   }
}


PUBLIC void
Update_time_stamp( cp )/*
=========================
   Update the time stamp of cp and scan the list of its prerequisites for
   files being marked as removable (ie. an inferred intermediate node).
   Remove them if there are any. */
CELLPTR cp;
{
   HASHPTR hp;
   LINKPTR dp;
   CELLPTR tcp;
   time_t  mintime;
   int     phony = ((cp->ce_attr&A_PHONY) != 0);

   for(dp=CeMeToo(cp); dp; dp=dp->cl_next) {
      tcp=dp->cl_prq;
      /* When calling Make() on this target ce_time was set to the minimal
       * required time the target should have after building, i.e. the time
       * stamp of the newest prerequisite or 1L if there is no
       * prerequisite. */
      mintime = tcp->ce_time;

      if( tcp->ce_attr & A_LIBRARY )
     Void_lib_cache( tcp->ce_fname, NIL(char) );
      else if( !Touch && (tcp->ce_attr & A_LIBRARYM) )
     Void_lib_cache( tcp->ce_lib, tcp->ce_fname );

      /* phony targets are treated as if they were recently made
       * and get the current time assigned. */
      if( phony ) {
     tcp->ce_time = Do_time();
      }
      else if (Trace) {
     tcp->ce_time = Do_time();
      }
      else {
     Stat_target(tcp, -1, TRUE);

     if( tcp->ce_time == (time_t) 0L ) {
        /* If the target does not exist after building set its
         * time stamp depending if it has recipes or not. Virtual
         * Targets (without recipes) get the newest time stamp of
         * its prerequisites assigned. (This was conveniently stored
         * in mintime.)
         * Targets with recipes are treated as if they were recently
         * made and get the current time assigned. */
        if( cp->ce_recipe == NIL(STRING) && mintime > 1 ) {
           tcp->ce_time = mintime;
        }
        else {
           tcp->ce_time = Do_time();
        }
     }
     else {
        /* The target exist. If the target does not have recipe
         * lines use the newest time stamp of either the target or
         * the newest time stamp of its prerequisites and issue
         * a warning. */
        if( cp->ce_recipe == NIL(STRING) ) {
           time_t  newtime = ( mintime > 1 ? mintime : Do_time() );

           if( !(tcp->ce_attr & A_SILENT) )
          Warning( "Found file corresponding to virtual target [%s].",
               tcp->CE_NAME );

           if( newtime > tcp->ce_time )
          tcp->ce_time = mintime;
        }
     }
      }

      if( Trace ) {
     tcp->ce_flag |= F_STAT;        /* pretend we stated ok */
      }

      if( Verbose & V_MAKE )
     printf( "%s:  <<<< Set [%s] time stamp to %lu\n",
         Pname, tcp->CE_NAME, tcp->ce_time );

      if( Measure & M_TARGET )
     Do_profile_output( "e", M_TARGET, tcp );

      /* At this point cp->ce_time is updated to either the actual file
       * time or the current time. */
      DB_PRINT( "make", ("time stamp: %ld, required mintime: %ld",
             cp->ce_time, mintime) );
      if( tcp->ce_time < mintime && !(tcp->ce_attr & A_SILENT) ) {
     Warning( "Target [%s] was made but the time stamp has not been updated.",
          tcp->CE_NAME );
      }

      /* The target was made, remove the temp files now. */
      Unlink_temp_files( tcp );
      tcp->ce_flag |= F_MADE;
      tcp->ce_attr |= A_UPDATED;
   }

   /* Scan the list of prerequisites and if we find one that is
    * marked as being removable, (ie. an inferred intermediate node)
    * then remove it.  We remove a prerequisite by running the recipe
    * associated with the special target .REMOVE.
    * Typically .REMOVE is defined in the startup file as:
    *  .REMOVE :; $(RM) $<
    * with $< being the list of prerequisites specified in the current
    * target. (Make() sets $< .) */

   /* Make sure we don't try to remove prerequisites for the .REMOVE
    * target. */
   if( strcmp(cp->CE_NAME,".REMOVE") != 0 &&
       (hp = Get_name(".REMOVE", Defs, FALSE)) != NIL(HASH) ) {
      register LINKPTR dp;
      int flag = FALSE;
      int rem;
      t_attr attr;

      tcp = hp->CP_OWNR;

      /* The .REMOVE target is re-used. Remove old prerequisites. */
      tcp->ce_flag |= F_TARGET;
      Clear_prerequisites( tcp );

      for(dp=cp->ce_prq; dp != NIL(LINK); dp=dp->cl_next) {
     register CELLPTR prq = dp->cl_prq;

     attr = Glob_attr | prq->ce_attr;
     /* We seem to have problems here that F_MULTI subtargets get removed
      * that even though they are still needed because the A_PRECIOUS
      * was not propagated correctly. Solution: Don't remove subtargets, the
      * master target will be removed if is not needed. */
     rem  = (prq->ce_flag & F_REMOVE) &&
        (prq->ce_flag & F_MADE  ) &&
            !(prq->ce_count ) && /* Don't remove F_MULTI subtargets. */
        !(prq->ce_attr & A_PHONY) &&
        !(attr & A_PRECIOUS);

     /* remove if rem is != 0 */
     if(rem) {
        LINKPTR tdp;

        /* Add the target plus all that are linked to it with the .UPDATEALL
         * attribute. */
        for(tdp=CeMeToo(prq); tdp; tdp=tdp->cl_next) {
           CELLPTR tmpcell=tdp->cl_prq;

           (Add_prerequisite(tcp,tmpcell,FALSE,FALSE))->cl_flag|=F_TARGET;
           tmpcell->ce_flag &= ~F_REMOVE;
        }
        flag = TRUE;
     }
      }

      if( flag ) {
     int sv_force = Force;

     Force = FALSE;
     Remove_prq( tcp );
     Force = sv_force;

     for(dp=tcp->ce_prq; dp != NIL(LINK); dp=dp->cl_next) {
        register CELLPTR prq = dp->cl_prq;

        prq->ce_flag &= ~(F_MADE|F_VISITED|F_STAT);
        prq->ce_flag |= F_REMOVE;
        prq->ce_time  = (time_t)0L;
     }
      }
   }
}


PUBLIC int
Remove_file( name )
char *name;
{
   struct stat buf;

   if( stat(name, &buf) != 0 )
      return 1;
   if( (buf.st_mode & S_IFMT) == S_IFDIR )
      return 1;
   return(unlink(name));
}


#if defined(__CYGWIN__)
char *
cygdospath(char *src, int winpath)/*
====================================
   Convert to DOS path if winpath is true. The returned pointer is
   either the original pointer or a pointer to a static buffer.
*/
{
   static char *buf = NIL(char);

   if ( !buf && ( (buf = MALLOC( PATH_MAX, char)) == NIL(char) ) )
      No_ram();

   DB_PRINT( "cygdospath", ("converting [%s] with winpath [%d]", src, winpath ) );

   /* Return immediately on NULL pointer or when .WINPATH is
    * not set. */
   if( !src || !winpath )
      return src;

   if( *src && src[0] == '/' ) {
      char *tmp;
      int err = cygwin_conv_to_win32_path(src, buf);
      if (err)
     Fatal( "error converting \"%s\" - %s\n",
        src, strerror (errno));

      tmp = buf;
      while ((tmp = strchr (tmp, '\\')) != NULL) {
     *tmp = '/';
     tmp++;
      }

      return buf;
   }
   else
      return src;
}
#endif
