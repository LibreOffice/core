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

#include <sal/config.h>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/accessiblekeybindinghelper.hxx>


namespace comphelper
{


    using namespace ::com::sun::star; // MT 04/2003: was ::drafts::com::sun::star - otherwise too many changes
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;


    // OAccessibleKeyBindingHelper


    OAccessibleKeyBindingHelper::OAccessibleKeyBindingHelper()
    {
    }


    OAccessibleKeyBindingHelper::OAccessibleKeyBindingHelper( const OAccessibleKeyBindingHelper& rHelper )
        : cppu::WeakImplHelper<XAccessibleKeyBinding>( rHelper )
        , m_aKeyBindings( rHelper.m_aKeyBindings )
    {
    }


    OAccessibleKeyBindingHelper::~OAccessibleKeyBindingHelper()
    {
    }


    void OAccessibleKeyBindingHelper::AddKeyBinding( const Sequence< awt::KeyStroke >& rKeyBinding )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_aKeyBindings.push_back( rKeyBinding );
    }


    void OAccessibleKeyBindingHelper::AddKeyBinding( const awt::KeyStroke& rKeyStroke )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Sequence< awt::KeyStroke > aSeq(1);
        aSeq[0] = rKeyStroke;
        m_aKeyBindings.push_back( aSeq );
    }


    // XAccessibleKeyBinding


    sal_Int32 OAccessibleKeyBindingHelper::getAccessibleKeyBindingCount()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return m_aKeyBindings.size();
    }


    Sequence< awt::KeyStroke > OAccessibleKeyBindingHelper::getAccessibleKeyBinding( sal_Int32 nIndex )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( nIndex < 0 || nIndex >= static_cast<sal_Int32>(m_aKeyBindings.size()) )
            throw IndexOutOfBoundsException();

        return m_aKeyBindings[nIndex];
    }


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
