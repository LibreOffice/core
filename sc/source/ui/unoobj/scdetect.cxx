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

#include "scdetect.hxx"

#include <sal/macros.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <unotools/mediadescriptor.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>

using namespace ::com::sun::star;
using utl::MediaDescriptor;

namespace {

const sal_Char pFilterLotus[]        = "Lotus";
const sal_Char pFilterQPro6[]        = "Quattro Pro 6.0";
const sal_Char pFilterDBase[]        = "dBase";
const sal_Char pFilterDif[]      = "DIF";
const sal_Char pFilterSylk[]     = "SYLK";

// Tabelle mit Suchmustern
// Bedeutung der Sequenzen
// 0x00??: genau Byte 0x?? muss an dieser Stelle stehen
// 0x0100: ein Byte ueberlesen (don't care)
// 0x02nn: ein Byte aus 0xnn Alternativen folgt
// 0x8000: Erkennung abgeschlossen

#define M_DC        0x0100
#define M_ALT(ANZ)  (0x0200+(ANZ))
#define M_ENDE      0x8000

const sal_uInt16 pLotus[] =      // Lotus 1/1A/2
    { 0x0000, 0x0000, 0x0002, 0x0000,
    M_ALT(2), 0x0004, 0x0006,
    0x0004, M_ENDE };

const sal_uInt16 pLotusNew[] =   // Lotus >= 9.7
    { 0x0000, 0x0000, M_DC, 0x0000,     // Rec# + Len (0x1a)
      M_ALT(3), 0x0003, 0x0004, 0x0005, // File Revision Code 97->ME
      0x0010, 0x0004, 0x0000, 0x0000,
      M_ENDE };

const sal_uInt16 pLotus2[] =     // Lotus >3
    { 0x0000, 0x0000, 0x001A, 0x0000,   // Rec# + Len (26)
    M_ALT(2), 0x0000, 0x0002,         // File Revision Code
    0x0010,
    0x0004, 0x0000,                   // File Revision Subcode
    M_ENDE };

const sal_uInt16 pQPro[] =
       { 0x0000, 0x0000, 0x0002, 0x0000,
         M_ALT(4), 0x0001, 0x0002, // WB1, WB2
         0x0006, 0x0007,           // QPro 6/7 (?)
         0x0010,
         M_ENDE };

const sal_uInt16 pDIF1[] =       // DIF mit CR-LF
    {
    'T', 'A', 'B', 'L', 'E',
    M_DC, M_DC,
    '0', ',', '1',
    M_DC, M_DC,
    '\"',
    M_ENDE };

const sal_uInt16 pDIF2[] =       // DIF mit CR oder LF
    {
    'T', 'A', 'B', 'L', 'E',
    M_DC,
    '0', ',', '1',
    M_DC,
    '\"',
    M_ENDE };

const sal_uInt16 pSylk[] =       // Sylk
    {
    'I', 'D', ';',
    M_ALT(3), 'P', 'N', 'E',        // 'P' plus undocumented Excel extensions 'N' and 'E'
    M_ENDE };

bool detectThisFormat(SvStream& rStr, const sal_uInt16* pSearch)
{
    sal_uInt8 nByte;
    rStr.Seek( 0 ); // am Anfang war alles Uebel...
    rStr.ReadUChar( nByte );
    bool bSync = true;
    while( !rStr.IsEof() && bSync )
    {
        sal_uInt16 nMuster = *pSearch;

        if( nMuster < 0x0100 )
        { //                                direkter Byte-Vergleich
            if( ( sal_uInt8 ) nMuster != nByte )
                bSync = false;
        }
        else if( nMuster & M_DC )
        { //                                             don't care
        }
        else if( nMuster & M_ALT(0) )
        { //                                      alternative Bytes
            sal_uInt8 nAnzAlt = ( sal_uInt8 ) nMuster;
            bSync = false;          // zunaechst unsynchron
            while( nAnzAlt > 0 )
            {
                pSearch++;
                if( ( sal_uInt8 ) *pSearch == nByte )
                    bSync = true;   // jetzt erst Synchronisierung
                nAnzAlt--;
            }
        }
        else if( nMuster & M_ENDE )
        { //                                        Format detected
            return true;
        }

        pSearch++;
        rStr.ReadUChar( nByte );
    }

    return false;
}

}

ScFilterDetect::ScFilterDetect( const uno::Reference<uno::XComponentContext>& /*xContext*/ )
{
}

ScFilterDetect::~ScFilterDetect()
{
}

#if 0
// This method is no longer used, but I do want to keep this for now to see
// if we could transfer this check to the now centralized ascii detection
// code in the filter module.
static sal_Bool lcl_MayBeAscii( SvStream& rStream )
{
    // ASCII/CSV is considered possible if there are no null bytes, or a Byte
    // Order Mark is present, or if, for Unicode UCS2/UTF-16, all null bytes
    // are on either even or uneven byte positions.

    rStream.Seek(STREAM_SEEK_TO_BEGIN);

    const size_t nBufSize = 2048;
    sal_uInt16 aBuffer[ nBufSize ];
    sal_uInt8* pByte = reinterpret_cast<sal_uInt8*>(aBuffer);
    sal_uLong nBytesRead = rStream.Read( pByte, nBufSize*2);

    if ( nBytesRead >= 2 && (aBuffer[0] == 0xfffe || aBuffer[0] == 0xfeff) )
    {
        // Unicode BOM file may contain null bytes.
        return sal_True;
    }

    const sal_uInt16* p = aBuffer;
    sal_uInt16 nMask = 0xffff;
    nBytesRead /= 2;
    while( nBytesRead-- && nMask )
    {
        sal_uInt16 nVal = *p++ & nMask;
        if (!(nVal & 0x00ff))
            nMask &= 0xff00;
        if (!(nVal & 0xff00))
            nMask &= 0x00ff;
    }

    return nMask != 0;
}
#endif

static bool lcl_MayBeDBase( SvStream& rStream )
{
    // Look for dbf marker, see connectivity/source/inc/dbase/DTable.hxx
    // DBFType for values.
    const sal_uInt8 nValidMarks[] = {
        0x03, 0x04, 0x05, 0x30, 0x43, 0xB3, 0x83, 0x8b, 0x8e, 0xf5 };
    sal_uInt8 nMark;
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream.ReadUChar( nMark );
    bool bValidMark = false;
    for (size_t i=0; i < sizeof(nValidMarks)/sizeof(nValidMarks[0]) && !bValidMark; ++i)
    {
        if (nValidMarks[i] == nMark)
            bValidMark = true;
    }
    if ( !bValidMark )
        return false;

    const size_t nHeaderBlockSize = 32;
    // Empty dbf is >= 32*2+1 bytes in size.
    const size_t nEmptyDbf = nHeaderBlockSize * 2 + 1;

    rStream.Seek(STREAM_SEEK_TO_END);
    sal_uLong nSize = rStream.Tell();
    if ( nSize < nEmptyDbf )
        return false;

    // length of header starts at 8
    rStream.Seek(8);
    sal_uInt16 nHeaderLen;
    rStream.ReadUInt16( nHeaderLen );

    if ( nHeaderLen < nEmptyDbf || nSize < nHeaderLen )
        return false;

    // Last byte of header must be 0x0d, this is how it's specified.
    // #i9581#,#i26407# but some applications don't follow the specification
    // and pad the header with one byte 0x00 to reach an
    // even boundary. Some (#i88577# ) even pad more or pad using a 0x1a ^Z
    // control character (#i8857#). This results in:
    // Last byte of header must be 0x0d on 32 bytes boundary.
    sal_uInt16 nBlocks = (nHeaderLen - 1) / nHeaderBlockSize;
    sal_uInt8 nEndFlag = 0;
    while ( nBlocks > 1 && nEndFlag != 0x0d ) {
        rStream.Seek( nBlocks-- * nHeaderBlockSize );
        rStream.ReadUChar( nEndFlag );
    }

    return ( 0x0d == nEndFlag );
}

OUString SAL_CALL ScFilterDetect::detect( uno::Sequence<beans::PropertyValue>& lDescriptor )
    throw( uno::RuntimeException, std::exception )
{
    MediaDescriptor aMediaDesc( lDescriptor );
    OUString aTypeName = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_TYPENAME(), OUString() );
    uno::Reference< io::XInputStream > xStream ( aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM()], uno::UNO_QUERY );
    if ( !xStream.is() )
        return OUString();

    SfxMedium aMedium;
    aMedium.UseInteractionHandler( false );
    aMedium.setStreamToLoadFrom( xStream, true );

    SvStream* pStream = aMedium.GetInStream();
    if ( !pStream || pStream->GetError() )
        // No stream, no detection.
        return OUString();

    const char* pSearchFilterName = nullptr;
    if (aTypeName == "calc_Lotus")
    {
        if (!detectThisFormat(*pStream, pLotus) && !detectThisFormat(*pStream, pLotusNew) && !detectThisFormat(*pStream, pLotus2))
            return OUString();

        pSearchFilterName = pFilterLotus;
    }
    else if (aTypeName == "calc_QPro")
    {
        if (!detectThisFormat(*pStream, pQPro))
            return OUString();

        pSearchFilterName = pFilterQPro6;
    }
    else if (aTypeName == "calc_SYLK")
    {
        if (!detectThisFormat(*pStream, pSylk))
            return OUString();

        pSearchFilterName = pFilterSylk;
    }
    else if (aTypeName == "calc_DIF")
    {
        if (!detectThisFormat(*pStream, pDIF1) && !detectThisFormat(*pStream, pDIF2))
            return OUString();

        pSearchFilterName = pFilterDif;
    }
    else if (aTypeName == "calc_dBase")
    {
        if (!lcl_MayBeDBase(*pStream))
            return OUString();

        pSearchFilterName = pFilterDBase;
    }
    else
        return OUString();

    SfxFilterMatcher aMatcher("scalc");
    const SfxFilter* pFilter = aMatcher.GetFilter4FilterName(OUString::createFromAscii(pSearchFilterName));

    if (!pFilter)
        return OUString();

    aMediaDesc[MediaDescriptor::PROP_FILTERNAME()] <<= pFilter->GetName();
    aMediaDesc >> lDescriptor;
    return aTypeName;
}

OUString SAL_CALL ScFilterDetect::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.calc.FormatDetector");
}

sal_Bool ScFilterDetect::supportsService( const OUString& sServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence<OUString> ScFilterDetect::getSupportedServiceNames()
    throw (uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> seqServiceNames { "com.sun.star.frame.ExtendedTypeDetection" };
    return seqServiceNames;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_calc_FormatDetector_get_implementation(css::uno::XComponentContext* context,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScFilterDetect(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
