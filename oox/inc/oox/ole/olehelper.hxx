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

#ifndef OOX_OLE_OLEHELPER_HXX
#define OOX_OLE_OLEHELPER_HXX

#include <rtl/ustring.hxx>
#include "oox/helper/binarystreambase.hxx"

namespace oox {
    class BinaryInputStream;
    class GraphicHelper;
}

namespace oox {
namespace ole {

// ============================================================================

const sal_Char* const OLE_GUID_STDFONT      = "{0BE35203-8F91-11CE-9DE3-00AA004BB851}";
const sal_Char* const OLE_GUID_STDPIC       = "{0BE35204-8F91-11CE-9DE3-00AA004BB851}";
const sal_Char* const OLE_GUID_STDHLINK     = "{79EAC9D0-BAF9-11CE-8C82-00AA004BA90B}";

// ============================================================================

const sal_uInt16 OLE_STDFONT_NORMAL     = 400;
const sal_uInt16 OLE_STDFONT_BOLD       = 700;

const sal_uInt8 OLE_STDFONT_ITALIC      = 0x02;
const sal_uInt8 OLE_STDFONT_UNDERLINE   = 0x04;
const sal_uInt8 OLE_STDFONT_STRIKE      = 0x08;

/** Stores data about a StdFont font structure. */
struct StdFontInfo
{
    ::rtl::OUString     maName;         /// Font name.
    sal_uInt32          mnHeight;       /// Font height (1/10,000 points).
    sal_uInt16          mnWeight;       /// Font weight (normal/bold).
    sal_uInt16          mnCharSet;      /// Font charset.
    sal_uInt8           mnFlags;        /// Font flags.
    
    explicit            StdFontInfo();
    explicit            StdFontInfo(
                            const ::rtl::OUString& rName,
                            sal_uInt32 nHeight,
                            sal_uInt16 nWeight = OLE_STDFONT_NORMAL,
                            sal_uInt16 nCharSet = WINDOWS_CHARSET_ANSI,
                            sal_uInt8 nFlags = 0 );
};

// ============================================================================

/** Stores data about a StdHlink hyperlink. */
struct StdHlinkInfo
{
    ::rtl::OUString     maTarget;
    ::rtl::OUString     maLocation;
    ::rtl::OUString     maDisplay;
    ::rtl::OUString     maFrame;
};

// ============================================================================

/** Static helper functions for OLE import/export. */
class OleHelper
{
public:
    /** Returns the UNO RGB color from the passed encoded OLE color.

        @param bDefaultColorBgr
            True = OLE default color type is treated as BGR color.
            False = OLE default color type is treated as palette color.
     */
    static sal_Int32    decodeOleColor(
                            const GraphicHelper& rGraphicHelper,
                            sal_uInt32 nOleColor,
                            bool bDefaultColorBgr = true );

    /** Imports a GUID from the passed binary stream and returns its string
        representation (in uppercase characters).
     */
    static ::rtl::OUString importGuid( BinaryInputStream& rInStrm );

    /** Imports an OLE StdFont font structure from the current position of the
        passed binary stream.
     */
    static bool         importStdFont(
                            StdFontInfo& orFontInfo,
                            BinaryInputStream& rInStrm,
                            bool bWithGuid );

    /** Imports an OLE StdPic picture from the current position of the passed
        binary stream.
     */
    static bool         importStdPic(
                            StreamDataSequence& orGraphicData,
                            BinaryInputStream& rInStrm,
                            bool bWithGuid );

    /** Imports an OLE StdHlink from the current position of the passed binary
        stream.
     */
    static bool         importStdHlink(
                            StdHlinkInfo& orHlinkInfo,
                            BinaryInputStream& rInStrm,
                            bool bWithGuid );

private:
                        OleHelper();        // not implemented
                        ~OleHelper();       // not implemented
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

