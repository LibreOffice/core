/* RCS  $Id: db.h,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Front end to DBUG macros.
--
-- DESCRIPTION
--  This is a front end to Fred Fish's DBUG macros.  The intent was
--  to provide an interface so that if you don't have the DBUG code
--  you can still compile dmake, by undefining DBUG, if you do have
--  the code then you can use Fred Fish's DBUG package.  Originally
--  the DBUG stuff was copyrighted, it is now in the public domain
--  so the need for this is not as apparent.
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

#ifndef DB_h
#define DB_h

#ifdef DBUG

#  include "dbug.h"

#  define DB_ENTER(a1)                DBUG_ENTER(a1)
#  define DB_RETURN(a1)               DBUG_RETURN(a1)
#  define DB_VOID_RETURN              DBUG_VOID_RETURN
#  define DB_EXECUTE(keyword, a1)     DBUG_EXECUTE(keyword,a1)
#  define DB_PRINT(keyword,arglist)   DBUG_PRINT(keyword,arglist)
#  define DB_PUSH(a1)                 DBUG_PUSH(a1)
#  define DB_POP()                    DBUG_POP()
#  define DB_PROCESS(a1)              DBUG_PROCESS(a1)
#  define DB_FILE(file)               DBUG_FILE(file)
#  define DB_SETJMP                   DBUG_SETJMP
#  define DB_LONGJMP                  DBUG_LONGJMP

#else

#  define DB_ENTER(a1)
#  define DB_RETURN(a1)               return (a1)
#  define DB_VOID_RETURN              return
#  define DB_EXECUTE(keyword, a1)
#  define DB_PRINT(keyword,arglist)
#  define DB_PUSH(a1)
#  define DB_POP()
#  define DB_PROCESS(a1)
#  define DB_FILE(file)
#  define DB_SETJMP                   setjmp
#  define DB_LONGJMP                  longjmp

#endif
#endif

