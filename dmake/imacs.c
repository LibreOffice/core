/* RCS  $Id: imacs.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
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
#include "version.h"

static  void    _set_int_var ANSI((char *, char *, int, int *));
static  void    _set_string_var ANSI((char *, char *, int, char **));
static  void    _set_bit_var ANSI((char *, char *, int));

/*
** Arrange to parse the strings stored in Rules[]
*/
PUBLIC void
Make_rules()
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
#else
         DirSepStr = "/";
#endif
   _set_string_var("DIRSEPSTR", DirSepStr, M_DEFAULT,&DirSepStr);
   _set_string_var("DIRBRKSTR", DirBrkStr, M_DEFAULT, &DirBrkStr);
   swchar[0] = DEF_ESCAPE_CHAR, swchar[1] = '\0';
   _set_string_var(".ESCAPE_PREFIX", swchar, M_FLAG, &Escape_char);

   _set_bit_var(".SILENT",   "", A_SILENT  );
   _set_bit_var(".IGNORE",   "", A_IGNORE  );
   _set_bit_var(".PRECIOUS", "", A_PRECIOUS);
   _set_bit_var(".EPILOG",   "", A_EPILOG  );
   _set_bit_var(".PROLOG",   "", A_PROLOG  );
   _set_bit_var(".NOINFER",  "", A_NOINFER );
   _set_bit_var(".SEQUENTIAL","",A_SEQ     );
   _set_bit_var(".USESHELL", "", A_SHELL   );
   _set_bit_var(".SWAP",     "", A_SWAP    );
   _set_bit_var(".MKSARGS",  "", A_MKSARGS );
   _set_bit_var(".IGNOREGROUP","",A_IGNOREGROUP);

   Glob_attr = A_DEFAULT;        /* set all flags to NULL   */

   _set_string_var("SHELL",        "",  M_DEFAULT, &Shell       );
   _set_string_var("SHELLFLAGS",   " ", M_DEFAULT, &Shell_flags );
   _set_string_var("GROUPSHELL",   "",  M_DEFAULT, &GShell      );
   _set_string_var("GROUPFLAGS",   " ", M_DEFAULT, &GShell_flags);
   _set_string_var("SHELLMETAS",   "",  M_DEFAULT, &Shell_metas );
   _set_string_var("GROUPSUFFIX",  "",  M_DEFAULT, &Grp_suff    );
   _set_string_var("AUGMAKE",NIL(char), M_DEFAULT, &Augmake     );
   _set_string_var(".KEEP_STATE",  "",  M_DEFAULT, &Keep_state  );
   _set_string_var(".NOTABS",      "",  M_MULTI, &Notabs );
   _set_string_var(".DIRCACHE",    "y", M_DEFAULT, &UseDirCache );
   _set_string_var(".DIRCACHERESPECTCASE", "", M_DEFAULT, &DcacheRespCase);

   _set_string_var("MAKEDIR",Get_current_dir(),M_PRECIOUS|M_NOEXPORT,&Makedir);
   _set_string_var("MAKEVERSION", VERSION, M_DEFAULT|M_PRECIOUS, &version);
   _set_string_var("PWD",  Makedir,  M_DEFAULT|M_NOEXPORT, &Pwd);
   _set_string_var("TMD",  "",       M_DEFAULT|M_NOEXPORT, &Tmd);

   Def_macro("NULL", "", M_PRECIOUS|M_NOEXPORT|M_FLAG);

   _set_int_var( "MAXLINELENGTH", "0", M_DEFAULT|M_NOEXPORT, &Buffer_size );
   _set_int_var( "PREP",          "0", M_DEFAULT, &Prep );
   (void) Def_macro("MAXLINELENGTH", "1024", M_FLAG | M_DEFAULT);

   /* set MAXPROCESSLIMIT high initially so that it allows MAXPROCESS to
    * change from command line. */
   _set_int_var( "MAXPROCESSLIMIT", "100", M_DEFAULT|M_NOEXPORT,&Max_proclmt );
   _set_int_var( "MAXPROCESS", "1", M_DEFAULT|M_NOEXPORT, &Max_proc );
   _set_int_var( "DYNAMICNESTINGLEVEL", "100", M_DEFAULT|M_NOEXPORT,
         &DynamicNestLevel);
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


/*
** Define a bit variable value, and set up the macro.
*/
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
