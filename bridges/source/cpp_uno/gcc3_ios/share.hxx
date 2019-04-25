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
#pragma once

#include "uno/mapping.h"

#include <typeinfo>
#include <exception>
#include <cstddef>

// from opensource.apple.com: libcppabi-24.4/include/rtti.h
#include "rtti.h"

// from opensource.apple.com: libcppabi-24.4/include/unwind-cxx.h
#include "unwind-cxx.h"

// Import the __cxxabiv1 a.k.a. "abi" namespace
using namespace abi;

namespace CPPU_CURRENT_NAMESPACE
{
    void dummy_can_throw_anything( char const * );

    void raiseException(
        uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

    void fillUnoException(uno_Any *, uno_Mapping * pCpp2Uno);

    bool isSimpleReturnType(typelib_TypeDescription * pTD, bool recursive = false);
}

namespace arm
{
    enum armlimits {
        MAX_GPR_REGS = 8,
        MAX_FPR_REGS = 8
    };
    bool return_in_x8( typelib_TypeDescriptionReference *pTypeRef );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
