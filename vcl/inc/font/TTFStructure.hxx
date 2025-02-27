/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <o3tl/BigEndianTypes.hxx>

namespace font
{
// make sure no padding bytes are added for the struct
#pragma pack(push, 1)

/** Table directory of a TTF font, values are all in big endian */
struct TableDirectory
{
    o3tl::sal_uInt32_BE nSfntVersion;
    o3tl::sal_uInt16_BE nNumberOfTables;
    o3tl::sal_uInt16_BE nSearchRange;
    o3tl::sal_uInt16_BE nEntrySelector;
    o3tl::sal_uInt16_BE nRangeShift;
};

/** Table directory entry
 *
 * Array of those follows TableDirectory structure.
 */
struct TableDirectoryEntry
{
    o3tl::sal_uInt32_BE tag;
    o3tl::sal_uInt32_BE checkSum;
    o3tl::sal_uInt32_BE offset;
    o3tl::sal_uInt32_BE length;
};

/** Structure of the OS2 table (Version 4)
 *
 * See: https://learn.microsoft.com/en-us/typography/opentype/spec/os2
 */
struct OS2Table
{
    o3tl::sal_uInt16_BE nVersion;
    o3tl::sal_uInt16_BE nXAvgCharWidth; // FWORD
    o3tl::sal_uInt16_BE nWeightClass;
    o3tl::sal_uInt16_BE nWidthClass;
    o3tl::sal_uInt16_BE nFsType;
    o3tl::sal_uInt16_BE nSubscriptXSize; // FWORD
    o3tl::sal_uInt16_BE nSubscriptYSize; // FWORD
    o3tl::sal_uInt16_BE nSubscriptXOffset; // FWORD
    o3tl::sal_uInt16_BE nSubscriptYOffset; // FWORD
    o3tl::sal_uInt16_BE nSuperscriptXSize; // FWORD
    o3tl::sal_uInt16_BE nSuperscriptYSize; // FWORD
    o3tl::sal_uInt16_BE nSuperscriptXOffset; // FWORD
    o3tl::sal_uInt16_BE nSuperscriptYOffset; // FWORD
    o3tl::sal_uInt16_BE nStrikeoutSize; // FWORD
    o3tl::sal_uInt16_BE nStrikeoutPosition; // FWORD
    o3tl::sal_uInt16_BE nFamilyClass;
    sal_uInt8 nPanose[10];
    o3tl::sal_uInt32_BE nUnicodeRange1;
    o3tl::sal_uInt32_BE nUnicodeRange2;
    o3tl::sal_uInt32_BE nUnicodeRange3;
    o3tl::sal_uInt32_BE nUnicodeRange4;
    sal_uInt8 nFontVendorID[4]; // Tag type
    o3tl::sal_uInt16_BE nFsSelection;
    o3tl::sal_uInt16_BE nFirstCharIndex;
    o3tl::sal_uInt16_BE nLastCharIndex;
    o3tl::sal_uInt16_BE nTypoAscender; // FWORD
    o3tl::sal_uInt16_BE nTypoDescender; // FWORD
    o3tl::sal_uInt16_BE nTypoLineGap; // FWORD
    o3tl::sal_uInt16_BE nWinAscent; // UFWORD
    o3tl::sal_uInt16_BE nWinDescent; // UFWORD
    o3tl::sal_uInt32_BE nCodePageRange1;
    o3tl::sal_uInt32_BE nCodePageRange2;
    o3tl::sal_uInt16_BE nXHeight; // FWORD
    o3tl::sal_uInt16_BE nCapHeight; // FWORD
    o3tl::sal_uInt16_BE nDefaultChar;
    o3tl::sal_uInt16_BE nBreakChar;
    o3tl::sal_uInt16_BE nMaxContext;
    o3tl::sal_uInt16_BE nLowerOpticalPointSize;
    o3tl::sal_uInt16_BE nUpperOpticalPointSize;
};

// Check the size of OS2Table struct is as expected
static_assert(sizeof(OS2Table) == 100);

/** Structure of "head" table.
 *
 * See: https://learn.microsoft.com/en-us/typography/opentype/spec/head
 */
struct HeadTable
{
    sal_uInt16 nMajorVersion;
    sal_uInt16 nMinorVersion;
    sal_uInt32 nFontRevision;
    o3tl::sal_uInt32_BE nCheckSumAdjustment;
    o3tl::sal_uInt32_BE nMagicNumber;
    o3tl::sal_uInt16_BE nFlags;
    o3tl::sal_uInt16_BE nUnitsPerEm;
    sal_Int64 nCreated; // LONGDATETIME - signed 64-bit (TODO: need a BE type)
    sal_Int64 nModified; // LONGDATETIME - signed 64-bit (TODO: need a BE type)
    o3tl::sal_uInt16_BE nXMin;
    o3tl::sal_uInt16_BE nXMax;
    o3tl::sal_uInt16_BE nYMin;
    o3tl::sal_uInt16_BE nYMax;
    o3tl::sal_uInt16_BE nMacStyle;
    o3tl::sal_uInt16_BE nLowestRectPPEM;
    o3tl::sal_uInt16_BE nFontDirectionHint;
    o3tl::sal_uInt16_BE nIndexToLocFormat;
    o3tl::sal_uInt16_BE nGlyphDataFormat;
};

// Check the size of HeadTable struct is as expected
static_assert(sizeof(HeadTable) == 54);

/** Structure of "name" table (Version 0)
 *
 * See: https://learn.microsoft.com/en-us/typography/opentype/spec/name
 */
struct NameTable
{
    o3tl::sal_uInt16_BE nVersion;
    o3tl::sal_uInt16_BE nCount;
    o3tl::sal_uInt16_BE nStorageOffset;
    // Following this are NameRecords -> nCount times
};

/** Name record structure
 *
 * Array of those follows NameTable structure.
 */
struct NameRecord
{
    o3tl::sal_uInt16_BE nPlatformID;
    o3tl::sal_uInt16_BE nEncodingID;
    o3tl::sal_uInt16_BE nLanguageID;
    o3tl::sal_uInt16_BE nNameID;
    o3tl::sal_uInt16_BE nLength; // (in bytes)
    o3tl::sal_uInt16_BE nStringOffset; // offset from start of storage area (in bytes)
};

/** Name IDs
 *
 * See https://learn.microsoft.com/en-us/typography/opentype/spec/name#name-ids
 */
enum class NameID : sal_uInt16
{
    Copyright = 0, // example: "© Copyright..."
    FamilyName = 1, // example: "Times New Roman"
    SubfamilyName = 2, // example: "Bold"
    UniqueID = 3, // example: "Monotype: Times New Roman Bold: 1990"
    FullFontName = 4, // example: "Times New Roman Bold"
    Version = 5, // example: "Version 1.00 June 1, 1990, initial release"
    PostScriptName = 6, // example: "TimesNewRoman-Bold"
};

#pragma pack(pop)

} // end font namespace

// Standard TrueType table tags
constexpr sal_uInt32 T_maxp = 0x6D617870;
constexpr sal_uInt32 T_glyf = 0x676C7966;
constexpr sal_uInt32 T_head = 0x68656164;
constexpr sal_uInt32 T_loca = 0x6C6F6361;
constexpr sal_uInt32 T_name = 0x6E616D65;
constexpr sal_uInt32 T_hhea = 0x68686561;
constexpr sal_uInt32 T_hmtx = 0x686D7478;
constexpr sal_uInt32 T_cmap = 0x636D6170;
constexpr sal_uInt32 T_vhea = 0x76686561;
constexpr sal_uInt32 T_vmtx = 0x766D7478;
constexpr sal_uInt32 T_OS2 = 0x4F532F32;
constexpr sal_uInt32 T_post = 0x706F7374;
constexpr sal_uInt32 T_cvt = 0x63767420;
constexpr sal_uInt32 T_prep = 0x70726570;
constexpr sal_uInt32 T_fpgm = 0x6670676D;
constexpr sal_uInt32 T_CFF = 0x43464620;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
