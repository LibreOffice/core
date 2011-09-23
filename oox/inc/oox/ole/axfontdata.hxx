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

#ifndef OOX_OLE_AXFONTDATA_HXX
#define OOX_OLE_AXFONTDATA_HXX

#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/helper/refvector.hxx"

namespace oox {
namespace ole {

// ============================================================================

const sal_Char* const AX_GUID_CFONT         = "{AFC20920-DA4E-11CE-B943-00AA006887B4}";

const sal_uInt32 AX_FONTDATA_BOLD           = 0x00000001;
const sal_uInt32 AX_FONTDATA_ITALIC         = 0x00000002;
const sal_uInt32 AX_FONTDATA_UNDERLINE      = 0x00000004;
const sal_uInt32 AX_FONTDATA_STRIKEOUT      = 0x00000008;
const sal_uInt32 AX_FONTDATA_DISABLED       = 0x00002000;
const sal_uInt32 AX_FONTDATA_AUTOCOLOR      = 0x40000000;

const sal_Int32 AX_FONTDATA_LEFT            = 1;
const sal_Int32 AX_FONTDATA_RIGHT           = 2;
const sal_Int32 AX_FONTDATA_CENTER          = 3;

/** All entries of a font property. */
struct AxFontData
{
    ::rtl::OUString     maFontName;         /// Name of the used font.
    sal_uInt32          mnFontEffects;      /// Font effect flags.
    sal_Int32           mnFontHeight;       /// Height of the font (not really twips, see code).
    sal_Int32           mnFontCharSet;      /// Windows character set of the font.
    sal_Int32           mnHorAlign;         /// Horizontal text alignment.
    bool                mbDblUnderline;     /// True = double underline style (legacy VML drawing controls only).

    explicit            AxFontData();

    /** Converts the internal representation of the font height to points. */
    sal_Int16           getHeightPoints() const;
    /** Converts the passed font height from points to the internal representation. */
    void                setHeightPoints( sal_Int16 nPoints );

    /** Reads the font data settings from the passed input stream. */
    bool                importBinaryModel( BinaryInputStream& rInStrm );

    void                exportBinaryModel( BinaryOutputStream& rOutStrm );
    /** Reads the font data settings from the passed input stream that contains
        an OLE StdFont structure. */
    bool                importStdFont( BinaryInputStream& rInStrm );
    /** Reads the font data settings from the passed input stream depending on
        the GUID preceding the actual font data. */
    bool                importGuidAndFont( BinaryInputStream& rInStrm );
};


} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
