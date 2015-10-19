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

#ifndef INCLUDED_CONNECTIVITY_OSUBCOMPONENT_HXX
#define INCLUDED_CONNECTIVITY_OSUBCOMPONENT_HXX

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <connectivity/dbtoolsdllapi.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace lang
            {
                class XComponent;
            }
        }
    }
}
namespace connectivity
{
    OOO_DLLPUBLIC_DBTOOLS
    void release(oslInterlockedCount& _refCount,
                 ::cppu::OBroadcastHelper& rBHelper,
                 css::uno::Reference< css::uno::XInterface >& _xInterface,
                 css::lang::XComponent* _pObject);

    // OSubComponent

    template <class SELF, class WEAK> class OSubComponent
    {
    protected:
        // the parent must support the tunnel implementation
        css::uno::Reference< css::uno::XInterface > m_xParent;
        SELF*   m_pDerivedImplementation;

    public:
        OSubComponent(
                const css::uno::Reference< css::uno::XInterface >& _xParent,
                SELF* _pDerivedImplementation)
            :m_xParent(_xParent)
            ,m_pDerivedImplementation(_pDerivedImplementation)
        {
        }

    protected:
        void dispose_ChildImpl()
        {
            // avoid ambiguity
            ::osl::MutexGuard aGuard( m_pDerivedImplementation->WEAK::rBHelper.rMutex );
        m_xParent.clear();
        }
        void relase_ChildImpl()
        {
            ::connectivity::release(m_pDerivedImplementation->m_refCount,
                                    m_pDerivedImplementation->WEAK::rBHelper,
                                    m_xParent,
                                    m_pDerivedImplementation);

            m_pDerivedImplementation->WEAK::release();
        }
    };
}

#endif // INCLUDED_CONNECTIVITY_OSUBCOMPONENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
