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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include "resettable.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <cppuhelper/weak.hxx>

//........................................................................
namespace frm
{
//........................................................................

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
    using ::com::sun::star::form::XResetListener;
    using ::com::sun::star::lang::EventObject;
    /** === end UNO using === **/

    //====================================================================
    //= ResetHelper
    //====================================================================
    //--------------------------------------------------------------------
    void ResetHelper::addResetListener( const Reference< XResetListener >& _listener )
    {
        m_aResetListeners.addInterface( _listener );
    }

    //--------------------------------------------------------------------
    void ResetHelper::removeResetListener( const Reference< XResetListener >& _listener )
    {
        m_aResetListeners.removeInterface( _listener );
    }

    //--------------------------------------------------------------------
    bool ResetHelper::approveReset()
    {
        ::cppu::OInterfaceIteratorHelper aIter( m_aResetListeners );
        EventObject aResetEvent( m_rParent );

        sal_Bool bContinue = sal_True;
        while ( aIter.hasMoreElements() && bContinue )
            bContinue = static_cast< XResetListener* >( aIter.next() )->approveReset( aResetEvent );

        return bContinue;
    }

    //--------------------------------------------------------------------
    void ResetHelper::notifyResetted()
    {
        EventObject aResetEvent( m_rParent );
        m_aResetListeners.notifyEach( &XResetListener::resetted, aResetEvent );
    }

    //--------------------------------------------------------------------
    void ResetHelper::disposing()
    {
        EventObject aEvent( m_rParent );
        m_aResetListeners.disposeAndClear( aEvent );
    }

//........................................................................
} // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
