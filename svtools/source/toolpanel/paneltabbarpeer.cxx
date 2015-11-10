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


#include "paneltabbarpeer.hxx"
#include <svtools/toolpanel/paneltabbar.hxx>

#include <com/sun/star/lang/DisposedException.hpp>

#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>


namespace svt
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::accessibility::XAccessibleContext;
    using ::com::sun::star::lang::DisposedException;


    //= PanelTabBarPeer


    PanelTabBarPeer::PanelTabBarPeer( PanelTabBar& i_rTabBar )
        :VCLXWindow()
        ,m_pTabBar( &i_rTabBar )
    {
    }


    PanelTabBarPeer::~PanelTabBarPeer()
    {
    }


    Reference< XAccessibleContext > PanelTabBarPeer::CreateAccessibleContext()
    {
        SolarMutexGuard aSolarGuard;
        if ( m_pTabBar == nullptr )
            throw DisposedException( OUString(), *this );



        vcl::Window* pAccessibleParent( m_pTabBar->GetAccessibleParentWindow() );
        ENSURE_OR_RETURN( pAccessibleParent != nullptr, "no accessible parent => no accessible context", nullptr );
        Reference< XAccessible > xAccessibleParent( pAccessibleParent->GetAccessible(), UNO_SET_THROW );
        return m_aAccessibleFactory.getFactory().createAccessibleToolPanelTabBar( xAccessibleParent, m_pTabBar->GetPanelDeck(), *m_pTabBar );
    }


    void SAL_CALL PanelTabBarPeer::dispose() throw(RuntimeException, std::exception)
    {
        {
            SolarMutexGuard aSolarGuard;
            m_pTabBar.clear();
        }
        VCLXWindow::dispose();
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
