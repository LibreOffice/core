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

#include "precompiled_svtools.hxx"

#include "toolpaneldeckpeer.hxx"
#include "svtools/toolpanel/toolpaneldeck.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace svt
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
    using ::com::sun::star::accessibility::XAccessibleContext;
    using ::com::sun::star::lang::DisposedException;
    /** === end UNO using === **/

    //==================================================================================================================
    //= ToolPanelDeckPeer
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDeckPeer::ToolPanelDeckPeer( ToolPanelDeck& i_rDeck )
        :VCLXWindow()
        ,m_pDeck( &i_rDeck )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDeckPeer::~ToolPanelDeckPeer()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessibleContext > ToolPanelDeckPeer::CreateAccessibleContext()
    {
        ::vos::OGuard aSolarGuard( GetMutex() );
        if ( m_pDeck == NULL )
            throw DisposedException( ::rtl::OUString(), *this );

        Window* pAccessibleParent( m_pDeck->GetAccessibleParentWindow() );
        ENSURE_OR_RETURN( pAccessibleParent != NULL, "no accessible parent => no accessible context", NULL );
        Reference< XAccessible > xAccessibleParent( pAccessibleParent->GetAccessible(), UNO_SET_THROW );
        return m_aAccessibleFactory.getFactory().createAccessibleToolPanelDeck( xAccessibleParent, *m_pDeck );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL ToolPanelDeckPeer::dispose() throw(RuntimeException)
    {
        {
            ::vos::OGuard aSolarGuard( GetMutex() );
            m_pDeck = NULL;
        }
        VCLXWindow::dispose();
    }

//......................................................................................................................
} // namespace svt
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
