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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/seqstream.hxx>
#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>

#include "graphconvert.hxx"
#include "mtnotification.hxx"
#include <oleembobj.hxx>


using namespace ::com::sun::star;


bool ConvertBufferToFormat( void* pBuf,
                                sal_uInt32 nBufSize,
                                const OUString& aMimeType,
                                uno::Any& aResult )
{
    // produces sequence with data in requested format and returns it in aResult
    if ( pBuf )
    {
        // First, in case the buffer is already in the requested format, then avoid a conversion.
        SvMemoryStream aMemoryStream(pBuf, nBufSize, StreamMode::READ);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        sal_uInt16 nRetFormat = 0;
        if (rFilter.CanImportGraphic(OUString(), aMemoryStream, GRFILTER_FORMAT_DONTKNOW, &nRetFormat) == ERRCODE_NONE &&
                rFilter.GetImportFormatMediaType(nRetFormat) == aMimeType)
        {
            aResult <<= uno::Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemoryStream.GetData() ), aMemoryStream.TellEnd() );
            return true;
        }

        uno::Sequence < sal_Int8 > aData( static_cast<sal_Int8*>(pBuf), nBufSize );
        uno::Reference < io::XInputStream > xIn = new comphelper::SequenceInputStream( aData );
        try
        {
            uno::Reference < graphic::XGraphicProvider > xGraphicProvider( graphic::GraphicProvider::create(comphelper::getProcessComponentContext()));
            uno::Sequence< beans::PropertyValue > aMediaProperties{ comphelper::makePropertyValue(
                "InputStream", xIn) };
            uno::Reference< graphic::XGraphic > xGraphic( xGraphicProvider->queryGraphic( aMediaProperties  ) );
            if( xGraphic.is() )
            {
                SvMemoryStream aNewStream( 65535, 65535 );
                uno::Reference < io::XStream > xOut = new utl::OStreamWrapper( aNewStream );
                uno::Sequence< beans::PropertyValue > aOutMediaProperties{
                    comphelper::makePropertyValue("OutputStream", xOut),
                    comphelper::makePropertyValue("MimeType", aMimeType)
                };

                xGraphicProvider->storeGraphic( xGraphic, aOutMediaProperties );
                aResult <<= uno::Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aNewStream.GetData() ), aNewStream.TellEnd() );
                return true;
            }
        }
        catch (const uno::Exception&)
        {}
    }

    return false;
}


// MainThreadNotificationRequest

MainThreadNotificationRequest::MainThreadNotificationRequest( const ::rtl::Reference< OleEmbeddedObject >& xObj, sal_uInt16 nNotificationType, sal_uInt32 nAspect )
: m_pObject( xObj.get() )
, m_xObject( static_cast< embed::XEmbeddedObject* >( xObj.get() ) )
, m_nNotificationType( nNotificationType )
, m_nAspect( nAspect )
{}

void SAL_CALL MainThreadNotificationRequest::notify (const uno::Any& )
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
                    OleEmbeddedObject::OnIconChanged_Impl();
            }
        }
        catch( const uno::Exception& )
        {
            // ignore all the errors
        }
    }
}

MainThreadNotificationRequest::~MainThreadNotificationRequest()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
