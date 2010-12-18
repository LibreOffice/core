/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#include "precompiled_dbaccess.hxx"

#include "subcomponentloader.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/frame/XController2.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//........................................................................
namespace dbaccess
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
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::awt::WindowEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::ucb::Command;
    using ::com::sun::star::ucb::XCommandProcessor;
    using ::com::sun::star::frame::XController2;
    using ::com::sun::star::lang::XComponent;
    /** === end UNO using === **/

    //====================================================================
    //= SubComponentLoader
    //====================================================================
    struct DBACCESS_DLLPRIVATE SubComponentLoader_Data
    {
        const Reference< XCommandProcessor >    xDocDefCommands;
        const Reference< XComponent >           xNonDocComponent;
        Reference< XWindow >                    xAppComponentWindow;

        SubComponentLoader_Data( const Reference< XCommandProcessor >& i_rDocumentDefinition )
            :xDocDefCommands( i_rDocumentDefinition, UNO_SET_THROW )
            ,xNonDocComponent()
        {
        }

        SubComponentLoader_Data( const Reference< XComponent >& i_rNonDocumentComponent )
            :xDocDefCommands()
            ,xNonDocComponent( i_rNonDocumentComponent, UNO_SET_THROW )
        {
        }
    };

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        //................................................................
        void lcl_onWindowShown_nothrow( const SubComponentLoader_Data& i_rData )
        {
            try
            {
                if ( i_rData.xDocDefCommands.is() )
                {
                    Command aCommandOpen;
                    aCommandOpen.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "show" ) );

                    const sal_Int32 nCommandIdentifier = i_rData.xDocDefCommands->createCommandIdentifier();
                    i_rData.xDocDefCommands->execute( aCommandOpen, nCommandIdentifier, NULL );
                }
                else
                {
                    const Reference< XController > xController( i_rData.xNonDocComponent, UNO_QUERY_THROW );
                    const Reference< XFrame > xFrame( xController->getFrame(), UNO_SET_THROW );
                    const Reference< XWindow > xWindow( xFrame->getContainerWindow(), UNO_SET_THROW );
                    xWindow->setVisible( sal_True );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    //====================================================================
    //= SubComponentLoader
    //====================================================================
    //--------------------------------------------------------------------
    SubComponentLoader::SubComponentLoader( const Reference< XController >& i_rApplicationController,
            const Reference< XCommandProcessor >& i_rSubDocumentDefinition )
        :m_pData( new SubComponentLoader_Data( i_rSubDocumentDefinition ) )
    {
        // add as window listener to the controller's container window, so we get notified when it is shown
        Reference< XController2 > xController( i_rApplicationController, UNO_QUERY_THROW );
        m_pData->xAppComponentWindow.set( xController->getComponentWindow(), UNO_SET_THROW );

        osl_incrementInterlockedCount( &m_refCount );
        {
            m_pData->xAppComponentWindow->addWindowListener( this );
        }
        osl_decrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    SubComponentLoader::SubComponentLoader( const Reference< XController >& i_rApplicationController,
            const Reference< XComponent >& i_rNonDocumentComponent )
        :m_pData( new SubComponentLoader_Data( i_rNonDocumentComponent ) )
    {
        // add as window listener to the controller's container window, so we get notified when it is shown
        Reference< XController2 > xController( i_rApplicationController, UNO_QUERY_THROW );
        m_pData->xAppComponentWindow.set( xController->getComponentWindow(), UNO_SET_THROW );

        osl_incrementInterlockedCount( &m_refCount );
        {
            m_pData->xAppComponentWindow->addWindowListener( this );
        }
        osl_decrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    SubComponentLoader::~SubComponentLoader()
    {
        delete m_pData, m_pData = NULL;
    }

    //--------------------------------------------------------------------
    void SAL_CALL SubComponentLoader::windowResized( const WindowEvent& i_rEvent ) throw (RuntimeException)
    {
        // not interested in
        (void)i_rEvent;
    }

    //--------------------------------------------------------------------
    void SAL_CALL SubComponentLoader::windowMoved( const WindowEvent& i_rEvent ) throw (RuntimeException)
    {
        // not interested in
        (void)i_rEvent;
    }

    //--------------------------------------------------------------------
    void SAL_CALL SubComponentLoader::windowShown( const EventObject& i_rEvent ) throw (RuntimeException)
    {
        (void)i_rEvent;

        lcl_onWindowShown_nothrow( *m_pData );
        m_pData->xAppComponentWindow->removeWindowListener( this );
    }

    //--------------------------------------------------------------------
    void SAL_CALL SubComponentLoader::windowHidden( const EventObject& i_rEvent ) throw (RuntimeException)
    {
        // not interested in
        (void)i_rEvent;
    }

    //--------------------------------------------------------------------
    void SAL_CALL SubComponentLoader::disposing( const EventObject& i_rEvent ) throw (RuntimeException)
    {
        // not interested in
        (void)i_rEvent;
    }

//........................................................................
} // namespace dbaccess
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
