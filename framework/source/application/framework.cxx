/*************************************************************************
 *
 *  $RCSfile: framework.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_OPIPECONNECTION_HXX_
#include <helper/opipeconnection.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OINSTANCEPROVIDER_HXX_
#include <helper/oinstanceprovider.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_SERVICEMANAGER_HXX_
#include <classes/servicemanager.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_DEFINES_HXX_
#include <defines.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADER_HPP_
#include <com/sun/star/frame/XFrameLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XLOADEVENTLISTENER_HPP_
#include <com/sun/star/frame/XLoadEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMES_HPP_
#include <com/sun/star/frame/XFrames.hpp>
#endif

#ifndef _COM_SUN_STAR_CONNECTION_XConnection_HPP_
#include <com/sun/star/connection/XConnection.hpp>
#endif

#ifndef _COM_SUN_STAR_BRIDGE_XBridgeFactory_HPP_
#include <com/sun/star/bridge/XBridgeFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _SVT_UNOIFACE_HXX
#include <svtools/unoiface.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl                           ;
using namespace ::vos                           ;
using namespace ::utl                           ;
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
    OStartupInfo    aInfo       ;
    OUString        sArgument   ;
    sal_uInt32      nCount      = aInfo.getCommandArgCount();
    for ( sal_uInt32 nArgument=0; nArgument<nCount; ++nArgument )
    {
        // If extraction of current argument successfull ...
        if ( aInfo.getCommandArg( nArgument, sArgument ) == osl_Process_E_None )
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
    Reference< XMultiServiceFactory > xGlobalServiceManager = aManager.getManager();

    if ( xGlobalServiceManager.is() == sal_True )
    {
        // If it was successful - set in as static value in UNOTOOLS!
        setProcessServiceFactory( xGlobalServiceManager );
        // Initialize toolkit.
         InitExtVclToolkit();

        //---------------------------------------------------------------------------------------------------------
        // b) Create root of ouer frame tree

        // Create top of frame hierarchy - the desktop.
        Reference< XDesktop > xDesktop( xGlobalServiceManager->createInstance( SERVICENAME_DESKTOP ), UNO_QUERY );
        // Safe impossible cases
        // We need the desktop for working.
        LOG_ASSERT( !(xDesktop.is()==sal_False), "FrameWork::Main()\nCan't instanciate desktop!Servicename unknown?\n" )

        //---------------------------------------------------------------------------------------------------------
        // c) Initialize connection to possible PlugIn dll.
        if ( m_bUsePlugIn == sal_True )
        {
            Reference< XConnection >    xConnection         = new OPipeConnection( xGlobalServiceManager );
            Reference< XBridgeFactory > xBridgeFactory      ( xGlobalServiceManager->createInstance( SERVICENAME_BRIDGEFACTORY  ), UNO_QUERY );
            if  (
                    ( xConnection.is()          == sal_True )   &&
                    ( xBridgeFactory.is()       == sal_True )
                )
            {
                Reference< XBridge > xBridge = xBridgeFactory->createBridge(    NAME_PLUGINBRIDGE                               ,
                                                                                PROTOCOL_PLUGINBRIDGE                           ,
                                                                                xConnection                                     ,
                                                                                new OInstanceProvider( xGlobalServiceManager )  );
            }
            else
            {
                // Error handling ... !?
                LOG_ASSERT( sal_False, "FrameWork::Main()\nNo connection to plugin. Initialization of bridge failed.\n" )
            }
        }

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
