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


#include "defaulthelpprovider.hxx"
#include "pcrcommon.hxx"
#include "pcrservices.hxx"
#include "modulepcr.hxx"

#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <tools/diagnose_ex.h>


extern "C" void SAL_CALL createRegistryInfo_DefaultHelpProvider()
{
    ::pcr::OAutoRegistration< ::pcr::DefaultHelpProvider > aAutoRegistration;
}


namespace pcr
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::inspection::XObjectInspectorUI;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::awt::XVclWindowPeer;

    DefaultHelpProvider::DefaultHelpProvider()
        :m_bConstructed( false )
    {
    }


    DefaultHelpProvider::~DefaultHelpProvider()
    {
    }


    OUString DefaultHelpProvider::getImplementationName_static(  ) throw(RuntimeException)
    {
        return OUString("org.openoffice.comp.extensions.DefaultHelpProvider");
    }


    Sequence< OUString > DefaultHelpProvider::getSupportedServiceNames_static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(1);
        aSupported[0] = "com.sun.star.inspection.DefaultHelpProvider";
        return aSupported;
    }


    Reference< XInterface > SAL_CALL DefaultHelpProvider::Create( const Reference< XComponentContext >& )
    {
        return *new DefaultHelpProvider;
    }


    void SAL_CALL DefaultHelpProvider::focusGained( const Reference< XPropertyControl >& _Control ) throw (RuntimeException, std::exception)
    {
        if ( !m_xInspectorUI.is() )
            throw RuntimeException( OUString(), *this );

        try
        {
            m_xInspectorUI->setHelpSectionText( impl_getHelpText_nothrow( _Control ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void SAL_CALL DefaultHelpProvider::valueChanged( const Reference< XPropertyControl >& /*_Control*/ ) throw (RuntimeException, std::exception)
    {
        // not interested in
    }


    void SAL_CALL DefaultHelpProvider::initialize( const Sequence< Any >& _arguments ) throw (Exception, RuntimeException, std::exception)
    {
        if ( m_bConstructed )
            throw AlreadyInitializedException();

        StlSyntaxSequence< Any > arguments( _arguments );
        if ( arguments.size() == 1 )
        {   // constructor: "create( XObjectInspectorUI )"
            Reference< XObjectInspectorUI > xUI( arguments[0], UNO_QUERY );
            create( xUI );
            return;
        }

        throw IllegalArgumentException( OUString(), *this, 0 );
    }


    void DefaultHelpProvider::create( const Reference< XObjectInspectorUI >& _rxUI )
    {
        if ( !_rxUI.is() )
            throw IllegalArgumentException( OUString(), *this, 1 );

        try
        {
            m_xInspectorUI = _rxUI;
            m_xInspectorUI->registerControlObserver( this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        m_bConstructed = true;
    }


    vcl::Window* DefaultHelpProvider::impl_getVclControlWindow_nothrow( const Reference< XPropertyControl >& _rxControl )
    {
        vcl::Window* pControlWindow = nullptr;
        OSL_PRECOND( _rxControl.is(), "DefaultHelpProvider::impl_getVclControlWindow_nothrow: illegal control!" );
        if ( !_rxControl.is() )
            return pControlWindow;

        try
        {
            Reference< XWindow > xControlWindow( _rxControl->getControlWindow(), UNO_QUERY_THROW );
            pControlWindow = VCLUnoHelper::GetWindow( xControlWindow );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return pControlWindow;
    }


    OUString DefaultHelpProvider::impl_getHelpText_nothrow( const Reference< XPropertyControl >& _rxControl )
    {
        OUString sHelpText;
        OSL_PRECOND( _rxControl.is(), "DefaultHelpProvider::impl_getHelpText_nothrow: illegal control!" );
        if ( !_rxControl.is() )
            return sHelpText;

        vcl::Window* pControlWindow( impl_getVclControlWindow_nothrow( _rxControl ) );
        OSL_ENSURE( pControlWindow, "DefaultHelpProvider::impl_getHelpText_nothrow: could not determine the VCL window!" );
        if ( !pControlWindow )
            return sHelpText;

        sHelpText = pControlWindow->GetHelpText();
        return sHelpText;
    }

} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
