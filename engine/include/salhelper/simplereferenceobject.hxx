/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#ifndef INCLUDED_SALHELPER_SIMPLEREFERENCEOBJECT_HXX
#define INCLUDED_SALHELPER_SIMPLEREFERENCEOBJECT_HXX

#include "osl/interlck.h"
#include "salhelper/salhelperdllapi.h"

#include <cstddef>
#include <new>

namespace salhelper {

/** A simple base implementation for reference-counted objects.

    Classes that want to implement a reference-counting mechanism based on the
    acquire()/release() interface should derive from this class.

    Arrays of reference-counted objects are of no use, so it seems best to simply
    define operators new[] and delete[] as deleted.
 */
class SALHELPER_DLLPUBLIC SimpleReferenceObject
{
    static const size_t nStaticFlag = 0x80000000;

public:
    SimpleReferenceObject(): m_nCount(0) {}

    /** @attention
        If, for any individual instance of SimpleReferenceObject, the total
        number of calls to acquire() exceeds the total number of calls to
        release() by 2^31 - the object will never subsequently be released.
     */
    void acquire()
    {
        if (!(m_nCount & nStaticFlag))
            osl_atomic_increment(&m_nCount);
    }

    void release()
    {
        if (!(m_nCount & nStaticFlag) &&
            osl_atomic_decrement(&m_nCount) == 0)
            delete this;
    }

protected:
    virtual ~SimpleReferenceObject() COVERITY_NOEXCEPT_FALSE;

    /** mark reference count as not to be touched, and the
     * related object as having an indefinite lifespan.
     * NB. do not use if you have a non-empty destructor.
     */
    void staticize()
    {
        m_nCount |= nStaticFlag;
    }

    oslInterlockedCount m_nCount;

private:
    /** not implemented
     */
    SimpleReferenceObject(SimpleReferenceObject &) SAL_DELETED_FUNCTION;

    /** not implemented
     */
    void operator =(SimpleReferenceObject) SAL_DELETED_FUNCTION;

    /** see general class documentation
     */
    static void * operator new[](std::size_t) SAL_DELETED_FUNCTION;

    /** see general class documentation
     */
    static void operator delete[](void * pPtr) SAL_DELETED_FUNCTION;
};

}

#endif // INCLUDED_SALHELPER_SIMPLEREFERENCEOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
