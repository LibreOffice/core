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

#include "swdetect.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <sfx2/docfile.hxx>
#include <sot/storage.hxx>
#include <unotools/mediadescriptor.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using utl::MediaDescriptor;

SwFilterDetect::SwFilterDetect()
{
}

SwFilterDetect::~SwFilterDetect()
{
}

OUString SAL_CALL SwFilterDetect::detect( Sequence< PropertyValue >& lDescriptor )
{
    MediaDescriptor aMediaDesc( lDescriptor );
    OUString aTypeName = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_TYPENAME(), OUString() );
    uno::Reference< io::XInputStream > xInStream ( aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM()], uno::UNO_QUERY );
    if ( !xInStream.is() )
        return OUString();

    SfxMedium aMedium;
    aMedium.UseInteractionHandler( false );
    aMedium.setStreamToLoadFrom( xInStream, true );

    SvStream *pInStrm = aMedium.GetInStream();
    if ( !pInStrm || pInStrm->GetError() )
        return OUString();

    bool bIsDetected = false;

    if ( aTypeName == "writer_Rich_Text_Format" )
    {
        pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
        bIsDetected = ( read_uInt8s_ToOString( *pInStrm, 5 ) == "{\\rtf" );
    }
    else if ( aTypeName == "writer_MS_WinWord_5" )
    {
        pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
        const sal_uInt8 nBufSize = 3;
        sal_uInt8 nBuffer[ nBufSize ];
        if (pInStrm->ReadBytes(nBuffer, nBufSize) < nBufSize)
            return OUString();

        bIsDetected = (nBuffer[0] == 0x9B && nBuffer[1] == 0xA5 && nBuffer[2] == 0x21)  // WinWord 1
                   || (nBuffer[0] == 0x9C && nBuffer[1] == 0xA5 && nBuffer[2] == 0x21)  // PMWord 1
                   || (nBuffer[0] == 0xDB && nBuffer[1] == 0xA5 && nBuffer[2] == 0x2D)  // WinWord 2
                   || (nBuffer[0] == 0xDC && nBuffer[1] == 0xA5 && nBuffer[2] == 0x65); // WinWord 6.0/95, as a single stream file
    }
    else
    {
        // Do not attempt to create an SotStorage on a
        // 0-length stream as that would create the compound
        // document header on the stream and effectively write to
        // disk!
        pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
        if ( pInStrm->remainingSize() == 0 )
            return OUString();

        try
        {
            tools::SvRef<SotStorage> aStorage = new SotStorage ( pInStrm, false );
            if ( !aStorage->GetError() )
            {
                bIsDetected = aStorage->IsContained( "WordDocument" );
                if ( bIsDetected && aTypeName.startsWith( "writer_MS_Word_97" ) )
                {
                    bIsDetected = ( aStorage->IsContained("0Table") || aStorage->IsContained("1Table") );

                    // If we are checking the template type, and the document is not a .dot, don't
                    // mis-detect it.
                    if ( bIsDetected && aTypeName == "writer_MS_Word_97_Vorlage" )
                    {
                        // Super ugly hack, but we don't want to use the whole WW8Fib thing here in
                        // the swd library, apparently. We know (do we?) that the "aBits1" byte, as
                        // the variable is called in WW8Fib::WW8Fib(SvStream&,sal_uInt8,sal_uInt32),
                        // is at offset 10 in the WordDocument stream. The fDot bit is bit 0x01 of
                        // that byte.
                        tools::SvRef<SotStorageStream> xWordDocument = aStorage->OpenSotStream("WordDocument", StreamMode::STD_READ);
                        xWordDocument->Seek( 10 );
                        if ( xWordDocument->Tell() == 10 )
                        {
                            sal_uInt8 aBits1;
                            xWordDocument->ReadUChar( aBits1 );
                            // Check fDot bit
                            bIsDetected = ((aBits1 & 0x01) == 0x01);
                        }
                    }
                }
            }
        }
        catch (...)
        {
            bIsDetected = false;
        }
    }

    if ( bIsDetected )
        return aTypeName;

    return OUString();
}

/* XServiceInfo */
OUString SAL_CALL SwFilterDetect::getImplementationName()
{
    return OUString("com.sun.star.comp.writer.FormatDetector" );
}

/* XServiceInfo */
sal_Bool SAL_CALL SwFilterDetect::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SwFilterDetect::getSupportedServiceNames()
{
    Sequence< OUString > seqServiceNames( 3 );
    seqServiceNames.getArray() [0] = "com.sun.star.frame.ExtendedTypeDetection";
    seqServiceNames.getArray() [1] = "com.sun.star.text.FormatDetector";
    seqServiceNames.getArray() [2] = "com.sun.star.text.W4WFormatDetector";
    return seqServiceNames ;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_writer_FormatDetector_get_implementation(css::uno::XComponentContext*,
                                                           css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwFilterDetect());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
