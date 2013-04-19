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

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#define _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_

#include "osl/interlck.h"
#include "sal/types.h"
#include "salhelperdllapi.h"

#include <cstddef>
#include <new>

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
class SALHELPER_DLLPUBLIC SimpleReferenceObject
{
public:
    inline SimpleReferenceObject() SAL_THROW(()): m_nCount(0) {}

    /** @attention
        The results are undefined if, for any individual instance of
        SimpleReferenceObject, the total number of calls to acquire() exceeds
        the total number of calls to release() by a platform dependent amount
        (which, hopefully, is quite large).
     */
    inline void acquire() SAL_THROW(())
    { osl_atomic_increment(&m_nCount); }

    inline void release() SAL_THROW(())
    { if (osl_atomic_decrement(&m_nCount) == 0) delete this; }

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
     */
    SALHELPER_DLLPRIVATE SimpleReferenceObject(SimpleReferenceObject &);

    /** not implemented
     */
    SALHELPER_DLLPRIVATE void operator =(SimpleReferenceObject);

    /// @cond INTERNAL

#ifdef _MSC_VER
/* We can't now have these private with MSVC2008 at least, it leads to
   compilation errors in xmloff and other places.
*/
protected:
#endif
    /** not implemented (see general class documentation)
     */
    static void * operator new[](std::size_t);

    /** not implemented (see general class documentation)
     */
    static void operator delete[](void * pPtr);

    /// @endcond
};

}

#endif // _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
