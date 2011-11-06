/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    /** Returns the OLE color from the passed UNO RGB color.
     */
    static sal_uInt32   encodeOleColor( sal_Int32 nRgbColor );

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
