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
#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_MSVC_WIN32_X86_64_MSCX_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_MSVC_WIN32_X86_64_MSCX_HXX

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <rtl/ustring.hxx>


class type_info;
typedef struct _uno_Any uno_Any;
typedef struct _uno_Mapping uno_Mapping;

namespace CPPU_CURRENT_NAMESPACE
{

const DWORD MSVC_ExceptionCode = 0xe06d7363;
const long MSVC_magic_number = 0x19930520L;

typedef enum { REGPARAM_INT, REGPARAM_FLT } RegParamKind;

type_info * mscx_getRTTI( OUString const & rUNOname );

int mscx_filterCppException(
    EXCEPTION_POINTERS * pPointers, uno_Any * pUnoExc, uno_Mapping * pCpp2Uno );

void mscx_raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

}

//TODO: Work around missing __CxxDetectRethrow in clang-cl for now (predefined
// in cl, <www.geoffchappell.com/studies/msvc/language/predefined/index.html>):
#if defined __clang__
extern "C" int __cdecl __CxxDetectRethrow(void *);
#endif

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
