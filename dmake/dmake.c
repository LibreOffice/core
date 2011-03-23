/* $RCSfile: dmake.c,v $
-- $Revision: 1.13 $
-- last change: $Author: kz $ $Date: 2008-03-05 18:28:04 $
--
-- SYNOPSIS
--      The main program.
--
-- DESCRIPTION
--
--  dmake [-#dbug_string] [ options ]
--          [ macro definitions ] [ target ... ]
--
--  This file contains the main command line parser for the
--  make utility.  The valid flags recognized are as follows:
--
--  -f file         - use file as the makefile
--  -C file     - duplicate console output to file (MSDOS only)
--  -K file     - .KEEP_STATE file
--  -#dbug_string   - dump out debugging info, see below
--  -v[cdfimrtw]    - verbose, print what we are doing, as we do it
--  -m[trae]    - measure timing information
--
--   options: (can be catenated, ie -irn == -i -r -n)
--
--  -A      - enable AUGMAKE special target mapping
--  -B      - enable non-use of TABS to start recipe lines
--  -c      - use non-standard comment scanning
--      -d              - do not use directory cache
--  -i              - ignore errors
--  -n              - trace and print, do not execute commands
--  -t              - touch, update dates without executing commands
--  -T              - do not apply transitive closure on inference rules
--  -r              - don't use internal rules
--  -s              - do your work silently
--  -S      - force Sequential make, overrides -P
--  -q              - check if target is up to date.  Does not
--            do anything.  Returns 0 if up to date, -1
--            otherwise.
--  -p              - print out a version of the makefile
--  -P#     - set value of MAXPROCESS
--  -E              - define environment strings as macros
--  -e              - as -E but done after parsing makefile
--  -u              - force unconditional update of target
--  -k              - make all independent targets even if errors
--  -V              - print out this make version number
--  -M      - Microsoft make compatibility, (* disabled *)
--  -h              - print out usage info
--  -x      - export macro defs to environment
--      -X              - ignore #! lines found in makefile
--
--  NOTE:  - #ddbug_string is only availabe for versions of dmake that
--      have been compiled with -DDBUG switch on.  Not the case for
--      distributed versions.  Any such versions must be linked
--      together with a version of Fred Fish's debug code.
--
--  NOTE:  - in order to compile the code the include file stddef.h
--      must be shipped with the bundled code.
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

/* Set this flag to one, and the global variables in vextern.h will not
 * be defined as 'extern', instead they will be defined as global vars
 * when this module is compiled. */
#define _DEFINE_GLOBALS_ 1

#include "extern.h" /* this includes config.h */
#include "sysintf.h"

#ifndef MSDOS
#define USAGE \
"Usage:\n%s [-P#] [-{f|K} file] [-{w|W} target ...] [macro[!][[*][+][:]]=value ...]\n"
#define USAGE2 \
"%s [-v[cdfimrtw]] [-m[trae]] [-ABcdeEghiknpqrsStTuVxX] [target ...]\n"
#else
#define USAGE \
"Usage:\n%s [-P#] [-{f|C|K} file] [-{w|W} target ...] [macro[!][[*][+][:]]=value ...]\n"
#define USAGE2 \
"%s [-v[cdfimrtw]] [-m[trae]] [-ABcdeEghiknpqrsStTuVxX] [target ...]\n"
#endif

/* We don't use va_end at all, so define it out so that it doesn't produce
 * lots of "Value not used" warnings. */
#ifdef va_end
#undef va_end
#endif
#define va_end(expand_to_null)

/* Make certain that ARG macro is correctly defined. */
#ifdef ARG
#undef ARG
#endif
#define ARG(a,b) a b

static char *sccid = "Copyright (c) 1990,...,1997 by WTI Corp.";
static char _warn  = TRUE;      /* warnings on by default */

static  void    _do_VPATH();
static  void    _do_ReadEnvironment();
#if !defined(__GNUC__) && !defined(__IBMC__)
static  void    _do_f_flag ANSI((char, char *, char **));
#else
static  void    _do_f_flag ANSI((int, char *, char **));
#endif

PUBLIC int
main(argc, argv)
int  argc;
char **argv;
{
#ifdef MSDOS
   char*   std_fil_name = NIL(char);
#endif

   char*   fil_name = NIL(char);
   char*   state_name = NIL(char);
   char*   whatif = NIL(char);
   char*   cmdmacs;
   char*   targets;
   STRINGPTR cltarget = NIL(STRING); /* list of targets from command line. */
   STRINGPTR cltarget_first = NIL(STRING); /* Pointer to first element. */
   FILE*   mkfil;
   int     ex_val;
   int     m_export;

   /* Uncomment the following line to pass commands to the DBUG engine
    * before the command line switches (-#..) are evaluated. */
   /*
   DB_PUSH("d,path");
    */
   DB_ENTER("main");

   /* Initialize Global variables to their default values       */
   Prolog(argc, argv);
   /* Set internal macros to their initial values, some are changed
    * later again by Make_rules() that parses the values from ruletab.c. */
   Create_macro_vars();
   Catch_signals(Quit);

   /* This macro is only defined for some OSs, see sysintf.c for details *
    * and NULL if undefined.                                             */
   Def_macro("ABSMAKECMD", AbsPname, M_PRECIOUS|M_NOEXPORT|M_EXPANDED );

   Def_macro( "MAKECMD", Pname, M_PRECIOUS|M_NOEXPORT|M_EXPANDED );
   Pname = Basename(Pname);

   DB_PROCESS(Pname);
   (void) setvbuf(stdout, NULL, _IOLBF, BUFSIZ); /* stdout line buffered */

   Continue  = FALSE;
   Comment   = FALSE;
   Get_env   = FALSE;
   Force     = FALSE;
   Target    = FALSE;
   If_expand = FALSE;
   Listing   = FALSE;
   Readenv   = FALSE;
   Rules     = TRUE;
   Trace     = FALSE;
   Touch     = FALSE;
   Check     = FALSE;
   Microsoft = FALSE;
   Makemkf   = FALSE;
   UseWinpath= FALSE;
   No_exec   = FALSE;
   m_export  = FALSE;
   cmdmacs   = NIL(char);
   targets   = NIL(char);
   Is_exec_shell = FALSE;
   Shell_exec_target = NIL(CELL);
   stdout_redir = NIL(FILE);

   /* Get fd for for @@-recipe silencing. */
   if( (zerofd = open(NULLDEV, O_WRONLY)) == -1 )
      Fatal( "Error opening %s !", NULLDEV );

   Verbose     = V_NOFLAG;
   Measure     = M_NOFLAG;
   Transitive  = TRUE;
   Nest_level  = 0;
   Line_number = 0;
   Suppress_temp_file = FALSE;
   Skip_to_eof = FALSE;

   while( --argc > 0 ) {
      register char *p;
      char *q;

      if( *(p = *++argv) == '-' ) {
         if( p[1] == '\0' ) Fatal("Missing option letter");

         /* copy options to Buffer for $(MFLAGS), strip 'f' and 'C'*/
         q = strchr(Buffer, '\0');
         while (*p != '\0') {
        char c = (*q++ = *p++);
            if( c == 'f' || c == 'C' ) q--;
     }

     if( *(q-1) == '-' )
        q--;
     else
            *q++ = ' ';

     *q = '\0';

         for( p = *argv+1; *p; p++) switch (*p) {
        case 'f':
           _do_f_flag( 'f', *++argv, &fil_name ); argc--;
           break;

#if defined(MSDOS) && !defined(OS2)
        case 'C':
           _do_f_flag( 'C', *++argv, &std_fil_name ); argc--;
           Hook_std_writes( std_fil_name );
           break;
#endif

        case 'K':
           _do_f_flag( 'K', *++argv, &state_name ); argc--;
           Def_macro(".KEEP_STATE", state_name, M_EXPANDED|M_PRECIOUS);
           break;

        case 'W':
        case 'w': {
           CELLPTR wif;
           _do_f_flag( 'w', *++argv, &whatif ); argc--;
           wif = Def_cell(whatif);
           wif->ce_attr |= A_WHATIF;
           whatif = NIL(char);

           if ( *p == 'W')
          break;
        }
        /*FALLTHRU*/

        case 'n':
           Trace = TRUE;
           break;

        case 'k': Continue   = TRUE;  break;
        case 'c': Comment    = TRUE;  break;
        case 'p': Listing    = TRUE;  break;
        case 'r': Rules      = FALSE; break;
        case 't': Touch      = TRUE;  break;
        case 'q': Check      = TRUE;  break;
        case 'u': Force      = TRUE;  break;
        case 'x': m_export   = TRUE;  break;
        case 'X': No_exec    = TRUE;  break;
        case 'T': Transitive = FALSE; break;
        case 'e': Get_env    = 'e';   break;
        case 'E': Get_env    = 'E';   break;

        case 'V': Version();  Quit(0);  break;
        case 'A': Def_macro("AUGMAKE", "y", M_EXPANDED); break;
        case 'B': Def_macro(".NOTABS", "y", M_EXPANDED); break;
        case 'i': Def_macro(".IGNORE", "y", M_EXPANDED); break;
        case 's': Def_macro(".SILENT", "y", M_EXPANDED); break;
        case 'S': Def_macro(".SEQUENTIAL", "y", M_EXPANDED); break;
        case 'g': Def_macro(".IGNOREGROUP","y", M_EXPANDED); break;
        case 'd': Def_macro(".DIRCACHE",NIL(char),M_EXPANDED); break;

        case 'v':
           if( p[-1] != '-' ) Usage(TRUE);
           while( p[1] ) switch( *++p ) {
          case 'c': Verbose |= V_DIR_CACHE; break;
          case 'd': Verbose |= V_DIR_SET;   break;
          case 'f': Verbose |= V_FILE_IO;   break;
          case 'i': Verbose |= V_INFER;     break;
          case 'm': Verbose |= V_MAKE;      break;
                  case 'r': Verbose |= V_FORCEECHO; break;
          case 't': Verbose |= V_LEAVE_TMP; break;
                  case 'w': Verbose |= V_WARNALL;   break;

          default: Usage(TRUE); break;
           }
           if( !Verbose ) Verbose = V_ALL;
               if( Verbose & V_FORCEECHO ) {
                 HASHPTR hp;
                 /* This cleans the .SILENT setting */
                 hp = Def_macro(".SILENT", "", M_EXPANDED);
                 /* This overrides the bitmask for further occurrences of
                  * .SILENT to "no bits allowed", see bit variables in the
                  * set_macro_value() definition in dag.c.
                  * The bitmask is already set by Create_macro_vars() in
                  * imacs.c and is overridden for the V_FORCEECHO case. */
                 hp->MV_MASK  = A_DEFAULT;
               }
           break;

        case 'm':
           if( p[-1] != '-' ) Usage(TRUE);
           while( p[1] ) switch( *++p ) {
          case 't': Measure |= M_TARGET;    break;
          case 'r': Measure |= M_RECIPE;    break;
          case 'a': Measure |= M_ABSPATH;   break;
          case 'e': Measure |= M_SHELLESC;  break;

          default: Usage(TRUE); break;
           }
           if( !Measure ) Measure = M_TARGET;
           break;

        case 'P':
           if( p[1] ) {
          /* Only set MAXPROCESS if -S flag is *not* used. */
          if( !(Glob_attr & A_SEQ) ) {
             Def_macro( "MAXPROCESS", p+1, M_MULTI|M_EXPANDED );
          }
          p += strlen(p)-1;
           }
           else
          Fatal( "Missing number for -P flag" );
           break;

#ifdef DBUG
        case '#':
           DB_PUSH(p+1);
           p += strlen(p)-1;
           break;
#endif

        case 'h': Usage(FALSE); break;
        case 0:   break;    /* lone - */
        default:  Usage(TRUE);  break;
     }
      }
      else if( (q = strchr(p, '=')) != NIL(char) ) {
     cmdmacs = DmStrAdd( cmdmacs, DmStrDup2(p), TRUE );
     /* Macros defined on the command line are marked precious.
      * FIXME: The exception for += appears to be bogus. */
     Parse_macro( p, (q[-1]!='+')?M_PRECIOUS:M_DEFAULT );
      }
      else {
     /* Remember the targets from the command line. */
     register STRINGPTR nsp;

     targets = DmStrAdd( targets, DmStrDup(p), TRUE );

     TALLOC(nsp, 1, STRING);
     nsp->st_string = DmStrDup( p );
     nsp->st_next = NIL(STRING);

     if(cltarget != NIL(STRING) )
        cltarget->st_next = nsp;
     else
        cltarget_first = nsp;

     cltarget = nsp;
      }
   }

   Def_macro( "MAKEMACROS",  cmdmacs, M_PRECIOUS|M_NOEXPORT );
   Def_macro( "MAKETARGETS", targets, M_PRECIOUS|M_NOEXPORT );
   if( cmdmacs != NIL(char) ) FREE(cmdmacs);
   if( targets != NIL(char) ) FREE(targets);

   Def_macro( "MFLAGS", Buffer, M_PRECIOUS|M_NOEXPORT );
   Def_macro( "%", "$@", M_PRECIOUS|M_NOEXPORT );

   if( *Buffer ) Def_macro( "MAKEFLAGS", Buffer+1, M_PRECIOUS|M_NOEXPORT );

   _warn  = FALSE;  /* disable warnings for builtin rules */
   Target = TRUE;   /* make sure we don't mark any of the default rules as
             * potential targets. */
   Make_rules();    /* Parse the strings stored in Rule_tab. */
   _warn = TRUE;

   /* If -r was not given find and parse startup-makefile. */
   if( Rules )
   {
     char *fname = NIL(char);

     /* Search_file() also checks the environment variable. */
     if( (mkfil=Search_file("MAKESTARTUP", &fname)) != NIL(FILE) )
     {
       Parse(mkfil);
       Def_macro( "MAKESTARTUP", fname, M_EXPANDED|M_MULTI|M_FORCE );
     }
     else
       Fatal( "Configuration file `%s' not found", fname );
     if ( fname != NIL(char)) { FREE( fname ); fname = NIL(char); }
   }

   /* Define the targets set on the command line now. */
   Target = FALSE;  /* Will be set to TRUE when the default targets are set. */
   for( cltarget = cltarget_first; cltarget != NIL(STRING); ) {
      CELLPTR cp;
      STRINGPTR nta = cltarget->st_next;

      Add_prerequisite(Targets, cp = Def_cell(cltarget->st_string),
               FALSE, FALSE);
      cp->ce_flag |= F_TARGET;
      cp->ce_attr |= A_FRINGE;
      Target = TRUE;

      FREE(cltarget->st_string);
      FREE(cltarget);
      cltarget = nta;
   }

   if( Get_env == 'E' ) _do_ReadEnvironment();

   /* Search for and parse user makefile. */
   if( fil_name != NIL(char) )
      mkfil = Openfile( fil_name, FALSE, TRUE );
   else {
      /* Search .MAKEFILES dependent list looking for a makefile.
       */
      register CELLPTR cp;

      cp = Def_cell( ".MAKEFILES" );
      mkfil = TryFiles(cp->CE_PRQ);
   }

   if( mkfil != NIL(FILE) ) {
      char *f = Filename();
      char *p;

      if( strcmp(f, "stdin") == 0 ) f = "-";
      Def_macro( "MAKEFILE", p = DmStrAdd( "-f", f, FALSE ), M_PRECIOUS|M_NOEXPORT ); FREE(p);
      Parse( mkfil );
   }
   else if( !Rules )
      Fatal( "No `makefile' present" );

   if( Nest_level     ) Fatal( "Missing .END for .IF" );
   if( Get_env == 'e' ) _do_ReadEnvironment();

   _do_VPATH();                 /* kludge it up with .SOURCE    */

   if( Listing ) Dump();        /* print out the structures     */
   if( Trace ) Glob_attr &= ~A_SILENT;  /* make sure we see the trace   */

   if( !Target )
      Fatal( "No target" );
   else {
      Test_circle( Root, TRUE );
      Check_circle_dfa();
   }

   if( m_export ) {
      int i;

      for( i=0; i<HASH_TABLE_SIZE; ++i ) {
     HASHPTR hp = Macs[i];

     while( hp ) {
        if( !(hp->ht_flag & M_NOEXPORT) && hp->ht_value != NIL(char) )
           if( Write_env_string(hp->ht_name, hp->ht_value) != 0 )
           Warning( "Could not export %s", hp->ht_name );
        hp = hp->ht_next;
     }
      }
   }

   if( Buffer != NIL(char) ) {FREE( Buffer ); Buffer = NIL(char);}
   if( Trace ) Def_macro(".SEQUENTIAL", "y", M_EXPANDED);

   ex_val = Make_targets();

   Clear_signals();

   /* Close fd for for @@-recipe silencing. */
   if( close(zerofd) )
      Fatal( "Error closing %s !", NULLDEV );
   Epilog(ex_val);      /* Does not return -- EVER */
   return 0;
}


static void
_do_f_flag( flag, name, fname )
char   flag;
char  *name;
char **fname;
{
   if( *fname == NIL(char) ) {
      if( name != NIL(char) ) {
     *fname = name;
      } else
     Fatal("No file name for -%c", flag);
   } else
      Fatal("Only one `-%c file' allowed", flag);
}


static void
_do_ReadEnvironment()
{
   t_attr saveattr = Glob_attr;

   Glob_attr |= A_SILENT;
   ReadEnvironment();
   Glob_attr = saveattr;
}


static void
_do_VPATH()
{
   HASHPTR hp;
   char    *_rl[2];
   extern char **Rule_tab;

   hp = GET_MACRO("VPATH");
   if( hp == NIL(HASH) ) return;

   _rl[0] = ".SOURCE :^ $(VPATH:s/:/ /)";
   _rl[1] = NIL(char);

   Rule_tab = _rl;
   Parse( NIL(FILE) );
}


/*  The file table and pointer to the next FREE slot for use by both
    Openfile and Closefile.  Each open stacks the new file onto the open
    file stack, and a corresponding close will close the passed file, and
    return the next file on the stack.  The maximum number of nested
    include files is limited by the value of MAX_INC_DEPTH */

static struct {
   FILE         *file;      /* file pointer                 */
   char         *name;      /* name of file                 */
   int          numb;       /* line number                  */
} ftab[ MAX_INC_DEPTH ];

static int next_file_slot = 0;

/* Set the proper macro value to reflect the depth of the .INCLUDE directives
 * and the name of the file we are reading.
 */
static void
_set_inc_depth()
{
   char buf[10];
   sprintf( buf, "%d", next_file_slot );
   Def_macro( "INCDEPTH", buf, M_MULTI|M_NOEXPORT );
   Def_macro( "INCFILENAME",
              next_file_slot ? ftab[next_file_slot-1].name : "",
              M_MULTI|M_NOEXPORT|M_EXPANDED );
}


PUBLIC FILE *
Openfile(name, mode, err)/*
===========================
   This routine opens a file for input or output depending on mode.
   If the file name is `-' then it returns standard input.
   The file is pushed onto the open file stack.  */
char *name;
int  mode;
int  err;
{
   FILE *fil;

   DB_ENTER("Openfile");

   if( name == NIL(char) || !*name ) {
      if( !err )
         DB_RETURN(NIL(FILE));
      else
         Fatal( "Openfile:  NIL filename" );
   }

   if( next_file_slot == MAX_INC_DEPTH )
      Fatal( "Too many open files. Max nesting level is %d.", MAX_INC_DEPTH);

   DB_PRINT( "io", ("Opening file [%s], in slot %d", name, next_file_slot) );

   if( strcmp("-", name) == 0 ) {
      name = "stdin";
      fil = stdin;
   }
   else
      fil = fopen( name, mode ? "w":"r" );

   if( Verbose & V_FILE_IO )
      printf( "%s:  Openning [%s] for %s", Pname, name, mode?"write":"read" );

   if( fil == NIL(FILE) ) {
      if( Verbose & V_FILE_IO ) printf( " (fail)\n" );
      if( err )
         Fatal( mode ? "Cannot open file %s for write" : "File %s not found",
        name );
   }
   else {
      if( Verbose & V_FILE_IO ) printf( " (success)\n" );
      ftab[next_file_slot].file   = fil;
      ftab[next_file_slot].numb   = Line_number;
      ftab[next_file_slot++].name = DmStrDup(name);
      Line_number = 0;
      _set_inc_depth();
   }

   DB_RETURN(fil);
}


PUBLIC FILE *
Closefile()/*
=============
   This routine is used to close the last file opened.  This forces make
   to open files in a last open first close fashion.  It returns the
   file pointer to the next file on the stack, and NULL if the stack is empty.*/
{
   DB_ENTER("Closefile");

   if( !next_file_slot )
      DB_RETURN( NIL(FILE) );

   if( ftab[--next_file_slot].file != stdin ) {
      DB_PRINT( "io", ("Closing file in slot %d", next_file_slot) );

      if( Verbose & V_FILE_IO )
     printf( "%s:  Closing [%s]\n", Pname, ftab[next_file_slot].name );

      fclose( ftab[next_file_slot].file );
      FREE( ftab[next_file_slot].name );
   }

   _set_inc_depth();

   if( next_file_slot > 0 ) {
      Line_number = ftab[next_file_slot].numb;
      DB_RETURN( ftab[next_file_slot-1].file );
   }
   else
      Line_number = 0;

   DB_RETURN( NIL(FILE) );
}


PUBLIC FILE *
Search_file( macname, rname )
char *macname;
char **rname;
{
   HASHPTR hp;
   FILE *fil = NIL(FILE);
   char *fname = NIL(char);
   char *ename = NIL(char);

   /* order of precedence is:
    *
    *   MACNAME  from command line (precious is marked)
    *        ... via MACNAME:=filename definition.
    *   MACNAME  from environment
    *   MACNAME  from builtin rules (not precious)
    */

   if( (hp = GET_MACRO(macname)) != NIL(HASH) ) {
     /* Only expand if needed. */
     if( hp->ht_flag & M_EXPANDED ) {
       ename = fname = DmStrDup(hp->ht_value);
     } else {
       ename = fname = Expand(hp->ht_value);
     }

     if( hp->ht_flag & M_PRECIOUS ) fil = Openfile(fname, FALSE, FALSE);
   }

   if( fil == NIL(FILE) ) {
     fname=Expand(Read_env_string(macname));
     if( (fil = Openfile(fname, FALSE, FALSE)) != NIL(FILE) ) FREE(ename);
   }

   if( fil == NIL(FILE) && hp != NIL(HASH) ) {
     if ( fname != NIL(char) ) { FREE(fname); fname = NIL(char); }
     fil = Openfile(fname=ename, FALSE, FALSE);
   }

   if( rname ) *rname = fname;

   return(fil);
}


PUBLIC char *
Filename()/*
============
   Return name of file on top of stack */
{
   return( next_file_slot==0 ? NIL(char) : ftab[next_file_slot-1].name );
}


PUBLIC int
Nestlevel()/*
=============
   Return the file nesting level */
{
   return( next_file_slot );
}


PUBLIC FILE *
TryFiles(lp)/*
==============
   Try to open a makefile, try to make it if needed and return a
   filepointer to the first successful found or generated file.
   The function returns NIL(FILE) if nothing was found. */
LINKPTR lp;
{
   FILE *mkfil = NIL(FILE);

   if( lp != NIL(LINK) ) {
      int s_n, s_t, s_q;

      s_n = Trace;
      s_t = Touch;
      s_q = Check;

      Trace = Touch = Check = FALSE;
      /* We are making a makefile. Wait for it. */
      Makemkf = Wait_for_completion = TRUE;
      mkfil = NIL(FILE);

      for(;  lp != NIL(LINK) && mkfil == NIL(FILE); lp=lp->cl_next) {
     if( lp->cl_prq->ce_attr & A_FRINGE ) continue;

     mkfil = Openfile( lp->cl_prq->CE_NAME, FALSE, FALSE );

     /* Note that no error handling for failed Make() calls is possible
      * as expected errors (no rule to make the makefile) or unexpected
      * errors both return -1. */
     if( mkfil == NIL(FILE) && Make(lp->cl_prq, NIL(CELL)) != -1 ) {
        mkfil = Openfile( lp->cl_prq->CE_NAME, FALSE, FALSE );
        /* Remove flags that indicate that the target was already made.
         * This is also needed to avoid conflicts with the circular
         * dependency check in rulparse(), see issues 62118 and 81296
         * for details. */
        Unmake(lp->cl_prq);
     }
      }

      Trace = s_n;
      Touch = s_t;
      Check = s_q;
      Makemkf = Wait_for_completion = FALSE;
   }

   return(mkfil);
}


/*
** print error message from variable arg list
*/

static int errflg = TRUE;
static int warnflg = FALSE;

static void
errargs(fmt, args)
char    *fmt;
va_list  args;
{
   int warn = _warn && warnflg && !(Glob_attr & A_SILENT);

   if( errflg || warn ) {
      char *f = Filename();

      fprintf( stderr, "%s:  ", Pname );
      if( f != NIL(char) ) fprintf(stderr, "%s:  line %d:  ", f, Line_number);

      if( errflg )
         fprintf(stderr, "Error: -- ");
      else if( warn )
         fprintf(stderr, "Warning: -- ");

      vfprintf( stderr, fmt, args );
      putc( '\n', stderr );
      if( errflg && !Continue ) Quit(0);
   }
}


/*
** Print error message and abort
*/
PUBLIC void
Fatal(ARG(char *,fmt), ARG(va_alist_type,va_alist))
DARG(char *,fmt)
DARG(va_alist_type,va_alist)
{
   va_list args;

   va_start(args, fmt);
   Continue = FALSE;
   errargs(fmt, args);
   va_end(args);
}

/*
** error message and exit (unless -k)
*/
PUBLIC void
Error(ARG(char *,fmt), ARG(va_alist_type,va_alist))
DARG(char *,fmt)
DARG(va_alist_type,va_alist)
{
   va_list args;

   va_start(args, fmt);
   errargs(fmt, args);
   va_end(args);
}


/*
** non-fatal message
*/
PUBLIC void
Warning(ARG(char *,fmt), ARG(va_alist_type,va_alist))
DARG(char *,fmt)
DARG(va_alist_type,va_alist)
{
   va_list args;

   va_start(args, fmt);
   warnflg = TRUE;
   errflg = FALSE;
   errargs(fmt, args);
   errflg = TRUE;
   warnflg = FALSE;
   va_end(args);
}


PUBLIC void
No_ram()
{
   Fatal( "No more memory" );
}


PUBLIC void
Usage( eflag )
int eflag;
{
   register char *p;
   char *fill;

   fill = DmStrDup(Pname);
   for(p=fill; *p; p++) *p=' ';

   if( eflag ) {
      fprintf(stderr, USAGE, Pname);
      fprintf(stderr, USAGE2, fill);
   }
   else {
   printf(USAGE, Pname);
   printf(USAGE2, fill);
   puts("    -P#        - set max number of child processes for parallel make");
   puts("    -f file    - use file as the makefile");
#ifdef MSDOS
   puts("    -C [+]file - duplicate console output to file, ('+' => append)");
#endif
   puts("    -K file    - use file as the .KEEP_STATE file");
   puts("    -w target  - show what you would do if 'target' were out of date");
   puts("    -W target  - rebuild pretending that 'target' is out of date");
   puts("    -v[cdfimrtw] - verbose, indicate what we are doing, (-v => -vcdfimrtw)");
   puts("                   c => dump directory cache info only" );
   puts("                   d => dump change of directory info only" );
   puts("                   f => dump file open/close info only" );
   puts("                   i => dump inference information only" );
   puts("                   m => dump make of target information only" );
   puts("                   r => Force output of recipe lines and warnings," );
   puts("                        overrides -s" );
   puts("                   t => keep temporary files when done" );
   puts("                   w => issue non-essential warnings\n" );

   puts("    -m[trae] - Measure timing information, (-m => -mt)");
   puts("               t => display the start and end time of each target" );
   puts("               r => display the start and end time of each recipe" );
   puts("               a => display the target as an absolute path" );
   puts("               e => display the timing of shell escape macros\n" );

   puts("Options: (can be catenated, ie -irn == -i -r -n)");
   puts("    -A   - enable AUGMAKE special target mapping");
   puts("    -B   - enable the use of spaces instead of tabs to start recipes");
   puts("    -c   - use non standard comment scanning");
   puts("    -d   - do not use directory cache");
   puts("    -E   - define environment strings as macros");
   puts("    -e   - same as -E but done after parsing makefile");
   puts("    -g   - disable the special meaning of [ ... ] for group recipes");
   puts("    -h   - print out usage info");
   puts("    -i   - ignore errors");
   puts("    -k   - make independent targets, even if errors");
   puts("    -n   - trace and print, do not execute commands");
   puts("    -p   - print out a version of the makefile");
   puts("    -q   - check if target is up to date.  Does not do");
   puts("           anything.  Returns 0 if up to date, 1 otherwise");
   puts("    -r   - don't use internal rules");
   puts("    -s   - do your work silently");
   puts("    -S   - disable parallel (force sequential) make, overrides -P");
   puts("    -t   - touch, update time stamps without executing commands");
   puts("    -T   - do not apply transitive closure on inference rules");
   puts("    -u   - force unconditional update of target");
   puts("    -V   - print out version number");
   puts("    -x   - export macro values to environment");
   puts("    -X   - ignore #! lines at start of makefile");
   }

   Quit(0);
}


PUBLIC void
Version()
{
   extern char **Rule_tab;
   char **p;

   printf("%s - Version %s (%s)\n", Pname, VERSION, BUILDINFO);
   printf("%s\n\n", sccid);

   puts("Default Configuration:");
   for (p=Rule_tab;  *p != NIL(char);  p++)
      printf("\t%s\n", *p);

   printf("\n");

#if defined(HAVE_SPAWN_H) || defined(__CYGWIN__)
   /* Only systems that have spawn ar concerned whether spawn or fork/exec
    * are used. */
#if ENABLE_SPAWN
      printf("Subprocesses are executed using: spawn.\n\n");
#else
      printf("Subprocesses are executed using: fork/exec.\n\n");
#endif
#endif

      printf("Please read the NEWS file for the latest release notes.\n");
}
