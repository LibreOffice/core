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

#include "oox/ole/axfontdata.hxx"
#include "oox/ole/olehelper.hxx"
#include "oox/ole/axbinaryreader.hxx"
#include "oox/ole/axbinarywriter.hxx"

namespace oox {
namespace ole {

using ::rtl::OUString;

// ============================================================================

AxFontData::AxFontData() :
    mnFontEffects( 0 ),
    mnFontHeight( 160 ),
    mnFontCharSet( WINDOWS_CHARSET_DEFAULT ),
    mnHorAlign( AX_FONTDATA_LEFT ),
    mbDblUnderline( false )
{
}

sal_Int16 AxFontData::getHeightPoints() const
{
    /*  MSO uses weird font sizes:
        1pt->30, 2pt->45, 3pt->60, 4pt->75, 5pt->105, 6pt->120, 7pt->135,
        8pt->165, 9pt->180, 10pt->195, 11pt->225, ... */
    return getLimitedValue< sal_Int16, sal_Int32 >( (mnFontHeight + 10) / 20, 1, SAL_MAX_INT16 );
}

void AxFontData::setHeightPoints( sal_Int16 nPoints )
{
    mnFontHeight = getLimitedValue< sal_Int32, sal_Int32 >( ((nPoints * 4 + 1) / 3) * 15, 30, 4294967 );
}

bool AxFontData::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readStringProperty( maFontName );
    aReader.readIntProperty< sal_uInt32 >( mnFontEffects );
    aReader.readIntProperty< sal_Int32 >( mnFontHeight );
    aReader.skipIntProperty< sal_Int32 >(); // font offset
    aReader.readIntProperty< sal_uInt8 >( mnFontCharSet );
    aReader.skipIntProperty< sal_uInt8 >(); // font pitch/family
    aReader.readIntProperty< sal_uInt8 >( mnHorAlign );
    aReader.skipIntProperty< sal_uInt16 >(); // font weight
    mbDblUnderline = false;
    return aReader.finalizeImport();
}

void AxFontData::exportBinaryModel( BinaryOutputStream& rOutStrm )
{
    AxBinaryPropertyWriter aWriter( rOutStrm );
    aWriter.writeStringProperty( maFontName );
    aWriter.writeIntProperty< sal_uInt32 >( mnFontEffects );
    aWriter.writeIntProperty< sal_Int32 >( mnFontHeight );
    aWriter.skipProperty(); // font offset
    // TODO make AxFontDataModel::convertFromProperties convert the textencoding
    aWriter.writeIntProperty< sal_uInt8 >( mnFontCharSet );
    aWriter.skipProperty(); // font pitch/family

    aWriter.writeIntProperty< sal_uInt8 >( mnHorAlign );
    aWriter.skipProperty(); // font weight
    aWriter.finalizeExport();
}

bool AxFontData::importStdFont( BinaryInputStream& rInStrm )
{
    StdFontInfo aFontInfo;
    if( OleHelper::importStdFont( aFontInfo, rInStrm, false ) )
    {
        maFontName = aFontInfo.maName;
        mnFontEffects = 0;
        setFlag( mnFontEffects, AX_FONTDATA_BOLD,      aFontInfo.mnWeight >= OLE_STDFONT_BOLD );
        setFlag( mnFontEffects, AX_FONTDATA_ITALIC,    getFlag( aFontInfo.mnFlags, OLE_STDFONT_ITALIC ) );
        setFlag( mnFontEffects, AX_FONTDATA_UNDERLINE, getFlag( aFontInfo.mnFlags, OLE_STDFONT_UNDERLINE ) );
        setFlag( mnFontEffects, AX_FONTDATA_STRIKEOUT, getFlag( aFontInfo.mnFlags,OLE_STDFONT_STRIKE ) );
        mbDblUnderline = false;
        // StdFont stores font height in 1/10,000 of points
        setHeightPoints( getLimitedValue< sal_Int16, sal_Int32 >( aFontInfo.mnHeight / 10000, 0, SAL_MAX_INT16 ) );
        mnFontCharSet = aFontInfo.mnCharSet;
        mnHorAlign = AX_FONTDATA_LEFT;
        return true;
    }
    return false;
}

bool AxFontData::importGuidAndFont( BinaryInputStream& rInStrm )
{
    OUString aGuid = OleHelper::importGuid( rInStrm );
    if( aGuid.equalsAscii( AX_GUID_CFONT ) )
        return importBinaryModel( rInStrm );
    if( aGuid.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(OLE_GUID_STDFONT) ) )
        return importStdFont( rInStrm );
    return false;
}

// ============================================================================

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
