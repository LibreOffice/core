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
#include "precompiled_embeddedobj.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/link.hxx>
#include <osl/mutex.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>
#include <tools/stream.hxx>

#include "mtnotification.hxx"
#include "oleembobj.hxx"


using namespace ::com::sun::star;


sal_Bool ConvertBufferToFormat( void* pBuf,
                                sal_uInt32 nBufSize,
                                const ::rtl::OUString& aMimeType,
                                uno::Any& aResult )
{
    // produces sequence with data in requested format and returns it in aResult
    if ( pBuf )
    {
        uno::Sequence < sal_Int8 > aData( (sal_Int8*)pBuf, nBufSize );
        uno::Reference < io::XInputStream > xIn = new comphelper::SequenceInputStream( aData );
        try
        {
            uno::Reference < graphic::XGraphicProvider > xGraphicProvider( comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.graphic.GraphicProvider")) ), uno::UNO_QUERY );
            if( xGraphicProvider.is() )
            {
                uno::Sequence< beans::PropertyValue > aMediaProperties( 1 );
                aMediaProperties[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "InputStream" ));
                aMediaProperties[0].Value <<= xIn;
                uno::Reference< graphic::XGraphic > xGraphic( xGraphicProvider->queryGraphic( aMediaProperties  ) );
                if( xGraphic.is() )
                {
                    SvMemoryStream aNewStream( 65535, 65535 );
//                  uno::Reference < io::XOutputStream > xOut = new utl::OOutputStreamHelper( aNewStream.GetLockBytes() );
                    uno::Reference < io::XStream > xOut = new utl::OStreamWrapper( aNewStream );
                    uno::Sequence< beans::PropertyValue > aOutMediaProperties( 2 );
                    aOutMediaProperties[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "OutputStream" ));
                    aOutMediaProperties[0].Value <<= xOut;
                    aOutMediaProperties[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MimeType" ));
                    aOutMediaProperties[1].Value <<= aMimeType;

                    xGraphicProvider->storeGraphic( xGraphic, aOutMediaProperties );
                    aResult <<= uno::Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aNewStream.GetData() ), aNewStream.Seek( STREAM_SEEK_TO_END ) );
                    return sal_True;
                }
            }
        }
        catch (uno::Exception&)
        {}
    }

    return sal_False;
}

// =====================================================================
// MainThreadNotificationRequest
// =====================================================================
MainThreadNotificationRequest::MainThreadNotificationRequest( const ::rtl::Reference< OleEmbeddedObject >& xObj, sal_uInt16 nNotificationType, sal_uInt32 nAspect )
: m_pObject( xObj.get() )
, m_xObject( static_cast< embed::XEmbeddedObject* >( xObj.get() ) )
, m_nNotificationType( nNotificationType )
, m_nAspect( nAspect )
{}

void SAL_CALL MainThreadNotificationRequest::notify (const uno::Any& ) throw (uno::RuntimeException)
{
    if ( m_pObject )
    {
        try
        {
            uno::Reference< uno::XInterface > xLock = m_xObject.get();
            if ( xLock.is() )
            {
                // this is the main thread, the solar mutex must be locked
                if ( m_nNotificationType == OLECOMP_ONCLOSE )
                    m_pObject->OnClosed_Impl();
                else if ( m_nAspect == embed::Aspects::MSOLE_CONTENT )
                    m_pObject->OnViewChanged_Impl();
                else if ( m_nAspect == embed::Aspects::MSOLE_ICON )
                    m_pObject->OnIconChanged_Impl();
            }
        }
        catch( uno::Exception& )
        {
            // ignore all the errors
        }
    }
}

MainThreadNotificationRequest::~MainThreadNotificationRequest()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
