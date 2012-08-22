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

#ifndef OOX_OLE_AXFONTDATA_HXX
#define OOX_OLE_AXFONTDATA_HXX

#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/helper/refvector.hxx"
#include "oox/dllapi.h"

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
struct OOX_DLLPUBLIC AxFontData
{
    ::rtl::OUString     maFontName;         ///< Name of the used font.
    sal_uInt32          mnFontEffects;      ///< Font effect flags.
    sal_Int32           mnFontHeight;       ///< Height of the font (not really twips, see code).
    sal_Int32           mnFontCharSet;      ///< Windows character set of the font.
    sal_Int32           mnHorAlign;         ///< Horizontal text alignment.
    bool                mbDblUnderline;     ///< True = double underline style (legacy VML drawing controls only).

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
