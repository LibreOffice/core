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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_BIFFHELPER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_BIFFHELPER_HXX

#include <oox/helper/binarystreambase.hxx>

namespace oox { class SequenceInputStream; }

namespace oox {
namespace xls {

class BiffInputStream;

// BIFF12 record identifiers ==================================================

const sal_Int32 BIFF12_ID_ARRAY             = 0x01AA;
const sal_Int32 BIFF12_ID_AUTOFILTER        = 0x00A1;
const sal_Int32 BIFF12_ID_AUTOSORTSCOPE     = 0x01CB;
const sal_Int32 BIFF12_ID_BINARYINDEXBLOCK  = 0x002A;
const sal_Int32 BIFF12_ID_BINARYINDEXROWS   = 0x0028;
const sal_Int32 BIFF12_ID_BOOKVIEWS         = 0x0087;
const sal_Int32 BIFF12_ID_BORDER            = 0x002E;
const sal_Int32 BIFF12_ID_BORDERS           = 0x0265;
const sal_Int32 BIFF12_ID_BRK               = 0x018C;
const sal_Int32 BIFF12_ID_CALCPR            = 0x009D;
const sal_Int32 BIFF12_ID_CELL_BLANK        = 0x0001;
const sal_Int32 BIFF12_ID_CELL_BOOL         = 0x0004;
const sal_Int32 BIFF12_ID_CELL_DOUBLE       = 0x0005;
const sal_Int32 BIFF12_ID_CELL_ERROR        = 0x0003;
const sal_Int32 BIFF12_ID_CELL_RK           = 0x0002;
const sal_Int32 BIFF12_ID_CELL_RSTRING      = 0x003E;
const sal_Int32 BIFF12_ID_CELL_SI           = 0x0007;
const sal_Int32 BIFF12_ID_CELL_STRING       = 0x0006;
const sal_Int32 BIFF12_ID_CELLSTYLE         = 0x0030;
const sal_Int32 BIFF12_ID_CELLSTYLES        = 0x026B;
const sal_Int32 BIFF12_ID_CELLSTYLEXFS      = 0x0272;
const sal_Int32 BIFF12_ID_CELLXFS           = 0x0269;
const sal_Int32 BIFF12_ID_CFCOLOR           = 0x0234;
const sal_Int32 BIFF12_ID_CFRULE            = 0x01CF;
const sal_Int32 BIFF12_ID_CHARTPAGESETUP    = 0x028C;
const sal_Int32 BIFF12_ID_CHARTPROTECTION   = 0x029D;
const sal_Int32 BIFF12_ID_CHARTSHEETPR      = 0x028B;
const sal_Int32 BIFF12_ID_CHARTSHEETVIEW    = 0x008D;
const sal_Int32 BIFF12_ID_CHARTSHEETVIEWS   = 0x008B;
const sal_Int32 BIFF12_ID_COL               = 0x003C;
const sal_Int32 BIFF12_ID_COLBREAKS         = 0x018A;
const sal_Int32 BIFF12_ID_COLOR             = 0x023C;
const sal_Int32 BIFF12_ID_COLORS            = 0x01D9;
const sal_Int32 BIFF12_ID_COLORSCALE        = 0x01D5;
const sal_Int32 BIFF12_ID_COLS              = 0x0186;
const sal_Int32 BIFF12_ID_COMMENT           = 0x027B;
const sal_Int32 BIFF12_ID_COMMENTAUTHOR     = 0x0278;
const sal_Int32 BIFF12_ID_COMMENTAUTHORS    = 0x0276;
const sal_Int32 BIFF12_ID_COMMENTLIST       = 0x0279;
const sal_Int32 BIFF12_ID_COMMENTS          = 0x0274;
const sal_Int32 BIFF12_ID_COMMENTTEXT       = 0x027D;
const sal_Int32 BIFF12_ID_CONDFORMATTING    = 0x01CD;
const sal_Int32 BIFF12_ID_CONNECTION        = 0x00C9;
const sal_Int32 BIFF12_ID_CONNECTIONS       = 0x01AD;
const sal_Int32 BIFF12_ID_CONTROL           = 0x0284;
const sal_Int32 BIFF12_ID_CONTROLS          = 0x0283;
const sal_Int32 BIFF12_ID_CUSTOMCHARTVIEW   = 0x028F;
const sal_Int32 BIFF12_ID_CUSTOMCHARTVIEWS  = 0x028D;
const sal_Int32 BIFF12_ID_CUSTOMFILTER      = 0x00AE;
const sal_Int32 BIFF12_ID_CUSTOMFILTERS     = 0x00AC;
const sal_Int32 BIFF12_ID_CUSTOMSHEETVIEW   = 0x01A7;
const sal_Int32 BIFF12_ID_CUSTOMSHEETVIEWS  = 0x01A6;
const sal_Int32 BIFF12_ID_CUSTOMWORKBOOKVIEW= 0x018D;
const sal_Int32 BIFF12_ID_DATABAR           = 0x01D3;
const sal_Int32 BIFF12_ID_DATATABLE         = 0x01AC;
const sal_Int32 BIFF12_ID_DATAVALIDATION    = 0x0040;
const sal_Int32 BIFF12_ID_DATAVALIDATIONS   = 0x023D;
const sal_Int32 BIFF12_ID_DDEITEMVALUES     = 0x0242;
const sal_Int32 BIFF12_ID_DDEITEM_BOOL      = 0x0248;
const sal_Int32 BIFF12_ID_DDEITEM_DOUBLE    = 0x0244;
const sal_Int32 BIFF12_ID_DDEITEM_ERROR     = 0x0245;
const sal_Int32 BIFF12_ID_DDEITEM_STRING    = 0x0246;
const sal_Int32 BIFF12_ID_DEFINEDNAME       = 0x0027;
const sal_Int32 BIFF12_ID_DIMENSION         = 0x0094;
const sal_Int32 BIFF12_ID_DISCRETEFILTER    = 0x00A7;
const sal_Int32 BIFF12_ID_DISCRETEFILTERS   = 0x00A5;
const sal_Int32 BIFF12_ID_DRAWING           = 0x0226;
const sal_Int32 BIFF12_ID_DXF               = 0x01FB;
const sal_Int32 BIFF12_ID_DXFS              = 0x01F9;
const sal_Int32 BIFF12_ID_EXTCELL_BLANK     = 0x016F;
const sal_Int32 BIFF12_ID_EXTCELL_BOOL      = 0x0171;
const sal_Int32 BIFF12_ID_EXTCELL_DOUBLE    = 0x0170;
const sal_Int32 BIFF12_ID_EXTCELL_ERROR     = 0x0172;
const sal_Int32 BIFF12_ID_EXTCELL_STRING    = 0x0173;
const sal_Int32 BIFF12_ID_EXTERNALADDIN     = 0x029B;
const sal_Int32 BIFF12_ID_EXTERNALBOOK      = 0x0168;
const sal_Int32 BIFF12_ID_EXTERNALNAME      = 0x0241;
const sal_Int32 BIFF12_ID_EXTERNALREF       = 0x0163;
const sal_Int32 BIFF12_ID_EXTERNALREFS      = 0x0161;
const sal_Int32 BIFF12_ID_EXTERNALSELF      = 0x0165;
const sal_Int32 BIFF12_ID_EXTERNALSAME      = 0x0166;
const sal_Int32 BIFF12_ID_EXTERNALSHEETS    = 0x016A;
const sal_Int32 BIFF12_ID_EXTROW            = 0x016E;
const sal_Int32 BIFF12_ID_EXTSHEETDATA      = 0x016B;
const sal_Int32 BIFF12_ID_EXTERNALNAMEFLAGS = 0x024A;
const sal_Int32 BIFF12_ID_EXTSHEETNAMES     = 0x0167;
const sal_Int32 BIFF12_ID_FILESHARING       = 0x0224;
const sal_Int32 BIFF12_ID_FILEVERSION       = 0x0080;
const sal_Int32 BIFF12_ID_FILL              = 0x002D;
const sal_Int32 BIFF12_ID_FILLS             = 0x025B;
const sal_Int32 BIFF12_ID_FILTERCOLUMN      = 0x00A3;
const sal_Int32 BIFF12_ID_FONT              = 0x002B;
const sal_Int32 BIFF12_ID_FONTS             = 0x0263;
const sal_Int32 BIFF12_ID_FORMULA_STRING    = 0x0008;
const sal_Int32 BIFF12_ID_FORMULA_DOUBLE    = 0x0009;
const sal_Int32 BIFF12_ID_FORMULA_BOOL      = 0x000A;
const sal_Int32 BIFF12_ID_FORMULA_ERROR     = 0x000B;
const sal_Int32 BIFF12_ID_FUNCTIONGROUP     = 0x0299;
const sal_Int32 BIFF12_ID_FUNCTIONGROUPS    = 0x0298;
const sal_Int32 BIFF12_ID_HEADERFOOTER      = 0x01DF;
const sal_Int32 BIFF12_ID_HYPERLINK         = 0x01EE;
const sal_Int32 BIFF12_ID_ICONSET           = 0x01D1;
const sal_Int32 BIFF12_ID_INDEXEDCOLORS     = 0x0235;
const sal_Int32 BIFF12_ID_INPUTCELLS        = 0x01F8;
const sal_Int32 BIFF12_ID_LEGACYDRAWING     = 0x0227;
const sal_Int32 BIFF12_ID_MERGECELL         = 0x00B0;
const sal_Int32 BIFF12_ID_MERGECELLS        = 0x00B1;
const sal_Int32 BIFF12_ID_MRUCOLORS         = 0x0239;
const sal_Int32 BIFF12_ID_MULTCELL_BLANK    = 0x000C;
const sal_Int32 BIFF12_ID_MULTCELL_BOOL     = 0x000F;
const sal_Int32 BIFF12_ID_MULTCELL_DOUBLE   = 0x0010;
const sal_Int32 BIFF12_ID_MULTCELL_ERROR    = 0x000E;
const sal_Int32 BIFF12_ID_MULTCELL_RK       = 0x000D;
const sal_Int32 BIFF12_ID_MULTCELL_RSTRING  = 0x003D;
const sal_Int32 BIFF12_ID_MULTCELL_SI       = 0x0012;
const sal_Int32 BIFF12_ID_MULTCELL_STRING   = 0x0011;
const sal_Int32 BIFF12_ID_NUMFMT            = 0x002C;
const sal_Int32 BIFF12_ID_NUMFMTS           = 0x0267;
const sal_Int32 BIFF12_ID_OLEOBJECT         = 0x027F;
const sal_Int32 BIFF12_ID_OLEOBJECTS        = 0x027E;
const sal_Int32 BIFF12_ID_OLESIZE           = 0x0225;
const sal_Int32 BIFF12_ID_PAGEMARGINS       = 0x01DC;
const sal_Int32 BIFF12_ID_PAGESETUP         = 0x01DE;
const sal_Int32 BIFF12_ID_PANE              = 0x0097;
const sal_Int32 BIFF12_ID_PCDEFINITION      = 0x00B3;
const sal_Int32 BIFF12_ID_PCDFDISCRETEPR    = 0x00E1;
const sal_Int32 BIFF12_ID_PCDFGROUPITEMS    = 0x00DD;
const sal_Int32 BIFF12_ID_PCDFIELD          = 0x00B7;
const sal_Int32 BIFF12_ID_PCDFIELDGROUP     = 0x00DB;
const sal_Int32 BIFF12_ID_PCDFIELDS         = 0x00B5;
const sal_Int32 BIFF12_ID_PCDFRANGEPR       = 0x00DF;
const sal_Int32 BIFF12_ID_PCDFSHAREDITEMS   = 0x00BD;
const sal_Int32 BIFF12_ID_PCDSHEETSOURCE    = 0x00BB;
const sal_Int32 BIFF12_ID_PCDSOURCE         = 0x00B9;
const sal_Int32 BIFF12_ID_PCITEM_ARRAY      = 0x00BF;
const sal_Int32 BIFF12_ID_PCITEM_BOOL       = 0x0016;
const sal_Int32 BIFF12_ID_PCITEM_DATE       = 0x0019;
const sal_Int32 BIFF12_ID_PCITEM_DOUBLE     = 0x0015;
const sal_Int32 BIFF12_ID_PCITEM_ERROR      = 0x0017;
const sal_Int32 BIFF12_ID_PCITEM_INDEX      = 0x001A;
const sal_Int32 BIFF12_ID_PCITEM_MISSING    = 0x0014;
const sal_Int32 BIFF12_ID_PCITEM_STRING     = 0x0018;
const sal_Int32 BIFF12_ID_PCITEMA_BOOL      = 0x001D;
const sal_Int32 BIFF12_ID_PCITEMA_DATE      = 0x0020;
const sal_Int32 BIFF12_ID_PCITEMA_DOUBLE    = 0x001C;
const sal_Int32 BIFF12_ID_PCITEMA_ERROR     = 0x001E;
const sal_Int32 BIFF12_ID_PCITEMA_MISSING   = 0x001B;
const sal_Int32 BIFF12_ID_PCITEMA_STRING    = 0x001F;
const sal_Int32 BIFF12_ID_PCRECORD          = 0x0021;
const sal_Int32 BIFF12_ID_PCRECORDDT        = 0x0022;
const sal_Int32 BIFF12_ID_PCRECORDS         = 0x00C1;
const sal_Int32 BIFF12_ID_PHONETICPR        = 0x0219;
const sal_Int32 BIFF12_ID_PICTURE           = 0x0232;
const sal_Int32 BIFF12_ID_PIVOTAREA         = 0x00F7;
const sal_Int32 BIFF12_ID_PIVOTCACHE        = 0x0182;
const sal_Int32 BIFF12_ID_PIVOTCACHES       = 0x0180;
const sal_Int32 BIFF12_ID_PRINTOPTIONS      = 0x01DD;
const sal_Int32 BIFF12_ID_PTCOLFIELDS       = 0x0137;
const sal_Int32 BIFF12_ID_PTDATAFIELD       = 0x0125;
const sal_Int32 BIFF12_ID_PTDATAFIELDS      = 0x0127;
const sal_Int32 BIFF12_ID_PTDEFINITION      = 0x0118;
const sal_Int32 BIFF12_ID_PTFIELD           = 0x011D;
const sal_Int32 BIFF12_ID_PTFIELDS          = 0x011F;
const sal_Int32 BIFF12_ID_PTFILTER          = 0x0259;
const sal_Int32 BIFF12_ID_PTFILTERS         = 0x0257;
const sal_Int32 BIFF12_ID_PTFITEM           = 0x011A;
const sal_Int32 BIFF12_ID_PTFITEMS          = 0x011B;
const sal_Int32 BIFF12_ID_PTLOCATION        = 0x013A;
const sal_Int32 BIFF12_ID_PTPAGEFIELD       = 0x0121;
const sal_Int32 BIFF12_ID_PTPAGEFIELDS      = 0x0123;
const sal_Int32 BIFF12_ID_PTREFERENCE       = 0x00FB;
const sal_Int32 BIFF12_ID_PTREFERENCEITEM   = 0x017E;
const sal_Int32 BIFF12_ID_PTREFERENCES      = 0x00F9;
const sal_Int32 BIFF12_ID_PTROWFIELDS       = 0x0135;
const sal_Int32 BIFF12_ID_QUERYTABLE        = 0x01BF;
const sal_Int32 BIFF12_ID_QUERYTABLEREFRESH = 0x01C1;
const sal_Int32 BIFF12_ID_RGBCOLOR          = 0x01DB;
const sal_Int32 BIFF12_ID_ROW               = 0x0000;
const sal_Int32 BIFF12_ID_ROWBREAKS         = 0x0188;
const sal_Int32 BIFF12_ID_SCENARIO          = 0x01F6;
const sal_Int32 BIFF12_ID_SCENARIOS         = 0x01F4;
const sal_Int32 BIFF12_ID_SELECTION         = 0x0098;
const sal_Int32 BIFF12_ID_SHAREDFMLA        = 0x01AB;
const sal_Int32 BIFF12_ID_SHEET             = 0x009C;
const sal_Int32 BIFF12_ID_SHEETDATA         = 0x0091;
const sal_Int32 BIFF12_ID_SHEETFORMATPR     = 0x01E5;
const sal_Int32 BIFF12_ID_SHEETPR           = 0x0093;
const sal_Int32 BIFF12_ID_SHEETPROTECTION   = 0x0217;
const sal_Int32 BIFF12_ID_SHEETS            = 0x008F;
const sal_Int32 BIFF12_ID_SHEETVIEW         = 0x0089;
const sal_Int32 BIFF12_ID_SHEETVIEWS        = 0x0085;
const sal_Int32 BIFF12_ID_SI                = 0x0013;
const sal_Int32 BIFF12_ID_SST               = 0x009F;
const sal_Int32 BIFF12_ID_STYLESHEET        = 0x0116;
const sal_Int32 BIFF12_ID_TABLE             = 0x0157;
const sal_Int32 BIFF12_ID_TABLEPART         = 0x0295;
const sal_Int32 BIFF12_ID_TABLEPARTS        = 0x0294;
const sal_Int32 BIFF12_ID_TABLESTYLEINFO    = 0x0201;
const sal_Int32 BIFF12_ID_TABLESTYLES       = 0x01FC;
const sal_Int32 BIFF12_ID_TOP10FILTER       = 0x00AA;
const sal_Int32 BIFF12_ID_VOLTYPE           = 0x0204;
const sal_Int32 BIFF12_ID_VOLTYPEMAIN       = 0x0206;
const sal_Int32 BIFF12_ID_VOLTYPES          = 0x0202;
const sal_Int32 BIFF12_ID_VOLTYPESTP        = 0x020A;
const sal_Int32 BIFF12_ID_VOLTYPETR         = 0x020B;
const sal_Int32 BIFF12_ID_WEBPR             = 0x0105;
const sal_Int32 BIFF12_ID_WEBPRTABLES       = 0x0107;
const sal_Int32 BIFF12_ID_WORKBOOK          = 0x0083;
const sal_Int32 BIFF12_ID_WORKBOOKPR        = 0x0099;
const sal_Int32 BIFF12_ID_WORKBOOKVIEW      = 0x009E;
const sal_Int32 BIFF12_ID_WORKSHEET         = 0x0081;
const sal_Int32 BIFF12_ID_XF                = 0x002F;

// BIFF2-BIFF8 record identifiers =============================================

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


// record identifiers ---------------------------------------------------------

const sal_uInt16 BIFF2_ID_BOF               = 0x0009;
const sal_uInt16 BIFF3_ID_BOF               = 0x0209;
const sal_uInt16 BIFF4_ID_BOF               = 0x0409;
const sal_uInt16 BIFF5_ID_BOF               = 0x0809;
const sal_uInt16 BIFF_ID_CONT               = 0x003C;
const sal_uInt16 BIFF_ID_EOF                = 0x000A;
const sal_uInt16 BIFF_ID_PCDEFINITION       = 0x00C6;
const sal_uInt16 BIFF_ID_PCDEFINITION2      = 0x0122;
const sal_uInt16 BIFF_ID_PCDFDISCRETEPR     = 0x00D9;
const sal_uInt16 BIFF_ID_PCDFIELD           = 0x00C7;
const sal_uInt16 BIFF_ID_PCDFRANGEPR        = 0x00D8;
const sal_uInt16 BIFF_ID_PCDFSQLTYPE        = 0x01BB;
const sal_uInt16 BIFF_ID_PCITEM_BOOL        = 0x00CA;
const sal_uInt16 BIFF_ID_PCITEM_DATE        = 0x00CE;
const sal_uInt16 BIFF_ID_PCITEM_DOUBLE      = 0x00C9;
const sal_uInt16 BIFF_ID_PCITEM_ERROR       = 0x00CB;
const sal_uInt16 BIFF_ID_PCITEM_INDEXLIST   = 0x00C8;
const sal_uInt16 BIFF_ID_PCITEM_INTEGER     = 0x00CC;
const sal_uInt16 BIFF_ID_PCITEM_MISSING     = 0x00CF;
const sal_uInt16 BIFF_ID_PCITEM_STRING      = 0x00CD;

const sal_uInt16 BIFF_ID_UNKNOWN            = SAL_MAX_UINT16;

// record constants -----------------------------------------------------------

const sal_uInt8 BIFF_ERR_NULL               = 0x00;
const sal_uInt8 BIFF_ERR_DIV0               = 0x07;
const sal_uInt8 BIFF_ERR_VALUE              = 0x0F;
const sal_uInt8 BIFF_ERR_REF                = 0x17;
const sal_uInt8 BIFF_ERR_NAME               = 0x1D;
const sal_uInt8 BIFF_ERR_NUM                = 0x24;
const sal_uInt8 BIFF_ERR_NA                 = 0x2A;

// BIFF8 unicode strings ------------------------------------------------------

const sal_uInt8 BIFF_STRF_16BIT             = 0x01;
const sal_uInt8 BIFF_STRF_PHONETIC          = 0x04;
const sal_uInt8 BIFF_STRF_RICH              = 0x08;
const sal_uInt8 BIFF_STRF_UNKNOWN           = 0xF2;

/** Static helper functions for BIFF filters. */
class BiffHelper
{
public:
    // conversion -------------------------------------------------------------

    /** Converts the passed packed number to a double. */
    static double       calcDoubleFromRk( sal_Int32 nRkValue );

    /** Converts the passed BIFF error to a double containing the respective Calc error code. */
    static double       calcDoubleFromError( sal_uInt8 nErrorCode );

    // BIFF12 import ----------------------------------------------------------

    /** Reads a BIFF12 string with leading 16-bit or 32-bit length field. */
    static OUString readString( SequenceInputStream& rStrm, bool b32BitLen = true, bool bAllowNulChars = false );

    // BIFF2-BIFF8 import -----------------------------------------------------

    /** Returns true, if the current record of the stream is a BOF record. */
    static bool         isBofRecord( BiffInputStream& rStrm );

    /** Skips a block of records up to the specified end record.

        Skips all records until next end record. When this function returns,
        the stream points to the end record, and the next call of the function
        startNextRecord() at the stream will start the record following the end
        record.

        The identifier of the record that is active while this function is
        called is used as start record identifier. This identifier is used to
        correctly skip embedded record blocks with the same start and end
        record identifier.

        @return  True = stream points to the end record.
     */
    static bool         skipRecordBlock( BiffInputStream& rStrm, sal_uInt16 nEndRecId );

private:
                        BiffHelper() = delete;
                        ~BiffHelper() = delete;
};

/** BIFF12 stream operator for an OUString, reads 32-bit string length and Unicode array. */
inline SequenceInputStream& operator>>( SequenceInputStream& rStrm, OUString& orString )
{
    orString = BiffHelper::readString( rStrm );
    return rStrm;
}

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
