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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XLESCHER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XLESCHER_HXX

#include <tools/gen.hxx>
#include <tools/mapunit.hxx>
#include "fapihelper.hxx"
#include "xladdress.hxx"
#include "xlstyle.hxx"
#include "xistream.hxx"

namespace com { namespace sun { namespace star {
    namespace drawing { class XShape; }
    namespace awt { class XControlModel; }
    namespace script { struct ScriptEventDescriptor; }
} } }

class Rectangle;

// Constants and Enumerations =================================================

// (0x001C) NOTE --------------------------------------------------------------

const sal_uInt16 EXC_ID_NOTE                = 0x001C;
const sal_uInt16 EXC_NOTE_VISIBLE           = 0x0002;
const sal_uInt16 EXC_NOTE5_MAXLEN           = 2048;

// (0x005D) OBJ ---------------------------------------------------------------

const sal_uInt16 EXC_ID_OBJ                 = 0x005D;

const sal_uInt16 EXC_OBJ_INVALID_ID         = 0;

// object types
const sal_uInt16 EXC_OBJTYPE_GROUP          = 0;
const sal_uInt16 EXC_OBJTYPE_LINE           = 1;
const sal_uInt16 EXC_OBJTYPE_RECTANGLE      = 2;
const sal_uInt16 EXC_OBJTYPE_OVAL           = 3;
const sal_uInt16 EXC_OBJTYPE_ARC            = 4;
const sal_uInt16 EXC_OBJTYPE_CHART          = 5;
const sal_uInt16 EXC_OBJTYPE_TEXT           = 6;
const sal_uInt16 EXC_OBJTYPE_BUTTON         = 7;
const sal_uInt16 EXC_OBJTYPE_PICTURE        = 8;
const sal_uInt16 EXC_OBJTYPE_POLYGON        = 9;        // new in BIFF4
const sal_uInt16 EXC_OBJTYPE_CHECKBOX       = 11;       // new in BIFF5
const sal_uInt16 EXC_OBJTYPE_OPTIONBUTTON   = 12;
const sal_uInt16 EXC_OBJTYPE_EDIT           = 13;
const sal_uInt16 EXC_OBJTYPE_LABEL          = 14;
const sal_uInt16 EXC_OBJTYPE_DIALOG         = 15;
const sal_uInt16 EXC_OBJTYPE_SPIN           = 16;
const sal_uInt16 EXC_OBJTYPE_SCROLLBAR      = 17;
const sal_uInt16 EXC_OBJTYPE_LISTBOX        = 18;
const sal_uInt16 EXC_OBJTYPE_GROUPBOX       = 19;
const sal_uInt16 EXC_OBJTYPE_DROPDOWN       = 20;
const sal_uInt16 EXC_OBJTYPE_NOTE           = 25;       // new in BIFF8
const sal_uInt16 EXC_OBJTYPE_DRAWING        = 30;
const sal_uInt16 EXC_OBJTYPE_UNKNOWN        = 0xFFFF;   /// For internal use only.

// BIFF3-BIFF5 flags
const sal_uInt16 EXC_OBJ_HIDDEN             = 0x0100;
const sal_uInt16 EXC_OBJ_VISIBLE            = 0x0200;
const sal_uInt16 EXC_OBJ_PRINTABLE          = 0x0400;

// BIFF5 line formatting
const sal_uInt8 EXC_OBJ_LINE_AUTOCOLOR      = 64;

const sal_uInt8 EXC_OBJ_LINE_SOLID          = 0;
const sal_uInt8 EXC_OBJ_LINE_DASH           = 1;
const sal_uInt8 EXC_OBJ_LINE_DOT            = 2;
const sal_uInt8 EXC_OBJ_LINE_DASHDOT        = 3;
const sal_uInt8 EXC_OBJ_LINE_DASHDOTDOT     = 4;
const sal_uInt8 EXC_OBJ_LINE_MEDTRANS       = 5;
const sal_uInt8 EXC_OBJ_LINE_DARKTRANS      = 6;
const sal_uInt8 EXC_OBJ_LINE_LIGHTTRANS     = 7;
const sal_uInt8 EXC_OBJ_LINE_NONE           = 255;

const sal_uInt8 EXC_OBJ_LINE_HAIR           = 0;
const sal_uInt8 EXC_OBJ_LINE_THIN           = 1;
const sal_uInt8 EXC_OBJ_LINE_MEDIUM         = 2;
const sal_uInt8 EXC_OBJ_LINE_THICK          = 3;

const sal_uInt8 EXC_OBJ_LINE_AUTO           = 0x01;

const sal_uInt8 EXC_OBJ_ARROW_NONE          = 0;
const sal_uInt8 EXC_OBJ_ARROW_OPEN          = 1;
const sal_uInt8 EXC_OBJ_ARROW_FILLED        = 2;
const sal_uInt8 EXC_OBJ_ARROW_OPENBOTH      = 3;
const sal_uInt8 EXC_OBJ_ARROW_FILLEDBOTH    = 4;

const sal_uInt8 EXC_OBJ_ARROW_NARROW        = 0;
const sal_uInt8 EXC_OBJ_ARROW_MEDIUM        = 1;
const sal_uInt8 EXC_OBJ_ARROW_WIDE          = 2;

const sal_uInt8 EXC_OBJ_LINE_TL             = 0;
const sal_uInt8 EXC_OBJ_LINE_TR             = 1;
const sal_uInt8 EXC_OBJ_LINE_BR             = 2;
const sal_uInt8 EXC_OBJ_LINE_BL             = 3;

// BIFF5 fill formatting
const sal_uInt8 EXC_OBJ_FILL_AUTOCOLOR      = 65;

const sal_uInt8 EXC_OBJ_FILL_AUTO           = 0x01;

// BIFF5 frame formatting
const sal_uInt16 EXC_OBJ_FRAME_SHADOW       = 0x0002;

// BIFF5 text objects
const sal_uInt8 EXC_OBJ_HOR_LEFT            = 1;
const sal_uInt8 EXC_OBJ_HOR_CENTER          = 2;
const sal_uInt8 EXC_OBJ_HOR_RIGHT           = 3;
const sal_uInt8 EXC_OBJ_HOR_JUSTIFY         = 4;

const sal_uInt8 EXC_OBJ_VER_TOP             = 1;
const sal_uInt8 EXC_OBJ_VER_CENTER          = 2;
const sal_uInt8 EXC_OBJ_VER_BOTTOM          = 3;
const sal_uInt8 EXC_OBJ_VER_JUSTIFY         = 4;

const sal_uInt16 EXC_OBJ_ORIENT_NONE        = 0;
const sal_uInt16 EXC_OBJ_ORIENT_STACKED     = 1;        /// Stacked top to bottom.
const sal_uInt16 EXC_OBJ_ORIENT_90CCW       = 2;        /// 90 degr. counterclockwise.
const sal_uInt16 EXC_OBJ_ORIENT_90CW        = 3;        /// 90 degr. clockwise.

const sal_uInt16 EXC_OBJ_TEXT_AUTOSIZE      = 0x0080;
const sal_uInt16 EXC_OBJ_TEXT_LOCKED        = 0x0200;

const sal_Int32 EXC_OBJ_TEXT_MARGIN         = 20000;    /// Automatic text margin (EMUs).

// BIFF5 arc objects
const sal_uInt8 EXC_OBJ_ARC_TR              = 0;
const sal_uInt8 EXC_OBJ_ARC_TL              = 1;
const sal_uInt8 EXC_OBJ_ARC_BL              = 2;
const sal_uInt8 EXC_OBJ_ARC_BR              = 3;

// BIFF5 polygon objects
const sal_uInt16 EXC_OBJ_POLY_CLOSED        = 0x0100;

// BIFF5 pictures/OLE objects
const sal_uInt16 EXC_OBJ_PIC_MANUALSIZE     = 0x0001;
const sal_uInt16 EXC_OBJ_PIC_DDE            = 0x0002;
const sal_uInt16 EXC_OBJ_PIC_SYMBOL         = 0x0008;
const sal_uInt16 EXC_OBJ_PIC_CONTROL        = 0x0010;   /// Form control (BIFF8).
const sal_uInt16 EXC_OBJ_PIC_CTLSSTREAM     = 0x0020;   /// Data in Ctls stream (BIFF8).
const sal_uInt16 EXC_OBJ_PIC_AUTOLOAD       = 0x0200;   /// Auto-load form control (BIFF8).

// BIFF5 button objects
const sal_uInt16 EXC_OBJ_BUTTON_DEFAULT     = 0x0001;
const sal_uInt16 EXC_OBJ_BUTTON_HELP        = 0x0002;
const sal_uInt16 EXC_OBJ_BUTTON_CANCEL      = 0x0004;
const sal_uInt16 EXC_OBJ_BUTTON_CLOSE       = 0x0008;

// BIFF5 checkboxes, radio buttons
const sal_uInt16 EXC_OBJ_CHECKBOX_UNCHECKED = 0;
const sal_uInt16 EXC_OBJ_CHECKBOX_CHECKED   = 1;
const sal_uInt16 EXC_OBJ_CHECKBOX_TRISTATE  = 2;
const sal_uInt16 EXC_OBJ_CHECKBOX_FLAT      = 0x0001;

// BIFF5 editbox objects
const sal_uInt16 EXC_OBJ_EDIT_TEXT          = 0;
const sal_uInt16 EXC_OBJ_EDIT_INTEGER       = 1;
const sal_uInt16 EXC_OBJ_EDIT_DOUBLE        = 2;
const sal_uInt16 EXC_OBJ_EDIT_REFERENCE     = 3;
const sal_uInt16 EXC_OBJ_EDIT_FORMULA       = 4;

// BIFF5 scrollbars/spinbuttons
const sal_uInt16 EXC_OBJ_SCROLLBAR_MIN      = 0;
const sal_uInt16 EXC_OBJ_SCROLLBAR_MAX      = 30000;

const sal_uInt16 EXC_OBJ_SCROLLBAR_HOR      = 0x0001;

const sal_uInt16 EXC_OBJ_SCROLLBAR_DEFFLAGS = 0x0001;
const sal_uInt16 EXC_OBJ_SCROLLBAR_FLAT     = 0x0008;

// BIFF5 listboxes/dropdowns
const sal_uInt8 EXC_OBJ_LISTBOX_SINGLE      = 0;        /// Single selection.
const sal_uInt8 EXC_OBJ_LISTBOX_MULTI       = 1;        /// Multi selection.
const sal_uInt8 EXC_OBJ_LISTBOX_RANGE       = 2;        /// Range selection.

const sal_uInt16 EXC_OBJ_LISTBOX_EDIT       = 0x0002;
const sal_uInt16 EXC_OBJ_LISTBOX_FLAT       = 0x0008;

// BIFF5 dropdown listboxes
const sal_uInt16 EXC_OBJ_DROPDOWN_LISTBOX   = 0;        /// Listbox, text not editable.
const sal_uInt16 EXC_OBJ_DROPDOWN_COMBOBOX  = 1;        /// Dropdown listbox with editable text.
const sal_uInt16 EXC_OBJ_DROPDOWN_SIMPLE    = 2;        /// Dropdown button only, no text area.
const sal_uInt16 EXC_OBJ_DROPDOWN_MAX       = 3;
const sal_uInt16 EXC_OBJ_DROPDOWN_FILTERED  = 0x0008;   /// Dropdown style: filtered.

// BIFF5 groupboxes
const sal_uInt16 EXC_OBJ_GROUPBOX_FLAT      = 0x0001;

// BIFF8 sub records
const sal_uInt16 EXC_ID_OBJEND              = 0x0000;   /// End of OBJ.
const sal_uInt16 EXC_ID_OBJMACRO            = 0x0004;   /// Macro link.
const sal_uInt16 EXC_ID_OBJBUTTON           = 0x0005;   /// Button data.
const sal_uInt16 EXC_ID_OBJGMO              = 0x0006;   /// Group marker.
const sal_uInt16 EXC_ID_OBJCF               = 0x0007;   /// Clipboard format.
const sal_uInt16 EXC_ID_OBJFLAGS            = 0x0008;   /// Option flags.
const sal_uInt16 EXC_ID_OBJPICTFMLA         = 0x0009;   /// OLE link formula.
const sal_uInt16 EXC_ID_OBJCBLS             = 0x000A;   /// Check box/radio button data.
const sal_uInt16 EXC_ID_OBJRBO              = 0x000B;   /// Radio button group data.
const sal_uInt16 EXC_ID_OBJSBS              = 0x000C;   /// Scroll bar data.
const sal_uInt16 EXC_ID_OBJNTS              = 0x000D;   /// Note data.
const sal_uInt16 EXC_ID_OBJSBSFMLA          = 0x000E;   /// Scroll bar/list box/combo box cell link.
const sal_uInt16 EXC_ID_OBJGBODATA          = 0x000F;   /// Group box data.
const sal_uInt16 EXC_ID_OBJEDODATA          = 0x0010;   /// Edit box data.
const sal_uInt16 EXC_ID_OBJRBODATA          = 0x0011;   /// Radio button group data.
const sal_uInt16 EXC_ID_OBJCBLSDATA         = 0x0012;   /// Check box/radio button data.
const sal_uInt16 EXC_ID_OBJLBSDATA          = 0x0013;   /// List box/combo box data.
const sal_uInt16 EXC_ID_OBJCBLSFMLA         = 0x0014;   /// Check box/radio button cell link.
const sal_uInt16 EXC_ID_OBJCMO              = 0x0015;   /// Common object settings.
const sal_uInt16 EXC_ID_OBJUNKNOWN          = 0xFFFF;   /// For internal use only.

// BIFF8 OBJCMO: flags
const sal_uInt16 EXC_OBJCMO_PRINTABLE       = 0x0010;   /// Object printable.
const sal_uInt16 EXC_OBJCMO_AUTOLINE        = 0x2000;   /// Automatic line formatting.
const sal_uInt16 EXC_OBJCMO_AUTOFILL        = 0x4000;   /// Automatic fill formatting.

/** Value binding mode for cells linked to form controls. */
enum XclCtrlBindMode
{
    EXC_CTRL_BINDCONTENT,       /// Binds cell to content of control.
    EXC_CTRL_BINDPOSITION       /// Binds cell to position in control (e.g. listbox selection index).
};

// (0x007F) IMGDATA -----------------------------------------------------------

const sal_uInt16 EXC_ID3_IMGDATA            = 0x007F;
const sal_uInt16 EXC_ID8_IMGDATA            = 0x00E9;

const sal_uInt16 EXC_IMGDATA_WMF            = 2;
const sal_uInt16 EXC_IMGDATA_BMP            = 9;

const sal_uInt16 EXC_IMGDATA_WIN            = 1;
const sal_uInt16 EXC_IMGDATA_MAC            = 2;

const sal_uInt32 EXC_IMGDATA_MAXREC8        = 0x201C;
const sal_uInt32 EXC_IMGDATA_MAXCONT8       = 0x2014;

// (0x00A9) COORDLIST ---------------------------------------------------------

const sal_uInt16 EXC_ID_COORDLIST           = 0x00A9;

// (0x00EB) MSODRAWINGGROUP ---------------------------------------------------

const sal_uInt16 EXC_ID_MSODRAWINGGROUP     = 0x00EB;

// (0x00EC) MSODRAWING --------------------------------------------------------

const sal_uInt16 EXC_ID_MSODRAWING          = 0x00EC;

// additional flags not extant in svx headers
const sal_uInt16 EXC_ESC_ANCHOR_POSLOCKED   = 0x0001;
const sal_uInt16 EXC_ESC_ANCHOR_SIZELOCKED  = 0x0002;
const sal_uInt16 EXC_ESC_ANCHOR_LOCKED      = EXC_ESC_ANCHOR_POSLOCKED|EXC_ESC_ANCHOR_SIZELOCKED;

// (0x00ED) MSODRAWINGSELECTION -----------------------------------------------

const sal_uInt16 EXC_ID_MSODRAWINGSEL       = 0x00ED;

// (0x01B6) TXO ---------------------------------------------------------------

const sal_uInt16 EXC_ID_TXO                 = 0x01B6;

// TXO constants are equal to BIFF5 OBJ text object flags

// Structs and classes ========================================================

/** Represents the position (anchor) of an object in a Calc document. */
struct XclObjAnchor : public XclRange
{
    sal_uInt16          mnLX;       /// X offset in left column (1/1024 of column width).
    sal_uInt32          mnTY;       /// Y offset in top row (1/256 of row height).
    sal_uInt16          mnRX;       /// X offset in right column (1/1024 of column width).
    sal_uInt32          mnBY;       /// Y offset in bottom row (1/256 of row height).

    explicit            XclObjAnchor();

    /** Calculates a rectangle from the contained coordinates. */
    Rectangle           GetRect( const XclRoot& rRoot, SCTAB nScTab, MapUnit eMapUnit ) const;
    /** Initializes the anchor coordinates for a sheet. */
    void                SetRect( const XclRoot& rRoot, SCTAB nScTab, const Rectangle& rRect, MapUnit eMapUnit );

    /** Initializes the anchor coordinates for an embedded draw page. */
    void                SetRect( const Size& rPageSize, sal_Int32 nScaleX, sal_Int32 nScaleY,
                            const Rectangle& rRect, MapUnit eMapUnit, bool bDffAnchor );
};

inline SvStream& operator>>( SvStream& rStrm, XclObjAnchor& rAnchor )
{
    sal_uInt16 tmpFirstRow, tmpTY, tmpLastRow, tmpBY;

    rStrm
       .ReadUInt16( rAnchor.maFirst.mnCol ).ReadUInt16( rAnchor.mnLX )
       .ReadUInt16( tmpFirstRow ).ReadUInt16( tmpTY )
       .ReadUInt16( rAnchor.maLast.mnCol ).ReadUInt16( rAnchor.mnRX )
       .ReadUInt16( tmpLastRow ).ReadUInt16( tmpBY );

    rAnchor.maFirst.mnRow = static_cast<sal_uInt32> (tmpFirstRow);
    rAnchor.mnTY = static_cast<sal_uInt32> (tmpTY);
    rAnchor.maLast.mnRow = static_cast<sal_uInt32> (tmpLastRow);
    rAnchor.mnBY = static_cast<sal_uInt32> (tmpBY);

    return rStrm;
}

inline XclImpStream& operator>>( XclImpStream& rStrm, XclObjAnchor& rAnchor )
{
    sal_uInt16 tmpFirstRow, tmpTY, tmpLastRow, tmpBY;

    rAnchor.maFirst.mnCol = rStrm.ReaduInt16();
    rAnchor.mnLX = rStrm.ReaduInt16();
    tmpFirstRow = rStrm.ReaduInt16();
    tmpTY = rStrm.ReaduInt16();
    rAnchor.maLast.mnCol = rStrm.ReaduInt16();
    rAnchor.mnRX = rStrm.ReaduInt16();
    tmpLastRow = rStrm.ReaduInt16();
    tmpBY = rStrm.ReaduInt16();

    rAnchor.maFirst.mnRow = static_cast<sal_uInt32> (tmpFirstRow);
    rAnchor.mnTY = static_cast<sal_uInt32> (tmpTY);
    rAnchor.maLast.mnRow = static_cast<sal_uInt32> (tmpLastRow);
    rAnchor.mnBY = static_cast<sal_uInt32> (tmpBY);

    return rStrm;
}

inline SvStream& WriteXclObjAnchor( SvStream& rStrm, const XclObjAnchor& rAnchor )
{
    return rStrm
          .WriteUInt16( rAnchor.maFirst.mnCol ).WriteUInt16( rAnchor.mnLX )
          .WriteUInt16( rAnchor.maFirst.mnRow ).WriteUInt16( rAnchor.mnTY )
          .WriteUInt16( rAnchor.maLast.mnCol ).WriteUInt16( rAnchor.mnRX )
          .WriteUInt16( rAnchor.maLast.mnRow).WriteUInt16(rAnchor.mnBY);
}

struct XclObjLineData
{
    sal_uInt8           mnColorIdx;
    sal_uInt8           mnStyle;
    sal_uInt8           mnWidth;
    sal_uInt8           mnAuto;

    explicit            XclObjLineData();

    inline bool         IsAuto() const { return ::get_flag( mnAuto, EXC_OBJ_LINE_AUTO ); }
    inline bool         IsVisible() const { return IsAuto() || (mnStyle != EXC_OBJ_LINE_NONE); }
};

XclImpStream& operator>>( XclImpStream& rStrm, XclObjLineData& rLineData );

struct XclObjFillData
{
    sal_uInt8           mnBackColorIdx;
    sal_uInt8           mnPattColorIdx;
    sal_uInt8           mnPattern;
    sal_uInt8           mnAuto;

    explicit            XclObjFillData();

    inline bool         IsAuto() const { return ::get_flag( mnAuto, EXC_OBJ_FILL_AUTO ); }
    inline bool         IsFilled() const { return IsAuto() || (mnPattern != EXC_PATT_NONE); }
};

XclImpStream& operator>>( XclImpStream& rStrm, XclObjFillData& rFillData );

struct XclObjTextData
{
    sal_uInt16          mnTextLen;
    sal_uInt16          mnFormatSize;
    sal_uInt16          mnLinkSize;
    sal_uInt16          mnDefFontIdx;
    sal_uInt16          mnFlags;
    sal_uInt16          mnOrient;
    sal_uInt16          mnButtonFlags;
    sal_uInt16          mnShortcut;
    sal_uInt16          mnShortcutEA;

    explicit            XclObjTextData();

    /** Reads text data from a BIFF3/BIFF4 OBJ record. */
    void                ReadObj3( XclImpStream& rStrm );
    /** Reads text data from a BIFF5 OBJ record. */
    void                ReadObj5( XclImpStream& rStrm );
    /** Reads text data from a BIFF8 TXO record. */
    void                ReadTxo8( XclImpStream& rStrm );

    inline sal_uInt8    GetHorAlign() const { return ::extract_value< sal_uInt8 >( mnFlags, 1, 3 ); }
    inline sal_uInt8    GetVerAlign() const { return ::extract_value< sal_uInt8 >( mnFlags, 4, 3 ); }
};

enum XclTbxEventType
{
    EXC_TBX_EVENT_ACTION,       /// XActionListener.actionPerformed
    EXC_TBX_EVENT_MOUSE,        /// XMouseListener.mouseReleased
    EXC_TBX_EVENT_TEXT,         /// XTextListener.textChanged
    EXC_TBX_EVENT_VALUE,        /// XAdjustmentListener.adjustmentValueChanged
    EXC_TBX_EVENT_CHANGE        /// XChangeListener.changed
};

/** Provides static helper functions for form controls. */
class XclControlHelper
{
public:
    /** Returns the API control model from the passed API shape object. */
    static css::uno::Reference< css::awt::XControlModel >
                        GetControlModel( css::uno::Reference< css::drawing::XShape > xShape );

    /** Fills the macro descriptor according to the passed macro name. */
    static bool         FillMacroDescriptor(
                            css::script::ScriptEventDescriptor& rDescriptor,
                            XclTbxEventType eEventType,
                            const OUString& rXclMacroName,
                            SfxObjectShell* pDocShell = nullptr );
    /** Tries to extract an Excel macro name from the passed macro descriptor. */
    static OUString     ExtractFromMacroDescriptor(
                            const css::script::ScriptEventDescriptor& rDescriptor,
                            XclTbxEventType eEventType, SfxObjectShell* pShell = nullptr );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
