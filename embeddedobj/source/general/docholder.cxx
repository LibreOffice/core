/*************************************************************************
 *
 *  $RCSfile: docholder.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-18 12:47:07 $
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
 *  Source License Version 1.1 ( the "License" ); You may not use this file
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
 *  Contributor( s ): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEBROADCASTER_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVIEW_HPP_
#include <com/sun/star/awt/XView.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_XBRIDGESUPPLIER2_HPP_
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_MODELDEPENDENT_HPP_
#include <com/sun/star/bridge/ModelDependent.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

#include "docholder.hxx"
#include "commonembobj.hxx"
#include "intercept.hxx"


using namespace ::com::sun::star;


DocumentHolder::DocumentHolder( const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                                OCommonEmbeddedObject* pEmbObj )
: m_xFactory( xFactory ),
  m_pEmbedObj( pEmbObj ),
  m_pInterceptor( NULL ),
  m_bReadOnly( sal_False ),
  m_bWaitForClose( sal_False )
{
    const ::rtl::OUString aServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) );
    uno::Reference< frame::XDesktop > xDesktop( m_xFactory->createInstance( aServiceName ), uno::UNO_QUERY );
    if ( xDesktop.is() )
        xDesktop->addTerminateListener( ( frame::XTerminateListener* )this );
}

DocumentHolder::~DocumentHolder()
{
    if( m_xFrame.is() )
        CloseFrame();

    if ( m_xDocument.is() )
    {
        try {
            CloseDocument( sal_True, sal_False );
        } catch( uno::Exception& ) {}
    }

    if ( m_pInterceptor )
    {
        m_pInterceptor->DisconnectDocHolder();
        m_pInterceptor->release();
    }

    FreeOffice();
}

void DocumentHolder::CloseFrame()
{
    uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xFrame, uno::UNO_QUERY );
    if ( xCloseBroadcaster.is() )
        xCloseBroadcaster->removeCloseListener( ( util::XCloseListener* )this );

    uno::Reference<util::XCloseable> xCloseable(
        m_xFrame,uno::UNO_QUERY );
    if( xCloseable.is() )
        try {
            xCloseable->close( sal_True );
        }
        catch( const uno::Exception& ) {
        }
    else {
        uno::Reference<lang::XComponent> xComp( m_xFrame,uno::UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();
    }

    m_xFrame = uno::Reference< frame::XFrame >();
}

void DocumentHolder::FreeOffice()
{
    const ::rtl::OUString aServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) );
    uno::Reference< frame::XDesktop > xDesktop( m_xFactory->createInstance( aServiceName ), uno::UNO_QUERY );
    if ( xDesktop.is() )
    {
        xDesktop->removeTerminateListener( ( frame::XTerminateListener* )this );

        uno::Reference< frame::XFramesSupplier > xFramesSupplier( xDesktop, uno::UNO_QUERY );
        if ( xFramesSupplier.is() )
        {
            uno::Reference< frame::XFrames > xFrames = xFramesSupplier->getFrames();
            if ( xFrames.is() && !xFrames->hasElements() )
            {
                try
                {
                    xDesktop->terminate();
                }
                catch( uno::Exception & )
                {}
            }
        }
    }
}

void DocumentHolder::CloseDocument( sal_Bool bDeliverOwnership, sal_Bool bWaitForClose )
{
    uno::Reference< util::XCloseBroadcaster > xBroadcaster( m_xDocument, uno::UNO_QUERY );
    if ( xBroadcaster.is() )
    {
        uno::Reference< document::XEventBroadcaster > xEventBroadcaster( m_xDocument, uno::UNO_QUERY );
        if ( xEventBroadcaster.is() )
            xEventBroadcaster->removeEventListener( ( document::XEventListener* )this );

        uno::Reference< util::XCloseable > xCloseable( xBroadcaster, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            m_bWaitForClose = bWaitForClose;
            xCloseable->close( bDeliverOwnership );
        }
    }

    m_xDocument = uno::Reference< frame::XModel >();
}

uno::Reference< frame::XFrame > DocumentHolder::GetDocFrame()
{
    if ( !m_xFrame.is() )
    {
        uno::Reference< frame::XFrame > xDesktopFrame(
            m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
            uno::UNO_QUERY );

        if( !xDesktopFrame.is() )
            throw uno::RuntimeException();

        m_xFrame = xDesktopFrame->findFrame( rtl::OUString::createFromAscii( "_blank" ), 0 );

        uno::Reference< frame::XDispatchProviderInterception > xInterception( m_xFrame, uno::UNO_QUERY );
        if ( xInterception.is() )
        {
            if ( m_pInterceptor )
            {
                m_pInterceptor->DisconnectDocHolder();
                m_pInterceptor->release();
                m_pInterceptor = NULL;
            }

            m_pInterceptor = new Interceptor( this );
            m_pInterceptor->acquire();

            xInterception->registerDispatchProviderInterceptor( m_pInterceptor );
        }

        uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xFrame, uno::UNO_QUERY );
        if ( xCloseBroadcaster.is() )
            xCloseBroadcaster->addCloseListener( ( util::XCloseListener* )this );
    }

    if ( m_xDocument.is() )
        LoadDocToFrame();

    return m_xFrame;
}

void DocumentHolder::SetDocument( const uno::Reference< frame::XModel >& xDoc, sal_Bool bReadOnly )
{
    if ( m_xDocument.is() )
    {
        // May be should be improved
        try {
            CloseDocument( sal_True, sal_False );
        } catch( uno::Exception& )
        {}
    }

    m_xDocument = xDoc;
    m_bReadOnly = bReadOnly;

    uno::Reference< util::XCloseBroadcaster > xBroadcaster( m_xDocument, uno::UNO_QUERY );
    if ( xBroadcaster.is() )
        xBroadcaster->addCloseListener( ( util::XCloseListener* )this );

    uno::Reference< document::XEventBroadcaster > xEventBroadcaster( m_xDocument, uno::UNO_QUERY );
    if ( xEventBroadcaster.is() )
        xEventBroadcaster->addEventListener( ( document::XEventListener* )this );

    if ( m_xDocument.is() )
    {
        // set the document mode to embedded
        uno::Sequence< beans::PropertyValue > aSeq( 1 );
        aSeq[0].Name = ::rtl::OUString::createFromAscii( "SetEmbedded" );
        aSeq[0].Value <<= sal_True;
        m_xDocument->attachResource( ::rtl::OUString(), aSeq );
    }

    if ( m_xFrame.is() )
        LoadDocToFrame();
}

void DocumentHolder::LoadDocToFrame()
{
    if ( m_xFrame.is() && m_xDocument.is() )
    {
        // load new document in to the frame
        uno::Reference< frame::XComponentLoader > xComponentLoader( m_xFrame, uno::UNO_QUERY );
        if( !xComponentLoader.is() )
            throw uno::RuntimeException();

        uno::Sequence< beans::PropertyValue > aArgs( 2 );
        aArgs[0].Name = ::rtl::OUString::createFromAscii( "Model" );
        aArgs[0].Value <<= m_xDocument;
        aArgs[1].Name = ::rtl::OUString::createFromAscii( "ReadOnly" );
        aArgs[1].Value <<= m_bReadOnly;

        xComponentLoader->loadComponentFromURL( rtl::OUString::createFromAscii( "private:object" ),
                                                rtl::OUString::createFromAscii( "_self" ),
                                                0,
                                                aArgs );

        SetTitle( m_aDocumentNamePart );
    }
}

void DocumentHolder::Show()
{
    if( m_xFrame.is() )
    {
        m_xFrame->activate();
        uno::Reference<awt::XTopWindow> xTopWindow( m_xFrame->getContainerWindow(),uno::UNO_QUERY );
        if( xTopWindow.is() )
            xTopWindow->toFront();
    }
    else
        GetDocFrame();
}

void DocumentHolder::SetTitle( const rtl::OUString& aDocumentName )
{
    // TODO: to have a different title for links
    if( m_xFrame.is() )
    {
        rtl::OUString aFilterName;
        uno::Sequence<beans::PropertyValue> aSeq;
        if( m_xDocument.is() )
        {
            aSeq = m_xDocument->getArgs();
            for( sal_Int32 j = 0; j < aSeq.getLength(); ++j )
            {
                if( aSeq[j].Name == rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) ) )
                {
                    aSeq[j].Value >>= aFilterName;
                    break;
                }
            }
        }

        if( aFilterName.getLength() )
        {
            uno::Reference<container::XNameAccess> xNameAccess(
                m_xFactory->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" ) ) ),
                uno::UNO_QUERY );
            try {
                if( xNameAccess.is() && ( xNameAccess->getByName( aFilterName ) >>= aSeq ) )
                {
                    for( sal_Int32 j = 0; j < aSeq.getLength(); ++j )
                        if( aSeq[j].Name == rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ) ) )
                        {
                            aSeq[j].Value >>= aFilterName;
                            break;
                        }
                }
            }
            catch( const uno::Exception& )
            {
            }
        }

        // set the title
        uno::Reference<beans::XPropertySet> xPropSet( m_xFrame,uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            uno::Any aAny;
            static const sal_Unicode u[] = { ' ','( ',0 };
            static const sal_Unicode c[] = { ' )',0 };
            rtl::OUString aTotalName( aFilterName );
            aTotalName += rtl::OUString( u );
            aTotalName += aDocumentName;
            aTotalName += rtl::OUString( c );
            aAny <<= aTotalName;

            try
            {
                xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                                            aAny );
            }
            catch( const uno::Exception& )
            {
            }
        }
    }

    m_aDocumentNamePart = aDocumentName;

    if( m_pInterceptor )
        m_pInterceptor->GenerateFeatureStateEvent();
}

void DocumentHolder::SetContainerName( const rtl::OUString& aContainerName )
{
    m_aContainerName = aContainerName;
}

void DocumentHolder::Hide()
{
    if( m_xFrame.is() )
        m_xFrame->deactivate();
}

sal_Bool DocumentHolder::SetVisArea( const awt::Rectangle& aRect )
{
    if ( m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name.equalsAscii( "WinExtent" ) )
            {
                // should allways be there
                uno::Sequence< sal_Int32 > aSeqRect( 4 );

                aSeqRect[0] = aRect.X;
                aSeqRect[1] = aRect.Y;
                aSeqRect[2] = aRect.X + aRect.Width;
                aSeqRect[3] = aRect.Y + aRect.Height;

                aArgs[nInd].Value <<= aSeqRect;

                m_xDocument->attachResource( m_xDocument->getURL(), aArgs );
                return sal_True;
            }

        OSL_ENSURE( sal_False, "WinExtent seems not to be implemented!\n" );
    }

    return sal_False;
}

sal_Bool DocumentHolder::GetVisArea( awt::Rectangle *pRect )
{
    if ( pRect && m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name.equalsAscii( "WinExtent" ) )
            {
                uno::Sequence< sal_Int32 > aRect;
                if ( ( aArgs[nInd].Value >>= aRect ) && aRect.getLength() == 4 )
                {
                    pRect->X   = aRect[0];
                    pRect->Y    = aRect[1];
                    pRect->Width  = aRect[2] - pRect->X;
                    pRect->Height = aRect[3] - pRect->Y;

                    return sal_True;
                }

                break;
            }
    }

    return sal_False;
}

sal_Bool DocumentHolder::SetExtent( const awt::Size& aSize )
{
    if ( m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name.equalsAscii( "WinExtent" ) )
            {
                // should allways be there
                uno::Sequence< sal_Int32 > aRect;
                if ( ( aArgs[nInd].Value >>= aRect ) && aRect.getLength() == 4 )
                {
                    aRect[2] = aRect[0] + aSize.Width;
                    aRect[3] = aRect[1] + aSize.Height;

                    aArgs[nInd].Value <<= aRect;

                    m_xDocument->attachResource( m_xDocument->getURL(), aArgs );
                    return sal_True;
                }
            }

        OSL_ENSURE( sal_False, "WinExtent seems not to be implemented!\n" );
    }

    return sal_False;
}

sal_Bool DocumentHolder::GetExtent( awt::Size *pSize )
{
    awt::Rectangle aRect;
    if ( pSize && GetVisArea( &aRect ) )
    {
        pSize->Width = aRect.Width;
        pSize->Height = aRect.Height;

        return sal_True;
    }

    return sal_False;
}

void SAL_CALL DocumentHolder::disposing( const com::sun::star::lang::EventObject& aSource )
        throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
    {
        m_xDocument = uno::Reference< frame::XModel >();
        if ( m_bWaitForClose )
        {
            m_bWaitForClose = sal_False;
            FreeOffice();
        }
    }

    if( m_xFrame.is() && m_xFrame == aSource.Source )
        m_xFrame = uno::Reference< frame::XFrame >();
}


void SAL_CALL DocumentHolder::queryClosing( const lang::EventObject& aSource, sal_Bool bGetsOwnership )
        throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException)
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source && m_bWaitForClose )
        throw util::CloseVetoException();
}

void SAL_CALL DocumentHolder::notifyClosing( const lang::EventObject& aSource )
        throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
    {
        m_xDocument = uno::Reference< frame::XModel >();
        if ( m_bWaitForClose )
        {
            m_bWaitForClose = sal_False;
            FreeOffice();
        }
    }

    if( m_xFrame.is() && m_xFrame == aSource.Source )
        m_xFrame = uno::Reference< frame::XFrame >();
}

void SAL_CALL DocumentHolder::queryTermination( const lang::EventObject& aSource )
        throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException)
{
    if ( m_bWaitForClose )
        throw frame::TerminationVetoException();
}

void SAL_CALL DocumentHolder::notifyTermination( const lang::EventObject& aSource )
        throw (::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( !m_xDocument.is(), "Just a disaster..." );

    uno::Reference< frame::XDesktop > xDesktop( aSource.Source, uno::UNO_QUERY );

    if ( xDesktop.is() )
        xDesktop->removeTerminateListener( ( frame::XTerminateListener* )this );
}

void SAL_CALL DocumentHolder::modified( const lang::EventObject& aEvent )
    throw ( uno::RuntimeException )
{
    if( m_pEmbedObj )
        m_pEmbedObj->update(); // TODO ???
}

void SAL_CALL DocumentHolder::notifyEvent( const ::com::sun::star::document::EventObject& Event )
    throw ( ::com::sun::star::uno::RuntimeException )
{
    if( m_pEmbedObj && Event.Source == m_xDocument )
        m_pEmbedObj->PostEvent_Impl( Event.EventName );
}


