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

#ifndef INCLUDED_WRITERFILTER_NotSoSimpleReferenceObject_HXX
#define INCLUDED_WRITERFILTER_NotSoSimpleReferenceObject_HXX

#include <osl/interlck.h>
#include <cstddef>

namespace writerfilter {

/** Cut'n'paste of salhelper::SimpleReferenceObject, because sberg thinks that it's spooky that
we add copy construction capabilities, so lets make our own copy for a while until we have
some consensus, rather than modifying a class that visible to all of LO and also via the UDK.
 */
class NotSoSimpleReferenceObject
{
public:
    NotSoSimpleReferenceObject(): m_nCount(0) {}

    /** These are only here so that subclasses get the default implicits */

    NotSoSimpleReferenceObject(NotSoSimpleReferenceObject const &) : m_nCount(0) {}
    NotSoSimpleReferenceObject(NotSoSimpleReferenceObject&&) : m_nCount(0) {}
    NotSoSimpleReferenceObject& operator=(NotSoSimpleReferenceObject const &) { return *this; }
    NotSoSimpleReferenceObject& operator=(NotSoSimpleReferenceObject &&) { return *this; }

    /** @attention
        The results are undefined if, for any individual instance of
        NotSoSimpleReferenceObject, the total number of calls to acquire() exceeds
        the total number of calls to release() by a platform dependent amount
        (which, hopefully, is quite large).
     */
    void acquire()
    { osl_atomic_increment(&m_nCount); }

    void release()
    { if (osl_atomic_decrement(&m_nCount) == 0) delete this; }

protected:
    virtual ~NotSoSimpleReferenceObject() COVERITY_NOEXCEPT_FALSE
    { assert(m_nCount == 0); }

private:
    oslInterlockedCount m_nCount;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
