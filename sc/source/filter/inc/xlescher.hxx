/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlescher.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:03:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XLESCHER_HXX
#define SC_XLESCHER_HXX

#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif
#ifndef SC_XLADDRESS_HXX
#include "xladdress.hxx"
#endif

// Constants and Enumerations =================================================

// misc -----------------------------------------------------------------------

const long EXC_ESCHER_AUTOMARGIN            = 20000;    /// Automatic text margin.

// (0x001C) NOTE --------------------------------------------------------------

const sal_uInt16 EXC_ID_NOTE                = 0x001C;
const sal_uInt16 EXC_NOTE_VISIBLE           = 0x0002;
const sal_uInt16 EXC_NOTE5_MAXLEN           = 2048;

// (0x005D) OBJ ---------------------------------------------------------------

const sal_uInt16 EXC_ID_OBJ                 = 0x005D;

const sal_uInt16 EXC_OBJ_INVALID_ID         = 0x0000;

// sub records
const sal_uInt16 EXC_ID_OBJ_FTEND           = 0x0000;   /// End of OBJ.
const sal_uInt16 EXC_ID_OBJ_FTMACRO         = 0x0004;   /// Macro link.
const sal_uInt16 EXC_ID_OBJ_FTGMO           = 0x0006;   /// Group marker.
const sal_uInt16 EXC_ID_OBJ_FTCF            = 0x0007;   /// Clipboard format.
const sal_uInt16 EXC_ID_OBJ_FTPIOGRBIT      = 0x0008;   /// Option flags.
const sal_uInt16 EXC_ID_OBJ_FTPICTFMLA      = 0x0009;   /// OLE link formula.
const sal_uInt16 EXC_ID_OBJ_FTCBLS          = 0x000A;   /// Check box/radio button data.
const sal_uInt16 EXC_ID_OBJ_FTSBS           = 0x000C;   /// Scroll bar data.
const sal_uInt16 EXC_ID_OBJ_FTSBSFMLA       = 0x000E;   /// Scroll bar/list box/combo box cell link.
const sal_uInt16 EXC_ID_OBJ_FTGBODATA       = 0x000F;   /// Group box data.
const sal_uInt16 EXC_ID_OBJ_FTLBSDATA       = 0x0013;   /// List box/combo box data.
const sal_uInt16 EXC_ID_OBJ_FTCBLSFMLA      = 0x0014;   /// Check box/radio button cell link.
const sal_uInt16 EXC_ID_OBJ_FTCMO           = 0x0015;   /// Common object settings.
const sal_uInt16 EXC_ID_OBJ_FTUNKNOWN       = 0xFFFF;   /// For internal use only.

// ftCmo: object types
const sal_uInt16 EXC_OBJ_CMO_GROUP          = 0x0000;
const sal_uInt16 EXC_OBJ_CMO_LINE           = 0x0001;
const sal_uInt16 EXC_OBJ_CMO_RECTANGLE      = 0x0002;
const sal_uInt16 EXC_OBJ_CMO_ELLIPSE        = 0x0003;
const sal_uInt16 EXC_OBJ_CMO_ARC            = 0x0004;
const sal_uInt16 EXC_OBJ_CMO_CHART          = 0x0005;
const sal_uInt16 EXC_OBJ_CMO_TEXT           = 0x0006;
const sal_uInt16 EXC_OBJ_CMO_BUTTON         = 0x0007;
const sal_uInt16 EXC_OBJ_CMO_PICTURE        = 0x0008;
const sal_uInt16 EXC_OBJ_CMO_POLYGON        = 0x0009;
const sal_uInt16 EXC_OBJ_CMO_CHECKBOX       = 0x000B;
const sal_uInt16 EXC_OBJ_CMO_OPTIONBUTTON   = 0x000C;
const sal_uInt16 EXC_OBJ_CMO_EDIT           = 0x000D;
const sal_uInt16 EXC_OBJ_CMO_LABEL          = 0x000E;
const sal_uInt16 EXC_OBJ_CMO_DIALOG         = 0x000F;
const sal_uInt16 EXC_OBJ_CMO_SPIN           = 0x0010;
const sal_uInt16 EXC_OBJ_CMO_SCROLLBAR      = 0x0011;
const sal_uInt16 EXC_OBJ_CMO_LISTBOX        = 0x0012;
const sal_uInt16 EXC_OBJ_CMO_GROUPBOX       = 0x0013;
const sal_uInt16 EXC_OBJ_CMO_COMBOBOX       = 0x0014;
const sal_uInt16 EXC_OBJ_CMO_NOTE           = 0x0019;
const sal_uInt16 EXC_OBJ_CMO_DRAWING        = 0x001E;
const sal_uInt16 EXC_OBJ_CMO_UNKNOWN        = 0xFFFF;   /// For internal use only.

// ftCmo: flags
const sal_uInt16 EXC_OBJ_CMO_PRINTABLE      = 0x0010;   /// Object printable

// ftPioGrbit: flags
const sal_uInt16 EXC_OBJ_PIO_MANUALSIZE     = 0x0001;
const sal_uInt16 EXC_OBJ_PIO_LINKED         = 0x0002;
const sal_uInt16 EXC_OBJ_PIO_SYMBOL         = 0x0008;
const sal_uInt16 EXC_OBJ_PIO_CONTROL        = 0x0010;   /// Form control.
const sal_uInt16 EXC_OBJ_PIO_CTLSSTREAM     = 0x0020;   /// Data in Ctls stream.
const sal_uInt16 EXC_OBJ_PIO_AUTOLOAD       = 0x0200;

// ftCbls: Check box/radio button data
const sal_uInt16 EXC_OBJ_CBLS_STATEMASK     = 0x0003;
const sal_uInt16 EXC_OBJ_CBLS_STATE_UNCHECK = 0x0000;
const sal_uInt16 EXC_OBJ_CBLS_STATE_CHECK   = 0x0001;
const sal_uInt16 EXC_OBJ_CBLS_STATE_TRI     = 0x0002;
const sal_uInt16 EXC_OBJ_CBLS_FLAT          = 0x0001;

// ftGboData: Group box data
const sal_uInt16 EXC_OBJ_GBO_FLAT           = 0x0001;

// ftLbsData: List box data
const sal_uInt16 EXC_OBJ_LBS_SELMASK        = 0x0030;   /// Mask for selection type.
const sal_uInt16 EXC_OBJ_LBS_SEL_SIMPLE     = 0x0000;   /// Simple selection.
const sal_uInt16 EXC_OBJ_LBS_SEL_MULTI      = 0x0010;   /// Multi selection.
const sal_uInt16 EXC_OBJ_LBS_SEL_EXT        = 0x0020;   /// Extended selection.
const sal_uInt16 EXC_OBJ_LBS_FLAT           = 0x0008;
const sal_uInt16 EXC_OBJ_LBS_COMBOMASK      = 0x0003;   /// Mask for combobox style.
const sal_uInt16 EXC_OBJ_LBS_COMBO_STD      = 0x0000;   /// Standard combo box.
const sal_uInt16 EXC_OBJ_LBS_COMBO_SIMPLE   = 0x0002;   /// Simple dropdown without field.
const sal_uInt16 EXC_OBJ_LBS_FILTERED       = 0x0008;   /// Drowdown style: filtered.

// ftSbs: Spin button/scrollbar data
const sal_uInt16 EXC_OBJ_SBS_HORIZONTAL     = 0x0001;
const sal_uInt16 EXC_OBJ_SBS_DEFAULTFLAGS   = 0x0001;
const sal_uInt16 EXC_OBJ_SBS_FLAT           = 0x0008;
const sal_Int16 EXC_OBJ_SBS_MINSCROLL       = 0;
const sal_Int16 EXC_OBJ_SBS_MAXSCROLL       = 30000;

/** Value binding mode for cells linked to form controls. */
enum XclCtrlBindMode
{
    xlBindContent,      /// Binds cell to content of control.
    xlBindPosition      /// Binds cell to position in control (e.g. listbox selection index).
};

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

const sal_uInt8 EXC_TXO_HOR_LEFT            = 0x01;
const sal_uInt8 EXC_TXO_HOR_CENTER          = 0x02;
const sal_uInt8 EXC_TXO_HOR_RIGHT           = 0x03;
const sal_uInt8 EXC_TXO_HOR_JUSTIFY         = 0x04;

const sal_uInt8 EXC_TXO_VER_TOP             = 0x01;
const sal_uInt8 EXC_TXO_VER_CENTER          = 0x02;
const sal_uInt8 EXC_TXO_VER_BOTTOM          = 0x03;
const sal_uInt8 EXC_TXO_VER_JUSTIFY         = 0x04;

const sal_uInt16 EXC_TXO_TEXTROT_NONE       = 0x0000;
const sal_uInt16 EXC_TXO_TEXTROT_STACKED    = 0x0001;      /// Stacked top to bottom.
const sal_uInt16 EXC_TXO_TEXTROT_90_CCW     = 0x0002;      /// 90 degr. counterclockwise.
const sal_uInt16 EXC_TXO_TEXTROT_90_CW      = 0x0003;      /// 90 degr. clockwise.

// Structs and classes ========================================================

// Escher client anchor -------------------------------------------------------

class Rectangle;
class ScDocument;
class SvStream;
class XclImpStream;
class XclExpStream;

/** Represents the position (anchor) of an Escher object in a Calc document. */
struct XclEscherAnchor
{
    XclRange            maXclRange; /// Cell range address.
    SCTAB               mnScTab;    /// Calc sheet index.
    sal_uInt16          mnLX;       /// X offset in left column (1/1024 of column width).
    sal_uInt16          mnTY;       /// Y offset in top row (1/256 of row height).
    sal_uInt16          mnRX;       /// X offset in right column (1/1024 of column width).
    sal_uInt16          mnBY;       /// Y offset in bottom row (1/256 of row height).

    explicit            XclEscherAnchor( SCTAB nScTab );

    /** Calculates a rectangle from the contained coordinates. */
    Rectangle           GetRect( ScDocument& rDoc, MapUnit eMapUnit ) const;
    /** Initializes the anchor coordinates from a rectangle. */
    void                SetRect( ScDocument& rDoc, const Rectangle& rRect, MapUnit eMapUnit );
};

SvStream& operator>>( SvStream& rStrm, XclEscherAnchor& rAnchor );
SvStream& operator<<( SvStream& rStrm, const XclEscherAnchor& rAnchor );

XclImpStream& operator>>( XclImpStream& rStrm, XclEscherAnchor& rAnchor );
XclExpStream& operator<<( XclExpStream& rStrm, const XclEscherAnchor& rAnchor );

// ----------------------------------------------------------------------------

/** Identifies an Escher object by sheet index and object identifier. */
struct XclObjId
{
    SCTAB               mnScTab;        /// Calc sheet index.
    sal_uInt16          mnObjId;        /// Excel object identifier.

    explicit            XclObjId();
    explicit            XclObjId( SCTAB nScTab, sal_uInt16 nObjId );
};

bool operator==( const XclObjId& rL, const XclObjId& rR );
bool operator<( const XclObjId& rL, const XclObjId& rR );

// ----------------------------------------------------------------------------

/** Contains data of a TXO record for text boxes. */
struct XclTxoData
{
    sal_uInt16          mnFlags;        /// Option flags and alignment.
    sal_uInt16          mnOrient;       /// Text orientation.
    sal_uInt16          mnTextLen;      /// Length of the string.
    sal_uInt16          mnFormatSize;   /// Size of the format run buffer (bytes).

    explicit            XclTxoData();

    sal_uInt8           GetXclHorAlignment() const;
    void                SetXclHorAlignment( sal_uInt8 nXclAlign );

    sal_uInt8           GetXclVerAlignment() const;
    void                SetXclVerAlignment( sal_uInt8 nXclAlign );
};

XclImpStream& operator>>( XclImpStream& rStrm, XclTxoData& rData );
XclExpStream& operator<<( XclExpStream& rStrm, const XclTxoData& rData );

// ----------------------------------------------------------------------------

/** Provides static helper functions for textbox (TBX) form controls. */
class XclTbxControlHelper
{
public:
    /** Returns the component service name for the passed control type. */
    static ::rtl::OUString GetServiceName( sal_uInt16 nCtrlType );
    /** Returns a default control name for the passed control type. */
    static ::rtl::OUString GetControlName( sal_uInt16 nCtrlType );

    /** Returns the listener type (interface name) for macro events for the passed control type. */
    static ::rtl::OUString GetListenerType( sal_uInt16 nCtrlType );
    /** Returns the event method (function name) for macro events for the passed control type. */
    static ::rtl::OUString GetEventMethod( sal_uInt16 nCtrlType );
    /** Returns the script type string needed for a script event descriptor. */
    static ::rtl::OUString GetScriptType();

    /** Returns the Calc macro name from an Excel macro name. */
    static ::rtl::OUString GetScMacroName( const String& rXclMacroName );
    /** Returns the Excel macro name from a Calc macro name. */
    static String       GetXclMacroName( const ::rtl::OUString& rScMacroName );
};

// ============================================================================

#endif

