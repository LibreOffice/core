/*************************************************************************
 *
 *  $RCSfile: docholder.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: abi $ $Date: 2003-03-26 13:51:22 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


using namespace ::com::sun::star;

extern ::rtl::OUString  getFilterNameFromGUID_Impl( GUID* );

// add mutex locking ???

DocumentHolder::DocumentHolder( const uno::Reference< lang::XMultiServiceFactory >& xFactory,EmbedDocument_Impl *pOLEInterface)
    : m_xFactory( xFactory ),
      m_pOLEInterface(pOLEInterface)
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
    else
        uno::Reference<lang::XComponent>(m_xFrame,uno::UNO_QUERY)->dispose();

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

    return m_xFrame;
}


void DocumentHolder::show()
{
    if(m_xFrame.is())
        m_xFrame->activate();
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
    }
}


void DocumentHolder::hide()
{
    if(m_xFrame.is())
        m_xFrame->deactivate();
}


void SAL_CALL DocumentHolder::disposing( const com::sun::star::lang::EventObject& aSource )
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
        m_xDocument = uno::Reference< frame::XModel >();

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
        m_xDocument = uno::Reference< frame::XModel >();

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

