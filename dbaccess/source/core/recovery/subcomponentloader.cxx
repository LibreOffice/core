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
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
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
    SubComponentLoader::SubComponentLoader( const Reference< XController >& i_rApplicationController,
            const Reference< XCommandProcessor >& i_rSubDocumentDefinition )
        :mxDocDefCommands( i_rSubDocumentDefinition )
    {
        // add as window listener to the controller's container window, so we get notified when it is shown
        Reference< XController2 > xController( i_rApplicationController, UNO_QUERY_THROW );
        mxAppComponentWindow.set( xController->getComponentWindow(), UNO_SET_THROW );

        osl_atomic_increment( &m_refCount );
        {
            mxAppComponentWindow->addWindowListener( this );
        }
        osl_atomic_decrement( &m_refCount );
    }

    SubComponentLoader::SubComponentLoader( const Reference< XController >& i_rApplicationController,
            const Reference< XComponent >& i_rNonDocumentComponent )
        :mxNonDocComponent( i_rNonDocumentComponent )
    {
        // add as window listener to the controller's container window, so we get notified when it is shown
        Reference< XController2 > xController( i_rApplicationController, UNO_QUERY_THROW );
        mxAppComponentWindow.set( xController->getComponentWindow(), UNO_SET_THROW );

        osl_atomic_increment( &m_refCount );
        {
            mxAppComponentWindow->addWindowListener( this );
        }
        osl_atomic_decrement( &m_refCount );
    }

    SubComponentLoader::~SubComponentLoader()
    {
    }

    void SAL_CALL SubComponentLoader::windowResized( const WindowEvent&  )
    {
    }

    void SAL_CALL SubComponentLoader::windowMoved( const WindowEvent& )
    {
    }

    void SAL_CALL SubComponentLoader::windowShown( const EventObject& )
    {
        try
        {
            if ( mxDocDefCommands.is() )
            {
                Command aCommandOpen;
                aCommandOpen.Name = "show";

                const sal_Int32 nCommandIdentifier = mxDocDefCommands->createCommandIdentifier();
                mxDocDefCommands->execute( aCommandOpen, nCommandIdentifier, nullptr );
            }
            else
            {
                const Reference< XController > xController( mxNonDocComponent, UNO_QUERY_THROW );
                const Reference< XFrame > xFrame( xController->getFrame(), UNO_SET_THROW );
                const Reference< XWindow > xWindow( xFrame->getContainerWindow(), UNO_SET_THROW );
                xWindow->setVisible( true );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        mxAppComponentWindow->removeWindowListener( this );
    }

    void SAL_CALL SubComponentLoader::windowHidden( const EventObject& )
    {
    }

    void SAL_CALL SubComponentLoader::disposing( const EventObject& )
    {
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
