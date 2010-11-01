/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <helper/oinstanceprovider.hxx>
#include <classes/servicemanager.hxx>
#include <macros/debug.hxx>
#include <osl/process.h>

#include <defines.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XFrames.hpp>

#include <com/sun/star/connection/XConnection.hpp>

#include <com/sun/star/bridge/XBridgeFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/unoiface.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl                           ;
using namespace ::comphelper                    ;
using namespace ::framework                     ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::awt           ;
using namespace ::com::sun::star::beans         ;
using namespace ::com::sun::star::util          ;
using namespace ::com::sun::star::connection    ;
using namespace ::com::sun::star::bridge        ;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define APPLICATIONNAME                         "FrameWork"
#define COMMANDARGUMENT_PLUGIN                  DECLARE_ASCII("-plugin"                 )
#define NAME_PLUGINBRIDGE                       DECLARE_ASCII("mozilla plugin bridge"   )
#define PROTOCOL_PLUGINBRIDGE                   DECLARE_ASCII("urp"                     )

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short      normal application
    @descr      An instance of these class can be a normal node in frame tree only. The highest level to be allowed is 3!
                On 1 stand the desktop himself as the only one, on 2 are all tasks present ... and then comes frames only.
                A frame support influencing of his subtree, find of subframes, activate- and deactivate-mechanism as well as
                set/get of a frame window, component or controller.

    @implements XInterface
                XTypeProvider
                XServiceInfo
                XFramesSupplier
                XFrame
                XComponent
                XStatusIndicatorSupplier
                XDispatchProvider
                XDispatchProviderInterception
                XBrowseHistoryRegistry
                XLoadEventListener
                XEventListener
                XWindowListener
                XTopWindowListener
                [ XDebugging, if TEST_TREE is defined! ]
    @base       OMutexMember
                OWeakObject

    @devstatus  deprecated
*//*-*************************************************************************************************************/
class FrameWork : public Application
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:
        void Main();

    private:
        void impl_analyzeCommandArguments();

    private:
        sal_Bool    m_bUsePlugIn    ;

};  //  class FrameWork

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  global variables
//_________________________________________________________________________________________________________________

FrameWork   aFrameWork  ;

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  private methods
//*****************************************************************************************************************
void FrameWork::impl_analyzeCommandArguments()
{
    // First reset all member variables which present states of incoming arguments!
    m_bUsePlugIn = sal_False;   // depends from "/plugin"

    // Then step over all given arguments and search for supported one.
    OUString        sArgument   ;
    sal_uInt32      nCount = osl_getCommandArgCount();
    for ( sal_uInt32 nArgument=0; nArgument<nCount; ++nArgument )
    {
        // If extraction of current argument successfull ...
        if ( osl_getCommandArg( nArgument, &sArgument.pData ) == osl_Process_E_None )
        {
            // ... search for matching with supported values.
            if ( sArgument == COMMANDARGUMENT_PLUGIN )
            {
                // We found "/plugin" => set internal equivalent.
                m_bUsePlugIn = sal_True;
            }
        }
    }
}

//_________________________________________________________________________________________________________________
//  main
//_________________________________________________________________________________________________________________

void FrameWork::Main()
{
    //-------------------------------------------------------------------------------------------------------------
    // a) Initialize ouer application

    // Analyze command arguments.
    impl_analyzeCommandArguments();

    // Create new global servicemanager.
    ServiceManager aManager;
    Reference< XMultiServiceFactory > xGlobalServiceManager = aManager.getGlobalUNOServiceManager();

    if ( xGlobalServiceManager.is() == sal_True )
    {
        // If it was successful - set in as static value in UNOTOOLS!
        setProcessServiceFactory( xGlobalServiceManager );

        //---------------------------------------------------------------------------------------------------------
        // b) Create root of ouer frame tree

        // Create top of frame hierarchy - the desktop.
        Reference< XDesktop > xDesktop( xGlobalServiceManager->createInstance( SERVICENAME_DESKTOP ), UNO_QUERY );
        // Safe impossible cases
        // We need the desktop for working.
        LOG_ASSERT( !(xDesktop.is()==sal_False), "FrameWork::Main()\nCan't instanciate desktop!Servicename unknown?\n" )

        //---------------------------------------------------------------------------------------------------------
        // c) Initialize connection to possible PlugIn dll.

// OPipeConnection removed, connection to plugin now uses acceptor service
        //---------------------------------------------------------------------------------------------------------
        // d) Initialize new task with a HTML-URL in it.

        // Cast desktop to right interface to do this.
        Reference< XDispatchProvider > xDispatchProvider( xDesktop, UNO_QUERY );
        // Safe impossible cases.
        // Desktop MUST support these interface!
        LOG_ASSERT( !(xDispatchProvider.is()==sal_False), "FrameWork::Main()\nDesktop don't support XDispatchProvider interface.\n" )
        if ( xDispatchProvider.is() == sal_True )
        {
            // Build URL ...
            OUString sURL( RTL_CONSTASCII_USTRINGPARAM( "file://e|/dokumentation/Documentation/projekte/services/inimanager/inimanager/index.html" ));
            URL aURL;
            aURL.Complete = sURL;
            // ... and dispatch it.
            Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aURL, FRAMETYPE_BLANK, 0 );
            xDispatch->dispatch( aURL, Sequence< PropertyValue >() );

            // Use special feature of desktop service and log current tree state in file.
//          LOG_TREE( xDesktop )

            // Build URL ...
            sURL = OUString( RTL_CONSTASCII_USTRINGPARAM( "file://d|/menu.htm" ));
            aURL.Complete = sURL;
            // ... and dispatch it.
            xDispatch = xDispatchProvider->queryDispatch( aURL, FRAMETYPE_BLANK, 0 );
            xDispatch->dispatch( aURL, Sequence< PropertyValue >() );

            // Use special feature of desktop service and log current tree state in file.
//          LOG_TREE( xDesktop )
        }

        // Set running-mode for application.
        Execute();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
