/* RCS  $Id: spawn.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Spawnvpe code to emulate spawnvpe call common to DOS compilers.
--
-- DESCRIPTION
--  This implementation is further integrated into dmake in that it
--  determines the program to execute and if it's extension is either
--  .bat or .ksh it executes it using the appropriate shell based on the
--  setting of .MKSARGS.  If .MKSARGS is set then in addition
--  to the command tail getting built the arguments are also passed in the
--  environment pursuant to the published MKS argument passing conventions.
--  If the variable Swap_on_exec is set and the DOS OS supports it
--  then the dmake executable image is swapped to secondary storage prior
--  to running the child process.  This is requested by setting the
--  appropriate flag in the call to exec.
--
--  This and the exec.asm routine are derived from work that was supplied
--  to me by Kent Williams (williams@umaxc.weeg.uiowa.edu) and by
--      Len Reed, (..!gatech!holos0!lbr or holos0!lbr@gatech.edu., Holos
--  Software, Inc., Tucker, Ga.).  I sincerely acknowledge their help since
--  their Turbo C, and MSC 6.0 code lead directly to this combined
--  swapping exec that hopefully works with either compiler in all memory
--  models.
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

#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER) && _MSC_VER >= 600
    /* Ignore the MSC 6.0 library's "const"-riddled prototype
       for spawnvpe.
    */
# define spawnvpe _ignore_msc_spawnvpe
# include <process.h>
# undef spawnvpe
  int spawnvpe(int, char *, char **, char **);
#else
# include <process.h>
#endif

#include <dos.h>
#include <errno.h>
#include <string.h>
#include <alloc.h>
#include <fcntl.h>
#include "extern.h"
#include "dosdta.h"
#include "exec.h"
#include "sysintf.h"

extern int Interrupted;

/* variables and functions local to this file */
static char     *_findexec ANSI((char *, int *));
static char    **_getpath ANSI(());
static char far *_dos_alloc ANSI((uint16));

static uint16 _swap_mask;
static int    _mks_args;
static char   dot_com[] = ".COM",
          dot_exe[] = ".EXE",
              dot_bat[] = ".BAT",
          dot_ksh[] = ".KSH";

/* Kinds of executables */
#define SCR 1
#define COM 2
#define EXE 4
#define ALL (SCR|COM|EXE)

/* How to make a long pointer */
#define CF(x) (char far *)x

/* Make sure we know how to get a segment out of a long pointer */
#ifndef FP_SEG
#define FP_SEG(fp)  ((unsigned)((unsigned long)(fp) >> 16))
#endif


PUBLIC int
spawnvpe(mode, program, av, ep)/*
=================================
   Spawn a process using an environment and a vector of arguments.
   The code computes a new environment, puts the MKS arguments into
   it if need be, and calls the appropriate routines to search the
   path and to invoke the process. */
int  mode;
char *program;
char **av;
char **ep;
{
   char pwd[PATH_MAX+1];
   char **envp = ep;        /* Cause we are going to mess with it. */
   char **argv = av;        /* Same with this one.             */
   char cmdtail[129];
   char far *environment;
   char *tail;
   char *swptmp;
   unsigned int envsize;
   unsigned int cmdsize;
   int  cmdtailen;
   int  i;
   int  doswap;

   /* First check to see if we can find the program to execute this way we
    * don't alloc the environment and other such stuff prior to figuring out
    * we don't know how to run the program. */
find_program:
   if((program = _findexec(program, &i)) == NIL(char)) {
      errno = ENOENT;
      return( -1 );
   }

   /* i is set to TRUE in _findexec if the exec is a shell
    * script (either .BAT or .KSH file), returns FALSE for all others. */
   if( i && !Packed_shell ) {
      /* Restore the spaces into the command line that were erased by
       * the previous call to Pack_argv.  This enables us to repack the
       * command as a shell command using Pack_argv again. */
      for( i=0; argv[i] != NIL(char); i++ ) {
         int x = strlen(argv[i]);
         if( argv[i+1] != NIL(char) ) argv[i][x] = ' ';
      }

      argv = Pack_argv( FALSE, TRUE, *argv );

      /* Go and find the program again, I hate goto's but it seems silly to
       * use tail recursion here just for aesthetic purity. */
      program = *argv;
      goto find_program;
   }

   /* Compute size of *argv vector for passing as MKS style arguments */
   cmdsize = strlen(*argv)+2;

   /* So we have decided on a program to run, therefore pack the command tail
    * and build the environment to pass to the exec code.  This loop packs the
    * DOS command tail, and computes the size of all arguments for the MKS
    * argument passing convention.  Note that we reserve one less byte in the
    * command tail if we are not using MKS style argument passing.
    *
    * Make sure the command tail contains at leat a space.  Some commands fail
    * to work if the command tail is only a \r, STUPID DOS! */
   cmdtailen = ((_mks_args = ((Glob_attr & A_MKSARGS) != 0)) != 0)?3:2;
   tail      = cmdtail+1;

   if( argv[1] != NIL(char) )
      for( i=1; argv[i] != NIL(char); i++ ) {
     int arglen = strlen(argv[i]);

     cmdsize += arglen+2;       /* Compute all args size for MKS */

     if( (cmdtailen += arglen+1) <= 128 ) {
        register char *p = argv[i];
        tail[-1] = ' ';     /* put in the space */
        while( *tail++ = *p++ );    /* put in the arg   */
     }
     else if( !_mks_args ) {
        errno = E2BIG;      /* unless its MKS exit if arglist */
        return(-1);         /* is too long.           */
     }
      }
   else
      *tail++ = ' ';

   /* Finish the command tail set up, placing the length in the first byte,
    * and the \r \n \0 at the end for DOS, MKS and us respectively. */
   *cmdtail = tail-cmdtail-2;
   tail[-1] = '\r';
   if( _mks_args ) *tail++ = '\n';
   *tail = '\0';

   /* Compute size of environment, skipping any MKS arguments passed in our
    * environment */
   for(; *envp && **envp == '~'; envp++ );
   for(i=0, envsize=_mks_args?cmdsize:1; envp[i] != NIL(char); i++ )
      envsize += strlen(envp[i]) + 1;

   /* Check the DOS version number here.  If it is < 3.0 then we don't
    * even want to think about executing the swapping code.   Permanently
    * set swap to 0. */
   doswap = (_osmajor < 3) ? 0 : Swap_on_exec;

   /* Set up temporary file for swapping */
   swptmp = doswap?tempnam(NIL(char),"mk"):"";

   /* Allocate an appropriate sized environment block and align it on a
    * paragraph boundary.  It will later get copied to an appropriately low
    * place in the executable image so that when we swap out the environment
    * is still present.  Use
    *    _dos_alloc
    * to allocate the environment segment.  The segment is freed by the call
    * to exec. */
   environment = _dos_alloc( envsize = ((envsize+16)>>4) );
   *environment = '\0';

   /* First copy the arguments preceeded by ~ character if we are using
    * MKS style argument passing */
   if( _mks_args )
      for(; *argv; argv++) {
         register char *p = *argv;

     *environment++ = '~';
     while( *environment++ = *p++ );    /* Far dest, poss near ptr */
      }

   /* Now stick in the current evironment vectors. */
   for(; *envp; envp++) {
      register char *p = *envp;
      while( *environment++ = *p++ );       /* Far dest, poss near ptr */
   }

   /* Clear the interrupted flag, and exec  */
   Interrupted = 0;

   /* Preserve the current working directory accross a spawn call
    * DOS is brain dead about this.  This way we have some hope of cleaning
    * up the swapping tempfiles after we return. */
   strcpy(pwd,Get_current_dir());
   i = exec(doswap,CF(program),CF(cmdtail),FP_SEG(environment),CF(swptmp));
   Set_dir(pwd);

   /* Now free the temporary file name */
   if( doswap ) FREE(swptmp);

   /* If swap was interrupted then quit properly from dmake. */
   if( Interrupted ) Quit();

   return(i);
}


PUBLIC void
Hook_std_writes( file )
char *file;
{
   if( file!= NIL(char) ) {
      int mode = O_BINARY | O_WRONLY | O_CREAT | O_TRUNC;
      int handle;

      if (*file == '+') {
          ++file;             /* -F +file means append to file */
          mode = O_BINARY | O_WRONLY | O_CREAT | O_APPEND;
      }
      handle = open(file, mode, S_IREAD | S_IWRITE);
      if (handle < 0) {
          Fatal( "Could not open -F file");
      }
      (void) lseek(handle, 0L, SEEK_END);
      do_hook_std_writes(handle);
   }
   else
      do_unhook_std_writes();
}


/*
** _findexec finds executables on the path.
** Note that it is pretty simple to add support for other executable types
** shell scripts, etc.
**
** This follows the command.com behavior very closely.
*/
static char *
_findexec( s, is_shell )/*
==========================
   Cloned closely from code provided by Kent Williams.  Stripped his down to
   a reduced search since dmake doesn't need to recompute the PATH vector
   each time it does the search since it cannot alter the path vector once
   it begins to make recipes.  Also modified it to use findfirst and findnext
   as provided for dirlib package that I got off the net. */
char *s;
int  *is_shell;
{
   unsigned found_flags;
   char     **pathv = NIL(char *);
   char     *ext    = NIL(char);
   char     *buf    = NIL(char);
   char     *p[2];
   char     *dot_scr;
   char     *dot;

   p[0] = ""; p[1] = NIL(char);
   if( strchr("./\\", *s) || s[1] == ':' )
      pathv = p;
   else if( (pathv = _getpath()) == NIL(char *) )
      return( NIL(char) );

   /* Compute the extension we need if any. */
   if( (dot = strrchr(s,'.')) != NIL(char) &&
        dot > strrchr(s,'/') && dot > strrchr(s,'\\') )
      ext = dot+1;

   dot_scr   = _mks_args ? dot_ksh : dot_bat;
   *is_shell = FALSE;

   for( found_flags = 0; *pathv && !found_flags; pathv++ ) {
      DTA dta;

      if( !ext ) {
     char *name;
     buf = Build_path( *pathv, name=DmStrJoin(s, ".???", -1, FALSE) );
     FREE(name);
      }
      else
     buf = Build_path( *pathv, s );

      if( findfirst((char *)strupr(buf), &dta) != NIL(DTA) ) {
     if( !ext ) {
        char *dot;

        /* search order is .com .exe (.ksh || .bat)
         * there has to be a '.' */
        do {
           dot = strrchr(dta.name,'.');
           if(0 == strcmp(dot,dot_com))
          found_flags |= COM;
           else if(0 == strcmp(dot,dot_exe))
          found_flags |= EXE;
           else if( 0 == strcmp(dot,dot_scr) )
          found_flags |= SCR;
        } while( found_flags != ALL && findnext(&dta) != NIL(DTA) );

        if(found_flags & COM)      ext = dot_com;
        else if(found_flags & EXE) ext = dot_exe;
        else if(found_flags & SCR) {
           ext = dot_scr;
           *is_shell = TRUE;
        }

        if( found_flags ) {
           char *name;
           buf = Build_path( *pathv, name=DmStrJoin(s,ext,-1,FALSE) );
           FREE(name);
           strupr(buf);
        }
     }
     else
        found_flags++;
      }
   }

   return( found_flags ? buf : NIL(char) );
}


/*
** getpath turns the DOS path into a char *vector, It is gotten and
** transformed only once since dmake can't modify the value of PATH while
** it is making targets.
*/
static char **
_getpath()
{
   static   char **dir = NIL(char *);
   register char *p;

   if( !dir ) {
      register char *t;
      int           i;
      char          *semi = NIL(char);

      if( (p = getenv("PATH")) == NIL(char) ) p = "";
      for( i=1, t=p; *t; t++ ) if( *t == ';' ) i++;

      TALLOC(dir, i+1, char *);
      p   = DmStrDup(p);

      for( i=0; p; p = semi ? (semi+1):NIL(char),i++ ){
     if( (semi = strchr(p,';')) != NIL(char) ) *semi = '\0';
     dir[i] = p;
      }
      dir[i]=NIL(char);
   }

   return( dir );
}


static char far *
_dos_alloc( size )/*
====================
   This routine allocates size paragraphs from DOS.  It changes the memory
   allocation strategy to allocate from the tail and then changes it back.
   to using first fit. */
uint16 size;
{
   union REGS r;

   r.h.ah = 0x48;
   r.x.bx = size;

   intdos( &r, &r );
   if( r.x.cflag ) No_ram();

   return( (char far *) MK_FP(r.x.ax, 0) );
}
