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

#include <msvc/except.hxx>

#pragma pack(push, 8)

struct ObjectFunction final
{
    char somecode[12];
    typelib_TypeDescription* _pTypeDescr; // type of object

    inline static void* operator new(size_t nSize);
    inline static void operator delete(void* pMem);

    ObjectFunction(typelib_TypeDescription* pTypeDescr, void* fpFunc) throw();
    ~ObjectFunction() throw();
};

struct ExceptionType final
{
    sal_Int32 _n0;
    type_info* _pTypeInfo;
    sal_Int32 _n1, _n2, _n3, _n4;
    ObjectFunction* _pCopyCtor;
    sal_Int32 _n5;

    explicit ExceptionType(typelib_TypeDescription* pTypeDescr) throw();
    ~ExceptionType() throw();

    // Copy assignment is forbidden and not implemented.
    ExceptionType(const ExceptionType&) = delete;
    ExceptionType& operator=(const ExceptionType&) = delete;
};

struct RaiseInfo final
{
    sal_Int32 _n0;
    ObjectFunction* _pDtor;
    sal_Int32 _n2;
    void* _types;
    sal_Int32 _n3, _n4;

    explicit RaiseInfo(typelib_TypeDescription* pTypeDescr) throw();
    ~RaiseInfo() throw();
};

#pragma pack(pop)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
