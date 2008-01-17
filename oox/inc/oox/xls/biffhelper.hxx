/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: biffhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:48 $
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

#ifndef OOX_XLS_BIFFHELPER_HXX
#define OOX_XLS_BIFFHELPER_HXX

#include "oox/helper/helper.hxx"

namespace oox {
namespace xls {

class BiffInputStream;
class BiffOutputStream;

// ============================================================================

/** An enumeration for all binary Excel file format types (BIFF types). */
enum BiffType
{
    BIFF2 = 0,                  /// MS Excel 2.1.
    BIFF3,                      /// MS Excel 3.0.
    BIFF4,                      /// MS Excel 4.0.
    BIFF5,                      /// MS Excel 5.0, MS Excel 7.0 (95).
    BIFF8,                      /// MS Excel 8.0 (97), 9.0 (2000), 10.0 (XP), 11.0 (2003).
    BIFF_UNKNOWN                /// Unknown BIFF version.
};

const sal_uInt16 BIFF2_MAXRECSIZE           = 2080;
const sal_uInt16 BIFF8_MAXRECSIZE           = 8224;

// record identifiers ---------------------------------------------------------

const sal_uInt16 BIFF2_ID_ARRAY             = 0x0021;
const sal_uInt16 BIFF3_ID_ARRAY             = 0x0221;
const sal_uInt16 BIFF2_ID_BLANK             = 0x0001;
const sal_uInt16 BIFF3_ID_BLANK             = 0x0201;
const sal_uInt16 BIFF2_ID_BOF               = 0x0009;
const sal_uInt16 BIFF3_ID_BOF               = 0x0209;
const sal_uInt16 BIFF4_ID_BOF               = 0x0409;
const sal_uInt16 BIFF5_ID_BOF               = 0x0809;
const sal_uInt16 BIFF_ID_BOOKBOOL           = 0x00DA;
const sal_uInt16 BIFF2_ID_BOOLERR           = 0x0005;
const sal_uInt16 BIFF3_ID_BOOLERR           = 0x0205;
const sal_uInt16 BIFF_ID_BOTTOMMARGIN       = 0x0029;
const sal_uInt16 BIFF_ID_CALCCOUNT          = 0x000C;
const sal_uInt16 BIFF_ID_CALCMODE           = 0x000D;
const sal_uInt16 BIFF_ID_CH3DDATAFORMAT     = 0x105F;
const sal_uInt16 BIFF_ID_CHAREA             = 0x101A;
const sal_uInt16 BIFF_ID_CHAREAFORMAT       = 0x100A;
const sal_uInt16 BIFF_ID_CHATTACHEDLABEL    = 0x100C;
const sal_uInt16 BIFF_ID_CHAXESSET          = 0x1041;
const sal_uInt16 BIFF_ID_CHAXIS             = 0x101D;
const sal_uInt16 BIFF_ID_CHAXISLINE         = 0x1021;
const sal_uInt16 BIFF_ID_CHBAR              = 0x1017;
const sal_uInt16 BIFF_ID_CHBEGIN            = 0x1033;
const sal_uInt16 BIFF_ID_CHCHART            = 0x1002;
const sal_uInt16 BIFF_ID_CHCHART3D          = 0x103A;
const sal_uInt16 BIFF_ID_CHCHARTLINE        = 0x101C;
const sal_uInt16 BIFF_ID_CHDATAFORMAT       = 0x1006;
const sal_uInt16 BIFF_ID_CHDEFAULTTEXT      = 0x1024;
const sal_uInt16 BIFF_ID_CHDROPBAR          = 0x103D;
const sal_uInt16 BIFF_ID_CHEND              = 0x1034;
const sal_uInt16 BIFF_ID_CHESCHERFORMAT     = 0x1066;
const sal_uInt16 BIFF_ID_CHFONT             = 0x1026;
const sal_uInt16 BIFF_ID_CHFORMAT           = 0x104E;
const sal_uInt16 BIFF_ID_CHFORMATRUNS       = 0x1050;
const sal_uInt16 BIFF_ID_CHFRAME            = 0x1032;
const sal_uInt16 BIFF_ID_CHFRAMEPOS         = 0x104F;
const sal_uInt16 BIFF_ID_CHLABELRANGE       = 0x1020;
const sal_uInt16 BIFF_ID_CHLABELRANGE2      = 0x1062;
const sal_uInt16 BIFF_ID_CHLEGEND           = 0x1015;
const sal_uInt16 BIFF_ID_CHLINE             = 0x1018;
const sal_uInt16 BIFF_ID_CHLINEFORMAT       = 0x1007;
const sal_uInt16 BIFF_ID_CHMARKERFORMAT     = 0x1009;
const sal_uInt16 BIFF_ID_CHOBJECTLINK       = 0x1027;
const sal_uInt16 BIFF_ID_CHPICFORMAT        = 0x103C;
const sal_uInt16 BIFF_ID_CHPIE              = 0x1019;
const sal_uInt16 BIFF_ID_CHPIEEXT           = 0x1061;
const sal_uInt16 BIFF_ID_CHPIEFORMAT        = 0x100B;
const sal_uInt16 BIFF5_ID_CHPIVOTREF        = 0x1048;
const sal_uInt16 BIFF8_ID_CHPIVOTREF        = 0x0858;
const sal_uInt16 BIFF_ID_CHPLOTFRAME        = 0x1035;
const sal_uInt16 BIFF_ID_CHPLOTGROWTH       = 0x1064;
const sal_uInt16 BIFF_ID_CHPROPERTIES       = 0x1044;
const sal_uInt16 BIFF_ID_CHRADARLINE        = 0x103E;
const sal_uInt16 BIFF_ID_CHRADARAREA        = 0x1040;
const sal_uInt16 BIFF_ID_CHSCATTER          = 0x101B;
const sal_uInt16 BIFF_ID_CHSERERRORBAR      = 0x105B;
const sal_uInt16 BIFF_ID_CHSERGROUP         = 0x1045;
const sal_uInt16 BIFF_ID_CHSERIES           = 0x1003;
const sal_uInt16 BIFF_ID_CHSERIESFORMAT     = 0x105D;
const sal_uInt16 BIFF_ID_CHSERPARENT        = 0x104A;
const sal_uInt16 BIFF_ID_CHSERTRENDLINE     = 0x104B;
const sal_uInt16 BIFF_ID_CHSOURCELINK       = 0x1051;
const sal_uInt16 BIFF_ID_CHSTRING           = 0x100D;
const sal_uInt16 BIFF_ID_CHSURFACE          = 0x103F;
const sal_uInt16 BIFF_ID_CHTEXT             = 0x1025;
const sal_uInt16 BIFF_ID_CHTICK             = 0x101E;
const sal_uInt16 BIFF_ID_CHTYPEGROUP        = 0x1014;
const sal_uInt16 BIFF_ID_CHUNITPROPERTIES   = 0x0857;
const sal_uInt16 BIFF_ID_CHVALUERANGE       = 0x101F;
const sal_uInt16 BIFF_ID_CHWRAPPEDRECORD    = 0x0851;
const sal_uInt16 BIFF_ID_CFHEADER           = 0x01B0;
const sal_uInt16 BIFF_ID_CFRULE             = 0x01B1;
const sal_uInt16 BIFF_ID_CODENAME           = 0x01BA;
const sal_uInt16 BIFF_ID_CODEPAGE           = 0x0042;
const sal_uInt16 BIFF_ID_COLINFO            = 0x007D;
const sal_uInt16 BIFF_ID_COLUMNDEFAULT      = 0x0020;
const sal_uInt16 BIFF_ID_COLWIDTH           = 0x0024;
const sal_uInt16 BIFF_ID_CONT               = 0x003C;
const sal_uInt16 BIFF_ID_CRN                = 0x005A;
const sal_uInt16 BIFF2_ID_DATATABLE         = 0x0036;
const sal_uInt16 BIFF3_ID_DATATABLE         = 0x0236;
const sal_uInt16 BIFF2_ID_DATATABLE2        = 0x0037;
const sal_uInt16 BIFF_ID_DATAVALIDATION     = 0x01BE;
const sal_uInt16 BIFF_ID_DATAVALIDATIONS    = 0x01B2;
const sal_uInt16 BIFF_ID_DATEMODE           = 0x0022;
const sal_uInt16 BIFF_ID_DCONNAME           = 0x0052;
const sal_uInt16 BIFF_ID_DCONREF            = 0x0051;
const sal_uInt16 BIFF_ID_DEFCOLWIDTH        = 0x0055;
const sal_uInt16 BIFF2_ID_DEFINEDNAME       = 0x0018;
const sal_uInt16 BIFF3_ID_DEFINEDNAME       = 0x0218;
const sal_uInt16 BIFF5_ID_DEFINEDNAME       = 0x0018;
const sal_uInt16 BIFF2_ID_DEFROWHEIGHT      = 0x0025;
const sal_uInt16 BIFF3_ID_DEFROWHEIGHT      = 0x0225;
const sal_uInt16 BIFF_ID_DELTA              = 0x0010;
const sal_uInt16 BIFF2_ID_DIMENSION         = 0x0000;
const sal_uInt16 BIFF3_ID_DIMENSION         = 0x0200;
const sal_uInt16 BIFF_ID_EOF                = 0x000A;
const sal_uInt16 BIFF_ID_EXTERNALBOOK       = 0x01AE;
const sal_uInt16 BIFF2_ID_EXTERNALNAME      = 0x0023;
const sal_uInt16 BIFF3_ID_EXTERNALNAME      = 0x0223;
const sal_uInt16 BIFF5_ID_EXTERNALNAME      = 0x0023;
const sal_uInt16 BIFF_ID_EXTERNSHEET        = 0x0017;
const sal_uInt16 BIFF_ID_EXTSST             = 0x00FF;
const sal_uInt16 BIFF_ID_FILEPASS           = 0x002F;
const sal_uInt16 BIFF2_ID_FONT              = 0x0031;
const sal_uInt16 BIFF3_ID_FONT              = 0x0231;
const sal_uInt16 BIFF5_ID_FONT              = 0x0031;
const sal_uInt16 BIFF_ID_FONTCOLOR          = 0x0045;
const sal_uInt16 BIFF_ID_FOOTER             = 0x0015;
const sal_uInt16 BIFF2_ID_FORMAT            = 0x001E;
const sal_uInt16 BIFF4_ID_FORMAT            = 0x041E;
const sal_uInt16 BIFF2_ID_FORMULA           = 0x0006;
const sal_uInt16 BIFF3_ID_FORMULA           = 0x0206;
const sal_uInt16 BIFF4_ID_FORMULA           = 0x0406;
const sal_uInt16 BIFF5_ID_FORMULA           = 0x0006;
const sal_uInt16 BIFF_ID_HCENTER            = 0x0083;
const sal_uInt16 BIFF_ID_HEADER             = 0x0014;
const sal_uInt16 BIFF_ID_HIDEOBJ            = 0x008D;
const sal_uInt16 BIFF_ID_HORPAGEBREAKS      = 0x001B;
const sal_uInt16 BIFF_ID_HYPERLINK          = 0x01B8;
const sal_uInt16 BIFF2_ID_INTEGER           = 0x0002;
const sal_uInt16 BIFF_ID_ITERATION          = 0x0011;
const sal_uInt16 BIFF_ID_IXFE               = 0x0044;
const sal_uInt16 BIFF2_ID_LABEL             = 0x0004;
const sal_uInt16 BIFF3_ID_LABEL             = 0x0204;
const sal_uInt16 BIFF_ID_LABELRANGES        = 0x015F;
const sal_uInt16 BIFF_ID_LABELSST           = 0x00FD;
const sal_uInt16 BIFF_ID_LEFTMARGIN         = 0x0026;
const sal_uInt16 BIFF_ID_MERGEDCELLS        = 0x00E5;
const sal_uInt16 BIFF_ID_MSODRAWING         = 0x00EC;
const sal_uInt16 BIFF_ID_MSODRAWINGGROUP    = 0x00EB;
const sal_uInt16 BIFF_ID_MSODRAWINGSEL      = 0x00ED;
const sal_uInt16 BIFF_ID_MULTBLANK          = 0x00BE;
const sal_uInt16 BIFF_ID_MULTRK             = 0x00BD;
const sal_uInt16 BIFF_ID_NOTE               = 0x001C;
const sal_uInt16 BIFF2_ID_NUMBER            = 0x0003;
const sal_uInt16 BIFF3_ID_NUMBER            = 0x0203;
const sal_uInt16 BIFF_ID_OBJ                = 0x005D;
const sal_uInt16 BIFF_ID_OBJECTPROTECT      = 0x0063;
const sal_uInt16 BIFF_ID_PALETTE            = 0x0092;
const sal_uInt16 BIFF_ID_PANE               = 0x0041;
const sal_uInt16 BIFF_ID_PASSWORD           = 0x0013;
const sal_uInt16 BIFF_ID_PHONETICPR         = 0x00EF;
const sal_uInt16 BIFF_ID_PRECISION          = 0x000E;
const sal_uInt16 BIFF_ID_PRINTGRIDLINES     = 0x002B;
const sal_uInt16 BIFF_ID_PRINTHEADERS       = 0x002A;
const sal_uInt16 BIFF_ID_PROJEXTSHEET       = 0x00A3;
const sal_uInt16 BIFF_ID_PROTECT            = 0x0012;
const sal_uInt16 BIFF_ID_REFMODE            = 0x000F;
const sal_uInt16 BIFF_ID_RIGHTMARGIN        = 0x0027;
const sal_uInt16 BIFF_ID_RK                 = 0x027E;
const sal_uInt16 BIFF2_ID_ROW               = 0x0008;
const sal_uInt16 BIFF3_ID_ROW               = 0x0208;
const sal_uInt16 BIFF_ID_RSTRING            = 0x00D6;
const sal_uInt16 BIFF_ID_SAVERECALC         = 0x005F;
const sal_uInt16 BIFF_ID_SCL                = 0x00A0;
const sal_uInt16 BIFF_ID_SCENPROTECT        = 0x00DD;
const sal_uInt16 BIFF_ID_SCREENTIP          = 0x0800;
const sal_uInt16 BIFF_ID_SELECTION          = 0x001D;
const sal_uInt16 BIFF_ID_PAGESETUP          = 0x00A1;
const sal_uInt16 BIFF_ID_SHAREDFMLA         = 0x04BC;
const sal_uInt16 BIFF_ID_SHEET              = 0x0085;
const sal_uInt16 BIFF_ID_SHEETHEADER        = 0x008F;
const sal_uInt16 BIFF_ID_SHEETPR            = 0x0081;
const sal_uInt16 BIFF_ID_SHEETPROTECTION    = 0x0867;
const sal_uInt16 BIFF_ID_SST                = 0x00FC;
const sal_uInt16 BIFF_ID_STANDARDWIDTH      = 0x0099;
const sal_uInt16 BIFF_ID_SXDATETIME         = 0x00CE;
const sal_uInt16 BIFF_ID_SXDB               = 0x00C6;
const sal_uInt16 BIFF_ID_SXDI               = 0x00C5;
const sal_uInt16 BIFF_ID_SXEXT              = 0x00DC;
const sal_uInt16 BIFF_ID_SXFIELD            = 0x00C7;
const sal_uInt16 BIFF_ID_SXIVD              = 0x00B4;
const sal_uInt16 BIFF_ID_SXLI               = 0x00B5;
const sal_uInt16 BIFF_ID_SXSTRING           = 0x00CD;
const sal_uInt16 BIFF_ID_SXVD               = 0x00B1;
const sal_uInt16 BIFF_ID_SXVDEX             = 0x0100;
const sal_uInt16 BIFF_ID_SXVI               = 0x00B2;
const sal_uInt16 BIFF_ID_SXVIEW             = 0x00B0;
const sal_uInt16 BIFF2_ID_STRING            = 0x0007;
const sal_uInt16 BIFF3_ID_STRING            = 0x0207;
const sal_uInt16 BIFF_ID_STYLE              = 0x0293;
const sal_uInt16 BIFF_ID_TOPMARGIN          = 0x0028;
const sal_uInt16 BIFF_ID_TXO                = 0x01B6;
const sal_uInt16 BIFF_ID_UNCALCED           = 0x005E;
const sal_uInt16 BIFF_ID_USESELFS           = 0x0160;
const sal_uInt16 BIFF_ID_VCENTER            = 0x0084;
const sal_uInt16 BIFF_ID_VERPAGEBREAKS      = 0x001A;
const sal_uInt16 BIFF_ID_WINDOW1            = 0x003D;
const sal_uInt16 BIFF2_ID_WINDOW2           = 0x003E;
const sal_uInt16 BIFF3_ID_WINDOW2           = 0x023E;
const sal_uInt16 BIFF_ID_XCT                = 0x0059;
const sal_uInt16 BIFF2_ID_XF                = 0x0043;
const sal_uInt16 BIFF3_ID_XF                = 0x0243;
const sal_uInt16 BIFF4_ID_XF                = 0x0443;
const sal_uInt16 BIFF5_ID_XF                = 0x00E0;

const sal_uInt16 BIFF_ID_UNKNOWN            = SAL_MAX_UINT16;

const sal_uInt16 BIFF_ID_OBJEND             = 0x0000;   /// End of OBJ.
const sal_uInt16 BIFF_ID_OBJMACRO           = 0x0004;   /// Macro link.
const sal_uInt16 BIFF_ID_OBJGMO             = 0x0006;   /// Group marker.
const sal_uInt16 BIFF_ID_OBJCF              = 0x0007;   /// Clipboard format.
const sal_uInt16 BIFF_ID_OBJPIOGRBIT        = 0x0008;   /// Option flags.
const sal_uInt16 BIFF_ID_OBJPICTFMLA        = 0x0009;   /// OLE link formula.
const sal_uInt16 BIFF_ID_OBJCBLS            = 0x000A;   /// Check box/radio button data.
const sal_uInt16 BIFF_ID_OBJSBS             = 0x000C;   /// Scroll bar data.
const sal_uInt16 BIFF_ID_OBJSBSFMLA         = 0x000E;   /// Scroll bar/list box/combo box cell link.
const sal_uInt16 BIFF_ID_OBJGBODATA         = 0x000F;   /// Group box data.
const sal_uInt16 BIFF_ID_OBJLBSDATA         = 0x0013;   /// List box/combo box data.
const sal_uInt16 BIFF_ID_OBJCBLSFMLA        = 0x0014;   /// Check box/radio button cell link.
const sal_uInt16 BIFF_ID_OBJCMO             = 0x0015;   /// Common object settings.

// record constants -----------------------------------------------------------

const sal_uInt16 BIFF_BOF_BIFF2             = 0x0200;
const sal_uInt16 BIFF_BOF_BIFF3             = 0x0300;
const sal_uInt16 BIFF_BOF_BIFF4             = 0x0400;
const sal_uInt16 BIFF_BOF_BIFF5             = 0x0500;
const sal_uInt16 BIFF_BOF_BIFF8             = 0x0600;

const sal_uInt8 BIFF_ERR_NULL               = 0x00;
const sal_uInt8 BIFF_ERR_DIV0               = 0x07;
const sal_uInt8 BIFF_ERR_VALUE              = 0x0F;
const sal_uInt8 BIFF_ERR_REF                = 0x17;
const sal_uInt8 BIFF_ERR_NAME               = 0x1D;
const sal_uInt8 BIFF_ERR_NUM                = 0x24;
const sal_uInt8 BIFF_ERR_NA                 = 0x2A;

const sal_uInt8 BIFF_DATATYPE_EMPTY         = 0;
const sal_uInt8 BIFF_DATATYPE_DOUBLE        = 1;
const sal_uInt8 BIFF_DATATYPE_STRING        = 2;
const sal_uInt8 BIFF_DATATYPE_BOOL          = 4;
const sal_uInt8 BIFF_DATATYPE_ERROR         = 16;

// unicode strings ------------------------------------------------------------

const sal_uInt8 BIFF_STRF_16BIT             = 0x01;
const sal_uInt8 BIFF_STRF_PHONETIC          = 0x04;
const sal_uInt8 BIFF_STRF_RICH              = 0x08;
const sal_uInt8 BIFF_STRF_UNKNOWN           = 0xF2;

/** Flags used to specify import/export mode of strings. */
typedef sal_Int32 BiffStringFlags;

const BiffStringFlags BIFF_STR_DEFAULT      = 0x0000;   /// Default string settings.
const BiffStringFlags BIFF_STR_FORCEUNICODE = 0x0001;   /// Always use UCS-2 characters (default: try to compress). BIFF8 export only.
const BiffStringFlags BIFF_STR_8BITLENGTH   = 0x0002;   /// 8-bit string length field (default: 16-bit).
const BiffStringFlags BIFF_STR_SMARTFLAGS   = 0x0004;   /// Omit flags on empty string (default: read/write always). BIFF8 only.
const BiffStringFlags BIFF_STR_KEEPFONTS    = 0x0008;   /// Keep old fonts when reading unformatted string (default: clear fonts). Import only.
const BiffStringFlags BIFF_STR_EXTRAFONTS   = 0x0010;   /// Read trailing rich-string font array (default: nothing). BIFF2-BIFF5 import only.

// GUID =======================================================================

/** This struct stores a GUID (class ID) and supports reading, writing and comparison.
 */
struct BiffGuid
{
    sal_uInt8           mpnData[ 16 ];      /// Stores the GUID, always in little-endian.

    explicit            BiffGuid();
    explicit            BiffGuid(
                            sal_uInt32 nData1,
                            sal_uInt16 nData2, sal_uInt16 nData3,
                            sal_uInt8 nData41, sal_uInt8 nData42,
                            sal_uInt8 nData43, sal_uInt8 nData44,
                            sal_uInt8 nData45, sal_uInt8 nData46,
                            sal_uInt8 nData47, sal_uInt8 nData48 );
};

bool operator==( const BiffGuid& rGuid1, const BiffGuid& rGuid2 );
bool operator<( const BiffGuid& rGuid1, const BiffGuid& rGuid2 );

BiffInputStream& operator>>( BiffInputStream& rStrm, BiffGuid& rGuid );
BiffOutputStream& operator<<( BiffOutputStream& rStrm, const BiffGuid& rGuid );

// ============================================================================

/** Static helper functions for BIFF filters. */
class BiffHelper
{
public:
    static const BiffGuid maGuidStdHlink;       /// GUID of StdHlink (HLINK record).
    static const BiffGuid maGuidUrlMoniker;     /// GUID of URL moniker (HLINK record).
    static const BiffGuid maGuidFileMoniker;    /// GUID of file moniker (HLINK record).

    // conversion -------------------------------------------------------------

    /** Converts the passed packed number to a double. */
    static double       calcDoubleFromRk( sal_Int32 nRkValue );
    /** Converts the passed double to a packed number, returns true on success. */
    static bool         calcRkFromDouble( sal_Int32& ornRkValue, double fValue );

    /** Converts the passed BIFF error to a double containing the respective Calc error code. */
    static double       calcDoubleFromError( sal_uInt8 nErrorCode );

    /** Returns a text encoding from an Windows code page.
        @return  The corresponding text encoding or RTL_TEXTENCODING_DONTKNOW. */
    static rtl_TextEncoding calcTextEncodingFromCodePage( sal_uInt16 nCodePage );
    /** Returns a Windows code page from a text encoding. */
    static sal_uInt16   calcCodePageFromTextEncoding( rtl_TextEncoding eTextEnc );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

