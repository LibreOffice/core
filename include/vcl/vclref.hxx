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

#ifndef INCLUDED_VCL_REFERENCE_HXX
#define INCLUDED_VCL_REFERENCE_HXX

#include <rtl/ref.hxx>

/**
 * A thin wrapper around rtl::Reference to implement the acquire and dispose semantics we want for references to vcl::Window subclasses.
 * @param reference_type must be a subclass of vcl::Window
 */
template <class reference_type>
class VclReference
{

    ::rtl::Reference<reference_type> m_rInnerRef;

public:
    /** Constructor...
     */
    inline VclReference()
        : m_rInnerRef()
    {}


    /** Constructor...
     */
    inline VclReference (reference_type * pBody)
        : m_rInnerRef(pBody)
    {}


    /** Copy constructor...
     */
    inline VclReference (const VclReference<reference_type> & handle)
        : m_rInnerRef (handle)
    {}


    /** Get the body. Can be used instead of operator->().
         I.e. handle->someBodyOp() and handle.get()->someBodyOp()
         are the same.
     */
    inline reference_type * SAL_CALL get() const
    {
        return m_rInnerRef.get();
    }

    /** Probably most common used: handle->someBodyOp().
     */
    inline reference_type * SAL_CALL operator->() const
    {
        return m_rInnerRef.get();
    }

    inline void disposeAndClear()
    {
        // hold it alive for the lifetime of this method
        ::rtl::Reference<reference_type> aTmp(m_rInnerRef);
        m_rInnerRef.clear(); // we should use some 'swap' method ideally ;-)
        if (aTmp.get())
            aTmp->dispose();
        // ~Window will get called as we exit this method if aTmp holds the last reference to the Window object
    }

}; // class VclReference

#endif // INCLUDED_VCL_REFERENCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
