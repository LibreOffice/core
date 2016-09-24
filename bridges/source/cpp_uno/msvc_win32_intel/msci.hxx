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
#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_MSVC_WIN32_INTEL_MSCI_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_MSVC_WIN32_INTEL_MSCI_HXX

#pragma warning(push, 1)
#include <windows.h>
#pragma warning(pop)

#include "rtl/ustring.hxx"


class type_info;
typedef struct _uno_Any uno_Any;
typedef struct _uno_Mapping uno_Mapping;

namespace CPPU_CURRENT_NAMESPACE
{

const DWORD MSVC_ExceptionCode = 0xe06d7363;
const long MSVC_magic_number = 0x19930520L;

type_info * msci_getRTTI( OUString const & rUNOname );

int msci_filterCppException(
    EXCEPTION_POINTERS * pPointers, uno_Any * pUnoExc, uno_Mapping * pCpp2Uno );

void msci_raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
