/*************************************************************************
 *
 *  $RCSfile: docholder.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 13:54:12 $
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

#include "docholder.hxx"
#include "embeddoc.hxx"
#include "intercept.hxx"


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
#include <com/sun/star/util/XCloseAble.hpp>
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

using namespace ::com::sun::star;

extern ::rtl::OUString  getFilterNameFromGUID_Impl( GUID* );

// add mutex locking ???

DocumentHolder::DocumentHolder( const uno::Reference< lang::XMultiServiceFactory >& xFactory,EmbedDocument_Impl *pOLEInterface)
    : m_xFactory( xFactory ),
      m_pOLEInterface(pOLEInterface),
      m_pInterceptor(0)
{
    const ::rtl::OUString aServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) );
    uno::Reference< frame::XDesktop > xDesktop( m_xFactory->createInstance( aServiceName ), uno::UNO_QUERY );
    if ( xDesktop.is() )
        xDesktop->addTerminateListener( (frame::XTerminateListener*)this );
}

DocumentHolder::~DocumentHolder()
{
    if ( m_xDocument.is() )
        CloseDocument();

    if( m_xFrame.is() )
        CloseFrame();

    if ( m_xFactory.is() )
        FreeOffice();
}

void DocumentHolder::FreeOffice()
{
    const ::rtl::OUString aServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) );
    uno::Reference< frame::XDesktop > xDesktop( m_xFactory->createInstance( aServiceName ), uno::UNO_QUERY );
    if ( xDesktop.is() )
    {
        xDesktop->removeTerminateListener( (frame::XTerminateListener*)this );

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

        m_xFactory = uno::Reference< lang::XMultiServiceFactory >();
    }
}

void DocumentHolder::CloseDocument()
{
    uno::Reference< util::XCloseBroadcaster > xBroadcaster( m_xDocument, uno::UNO_QUERY );
    if ( xBroadcaster.is() )
    {
        xBroadcaster->removeCloseListener( (util::XCloseListener*)this );

        uno::Reference< util::XCloseable > xCloseable( xBroadcaster, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( uno::Exception& )
            {}
        }
    }

    m_pIDispatch = NULL;
    m_xDocument = uno::Reference< frame::XModel >();
}


void DocumentHolder::CloseFrame()
{
    uno::Reference<util::XCloseable> xCloseable(
        m_xFrame,uno::UNO_QUERY);
    if(xCloseable.is())
        try {
            xCloseable->close(sal_True);
        }
        catch( const uno::Exception& ) {
        }
    else {
        uno::Reference<lang::XComponent> xComp(m_xFrame,uno::UNO_QUERY);
        if(xComp.is())
            xComp->dispose();
    }

    m_xFrame = uno::Reference< frame::XFrame >();
}

void DocumentHolder::SetDocument( const uno::Reference< frame::XModel >& xDoc)
{
    if ( m_xDocument.is() )
        CloseDocument();

    m_xDocument = xDoc;

    uno::Reference< util::XCloseBroadcaster > xBroadcaster( m_xDocument, uno::UNO_QUERY );
    if ( xBroadcaster.is() )
        xBroadcaster->addCloseListener( (util::XCloseListener*)this );

    if ( m_xDocument.is() )
    {
        // set the document mode to embedded
        uno::Sequence< beans::PropertyValue > aSeq( 1 );
        aSeq[0].Name = ::rtl::OUString::createFromAscii( "SetEmbedded" );
        aSeq[0].Value <<= sal_True;
        m_xDocument->attachResource( ::rtl::OUString(), aSeq );
    }
}


uno::Reference< frame::XFrame > DocumentHolder::DocumentFrame()
{
    if(! m_xFrame.is() )
    {
        rtl::OUString aDesktopSrvNm(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"));

        uno::Reference<frame::XDesktop> xDesktop(
            m_xFactory->createInstance(aDesktopSrvNm),
            uno::UNO_QUERY);

        uno::Reference<frame::XFrame> xFrame(
            xDesktop,uno::UNO_QUERY);

        if( xFrame.is() )
            m_xFrame = xFrame->findFrame(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank")),0);
    }

    if( m_xFrame.is() )
    {
        // intercept
        uno::Reference<frame::XDispatchProviderInterception>
            xDPI(m_xFrame,uno::UNO_QUERY);
        if(xDPI.is())
            xDPI->registerDispatchProviderInterceptor(
                m_pInterceptor = new Interceptor(m_pOLEInterface,this));
    }

    return m_xFrame;
}

void DocumentHolder::ClearInterceptor()
{
    m_pInterceptor = 0;
}

void DocumentHolder::show()
{
    if(m_xFrame.is()) {
        m_xFrame->activate();
        uno::Reference<awt::XTopWindow> xTopWindow(
            m_xFrame->getContainerWindow(),uno::UNO_QUERY);
        if(xTopWindow.is())
            xTopWindow->toFront();
    }
    else {
        uno::Reference<frame::XComponentLoader> xComponentLoader(
            DocumentFrame(),uno::UNO_QUERY);

        if(xComponentLoader.is())
        {
            uno::Sequence<beans::PropertyValue> aSeq(2);

            uno::Any aAny;
            aAny <<= uno::Reference<uno::XInterface>(
                GetDocument(),uno::UNO_QUERY);
            aSeq[0] = beans::PropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("Model")),
                -1,
                aAny,
                beans::PropertyState_DIRECT_VALUE);

            aAny <<= sal_False;
            aSeq[1] = beans::PropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("ReadOnly")),
                -1,
                aAny,
                beans::PropertyState_DIRECT_VALUE);

            xComponentLoader->loadComponentFromURL(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("private:object")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")),
                0,
                aSeq);
        }
        setTitle(m_aDocumentNamePart);
    }
}

void DocumentHolder::resizeWin( const SIZEL& rNewSize )
{
    if ( m_xFrame.is() && m_pOLEInterface )
    {
        uno::Reference< awt::XWindow > xWindow( m_xFrame->getContainerWindow(), uno::UNO_QUERY );
        uno::Reference< awt::XView > xView( xWindow, uno::UNO_QUERY );

        if ( xWindow.is() && xView.is() )
        {
            float fScale = 1;
            xView->setZoom( fScale, fScale );

            SIZEL aOldSize;
            GetExtent( &aOldSize );

            if ( aOldSize.cx != rNewSize.cx || aOldSize.cy != rNewSize.cy )
            {
                HDC hdc = GetDC( NULL );
                SetMapMode( hdc, MM_HIMETRIC );

                RECT aRectOld = { 0, 0, 0, 0 };
                aRectOld.right = aOldSize.cx;
                aRectOld.bottom = -aOldSize.cy;
                LPtoDP( hdc, (POINT*)&aRectOld, 2 );

                RECT aRectNew = { 0, 0, 0, 0 };
                aRectNew.right = rNewSize.cx;
                aRectNew.bottom = -rNewSize.cy;
                LPtoDP( hdc, (POINT*)&aRectNew, 2 );

                ReleaseDC( NULL, hdc );

                awt::Rectangle aWinRect = xWindow->getPosSize();
                sal_Int32 aWidthDelta = aWinRect.Width - ( aRectOld.right - aRectOld.left );
                sal_Int32 aHeightDelta = aWinRect.Height - ( aRectOld.bottom - aRectOld.top );

                if ( aWidthDelta > 0 && aHeightDelta > 0 )
                    xWindow->setPosSize(0,
                                        0,
                                        aRectNew.right - aRectNew.left + aWidthDelta,
                                        aRectNew.bottom - aRectNew.top + aHeightDelta,
                                        awt::PosSize::SIZE );
            }
        }
    }
}

void DocumentHolder::setTitle(const rtl::OUString& aDocumentName)
{
    if(m_xFrame.is())
    {
        if(m_aFilterName.getLength() == 0)
        {
            rtl::OUString aFilterName;
            uno::Sequence<beans::PropertyValue> aSeq;
            if(m_xDocument.is())
            {
                aSeq =
                    m_xDocument->getArgs();
                for(sal_Int32 j = 0; j < aSeq.getLength(); ++j)
                {
                    if(aSeq[j].Name ==
                       rtl::OUString(
                           RTL_CONSTASCII_USTRINGPARAM("FilterName")))
                    {
                        aSeq[j].Value >>= aFilterName;
                        break;
                    }
                }
            }

            if(aFilterName.getLength())
            {
                uno::Reference<container::XNameAccess> xNameAccess(
                    m_xFactory->createInstance(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.document.FilterFactory"))),
                    uno::UNO_QUERY);
                try {
                    if(xNameAccess.is() &&
                       (xNameAccess->getByName(aFilterName) >>= aSeq))
                    {
                        for(sal_Int32 j = 0; j < aSeq.getLength(); ++j)
                            if(aSeq[j].Name ==
                               rtl::OUString(
                                   RTL_CONSTASCII_USTRINGPARAM("UIName")))
                            {
                                aSeq[j].Value >>= m_aFilterName;
                                break;
                            }
                    }
                }
                catch(const uno::Exception& ) {
                    // nothing better to do here
                    m_aFilterName = aFilterName;
                }
            }
        }
        // set the title
        uno::Reference<beans::XPropertySet> xPropSet(
            m_xFrame,uno::UNO_QUERY);
        if(xPropSet.is()) {
            uno::Any aAny;
            static const sal_Unicode u[] = { ' ','(',0 };
            static const sal_Unicode c[] = { ')',0 };
            rtl::OUString aTotalName(m_aFilterName);
            aTotalName += rtl::OUString(u);
            aTotalName += aDocumentName;
            aTotalName += rtl::OUString(c);
            aAny <<= aTotalName;
            try {
                xPropSet->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title")),
                    aAny);
            }
            catch( const uno::Exception& ) {
            }
        }
    }

    m_aDocumentNamePart = aDocumentName;

    if(m_pInterceptor)
        m_pInterceptor->generateFeatureStateEvent();
}


void DocumentHolder::setContainerName(const rtl::OUString& aContainerName)
{
    m_aContainerName = aContainerName;
}


void DocumentHolder::hide()
{
    if(m_xFrame.is())
        m_xFrame->deactivate();
}

IDispatch* DocumentHolder::GetIDispatch()
{
    if ( !m_pIDispatch && m_xDocument.is() )
    {
        const ::rtl::OUString aServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.bridge.OleBridgeSupplier2" ) );
        uno::Reference< bridge::XBridgeSupplier2 > xSupplier( m_xFactory->createInstance( aServiceName ), uno::UNO_QUERY );

        if ( xSupplier.is() )
        {
            uno::Sequence< sal_Int8 > aProcId( 16 );
            rtl_getGlobalProcessId( (sal_uInt8*)aProcId.getArray() );

            try {
                uno::Any anyResult = xSupplier->createBridge( uno::makeAny( m_xDocument ),
                                                              aProcId,
                                                              bridge::ModelDependent::UNO,
                                                              bridge::ModelDependent::OLE );

                if ( anyResult.getValueTypeClass() == getCppuType((sal_uInt32*) 0).getTypeClass() )
                {
                    VARIANT* pVariant = *(VARIANT**)anyResult.getValue();
                    if ( pVariant->vt == VT_DISPATCH )
                        m_pIDispatch = pVariant->pdispVal;

                    VariantClear( pVariant );
                    CoTaskMemFree( pVariant );
                }
            }
            catch ( uno::Exception& )
            {}
        }
    }

    return m_pIDispatch;
}

HRESULT DocumentHolder::SetVisArea( const RECTL *pRect )
{
    if ( pRect && m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name.equalsAscii( "WinExtent" ) )
            {
                // should allways be there
                uno::Sequence< sal_Int32 > aRect(4);

                aRect[0] = pRect->left;
                aRect[1] = pRect->top;
                aRect[2] = pRect->right;
                aRect[3] = pRect->bottom;

                aArgs[nInd].Value <<= aRect;

                m_xDocument->attachResource( m_xDocument->getURL(), aArgs );
                return S_OK;
            }

        OSL_ENSURE( sal_False, "WinExtent seems not to be implemented!\n" );
    }

    return E_FAIL;
}

HRESULT DocumentHolder::GetVisArea( RECTL *pRect )
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
                    pRect->left   = aRect[0];
                    pRect->top    = aRect[1];
                    pRect->right  = aRect[2];
                    pRect->bottom = aRect[3];

                    return S_OK;
                }

                break;
            }
    }

    return E_FAIL;
}

HRESULT DocumentHolder::SetExtent( const SIZEL *pSize )
{
    if ( pSize && m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name.equalsAscii( "WinExtent" ) )
            {
                // should allways be there
                uno::Sequence< sal_Int32 > aRect;
                if ( ( aArgs[nInd].Value >>= aRect ) && aRect.getLength() == 4 )
                {
                    aRect[2] = aRect[0] + pSize->cx; // right = left + cx
                    aRect[3] = aRect[1] + pSize->cy; // bottom = top + cy

                    aArgs[nInd].Value <<= aRect;

                    m_xDocument->attachResource( m_xDocument->getURL(), aArgs );
                    return S_OK;
                }
            }

        OSL_ENSURE( sal_False, "WinExtent seems not to be implemented!\n" );
    }

    return E_FAIL;
}

HRESULT DocumentHolder::GetExtent( SIZEL *pSize )
{
    RECTL aRect;
    if ( pSize && GetVisArea( &aRect ) )
    {
        pSize->cx = aRect.right - aRect.left;
        pSize->cy = aRect.top - aRect.bottom;

        return S_OK;
    }

    return E_FAIL;
}

void SAL_CALL DocumentHolder::disposing( const com::sun::star::lang::EventObject& aSource )
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
    {
        m_pIDispatch = NULL;
        m_xDocument = uno::Reference< frame::XModel >();

    }

    if( m_xFrame.is() && m_xFrame == aSource.Source )
        m_xFrame = uno::Reference< frame::XFrame >();
}


void SAL_CALL DocumentHolder::queryClosing( const lang::EventObject& aSource, sal_Bool bGetsOwnership )
        throw( util::CloseVetoException )
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
        throw util::CloseVetoException();
}

void SAL_CALL DocumentHolder::notifyClosing( const lang::EventObject& aSource )
{
    uno::Reference< util::XCloseBroadcaster > xEventBroadcaster(
        aSource.Source, uno::UNO_QUERY );
    if ( xEventBroadcaster.is() )
        xEventBroadcaster->removeCloseListener( (util::XCloseListener*)this );

    if ( m_xDocument.is() && m_xDocument == aSource.Source )
    {
        m_pIDispatch = NULL;
        m_xDocument = uno::Reference< frame::XModel >();
    }

    if( m_xFrame.is() && m_xFrame == aSource.Source )
        m_xFrame = uno::Reference< frame::XFrame >();
}

void SAL_CALL DocumentHolder::queryTermination( const lang::EventObject& aSource )
        throw( frame::TerminationVetoException )
{
    if ( m_xDocument.is() )
        throw frame::TerminationVetoException();
}

void SAL_CALL DocumentHolder::notifyTermination( const lang::EventObject& aSource )
{
    OSL_ENSURE( !m_xDocument.is(), "Just a disaster..." );

    uno::Reference< frame::XDesktop > xDesktop( aSource.Source, uno::UNO_QUERY );

    if ( xDesktop.is() )
        xDesktop->removeTerminateListener( (frame::XTerminateListener*)this );
}



void SAL_CALL DocumentHolder::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    if(m_pOLEInterface)
        m_pOLEInterface->notify();
}

