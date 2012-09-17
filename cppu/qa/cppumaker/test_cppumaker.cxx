/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/types.h>

#include "FILE.hpp"
#include "lconv.hpp"
#include "tm.hpp"
#include "std.hpp"
#include "test/codemaker/cppumaker/XTest.hpp"
#include "test/codemaker/cppumaker/S1.hpp"
#include "test/codemaker/cppumaker/services/asm.hpp"
#include "test/codemaker/cppumaker/services/auto.hpp"
#include "test/codemaker/cppumaker/services/bool.hpp"
#include "test/codemaker/cppumaker/services/break.hpp"
//TODO: #include "test/codemaker/cppumaker/services/case.hpp"
#include "test/codemaker/cppumaker/services/catch.hpp"
//TODO: #include "test/codemaker/cppumaker/services/char.hpp"
#include "test/codemaker/cppumaker/services/class.hpp"
//TODO: #include "test/codemaker/cppumaker/services/const.hpp"
#include "test/codemaker/cppumaker/services/continue.hpp"
//TODO: #include "test/codemaker/cppumaker/services/default.hpp"
#include "test/codemaker/cppumaker/services/delete.hpp"
#include "test/codemaker/cppumaker/services/do.hpp"
//TODO: #include "test/codemaker/cppumaker/services/double.hpp"
#include "test/codemaker/cppumaker/services/else.hpp"
//TODO: #include "test/codemaker/cppumaker/services/enum.hpp"
#include "test/codemaker/cppumaker/services/explicit.hpp"
#include "test/codemaker/cppumaker/services/export.hpp"
#include "test/codemaker/cppumaker/services/extern.hpp"
#include "test/codemaker/cppumaker/services/false.hpp"
//TODO: #include "test/codemaker/cppumaker/services/float.hpp"
#include "test/codemaker/cppumaker/services/for.hpp"
#include "test/codemaker/cppumaker/services/friend.hpp"
#include "test/codemaker/cppumaker/services/goto.hpp"
#include "test/codemaker/cppumaker/services/if.hpp"
#include "test/codemaker/cppumaker/services/inline.hpp"
#include "test/codemaker/cppumaker/services/int.hpp"
//TODO: #include "test/codemaker/cppumaker/services/long.hpp"
#include "test/codemaker/cppumaker/services/mutable.hpp"
#include "test/codemaker/cppumaker/services/namespace.hpp"
#include "test/codemaker/cppumaker/services/new.hpp"
#include "test/codemaker/cppumaker/services/operator.hpp"
#include "test/codemaker/cppumaker/services/private.hpp"
#include "test/codemaker/cppumaker/services/protected.hpp"
#include "test/codemaker/cppumaker/services/public.hpp"
#include "test/codemaker/cppumaker/services/register.hpp"
#include "test/codemaker/cppumaker/services/return.hpp"
//TODO: #include "test/codemaker/cppumaker/services/short.hpp"
#include "test/codemaker/cppumaker/services/signed.hpp"
#include "test/codemaker/cppumaker/services/sizeof.hpp"
#include "test/codemaker/cppumaker/services/static.hpp"
//TODO: #include "test/codemaker/cppumaker/services/struct.hpp"
//TODO: #include "test/codemaker/cppumaker/services/switch.hpp"
#include "test/codemaker/cppumaker/services/template.hpp"
#include "test/codemaker/cppumaker/services/this.hpp"
#include "test/codemaker/cppumaker/services/throw.hpp"
#include "test/codemaker/cppumaker/services/true.hpp"
#include "test/codemaker/cppumaker/services/try.hpp"
//TODO: #include "test/codemaker/cppumaker/services/typedef.hpp"
#include "test/codemaker/cppumaker/services/typeid.hpp"
#include "test/codemaker/cppumaker/services/typename.hpp"
//TODO: #include "test/codemaker/cppumaker/services/union.hpp"
//TODO: #include "test/codemaker/cppumaker/services/unsigned.hpp"
#include "test/codemaker/cppumaker/services/using.hpp"
#include "test/codemaker/cppumaker/services/virtual.hpp"
//TODO: #include "test/codemaker/cppumaker/services/void.hpp"
#include "test/codemaker/cppumaker/services/volatile.hpp"
#include "test/codemaker/cppumaker/services/while.hpp"
#include "test/codemaker/cppumaker/services/and.hpp"
#include "test/codemaker/cppumaker/services/bitand.hpp"
#include "test/codemaker/cppumaker/services/bitor.hpp"
#include "test/codemaker/cppumaker/services/compl.hpp"
#include "test/codemaker/cppumaker/services/not.hpp"
#include "test/codemaker/cppumaker/services/or.hpp"
#include "test/codemaker/cppumaker/services/xor.hpp"
#include "test/codemaker/cppumaker/services/BUFSIZ.hpp"
#include "test/codemaker/cppumaker/services/CLOCKS_PER_SEC.hpp"
#include "test/codemaker/cppumaker/services/EDOM.hpp"
#include "test/codemaker/cppumaker/services/EOF.hpp"
#include "test/codemaker/cppumaker/services/ERANGE.hpp"
#include "test/codemaker/cppumaker/services/EXIT_FAILURE.hpp"
#include "test/codemaker/cppumaker/services/EXIT_SUCCESS.hpp"
#include "test/codemaker/cppumaker/services/FILENAME_MAX.hpp"
#include "test/codemaker/cppumaker/services/FOPEN_MAX.hpp"
#include "test/codemaker/cppumaker/services/HUGE_VAL.hpp"
#include "test/codemaker/cppumaker/services/LC_ALL.hpp"
#include "test/codemaker/cppumaker/services/LC_COLLATE.hpp"
#include "test/codemaker/cppumaker/services/LC_CTYPE.hpp"
#include "test/codemaker/cppumaker/services/LC_MONETARY.hpp"
#include "test/codemaker/cppumaker/services/LC_NUMERIC.hpp"
#include "test/codemaker/cppumaker/services/LC_TIME.hpp"
#include "test/codemaker/cppumaker/services/L_tmpnam.hpp"
#include "test/codemaker/cppumaker/services/MB_CUR_MAX.hpp"
#include "test/codemaker/cppumaker/services/NULL.hpp"
#include "test/codemaker/cppumaker/services/RAND_MAX.hpp"
#include "test/codemaker/cppumaker/services/SEEK_CUR.hpp"
#include "test/codemaker/cppumaker/services/SEEK_END.hpp"
#include "test/codemaker/cppumaker/services/SEEK_SET.hpp"
#include "test/codemaker/cppumaker/services/SIGABRT.hpp"
#include "test/codemaker/cppumaker/services/SIGFPE.hpp"
#include "test/codemaker/cppumaker/services/SIGILL.hpp"
#include "test/codemaker/cppumaker/services/SIGINT.hpp"
#include "test/codemaker/cppumaker/services/SIGSEGV.hpp"
#include "test/codemaker/cppumaker/services/SIGTERM.hpp"
#include "test/codemaker/cppumaker/services/SIG_DFL.hpp"
#include "test/codemaker/cppumaker/services/SIG_ERR.hpp"
#include "test/codemaker/cppumaker/services/SIG_IGN.hpp"
#include "test/codemaker/cppumaker/services/TMP_MAX.hpp"
#include "test/codemaker/cppumaker/services/WCHAR_MAX.hpp"
#include "test/codemaker/cppumaker/services/WCHAR_MIN.hpp"
#include "test/codemaker/cppumaker/services/WEOF.hpp"
#include "test/codemaker/cppumaker/services/assert.hpp"
#include "test/codemaker/cppumaker/services/errno.hpp"
#include "test/codemaker/cppumaker/services/offsetof.hpp"
#include "test/codemaker/cppumaker/services/setjmp.hpp"
#include "test/codemaker/cppumaker/services/stderr.hpp"
#include "test/codemaker/cppumaker/services/stdin.hpp"
#include "test/codemaker/cppumaker/services/stdout.hpp"
#include "test/codemaker/cppumaker/services/CHAR_BIT.hpp"
#include "test/codemaker/cppumaker/services/CHAR_MAX.hpp"
#include "test/codemaker/cppumaker/services/CHAR_MIN.hpp"
#include "test/codemaker/cppumaker/services/DBL_DIG.hpp"
#include "test/codemaker/cppumaker/services/DBL_EPSILON.hpp"
#include "test/codemaker/cppumaker/services/DBL_MANT_DIG.hpp"
#include "test/codemaker/cppumaker/services/DBL_MAX.hpp"
#include "test/codemaker/cppumaker/services/DBL_MAX_10_EXP.hpp"
#include "test/codemaker/cppumaker/services/DBL_MAX_EXP.hpp"
#include "test/codemaker/cppumaker/services/DBL_MIN.hpp"
#include "test/codemaker/cppumaker/services/DBL_MIN_10_EXP.hpp"
#include "test/codemaker/cppumaker/services/DBL_MIN_EXP.hpp"
#include "test/codemaker/cppumaker/services/FLT_DIG.hpp"
#include "test/codemaker/cppumaker/services/FLT_EPSILON.hpp"
#include "test/codemaker/cppumaker/services/FLT_MANT_DIG.hpp"
#include "test/codemaker/cppumaker/services/FLT_MAX.hpp"
#include "test/codemaker/cppumaker/services/FLT_MAX_10_EXP.hpp"
#include "test/codemaker/cppumaker/services/FLT_MAX_EXP.hpp"
#include "test/codemaker/cppumaker/services/FLT_MIN.hpp"
#include "test/codemaker/cppumaker/services/FLT_MIN_10_EXP.hpp"
#include "test/codemaker/cppumaker/services/FLT_MIN_EXP.hpp"
#include "test/codemaker/cppumaker/services/FLT_RADIX.hpp"
#include "test/codemaker/cppumaker/services/FLT_ROUNDS.hpp"
#include "test/codemaker/cppumaker/services/INT_MAX.hpp"
#include "test/codemaker/cppumaker/services/INT_MIN.hpp"
#include "test/codemaker/cppumaker/services/LDBL_DIG.hpp"
#include "test/codemaker/cppumaker/services/LDBL_EPSILON.hpp"
#include "test/codemaker/cppumaker/services/LDBL_MANT_DIG.hpp"
#include "test/codemaker/cppumaker/services/LDBL_MAX.hpp"
#include "test/codemaker/cppumaker/services/LDBL_MAX_10_EXP.hpp"
#include "test/codemaker/cppumaker/services/LDBL_MAX_EXP.hpp"
#include "test/codemaker/cppumaker/services/LDBL_MIN.hpp"
#include "test/codemaker/cppumaker/services/LDBL_MIN_10_EXP.hpp"
#include "test/codemaker/cppumaker/services/LDBL_MIN_EXP.hpp"
#include "test/codemaker/cppumaker/services/LONG_MAX.hpp"
#include "test/codemaker/cppumaker/services/LONG_MIN.hpp"
#include "test/codemaker/cppumaker/services/MB_LEN_MAX.hpp"
#include "test/codemaker/cppumaker/services/SCHAR_MAX.hpp"
#include "test/codemaker/cppumaker/services/SCHAR_MIN.hpp"
#include "test/codemaker/cppumaker/services/SHRT_MAX.hpp"
#include "test/codemaker/cppumaker/services/SHRT_MIN.hpp"
#include "test/codemaker/cppumaker/services/UCHAR_MAX.hpp"
#include "test/codemaker/cppumaker/services/UINT_MAX.hpp"
#include "test/codemaker/cppumaker/services/ULONG_MAX.hpp"
#include "test/codemaker/cppumaker/services/USHRT_MAX.hpp"
#include "test/codemaker/cppumaker/services/FILE.hpp"
#include "test/codemaker/cppumaker/services/lconv.hpp"
#include "test/codemaker/cppumaker/services/tm.hpp"
#include "test/codemaker/cppumaker/services/std.hpp"
#include "test/codemaker/cppumaker/services/NDEBUG.hpp"
#include "test/codemaker/cppumaker/services/create.hpp"
#include "test/codemaker/cppumaker/singletons/asm.hpp"
#include "test/codemaker/cppumaker/singletons/auto.hpp"
#include "test/codemaker/cppumaker/singletons/bool.hpp"
#include "test/codemaker/cppumaker/singletons/break.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/case.hpp"
#include "test/codemaker/cppumaker/singletons/catch.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/char.hpp"
#include "test/codemaker/cppumaker/singletons/class.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/const.hpp"
#include "test/codemaker/cppumaker/singletons/continue.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/default.hpp"
#include "test/codemaker/cppumaker/singletons/delete.hpp"
#include "test/codemaker/cppumaker/singletons/do.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/double.hpp"
#include "test/codemaker/cppumaker/singletons/else.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/enum.hpp"
#include "test/codemaker/cppumaker/singletons/explicit.hpp"
#include "test/codemaker/cppumaker/singletons/export.hpp"
#include "test/codemaker/cppumaker/singletons/extern.hpp"
#include "test/codemaker/cppumaker/singletons/false.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/float.hpp"
#include "test/codemaker/cppumaker/singletons/for.hpp"
#include "test/codemaker/cppumaker/singletons/friend.hpp"
#include "test/codemaker/cppumaker/singletons/goto.hpp"
#include "test/codemaker/cppumaker/singletons/if.hpp"
#include "test/codemaker/cppumaker/singletons/inline.hpp"
#include "test/codemaker/cppumaker/singletons/int.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/long.hpp"
#include "test/codemaker/cppumaker/singletons/mutable.hpp"
#include "test/codemaker/cppumaker/singletons/namespace.hpp"
#include "test/codemaker/cppumaker/singletons/new.hpp"
#include "test/codemaker/cppumaker/singletons/operator.hpp"
#include "test/codemaker/cppumaker/singletons/private.hpp"
#include "test/codemaker/cppumaker/singletons/protected.hpp"
#include "test/codemaker/cppumaker/singletons/public.hpp"
#include "test/codemaker/cppumaker/singletons/register.hpp"
#include "test/codemaker/cppumaker/singletons/return.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/short.hpp"
#include "test/codemaker/cppumaker/singletons/signed.hpp"
#include "test/codemaker/cppumaker/singletons/sizeof.hpp"
#include "test/codemaker/cppumaker/singletons/static.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/struct.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/switch.hpp"
#include "test/codemaker/cppumaker/singletons/template.hpp"
#include "test/codemaker/cppumaker/singletons/this.hpp"
#include "test/codemaker/cppumaker/singletons/throw.hpp"
#include "test/codemaker/cppumaker/singletons/true.hpp"
#include "test/codemaker/cppumaker/singletons/try.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/typedef.hpp"
#include "test/codemaker/cppumaker/singletons/typeid.hpp"
#include "test/codemaker/cppumaker/singletons/typename.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/union.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/unsigned.hpp"
#include "test/codemaker/cppumaker/singletons/using.hpp"
#include "test/codemaker/cppumaker/singletons/virtual.hpp"
//TODO: #include "test/codemaker/cppumaker/singletons/void.hpp"
#include "test/codemaker/cppumaker/singletons/volatile.hpp"
#include "test/codemaker/cppumaker/singletons/while.hpp"
#include "test/codemaker/cppumaker/singletons/and.hpp"
#include "test/codemaker/cppumaker/singletons/bitand.hpp"
#include "test/codemaker/cppumaker/singletons/bitor.hpp"
#include "test/codemaker/cppumaker/singletons/compl.hpp"
#include "test/codemaker/cppumaker/singletons/not.hpp"
#include "test/codemaker/cppumaker/singletons/or.hpp"
#include "test/codemaker/cppumaker/singletons/xor.hpp"
#include "test/codemaker/cppumaker/singletons/BUFSIZ.hpp"
#include "test/codemaker/cppumaker/singletons/CLOCKS_PER_SEC.hpp"
#include "test/codemaker/cppumaker/singletons/EDOM.hpp"
#include "test/codemaker/cppumaker/singletons/EOF.hpp"
#include "test/codemaker/cppumaker/singletons/ERANGE.hpp"
#include "test/codemaker/cppumaker/singletons/EXIT_FAILURE.hpp"
#include "test/codemaker/cppumaker/singletons/EXIT_SUCCESS.hpp"
#include "test/codemaker/cppumaker/singletons/FILENAME_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/FOPEN_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/HUGE_VAL.hpp"
#include "test/codemaker/cppumaker/singletons/LC_ALL.hpp"
#include "test/codemaker/cppumaker/singletons/LC_COLLATE.hpp"
#include "test/codemaker/cppumaker/singletons/LC_CTYPE.hpp"
#include "test/codemaker/cppumaker/singletons/LC_MONETARY.hpp"
#include "test/codemaker/cppumaker/singletons/LC_NUMERIC.hpp"
#include "test/codemaker/cppumaker/singletons/LC_TIME.hpp"
#include "test/codemaker/cppumaker/singletons/L_tmpnam.hpp"
#include "test/codemaker/cppumaker/singletons/MB_CUR_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/NULL.hpp"
#include "test/codemaker/cppumaker/singletons/RAND_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/SEEK_CUR.hpp"
#include "test/codemaker/cppumaker/singletons/SEEK_END.hpp"
#include "test/codemaker/cppumaker/singletons/SEEK_SET.hpp"
#include "test/codemaker/cppumaker/singletons/SIGABRT.hpp"
#include "test/codemaker/cppumaker/singletons/SIGFPE.hpp"
#include "test/codemaker/cppumaker/singletons/SIGILL.hpp"
#include "test/codemaker/cppumaker/singletons/SIGINT.hpp"
#include "test/codemaker/cppumaker/singletons/SIGSEGV.hpp"
#include "test/codemaker/cppumaker/singletons/SIGTERM.hpp"
#include "test/codemaker/cppumaker/singletons/SIG_DFL.hpp"
#include "test/codemaker/cppumaker/singletons/SIG_ERR.hpp"
#include "test/codemaker/cppumaker/singletons/SIG_IGN.hpp"
#include "test/codemaker/cppumaker/singletons/TMP_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/WCHAR_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/WCHAR_MIN.hpp"
#include "test/codemaker/cppumaker/singletons/WEOF.hpp"
#include "test/codemaker/cppumaker/singletons/assert.hpp"
#include "test/codemaker/cppumaker/singletons/errno.hpp"
#include "test/codemaker/cppumaker/singletons/offsetof.hpp"
#include "test/codemaker/cppumaker/singletons/setjmp.hpp"
#include "test/codemaker/cppumaker/singletons/stderr.hpp"
#include "test/codemaker/cppumaker/singletons/stdin.hpp"
#include "test/codemaker/cppumaker/singletons/stdout.hpp"
#include "test/codemaker/cppumaker/singletons/CHAR_BIT.hpp"
#include "test/codemaker/cppumaker/singletons/CHAR_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/CHAR_MIN.hpp"
#include "test/codemaker/cppumaker/singletons/DBL_DIG.hpp"
#include "test/codemaker/cppumaker/singletons/DBL_EPSILON.hpp"
#include "test/codemaker/cppumaker/singletons/DBL_MANT_DIG.hpp"
#include "test/codemaker/cppumaker/singletons/DBL_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/DBL_MAX_10_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/DBL_MAX_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/DBL_MIN.hpp"
#include "test/codemaker/cppumaker/singletons/DBL_MIN_10_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/DBL_MIN_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_DIG.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_EPSILON.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_MANT_DIG.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_MAX_10_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_MAX_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_MIN.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_MIN_10_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_MIN_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_RADIX.hpp"
#include "test/codemaker/cppumaker/singletons/FLT_ROUNDS.hpp"
#include "test/codemaker/cppumaker/singletons/INT_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/INT_MIN.hpp"
#include "test/codemaker/cppumaker/singletons/LDBL_DIG.hpp"
#include "test/codemaker/cppumaker/singletons/LDBL_EPSILON.hpp"
#include "test/codemaker/cppumaker/singletons/LDBL_MANT_DIG.hpp"
#include "test/codemaker/cppumaker/singletons/LDBL_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/LDBL_MAX_10_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/LDBL_MAX_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/LDBL_MIN.hpp"
#include "test/codemaker/cppumaker/singletons/LDBL_MIN_10_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/LDBL_MIN_EXP.hpp"
#include "test/codemaker/cppumaker/singletons/LONG_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/LONG_MIN.hpp"
#include "test/codemaker/cppumaker/singletons/MB_LEN_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/SCHAR_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/SCHAR_MIN.hpp"
#include "test/codemaker/cppumaker/singletons/SHRT_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/SHRT_MIN.hpp"
#include "test/codemaker/cppumaker/singletons/UCHAR_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/UINT_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/ULONG_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/USHRT_MAX.hpp"
#include "test/codemaker/cppumaker/singletons/FILE.hpp"
#include "test/codemaker/cppumaker/singletons/lconv.hpp"
#include "test/codemaker/cppumaker/singletons/tm.hpp"
#include "test/codemaker/cppumaker/singletons/std.hpp"
#include "test/codemaker/cppumaker/singletons/NDEBUG.hpp"
#include "test/codemaker/cppumaker/singletons/get.hpp"
#include "test/codemaker/cppumaker/HelperEnum.hpp"
#include "test/codemaker/cppumaker/HelperStruct.hpp"
#include "test/codemaker/cppumaker/BigStruct.hpp"
#include "test/codemaker/cppumaker/Struct.hpp"
#include "test/codemaker/cppumaker/StructUsage.hpp"
#include "test/codemaker/cppumaker/AlignmentDerivedStruct.hpp"
#include "test/codemaker/cppumaker/TestException1.hpp"
#include "test/codemaker/cppumaker/TestException2.hpp"
#include "test/codemaker/cppumaker/Constants.hpp"

#include "boost/scoped_array.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include "rtl/ustring.hxx"

#include <cstddef>
#include <iostream>

namespace com { namespace sun { namespace star { namespace uno {

std::ostream & operator <<(
    std::ostream & out,
    SAL_UNUSED_PARAMETER com::sun::star::uno::Exception const &)
{
    return out << "<UNO exception>";
}

} } } }

namespace test { namespace codemaker { namespace cppumaker {

bool operator ==(
    test::codemaker::cppumaker::TestException1 const & e1,
    test::codemaker::cppumaker::TestException1 const & e2)
{
    return e1.Message == e2.Message && e1.Context == e2.Context
        && e1.m1 == e2.m1 && e1.m2 == e2.m2 && e1.m3 == e2.m3
        && e1.m4.member1 == e2.m4.member1 && e1.m4.member2 == e2.m4.member2;
}

} } }

namespace {

class Test: public CppUnit::TestFixture {
public:
    void testBigStruct();

    void testPolyStruct();

    void testExceptions();

    void testConstants();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testBigStruct);
    CPPUNIT_TEST(testPolyStruct);
    CPPUNIT_TEST(testExceptions);
    CPPUNIT_TEST(testConstants);
    CPPUNIT_TEST_SUITE_END();
};

struct Guard {
    explicit Guard(void * buffer):
        p(new(buffer) test::codemaker::cppumaker::BigStruct) {}

    ~Guard() { p->test::codemaker::cppumaker::BigStruct::~BigStruct(); }

    test::codemaker::cppumaker::BigStruct * const p;
};

void Test::testBigStruct() {
    // Default-initialize a BigStruct instance on top of a memory buffer filled
    // with random data, and make sure that all members are default-initialized:
    boost::scoped_array< char > buffer(
        new char[sizeof (test::codemaker::cppumaker::BigStruct)]);
    for (std::size_t i = 0; i < sizeof (test::codemaker::cppumaker::BigStruct);
         ++i)
    {
        buffer[i] = '\x56';
    }
    Guard guard(buffer.get());
    CPPUNIT_ASSERT_EQUAL(guard.p->m1, sal_False);
    CPPUNIT_ASSERT_EQUAL(guard.p->m2, static_cast< sal_Int8 >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m3, static_cast< sal_Int16 >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m4, static_cast< sal_uInt16 >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m5, static_cast< sal_Int32 >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m6, static_cast< sal_uInt32 >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m7, static_cast< sal_Int64 >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m8, static_cast< sal_uInt64 >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m9, 0.0f);
    CPPUNIT_ASSERT_EQUAL(guard.p->m10, 0.0);
    CPPUNIT_ASSERT_EQUAL(guard.p->m11, static_cast< sal_Unicode >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m12.getLength(), static_cast< sal_Int32 >(0));
    CPPUNIT_ASSERT_EQUAL(
        +guard.p->m13.getTypeClass(), +com::sun::star::uno::TypeClass_VOID);
    CPPUNIT_ASSERT_EQUAL(guard.p->m14.hasValue(), sal_False);
    CPPUNIT_ASSERT_EQUAL(guard.p->m15.getLength(), static_cast< sal_Int32 >(0));
    CPPUNIT_ASSERT_EQUAL(
        +guard.p->m16, +test::codemaker::cppumaker::HelperEnum_ZERO);
    CPPUNIT_ASSERT_EQUAL(guard.p->m17.m1, sal_False);
    CPPUNIT_ASSERT_EQUAL(guard.p->m17.m2.is(), sal_False);
    CPPUNIT_ASSERT_EQUAL(guard.p->m18.is(), sal_False);
    CPPUNIT_ASSERT_EQUAL(guard.p->m19, static_cast< sal_Int8 >(0));
    CPPUNIT_ASSERT_EQUAL(
        +guard.p->m20, +test::codemaker::cppumaker::HelperEnum_ZERO);
    CPPUNIT_ASSERT_EQUAL(guard.p->m21.getLength(), static_cast< sal_Int32 >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m22.getLength(), static_cast< sal_Int32 >(0));
    CPPUNIT_ASSERT_EQUAL(guard.p->m23.getLength(), static_cast< sal_Int32 >(0));

//This is a very platform specific test.
#if defined __GNUC__ && __GNUC__ >= 3 // see CPPU_GCC3_ALIGN
#if defined(LINUX) && (defined (X86_64) || defined(X86) || defined(PPC))
    CPPUNIT_ASSERT_EQUAL(
#if defined X86_64 || defined PPC
        static_cast< std::size_t >(24),
#else
        static_cast< std::size_t >(16),
#endif
        sizeof (test::codemaker::cppumaker::AlignmentDerivedStruct));
#endif
#endif

    com::sun::star::uno::Type t(
        cppu::UnoType< test::codemaker::cppumaker::BigStruct >::get());
    typelib_TypeDescription * td = NULL;
    t.getDescription(&td);
    typelib_typedescription_complete(&td);
    fprintf(stdout, "#### 1\n");
    CPPUNIT_ASSERT(td != NULL);
    CPPUNIT_ASSERT_EQUAL(+typelib_TypeClass_STRUCT, +td->eTypeClass);
    typelib_StructTypeDescription * std =
        reinterpret_cast< typelib_StructTypeDescription * >(td);
    CPPUNIT_ASSERT_EQUAL(+typelib_TypeClass_UNSIGNED_SHORT, +std->aBase.ppTypeRefs[3]->eTypeClass); // unsigned short m4;
    CPPUNIT_ASSERT_EQUAL(+typelib_TypeClass_CHAR, +std->aBase.ppTypeRefs[10]->eTypeClass); // char m11;
}

void Test::testPolyStruct() {
    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "test.codemaker.cppumaker.Struct<char,short>")),
        (com::sun::star::uno::makeAny(
            test::codemaker::cppumaker::Struct< sal_Unicode, sal_Int16 >()).
         getValueType().getTypeName()));

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >
        aEmptySequence;

    CPPUNIT_ASSERT_EQUAL(
        (test::codemaker::cppumaker::make_Struct< sal_uInt32, sal_Bool >(5,
            aEmptySequence).member1),
        static_cast< sal_uInt32 >(5));
}

void Test::testExceptions() {
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >
        aEmptySequence;

    test::codemaker::cppumaker::TestException1 e11(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")), 0, 1,
        com::sun::star::uno::makeAny(123.0),
        test::codemaker::cppumaker::HelperEnum_ONE,
        test::codemaker::cppumaker::Struct<sal_Int32, sal_Int32>(5, aEmptySequence), 2);
    test::codemaker::cppumaker::TestException1 e12(e11);
    CPPUNIT_ASSERT_EQUAL(e11, e12);
    test::codemaker::cppumaker::TestException1 e13;
    e13 = e11;
    CPPUNIT_ASSERT_EQUAL(e11, e13);
    test::codemaker::cppumaker::TestException2 e21(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")), 0, 1,
        com::sun::star::uno::makeAny(123.0),
        test::codemaker::cppumaker::HelperEnum_ONE,
        test::codemaker::cppumaker::Struct<sal_Int32, sal_Int32>(5, aEmptySequence), 2);
    test::codemaker::cppumaker::TestException2 e22(e21);
    CPPUNIT_ASSERT_EQUAL(e21, e22);
    test::codemaker::cppumaker::TestException2 e23;
    e23 = e21;
    CPPUNIT_ASSERT_EQUAL(e21, e23);
}

void Test::testConstants() {
    CPPUNIT_ASSERT_EQUAL(
        SAL_MIN_INT8, test::codemaker::cppumaker::Constants::byteMin);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MAX_INT8, test::codemaker::cppumaker::Constants::byteMax);
    CPPUNIT_ASSERT_EQUAL(
        static_cast< sal_Int8 >(-1),
        test::codemaker::cppumaker::Constants::byteNeg);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MIN_INT16, test::codemaker::cppumaker::Constants::shortMin);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MAX_INT16, test::codemaker::cppumaker::Constants::shortMax);
    CPPUNIT_ASSERT_EQUAL(
        static_cast< sal_uInt16 >(0),
        test::codemaker::cppumaker::Constants::unsignedShortMin);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MAX_UINT16,
        test::codemaker::cppumaker::Constants::unsignedShortMax);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MIN_INT32, test::codemaker::cppumaker::Constants::longMin);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MAX_INT32, test::codemaker::cppumaker::Constants::longMax);
    CPPUNIT_ASSERT_EQUAL(
        static_cast< sal_uInt32 >(0),
        test::codemaker::cppumaker::Constants::unsignedLongMin);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MAX_UINT32, test::codemaker::cppumaker::Constants::unsignedLongMax);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MIN_INT64, test::codemaker::cppumaker::Constants::hyperMin);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MAX_INT64, test::codemaker::cppumaker::Constants::hyperMax);
    CPPUNIT_ASSERT_EQUAL(
        static_cast< sal_uInt64 >(0),
        test::codemaker::cppumaker::Constants::unsignedHyperMin);
    CPPUNIT_ASSERT_EQUAL(
        SAL_MAX_UINT64,
        test::codemaker::cppumaker::Constants::unsignedHyperMax);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
