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

#ifndef INCLUDED_SD_INC_HELPER_SIMPLEREFERENCECOMPONENT_HXX
#define INCLUDED_SD_INC_HELPER_SIMPLEREFERENCECOMPONENT_HXX

#include "osl/interlck.h"

#include <cstddef>
#include <new>

#include <sddllapi.h>

namespace sd {

/** A simple base implementation for reference-counted components.
    acts like sal::SimpleReferenceObject but calls the virtual disposing()
    methods before the ref count switches from 1 to zero.
 */
class SimpleReferenceComponent
{
public:
    SimpleReferenceComponent();

    /** @ATTENTION
        The results are undefined if, for any individual instance of
        SimpleReferenceComponent, the total number of calls to acquire() exceeds
        the total number of calls to release() by a platform dependent amount
        (which, hopefully, is quite large).
     */
    SD_DLLPUBLIC void acquire();
    SD_DLLPUBLIC void release();

    void Dispose();

    bool isDisposed() const { return mbDisposed; }

    /** see general class documentation
     */
    static void * operator new(std::size_t nSize);

    /** see general class documentation
     */
    static void * operator new(std::size_t nSize,
                               std::nothrow_t const & rNothrow)
       ;

    /** see general class documentation
     */
    static void operator delete(void * pPtr);

    /** see general class documentation
     */
    static void operator delete(void * pPtr, std::nothrow_t const & rNothrow)
       ;

protected:
    virtual void disposing();

    virtual ~SimpleReferenceComponent();

private:
    oslInterlockedCount m_nCount;

    SimpleReferenceComponent(SimpleReferenceComponent &) = delete;

    void operator =(SimpleReferenceComponent) = delete;

    static void * operator new[](std::size_t) = delete;

    static void operator delete[](void * pPtr) = delete;

    bool mbDisposed;
};

}

#endif // _SALHELPER_SimpleReferenceComponent_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
