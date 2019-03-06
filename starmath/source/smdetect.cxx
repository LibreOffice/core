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

#include "smdetect.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <sfx2/docfile.hxx>
#include <unotools/mediadescriptor.hxx>
#include <sal/log.hxx>
#include <sot/storage.hxx>

#include "eqnolefilehdr.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using utl::MediaDescriptor;

SmFilterDetect::SmFilterDetect()
{
}

SmFilterDetect::~SmFilterDetect()
{
}

OUString SAL_CALL SmFilterDetect::detect( Sequence< PropertyValue >& lDescriptor )
{
    MediaDescriptor aMediaDesc( lDescriptor );
    uno::Reference< io::XInputStream > xInStream ( aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM()], uno::UNO_QUERY );
    if ( !xInStream.is() )
        return OUString();

    SfxMedium aMedium;
    aMedium.UseInteractionHandler( false );
    aMedium.setStreamToLoadFrom( xInStream, true );

    SvStream *pInStrm = aMedium.GetInStream();
    if ( !pInStrm || pInStrm->GetError() )
        return OUString();

    // Do not attempt to create an SotStorage on a
    // 0-length stream as that would create the compound
    // document header on the stream and effectively write to
    // disk!
    pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
    if ( pInStrm->remainingSize() == 0 )
        return OUString();

    bool bStorageOk = false;
    try
    {
        tools::SvRef<SotStorage> aStorage = new SotStorage( pInStrm, false );
        bStorageOk = !aStorage->GetError();
        if (bStorageOk)
        {
            if ( aStorage->IsStream("Equation Native") )
            {
                sal_uInt8 nVersion;
                if ( GetMathTypeVersion( aStorage.get(), nVersion ) && nVersion <=3 )
                    return OUString("math_MathType_3x");
            }
        }
    }
    catch (const css::ucb::ContentCreationException &e)
    {
        SAL_WARN("starmath", "SmFilterDetect::detect caught " << e);
    }

    if (!bStorageOk)
    {
        // 200 should be enough for the XML
        // version, encoding and !DOCTYPE
        // stuff I hope?
        static const sal_uInt16 nBufferSize = 200;
        char aBuffer[nBufferSize+1];
        pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
        pInStrm->StartReadingUnicodeText( RTL_TEXTENCODING_DONTKNOW ); // avoid BOM marker
        auto nBytesRead = pInStrm->ReadBytes( aBuffer, nBufferSize );
        if (nBytesRead >= 6)
        {
            aBuffer[nBytesRead] = 0;
            bool bIsMathType = false;
            if (0 == strncmp( "<?xml", aBuffer, 5))
                bIsMathType = (strstr( aBuffer, "<math>" ) ||
                               strstr( aBuffer, "<math " ) ||
                               strstr( aBuffer, "<math:math " ));
            else
                // this is the old <math tag to MathML in the beginning of the XML file
                bIsMathType = (0 == strncmp( "<math ", aBuffer, 6) ||
                               0 == strncmp( "<math> ", aBuffer, 7) ||
                               0 == strncmp( "<math:math> ", aBuffer, 12));

            if ( bIsMathType )
                return OUString("math_MathML_XML_Math");
        }
    }

    return OUString();
}

/* XServiceInfo */
OUString SAL_CALL SmFilterDetect::getImplementationName()
{
    return OUString("com.sun.star.comp.math.FormatDetector");
}

/* XServiceInfo */
sal_Bool SAL_CALL SmFilterDetect::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SmFilterDetect::getSupportedServiceNames()
{
    return Sequence< OUString >{ "com.sun.star.frame.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
math_FormatDetector_get_implementation(uno::XComponentContext* /*pCtx*/,
                                       uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SmFilterDetect);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
