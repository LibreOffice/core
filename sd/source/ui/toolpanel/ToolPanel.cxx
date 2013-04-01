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


#include "ToolPanel.hxx"
#include "MethodGuard.hxx"
#include <taskpane/TaskPaneTreeNode.hxx>

#include <com/sun/star/lang/DisposedException.hpp>

#include <vcl/window.hxx>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

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
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::accessibility::XAccessible;

    typedef MethodGuard< ToolPanel > ToolPanelGuard;

    //==================================================================================================================
    //= ToolPanel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ToolPanel::ToolPanel( ::std::auto_ptr< TreeNode >& i_rControl )
        :ToolPanel_Base( m_aMutex )
        ,m_pControl( i_rControl )
    {
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

    //------------------------------------------------------------------------------------------------------------------
    ToolPanel::~ToolPanel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanel::checkDisposed()
    {
        if ( m_pControl.get() == NULL )
            throw DisposedException( ::rtl::OUString(), *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XWindow > SAL_CALL ToolPanel::getWindow() throw (RuntimeException)
    {
        ToolPanelGuard aGuard( *this );
        return Reference< XWindow >( m_pControl->GetWindow()->GetComponentInterface(), UNO_QUERY_THROW );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL ToolPanel::createAccessible( const Reference< XAccessible >& i_rParentAccessible ) throw (RuntimeException)
    {
        ToolPanelGuard aGuard( *this );
        Reference< XAccessible > xAccessible( m_pControl->GetWindow()->GetAccessible( sal_False ) );
        if ( !xAccessible.is() )
        {
            xAccessible.set( m_pControl->CreateAccessibleObject( i_rParentAccessible ) );
            m_pControl->GetWindow()->SetAccessible( xAccessible );
        }
        return xAccessible;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL ToolPanel::disposing()
    {
        m_pControl.reset();
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
