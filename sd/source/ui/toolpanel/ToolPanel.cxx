/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#include "precompiled_sd.hxx"

#include "ToolPanel.hxx"
#include "MethodGuard.hxx"
#include <taskpane/TaskPaneTreeNode.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <vcl/window.hxx>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    /** === begin UNO using === **/
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
    /** === end UNO using === **/

    typedef MethodGuard< ToolPanel > ToolPanelGuard;

    //==================================================================================================================
    //= ToolPanel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanel::ToolPanel( ::std::auto_ptr< TreeNode >& i_rControl )
        :ToolPanel_Base( m_aMutex )
        ,m_pControl( i_rControl )
    {
    }

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
