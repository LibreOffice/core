/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: so_instance.cxx,v $
 * $Revision: 1.9 $
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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/awt/XSystemChildFactory.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <tools/debug.hxx>
#include <tools/color.hxx>
#include <vcl/window.hxx>
#include <rtl/textenc.h>
#include <rtl/locale.h>
#include <osl/nlsupport.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "so_instance.hxx"
#include "ns_debug.hxx"
#include "so_env.hxx"
#include "nsp_windows.hxx"
#include "so_closelistener.hxx"
#include <osl/process.h>

namespace css=com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::connection;
using namespace cppu;
using namespace rtl;
using namespace com::sun::star;

char SoPluginInstance::sSO_Dir[] = {0};
Reference< XMultiServiceFactory > SoPluginInstance::mxRemoteMSF = Reference< XMultiServiceFactory >(NULL);
Reference< XMultiServiceFactory > SoPluginInstance::mxLocalMSF  = Reference< XMultiServiceFactory >(NULL);


extern "C"{
    sal_Bool restart_office(void);
}


SoPluginInstance::SoPluginInstance(long pParent):
    m_xUnoWin(NULL),
    m_xComponent(NULL),
    m_xFrame(NULL),
    m_xFrames(NULL),
    m_xDispatcher(NULL),
    m_xDispatchProvider(NULL),
    m_pCloseListener(NULL)
{
    m_nWidth = 0;
    m_nHeight = 0;
    m_nX = 0;
    m_nY = 0;
    m_nFlag = 15;
    m_bInit = sal_False;
    m_sURL = rtl::OUString::createFromAscii("");
    m_hParent = 0;
    m_pParent = pParent;
    m_dParentStyl = 0;
}

SoPluginInstance::~SoPluginInstance()
{
}

// Start listening staroffice and connect to it
sal_Bool SoPluginInstance::Connect()
{
    Reference< XComponentContext > xComponentContext;
    sal_Bool bRetval(sal_False);

    if(mxRemoteMSF.is())
    {
        debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, remote ServiceManager has been created. no need to create it again.\n");
        return sal_True;
    }
    debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  try to create defaultBootstrap_InitialComponentContext.\n");
/*
#ifdef UNIX
    debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  HOME:%s; LANG:%s; LC_ALL:%s; LC_COLLATE:%s; PATH:%s; current dir:%s \n",
        getenv("HOME"), getenv("LANG"), getenv("LC_ALL"), getenv("LC_COLLATE"), getenv("PATH"), get_current_dir_name());
#endif // end of UNIX
*/
    //create local service manager
    if(!mxLocalMSF.is())
    {
#ifdef WNT
        xComponentContext = defaultBootstrap_InitialComponentContext(
             OUString(RTL_CONSTASCII_USTRINGPARAM("uno.ini")) );
#else
        xComponentContext = defaultBootstrap_InitialComponentContext(
             OUString(RTL_CONSTASCII_USTRINGPARAM("unorc")) );
#endif
        if (xComponentContext.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  try to create mxLocalMSF.\n");
            mxLocalMSF = Reference< XMultiServiceFactory >::query(xComponentContext->getServiceManager());
            bRetval = sal_True;
            if(!mxLocalMSF.is())
            {
                debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  create mxLocalMSF failure.\n");
                return sal_False;
            }
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  create mxLocalMSF success.\n");
        }
        else
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  try to create mxLocalMSF false.\n");
            return bRetval;
        }
    }

    //then try to connect to the remote StarOffice process
    bRetval = sal_True;
    try
    {
        const OUString sUnoUrlResolver(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.UnoUrlResolver"));
        char para[128] = {0};
        sprintf(para, "uno:socket,host=localhost,port=%d;urp;StarOffice.ServiceManager", SO_SERVER_PORT);
        const OUString sResolverArguments(OUString::createFromAscii(para));

        // Create UnoUrlResolver
        Reference < XInterface > rInterface = mxLocalMSF->createInstance(sUnoUrlResolver);
        if (!rInterface.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, create sUnoUrlResolver, failure.\n");
            return sal_False;
        }

        // Create XUnoUrlResolver by querying Interface of UnoUrlResolver
        Reference < com::sun::star::bridge::XUnoUrlResolver > rResolver(rInterface, UNO_QUERY);
        if(!rResolver.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, create XUnoUrlResolver, failure.\n");
            return sal_False;
        }

        // Resolve the arguments
        rInterface = rResolver->resolve(sResolverArguments);
        if(!rInterface.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, rResolver->resolve(sResolverArguments), failure\n");
            return sal_False;
        }

        // Create XPropertySet
        Reference< ::com::sun::star::beans::XPropertySet > xPropSet( rInterface, UNO_QUERY );
        if(!xPropSet.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, xPropSet( rInterface, UNO_QUERY ), failure\n");
            return sal_False;
        }

        // Get remote xComponentContext
        xPropSet->getPropertyValue( OUString::createFromAscii("DefaultContext") ) >>= xComponentContext;

        // Get the service manager from the remote context
        mxRemoteMSF =  Reference< XMultiServiceFactory >( xComponentContext->getServiceManager(), UNO_QUERY);
        if(!mxRemoteMSF.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, create mcRemoteMSF, failure.\n");
            bRetval = sal_False;
        }
    }
    catch (ConnectionSetupException& )
    {
        debug_fprintf(NSP_LOG_APPEND, "couldn't access local resource (possible security resons)\n");
        bRetval = sal_False;
    }
    catch (NoConnectException& )
    {
        debug_fprintf(NSP_LOG_APPEND, "no server listening on the resource\n");
        bRetval = sal_False;
    }
    catch (IllegalArgumentException& )
    {
        debug_fprintf(NSP_LOG_APPEND, "uno url invalid\n");
        bRetval = sal_False;
    }
    catch (RuntimeException& )
    {
        debug_fprintf(NSP_LOG_APPEND, "a remote call was aborted\n");
        bRetval = sal_False;
    }
    catch (uno::Exception&)
    {
        debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  unknown error while connect to remote Office.\n");
        bRetval = sal_False;
    }
    debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  connect over.\n");
    return bRetval;
}

void SoPluginInstance::SetPosSize(int x, int y, int w, int h)
{
    m_nX = x;
    m_nY = y;
    m_nWidth = w;
    m_nHeight = h;
}

void SoPluginInstance::Setflag(sal_Int16 n)
{
    m_nFlag = n;
}

sal_Bool SoPluginInstance::SetURL(char* aURL)
{
    debug_fprintf(NSP_LOG_APPEND, "SoPluginInstance::SetURL %s\n", aURL);
    sal_uInt16 encoding = 11;

    rtl_Locale* pLocale = NULL;
    osl_getProcessLocale(&pLocale);
    encoding = osl_getTextEncodingFromLocale(pLocale);

    m_sURL = ::rtl::OUString(aURL, strlen(aURL), encoding);
    debug_fprintf(NSP_LOG_APPEND, "SetURL %s\nencoding is: %d\n", ::rtl::OUStringToOString(m_sURL,
        RTL_TEXTENCODING_GB_18030).getStr(), m_sURL.getLength(), encoding);
    return sal_True;
}

// plugin window UI part: create window, load document
bool SoPluginInstance::LoadDocument(NSP_HWND hParent)
{
    // If doc has been loaded, we just resize the window and return
    if(m_bInit)
    {
        debug_fprintf(NSP_LOG_APPEND, "only reset windows size\n");
        m_xUnoWin->setPosSize( m_nX, m_nY, m_nWidth, m_nHeight, m_nFlag );
        debug_fprintf(NSP_LOG_APPEND, "set windows to x:%d y:%d w:%d h%d falg:%d\n",
            m_nX, m_nY, m_nWidth, m_nHeight, m_nFlag);
        return true;
    }

    // If mxRemoteMSF is not initialized, we assert and return sal_False
    if(!mxRemoteMSF.is())
    {
        debug_fprintf(NSP_LOG_APPEND, "Remote StarOfiice ServiceManager is not initilzed correctly!\n");

        // first, try restar office and reconnect
        if(!restart_office()){
            debug_fprintf(NSP_LOG_APPEND, "restar office error!\n");
            return false;
        }
        if(!Connect()){
            debug_fprintf(NSP_LOG_APPEND, "reconnect office error!\n");
            return false;
        }
        debug_fprintf(NSP_LOG_APPEND, "Restore StarOfiice ServiceManager is not initilzed correctly!\n");
    }

    try
    {
        // try to create netscape plugin window
        Reference< awt::XToolkit > xToolkit(
            mxRemoteMSF->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.awt.Toolkit") ),
            uno::UNO_QUERY );
        if( !xToolkit.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "Can not create Toolkit!\n");
            return false;
        }

        // prepare parameters for plugin window
        css::uno::Any hwndParent = css::uno::makeAny((sal_Int32)hParent);
#ifdef UNIX
        sal_Int16 nWindowType = css::lang::SystemDependent::SYSTEM_XWINDOW;
#endif //end of UNIX
#ifdef WNT
        sal_Int16 nWindowType = css::lang::SystemDependent::SYSTEM_WIN32;
#endif //end of WNT

        Reference< awt::XSystemChildFactory > xToolkitSystemChildFactory(xToolkit, uno::UNO_QUERY);
        if (!xToolkitSystemChildFactory.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, get xToolkitSystemChildFactory failure.\n");
            return false;
        }

        debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  try to create plugin container window HWIN:%ld.\n", hParent);

        // create the plugin window
        Reference< awt::XWindowPeer > xNewWinPeer =
            xToolkitSystemChildFactory->createSystemChild( hwndParent,
                css::uno::Sequence<sal_Int8>(), nWindowType );
        if ( !xNewWinPeer.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "can not create first window\n", hParent);
            return false;
        }

        // get interface of first window
        m_xUnoWin = Reference< awt::XWindow >( xNewWinPeer, uno::UNO_QUERY );
        if( !m_xUnoWin.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "can not get interface of first window\n", hParent);
            return false;
        }

        // initialize window
        xNewWinPeer->setBackground( COL_GRAY );
        m_xUnoWin->setVisible( sal_True );
        m_xUnoWin->setPosSize( m_nX, m_nY, m_nWidth, m_nHeight, m_nFlag );
        debug_fprintf(NSP_LOG_APPEND, "set windows to x:%d y:%d w:%d h%d falg:%d\n",
            m_nX, m_nY, m_nWidth, m_nHeight, m_nFlag);

        // create frame
        m_xFrame = Reference< frame::XFrame >(
            mxRemoteMSF->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.frame.Frame") ),
            uno::UNO_QUERY );
        if (!m_xFrame.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "can not create frame\n");
            return false;
        }

        // initialize frame
        m_xFrame->initialize( m_xUnoWin );

        try
        {
            // currently ignore errors in this code
            uno::Reference< beans::XPropertySet > xFrameProps( m_xFrame, uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xLMProps;
            xFrameProps->getPropertyValue( ::rtl::OUString::createFromAscii("LayoutManager") ) >>= xLMProps;
            if ( xLMProps.is() )
                xLMProps->setPropertyValue( ::rtl::OUString::createFromAscii("AutomaticToolbars"), uno::makeAny( (sal_Bool)sal_False ) );
        }
        catch( uno::Exception& )
        {}

        // get frames supplier
        Reference< frame::XFramesSupplier > m_xFramesSupplier(
            mxRemoteMSF->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop") ),
            uno::UNO_QUERY );
        if ( !m_xFramesSupplier.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "can not get desktop\n");
            return false;
        }

        // get frames
        m_xFrames = m_xFramesSupplier->getFrames();
        if ( !m_xFrames.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "can not get frames from FramesSupplier\n");
            return false;
        }

        // append m_xFrame to m_xFrames
        m_xFrames->append( m_xFrame );

        // create XComponentLoader
        Reference< frame::XComponentLoader > xLoader( m_xFrame, uno::UNO_QUERY );
        if ( !xLoader.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "can not get ComponentLoader to load URL\n");
            return false;
        }

        //create stream for the document
        Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSimpleFileAccess(
            mxRemoteMSF->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess") ),
            uno::UNO_QUERY );
        if(!xSimpleFileAccess.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "can not create SimpleFileAccess to load URL\n");
            return false;
        }
        Reference<io::XInputStream> xInputStream = xSimpleFileAccess->openFileRead( m_sURL );

        if(!xInputStream.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "can not create XInputStream for URL\n");
            return false;
        }

        // prepare to load document
        debug_fprintf(NSP_LOG_APPEND, "try to load document\n");
        Sequence< ::com::sun::star::beans::PropertyValue > setPropValues(3);
        setPropValues[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("ViewOnly") );
        setPropValues[0].Value <<= sal_True;
        setPropValues[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") );
        setPropValues[1].Value <<= sal_True;
        setPropValues[2].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream"));
        setPropValues[2].Value <<= xInputStream;
        /*
        setPropValues[ 3 ].Name = "FilterName";
        setPropValues[ 3 ].Value =  "Flat XML File";*/

        debug_fprintf(NSP_LOG_APPEND, "try to load copy of URL from local file %s:%d\n",
            ::rtl::OUStringToOString( m_sURL, RTL_TEXTENCODING_ASCII_US ).getStr( ),
            m_sURL.getLength() );


        // load document
/*
        m_xComponent = xLoader->loadComponentFromURL(
            OUString(RTL_CONSTASCII_USTRINGPARAM("private:stream")), //m_sURL,
            m_xFrame->getName(), 0, setPropValues );
  */

         Sequence< ::com::sun::star::beans::PropertyValue > setPropValues2(3);
        setPropValues2[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("ViewOnly") );
        setPropValues2[0].Value <<= sal_True;
        setPropValues2[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") );
        setPropValues2[1].Value <<= sal_True;
        setPropValues2[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("AsTemplate") );
        setPropValues2[2].Value <<= sal_False;
        m_xComponent = xLoader->loadComponentFromURL(
            m_sURL,
            m_xFrame->getName(), 0, setPropValues2 );

        if ( !m_xComponent.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, Load Componment error\n");
            return false;
        }

         // register the closelistener that will prevent closing of the component
        Reference< util::XCloseable > xCloseable( m_xFrame, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            m_pCloseListener = new PluginDocumentClosePreventer();
            m_xCloseListener = Reference< util::XCloseListener >( m_pCloseListener );
            xCloseable->addCloseListener( m_xCloseListener );
        }

        //free the input stream, it is hold by the m_xComponent
        xInputStream = Reference<io::XInputStream>();

        debug_fprintf(NSP_LOG_APPEND, "load document success\n");

        // create frame::XDispatchHelper and frame::XDispatchProvider
        m_xDispatcher = Reference< frame::XDispatchHelper > (
            mxRemoteMSF->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.DispatchHelper")),
            uno::UNO_QUERY );
        if(!m_xDispatcher.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "m_xDispatcher can not be getten\n");
            return false;
        }
        m_xDispatchProvider = Reference< frame::XDispatchProvider >(m_xFrame, uno::UNO_QUERY);
        if(!m_xDispatchProvider.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "m_xDispatchProvider can not be getten\n");
            return false;
        }

        //try to enable toolbar and tool windows
        Sequence< ::com::sun::star::beans::PropertyValue > propertyValue(1);
        propertyValue[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FunctionBarVisible") );
        propertyValue[0].Value <<= sal_True;
        m_xDispatcher->executeDispatch(m_xDispatchProvider,
                ::rtl::OUString::createFromAscii(".uno:FunctionBarVisible"),
                m_xFrame->getName(), 0,
                propertyValue );

        // reset window style of parent window
#ifdef WNT
        m_dParentStyl = ::NSP_ResetWinStyl (m_hParent);
#endif
        m_bInit = sal_True;
    }
    catch( uno::Exception& e )
    {
        debug_fprintf(NSP_LOG_APPEND, "Unknown exception while loading document in netscape plugin windows\n");
        OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
        debug_fprintf(NSP_LOG_APPEND, "error: %s \n", o.pData->buffer );
        return false;
    }
    return true;
}

sal_Bool SoPluginInstance::SetSODir(char * sDir)
{
    if(strlen(sDir) >= NPP_PATH_MAX) return sal_False;

    strcpy(sSO_Dir, sDir);
    return sal_True;
}

sal_Bool SoPluginInstance::SetWindow(NSP_HWND hParent, int x, int y, int w, int h)
{
    sal_Bool bRetval(sal_True);

    if ( m_hParent && !hParent )
    {
        // the plugin instance has the parent window set already and now it becomes NULL as parent window
        // that means that the instance should be closed
        Destroy();
        m_hParent = hParent;
    }
    else
    {
        m_hParent = hParent;

        debug_fprintf(NSP_LOG_APPEND, "SoPluginInstance::SetWindow %s : %d\n",
            ::rtl::OUStringToOString(m_sURL, RTL_TEXTENCODING_ASCII_US).getStr(),
            m_sURL.getLength() );
        m_nWidth = w;
        m_nHeight =h;
        m_nX = x;
        m_nY = y;

        if (0 == m_sURL.getLength())
            return sal_True;

        if(!mxRemoteMSF.is())
        {
            bRetval = Connect();  // Connect to listening so and get mxRemoteMSF
            if(!bRetval)
            {
                debug_fprintf(NSP_LOG_APPEND, "can not connect to remote service manager\n");
                return sal_False;
            }
        }
        debug_fprintf(NSP_LOG_APPEND, "in SoPluginInstance::SetWindow, begin LoadDocument(hParent)\n");
        bRetval = LoadDocument(hParent);  // Load document into current window
        if(!bRetval){
            // try to reload document again
            debug_fprintf(NSP_LOG_APPEND, "load document error, try to reload it once again\n");
            mxRemoteMSF = Reference< XMultiServiceFactory >();
            m_bInit = sal_False;
            bRetval = LoadDocument(hParent);
            debug_fprintf(NSP_LOG_APPEND, "load document again, return %d\n", bRetval);
        }
    }

    return bRetval;
}

sal_Bool SoPluginInstance::Destroy(void)
{
#ifdef WNT
    if(m_dParentStyl != 0)
        NSP_RestoreWinStyl(m_hParent, m_dParentStyl);
#endif
//  if(m_xDispatcher.is()){
//   m_xDispatcher->executeDispatch(m_xDispatchProvider,
//                ::rtl::OUString::createFromAscii(".uno:CloseFrame"),
//                ::rtl::OUString::createFromAscii("_top"), 0,
//                Sequence< ::com::sun::star::beans::PropertyValue >() );
//  }

    uno::Reference< util::XCloseable > xCloseable( m_xFrame, uno::UNO_QUERY );

    try
    {
        if ( m_xCloseListener.is() )
        {
            if ( m_pCloseListener )
                m_pCloseListener->StopPreventClose();

            if ( xCloseable.is() )
                xCloseable->removeCloseListener( m_xCloseListener );
        }
    }
    catch( uno::Exception& )
    {}

    try
    {
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[0] <<= m_xFrame;
        uno::Reference< lang::XComponent > xDocumentCloser(
            mxRemoteMSF->createInstanceWithArguments(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.DocumentCloser" ) ),
                aArgs ),
            uno::UNO_QUERY_THROW );

        xDocumentCloser->dispose(); // this call should close the document
    }
    catch( uno::Exception& )
    {
           debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin.exe, could not close the document correctly!\n");
        try
        {
            if ( xCloseable.is() )
                xCloseable->close( sal_True );
            else
            {
                uno::Reference< lang::XComponent > xDisposable( m_xFrame, uno::UNO_QUERY );
                if ( xDisposable.is() )
                    xDisposable->dispose();
            }
        }
        catch (const uno::Exception&)
        {
        }
    }

    m_xUnoWin = Reference< awt::XWindow >();
    m_xComponent = Reference< XComponent >();
    m_xFrame = Reference< frame::XFrame >();
    m_xFrames = Reference< frame::XFrames >();
    m_xDispatcher = Reference< frame::XDispatchHelper >();
    m_xDispatchProvider = Reference< frame::XDispatchProvider >();
    return sal_True;
}

sal_Bool SoPluginInstance::Print(void)
{
    if(!m_xDispatcher.is())
        return sal_False;

    Sequence< ::com::sun::star::beans::PropertyValue > propertyValue(1);
    m_xDispatcher->executeDispatch(m_xDispatchProvider,
        ::rtl::OUString::createFromAscii(".uno:PrintDefault"),
        m_xFrame->getName(), 0,
        propertyValue );
    return sal_True;
}

sal_Bool SoPluginInstance::ShutDown(void)
{
    mxRemoteMSF = Reference< XMultiServiceFactory >();
    mxLocalMSF = Reference< XMultiServiceFactory >();
    return sal_True;
}

