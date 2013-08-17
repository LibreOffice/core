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

#include "subcomponentloader.hxx"

#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/frame/XController2.hpp>

#include <tools/diagnose_ex.h>

namespace dbaccess
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
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::awt::WindowEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::ucb::Command;
    using ::com::sun::star::ucb::XCommandProcessor;
    using ::com::sun::star::frame::XController2;
    using ::com::sun::star::lang::XComponent;

    // SubComponentLoader
    struct DBACCESS_DLLPRIVATE SubComponentLoader_Data
    {
        const Reference< XCommandProcessor >    xDocDefCommands;
        const Reference< XComponent >           xNonDocComponent;
        Reference< XWindow >                    xAppComponentWindow;

        SubComponentLoader_Data( const Reference< XCommandProcessor >& i_rDocumentDefinition )
            :xDocDefCommands( i_rDocumentDefinition )
            ,xNonDocComponent()
        {
        }

        SubComponentLoader_Data( const Reference< XComponent >& i_rNonDocumentComponent )
            :xDocDefCommands()
            ,xNonDocComponent( i_rNonDocumentComponent )
        {
        }
    };

    // helper
    namespace
    {
        void lcl_onWindowShown_nothrow( const SubComponentLoader_Data& i_rData )
        {
            try
            {
                if ( i_rData.xDocDefCommands.is() )
                {
                    Command aCommandOpen;
                    aCommandOpen.Name = "show";

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

    // SubComponentLoader
    SubComponentLoader::SubComponentLoader( const Reference< XController >& i_rApplicationController,
            const Reference< XCommandProcessor >& i_rSubDocumentDefinition )
        :m_pData( new SubComponentLoader_Data( i_rSubDocumentDefinition ) )
    {
        // add as window listener to the controller's container window, so we get notified when it is shown
        Reference< XController2 > xController( i_rApplicationController, UNO_QUERY_THROW );
        m_pData->xAppComponentWindow.set( xController->getComponentWindow(), UNO_SET_THROW );

        osl_atomic_increment( &m_refCount );
        {
            m_pData->xAppComponentWindow->addWindowListener( this );
        }
        osl_atomic_decrement( &m_refCount );
    }

    SubComponentLoader::SubComponentLoader( const Reference< XController >& i_rApplicationController,
            const Reference< XComponent >& i_rNonDocumentComponent )
        :m_pData( new SubComponentLoader_Data( i_rNonDocumentComponent ) )
    {
        // add as window listener to the controller's container window, so we get notified when it is shown
        Reference< XController2 > xController( i_rApplicationController, UNO_QUERY_THROW );
        m_pData->xAppComponentWindow.set( xController->getComponentWindow(), UNO_SET_THROW );

        osl_atomic_increment( &m_refCount );
        {
            m_pData->xAppComponentWindow->addWindowListener( this );
        }
        osl_atomic_decrement( &m_refCount );
    }

    SubComponentLoader::~SubComponentLoader()
    {
        delete m_pData, m_pData = NULL;
    }

    void SAL_CALL SubComponentLoader::windowResized( const WindowEvent& i_rEvent ) throw (RuntimeException)
    {
        // not interested in
        (void)i_rEvent;
    }

    void SAL_CALL SubComponentLoader::windowMoved( const WindowEvent& i_rEvent ) throw (RuntimeException)
    {
        // not interested in
        (void)i_rEvent;
    }

    void SAL_CALL SubComponentLoader::windowShown( const EventObject& i_rEvent ) throw (RuntimeException)
    {
        (void)i_rEvent;

        lcl_onWindowShown_nothrow( *m_pData );
        m_pData->xAppComponentWindow->removeWindowListener( this );
    }

    void SAL_CALL SubComponentLoader::windowHidden( const EventObject& i_rEvent ) throw (RuntimeException)
    {
        // not interested in
        (void)i_rEvent;
    }

    void SAL_CALL SubComponentLoader::disposing( const EventObject& i_rEvent ) throw (RuntimeException)
    {
        // not interested in
        (void)i_rEvent;
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
