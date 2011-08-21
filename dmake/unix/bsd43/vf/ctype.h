/* RCS  $Id: ctype.h,v 1.1.1.1 2000-09-22 15:33:34 hr Exp $
--
-- SYNOPSIS
--      ctype
--
-- DESCRIPTION
--      ctype.h 4.2 85/09/04
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

#define _U  01
#define _L  02
#define _N  04
#define _S  010
#define _P  020
#define _C  040
#define _X  0100
#define _B  0200

extern  char    _ctype_[];

#define isalpha(c)  ((_ctype_+1)[c]&(_U|_L))
#define isupper(c)  ((_ctype_+1)[c]&_U)
#define islower(c)  ((_ctype_+1)[c]&_L)
#define isdigit(c)  ((_ctype_+1)[c]&_N)
#define isxdigit(c) ((_ctype_+1)[c]&(_N|_X))
#define isspace(c)  ((_ctype_+1)[c]&_S)
#define ispunct(c)  ((_ctype_+1)[c]&_P)
#define isalnum(c)  ((_ctype_+1)[c]&(_U|_L|_N))
#define isprint(c)  ((_ctype_+1)[c]&(_P|_U|_L|_N|_B))
#define isgraph(c)  ((_ctype_+1)[c]&(_P|_U|_L|_N))
#define iscntrl(c)  ((_ctype_+1)[c]&_C)
#define isascii(c)  ((unsigned)(c)<=0177)
#define toupper(c)  ((c)-'a'+'A')
#define tolower(c)  ((c)-'A'+'a')
#define toascii(c)  ((c)&0177)
