/* RCS  $Id: imacs.c,v 1.9 2008-03-05 18:29:01 kz Exp $
--
-- SYNOPSIS
--      Define default internal macros.
--
-- DESCRIPTION
--  This file adds to the internal macro tables the set of default
--  internal macros, and for those that are accessible internally via
--  variables creates these variables, and initializes them to point
--  at the default values of these macros.
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

static  void    _set_int_var ANSI((char *, char *, int, int *));
static  void    _set_string_var ANSI((char *, char *, int, char **));
static  void    _set_bit_var ANSI((char *, char *, int));


PUBLIC void
Make_rules()/*
==============
   Parse the strings stored in Rule_tab (from ruletab.c). */
{
   Parse(NIL(FILE));
}


#define M_FLAG   M_DEFAULT | M_EXPANDED

/*
** Add to the macro table all of the internal macro variables plus
** create secondary variables which will give access to their values
** easily, both when needed and when the macro value is modified.
** The latter is accomplished by providing a flag in the macro and a field
** which gives a pointer to the value if it is a char or string macro value
** and a mask representing the bit of the global flag register that is affected
** by this macro's value.
*/
PUBLIC void
Create_macro_vars()
{
   static char* switchar;
   static char* version;
   char   swchar[2];
   char   buf[20];

   swchar[0] = Get_switch_char(), swchar[1] = '\0';
   _set_string_var("SWITCHAR", swchar, M_PRECIOUS, &switchar);
   if (*swchar == '/')
      DirSepStr = "\\";
   else
#if (_MPW)
         DirSepStr = ":";
#elif defined( __EMX__)
   /* Use '\' for OS/2 port. */
         DirSepStr = "\\";
#else
         DirSepStr = "/";
#endif
   _set_string_var("DIRSEPSTR", DirSepStr, M_DEFAULT,&DirSepStr);
   _set_string_var("DIRBRKSTR", DirBrkStr, M_DEFAULT, &DirBrkStr);
   swchar[0] = DEF_ESCAPE_CHAR, swchar[1] = '\0';
   _set_string_var(".ESCAPE_PREFIX", swchar, M_FLAG, &Escape_char);

   /* Each one the following attributes corresponds to a bit of
    * Glob_attr. */
   _set_bit_var(".SILENT",   "", A_SILENT  );
   _set_bit_var(".IGNORE",   "", A_IGNORE  );
   _set_bit_var(".PRECIOUS", "", A_PRECIOUS);
   _set_bit_var(".EPILOG",   "", A_EPILOG  );
   _set_bit_var(".PROLOG",   "", A_PROLOG  );
   _set_bit_var(".NOINFER",  "", A_NOINFER );
   _set_bit_var(".SEQUENTIAL","",A_SEQ     );
   _set_bit_var(".USESHELL", "", A_SHELL   );
   /* .SWAP (MSDOS) and .WINPATH (cygwin) share the same bit. */
   _set_bit_var(".SWAP",     "", A_SWAP    );
   _set_bit_var(".WINPATH",  "", A_WINPATH );
   _set_bit_var(".MKSARGS",  "", A_MKSARGS );
   _set_bit_var(".IGNOREGROUP","",A_IGNOREGROUP);

   Glob_attr = A_DEFAULT;        /* set all flags to NULL   */

   _set_string_var("SHELL",        "",  M_DEFAULT, &Shell       );
   _set_string_var("SHELLFLAGS",   " ", M_DEFAULT, &Shell_flags );
   _set_string_var("SHELLCMDQUOTE","",  M_DEFAULT, &Shell_quote );
   _set_string_var("GROUPSHELL",   "",  M_DEFAULT, &GShell      );
   _set_string_var("GROUPFLAGS",   " ", M_DEFAULT, &GShell_flags);
   _set_string_var("SHELLMETAS",   "",  M_DEFAULT, &Shell_metas );
   _set_string_var("GROUPSUFFIX",  "",  M_DEFAULT, &Grp_suff    );
   _set_string_var("AUGMAKE",NIL(char), M_DEFAULT, &Augmake     );
   _set_string_var("OOODMAKEMODE", "",  M_DEFAULT, &OOoDmMode );
   _set_string_var(".KEEP_STATE",  "",  M_DEFAULT, &Keep_state  );
   _set_string_var(".NOTABS",      "",  M_MULTI, &Notabs );
   _set_string_var(".DIRCACHE",    "y", M_DEFAULT, &UseDirCache );

#if CASE_INSENSITIVE_FS
#define DIRCACHERESPCASEDEFAULT ""
#else
#define DIRCACHERESPCASEDEFAULT "y"
#endif
   _set_string_var(".DIRCACHERESPCASE", DIRCACHERESPCASEDEFAULT, M_DEFAULT, &DcacheRespCase);

   _set_string_var("MAKEDIR",Get_current_dir(),M_PRECIOUS|M_NOEXPORT,
           &Makedir_macval);
   Makedir = DmStrDup(Makedir_macval); /* Later done by Def_macro(). */
   _set_string_var("MAKEVERSION", VERSION, M_PRECIOUS, &version);
   _set_string_var("PWD",  Makedir,  M_PRECIOUS|M_NOEXPORT, &Pwd_macval);
   Pwd = DmStrDup(Pwd_macval); /* Later done by Def_macro(). */
   _set_string_var("TMD",  ".",      M_PRECIOUS|M_NOEXPORT, &Tmd_macval);
   Tmd = DmStrDup(Tmd_macval); /* Later done by _set_tmd(). */

   Def_macro("NULL", "", M_PRECIOUS|M_NOEXPORT|M_FLAG);

   /* Initialize a macro that contains a space. As leading and trailing
    * spaces are stripped by Def_macro a little cheating is necessary. */
   _set_string_var("SPACECHAR", "x", M_PRECIOUS|M_NOEXPORT|M_FLAG, &Spacechar );
   Spacechar[0] = ' ';

   _set_int_var( "MAXLINELENGTH", "0", M_DEFAULT|M_NOEXPORT, &Buffer_size );
   _set_int_var( "PREP",          "0", M_DEFAULT, &Prep );
   (void) Def_macro("MAXLINELENGTH", "1024", M_FLAG | M_DEFAULT);

   /* MAXPROCESSLIMIT is overwritten by the ruletab.c settings. Set its
    * initial value high so that it allows MAXPROCESS to be changed
    * from the command line. */
   _set_int_var( "MAXPROCESSLIMIT", "100", M_DEFAULT|M_NOEXPORT,&Max_proclmt );
#if defined(USE_CREATEPROCESS)
   /* Set the OS early enough. */
   Max_proclmt = MAXIMUM_WAIT_OBJECTS;
#endif
   _set_int_var( "MAXPROCESS", "1", M_DEFAULT|M_NOEXPORT, &Max_proc );
   sprintf(buf,"%d",NAME_MAX);
   _set_int_var( "NAMEMAX", buf, M_DEFAULT|M_NOEXPORT, &NameMax);
}


/*
** Define an integer variable value, and set up the macro.
*/
static void
_set_int_var(name, val, flag, var)
char *name;
char *val;
int  flag;
int  *var;
{
   HASHPTR hp;

   hp = Def_macro(name, val, M_FLAG | flag);
   hp->ht_flag |= M_VAR_INT | M_MULTI | M_INIT;
   hp->MV_IVAR  = var;
   *var         = atoi(val);
}


/*
** Define a string variables value, and set up the macro.
*/
static void
_set_string_var(name, val, flag, var)
char *name;
char *val;
int  flag;
char **var;
{
   HASHPTR hp;

   hp = Def_macro(name, val, M_FLAG | flag);
   hp->ht_flag |= M_VAR_STRING | M_MULTI | M_INIT;
   hp->MV_SVAR  = var;
   *var         = hp->ht_value;
}


/* Define a bit variable value, and set up the macro. Each of the bits
 * corresponds to an attribute bit of Glob_attr. */
static void
_set_bit_var(name, val, mask)
char *name;
char *val;
int  mask;
{
   HASHPTR hp;

   hp           = Def_macro(name, val, M_FLAG);
   hp->ht_flag |= M_VAR_BIT | M_MULTI | M_INIT;
   hp->MV_MASK  = mask;
   hp->MV_BVAR  = &Glob_attr;
}
