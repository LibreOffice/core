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

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#define _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_

#include "osl/interlck.h"
#include "sal/types.h"

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif
#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif

namespace salhelper {

/** A simple base implementation for reference-counted objects.

    Classes that want to implement a reference-counting mechanism based on the
    acquire()/release() interface should derive from this class.

    The reason to have class local operators new and delete here is technical.
    Imagine a class D derived from SimpleReferenceObject, but implemented in
    another shared library that happens to use different global operators new
    and delete from those used in this shared library (which, sadly, seems to
    be possible with shared libraries).  Now, without the class local
    operators new and delete here, a code sequence like "new D" would use the
    global operator new as found in the other shared library, while the code
    sequence "delete this" in release() would use the global operator delete
    as found in this shared library---and these two operators would not be
    guaranteed to match.

    There are no overloaded operators new and delete for placement new here,
    because it is felt that the concept of placement new does not work well
    with the concept of reference-counted objects; so it seems best to simply
    leave those operators out.

    The same problem as with operators new and delete would also be there with
    operators new[] and delete[].  But since arrays of reference-counted
    objects are of no use, anyway, it seems best to simply declare and not
    define (private) operators new[] and delete[].
 */
class SimpleReferenceObject
{
public:
    inline SimpleReferenceObject() SAL_THROW(()): m_nCount(0) {}

    /** @ATTENTION
        The results are undefined if, for any individual instance of
        SimpleReferenceObject, the total number of calls to acquire() exceeds
        the total number of calls to release() by a plattform dependent amount
        (which, hopefully, is quite large).
     */
    inline void acquire() SAL_THROW(())
    { osl_incrementInterlockedCount(&m_nCount); }

    inline void release() SAL_THROW(())
    { if (osl_decrementInterlockedCount(&m_nCount) == 0) delete this; }

    /** see general class documentation
     */
    static void * operator new(std::size_t nSize) SAL_THROW((std::bad_alloc));

    /** see general class documentation
     */
    static void * operator new(std::size_t nSize,
                               std::nothrow_t const & rNothrow)
        SAL_THROW(());

    /** see general class documentation
     */
    static void operator delete(void * pPtr) SAL_THROW(());

    /** see general class documentation
     */
    static void operator delete(void * pPtr, std::nothrow_t const & rNothrow)
        SAL_THROW(());

protected:
    virtual ~SimpleReferenceObject() SAL_THROW(());

private:
    oslInterlockedCount m_nCount;

    /** not implemented
        @internal
     */
    SimpleReferenceObject(SimpleReferenceObject &);

    /** not implemented
        @internal
     */
    void operator =(SimpleReferenceObject);

    /** not implemented (see general class documentation)
        @internal
     */
    static void * operator new[](std::size_t);

    /** not implemented (see general class documentation)
        @internal
     */
    static void operator delete[](void * pPtr);
};

}

#endif // _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
