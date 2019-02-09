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

#include "xmlstyle.hxx"
#include "xmlexprt.hxx"

#include <rangeutl.hxx>
#include <unonames.hxx>

#include <o3tl/safeint.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/families.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/txtprmap.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/util/CellProtection.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellVertJustify2.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/table/CellJustifyMethod.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <comphelper/extract.hxx>

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star;
using namespace ::xmloff::token;
using namespace ::formula;

#define MAP(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context, SvtSaveOptions::ODFVER_010, false }
// extensions import/export
#define MAP_EXT(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context, SvtSaveOptions::ODFVER_012_EXT_COMPAT, false }
// extensions import only
#define MAP_EXT_I(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context, SvtSaveOptions::ODFVER_012_EXT_COMPAT, true }
#define MAP_END()   { nullptr, 0, 0, XML_TOKEN_INVALID, 0, 0, SvtSaveOptions::ODFVER_010, false }

const XMLPropertyMapEntry aXMLScCellStylesProperties[] =
{
    MAP( "AsianVerticalMode", XML_NAMESPACE_STYLE, XML_GLYPH_ORIENTATION_VERTICAL, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_VERTICAL, 0),
    MAP( "BottomBorder", XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER, CTF_SC_BOTTOMBORDER ),
    MAP( "BottomBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER_WIDTH, CTF_SC_BOTTOMBORDERWIDTH ),
    MAP( "CellBackColor", XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "CellProtection", XML_NAMESPACE_STYLE, XML_CELL_PROTECT, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_CELLPROTECTION|MID_FLAG_MERGE_PROPERTY, 0 ),
    MAP( "CellProtection", XML_NAMESPACE_STYLE, XML_PRINT_CONTENT, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_PRINTCONTENT|MID_FLAG_MERGE_PROPERTY, 0 ),
    MAP( "CellStyle", XML_NAMESPACE_STYLE, XML_STYLE, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_STRING, CTF_SC_CELLSTYLE ),
    MAP( "ConditionalFormatXML", XML_NAMESPACE_STYLE, XML_MAP, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM, CTF_SC_IMPORT_MAP ),
    MAP( "ConditionalFormatXML", XML_NAMESPACE_STYLE, XML_MAP, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM, CTF_SC_MAP ),
    MAP( "DiagonalBLTR", XML_NAMESPACE_STYLE, XML_DIAGONAL_BL_TR, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER, CTF_SC_DIAGONALBLTR ),
    MAP( "DiagonalBLTR", XML_NAMESPACE_STYLE, XML_DIAGONAL_BL_TR_WIDTH, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER_WIDTH, CTF_SC_DIAGONALBLTRWIDTH ), // #i102690# for old files
    MAP( "DiagonalBLTR", XML_NAMESPACE_STYLE, XML_DIAGONAL_BL_TR_WIDTHS, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER_WIDTH, CTF_SC_DIAGONALBLTRWIDTHS ),
    MAP( "DiagonalTLBR", XML_NAMESPACE_STYLE, XML_DIAGONAL_TL_BR, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER, CTF_SC_DIAGONALTLBR ),
    MAP( "DiagonalTLBR", XML_NAMESPACE_STYLE, XML_DIAGONAL_TL_BR_WIDTH, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER_WIDTH, CTF_SC_DIAGONALTLBRWIDTH ), // #i102690# for old files
    MAP( "DiagonalTLBR", XML_NAMESPACE_STYLE, XML_DIAGONAL_TL_BR_WIDTHS, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER_WIDTH, CTF_SC_DIAGONALTLBRWIDTHS ),
    MAP( "HoriJustify", XML_NAMESPACE_FO, XML_TEXT_ALIGN, XML_TYPE_PROP_PARAGRAPH|XML_SC_TYPE_HORIJUSTIFY|MID_FLAG_MERGE_PROPERTY, 0 ),
    MAP( "HoriJustify", XML_NAMESPACE_STYLE, XML_TEXT_ALIGN_SOURCE, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_HORIJUSTIFYSOURCE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MAP( "HoriJustify", XML_NAMESPACE_STYLE, XML_REPEAT_CONTENT, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_HORIJUSTIFYREPEAT|MID_FLAG_MERGE_PROPERTY, 0 ),
    MAP_EXT( SC_UNONAME_HYPERLINK, XML_NAMESPACE_STYLE, XML_HYPERLINK, XML_TYPE_PROP_TABLE_CELL | XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM, CTF_SC_HYPERLINK ),
    MAP_EXT( SC_UNONAME_CELLHJUS_METHOD, XML_NAMESPACE_CSS3TEXT, XML_TEXT_JUSTIFY, XML_TYPE_PROP_PARAGRAPH|XML_SC_TYPE_HORIJUSTIFY_METHOD, 0 ),
    MAP( "IsCellBackgroundTransparent", XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_ISTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "IsTextWrapped", XML_NAMESPACE_FO, XML_WRAP_OPTION, XML_TYPE_PROP_TABLE_CELL|XML_SC_ISTEXTWRAPPED, 0 ),
    MAP( "LeftBorder", XML_NAMESPACE_FO, XML_BORDER, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER, CTF_SC_ALLBORDER ),
    MAP( "LeftBorder", XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER, CTF_SC_LEFTBORDER ),
    MAP( "LeftBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER_WIDTH, CTF_SC_ALLBORDERWIDTH ),
    MAP( "LeftBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER_WIDTH, CTF_SC_LEFTBORDERWIDTH ),
    MAP( "NumberFormat", XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_NUMBER|MID_FLAG_SPECIAL_ITEM, CTF_SC_NUMBERFORMAT),
    MAP( "Orientation", XML_NAMESPACE_STYLE, XML_DIRECTION, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_ORIENTATION, 0 ),
    MAP( "ParaBottomMargin", XML_NAMESPACE_FO, XML_PADDING, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_MEASURE, CTF_SC_ALLPADDING ),
    MAP( "ParaBottomMargin", XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_MEASURE, CTF_SC_BOTTOMPADDING ),
    MAP( "ParaIndent", XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_TYPE_PROP_PARAGRAPH|XML_TYPE_MEASURE16, 0 ),
    MAP( "ParaLeftMargin", XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_MEASURE, CTF_SC_LEFTPADDING ),
    MAP( "ParaRightMargin", XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_MEASURE, CTF_SC_RIGHTPADDING ),
    MAP( "ParaTopMargin", XML_NAMESPACE_FO, XML_PADDING_TOP, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_MEASURE, CTF_SC_TOPPADDING ),
    MAP( "RightBorder", XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER, CTF_SC_RIGHTBORDER ),
    MAP( "RightBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER_WIDTH, CTF_SC_RIGHTBORDERWIDTH ),
    MAP( "RotateAngle", XML_NAMESPACE_STYLE, XML_ROTATION_ANGLE, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_ROTATEANGLE, 0 ),
    MAP( "RotateReference", XML_NAMESPACE_STYLE, XML_ROTATION_ALIGN, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_ROTATEREFERENCE, 0),
    MAP( "ShadowFormat", XML_NAMESPACE_STYLE, XML_SHADOW, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_TEXT_SHADOW, 0 ),
    MAP( "ShrinkToFit", XML_NAMESPACE_STYLE, XML_SHRINK_TO_FIT, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BOOL, 0 ),
    MAP( "StandardDecimals", XML_NAMESPACE_STYLE, XML_DECIMAL_PLACES, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_NUMBER16, 0 ),
    MAP( "TopBorder", XML_NAMESPACE_FO, XML_BORDER_TOP, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER, CTF_SC_TOPBORDER ),
    MAP( "TopBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BORDER_WIDTH, CTF_SC_TOPBORDERWIDTH ),
    MAP( "UserDefinedAttributes", XML_NAMESPACE_TEXT, XML_XMLNS, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),
    MAP( "ValidationXML", XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION, XML_TYPE_PROP_TABLE_CELL|XML_TYPE_BUILDIN_CMP_ONLY, CTF_SC_VALIDATION ),
    MAP( "VertJustify", XML_NAMESPACE_STYLE, XML_VERTICAL_ALIGN, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_VERTJUSTIFY, 0),
    MAP_EXT_I( SC_UNONAME_CELLVJUS_METHOD, XML_NAMESPACE_STYLE, XML_VERTICAL_JUSTIFY, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_VERTJUSTIFY_METHOD, 0 ), // proposed ODF 1.2+
    MAP_EXT( SC_UNONAME_CELLVJUS_METHOD, XML_NAMESPACE_LO_EXT, XML_VERTICAL_JUSTIFY, XML_TYPE_PROP_TABLE_CELL|XML_SC_TYPE_VERTJUSTIFY_METHOD, 0 ),  // extension namespace
    MAP_END()
};

const XMLPropertyMapEntry aXMLScColumnStylesProperties[] =
{
    MAP( "IsManualPageBreak", XML_NAMESPACE_FO, XML_BREAK_BEFORE, XML_TYPE_PROP_TABLE_COLUMN|XML_SC_TYPE_BREAKBEFORE, 0),
    MAP( "IsVisible", XML_NAMESPACE_TABLE, XML_DISPLAY, XML_TYPE_PROP_TABLE_COLUMN|XML_SC_TYPE_EQUAL|MID_FLAG_SPECIAL_ITEM, CTF_SC_ISVISIBLE ),
    MAP( "Width", XML_NAMESPACE_STYLE, XML_COLUMN_WIDTH, XML_TYPE_PROP_TABLE_COLUMN|XML_TYPE_MEASURE, 0 ),
    MAP_END()
};

const XMLPropertyMapEntry aXMLScRowStylesImportProperties[] =
{
    // #i57867# Include background color (CellBackColor/IsCellBackgroundTransparent) for import only.
    // Import and export should use the same map, with MID_FLAG_NO_PROPERTY_EXPORT for the background entries,
    // but this doesn't work at the moment because SvXMLImportPropertyMapper compares MID_FLAG_NO_PROPERTY to 0.
    // If this is changed (not for 2.0.x), a single map can be used again.

    MAP( "CellBackColor", XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_TYPE_PROP_TABLE_ROW|XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "Height", XML_NAMESPACE_STYLE, XML_ROW_HEIGHT, XML_TYPE_PROP_TABLE_ROW|XML_TYPE_MEASURE, CTF_SC_ROWHEIGHT),
    MAP( "IsCellBackgroundTransparent", XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_TYPE_PROP_TABLE_ROW|XML_TYPE_ISTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "IsManualPageBreak", XML_NAMESPACE_FO, XML_BREAK_BEFORE, XML_TYPE_PROP_TABLE_ROW|XML_SC_TYPE_BREAKBEFORE, CTF_SC_ROWBREAKBEFORE),
    MAP( "OptimalHeight", XML_NAMESPACE_STYLE, XML_USE_OPTIMAL_ROW_HEIGHT, XML_TYPE_PROP_TABLE_ROW|XML_TYPE_BOOL, CTF_SC_ROWOPTIMALHEIGHT),
    MAP_END()
};

const XMLPropertyMapEntry aXMLScRowStylesProperties[] =
{
    MAP( "Height", XML_NAMESPACE_STYLE, XML_ROW_HEIGHT, XML_TYPE_PROP_TABLE_ROW|XML_TYPE_MEASURE, CTF_SC_ROWHEIGHT),
    MAP( "IsManualPageBreak", XML_NAMESPACE_FO, XML_BREAK_BEFORE, XML_TYPE_PROP_TABLE_ROW|XML_SC_TYPE_BREAKBEFORE, CTF_SC_ROWBREAKBEFORE),
    MAP( "OptimalHeight", XML_NAMESPACE_STYLE, XML_USE_OPTIMAL_ROW_HEIGHT, XML_TYPE_PROP_TABLE_ROW|XML_TYPE_BOOL, CTF_SC_ROWOPTIMALHEIGHT),
    MAP_END()
};

const XMLPropertyMapEntry aXMLScFromXLSRowStylesProperties[] =
{
    MAP( "Height", XML_NAMESPACE_STYLE, XML_ROW_HEIGHT, XML_TYPE_PROP_TABLE_ROW|XML_TYPE_MEASURE, CTF_SC_ROWHEIGHT),
    MAP( "IsManualPageBreak", XML_NAMESPACE_FO, XML_BREAK_BEFORE, XML_TYPE_PROP_TABLE_ROW|XML_SC_TYPE_BREAKBEFORE, CTF_SC_ROWBREAKBEFORE),
    MAP( "OptimalHeight", XML_NAMESPACE_STYLE, XML_USE_OPTIMAL_ROW_HEIGHT, XML_TYPE_PROP_TABLE_ROW|XML_TYPE_BOOL_FALSE, CTF_SC_ROWOPTIMALHEIGHT),
    MAP_END()
};

const XMLPropertyMapEntry aXMLScTableStylesImportProperties[] =
{
    // #i57869# Include background color (CellBackColor/IsCellBackgroundTransparent) for import only.
    // Import and export should use the same map, with MID_FLAG_NO_PROPERTY_EXPORT for the background entries,
    // but this doesn't work at the moment because SvXMLImportPropertyMapper compares MID_FLAG_NO_PROPERTY to 0.
    // If this is changed (not for 2.0.x), a single map can be used again.

    MAP( "CellBackColor", XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_TYPE_PROP_TABLE|XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "IsCellBackgroundTransparent", XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_TYPE_PROP_TABLE|XML_TYPE_ISTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "IsVisible", XML_NAMESPACE_TABLE, XML_DISPLAY, XML_TYPE_PROP_TABLE|XML_TYPE_BOOL, 0 ),
    MAP( "PageStyle", XML_NAMESPACE_STYLE, XML_MASTER_PAGE_NAME, XML_TYPE_PROP_TABLE|XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM, CTF_SC_MASTERPAGENAME ),
    MAP( "TableLayout", XML_NAMESPACE_STYLE, XML_WRITING_MODE, XML_TYPE_PROP_TABLE|XML_TYPE_TEXT_WRITING_MODE, 0 ),
    MAP( "TabColor", XML_NAMESPACE_TABLE, XML_TAB_COLOR, XML_TYPE_PROP_TABLE|XML_TYPE_COLORAUTO, 0 ),
    MAP_EXT( "TabColor", XML_NAMESPACE_TABLE_EXT, XML_TAB_COLOR, XML_TYPE_PROP_TABLE|XML_TYPE_COLORAUTO, 0 ),
    MAP_END()
};

const XMLPropertyMapEntry aXMLScTableStylesProperties[] =
{
    MAP( "IsVisible", XML_NAMESPACE_TABLE, XML_DISPLAY, XML_TYPE_PROP_TABLE|XML_TYPE_BOOL, 0 ),
    MAP( "PageStyle", XML_NAMESPACE_STYLE, XML_MASTER_PAGE_NAME, XML_TYPE_PROP_TABLE|XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM, CTF_SC_MASTERPAGENAME ),
    MAP( "TableLayout", XML_NAMESPACE_STYLE, XML_WRITING_MODE, XML_TYPE_PROP_TABLE|XML_TYPE_TEXT_WRITING_MODE, 0 ),
    MAP_EXT( "TabColor", XML_NAMESPACE_TABLE_EXT, XML_TAB_COLOR, XML_TYPE_PROP_TABLE|XML_TYPE_COLORAUTO, 0 ),
    MAP_END()
};

ScXMLCellExportPropertyMapper::ScXMLCellExportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper )
            : SvXMLExportPropertyMapper(rMapper)
{
}

ScXMLCellExportPropertyMapper::~ScXMLCellExportPropertyMapper()
{
}

void ScXMLCellExportPropertyMapper::ContextFilter(
    bool bEnableFoFontFamily,
    ::std::vector< XMLPropertyState >& rProperties,
    const uno::Reference< beans::XPropertySet >& rPropSet ) const
{
    XMLPropertyState* pPadding = nullptr;
    XMLPropertyState* pPadding_Bottom = nullptr;
    XMLPropertyState* pPadding_Left = nullptr;
    XMLPropertyState* pPadding_Right = nullptr;
    XMLPropertyState* pPadding_Top = nullptr;

    XMLPropertyState* pBorder = nullptr;
    XMLPropertyState* pBorder_Bottom = nullptr;
    XMLPropertyState* pBorder_Left = nullptr;
    XMLPropertyState* pBorder_Right = nullptr;
    XMLPropertyState* pBorder_Top = nullptr;
    XMLPropertyState* pSWBorder = nullptr;
    XMLPropertyState* pSWBorder_Bottom = nullptr;
    XMLPropertyState* pSWBorder_Left = nullptr;
    XMLPropertyState* pSWBorder_Right = nullptr;
    XMLPropertyState* pSWBorder_Top = nullptr;

    XMLPropertyState* pAllBorderWidthState = nullptr;
    XMLPropertyState* pLeftBorderWidthState = nullptr;
    XMLPropertyState* pRightBorderWidthState = nullptr;
    XMLPropertyState* pTopBorderWidthState = nullptr;
    XMLPropertyState* pBottomBorderWidthState = nullptr;
    XMLPropertyState* pSWAllBorderWidthState = nullptr;
    XMLPropertyState* pSWLeftBorderWidthState = nullptr;
    XMLPropertyState* pSWRightBorderWidthState = nullptr;
    XMLPropertyState* pSWTopBorderWidthState = nullptr;
    XMLPropertyState* pSWBottomBorderWidthState = nullptr;
    XMLPropertyState* pDiagonalTLBRWidthState = nullptr;
    XMLPropertyState* pDiagonalBLTRWidthState = nullptr;

    XMLPropertyState* pParaMarginLeft = nullptr;
    XMLPropertyState* pParaMarginLeftRel = nullptr;
    XMLPropertyState* pParaMarginRight = nullptr;
    XMLPropertyState* pParaMarginRightRel = nullptr;
    XMLPropertyState* pParaMarginTop = nullptr;
    XMLPropertyState* pParaMarginTopRel = nullptr;
    XMLPropertyState* pParaMarginBottom = nullptr;
    XMLPropertyState* pParaMarginBottomRel = nullptr;

    XMLPropertyState* pParaAdjust = nullptr;
    XMLPropertyState* pParaAdjustLast = nullptr;

    for( auto& rProperty : rProperties )
    {
        XMLPropertyState* propertyState = &rProperty;
        if (propertyState->mnIndex != -1)
        {
            switch( getPropertySetMapper()->GetEntryContextId( propertyState->mnIndex ) )
            {
                case CTF_SC_ALLPADDING:         pPadding = propertyState; break;
                case CTF_SC_BOTTOMPADDING:      pPadding_Bottom = propertyState; break;
                case CTF_SC_LEFTPADDING:        pPadding_Left = propertyState; break;
                case CTF_SC_RIGHTPADDING:       pPadding_Right = propertyState; break;
                case CTF_SC_TOPPADDING:         pPadding_Top = propertyState; break;
                case CTF_SC_ALLBORDER:          pBorder = propertyState; break;
                case CTF_SC_LEFTBORDER:         pBorder_Left = propertyState; break;
                case CTF_SC_RIGHTBORDER:        pBorder_Right = propertyState; break;
                case CTF_SC_BOTTOMBORDER:       pBorder_Bottom = propertyState; break;
                case CTF_SC_TOPBORDER:          pBorder_Top = propertyState; break;
                case CTF_SC_ALLBORDERWIDTH:     pAllBorderWidthState = propertyState; break;
                case CTF_SC_LEFTBORDERWIDTH:    pLeftBorderWidthState = propertyState; break;
                case CTF_SC_RIGHTBORDERWIDTH:   pRightBorderWidthState = propertyState; break;
                case CTF_SC_TOPBORDERWIDTH:     pTopBorderWidthState = propertyState; break;
                case CTF_SC_BOTTOMBORDERWIDTH:  pBottomBorderWidthState = propertyState; break;
                case CTF_ALLBORDER:             pSWBorder = propertyState; break;
                case CTF_LEFTBORDER:            pSWBorder_Left = propertyState; break;
                case CTF_RIGHTBORDER:           pSWBorder_Right = propertyState; break;
                case CTF_BOTTOMBORDER:          pSWBorder_Bottom = propertyState; break;
                case CTF_TOPBORDER:             pSWBorder_Top = propertyState; break;
                case CTF_ALLBORDERWIDTH:        pSWAllBorderWidthState = propertyState; break;
                case CTF_LEFTBORDERWIDTH:       pSWLeftBorderWidthState = propertyState; break;
                case CTF_RIGHTBORDERWIDTH:      pSWRightBorderWidthState = propertyState; break;
                case CTF_TOPBORDERWIDTH:        pSWTopBorderWidthState = propertyState; break;
                case CTF_BOTTOMBORDERWIDTH:     pSWBottomBorderWidthState = propertyState; break;
                case CTF_SC_DIAGONALTLBR:       break; //old diagonal line attribute names without "s" are only read, not written
                case CTF_SC_DIAGONALTLBRWIDTH:  pDiagonalTLBRWidthState = propertyState; break;
                case CTF_SC_DIAGONALBLTR:       break; //old diagonal line attribute names without "s" are only read, not written
                case CTF_SC_DIAGONALBLTRWIDTH:  pDiagonalBLTRWidthState = propertyState; break;
                case CTF_SD_SHAPE_PARA_ADJUST:  pParaAdjust = propertyState; break;
                case CTF_PARA_ADJUSTLAST:       pParaAdjustLast = propertyState; break;
                case CTF_PARALEFTMARGIN:        pParaMarginLeft = propertyState; break;
                case CTF_PARALEFTMARGIN_REL:    pParaMarginLeftRel = propertyState; break;
                case CTF_PARARIGHTMARGIN:       pParaMarginRight = propertyState; break;
                case CTF_PARARIGHTMARGIN_REL:   pParaMarginRightRel = propertyState; break;
                case CTF_PARATOPMARGIN:         pParaMarginTop = propertyState; break;
                case CTF_PARATOPMARGIN_REL:     pParaMarginTopRel = propertyState; break;
                case CTF_PARABOTTOMMARGIN:      pParaMarginBottom = propertyState; break;
                case CTF_PARABOTTOMMARGIN_REL:  pParaMarginBottomRel = propertyState; break;
            }
        }
    }

    if (pPadding && pPadding_Bottom && pPadding_Left && pPadding_Right && pPadding_Top)
    {
        sal_Int32 nBottom = 0, nTop = 0, nLeft = 0, nRight = 0;
        if ((pPadding_Bottom->maValue >>= nBottom) &&
            (pPadding_Left->maValue >>= nLeft) &&
            (pPadding_Right->maValue >>= nRight) &&
            (pPadding_Top->maValue >>= nTop))
        {
            if ((nBottom == nTop) && (nLeft == nRight) && (nTop == nLeft))
            {
                pPadding_Bottom->mnIndex = -1;
                pPadding_Bottom->maValue.clear();
                pPadding_Left->mnIndex = -1;
                pPadding_Left->maValue.clear();
                pPadding_Right->mnIndex = -1;
                pPadding_Right->maValue.clear();
                pPadding_Top->mnIndex = -1;
                pPadding_Top->maValue.clear();
            }
            else
            {
                pPadding->mnIndex = -1;
                pPadding->maValue.clear();
            }
        }
    }
    if( pBorder )
    {
        if( pBorder_Left && pBorder_Right && pBorder_Top && pBorder_Bottom )
        {
            table::BorderLine2 aLeft, aRight, aTop, aBottom;

            pBorder_Left->maValue >>= aLeft;
            pBorder_Right->maValue >>= aRight;
            pBorder_Top->maValue >>= aTop;
            pBorder_Bottom->maValue >>= aBottom;
            if( aLeft.Color == aRight.Color && aLeft.InnerLineWidth == aRight.InnerLineWidth &&
                aLeft.OuterLineWidth == aRight.OuterLineWidth && aLeft.LineDistance == aRight.LineDistance &&
                aLeft.Color == aTop.Color && aLeft.InnerLineWidth == aTop.InnerLineWidth &&
                aLeft.OuterLineWidth == aTop.OuterLineWidth && aLeft.LineDistance == aTop.LineDistance &&
                aLeft.Color == aBottom.Color && aLeft.InnerLineWidth == aBottom.InnerLineWidth &&
                aLeft.OuterLineWidth == aBottom.OuterLineWidth && aLeft.LineDistance == aBottom.LineDistance &&
                aLeft.LineStyle == aRight.LineStyle && aLeft.LineStyle == aTop.LineStyle &&
                aLeft.LineStyle == aBottom.LineStyle && aLeft.LineWidth == aRight.LineWidth &&
                aLeft.LineWidth == aTop.LineWidth && aLeft.LineWidth == aBottom.LineWidth )
            {
                pBorder_Left->mnIndex = -1;
                pBorder_Left->maValue.clear();
                pBorder_Right->mnIndex = -1;
                pBorder_Right->maValue.clear();
                pBorder_Top->mnIndex = -1;
                pBorder_Top->maValue.clear();
                pBorder_Bottom->mnIndex = -1;
                pBorder_Bottom->maValue.clear();
            }
            else
            {
                pBorder->mnIndex = -1;
                pBorder->maValue.clear();
            }
        }
        else
        {
            pBorder->mnIndex = -1;
            pBorder->maValue.clear();
        }
    }
    if( pAllBorderWidthState )
    {
        if( pLeftBorderWidthState && pRightBorderWidthState && pTopBorderWidthState && pBottomBorderWidthState )
        {
            table::BorderLine2 aLeft, aRight, aTop, aBottom;

            pLeftBorderWidthState->maValue >>= aLeft;
            pRightBorderWidthState->maValue >>= aRight;
            pTopBorderWidthState->maValue >>= aTop;
            pBottomBorderWidthState->maValue >>= aBottom;
            if( aLeft.InnerLineWidth == aRight.InnerLineWidth && aLeft.OuterLineWidth == aRight.OuterLineWidth &&
                aLeft.LineDistance == aRight.LineDistance && aLeft.InnerLineWidth == aTop.InnerLineWidth &&
                aLeft.OuterLineWidth == aTop.OuterLineWidth && aLeft.LineDistance == aTop.LineDistance &&
                aLeft.InnerLineWidth == aBottom.InnerLineWidth && aLeft.OuterLineWidth == aBottom.OuterLineWidth &&
                aLeft.LineDistance == aBottom.LineDistance && aLeft.LineWidth == aRight.LineWidth &&
                aLeft.LineWidth == aTop.LineWidth && aLeft.LineWidth == aBottom.LineWidth )
            {
                pLeftBorderWidthState->mnIndex = -1;
                pLeftBorderWidthState->maValue.clear();
                pRightBorderWidthState->mnIndex = -1;
                pRightBorderWidthState->maValue.clear();
                pTopBorderWidthState->mnIndex = -1;
                pTopBorderWidthState->maValue.clear();
                pBottomBorderWidthState->mnIndex = -1;
                pBottomBorderWidthState->maValue.clear();
            }
            else
            {
                pAllBorderWidthState->mnIndex = -1;
                pAllBorderWidthState->maValue.clear();
            }
        }
        else
        {
            pAllBorderWidthState->mnIndex = -1;
            pAllBorderWidthState->maValue.clear();
        }
    }

    if (pParaAdjust)
    {
        pParaAdjust->mnIndex = -1;
        pParaAdjust->maValue.clear();
    }
    if (pParaAdjustLast)
    {
        pParaAdjustLast->mnIndex = -1;
        pParaAdjustLast->maValue.clear();
    }
    if (pSWBorder)
    {
        pSWBorder->mnIndex = -1;
        pSWBorder->maValue.clear();
    }
    if (pSWBorder_Left)
    {
        pSWBorder_Left->mnIndex = -1;
        pSWBorder_Left->maValue.clear();
    }
    if (pSWBorder_Right)
    {
        pSWBorder_Right->mnIndex = -1;
        pSWBorder_Right->maValue.clear();
    }
    if (pSWBorder_Bottom)
    {
        pSWBorder_Bottom->mnIndex = -1;
        pSWBorder_Bottom->maValue.clear();
    }
    if (pSWBorder_Top)
    {
        pSWBorder_Top->mnIndex = -1;
        pSWBorder_Top->maValue.clear();
    }
    if (pSWAllBorderWidthState)
    {
        pSWAllBorderWidthState->mnIndex = -1;
        pSWAllBorderWidthState->maValue.clear();
    }
    if (pSWLeftBorderWidthState)
    {
        pSWLeftBorderWidthState->mnIndex = -1;
        pSWLeftBorderWidthState->maValue.clear();
    }
    if (pSWRightBorderWidthState)
    {
        pSWRightBorderWidthState->mnIndex = -1;
        pSWRightBorderWidthState->maValue.clear();
    }
    if (pSWTopBorderWidthState)
    {
        pSWTopBorderWidthState->mnIndex = -1;
        pSWTopBorderWidthState->maValue.clear();
    }
    if (pSWBottomBorderWidthState)
    {
        pSWBottomBorderWidthState->mnIndex = -1;
        pSWBottomBorderWidthState->maValue.clear();
    }

    if (pParaMarginLeft)
    {
        pParaMarginLeft->mnIndex = -1;
        pParaMarginLeft->maValue.clear();
    }
    if (pParaMarginLeftRel)
    {
        pParaMarginLeftRel->mnIndex = -1;
        pParaMarginLeftRel->maValue.clear();
    }
    if (pParaMarginRight)
    {
        pParaMarginRight->mnIndex = -1;
        pParaMarginRight->maValue.clear();
    }
    if (pParaMarginRightRel)
    {
        pParaMarginRightRel->mnIndex = -1;
        pParaMarginRightRel->maValue.clear();
    }
    if (pParaMarginTop)
    {
        pParaMarginTop->mnIndex = -1;
        pParaMarginTop->maValue.clear();
    }
    if (pParaMarginTopRel)
    {
        pParaMarginTopRel->mnIndex = -1;
        pParaMarginTopRel->maValue.clear();
    }
    if (pParaMarginBottom)
    {
        pParaMarginBottom->mnIndex = -1;
        pParaMarginBottom->maValue.clear();
    }
    if (pParaMarginBottomRel)
    {
        pParaMarginBottomRel->mnIndex = -1;
        pParaMarginBottomRel->maValue.clear();
    }

    // #i102690# old diagonal line attribute names without "s" are only read, not written
    if (pDiagonalTLBRWidthState)
    {
        pDiagonalTLBRWidthState->mnIndex = -1;
        pDiagonalTLBRWidthState->maValue.clear();
    }
    if (pDiagonalBLTRWidthState)
    {
        pDiagonalBLTRWidthState->mnIndex = -1;
        pDiagonalBLTRWidthState->maValue.clear();
    }

    SvXMLExportPropertyMapper::ContextFilter(bEnableFoFontFamily, rProperties, rPropSet);
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void ScXMLCellExportPropertyMapper::handleSpecialItem(
            SvXMLAttributeList& /* rAttrList */,
            const XMLPropertyState& /* rProperty */,
            const SvXMLUnitConverter& /* rUnitConverter */,
            const SvXMLNamespaceMap& /* rNamespaceMap */,
            const ::std::vector< XMLPropertyState > * /* pProperties */,
            sal_uInt32 /* nIdx */ ) const
{
    // the SpecialItem NumberFormat must not be handled by this method
    // the SpecialItem ConditionlaFormat must not be handled by this method
    // the SpecialItem CharBackColor must not be handled by this method
}
void ScXMLCellExportPropertyMapper::handleElementItem(
            SvXMLExport& rExport,
            const XMLPropertyState& rProperty,
            SvXmlExportFlags /* nFlags */,
            const ::std::vector< XMLPropertyState > * /* pProperties */,
            sal_uInt32 /* nIdx */) const
{
    sal_uInt32 nContextId = getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex );
    OUString sURL;
    if ( ( nContextId == CTF_SC_HYPERLINK ) &&
        ( rProperty.maValue >>= sURL ) &&
        !sURL.isEmpty() )
    {
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sURL );
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE,
                                      XML_SIMPLE );
        sal_uInt32 nPropIndex = rProperty.mnIndex;
        sal_uInt16 nPrefix = getPropertySetMapper()->GetEntryNameSpace( nPropIndex );
        OUString sLocalName = getPropertySetMapper()->GetEntryXMLName( nPropIndex );
        SvXMLElementExport aElem( rExport, nPrefix, sLocalName, true, true );
    }
}

ScXMLRowExportPropertyMapper::ScXMLRowExportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper )
            : SvXMLExportPropertyMapper(rMapper)
{
}

ScXMLRowExportPropertyMapper::~ScXMLRowExportPropertyMapper()
{
}

void ScXMLRowExportPropertyMapper::ContextFilter(
    bool /* bEnableFoFontFamily */,
    ::std::vector< XMLPropertyState >& /* rProperties */,
    const uno::Reference< beans::XPropertySet >& /* rPropSet */ ) const
{
    //#108550#; don't filter the height, so other applications know the calculated height
}

ScXMLColumnExportPropertyMapper::ScXMLColumnExportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper )
            : SvXMLExportPropertyMapper(rMapper)
{
}

ScXMLColumnExportPropertyMapper::~ScXMLColumnExportPropertyMapper()
{
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void ScXMLColumnExportPropertyMapper::handleSpecialItem(
            SvXMLAttributeList& /* rAttrList */,
            const XMLPropertyState& /* rProperty */,
            const SvXMLUnitConverter& /* rUnitConverter */,
            const SvXMLNamespaceMap& /* rNamespaceMap */,
            const ::std::vector< XMLPropertyState > * /* pProperties */,
            sal_uInt32 /* nIdx */ ) const
{
    // the SpecialItem IsVisible must not be handled by this method
}

ScXMLTableExportPropertyMapper::ScXMLTableExportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper )
            : SvXMLExportPropertyMapper(rMapper)
{
}

ScXMLTableExportPropertyMapper::~ScXMLTableExportPropertyMapper()
{
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void ScXMLTableExportPropertyMapper::handleSpecialItem(
            SvXMLAttributeList& /* rAttrList */,
            const XMLPropertyState& /* rProperty */,
            const SvXMLUnitConverter& /* rUnitConverter */,
            const SvXMLNamespaceMap& /* rNamespaceMap */,
            const ::std::vector< XMLPropertyState > * /* pProperties */,
            sal_uInt32 /* nIdx */ ) const
{
    // the SpecialItem PageStyle must not be handled by this method
}

void ScXMLAutoStylePoolP::exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const
{
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties, rPropExp, rUnitConverter, rNamespaceMap );
    if (nFamily == XML_STYLE_FAMILY_TABLE_CELL)
    {
        for(const auto& rProperty : rProperties)
        {
            rtl::Reference< XMLPropertySetMapper > aPropMapper(rScXMLExport.GetCellStylesPropertySetMapper());
            sal_Int16 nContextID(aPropMapper->GetEntryContextId(rProperty.mnIndex));
            switch (nContextID)
            {
                case CTF_SC_NUMBERFORMAT :
                {
                    sal_Int32 nNumberFormat = 0;
                    if (rProperty.maValue >>= nNumberFormat)
                    {
                        OUString sAttrValue(rScXMLExport.getDataStyleName(nNumberFormat));
                        if (!sAttrValue.isEmpty())
                        {
                            GetExport().AddAttribute(
                                aPropMapper->GetEntryNameSpace(rProperty.mnIndex),
                                aPropMapper->GetEntryXMLName(rProperty.mnIndex),
                                sAttrValue );
                        }
                    }
                }
                break;
            }
        }
    }
    else if (nFamily == XML_STYLE_FAMILY_TABLE_TABLE)
    {
        for(const auto& rProperty : rProperties)
        {
            rtl::Reference< XMLPropertySetMapper > aPropMapper(rScXMLExport.GetTableStylesPropertySetMapper());
            sal_Int16 nContextID(aPropMapper->GetEntryContextId(rProperty.mnIndex));
            switch (nContextID)
            {
                case CTF_SC_MASTERPAGENAME :
                {
                    OUString sName;
                    if (rProperty.maValue >>= sName)
                    {
                        GetExport().AddAttribute(
                            aPropMapper->GetEntryNameSpace(rProperty.mnIndex),
                            aPropMapper->GetEntryXMLName(rProperty.mnIndex),
                            GetExport().EncodeStyleName( sName ));
                    }
                }
                break;
            }
        }
    }
}

void ScXMLAutoStylePoolP::exportStyleContent(
        const css::uno::Reference< css::xml::sax::XDocumentHandler > & rHandler,
        sal_Int32 nFamily,
        const std::vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp
        , const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap
        ) const
{
    SvXMLAutoStylePoolP::exportStyleContent( rHandler, nFamily, rProperties, rPropExp, rUnitConverter, rNamespaceMap );
    if (nFamily == XML_STYLE_FAMILY_TABLE_CELL)
    {
        for(const auto& rProperty : rProperties)
        {
            if (rProperty.mnIndex != -1)
            {
                sal_Int16 nContextID = rScXMLExport.GetCellStylesPropertySetMapper()->GetEntryContextId(rProperty.mnIndex);
                switch (nContextID)
                {
                    case CTF_SC_MAP :
                    {
                        uno::Reference<container::XIndexAccess> xIndex( rProperty.maValue, uno::UNO_QUERY );
                        if ( xIndex.is() )
                        {
                            sal_Int32 nConditionCount(xIndex->getCount());
                            for (sal_Int32 nCondition = 0; nCondition < nConditionCount; ++nCondition)
                            {
                                uno::Reference <sheet::XSheetConditionalEntry> xSheetConditionalEntry(xIndex->getByIndex(nCondition), uno::UNO_QUERY);
                                if (xSheetConditionalEntry.is())
                                {
                                    OUString sStyleName(xSheetConditionalEntry->getStyleName());
                                    uno::Reference <sheet::XSheetCondition> xSheetCondition(xSheetConditionalEntry, uno::UNO_QUERY);
                                    if (xSheetCondition.is())
                                    {
                                        sheet::ConditionOperator aOperator = xSheetCondition->getOperator();
                                        if (aOperator != sheet::ConditionOperator_NONE)
                                        {
                                            if (aOperator == sheet::ConditionOperator_FORMULA)
                                            {
                                                OUString sCondition = "is-true-formula("
                                                                    + xSheetCondition->getFormula1()
                                                                    + ")";
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_CONDITION, sCondition);
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_APPLY_STYLE_NAME, rScXMLExport.EncodeStyleName( sStyleName ));
                                                OUString sOUBaseAddress;
                                                ScDocument* pDoc = rScXMLExport.GetDocument();
                                                ScRangeStringConverter::GetStringFromAddress( sOUBaseAddress,
                                                    xSheetCondition->getSourcePosition(), pDoc, FormulaGrammar::CONV_OOO );
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_BASE_CELL_ADDRESS, sOUBaseAddress);
                                                SvXMLElementExport aMElem(rScXMLExport, XML_NAMESPACE_STYLE, XML_MAP, true, true);
                                            }
                                            else
                                            {
                                                OUString sCondition;
                                                if (aOperator == sheet::ConditionOperator_BETWEEN ||
                                                    aOperator == sheet::ConditionOperator_NOT_BETWEEN)
                                                {
                                                    if (aOperator == sheet::ConditionOperator_BETWEEN)
                                                        sCondition = "cell-content-is-between(";
                                                    else
                                                        sCondition = "cell-content-is-not-between(";
                                                    sCondition += xSheetCondition->getFormula1()
                                                                + ","
                                                                + xSheetCondition->getFormula2()
                                                                + ")";
                                                }
                                                else
                                                {
                                                    sCondition = "cell-content()";
                                                    switch (aOperator)
                                                    {
                                                        case sheet::ConditionOperator_LESS:
                                                            sCondition += "<";
                                                        break;
                                                        case sheet::ConditionOperator_GREATER:
                                                            sCondition += ">";
                                                        break;
                                                        case sheet::ConditionOperator_LESS_EQUAL:
                                                            sCondition += "<=";
                                                        break;
                                                        case sheet::ConditionOperator_GREATER_EQUAL:
                                                            sCondition += ">=";
                                                        break;
                                                        case sheet::ConditionOperator_EQUAL:
                                                            sCondition += "=";
                                                        break;
                                                        case sheet::ConditionOperator_NOT_EQUAL:
                                                            sCondition += "!=";
                                                        break;
                                                        default:
                                                        {
                                                            // added to avoid warnings
                                                        }
                                                    }
                                                    sCondition += xSheetCondition->getFormula1();
                                                }
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_CONDITION, sCondition);
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_APPLY_STYLE_NAME, rScXMLExport.EncodeStyleName( sStyleName ));
                                                OUString sOUBaseAddress;
                                                ScRangeStringConverter::GetStringFromAddress( sOUBaseAddress,
                                                    xSheetCondition->getSourcePosition(), rScXMLExport.GetDocument(), FormulaGrammar::CONV_OOO );
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_BASE_CELL_ADDRESS, sOUBaseAddress);
                                                SvXMLElementExport aMElem(rScXMLExport, XML_NAMESPACE_STYLE, XML_MAP, true, true);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}

ScXMLAutoStylePoolP::ScXMLAutoStylePoolP(ScXMLExport& rTempScXMLExport):
    SvXMLAutoStylePoolP(rTempScXMLExport),
    rScXMLExport(rTempScXMLExport)
{
}

ScXMLAutoStylePoolP::~ScXMLAutoStylePoolP()
{
}

void ScXMLStyleExport::exportStyleAttributes(
        const css::uno::Reference< css::style::XStyle > & rStyle )
{
    uno::Reference< beans::XPropertySet > xPropSet( rStyle, uno::UNO_QUERY );
    if (xPropSet.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo(xPropSet->getPropertySetInfo());
        OUString sNumberFormat("NumberFormat");
        if( xPropSetInfo->hasPropertyByName( sNumberFormat ) )
        {
            uno::Reference< beans::XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
            if( xPropState.is() && (beans::PropertyState_DIRECT_VALUE ==
                    xPropState->getPropertyState( sNumberFormat )) )
            {
                sal_Int32 nNumberFormat = 0;
                if (xPropSet->getPropertyValue( sNumberFormat ) >>= nNumberFormat)
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME,
                                                        GetExport().getDataStyleName(nNumberFormat) );
            }
        }
    }
}

void ScXMLStyleExport::exportStyleContent( const css::uno::Reference<css::style::XStyle > & /* rStyle */ )
{
}

ScXMLStyleExport::ScXMLStyleExport(
        SvXMLExport& rExp,
        SvXMLAutoStylePoolP *pAutoStyleP )
    : XMLStyleExport(rExp, pAutoStyleP)
{
}

ScXMLStyleExport::~ScXMLStyleExport()
{
}

XMLScPropHdlFactory::XMLScPropHdlFactory()
    : XMLPropertyHandlerFactory()
{
}

XMLScPropHdlFactory::~XMLScPropHdlFactory()
{
}

const XMLPropertyHandler* XMLScPropHdlFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    nType &= MID_FLAG_MASK;

    XMLPropertyHandler* pHdl(const_cast<XMLPropertyHandler*>(XMLPropertyHandlerFactory::GetPropertyHandler( nType )));
    if(!pHdl)
    {
        switch(nType)
        {
            case XML_SC_TYPE_CELLPROTECTION :
            {
                pHdl = new XmlScPropHdl_CellProtection;
            }
            break;
            case XML_SC_TYPE_PRINTCONTENT :
            {
                pHdl = new XmlScPropHdl_PrintContent;
            }
            break;
            case XML_SC_TYPE_HORIJUSTIFY_METHOD:
            case XML_SC_TYPE_VERTJUSTIFY_METHOD:
            {
                pHdl = new XmlScPropHdl_JustifyMethod;
            }
            break;
            case XML_SC_TYPE_HORIJUSTIFY :
            {
                pHdl = new XmlScPropHdl_HoriJustify;
            }
            break;
            case XML_SC_TYPE_HORIJUSTIFYSOURCE :
            {
                pHdl = new XmlScPropHdl_HoriJustifySource;
            }
            break;
            case XML_SC_TYPE_HORIJUSTIFYREPEAT :
            {
                pHdl = new XmlScPropHdl_HoriJustifyRepeat;
            }
            break;
            case XML_SC_TYPE_ORIENTATION :
            {
                pHdl = new XmlScPropHdl_Orientation;
            }
            break;
            case XML_SC_TYPE_ROTATEANGLE :
            {
                pHdl = new XmlScPropHdl_RotateAngle;
            }
            break;
            case XML_SC_TYPE_ROTATEREFERENCE :
            {
                pHdl = new XmlScPropHdl_RotateReference;
            }
            break;
            case XML_SC_TYPE_VERTJUSTIFY :
            {
                pHdl = new XmlScPropHdl_VertJustify;
            }
            break;
            case XML_SC_TYPE_BREAKBEFORE :
            {
                pHdl = new XmlScPropHdl_BreakBefore;
            }
            break;
            case XML_SC_ISTEXTWRAPPED :
            {
                pHdl = new XmlScPropHdl_IsTextWrapped;
            }
            break;
            case XML_SC_TYPE_EQUAL :
            {
                pHdl = new XmlScPropHdl_IsEqual;
            }
            break;
            case XML_SC_TYPE_VERTICAL :
            {
                pHdl = new XmlScPropHdl_Vertical;
            }
            break;
        }

        if(pHdl)
            PutHdlCache(nType, pHdl);
    }

    return pHdl;
}

XmlScPropHdl_CellProtection::~XmlScPropHdl_CellProtection()
{
}

bool XmlScPropHdl_CellProtection::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    util::CellProtection aCellProtection1, aCellProtection2;

    if((r1 >>= aCellProtection1) && (r2 >>= aCellProtection2))
    {
        return ((aCellProtection1.IsHidden == aCellProtection2.IsHidden) &&
                (aCellProtection1.IsLocked == aCellProtection2.IsLocked) &&
                (aCellProtection1.IsFormulaHidden == aCellProtection2.IsFormulaHidden));
    }
    return false;
}

bool XmlScPropHdl_CellProtection::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    util::CellProtection aCellProtection;
    bool bDefault(false);
    if (!rValue.hasValue())
    {
        aCellProtection.IsHidden = false;
        aCellProtection.IsLocked = true;
        aCellProtection.IsFormulaHidden = false;
        aCellProtection.IsPrintHidden = false;
        bDefault = true;
    }
    if ((rValue >>= aCellProtection) || bDefault)
    {
        if (IsXMLToken(rStrImpValue, XML_NONE))
        {
            aCellProtection.IsFormulaHidden = false;
            aCellProtection.IsHidden = false;
            aCellProtection.IsLocked = false;
            rValue <<= aCellProtection;
            bRetval = true;
        }
        else if (IsXMLToken(rStrImpValue, XML_HIDDEN_AND_PROTECTED))
        {
            aCellProtection.IsFormulaHidden = true;
            aCellProtection.IsHidden = true;
            aCellProtection.IsLocked = true;
            rValue <<= aCellProtection;
            bRetval = true;
        }
        else if (IsXMLToken(rStrImpValue, XML_PROTECTED))
        {
            aCellProtection.IsFormulaHidden = false;
            aCellProtection.IsHidden = false;
            aCellProtection.IsLocked = true;
            rValue <<= aCellProtection;
            bRetval = true;
        }
        else if (IsXMLToken(rStrImpValue, XML_FORMULA_HIDDEN))
        {
            aCellProtection.IsFormulaHidden = true;
            aCellProtection.IsHidden = false;
            aCellProtection.IsLocked = false;
            rValue <<= aCellProtection;
            bRetval = true;
        }
        else
        {
            sal_Int32 i(0);
            while (i < rStrImpValue.getLength() && rStrImpValue[i] != ' ')
                ++i;
            OUString sFirst(rStrImpValue.copy(0, i));
            OUString sSecond(rStrImpValue.copy(i + 1));
            aCellProtection.IsFormulaHidden = false;
            aCellProtection.IsHidden = false;
            aCellProtection.IsLocked = false;
            if ((IsXMLToken(sFirst, XML_PROTECTED)) || (IsXMLToken(sSecond, XML_PROTECTED)))
                aCellProtection.IsLocked = true;
            if ((IsXMLToken(sFirst, XML_FORMULA_HIDDEN)) || (IsXMLToken(sSecond, XML_FORMULA_HIDDEN)))
                aCellProtection.IsFormulaHidden = true;
            rValue <<= aCellProtection;
            bRetval = true;
        }
    }

    return bRetval;
}

bool XmlScPropHdl_CellProtection::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);
    util::CellProtection aCellProtection;

    if(rValue >>= aCellProtection)
    {
        if (!(aCellProtection.IsFormulaHidden || aCellProtection.IsHidden || aCellProtection.IsLocked))
        {
            rStrExpValue = GetXMLToken(XML_NONE);
            bRetval = true;
        }
        else if (aCellProtection.IsHidden)
        {
            // #i105964# "Hide all" implies "Protected" in the UI, so it must be saved as "hidden-and-protected"
            // even if "IsLocked" is not set in the CellProtection struct.
            rStrExpValue = GetXMLToken(XML_HIDDEN_AND_PROTECTED);
            bRetval = true;
        }
        else if (aCellProtection.IsLocked && !aCellProtection.IsFormulaHidden)
        {
            rStrExpValue = GetXMLToken(XML_PROTECTED);
            bRetval = true;
        }
        else if (aCellProtection.IsFormulaHidden && !aCellProtection.IsLocked)
        {
            rStrExpValue = GetXMLToken(XML_FORMULA_HIDDEN);
            bRetval = true;
        }
        else if (aCellProtection.IsFormulaHidden && aCellProtection.IsLocked)
        {
            rStrExpValue = GetXMLToken(XML_PROTECTED);
            rStrExpValue += " ";
            rStrExpValue += GetXMLToken(XML_FORMULA_HIDDEN);
            bRetval = true;
        }
    }

    return bRetval;
}

XmlScPropHdl_PrintContent::~XmlScPropHdl_PrintContent()
{
}

bool XmlScPropHdl_PrintContent::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    util::CellProtection aCellProtection1, aCellProtection2;

    if((r1 >>= aCellProtection1) && (r2 >>= aCellProtection2))
    {
        return (aCellProtection1.IsPrintHidden == aCellProtection2.IsPrintHidden);
    }
    return false;
}

bool XmlScPropHdl_PrintContent::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);
    util::CellProtection aCellProtection;
    bool bDefault(false);
    if (!rValue.hasValue())
    {
        aCellProtection.IsHidden = false;
        aCellProtection.IsLocked = true;
        aCellProtection.IsFormulaHidden = false;
        aCellProtection.IsPrintHidden = false;
        bDefault = true;
    }
    if ((rValue >>= aCellProtection) || bDefault)
    {
        bool bValue(false);
        if (::sax::Converter::convertBool(bValue, rStrImpValue))
        {
            aCellProtection.IsPrintHidden = !bValue;
            rValue <<= aCellProtection;
            bRetval = true;
        }
    }

    return bRetval;
}

bool XmlScPropHdl_PrintContent::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    util::CellProtection aCellProtection;
    if(rValue >>= aCellProtection)
    {
        OUStringBuffer sValue;
        ::sax::Converter::convertBool(sValue, !aCellProtection.IsPrintHidden);
        rStrExpValue = sValue.makeStringAndClear();
        bRetval = true;
    }

    return bRetval;
}

XmlScPropHdl_JustifyMethod::~XmlScPropHdl_JustifyMethod()
{
}

bool XmlScPropHdl_JustifyMethod::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    sal_Int32 nVal1(0), nVal2(0);

    if((r1 >>= nVal1) && (r2 >>= nVal2))
        return (nVal1 == nVal2);
    return false;
}

bool XmlScPropHdl_JustifyMethod::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval = false;

    sal_Int32 nValue = table::CellJustifyMethod::AUTO;
    if (IsXMLToken(rStrImpValue, XML_AUTO))
    {
        nValue = table::CellJustifyMethod::AUTO;
        rValue <<= nValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_DISTRIBUTE))
    {
        nValue = table::CellJustifyMethod::DISTRIBUTE;
        rValue <<= nValue;
        bRetval = true;
    }
    else
        bRetval = true;

    return bRetval;
}

bool XmlScPropHdl_JustifyMethod::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    sal_Int32 nVal(0);
    bool bRetval = false;

    if (rValue >>= nVal)
    {
        switch (nVal)
        {
            case table::CellJustifyMethod::AUTO:
            {
                rStrExpValue = GetXMLToken(XML_AUTO);
                bRetval = true;
            }
            break;
            case table::CellJustifyMethod::DISTRIBUTE:
            {
                rStrExpValue = GetXMLToken(XML_DISTRIBUTE);
                bRetval = true;
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
    return bRetval;
}

XmlScPropHdl_HoriJustify::~XmlScPropHdl_HoriJustify()
{
}

bool XmlScPropHdl_HoriJustify::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    table::CellHoriJustify aHoriJustify1, aHoriJustify2;

    if((r1 >>= aHoriJustify1) && (r2 >>= aHoriJustify2))
        return (aHoriJustify1 == aHoriJustify2);
    return false;
}

bool XmlScPropHdl_HoriJustify::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    table::CellHoriJustify nValue = table::CellHoriJustify_LEFT;
    rValue >>= nValue;
    if (nValue != table::CellHoriJustify_REPEAT)
    {
        if (IsXMLToken(rStrImpValue, XML_START))
        {
            nValue = table::CellHoriJustify_LEFT;
            rValue <<= nValue;
            bRetval = true;
        }
        else if (IsXMLToken(rStrImpValue, XML_END))
        {
            nValue = table::CellHoriJustify_RIGHT;
            rValue <<= nValue;
            bRetval = true;
        }
        else if (IsXMLToken(rStrImpValue, XML_CENTER))
        {
            nValue = table::CellHoriJustify_CENTER;
            rValue <<= nValue;
            bRetval = true;
        }
        else if (IsXMLToken(rStrImpValue, XML_JUSTIFY))
        {
            nValue = table::CellHoriJustify_BLOCK;
            rValue <<= nValue;
            bRetval = true;
        }
    }
    else
        bRetval = true;

    return bRetval;
}

bool XmlScPropHdl_HoriJustify::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    table::CellHoriJustify nVal;
    bool bRetval(false);

    if(rValue >>= nVal)
    {
        switch (nVal)
        {
            case table::CellHoriJustify_REPEAT:
            case table::CellHoriJustify_LEFT:
            {
                rStrExpValue = GetXMLToken(XML_START);
                bRetval = true;
            }
            break;
            case table::CellHoriJustify_RIGHT:
            {
                rStrExpValue = GetXMLToken(XML_END);
                bRetval = true;
            }
            break;
            case table::CellHoriJustify_CENTER:
            {
                rStrExpValue = GetXMLToken(XML_CENTER);
                bRetval = true;
            }
            break;
            case table::CellHoriJustify_BLOCK:
            {
                rStrExpValue = GetXMLToken(XML_JUSTIFY);
                bRetval = true;
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    return bRetval;
}

XmlScPropHdl_HoriJustifySource::~XmlScPropHdl_HoriJustifySource()
{
}

bool XmlScPropHdl_HoriJustifySource::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    table::CellHoriJustify aHoriJustify1, aHoriJustify2;

    if((r1 >>= aHoriJustify1) && (r2 >>= aHoriJustify2))
        return (aHoriJustify1 == aHoriJustify2);
    return false;
}

bool XmlScPropHdl_HoriJustifySource::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    if (IsXMLToken(rStrImpValue, XML_FIX))
    {
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_VALUE_TYPE))
    {
        rValue <<= table::CellHoriJustify_STANDARD;
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_HoriJustifySource::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    table::CellHoriJustify nVal;
    bool bRetval(false);

    if(rValue >>= nVal)
    {
        if (nVal == table::CellHoriJustify_STANDARD)
        {
            rStrExpValue = GetXMLToken(XML_VALUE_TYPE);
            bRetval = true;
        }
        else
        {
            rStrExpValue = GetXMLToken(XML_FIX);
            bRetval = true;
        }
    }

    return bRetval;
}

XmlScPropHdl_HoriJustifyRepeat::~XmlScPropHdl_HoriJustifyRepeat()
{
}

bool XmlScPropHdl_HoriJustifyRepeat::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    table::CellHoriJustify aHoriJustify1, aHoriJustify2;

    if((r1 >>= aHoriJustify1) && (r2 >>= aHoriJustify2))
        return (aHoriJustify1 == aHoriJustify2);
    return false;
}

bool XmlScPropHdl_HoriJustifyRepeat::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    if (IsXMLToken(rStrImpValue, XML_FALSE))
    {
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_TRUE))
    {
        rValue <<= table::CellHoriJustify_REPEAT;
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_HoriJustifyRepeat::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    table::CellHoriJustify nVal;
    bool bRetval(false);

    if(rValue >>= nVal)
    {
        if (nVal == table::CellHoriJustify_REPEAT)
        {
            rStrExpValue = GetXMLToken(XML_TRUE);
            bRetval = true;
        }
        else
        {
            rStrExpValue = GetXMLToken(XML_FALSE);
            bRetval = true;
        }
    }

    return bRetval;
}

XmlScPropHdl_Orientation::~XmlScPropHdl_Orientation()
{
}

bool XmlScPropHdl_Orientation::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    table::CellOrientation aOrientation1, aOrientation2;

    if((r1 >>= aOrientation1) && (r2 >>= aOrientation2))
        return (aOrientation1 == aOrientation2);
    return false;
}

bool XmlScPropHdl_Orientation::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    table::CellOrientation nValue;
    if (IsXMLToken(rStrImpValue, XML_LTR))
    {
        nValue = table::CellOrientation_STANDARD;
        rValue <<= nValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_TTB))
    {
        nValue = table::CellOrientation_STACKED;
        rValue <<= nValue;
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_Orientation::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    table::CellOrientation nVal;
    bool bRetval(false);

    if(rValue >>= nVal)
    {
        switch (nVal)
        {
            case table::CellOrientation_STACKED :
            {
                rStrExpValue = GetXMLToken(XML_TTB);
                bRetval = true;
            }
            break;
            default:
            {
                rStrExpValue = GetXMLToken(XML_LTR);
                bRetval = true;
            }
            break;
        }
    }

    return bRetval;
}

XmlScPropHdl_RotateAngle::~XmlScPropHdl_RotateAngle()
{
}

bool XmlScPropHdl_RotateAngle::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    sal_Int32 aAngle1 = 0, aAngle2 = 0;

    if((r1 >>= aAngle1) && (r2 >>= aAngle2))
        return (aAngle1 == aAngle2);
    return false;
}

bool XmlScPropHdl_RotateAngle::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    sal_Int32 nValue;
    if (::sax::Converter::convertNumber(nValue, rStrImpValue) && !o3tl::checked_multiply<sal_Int32>(nValue, 100, nValue))
    {
        rValue <<= nValue;
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_RotateAngle::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    sal_Int32 nVal = 0;
    bool bRetval(false);

    if(rValue >>= nVal)
    {
        rStrExpValue = OUString::number(nVal / 100);
        bRetval = true;
    }

    return bRetval;
}

XmlScPropHdl_RotateReference::~XmlScPropHdl_RotateReference()
{
}

bool XmlScPropHdl_RotateReference::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    sal_Int32 aReference1(0), aReference2(0);

    if((r1 >>= aReference1) && (r2 >>= aReference2))
        return (aReference1 == aReference2);
    return false;
}

bool XmlScPropHdl_RotateReference::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    sal_Int32 nValue;
    if (IsXMLToken(rStrImpValue, XML_NONE))
    {
        nValue = table::CellVertJustify2::STANDARD;
        rValue <<= nValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_BOTTOM))
    {
        nValue = table::CellVertJustify2::BOTTOM;
        rValue <<= nValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_TOP))
    {
        nValue = table::CellVertJustify2::TOP;
        rValue <<= nValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_CENTER))
    {
        nValue = table::CellVertJustify2::CENTER;
        rValue <<= nValue;
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_RotateReference::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    sal_Int32 nVal(0);
    bool bRetval(false);

    if(rValue >>= nVal)
    {
        switch (nVal)
        {
            case table::CellVertJustify2::BOTTOM :
            {
                rStrExpValue = GetXMLToken(XML_BOTTOM);
                bRetval = true;
            }
            break;
            case table::CellVertJustify2::CENTER :
            {
                rStrExpValue = GetXMLToken(XML_CENTER);
                bRetval = true;
            }
            break;
            case table::CellVertJustify2::STANDARD :
            {
                rStrExpValue = GetXMLToken(XML_NONE);
                bRetval = true;
            }
            break;
            case table::CellVertJustify2::TOP :
            {
                rStrExpValue = GetXMLToken(XML_TOP);
                bRetval = true;
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    return bRetval;
}

XmlScPropHdl_VertJustify::~XmlScPropHdl_VertJustify()
{
}

bool XmlScPropHdl_VertJustify::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    sal_Int32 aReference1(0), aReference2(0);

    if((r1 >>= aReference1) && (r2 >>= aReference2))
        return (aReference1 == aReference2);
    return false;
}

bool XmlScPropHdl_VertJustify::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    sal_Int32 nValue;
    if (IsXMLToken(rStrImpValue, XML_AUTOMATIC))
    {
        nValue = table::CellVertJustify2::STANDARD;
        rValue <<= nValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_BOTTOM))
    {
        nValue = table::CellVertJustify2::BOTTOM;
        rValue <<= nValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_TOP))
    {
        nValue = table::CellVertJustify2::TOP;
        rValue <<= nValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_MIDDLE))
    {
        nValue = table::CellVertJustify2::CENTER;
        rValue <<= nValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_JUSTIFY))
    {
        nValue = table::CellVertJustify2::BLOCK;
        rValue <<= nValue;
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_VertJustify::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    sal_Int32 nVal(0);
    bool bRetval(false);

    if(rValue >>= nVal)
    {
        switch (nVal)
        {
            case table::CellVertJustify2::BOTTOM :
            {
                rStrExpValue = GetXMLToken(XML_BOTTOM);
                bRetval = true;
            }
            break;
            case table::CellVertJustify2::CENTER :
            {
                rStrExpValue = GetXMLToken(XML_MIDDLE);
                bRetval = true;
            }
            break;
            case table::CellVertJustify2::STANDARD :
            {
                rStrExpValue = GetXMLToken(XML_AUTOMATIC);
                bRetval = true;
            }
            break;
            case table::CellVertJustify2::TOP :
            {
                rStrExpValue = GetXMLToken(XML_TOP);
                bRetval = true;
            }
            break;
            case table::CellVertJustify2::BLOCK :
            {
                rStrExpValue = GetXMLToken(XML_JUSTIFY);
                bRetval = true;
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    return bRetval;
}

XmlScPropHdl_BreakBefore::~XmlScPropHdl_BreakBefore()
{
}

bool XmlScPropHdl_BreakBefore::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    bool aBreak1 = false, aBreak2 = false;

    if((r1 >>= aBreak1) && (r2 >>= aBreak2))
        return (aBreak1 == aBreak2);
    return false;
}

bool XmlScPropHdl_BreakBefore::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    bool bValue;
    if (IsXMLToken(rStrImpValue, XML_AUTO))
    {
        bValue = false;
        rValue <<= bValue;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_PAGE))
    {
        bValue = true;
        rValue <<= bValue;
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_BreakBefore::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    if(::cppu::any2bool(rValue))
    {
        rStrExpValue = GetXMLToken(XML_PAGE);
        bRetval = true;
    }
    else
    {
        rStrExpValue = GetXMLToken(XML_AUTO);
        bRetval = true;
    }

    return bRetval;
}

XmlScPropHdl_IsTextWrapped::~XmlScPropHdl_IsTextWrapped()
{
}

bool XmlScPropHdl_IsTextWrapped::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    return (::cppu::any2bool(r1) == ::cppu::any2bool(r2));
}

bool XmlScPropHdl_IsTextWrapped::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    if (IsXMLToken(rStrImpValue, XML_WRAP))
    {
        rValue <<= true;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_NO_WRAP))
    {
        rValue <<= false;
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_IsTextWrapped::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    if (::cppu::any2bool(rValue))
    {
        rStrExpValue = GetXMLToken(XML_WRAP);
        bRetval = true;
    }
    else
    {
        rStrExpValue = GetXMLToken(XML_NO_WRAP);
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_IsEqual::importXML( const OUString& /* rStrImpValue */,
    css::uno::Any& /* rValue */,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    OSL_FAIL("should never be called");
    return false;
}

bool XmlScPropHdl_IsEqual::exportXML( OUString& /* rStrExpValue */,
    const css::uno::Any& /* rValue */,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    OSL_FAIL("should never be called");
    return false;
}

XmlScPropHdl_Vertical::~XmlScPropHdl_Vertical()
{
}

bool XmlScPropHdl_Vertical::equals(
    const css::uno::Any& r1,
    const css::uno::Any& r2 ) const
{
    return (::cppu::any2bool(r1) == ::cppu::any2bool(r2));
}

bool XmlScPropHdl_Vertical::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    if (IsXMLToken(rStrImpValue, XML_AUTO))
    {
        rValue <<= true;
        bRetval = true;
    }
    else if (IsXMLToken(rStrImpValue, XML_0))
    {
        rValue <<= false;
        bRetval = true;
    }

    return bRetval;
}

bool XmlScPropHdl_Vertical::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    bool bRetval(false);

    if (::cppu::any2bool(rValue))
    {
        rStrExpValue = GetXMLToken(XML_AUTO);
        bRetval = true;
    }
    else
    {
        rStrExpValue = GetXMLToken(XML_0);
        bRetval = true;
    }

    return bRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
