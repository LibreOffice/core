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
#include "precompiled_extensions.hxx"

#include "propertycontrolextender.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/awt/KeyFunction.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//........................................................................
namespace pcr
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
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::awt::KeyEvent;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::inspection::XPropertyControlContext;
    /** === end UNO using === **/
    namespace KeyFunction = ::com::sun::star::awt::KeyFunction;

    //====================================================================
    //= PropertyControlExtender_Data
    //====================================================================
    struct PropertyControlExtender_Data
    {
        Reference< XPropertyControl >   xControl;
        Reference< XWindow >            xControlWindow;
    };

    //====================================================================
    //= PropertyControlExtender
    //====================================================================
    //--------------------------------------------------------------------
    PropertyControlExtender::PropertyControlExtender( const Reference< XPropertyControl >& _rxObservedControl )
        :m_pData( new PropertyControlExtender_Data )
    {
        try
        {
            m_pData->xControl.set( _rxObservedControl, UNO_SET_THROW );
            m_pData->xControlWindow.set( m_pData->xControl->getControlWindow(), UNO_SET_THROW );
            m_pData->xControlWindow->addKeyListener( this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    PropertyControlExtender::~PropertyControlExtender()
    {
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyControlExtender::keyPressed( const KeyEvent& _event ) throw (RuntimeException)
    {
        OSL_ENSURE( _event.Source == m_pData->xControlWindow, "PropertyControlExtender::keyPressed: where does this come from?" );
        if  (   ( _event.KeyFunc == KeyFunction::DELETE )
            &&  ( _event.Modifiers == 0 )
            )
        {
            try
            {
                Reference< XPropertyControl > xControl( m_pData->xControl, UNO_SET_THROW );

                // reset the value
                xControl->setValue( Any() );

                // and notify the change
                // don't use XPropertyControl::notifyModifiedValue. It only notifies when the control content
                // is recognized as being modified by the user, which is not the case, since we just modified
                // it programmatically.
                Reference< XPropertyControlContext > xControlContext( xControl->getControlContext(), UNO_SET_THROW );
                xControlContext->valueChanged( xControl );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyControlExtender::keyReleased( const KeyEvent& /*_event*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyControlExtender::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        OSL_ENSURE( Source.Source == m_pData->xControlWindow, "PropertyControlExtender::disposing: where does this come from?" );
        (void)Source.Source;
        m_pData->xControlWindow.clear();
        m_pData->xControl.clear();
    }


//........................................................................
} // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
