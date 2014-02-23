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

#include "oox/ole/axfontdata.hxx"
#include "oox/ole/olehelper.hxx"
#include "oox/ole/axbinaryreader.hxx"
#include "oox/ole/axbinarywriter.hxx"

namespace oox {
namespace ole {



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
    if ( aGuid == OLE_GUID_STDFONT )
        return importStdFont( rInStrm );
    return false;
}



} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
