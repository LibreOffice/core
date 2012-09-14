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

#include <stdio.h>
#include <unistd.h>

#include "vcl/svapp.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/unowrap.hxx"

#include "padialog.hxx"
#include "helper.hxx"
#include "desktopcontext.hxx"

#include "cppuhelper/bootstrap.hxx"
#include "comphelper/processfactory.hxx"
#include "unotools/configmgr.hxx"

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/ucb/UniversalContentBroker.hpp"

using namespace padmin;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace comphelper;

using ::rtl::OUString;

class MyApp : public Application
{
public:
    int Main();
    virtual sal_uInt16  Exception( sal_uInt16 nError );

    static rtl::OUString ReadStringHook( const rtl::OUString& );
};

MyApp aMyApp;

rtl::OUString MyApp::ReadStringHook( const rtl::OUString& rStr )
{
    return rStr.replaceAll(
        rtl::OUString("%PRODUCTNAME"), utl::ConfigManager::getProductName() );
};


// -----------------------------------------------------------------------

sal_uInt16 MyApp::Exception( sal_uInt16 nError )
{
    switch( nError & EXC_MAJORTYPE )
    {
        case EXC_RSCNOTLOADED:
            Abort( rtl::OUString( "Error: could not load language resources.\nPlease check your installation.\n" ) );
            break;
    }
    return 0;
}

int MyApp::Main()
{
    PADialog* pPADialog;

    EnableAutoHelpId();

    //-------------------------------------------------
    // create the global service-manager
    //-------------------------------------------------
    Reference< XComponentContext > xCtx;
    Reference< XMultiServiceFactory > xFactory;
    try
    {
        xCtx = defaultBootstrap_InitialComponentContext();
        xFactory = Reference< XMultiServiceFactory >(  xCtx->getServiceManager(), UNO_QUERY );
        if( xFactory.is() )
            setProcessServiceFactory( xFactory );
    }
    catch( const com::sun::star::uno::Exception& )
    {
    }

    if( ! xFactory.is() )
    {
        fprintf( stderr, "Could not bootstrap UNO, installation must be in disorder. Exiting.\n" );
        exit( 1 );
    }

    // Detect desktop environment - need to do this as early as possible
    com::sun::star::uno::setCurrentContext(
        new DesktopContext( com::sun::star::uno::getCurrentContext() ) );

    // Create UCB (for backwards compatibility, in case some code still uses
    // plain createInstance w/o args directly to obtain an instance):
    com::sun::star::ucb::UniversalContentBroker::create(xCtx);

    /*
     * Initialize the Java UNO AccessBridge if accessibility is turned on
     */

    if( Application::GetSettings().GetMiscSettings().GetEnableATToolSupport() )
    {
        sal_Bool bQuitApp;
        if( !InitAccessBridge( true, bQuitApp ) )
            if( bQuitApp )
                return EXIT_FAILURE;
    }

    ResMgr::SetReadStringHook( MyApp::ReadStringHook );

    pPADialog = PADialog::Create( NULL , sal_False );
    Application::SetDisplayName( pPADialog->GetText() );
    pPADialog->SetIcon(501);
    pPADialog->Execute();
    delete pPADialog;

    /*
     *  clean up UNO
     */
    try
    {
        Reference<XComponent> xComp(xCtx, UNO_QUERY_THROW);
        xComp->dispose();
    }
    catch(...)
    {
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
