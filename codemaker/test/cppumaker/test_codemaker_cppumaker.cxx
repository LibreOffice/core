/*************************************************************************
 *
 *  $RCSfile: test_codemaker_cppumaker.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-03-30 16:54:02 $
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

#include "FILE.hpp"
#include "lconv.hpp"
#include "tm.hpp"
#include "std.hpp"
#include "test/codemaker/cppumaker/XTest.hpp"
#include "test/codemaker/cppumaker/S1.hpp"
#include "test/codemaker/cppumaker/asm.hpp"
#include "test/codemaker/cppumaker/auto.hpp"
#include "test/codemaker/cppumaker/bool.hpp"
#include "test/codemaker/cppumaker/break.hpp"
//TODO: #include "test/codemaker/cppumaker/case.hpp"
#include "test/codemaker/cppumaker/catch.hpp"
//TODO: #include "test/codemaker/cppumaker/char.hpp"
#include "test/codemaker/cppumaker/class.hpp"
//TODO: #include "test/codemaker/cppumaker/const.hpp"
#include "test/codemaker/cppumaker/continue.hpp"
//TODO: #include "test/codemaker/cppumaker/default.hpp"
#include "test/codemaker/cppumaker/delete.hpp"
#include "test/codemaker/cppumaker/do.hpp"
//TODO: #include "test/codemaker/cppumaker/double.hpp"
#include "test/codemaker/cppumaker/else.hpp"
//TODO: #include "test/codemaker/cppumaker/enum.hpp"
#include "test/codemaker/cppumaker/explicit.hpp"
#include "test/codemaker/cppumaker/export.hpp"
#include "test/codemaker/cppumaker/extern.hpp"
#include "test/codemaker/cppumaker/false.hpp"
//TODO: #include "test/codemaker/cppumaker/float.hpp"
#include "test/codemaker/cppumaker/for.hpp"
#include "test/codemaker/cppumaker/friend.hpp"
#include "test/codemaker/cppumaker/goto.hpp"
#include "test/codemaker/cppumaker/if.hpp"
#include "test/codemaker/cppumaker/inline.hpp"
#include "test/codemaker/cppumaker/int.hpp"
//TODO: #include "test/codemaker/cppumaker/long.hpp"
#include "test/codemaker/cppumaker/mutable.hpp"
#include "test/codemaker/cppumaker/namespace.hpp"
#include "test/codemaker/cppumaker/new.hpp"
#include "test/codemaker/cppumaker/operator.hpp"
#include "test/codemaker/cppumaker/private.hpp"
#include "test/codemaker/cppumaker/protected.hpp"
#include "test/codemaker/cppumaker/public.hpp"
#include "test/codemaker/cppumaker/register.hpp"
#include "test/codemaker/cppumaker/return.hpp"
//TODO: #include "test/codemaker/cppumaker/short.hpp"
#include "test/codemaker/cppumaker/signed.hpp"
#include "test/codemaker/cppumaker/sizeof.hpp"
#include "test/codemaker/cppumaker/static.hpp"
//TODO: #include "test/codemaker/cppumaker/struct.hpp"
//TODO: #include "test/codemaker/cppumaker/switch.hpp"
#include "test/codemaker/cppumaker/template.hpp"
#include "test/codemaker/cppumaker/this.hpp"
#include "test/codemaker/cppumaker/throw.hpp"
#include "test/codemaker/cppumaker/true.hpp"
#include "test/codemaker/cppumaker/try.hpp"
//TODO: #include "test/codemaker/cppumaker/typedef.hpp"
#include "test/codemaker/cppumaker/typeid.hpp"
#include "test/codemaker/cppumaker/typename.hpp"
//TODO: #include "test/codemaker/cppumaker/union.hpp"
//TODO: #include "test/codemaker/cppumaker/unsigned.hpp"
#include "test/codemaker/cppumaker/using.hpp"
#include "test/codemaker/cppumaker/virtual.hpp"
//TODO: #include "test/codemaker/cppumaker/void.hpp"
#include "test/codemaker/cppumaker/volatile.hpp"
#include "test/codemaker/cppumaker/while.hpp"
#include "test/codemaker/cppumaker/and.hpp"
#include "test/codemaker/cppumaker/bitand.hpp"
#include "test/codemaker/cppumaker/bitor.hpp"
#include "test/codemaker/cppumaker/compl.hpp"
#include "test/codemaker/cppumaker/not.hpp"
#include "test/codemaker/cppumaker/or.hpp"
#include "test/codemaker/cppumaker/xor.hpp"
#include "test/codemaker/cppumaker/BUFSIZ.hpp"
#include "test/codemaker/cppumaker/CLOCKS_PER_SEC.hpp"
#include "test/codemaker/cppumaker/EDOM.hpp"
#include "test/codemaker/cppumaker/EOF.hpp"
#include "test/codemaker/cppumaker/ERANGE.hpp"
#include "test/codemaker/cppumaker/EXIT_FAILURE.hpp"
#include "test/codemaker/cppumaker/EXIT_SUCCESS.hpp"
#include "test/codemaker/cppumaker/FILENAME_MAX.hpp"
#include "test/codemaker/cppumaker/FOPEN_MAX.hpp"
#include "test/codemaker/cppumaker/HUGE_VAL.hpp"
#include "test/codemaker/cppumaker/LC_ALL.hpp"
#include "test/codemaker/cppumaker/LC_COLLATE.hpp"
#include "test/codemaker/cppumaker/LC_CTYPE.hpp"
#include "test/codemaker/cppumaker/LC_MONETARY.hpp"
#include "test/codemaker/cppumaker/LC_NUMERIC.hpp"
#include "test/codemaker/cppumaker/LC_TIME.hpp"
#include "test/codemaker/cppumaker/L_tmpnam.hpp"
#include "test/codemaker/cppumaker/MB_CUR_MAX.hpp"
#include "test/codemaker/cppumaker/NULL.hpp"
#include "test/codemaker/cppumaker/RAND_MAX.hpp"
#include "test/codemaker/cppumaker/SEEK_CUR.hpp"
#include "test/codemaker/cppumaker/SEEK_END.hpp"
#include "test/codemaker/cppumaker/SEEK_SET.hpp"
#include "test/codemaker/cppumaker/SIGABRT.hpp"
#include "test/codemaker/cppumaker/SIGFPE.hpp"
#include "test/codemaker/cppumaker/SIGILL.hpp"
#include "test/codemaker/cppumaker/SIGINT.hpp"
#include "test/codemaker/cppumaker/SIGSEGV.hpp"
#include "test/codemaker/cppumaker/SIGTERM.hpp"
#include "test/codemaker/cppumaker/SIG_DFL.hpp"
#include "test/codemaker/cppumaker/SIG_ERR.hpp"
#include "test/codemaker/cppumaker/SIG_IGN.hpp"
#include "test/codemaker/cppumaker/TMP_MAX.hpp"
#include "test/codemaker/cppumaker/WCHAR_MAX.hpp"
#include "test/codemaker/cppumaker/WCHAR_MIN.hpp"
#include "test/codemaker/cppumaker/WEOF.hpp"
#include "test/codemaker/cppumaker/assert.hpp"
#include "test/codemaker/cppumaker/errno.hpp"
#include "test/codemaker/cppumaker/offsetof.hpp"
#include "test/codemaker/cppumaker/setjmp.hpp"
#include "test/codemaker/cppumaker/stderr.hpp"
#include "test/codemaker/cppumaker/stdin.hpp"
#include "test/codemaker/cppumaker/stdout.hpp"
#include "test/codemaker/cppumaker/CHAR_BIT.hpp"
#include "test/codemaker/cppumaker/CHAR_MAX.hpp"
#include "test/codemaker/cppumaker/CHAR_MIN.hpp"
#include "test/codemaker/cppumaker/DBL_DIG.hpp"
#include "test/codemaker/cppumaker/DBL_EPSILON.hpp"
#include "test/codemaker/cppumaker/DBL_MANT_DIG.hpp"
#include "test/codemaker/cppumaker/DBL_MAX.hpp"
#include "test/codemaker/cppumaker/DBL_MAX_10_EXP.hpp"
#include "test/codemaker/cppumaker/DBL_MAX_EXP.hpp"
#include "test/codemaker/cppumaker/DBL_MIN.hpp"
#include "test/codemaker/cppumaker/DBL_MIN_10_EXP.hpp"
#include "test/codemaker/cppumaker/DBL_MIN_EXP.hpp"
#include "test/codemaker/cppumaker/FLT_DIG.hpp"
#include "test/codemaker/cppumaker/FLT_EPSILON.hpp"
#include "test/codemaker/cppumaker/FLT_MANT_DIG.hpp"
#include "test/codemaker/cppumaker/FLT_MAX.hpp"
#include "test/codemaker/cppumaker/FLT_MAX_10_EXP.hpp"
#include "test/codemaker/cppumaker/FLT_MAX_EXP.hpp"
#include "test/codemaker/cppumaker/FLT_MIN.hpp"
#include "test/codemaker/cppumaker/FLT_MIN_10_EXP.hpp"
#include "test/codemaker/cppumaker/FLT_MIN_EXP.hpp"
#include "test/codemaker/cppumaker/FLT_RADIX.hpp"
#include "test/codemaker/cppumaker/FLT_ROUNDS.hpp"
#include "test/codemaker/cppumaker/INT_MAX.hpp"
#include "test/codemaker/cppumaker/INT_MIN.hpp"
#include "test/codemaker/cppumaker/LDBL_DIG.hpp"
#include "test/codemaker/cppumaker/LDBL_EPSILON.hpp"
#include "test/codemaker/cppumaker/LDBL_MANT_DIG.hpp"
#include "test/codemaker/cppumaker/LDBL_MAX.hpp"
#include "test/codemaker/cppumaker/LDBL_MAX_10_EXP.hpp"
#include "test/codemaker/cppumaker/LDBL_MAX_EXP.hpp"
#include "test/codemaker/cppumaker/LDBL_MIN.hpp"
#include "test/codemaker/cppumaker/LDBL_MIN_10_EXP.hpp"
#include "test/codemaker/cppumaker/LDBL_MIN_EXP.hpp"
#include "test/codemaker/cppumaker/LONG_MAX.hpp"
#include "test/codemaker/cppumaker/LONG_MIN.hpp"
#include "test/codemaker/cppumaker/MB_LEN_MAX.hpp"
#include "test/codemaker/cppumaker/SCHAR_MAX.hpp"
#include "test/codemaker/cppumaker/SCHAR_MIN.hpp"
#include "test/codemaker/cppumaker/SHRT_MAX.hpp"
#include "test/codemaker/cppumaker/SHRT_MIN.hpp"
#include "test/codemaker/cppumaker/UCHAR_MAX.hpp"
#include "test/codemaker/cppumaker/UINT_MAX.hpp"
#include "test/codemaker/cppumaker/ULONG_MAX.hpp"
#include "test/codemaker/cppumaker/USHRT_MAX.hpp"
#include "test/codemaker/cppumaker/FILE.hpp"
#include "test/codemaker/cppumaker/lconv.hpp"
#include "test/codemaker/cppumaker/tm.hpp"
#include "test/codemaker/cppumaker/std.hpp"
#include "test/codemaker/cppumaker/NDEBUG.hpp"
#include "test/codemaker/cppumaker/get.hpp"

//TODO
