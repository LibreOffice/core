/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SD_SIMPLEREFERENCECOMPONENT_HXX_
#define _SD_SIMPLEREFERENCECOMPONENT_HXX_

#include "osl/interlck.h"
#include "sal/types.h"

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
        the total number of calls to release() by a plattform dependent amount
        (which, hopefully, is quite large).
     */
    SD_DLLPUBLIC void acquire();
    SD_DLLPUBLIC void release();

    void Dispose();

    bool isDisposed() const { return mbDisposed; }

    /** see general class documentation
     */
    static void * operator new(std::size_t nSize) SAL_THROW((std::bad_alloc));

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

    /** not implemented
        @internal
     */
    SimpleReferenceComponent(SimpleReferenceComponent &);

    /** not implemented
        @internal
     */
    void operator =(SimpleReferenceComponent);

    /** not implemented (see general class documentation)
        @internal
     */
    static void * operator new[](std::size_t);

    /** not implemented (see general class documentation)
        @internal
     */
    static void operator delete[](void * pPtr);

    bool mbDisposed;
};

}

#endif // _SALHELPER_SimpleReferenceComponent_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
