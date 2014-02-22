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
 * <http:
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/presentation/XPresentation.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <tools/color.hxx>
#include <rtl/textenc.h>
#include <rtl/locale.h>
#include <osl/nlsupport.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "so_instance.hxx"
#include "ns_debug.hxx"
#include "so_env.hxx"
#include "nsp_windows.hxx"
#include "so_closelistener.hxx"
#include <osl/process.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::connection;
using namespace cppu;
using namespace com::sun::star;


char SoPluginInstance::sSO_Dir[] = {0};
Reference< XMultiServiceFactory > SoPluginInstance::mxRemoteMSF = Reference< XMultiServiceFactory >(NULL);

SoPluginInstance::SoPluginInstance(long pParent, Reference< XMultiServiceFactory > xMSF):
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
    m_hParent = 0;
    m_pParent = pParent;
    m_dParentStyl = 0;
    mxRemoteMSF = xMSF;
}

SoPluginInstance::~SoPluginInstance()
{
}

sal_Bool SoPluginInstance::SetURL(char* aURL)
{
    debug_fprintf(NSP_LOG_APPEND, "SoPluginInstance::SetURL %s\n", aURL);

    rtl_Locale* pLocale = NULL;
    osl_getProcessLocale(&pLocale);
    sal_uInt16 encoding = osl_getTextEncodingFromLocale(pLocale);

    m_sURL = OUString(aURL, strlen(aURL), encoding);
    debug_fprintf(NSP_LOG_APPEND, "SetURL %s\nencoding is: %d\n", OUStringToOString(m_sURL,
        RTL_TEXTENCODING_GB_18030).getStr(), m_sURL.getLength(), encoding);
    return sal_True;
}


sal_Bool SoPluginInstance::LoadDocument(NSP_HWND hParent)
{
    
    if(m_bInit)
    {
        debug_fprintf(NSP_LOG_APPEND, "only reset windows size\n");
        m_xUnoWin->setPosSize( m_nX, m_nY, m_nWidth, m_nHeight, m_nFlag );
        debug_fprintf(NSP_LOG_APPEND, "set windows to x:%d y:%d w:%d h%d falg:%d\n",
            m_nX, m_nY, m_nWidth, m_nHeight, m_nFlag);
        return sal_True;
    }

    
    if(!mxRemoteMSF.is())
    {
        debug_fprintf(NSP_LOG_APPEND, "Remote StarOfiice ServiceManager is not initilzed correctly!\n");
        return sal_False;
    }

    try
    {
        Reference< beans::XPropertySet > xFactoryProperties( mxRemoteMSF, uno::UNO_QUERY );
        Reference< uno::XComponentContext > xContext( xFactoryProperties->getPropertyValue( "DefaultContext" ), UNO_QUERY );

        
        Reference< awt::XToolkit2 > xToolkit( awt::Toolkit::create(xContext) );

        
        css::uno::Any hwndParent = css::uno::makeAny((sal_Int32)hParent);
#ifdef UNIX
        sal_Int16 nWindowType = css::lang::SystemDependent::SYSTEM_XWINDOW;
#endif 
#ifdef WNT
        sal_Int16 nWindowType = css::lang::SystemDependent::SYSTEM_WIN32;
#endif 

        debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin,  try to create plugin container window HWIN:%ld.\n", hParent);

        
        Reference< awt::XWindowPeer > xNewWinPeer =
            xToolkit->createSystemChild( hwndParent,
                css::uno::Sequence<sal_Int8>(), nWindowType );
        if ( !xNewWinPeer.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "can not create first window\n", hParent);
            return sal_False;
        }

        
        m_xUnoWin = Reference< awt::XWindow >( xNewWinPeer, uno::UNO_QUERY );
        if( !m_xUnoWin.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "can not get interface of first window\n", hParent);
            return sal_False;
        }

        
        xNewWinPeer->setBackground( COL_GRAY );
        m_xUnoWin->setVisible( sal_True );
        m_xUnoWin->setPosSize( m_nX, m_nY, m_nWidth, m_nHeight, m_nFlag );
        debug_fprintf(NSP_LOG_APPEND, "set windows to x:%d y:%d w:%d h%d falg:%d\n",
            m_nX, m_nY, m_nWidth, m_nHeight, m_nFlag);

        
        m_xFrame = frame::Frame::create( xContext );

        
        m_xFrame->initialize( m_xUnoWin );

        try
        {
            
            uno::Reference< beans::XPropertySet > xFrameProps( m_xFrame, uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xLMProps;
            xFrameProps->getPropertyValue("LayoutManager") >>= xLMProps;
            if ( xLMProps.is() )
                xLMProps->setPropertyValue("AutomaticToolbars", uno::makeAny( (sal_Bool)sal_False ) );
        }
        catch( const uno::Exception& )
        {}

        
        Reference< frame::XDesktop2 > xFramesSupplier = frame::Desktop::create( xContext );

        
        m_xFrames = xFramesSupplier->getFrames();
        if ( !m_xFrames.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "can not get frames from FramesSupplier\n");
            return sal_False;
        }

        
        m_xFrames->append( Reference<frame::XFrame>(m_xFrame, uno::UNO_QUERY_THROW) );

        
        Reference< frame::XComponentLoader > xLoader( m_xFrame, uno::UNO_QUERY );
        if ( !xLoader.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "can not get ComponentLoader to load URL\n");
            return sal_False;
        }

        
        Reference< ucb::XSimpleFileAccess3 > xSimpleFileAccess( ucb::SimpleFileAccess::create(xContext) );
        Reference<io::XInputStream> xInputStream = xSimpleFileAccess->openFileRead( m_sURL );

        if(!xInputStream.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "can not create XInputStream for URL\n");
            return sal_False;
        }

        
        debug_fprintf(NSP_LOG_APPEND, "try to load document\n");
        Sequence< ::com::sun::star::beans::PropertyValue > setPropValues(3);
        setPropValues[0].Name = "ViewOnly";
        setPropValues[0].Value <<= sal_True;
        setPropValues[1].Name = "ReadOnly";
        setPropValues[1].Value <<= sal_True;
        setPropValues[2].Name = "InputStream";
        setPropValues[2].Value <<= xInputStream;
        /*
        setPropValues[ 3 ].Name = "FilterName";
        setPropValues[ 3 ].Value =  "Flat XML File";*/

        debug_fprintf(NSP_LOG_APPEND, "try to load copy of URL from local file %s:%d\n",
            OUStringToOString( m_sURL, RTL_TEXTENCODING_ASCII_US ).getStr( ),
            m_sURL.getLength() );

        
         Sequence< ::com::sun::star::beans::PropertyValue > setPropValues2(3);
        setPropValues2[0].Name = "ViewOnly";
        setPropValues2[0].Value <<= sal_True;
        setPropValues2[1].Name = "ReadOnly";
        setPropValues2[1].Value <<= sal_True;
        setPropValues2[2].Name = "AsTemplate";
        setPropValues2[2].Value <<= sal_False;
        m_xComponent = xLoader->loadComponentFromURL(
            m_sURL,
            m_xFrame->getName(), 0, setPropValues2 );

        if ( !m_xComponent.is() )
        {
            debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, Load Componment error\n");
            return sal_False;
        }

         
        Reference< util::XCloseable > xCloseable( m_xFrame, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            m_pCloseListener = new PluginDocumentClosePreventer();
            m_xCloseListener = Reference< util::XCloseListener >( m_pCloseListener );
            xCloseable->addCloseListener( m_xCloseListener );
        }

        
        xInputStream.clear();

        debug_fprintf(NSP_LOG_APPEND, "load document success\n");

        
        m_xDispatcher = Reference< frame::XDispatchHelper >( frame::DispatchHelper::create( xContext ) );
        m_xDispatchProvider = Reference< frame::XDispatchProvider >(m_xFrame, uno::UNO_QUERY);
        if(!m_xDispatchProvider.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "m_xDispatchProvider can not be getten\n");
            return sal_False;
        }

        
        Sequence< ::com::sun::star::beans::PropertyValue > propertyValue(1);
        propertyValue[0].Name = "FunctionBarVisible";
        propertyValue[0].Value <<= sal_True;
        m_xDispatcher->executeDispatch(m_xDispatchProvider,
                OUString(".uno:FunctionBarVisible"),
                m_xFrame->getName(), 0,
                propertyValue );

        
#ifdef WNT
        m_dParentStyl = ::NSP_ResetWinStyl (m_hParent);
#endif
        m_bInit = sal_True;

        try
        {
            
            uno::Reference< presentation::XPresentationSupplier > xPresSuppl( m_xComponent, uno::UNO_QUERY_THROW );
            uno::Reference< presentation::XPresentation > xPres( xPresSuppl->getPresentation(), uno::UNO_SET_THROW );
            uno::Reference< beans::XPropertySet > xProps( xPresSuppl->getPresentation(), uno::UNO_QUERY_THROW );
            xProps->setPropertyValue("IsFullScreen", uno::makeAny( sal_False ) );
            xPres->start();
        }
        catch( const uno::Exception& )
        {}
    }
    catch( const uno::Exception& e )
    {
        debug_fprintf(NSP_LOG_APPEND, "Unknown exception while loading document in netscape plugin windows\n");
        OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
        debug_fprintf(NSP_LOG_APPEND, "error: %s \n", o.pData->buffer );
        return sal_False;
    }
    return sal_True;
}

sal_Bool SoPluginInstance::SetWindow(NSP_HWND hParent, int x, int y, int w, int h)
{
    sal_Bool bRetval(sal_True);

    if ( m_hParent && !hParent )
    {
        
        
        Destroy();
        m_hParent = hParent;
    }
    else
    {
        m_hParent = hParent;

        debug_fprintf(NSP_LOG_APPEND, "SoPluginInstance::SetWindow %s : %d\n",
            OUStringToOString(m_sURL, RTL_TEXTENCODING_ASCII_US).getStr(),
            m_sURL.getLength() );
        m_nWidth = w;
        m_nHeight =h;
        m_nX = x;
        m_nY = y;

        if ( m_sURL.isEmpty() )
            return sal_True;

        if(!mxRemoteMSF.is())
        {
            debug_fprintf(NSP_LOG_APPEND, "Remote StarOfiice ServiceManager is not initilzed correctly!\n");
            return sal_False;
        }
        debug_fprintf(NSP_LOG_APPEND, "in SoPluginInstance::SetWindow, begin LoadDocument(hParent)\n");
        bRetval = LoadDocument(hParent);  
        if(!bRetval){
            
            debug_fprintf(NSP_LOG_APPEND, "load document error, try to reload it once again\n");
            mxRemoteMSF.clear();
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
    catch( const uno::Exception& )
    {}

    try
    {
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[0] <<= m_xFrame;
        uno::Reference< lang::XComponent > xDocumentCloser(
            mxRemoteMSF->createInstanceWithArguments(
                OUString( "com.sun.star.embed.DocumentCloser" ),
                aArgs ),
            uno::UNO_QUERY_THROW );

        xDocumentCloser->dispose(); 
    }
    catch( const uno::Exception& )
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

    m_xUnoWin.clear();
    m_xComponent.clear();
    m_xFrame.clear();
    m_xFrames.clear();
    m_xDispatcher.clear();
    m_xDispatchProvider.clear();
    return sal_True;
}

sal_Bool SoPluginInstance::Print(void)
{
    if(!m_xDispatcher.is())
        return sal_False;

    Sequence< ::com::sun::star::beans::PropertyValue > propertyValue(1);
    m_xDispatcher->executeDispatch(m_xDispatchProvider,
        OUString(".uno:PrintDefault"),
        m_xFrame->getName(), 0,
        propertyValue );
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
