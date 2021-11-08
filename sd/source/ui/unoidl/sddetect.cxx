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

#include "sddetect.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/graphicfilter.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <sot/storage.hxx>
#include <unotools/mediadescriptor.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using utl::MediaDescriptor;

SdFilterDetect::SdFilterDetect()
{
}

SdFilterDetect::~SdFilterDetect()
{
}

OUString SAL_CALL SdFilterDetect::detect( Sequence< beans::PropertyValue >& lDescriptor )
{
    MediaDescriptor aMediaDesc( lDescriptor );
    OUString aTypeName = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_TYPENAME, OUString() );
    uno::Reference< io::XInputStream > xInStream ( aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM], uno::UNO_QUERY );
    if ( !xInStream.is() )
        return OUString();

    SfxMedium aMedium;
    aMedium.UseInteractionHandler( false );
    aMedium.setStreamToLoadFrom( xInStream, true );

    SvStream *pInStrm = aMedium.GetInStream();
    if ( !pInStrm || pInStrm->GetError() )
        return OUString();

    if ( aTypeName.startsWith( "impress_MS_PowerPoint_97" ) )
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
            tools::SvRef<SotStorage> aStorage = new SotStorage( pInStrm, false );
            if ( !aStorage->GetError() && aStorage->IsStream( "PowerPoint Document" ) )
                return aTypeName;
        }
        catch (const css::ucb::ContentCreationException&)
        {
        }
    }
    else
    {
        pInStrm->Seek( STREAM_SEEK_TO_BEGIN );

        const OUString aFileName( aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_URL, OUString() ) );
        GraphicDescriptor aDesc( *pInStrm, &aFileName );
        if( !aDesc.Detect() )
        {
            INetURLObject aCheckURL( aFileName );
            if( aCheckURL.getExtension().equalsIgnoreAsciiCase("cgm") )
            {
                sal_uInt8 n8;
                pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
                pInStrm->ReadUChar( n8 );
                if ( ( n8 & 0xf0 ) == 0 )
                    // we are supporting binary cgm format only, so
                    // this is a small test to exclude cgm text
                    return "impress_CGM_Computer_Graphics_Metafile";
            }
        }
        else
        {
            OUString aShortName( GraphicDescriptor::GetImportFormatShortName( aDesc.GetFileFormat() ) );
            GraphicFilter &rGrfFilter = GraphicFilter::GetGraphicFilter();
            const OUString aName( rGrfFilter.GetImportFormatTypeName( rGrfFilter.GetImportFormatNumberForShortName( aShortName ) ) );

            if ( aShortName.equalsIgnoreAsciiCase( "PCD" ) )    // there is a multiple pcd selection possible
            {
                sal_Int32 nBase = 2;    // default Base0
                if ( aTypeName == "pcd_Photo_CD_Base4" )
                    nBase = 1;
                else if ( aTypeName == "pcd_Photo_CD_Base16" )
                    nBase = 0;
                FilterConfigItem aFilterConfigItem( u"Office.Common/Filter/Graphic/Import/PCD" );
                aFilterConfigItem.WriteInt32( "Resolution" , nBase );
            }

            SfxFilterMatcher aMatch("sdraw");
            std::shared_ptr<const SfxFilter> pFilter = aMatch.GetFilter4FilterName( aName );
            if ( pFilter )
                return pFilter->GetRealTypeName();
        }
    }

    return OUString();
}

// XServiceInfo
OUString SAL_CALL SdFilterDetect::getImplementationName()
{
    return "com.sun.star.comp.draw.FormatDetector";
}

// XServiceInfo
sal_Bool SAL_CALL SdFilterDetect::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

// XServiceInfo
Sequence< OUString > SAL_CALL SdFilterDetect::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ExtendedTypeDetection" };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_draw_FormatDetector_get_implementation(css::uno::XComponentContext*,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SdFilterDetect());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
