/*************************************************************************
 *
 *  $RCSfile: test.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: as $ $Date: 2001-03-09 14:42:26 $
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

#ifndef __FRAMEWORK_HELPER_OINSTANCEPROVIDER_HXX_
#include <helper/oinstanceprovider.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_SERVICEMANAGER_HXX_
#include <classes/servicemanager.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#include <classes/filtercache.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
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

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCEPEER_HPP_
#include <com/sun/star/mozilla/XPluginInstancePeer.hpp>
#endif

#ifndef _COM_SUN_STAR_BRIDGE_XINSTANCEPROVIDER_HPP_
#include <com/sun/star/bridge/XInstanceProvider.hpp>
#endif

#ifdef TF_FILTER//MUSTFILTER
    #ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
    #include <com/sun/star/document/XTypeDetection.hpp>
    #endif
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XELEMENTACCESS_HPP_
#include <com/sun/star/container/XElementAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
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

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <stdio.h>

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define APPLICATIONNAME                         "FrameWork - Testapplication"

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl                       ;
using namespace ::comphelper                ;
using namespace ::framework                 ;
using namespace ::cppu                      ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::frame     ;
using namespace ::com::sun::star::awt       ;
using namespace ::com::sun::star::beans     ;
using namespace ::com::sun::star::util      ;
using namespace ::com::sun::star::task      ;
using namespace ::com::sun::star::mozilla   ;
using namespace ::com::sun::star::bridge    ;
#ifdef TF_FILTER//MUSTFILTER
using namespace ::com::sun::star::document  ;
#endif
using namespace ::com::sun::star::container ;

//_________________________________________________________________________________________________________________
//  defines
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-***************************************************************************************************************/
class TestApplication : public Application
{
    //*************************************************************************************************************
    public:

        void Main();

    //*************************************************************************************************************
    private:

        #ifdef TEST_DESKTOP
        void impl_testDesktop   ( const Reference< XDesktop >& xDesktop );
        void impl_buildTree     ( const Reference< XDesktop >& xDesktop );
        void impl_logTree       ( const Reference< XDesktop >& xDesktop );

        #endif

        #ifdef TEST_PLUGIN
        void impl_testPlugIn    ( const Reference< XDesktop >& xDesktop, const Reference< XMultiServiceFactory >& xFactory  );
        #endif

        #ifdef TEST_LOGINDIALOG
        void impl_testLoginDialog();
        #endif

#ifdef TF_FILTER
        #ifdef TEST_FILTERCACHE
        void impl_testFilterCache();
        #endif

        #ifdef TEST_TYPEDETECTION
        void impl_testTypeDetection();
        #endif

        #ifdef TEST_FILTERREGISTRATION
        void impl_testFilterRegistration();
        #endif
#endif

        #ifdef TEST_TREESEARCH
        sal_Bool impl_testTreeSearch();
        #endif

    //*************************************************************************************************************
    private:

    //*************************************************************************************************************
    private:

        Reference< XMultiServiceFactory >   m_xFactory;

};  //  class TestApplication

//_________________________________________________________________________________________________________________
//  global variables
//_________________________________________________________________________________________________________________

TestApplication aTestApplication ;

//_________________________________________________________________________________________________________________
//  main
//_________________________________________________________________________________________________________________

void TestApplication::Main()
{
//  RegistryCache aCache;

    /**-***********************************************************************************************************
        initialize program
    **************************************************************************************************************/

    // Init global servicemanager and set it.
    ServiceManager aManager;
    m_xFactory = aManager.getGlobalUNOServiceManager();
    setProcessServiceFactory( m_xFactory );

    // Control sucess of operation.
    LOG_ASSERT( !(m_xFactory.is()           ==sal_False ), "TestApplication::Main()\nCan't create global service manager.\n\n"          )
    LOG_ASSERT( !(getProcessServiceFactory()!=m_xFactory), "TestApplication::Main()\nGlobal servicemanager not set in UNOTOOLS.\n\n"    )

    // For some follow operations, we need the vcl-toolkit!
    InitExtVclToolkit();

    /**-***********************************************************************************************************
        test area
    **************************************************************************************************************/

    sal_Bool bState = sal_True;

    //-------------------------------------------------------------------------------------------------------------
    #ifdef TEST_FILTERCACHE
    impl_testFilterCache();
    #endif

    //-------------------------------------------------------------------------------------------------------------
#ifdef TF_FILTER
    #ifdef TEST_TYPEDETECTION
    impl_testTypeDetection();
    #endif
#endif

    //-------------------------------------------------------------------------------------------------------------
    #ifdef TEST_LOGINDIALOG
    ResMgr* pRessourceManager = CREATEVERSIONRESMGR( lgd );
    Resource::SetResManager( pRessourceManager );
    impl_testLoginDialog();
    #endif

    //-------------------------------------------------------------------------------------------------------------
    #ifdef TEST_DESKTOP
    Reference< XDesktop > xDesktop( xGlobalServiceManager->createInstance( SERVICENAME_DESKTOP ), UNO_QUERY );
    impl_testDesktop( xDesktop );
    #endif

    //-------------------------------------------------------------------------------------------------------------
    #ifdef TEST_PLUGIN
    impl_testPlugIn( xDesktop, xGlobalServiceManager );
    #endif

    //-------------------------------------------------------------------------------------------------------------
    #ifdef TEST_FILTERREGISTRATION
    impl_testFilterRegistration();
    #endif

    //-------------------------------------------------------------------------------------------------------------
    #ifdef TEST_TREESEARCH
    bState = impl_testTreeSearch();
    #endif

//  Execute();
//    xFrame->dispose();
//    delete pMainWindow;
    if( bState = sal_True )
    {
        LOG_ERROR( "TestApplication::Main()", "Test successful ..." )
    }
    else
    {
        LOG_ERROR( "TestApplication::Main()", "Test failed ..." )
    }
}

//_________________________________________________________________________________________________________________
//  test method
//_________________________________________________________________________________________________________________
#ifdef TF_FILTER
#ifdef TEST_TYPEDETECTION
void TestApplication::impl_testTypeDetection()
{
    // We use a string buffer to log important informations and search results.
    // Errors are shown directly by an assert!
    OUStringBuffer sBuffer( 100000 );

    // Create a new type detection service.
    Reference< XTypeDetection > xTypeDetection( getProcessServiceFactory()->createInstance( SERVICENAME_TYPEDETECTION ), UNO_QUERY );
    LOG_ASSERT( !(xTypeDetection.is()==sal_False), "TestApplication::impl_testTypeDetection()\nCouldn't create the type detection service.\n" );

    if( xTypeDetection.is() == sal_True )
    {
        // a) Check his implementation and his supported interfaces first.
        Reference< XInterface >     xInterface      ( xTypeDetection, UNO_QUERY );
        Reference< XTypeProvider >  xTypeProvider   ( xTypeDetection, UNO_QUERY );
        Reference< XServiceInfo >   xServiceInfo    ( xTypeDetection, UNO_QUERY );
        Reference< XNameAccess >    xNameAccess     ( xTypeDetection, UNO_QUERY );
        Reference< XElementAccess > xElementAccess  ( xTypeDetection, UNO_QUERY );

        LOG_ASSERT( !(  xInterface.is()     ==  sal_False   ||
                        xTypeProvider.is()  ==  sal_False   ||
                        xServiceInfo.is()   ==  sal_False   ||
                        xNameAccess.is()    ==  sal_False   ||
                        xElementAccess.is() ==  sal_False   ), "TestApplication::impl_testTypeDetection()\nMiss supported for searched interface!\n" )

        // b) Check OneInstance mode of service.
        Reference< XTypeDetection > xTypeDetection2( getProcessServiceFactory()->createInstance( SERVICENAME_TYPEDETECTION ), UNO_QUERY );
        LOG_ASSERT( !(xTypeDetection!=xTypeDetection2), "TestApplication::impl_testTypeDetection()\nService isn't \"OneInstance\" ...!\n" )
        xTypeDetection2 = Reference< XTypeDetection >();

        // c) Check "XTypeDetection" ... flat by URL
        // Define list of URLs for checking.
        OUString pURLs[] =
        {
            DECLARE_ASCII("file://c|/temp/test.sdw" ),
            DECLARE_ASCII("private:factory/scalc"   ),
            DECLARE_ASCII("file://c|/temp/test.txt" ),
            DECLARE_ASCII("slot:5000"               ),
        };
        sal_uInt32 nCount = 4;
        Sequence< OUString > seqURLs( pURLs, nCount );

        Reference< XMultiServiceFactory > xFilterFactory( getProcessServiceFactory()->createInstance( SERVICENAME_FILTERFACTORY     ), UNO_QUERY );
        Reference< XMultiServiceFactory > xLoaderFactory( getProcessServiceFactory()->createInstance( SERVICENAME_FRAMELOADERFACTORY), UNO_QUERY );
        LOG_ASSERT( !(xFilterFactory.is()==sal_False), "TestApplication::impl_testTypeDetection()\nCouldn't create filter factory!\n" )
        LOG_ASSERT( !(xLoaderFactory.is()==sal_False), "TestApplication::impl_testTypeDetection()\nCouldn't create loader factory!\n" )

        // Step over these list.
        for( sal_uInt32 nURL=0; nURL<nCount; ++nURL )
        {
            // Try to get a type name for every URL from list and log search result.
            OUString sTypeName = xTypeDetection->queryTypeByURL( seqURLs[nURL] );
            sBuffer.appendAscii ( "queryTypeByURL( \""      );
            sBuffer.append      ( seqURLs[nURL]             );
            sBuffer.appendAscii ( "\" ) returns type \""    );
            sBuffer.append      ( sTypeName                 );
            sBuffer.appendAscii ( "\"\n"                    );
            // If a type was found - try to get a filter and a frame loader for it.
            if( sTypeName.getLength() > 0 )
            {
                Reference< XInterface > xFilter = xFilterFactory->createInstance( sTypeName );
                Reference< XInterface > xLoader = xLoaderFactory->createInstance( sTypeName );
                if( xFilter.is() == sal_False )
                {
                    sBuffer.appendAscii( "Couldn't find an filter.\n" );
                }
                else
                {
                    Reference< XPropertySet > xFilterProperties( xFilter, UNO_QUERY );
                    LOG_ASSERT( !(xFilterProperties.is()==sal_False), "TestApplication::impl_testTypeDetection()\nFilter don't support XPropertySet!\n" )
                    if( xFilterProperties.is() == sal_True )
                    {
                        OUString sUIName;
                        xFilterProperties->getPropertyValue( DECLARE_ASCII("UIName") ) >>= sUIName;
                        sBuffer.appendAscii ( "Found filter \"" );
                        sBuffer.append      ( sUIName           );
                        sBuffer.appendAscii ( "\"\n"            );
                    }

                    Reference< XPropertySet > xLoaderProperties( xLoader, UNO_QUERY );
                    LOG_ASSERT( !(xLoaderProperties.is()==sal_False), "TestApplication::impl_testTypeDetection()\nLoader don't support XPropertySet!\n" )
                    if( xLoaderProperties.is() == sal_True )
                    {
                        OUString sUIName;
                        xLoaderProperties->getPropertyValue( DECLARE_ASCII("UIName") ) >>= sUIName;
                        sBuffer.appendAscii ( "Found loader \"" );
                        sBuffer.append      ( sUIName           );
                        sBuffer.appendAscii ( "\"\n"            );
                    }
                }
            }
        }
    }

    WRITE_LOGFILE( "testTypeDetection.log", U2B(sBuffer.makeStringAndClear()).getStr() )
}
#endif
#endif

//_________________________________________________________________________________________________________________
//  test method
//_________________________________________________________________________________________________________________
#ifdef TF_FILTER
#ifdef TEST_FILTERCACHE
void TestApplication::impl_testFilterCache()
{
    FilterCache aCache;

    OUStringBuffer sBuffer( 100000 );
    if( aCache.isValid() == sal_False )
    {
        sBuffer.appendAscii( "Cache isn't valid!\n" );
    }
    else
    {
/*
        // Step over all types and log his values.
        // These simulate a XNameAccess!
        const Sequence< OUString > seqAllTypeNames = aCache.getAllTypeNames();
        sal_uInt32 nCount = seqAllTypeNames.getLength();
        for( sal_uInt32 nPosition=0; nPosition<nCount; ++nPosition )
        {
            sBuffer.appendAscii( "--------------------------------------------------------------------------------\n" );
            const TType* pType = aCache.getTypeByName( seqAllTypeNames[nPosition] );
            if( pType == NULL )
            {
                sBuffer.appendAscii ( "Type ["                  );
                sBuffer.append      ( (sal_Int32)nPosition      );
                sBuffer.appendAscii ( "] \""                    );
                sBuffer.append      ( seqAllTypeNames[nPosition]);
                sBuffer.appendAscii ( "\" isn't valid!"         );
            }
            else
            {
                sBuffer.appendAscii ( "Type ["                      );
                sBuffer.append      ( (sal_Int32)nPosition          );
                sBuffer.appendAscii ( "] \""                        );
                sBuffer.append      ( seqAllTypeNames[nPosition]    );
                sBuffer.appendAscii ( "\"\n\t\tUIName\t=\t"         );
                sBuffer.append      ( pType->sUIName                );
                sBuffer.appendAscii ( "\n\t\tMediaType\t=\t"        );
                sBuffer.append      ( pType->sMediaType             );
                sBuffer.appendAscii ( "\n\t\tClipboardFormat\t=\t"  );
                sBuffer.append      ( pType->sClipboardFormat       );
                sBuffer.appendAscii ( "\n\t\tURLPattern\t=\t{"      );
                for( TConstStringIterator aIterator=pType->lURLPattern.begin(); aIterator!=pType->lURLPattern.end(); ++aIterator )
                {
                    sBuffer.append      ( *aIterator    );
                    sBuffer.appendAscii ( ";\n\t\t"     );
                }
                sBuffer.appendAscii( "}\nExtensions\t=\t" );
                for( aIterator=pType->lExtensions.begin(); aIterator!=pType->lExtensions.end(); ++aIterator )
                {
                    sBuffer.append      ( *aIterator    );
                    sBuffer.appendAscii ( ";\n\t\t"     );
                }
                sBuffer.appendAscii ( "}\nDocumentIconID\t=\t"          );
                sBuffer.append      ( (sal_Int32)pType->nDocumentIconID );
            }
        }
*/
        // searchFirstType( URL, MediaType, ClipboardFormat, startEntry )
        TCheckedTypeIterator aIterator;
        sBuffer.appendAscii( "search type for \"file://c|/temp/test.sdw\"; no media type; no clipboard format\n" );
        OUString sURL = DECLARE_ASCII("file://c|/temp/test.sdw");
        const OUString* pType = aCache.searchFirstType( &sURL, NULL, NULL, aIterator );
        while( pType != NULL )
        {
            sBuffer.appendAscii ( "\tfound \""  );
            sBuffer.append      ( *pType        );
            sBuffer.appendAscii ( "\"\n"        );
            pType = aCache.searchType( &sURL, NULL, NULL, aIterator );
        }
    }

    WRITE_LOGFILE( "test_FilterCache.log", U2B(sBuffer.makeStringAndClear()).getStr() )
}
#endif
#endif

//_________________________________________________________________________________________________________________
//  test method
//_________________________________________________________________________________________________________________
#ifdef TEST_LOGINDIALOG
void TestApplication::impl_testLoginDialog()
{
    // Get global servicemanager to create service "LoginDialog".
    Reference< XMultiServiceFactory > xFactory = getProcessServiceFactory();
    LOG_ASSERT( !(xFactory.is()==sal_False), "TestApplication::impl_testLoginDialog()\nServicemanager not valid.\n" )
    // Create LoginDialog.
    Reference< XDialog > xDialog( xFactory->createInstance( DECLARE_ASCII("com.sun.star.framework.LoginDialog") ), UNO_QUERY );
    LOG_ASSERT( !(xDialog.is()==sal_False), "TestApplication::impl_testLoginDialog()\nCould not create login dialog.\n" )
    // Check set/getTitle()
    /* not implemented yet! */
//  OUString sTitle = DECLARE_ASCII("Login Dialog Test");
//  xDialog->setTitle( sTitle );
//  LOG_ASSERT( !(xDialog->getTitle()!=sTitle), "TestApplication::impl_testLoginDialog()\nset/getTitle don't work correct.\n" )

    UniString sInternalURL = DECLARE_ASCII("private:factory/scalc");
    UniString sExternalURL = OUString();

    OString sOut = "internal: ";
    sOut += OUStringToOString( sInternalURL, RTL_TEXTENCODING_UTF8 ).getStr();
    sOut += "\nexternal: ";
    sOut += OUStringToOString( sExternalURL, RTL_TEXTENCODING_UTF8 ).getStr();
    sOut += "\n";
    LOG_ASSERT( sal_False, sOut.getStr() )

    INetURLObject::translateToExternal( sInternalURL, sExternalURL );
    sOut = "internal: ";
    sOut += OUStringToOString( sInternalURL, RTL_TEXTENCODING_UTF8 ).getStr();
    sOut += "\nexternal: ";
    sOut += OUStringToOString( sExternalURL, RTL_TEXTENCODING_UTF8 ).getStr();
    sOut += "\n";
    LOG_ASSERT( sal_False, sOut.getStr() )

    INetURLObject::translateToInternal( sExternalURL, sInternalURL );
    sOut = "internal: ";
    sOut += OUStringToOString( sInternalURL, RTL_TEXTENCODING_UTF8 ).getStr();
    sOut += "\nexternal: ";
    sOut += OUStringToOString( sExternalURL, RTL_TEXTENCODING_UTF8 ).getStr();
    sOut += "\n";
    LOG_ASSERT( sal_False, sOut.getStr() )

    // Work with properties of dialog.
    Reference< XPropertySet > xPropertySet( xDialog, UNO_QUERY );
    LOG_ASSERT( !(xPropertySet.is()==sal_False), "TestApplication::impl_testLoginDialog()\nCan't cast dialog to XPropertySet interface.\n" )
    Any aUserName       ;
    Any aPassword       ;
    Any aServer         ;
    Any aConnectionType ;
    Any aPort           ;
    aUserName       <<= DECLARE_ASCII("Andreas");
    aPassword       <<= DECLARE_ASCII("Test");
    aServer         <<= DECLARE_ASCII("www.yahoo.de:7777");
    aConnectionType <<= DECLARE_ASCII("Bla");
    sal_Int32 nPort = 8081;
    aPort           <<= nPort;
    try
    {
    xPropertySet->setPropertyValue( DECLARE_ASCII("UserName"            ), aUserName        );
    xPropertySet->setPropertyValue( DECLARE_ASCII("Password"            ), aPassword        );
    xPropertySet->setPropertyValue( DECLARE_ASCII("Server"              ), aServer          );
    xPropertySet->setPropertyValue( DECLARE_ASCII("ConnectionType"  ), aConnectionType  );
    xPropertySet->setPropertyValue( DECLARE_ASCII("Compressed"      ), aPort            );
    }
    catch( ::com::sun::star::beans::UnknownPropertyException& )
    {
        LOG_ASSERT( sal_False, "UnkownPropertyException detected!\n" )
    }
    catch( ::com::sun::star::beans::PropertyVetoException& )
    {
        LOG_ASSERT( sal_False, "PropertyVetoException detected!\n" )
    }
    catch( ::com::sun::star::lang::IllegalArgumentException& )
    {
        LOG_ASSERT( sal_False, "IllegalArgumentException detected!\n" )
    }
    catch( ::com::sun::star::lang::WrappedTargetException& )
    {
        LOG_ASSERT( sal_False, "WrappedTargetException detected!\n" )
    }
    catch( ::com::sun::star::uno::RuntimeException& )
    {
        LOG_ASSERT( sal_False, "RuntimeException detected!\n" )
    }

    xDialog->execute();

    OUString sUserName          ;
    OUString sPassword          ;
    OUString sServer            ;
    OUString sConnectionType    ;
    xPropertySet->getPropertyValue( DECLARE_ASCII("UserName"            ) ) >>= sUserName       ;
    xPropertySet->getPropertyValue( DECLARE_ASCII("Password"            ) ) >>= sPassword       ;
    xPropertySet->getPropertyValue( DECLARE_ASCII("Server"              ) ) >>= sServer         ;
    xPropertySet->getPropertyValue( DECLARE_ASCII("ConnectionType"  ) ) >>= sConnectionType ;
    xPropertySet->getPropertyValue( sConnectionType ) >>= nPort ;

    LOG_ASSERT( sal_False, OUStringToOString( sUserName, RTL_TEXTENCODING_UTF8 ).getStr() )
    LOG_ASSERT( sal_False, OUStringToOString( sPassword, RTL_TEXTENCODING_UTF8 ).getStr() )
    LOG_ASSERT( sal_False, OUStringToOString( sServer  , RTL_TEXTENCODING_UTF8 ).getStr() )
    LOG_ASSERT( sal_False, OUStringToOString( sConnectionType  , RTL_TEXTENCODING_UTF8 ).getStr() )
    LOG_ASSERT( sal_False, OString::valueOf( (sal_Int32)nPort ).getStr() )
}
#endif

//_________________________________________________________________________________________________________________
//  test method
//_________________________________________________________________________________________________________________
#ifdef TEST_PLUGIN
void TestApplication::impl_testPlugIn( const Reference< XDesktop >& xDesktop, const Reference< XMultiServiceFactory >& xFactory )
{
    // create instance provider for creation of factories.
    Reference< XInstanceProvider > xInstanceProvider( (OWeakObject*)(new OInstanceProvider( xFactory )), UNO_QUERY );
    LOG_ASSERT( !(xInstanceProvider.is()==sal_False), "TestApplication::impl_testPlugIn()\nCan't create new instance provider!\n" )

    // try to get factory for create a plugin
    Reference< XSingleServiceFactory > xPlugInFactory( xInstanceProvider->getInstance( INSTANCENAME_PLUGINFACTORY ), UNO_QUERY );
    LOG_ASSERT( !(xPlugInFactory.is()==sal_False), "TestApplication::impl_testPlugIn()\nCan't get PlugInFactory from instance provider!\n" )

    // initialize parameter for creation of plugin
    Reference< XPluginInstancePeer > xPlugInDLL =Reference< XPluginInstancePeer >();
    Sequence< Any > seqArguments(1);
    seqArguments[0] <<= xPlugInDLL;

    // create plugin
    Reference< XFrame > xPlugIn( xPlugInFactory->createInstanceWithArguments( seqArguments ), UNO_QUERY );
    LOG_ASSERT( !(xPlugIn.is()==sal_False), "TestApplication::impl_testPlugIn()\nFactory has created no valid plugin!\n" )

    xPlugIn->setName( OUString(RTL_CONSTASCII_USTRINGPARAM("PlugIn")) );
    Reference< XFramesSupplier > xSupplier( xDesktop, UNO_QUERY );
    xPlugIn->setCreator( xSupplier );
}
#endif

//_________________________________________________________________________________________________________________
//  test method
//_________________________________________________________________________________________________________________
#ifdef TEST_DESKTOP
#define LOGFILE_TARGETING "targeting.log"
void TestApplication::impl_testDesktop( const Reference< XDesktop >& xDesktop )
{
    //-------------------------------------------------------------------------------------------------------------
    //  1) Test cast to all supported interfaces of Desktop.

    Reference< XInterface >                 xDesktopInterface               ( xDesktop, UNO_QUERY );
    Reference< XTypeProvider >              xDesktopTypeProvider            ( xDesktop, UNO_QUERY );
    Reference< XServiceInfo >               xDesktopServiceInfo             ( xDesktop, UNO_QUERY );
    Reference< XComponentLoader >           xDesktopComponentLoader         ( xDesktop, UNO_QUERY );
    Reference< XTasksSupplier >             xDesktopTasksSupplier           ( xDesktop, UNO_QUERY );
    Reference< XDispatchProvider >          xDesktopDispatchProvider        ( xDesktop, UNO_QUERY );
    Reference< XFramesSupplier >            xDesktopFramesSupplier          ( xDesktop, UNO_QUERY );
    Reference< XFrame >                     xDesktopFrame                   ( xDesktop, UNO_QUERY );
    Reference< XComponent >                 xDesktopComponent               ( xDesktop, UNO_QUERY );
    Reference< XStatusIndicatorFactory >    xDesktopStatusIndicatorFactory  ( xDesktop, UNO_QUERY );
    Reference< XPropertySet >               xDesktopPropertySet             ( xDesktop, UNO_QUERY );
    Reference< XFastPropertySet >           xDesktopFastPropertySet         ( xDesktop, UNO_QUERY );
    Reference< XMultiPropertySet >          xDesktopMultiPropertySet        ( xDesktop, UNO_QUERY );

    LOG_ASSERT( !(xDesktopInterface.is()                ==sal_False), "TestApplication::impl_testDesktop()\nXInterface not supported by Desktop.\n\n"               )
    LOG_ASSERT( !(xDesktopTypeProvider.is()             ==sal_False), "TestApplication::impl_testDesktop()\nXTypeProvider not supported by Desktop.\n\n"            )
    LOG_ASSERT( !(xDesktopServiceInfo.is()              ==sal_False), "TestApplication::impl_testDesktop()\nXServiceInfo not supported by Desktop.\n\n"             )
    LOG_ASSERT( !(xDesktop.is()                         ==sal_False), "TestApplication::impl_testDesktop()\nXDesktop not supported by Desktop.\n\n"                 )
    LOG_ASSERT( !(xDesktopComponentLoader.is()          ==sal_False), "TestApplication::impl_testDesktop()\nXComponentLoader not supported by Desktop.\n\n"         )
    LOG_ASSERT( !(xDesktopTasksSupplier.is()            ==sal_False), "TestApplication::impl_testDesktop()\nXTasksSupplier not supported by Desktop.\n\n"           )
    LOG_ASSERT( !(xDesktopDispatchProvider.is()         ==sal_False), "TestApplication::impl_testDesktop()\nXDispatchProvider not supported by Desktop.\n\n"        )
    LOG_ASSERT( !(xDesktopFramesSupplier.is()           ==sal_False), "TestApplication::impl_testDesktop()\nXFramesSupplier not supported by Desktop.\n\n"          )
    LOG_ASSERT( !(xDesktopFrame.is()                    ==sal_False), "TestApplication::impl_testDesktop()\nXFrame not supported by Desktop.\n\n"                   )
    LOG_ASSERT( !(xDesktopComponent.is()                ==sal_False), "TestApplication::impl_testDesktop()\nXComponent not supported by Desktop.\n\n"               )
    LOG_ASSERT( !(xDesktopStatusIndicatorFactory.is()   ==sal_False), "TestApplication::impl_testDesktop()\nXStatusIndicatorFactory not supported by Desktop.\n\n"  )
    LOG_ASSERT( !(xDesktopPropertySet.is()              ==sal_False), "TestApplication::impl_testDesktop()\nXPropertySet not supported by Desktop.\n\n"             )
    LOG_ASSERT( !(xDesktopFastPropertySet.is()          ==sal_False), "TestApplication::impl_testDesktop()\nXFastPropertySet not supported by Desktop.\n\n"         )
    LOG_ASSERT( !(xDesktopMultiPropertySet.is()         ==sal_False), "TestApplication::impl_testDesktop()\nXMultiPropertySet not supported by Desktop.\n\n"        )

    //-------------------------------------------------------------------------------------------------------------
    //  2) Test set-/getName().

    //  2a) Test default value "Desktop".
    OUString sName( RTL_CONSTASCII_USTRINGPARAM("Desktop") );
    LOG_ASSERT( !(xDesktopFrame->getName()!=sName), "TestApplication::impl_testDesktop()\nDefault value of desktop name is invalid.\n\n" )

    //  2b) Set name and try to get the same name.
    sName = OUString( RTL_CONSTASCII_USTRINGPARAM("New Desktop") );
    xDesktopFrame->setName( sName );
    LOG_ASSERT( !(xDesktopFrame->getName()!=sName), "TestApplication::impl_testDesktop()\nSetting of name works not correct on desktop.\n\n" )

    //  Reset name do default!
    //  Its neccessary for follow operations.
    sName = OUString( RTL_CONSTASCII_USTRINGPARAM("Desktop") );
    xDesktopFrame->setName( sName );

    //-------------------------------------------------------------------------------------------------------------
    //  3) Test findFrame().

    //  Build new example tree and log initial structure in file.
    impl_buildTree  ( xDesktop );
    impl_logTree    ( xDesktop );

    OUString            sTargetFrameName;
    sal_uInt32          nSearchFlags    ;
    Reference< XFrame > xSearchFrame    ;
    Reference< XFrame > xTask_4         ;
    Reference< XFrame > xFrame_41       ;
    Reference< XFrame > xFrame_411      ;
    Reference< XFrame > xFrame_112      ;
    Reference< XFrame > xFrame_1123     ;
    Reference< XFrame > xFrame_11231    ;
    Reference< XFrame > xFrame_11221    ;

    //  3a) Search for Desktop
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with SELF for \"Desktop\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    sTargetFrameName    = OUString(RTL_CONSTASCII_USTRINGPARAM("Desktop"));
    nSearchFlags        = FrameSearchFlag::SELF;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_False), "TestApplication::impl_testDesktop()\nSearch 3a)-1 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with ALL for \"Desktop\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::ALL;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_False), "TestApplication::impl_testDesktop()\nSearch 3a)-2 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with CHILDREN for \"Desktop\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::CHILDREN;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_True), "TestApplication::impl_testDesktop()\nSearch 3a)-3 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with SIBLINGS for \"Desktop\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::SIBLINGS;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_True), "TestApplication::impl_testDesktop()\nSearch 3a)-4 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with AUTO for \"Desktop\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::AUTO;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_True), "TestApplication::impl_testDesktop()\nSearch 3a)-5 invalid\n" );

    //  3b) Search for Task_4
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with SELF for \"Task_4\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    sTargetFrameName    = OUString(RTL_CONSTASCII_USTRINGPARAM("Task_4"));
    nSearchFlags        = FrameSearchFlag::SELF;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_True), "TestApplication::impl_testDesktop()\nSearch 3b)-1 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with SIBLINGS for \"Task_4\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::SIBLINGS;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_True), "TestApplication::impl_testDesktop()\nSearch 3b)-2 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with CHILDREN for \"Task_4\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::CHILDREN;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_False), "TestApplication::impl_testDesktop()\nSearch 3b)-3 invalid\n" );

    xTask_4 = xSearchFrame;

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with ALL for \"Task_4\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::ALL;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_False), "TestApplication::impl_testDesktop()\nSearch 3b)-4 invalid\n" );

    //  3c) Search for Frame_41
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with SELF for \"Frame_41\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    sTargetFrameName    = OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_41"));
    nSearchFlags        = FrameSearchFlag::SELF;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_True), "TestApplication::impl_testDesktop()\nSearch 3c)-1 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with SIBLINGS for \"Frame_41\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::SIBLINGS;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_True), "TestApplication::impl_testDesktop()\nSearch 3c)-2 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with CHILDREN for \"Frame_41\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::CHILDREN;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_False), "TestApplication::impl_testDesktop()\nSearch 3c)-3 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with ALL for \"Frame_41\" on Desktop\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::ALL;
    xSearchFrame        = xDesktopFrame->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_False), "TestApplication::impl_testDesktop()\nSearch 3c)-4 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with SELF for \"Frame_41\" on Task_4\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::SELF;
    xSearchFrame        = xTask_4->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_True), "TestApplication::impl_testDesktop()\nSearch 3c)-5 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with SIBLINGS for \"Frame_41\" on Task_4\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::SIBLINGS;
    xSearchFrame        = xTask_4->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_True), "TestApplication::impl_testDesktop()\nSearch 3c)-6 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with CHILDREN for \"Frame_41\" on Task_4\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::CHILDREN;
    xSearchFrame        = xTask_4->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_False), "TestApplication::impl_testDesktop()\nSearch 3c)-7 invalid\n" );

    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, " Search with ALL for \"Frame_41\" on Task_4\n" )
    WRITE_LOGFILE( LOGFILE_TARGETING, "--------------------------------------------------------------------------------\n" )
    nSearchFlags        = FrameSearchFlag::ALL;
    xSearchFrame        = xTask_4->findFrame(sTargetFrameName, nSearchFlags);
    LOG_ASSERT( !(xSearchFrame.is()==sal_False), "TestApplication::impl_testDesktop()\nSearch 3c)-8 invalid\n" );

    xFrame_41 = xSearchFrame;

    //  3d) Search for Frame_411
    //  3e) Search for Frame_112
    //  3f) Search for Frame_1123
    //  3g) Search for Frame_11231
    //  3h) Search for Frame_11221
}

//_________________________________________________________________________________________________________________
//  helper method to build a new binaer-tree with desktop as top-frame
//_________________________________________________________________________________________________________________
void TestApplication::impl_buildTree( const Reference< XDesktop >& xDesktop )
{
    // You can append and remove frames only on XFRames interface of desktop.
    // But the desktop support this interface not directly! Use getFrames() instantly.
    Reference< XFramesSupplier > xDesktopSupplier( xDesktop, UNO_QUERY );
    Reference< XFrames > xFrames = xDesktopSupplier->getFrames();
    LOG_ASSERT( !(xFrames.is()==sal_False), "TestApplication::impl_buildTree()\nCan't get framesaccess on desktop.\n\n" )

    // Create some tasks and frames.
    Reference< XMultiServiceFactory > xServiceManager( getProcessServiceFactory() );

    Reference< XTask >  xTask_1( xServiceManager->createInstance(SERVICENAME_TASK), UNO_QUERY );
    Reference< XTask >  xTask_2( xServiceManager->createInstance(SERVICENAME_TASK), UNO_QUERY );
    Reference< XTask >  xTask_3( xServiceManager->createInstance(SERVICENAME_TASK), UNO_QUERY );
    Reference< XTask >  xTask_4( xServiceManager->createInstance(SERVICENAME_TASK), UNO_QUERY );
    Reference< XTask >  xTask_5( xServiceManager->createInstance(SERVICENAME_TASK), UNO_QUERY );
    Reference< XFrame > xFrame_11( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_12( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_21( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_22( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_31( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_32( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_41( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_42( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_51( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_52( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_111( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_112( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_121( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_411( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_1121( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_1122( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_1123( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_4111( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_4112( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_4113( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_11221( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );
    Reference< XFrame > xFrame_11231( xServiceManager->createInstance(SERVICENAME_FRAME), UNO_QUERY );

    // Initialize tasks and frames with names to support easy finding!
    Reference< XFrame > xTaskFrame_1( xTask_1, UNO_QUERY );
    Reference< XFrame > xTaskFrame_2( xTask_2, UNO_QUERY );
    Reference< XFrame > xTaskFrame_3( xTask_3, UNO_QUERY );
    Reference< XFrame > xTaskFrame_4( xTask_4, UNO_QUERY );
    Reference< XFrame > xTaskFrame_5( xTask_5, UNO_QUERY );

    xTaskFrame_1->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Task_1")));
    xTaskFrame_2->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Task_2")));
    xTaskFrame_3->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Task_3")));
    xTaskFrame_4->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Task_4")));
    xTaskFrame_5->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Task_5")));
    xFrame_11->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_11")));
    xFrame_12->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_12")));
    xFrame_21->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_21")));
    xFrame_22->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_22")));
    xFrame_31->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_31")));
    xFrame_32->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_32")));
    xFrame_41->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_41")));
    xFrame_42->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_42")));
    xFrame_51->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_51")));
    xFrame_52->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_52")));
    xFrame_111->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_111")));
    xFrame_112->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_112")));
    xFrame_121->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_121")));
    xFrame_411->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_411")));
    xFrame_1121->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_1121")));
    xFrame_1122->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_1122")));
    xFrame_1123->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_1123")));
    xFrame_4111->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_4111")));
    xFrame_4112->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_4112")));
    xFrame_4113->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_4113")));
    xFrame_11221->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_11221")));
    xFrame_11231->setName(OUString(RTL_CONSTASCII_USTRINGPARAM("Frame_11231")));

    // Build tree.
    // Append tasks as childs of desktop.
    xFrames->append( xTaskFrame_1 );
    xFrames->append( xTaskFrame_2 );
    xFrames->append( xTaskFrame_3 );
    xFrames->append( xTaskFrame_4 );
    xFrames->append( xTaskFrame_5 );

    // Append frames as childs of tasks.
    Reference< XFramesSupplier > xTaskFramesSupplier_1( xTask_1, UNO_QUERY );
    Reference< XFramesSupplier > xTaskFramesSupplier_2( xTask_2, UNO_QUERY );
    Reference< XFramesSupplier > xTaskFramesSupplier_3( xTask_3, UNO_QUERY );
    Reference< XFramesSupplier > xTaskFramesSupplier_4( xTask_4, UNO_QUERY );
    Reference< XFramesSupplier > xTaskFramesSupplier_5( xTask_5, UNO_QUERY );
    Reference< XFramesSupplier > xFramesSupplier_11( xFrame_11, UNO_QUERY );
    Reference< XFramesSupplier > xFramesSupplier_12( xFrame_12, UNO_QUERY );
    Reference< XFramesSupplier > xFramesSupplier_112( xFrame_112, UNO_QUERY );
    Reference< XFramesSupplier > xFramesSupplier_1122( xFrame_1122, UNO_QUERY );
    Reference< XFramesSupplier > xFramesSupplier_1123( xFrame_1123, UNO_QUERY );
    Reference< XFramesSupplier > xFramesSupplier_41( xFrame_41, UNO_QUERY );
    Reference< XFramesSupplier > xFramesSupplier_411( xFrame_411, UNO_QUERY );

    xFrames = xTaskFramesSupplier_1->getFrames();
    xFrames->append( xFrame_11 );
    xFrames->append( xFrame_12 );
    xFrames = xTaskFramesSupplier_2->getFrames();
    xFrames->append( xFrame_21 );
    xFrames->append( xFrame_22 );
    xFrames = xTaskFramesSupplier_3->getFrames();
    xFrames->append( xFrame_31 );
    xFrames->append( xFrame_32 );
    xFrames = xTaskFramesSupplier_4->getFrames();
    xFrames->append( xFrame_41 );
    xFrames->append( xFrame_42 );
    xFrames = xTaskFramesSupplier_5->getFrames();
    xFrames->append( xFrame_51 );
    xFrames->append( xFrame_52 );
    xFrames = xFramesSupplier_11->getFrames();
    xFrames->append( xFrame_111 );
    xFrames->append( xFrame_112 );
    xFrames = xFramesSupplier_12->getFrames();
    xFrames->append( xFrame_121 );
    xFrames = xFramesSupplier_112->getFrames();
    xFrames->append( xFrame_1121 );
    xFrames->append( xFrame_1122 );
    xFrames->append( xFrame_1123 );
    xFrames = xFramesSupplier_1122->getFrames();
    xFrames->append( xFrame_11221 );
    xFrames = xFramesSupplier_1123->getFrames();
    xFrames->append( xFrame_11231 );
    xFrames = xFramesSupplier_41->getFrames();
    xFrames->append( xFrame_411 );
    xFrames = xFramesSupplier_411->getFrames();
    xFrames->append( xFrame_4111 );
    xFrames->append( xFrame_4112 );
    xFrames->append( xFrame_4113 );

    // Create some active paths.
    // desktop => task_1 => frame_11 => frame_112 => frame_1122
//  xDesktopSupplier->setActiveFrame( xTaskFrame_1 );
//  xTaskFramesSupplier_1->setActiveFrame( xFrame_11 );
//  xFramesSupplier_11->setActiveFrame( xFrame_112 );
//  xFramesSupplier_112->setActiveFrame( xFrame_1122 );
    // frame_41 => frame_411 => frame_4111
//  xFramesSupplier_41->setActiveFrame( xFrame_411 );
//  xFramesSupplier_411->setActiveFrame( xFrame_4111 );
    // task_3 => frame_31
//  xTaskFramesSupplier_3->setActiveFrame( xFrame_31 );
    // frame_1123 => frame_11231
//  xFramesSupplier_1123->setActiveFrame( xFrame_11231 );

//  xTask_1->activate();
//  impl_logTree( xDesktop );
//  xFrame_41->activate();
//  impl_logTree( xDesktop );
//  xFrame_52->activate();
//  impl_logTree( xDesktop );
//  WRITE_LOGFILE( LOGFILENAME_TREE, "initiale Hierarchy:" )
    impl_logTree( xDesktop );

//  xFrame_121->activate();
//  impl_logTree( xDesktop );
//  xFrame_41->activate();
//  impl_logTree( xDesktop );
//  xFrame_52->activate();
//  impl_logTree( xDesktop );
//  WRITE_LOGFILE( LOGFILENAME_EVENTS, "nach xFrame_411->activate():" )
    xFrame_411->activate();
//  WRITE_LOGFILE( LOGFILENAME_TREE, "nach xFrame_411->activate():" )
    impl_logTree( xDesktop );
//  xFrame_41->deactivate();
//  impl_logTree( xDesktop );
//  xFrame_4113->activate();
//  impl_logTree( xDesktop );
//  xFrame_21->activate();
//  impl_logTree( xDesktop );
//  WRITE_LOGFILE( LOGFILENAME_EVENTS, "nach xFrame_11231->activate():" )
    xFrame_11231->activate();
//  WRITE_LOGFILE( LOGFILENAME_TREE, "nach xFrame_11231->activate():" )
    impl_logTree( xDesktop );

//  WRITE_LOGFILE( LOGFILENAME_EVENTS, "nach xFrame_11221->activate():" )
    xFrame_11221->activate();
//  WRITE_LOGFILE( LOGFILENAME_TREE, "nach xFrame_11221->activate():" )
    impl_logTree( xDesktop );

//  WRITE_LOGFILE( LOGFILENAME_EVENTS, "nach xFrame_112->deactivate():" )
//  xFrame_112->getCreator()->setActiveFrame( Reference< XFrame >() );
    xFrame_112->deactivate();
//  WRITE_LOGFILE( LOGFILENAME_TREE, "nach xFrame_112->deactivate():" )
    impl_logTree( xDesktop );

//  WRITE_LOGFILE( LOGFILENAME_EVENTS, "nach xFrame_41->activate():" )
    xFrame_41->activate();
//  WRITE_LOGFILE( LOGFILENAME_TREE, "nach xFrame_41->activate():" )
    impl_logTree( xDesktop );

//  WRITE_LOGFILE( LOGFILENAME_EVENTS, "nach xTask_4->activate():" )
    xTask_4->activate();
//  WRITE_LOGFILE( LOGFILENAME_TREE, "nach xTask_4->activate():" )
    impl_logTree( xDesktop );

//  WRITE_LOGFILE( LOGFILENAME_EVENTS, "nach xFrame_1123->deactivate():" )
    xFrame_1123->deactivate();
//  WRITE_LOGFILE( LOGFILENAME_TREE, "nach xFrame_1123->deactivate():" )
    impl_logTree( xDesktop );
}

//_________________________________________________________________________________________________________________
//  helper method to log current tree state
//_________________________________________________________________________________________________________________
void TestApplication::impl_logTree( const Reference< XDesktop >& xDesktop )
{
#ifdef ENABLE_SERVICEDEBUG
    // Use special feature of Desktop-implementation.
    // NEVER USE THIS IN RELEASE VERSIONS!!!
    Reference< XSPECIALDEBUGINTERFACE > xDebug( xDesktop, UNO_QUERY );

    // Get a "stream" of all names of frames in tree.
    OUString sTreeNamesStream = xDebug->dumpVariable( DUMPVARIABLE_TREEINFO, 0 );
    // And write it to logfile.
    OString sOutPut = OUStringToOString( sTreeNamesStream, RTL_TEXTENCODING_UTF8 );
//  WRITE_LOGFILE( LOGFILENAME_TREE, "\nNew tree log:\n\n"  );
//  WRITE_LOGFILE( LOGFILENAME_TREE, sOutPut.getStr()           );
//  WRITE_LOGFILE( LOGFILENAME_TREE, "\n"                       );
#endif
}
#endif // TEST_DESKTOP

//_________________________________________________________________________________________________________________
//  test method for registration of new filters in configuration
//_________________________________________________________________________________________________________________
#ifdef TEST_FILTERREGISTRATION
void TestApplication::impl_testFilterRegistration()
{
    Reference< XNameContainer > xContainer( m_xFactory->createInstance( SERVICENAME_FILTERFACTORY ), UNO_QUERY );
    LOG_ASSERT( !(xContainer.is()==sal_False), "TestApplication::impl_testFilterRegistration()\nCould not create FilterFactory-service or cast it to XNameContainer.\n" )
    if( xContainer.is() == sal_True )
    {
        Sequence< PropertyValue > lProperties( 8 );

        lProperties[0].Name     =   DECLARE_ASCII("Type"                    );
        lProperties[0].Value    <<= DECLARE_ASCII("component_DB"            );

        lProperties[1].Name     =   DECLARE_ASCII("UIName"                  );
        lProperties[1].Value    <<= DECLARE_ASCII("Ein neuer Filter-Eintrag");

        lProperties[2].Name     =   DECLARE_ASCII("DocumentService"         );
        lProperties[2].Value    <<= DECLARE_ASCII("test.document.service"   );

        lProperties[3].Name     =   DECLARE_ASCII("FilterService"           );
        lProperties[3].Value    <<= DECLARE_ASCII("test.filter.service"     );

        lProperties[4].Name     =   DECLARE_ASCII("Flags"                   );
        lProperties[4].Value    <<= (sal_Int32)100;

        Sequence< OUString > lTempData(1);
        lTempData[0] = DECLARE_ASCII("meine UserData");
        lProperties[5].Name     =   DECLARE_ASCII("UserData"                );
        lProperties[5].Value    <<= lTempData;

        lProperties[6].Name     =   DECLARE_ASCII("FileFormatVersion"       );
        lProperties[6].Value    <<= (sal_Int32)1;

        lProperties[7].Name     =   DECLARE_ASCII("TemplateName"            );
        lProperties[7].Value    <<= DECLARE_ASCII("Mein Template Name"      );

        Any aProperties;
        aProperties <<= lProperties;
        xContainer->insertByName( DECLARE_ASCII("mein_eigener_neuer_Filter"), aProperties );

        lProperties[0].Name     =   DECLARE_ASCII("Type"                    );
        lProperties[0].Value    <<= DECLARE_ASCII("component_DB"            );

        lProperties[1].Name     =   DECLARE_ASCII("UIName"                  );
        lProperties[1].Value    <<= DECLARE_ASCII("Ein neuer Filter-Eintrag 2");

        lProperties[2].Name     =   DECLARE_ASCII("DocumentService"         );
        lProperties[2].Value    <<= DECLARE_ASCII("test.document.service 2" );

        lProperties[3].Name     =   DECLARE_ASCII("FilterService"           );
        lProperties[3].Value    <<= DECLARE_ASCII("test.filter.service 2"       );

        lProperties[4].Name     =   DECLARE_ASCII("Flags"                   );
        lProperties[4].Value    <<= (sal_Int32)200;

        lTempData[0] = DECLARE_ASCII("meine UserData 2");
        lProperties[5].Name     =   DECLARE_ASCII("UserData"                );
        lProperties[5].Value    <<= lTempData;

        lProperties[6].Name     =   DECLARE_ASCII("FileFormatVersion"       );
        lProperties[6].Value    <<= (sal_Int32)2;

        lProperties[7].Name     =   DECLARE_ASCII("TemplateName"            );
        lProperties[7].Value    <<= DECLARE_ASCII("Mein Template Name 2"        );

        aProperties <<= lProperties;
        xContainer->insertByName( DECLARE_ASCII("mein_eigener_neuer_Filter_2"), aProperties );

        xContainer->removeByName( DECLARE_ASCII("mein_eigener_neuer_Filter") );
    }
}
#endif

//_________________________________________________________________________________________________________________
//  test method for search mechanism in our frame tree
//_________________________________________________________________________________________________________________
#ifdef TEST_TREESEARCH
sal_Bool TestApplication::impl_testTreeSearch()
{
    // Build an example tree.
    Reference< XFrame > xD      ( m_xFactory->createInstance( SERVICENAME_DESKTOP   ), UNO_QUERY );
    Reference< XFrame > xT1     ( m_xFactory->createInstance( SERVICENAME_TASK      ), UNO_QUERY );
    Reference< XFrame > xT2     ( m_xFactory->createInstance( SERVICENAME_TASK      ), UNO_QUERY );
    Reference< XFrame > xT3     ( m_xFactory->createInstance( SERVICENAME_TASK      ), UNO_QUERY );
    Reference< XFrame > xF11    ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF12    ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF21    ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF22    ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF211   ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF212   ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF221   ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF2111  ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF2112  ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF2121  ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF2122  ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF2211  ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF21111 ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF21112 ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF21121 ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF21122 ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF21211 ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF21212 ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF21221 ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF21222 ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );
    Reference< XFrame > xF22111 ( m_xFactory->createInstance( SERVICENAME_FRAME     ), UNO_QUERY );

    xD->setName     ( DECLARE_ASCII("D"     ) );
    xT1->setName    ( DECLARE_ASCII("T1"    ) );
    xT2->setName    ( DECLARE_ASCII("T2"    ) );
    xT3->setName    ( DECLARE_ASCII("T3"    ) );
    xF11->setName   ( DECLARE_ASCII("F11"   ) );
    xF12->setName   ( DECLARE_ASCII("F12"   ) );
    xF21->setName   ( DECLARE_ASCII("F21"   ) );
    xF22->setName   ( DECLARE_ASCII("F22"   ) );
    xF211->setName  ( DECLARE_ASCII("F211"  ) );
    xF212->setName  ( DECLARE_ASCII("F212"  ) );
    xF221->setName  ( DECLARE_ASCII("F221"  ) );
    xF2111->setName ( DECLARE_ASCII("F2111" ) );
    xF2112->setName ( DECLARE_ASCII("F2112" ) );
    xF2121->setName ( DECLARE_ASCII("F2121" ) );
    xF2122->setName ( DECLARE_ASCII("F2122" ) );
    xF2211->setName ( DECLARE_ASCII("F2211" ) );
    xF21111->setName( DECLARE_ASCII("F21111") );
    xF21112->setName( DECLARE_ASCII("F21112") );
    xF21121->setName( DECLARE_ASCII("F21121") );
    xF21122->setName( DECLARE_ASCII("F21122") );
    xF21211->setName( DECLARE_ASCII("F21211") );
    xF21212->setName( DECLARE_ASCII("F21212") );
    xF21221->setName( DECLARE_ASCII("F21221") );
    xF21222->setName( DECLARE_ASCII("F21222") );
    xF22111->setName( DECLARE_ASCII("F22111") );

    Reference< XFramesSupplier > xSD        ( xD        , UNO_QUERY );
    Reference< XFramesSupplier > xST1       ( xT1       , UNO_QUERY );
    Reference< XFramesSupplier > xST2       ( xT2       , UNO_QUERY );
    Reference< XFramesSupplier > xST3       ( xT3       , UNO_QUERY );
    Reference< XFramesSupplier > xSF11      ( xF11      , UNO_QUERY );
    Reference< XFramesSupplier > xSF12      ( xF12      , UNO_QUERY );
    Reference< XFramesSupplier > xSF21      ( xF21      , UNO_QUERY );
    Reference< XFramesSupplier > xSF22      ( xF22      , UNO_QUERY );
    Reference< XFramesSupplier > xSF211     ( xF211     , UNO_QUERY );
    Reference< XFramesSupplier > xSF212     ( xF212     , UNO_QUERY );
    Reference< XFramesSupplier > xSF221     ( xF221     , UNO_QUERY );
    Reference< XFramesSupplier > xSF2111    ( xF2111    , UNO_QUERY );
    Reference< XFramesSupplier > xSF2112    ( xF2112    , UNO_QUERY );
    Reference< XFramesSupplier > xSF2121    ( xF2121    , UNO_QUERY );
    Reference< XFramesSupplier > xSF2122    ( xF2122    , UNO_QUERY );
    Reference< XFramesSupplier > xSF2211    ( xF2211    , UNO_QUERY );
    Reference< XFramesSupplier > xSF21111   ( xF21111   , UNO_QUERY );
    Reference< XFramesSupplier > xSF21112   ( xF21112   , UNO_QUERY );
    Reference< XFramesSupplier > xSF21121   ( xF21121   , UNO_QUERY );
    Reference< XFramesSupplier > xSF21122   ( xF21122   , UNO_QUERY );
    Reference< XFramesSupplier > xSF21211   ( xF21211   , UNO_QUERY );
    Reference< XFramesSupplier > xSF21212   ( xF21212   , UNO_QUERY );
    Reference< XFramesSupplier > xSF21221   ( xF21221   , UNO_QUERY );
    Reference< XFramesSupplier > xSF21222   ( xF21222   , UNO_QUERY );
    Reference< XFramesSupplier > xSF22111   ( xF22111   , UNO_QUERY );

    xSD->getFrames()->append        ( xT1       );
    xSD->getFrames()->append        ( xT2       );
    xSD->getFrames()->append        ( xT3       );
    xST1->getFrames()->append       ( xF11      );
    xST1->getFrames()->append       ( xF12      );
    xST2->getFrames()->append       ( xF21      );
    xST2->getFrames()->append       ( xF22      );
    xSF21->getFrames()->append      ( xF211     );
    xSF21->getFrames()->append      ( xF212     );
    xSF211->getFrames()->append     ( xF2111    );
    xSF211->getFrames()->append     ( xF2112    );
    xSF212->getFrames()->append     ( xF2121    );
    xSF212->getFrames()->append     ( xF2122    );
    xSF2111->getFrames()->append    ( xF21111   );
    xSF2111->getFrames()->append    ( xF21112   );
    xSF2112->getFrames()->append    ( xF21121   );
    xSF2112->getFrames()->append    ( xF21122   );
    xSF2121->getFrames()->append    ( xF21211   );
    xSF2121->getFrames()->append    ( xF21212   );
    xSF2122->getFrames()->append    ( xF21221   );
    xSF2122->getFrames()->append    ( xF21222   );
    xSF22->getFrames()->append      ( xF221     );
    xSF221->getFrames()->append     ( xF2211    );
    xSF2211->getFrames()->append    ( xF22111   );

    sal_Int32 nFlags = 0;

    // Test deep down search
    nFlags = FrameSearchFlag::CHILDREN;
    if  (
            ( xD->findFrame( DECLARE_ASCII("T1"     ), nFlags ) !=  xT1     )   ||
            ( xD->findFrame( DECLARE_ASCII("T2"     ), nFlags ) !=  xT2     )   ||
            ( xD->findFrame( DECLARE_ASCII("T3"     ), nFlags ) !=  xT3     )   ||
            ( xD->findFrame( DECLARE_ASCII("F11"    ), nFlags ) !=  xF11    )   ||
            ( xD->findFrame( DECLARE_ASCII("F12"    ), nFlags ) !=  xF12    )   ||
            ( xD->findFrame( DECLARE_ASCII("F21"    ), nFlags ) !=  xF21    )   ||
            ( xD->findFrame( DECLARE_ASCII("F22"    ), nFlags ) !=  xF22    )   ||
            ( xD->findFrame( DECLARE_ASCII("F211"   ), nFlags ) !=  xF211   )   ||
            ( xD->findFrame( DECLARE_ASCII("F212"   ), nFlags ) !=  xF212   )   ||
            ( xD->findFrame( DECLARE_ASCII("F2111"  ), nFlags ) !=  xF2111  )   ||
            ( xD->findFrame( DECLARE_ASCII("F2112"  ), nFlags ) !=  xF2112  )   ||
            ( xD->findFrame( DECLARE_ASCII("F2121"  ), nFlags ) !=  xF2121  )   ||
            ( xD->findFrame( DECLARE_ASCII("F2122"  ), nFlags ) !=  xF2122  )   ||
            ( xD->findFrame( DECLARE_ASCII("F21111" ), nFlags ) !=  xF21111 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21112" ), nFlags ) !=  xF21112 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21121" ), nFlags ) !=  xF21121 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21122" ), nFlags ) !=  xF21122 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21211" ), nFlags ) !=  xF21211 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21212" ), nFlags ) !=  xF21212 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21221" ), nFlags ) !=  xF21221 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21222" ), nFlags ) !=  xF21222 )   ||
            ( xD->findFrame( DECLARE_ASCII("F221"   ), nFlags ) !=  xF221   )   ||
            ( xD->findFrame( DECLARE_ASCII("F2211"  ), nFlags ) !=  xF2211  )   ||
            ( xD->findFrame( DECLARE_ASCII("F22111" ), nFlags ) !=  xF22111 )
        )
    {
        LOG_ERROR( "TestApplikation::impl_testTreeSearch()", "deep down search failed" )
        return sal_False;
    }

    // Test flat down search
    nFlags = FrameSearchFlag::CHILDREN | FrameSearchFlag::SIBLINGS;
    if  (
            ( xD->findFrame( DECLARE_ASCII("T1"     ), nFlags ) !=  xT1     )   ||
            ( xD->findFrame( DECLARE_ASCII("T2"     ), nFlags ) !=  xT2     )   ||
            ( xD->findFrame( DECLARE_ASCII("T3"     ), nFlags ) !=  xT3     )   ||
            ( xD->findFrame( DECLARE_ASCII("F11"    ), nFlags ) !=  xF11    )   ||
            ( xD->findFrame( DECLARE_ASCII("F12"    ), nFlags ) !=  xF12    )   ||
            ( xD->findFrame( DECLARE_ASCII("F21"    ), nFlags ) !=  xF21    )   ||
            ( xD->findFrame( DECLARE_ASCII("F22"    ), nFlags ) !=  xF22    )   ||
            ( xD->findFrame( DECLARE_ASCII("F211"   ), nFlags ) !=  xF211   )   ||
            ( xD->findFrame( DECLARE_ASCII("F212"   ), nFlags ) !=  xF212   )   ||
            ( xD->findFrame( DECLARE_ASCII("F2111"  ), nFlags ) !=  xF2111  )   ||
            ( xD->findFrame( DECLARE_ASCII("F2112"  ), nFlags ) !=  xF2112  )   ||
            ( xD->findFrame( DECLARE_ASCII("F2121"  ), nFlags ) !=  xF2121  )   ||
            ( xD->findFrame( DECLARE_ASCII("F2122"  ), nFlags ) !=  xF2122  )   ||
            ( xD->findFrame( DECLARE_ASCII("F21111" ), nFlags ) !=  xF21111 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21112" ), nFlags ) !=  xF21112 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21121" ), nFlags ) !=  xF21121 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21122" ), nFlags ) !=  xF21122 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21211" ), nFlags ) !=  xF21211 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21212" ), nFlags ) !=  xF21212 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21221" ), nFlags ) !=  xF21221 )   ||
            ( xD->findFrame( DECLARE_ASCII("F21222" ), nFlags ) !=  xF21222 )   ||
            ( xD->findFrame( DECLARE_ASCII("F221"   ), nFlags ) !=  xF221   )   ||
            ( xD->findFrame( DECLARE_ASCII("F2211"  ), nFlags ) !=  xF2211  )   ||
            ( xD->findFrame( DECLARE_ASCII("F22111" ), nFlags ) !=  xF22111 )
        )
    {
        LOG_ERROR( "TestApplikation::impl_testTreeSearch()", "flat down search failed" )
        return sal_False;
    }

    // Test deep up search
    // All targets must be found. Control search steps in log files!
    nFlags = FrameSearchFlag::PARENT;
    if  (
            ( xF11->findFrame   ( DECLARE_ASCII("T1"), nFlags ) !=  xT1 )   ||  // search for valid targets
            ( xF12->findFrame   ( DECLARE_ASCII("T1"), nFlags ) !=  xT1 )   ||
            ( xF21->findFrame   ( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF22->findFrame   ( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF211->findFrame  ( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF212->findFrame  ( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF221->findFrame  ( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF2111->findFrame ( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF2121->findFrame ( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF2122->findFrame ( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF2211->findFrame ( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF21111->findFrame( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF21112->findFrame( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF21121->findFrame( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF21122->findFrame( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF21211->findFrame( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF21212->findFrame( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF21221->findFrame( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF21222->findFrame( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF22111->findFrame( DECLARE_ASCII("T2"), nFlags ) !=  xT2 )   ||
            ( xF11->findFrame   ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||  // search for existing but non valid targets
            ( xF12->findFrame   ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF21->findFrame   ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF22->findFrame   ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF211->findFrame  ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF212->findFrame  ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF221->findFrame  ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF2111->findFrame ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF2121->findFrame ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF2122->findFrame ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF2211->findFrame ( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF21111->findFrame( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF21112->findFrame( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF21121->findFrame( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF21122->findFrame( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF21211->findFrame( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF21212->findFrame( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF21221->findFrame( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF21222->findFrame( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )   ||
            ( xF22111->findFrame( DECLARE_ASCII("T3"), nFlags ).is()    ==  sal_True    )
        )
    {
        LOG_ERROR( "TestApplikation::impl_testTreeSearch()", "deep up search failed" )
        return sal_False;
    }

    // Test inside/outside tasks search
    // No frames outside current task should be found if TASKS flag isnt set.
    // Otherwise he must be found!
    if  (
            ( xF21211->findFrame( DECLARE_ASCII("F12"   ),  FrameSearchFlag::ALL                                )       == xF12     )   ||
            ( xF21211->findFrame( DECLARE_ASCII("F22111"),  FrameSearchFlag::GLOBAL                             )       != xF22111  )   ||
            ( xF21211->findFrame( DECLARE_ASCII("T4"    ),  FrameSearchFlag::GLOBAL | FrameSearchFlag::CREATE   ).is()  == sal_False)
        )
    {
        LOG_ERROR( "TestApplikation::impl_testTreeSearch()", "inside/outside task search failed" )
        return sal_False;
    }

    // Test SELF
    // Use the desktop, one task and one frame node to do that.
    // The desktop must ignore these question ... all other must return himself.
    LOG_ASSERT2( (xD->findFrame     (   DECLARE_ASCII(""),  FrameSearchFlag::SELF   )   == xD   ), "TestApplication::impl_testTreeSearch()", "SELF search for D failed\n"   )
    LOG_ASSERT2( (xT1->findFrame    (   DECLARE_ASCII(""),  FrameSearchFlag::SELF   )   != xT1  ), "TestApplication::impl_testTreeSearch()", "SELF search for T1 failed\n"  )
    LOG_ASSERT2( (xF12->findFrame   (   DECLARE_ASCII(""),  FrameSearchFlag::SELF   )   != xF12 ), "TestApplication::impl_testTreeSearch()", "SELF search for F12 failed\n" )

    // Test special task search at desktop
    // These search allow TASKS and CREATE flags only!
    // We make no deep search - we work on direct children of desktop only.
    // Supported for desktop only.
    LOG_ASSERT2( (xD->findFrame     (   DECLARE_ASCII("T1"),    FrameSearchFlag::TASKS                              )       != xT1          ), "TestApplication::impl_testTreeSearch()", "special TASKS search for T1 failed\n"             )
    LOG_ASSERT2( (xD->findFrame     (   DECLARE_ASCII("T2"),    FrameSearchFlag::TASKS                              )       != xT2          ), "TestApplication::impl_testTreeSearch()", "special TASKS search for T2 failed\n"             )
    LOG_ASSERT2( (xD->findFrame     (   DECLARE_ASCII("T3"),    FrameSearchFlag::TASKS                              )       != xT3          ), "TestApplication::impl_testTreeSearch()", "special TASKS search for T3 failed\n"             )
    // Attention: T4 was created before!
    LOG_ASSERT2( (xD->findFrame     (   DECLARE_ASCII("T5"),    FrameSearchFlag::TASKS                              ).is()  == sal_True     ), "TestApplication::impl_testTreeSearch()", "special TASKS search for T5 failed\n"             )
    LOG_ASSERT2( (xD->findFrame     (   DECLARE_ASCII("T5"),    FrameSearchFlag::TASKS | FrameSearchFlag::CREATE    ).is()  == sal_False    ), "TestApplication::impl_testTreeSearch()", "special TASKS+CREATE search for T5 failed\n"      )
    LOG_ASSERT2( (xD->findFrame     (   DECLARE_ASCII("F12"),   FrameSearchFlag::TASKS                              ).is()  == sal_True     ), "TestApplication::impl_testTreeSearch()", "special TASKS search for F12 failed\n"            )
    LOG_ASSERT2( (xF12->findFrame   (   DECLARE_ASCII("T1"),    FrameSearchFlag::TASKS                              ).is()  == sal_True     ), "TestApplication::impl_testTreeSearch()", "special TASKS search for T1 from F12 failed\n"    )
    LOG_ASSERT2( (xF22111->findFrame(   DECLARE_ASCII("T1"),    FrameSearchFlag::TASKS                              ).is()  == sal_True     ), "TestApplication::impl_testTreeSearch()", "special TASKS search for T1 from F22111 failed\n" )

    return sal_True;
}
#endif
