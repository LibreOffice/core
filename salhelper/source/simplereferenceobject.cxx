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

#include <salhelper/simplereferenceobject.hxx>

#include <cassert>
#include <new>

using salhelper::SimpleReferenceObject;

SimpleReferenceObject::~SimpleReferenceObject()
{
    assert(m_nCount == 0);
}

void *SimpleReferenceObject::operator new(std::size_t nSize)
{
    return ::operator new(nSize);
}

void *SimpleReferenceObject::operator new(std::size_t nSize,
                                           std::nothrow_t const &)
{
#if defined(_WIN32)
    return ::operator new(nSize);
        // WNT lacks a global nothrow operator new...
#else // WNT
    return ::operator new(nSize, std::nothrow);
#endif // WNT
}

void SimpleReferenceObject::operator delete(void * pPtr)
{
    ::operator delete(pPtr);
}

void SimpleReferenceObject::operator delete(void * pPtr, std::nothrow_t const &)
{
#if defined(_WIN32)
    ::operator delete(pPtr); // WNT lacks a global nothrow operator delete...
#else // WNT
    ::operator delete(pPtr, std::nothrow);
#endif // WNT
}

#ifdef _MSC_VER

/* This operator is supposed to be unimplemented, but that now leads
 * to compilation and/or linking errors with MSVC2008. Problem still
 * there with MSVC2013. As it can be left unimplemented just fine with
 * other compilers, presumably it is never called. So do implement it
 * then to avoid the compilation and/or linking errors, but make it
 * crash intentionally if called.
 */
void SimpleReferenceObject::operator delete[](void * /* pPtr */)
{
    abort();
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
