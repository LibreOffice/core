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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "xmlexprt.hxx"
#include "xmlimprt.hxx"

/*#ifndef _PROPIMP0_HXX
#endif*/
#include "XMLConverter.hxx"

#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_xmloff/xmluconv.hxx> 

#include <tools/debug.hxx>

#include <com/sun/star/util/CellProtection.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <comphelper/extract.hxx>

namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;

#define MAP(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context }
#define MAP_END()	{ NULL, 0, 0, XML_TOKEN_INVALID, 0 }

const XMLPropertyMapEntry aXMLScCellStylesProperties[] =
{
    MAP( "AsianVerticalMode", XML_NAMESPACE_STYLE, XML_GLYPH_ORIENTATION_VERTICAL, XML_SC_TYPE_VERTICAL, 0),
    MAP( "BottomBorder", XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_TYPE_BORDER, CTF_SC_BOTTOMBORDER ),
    MAP( "BottomBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_TYPE_BORDER_WIDTH, CTF_SC_BOTTOMBORDERWIDTH ),
    MAP( "CellBackColor", XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "CellProtection", XML_NAMESPACE_STYLE, XML_CELL_PROTECT, XML_SC_TYPE_CELLPROTECTION|MID_FLAG_MERGE_PROPERTY, 0 ),
    MAP( "CellProtection", XML_NAMESPACE_STYLE, XML_PRINT_CONTENT, XML_SC_TYPE_PRINTCONTENT|MID_FLAG_MERGE_PROPERTY, 0 ),
    MAP( "CellStyle", XML_NAMESPACE_STYLE, XML_STYLE, XML_TYPE_STRING, CTF_SC_CELLSTYLE ),
    MAP( "ConditionalFormat", XML_NAMESPACE_STYLE, XML_MAP, XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM, CTF_SC_IMPORT_MAP ),
    MAP( "ConditionalFormatXML", XML_NAMESPACE_STYLE, XML_MAP, XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM, CTF_SC_MAP ),
    MAP( "HoriJustify", XML_NAMESPACE_FO, XML_TEXT_ALIGN, XML_SC_TYPE_HORIJUSTIFY|MID_FLAG_MERGE_PROPERTY, 0 ),
    MAP( "HoriJustify", XML_NAMESPACE_STYLE, XML_TEXT_ALIGN_SOURCE, XML_SC_TYPE_HORIJUSTIFYSOURCE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MAP( "IsCellBackgroundTransparent", XML_NAMESPACE_FO, XML_BACKGROUND_COLOR, XML_TYPE_ISTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MAP( "IsTextWrapped", XML_NAMESPACE_FO, XML_WRAP_OPTION, XML_SC_ISTEXTWRAPPED, 0 ),
    MAP( "LeftBorder", XML_NAMESPACE_FO, XML_BORDER, XML_TYPE_BORDER, CTF_SC_ALLBORDER ),
    MAP( "LeftBorder", XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_TYPE_BORDER, CTF_SC_LEFTBORDER ),
    MAP( "LeftBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_TYPE_BORDER_WIDTH, CTF_SC_ALLBORDERWIDTH ),
    MAP( "LeftBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_TYPE_BORDER_WIDTH, CTF_SC_LEFTBORDERWIDTH ),
    MAP( "NumberFormat", XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME, XML_TYPE_NUMBER|MID_FLAG_SPECIAL_ITEM, CTF_SC_NUMBERFORMAT),
    MAP( "Orientation", XML_NAMESPACE_FO, XML_DIRECTION, XML_SC_TYPE_ORIENTATION, 0 ),
    MAP( "ParaBottomMargin", XML_NAMESPACE_FO, XML_PADDING, XML_TYPE_MEASURE, CTF_SC_ALLPADDING ),
    MAP( "ParaBottomMargin", XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_TYPE_MEASURE, CTF_SC_BOTTOMPADDING ),
    MAP( "ParaIndent", XML_NAMESPACE_FO, XML_MARGIN_LEFT, XML_TYPE_MEASURE16, 0 ),
    MAP( "ParaIsHyphenation", XML_NAMESPACE_FO, XML_HYPHENATE, XML_TYPE_BOOL, 0 ),
    MAP( "ParaLeftMargin", XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_TYPE_MEASURE, CTF_SC_LEFTPADDING ),
    MAP( "ParaRightMargin", XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_TYPE_MEASURE, CTF_SC_RIGHTPADDING ),
    MAP( "ParaTopMargin", XML_NAMESPACE_FO, XML_PADDING_TOP, XML_TYPE_MEASURE, CTF_SC_TOPPADDING ),
    MAP( "RightBorder", XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_TYPE_BORDER, CTF_SC_RIGHTBORDER ),
    MAP( "RightBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_TYPE_BORDER_WIDTH, CTF_SC_RIGHTBORDERWIDTH ),
    MAP( "RotateAngle", XML_NAMESPACE_STYLE, XML_ROTATION_ANGLE, XML_SC_TYPE_ROTATEANGLE, 0 ),
    MAP( "RotateReference", XML_NAMESPACE_STYLE, XML_ROTATION_ALIGN, XML_SC_TYPE_ROTATEREFERENCE, 0),
    MAP( "ShadowFormat", XML_NAMESPACE_STYLE, XML_SHADOW, XML_TYPE_TEXT_SHADOW, 0 ),
    MAP( "StandardDecimals", XML_NAMESPACE_STYLE, XML_DECIMAL_PLACES, XML_TYPE_NUMBER16, 0 ),
    MAP( "TopBorder", XML_NAMESPACE_FO, XML_BORDER_TOP, XML_TYPE_BORDER, CTF_SC_TOPBORDER ),
    MAP( "TopBorder", XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_TYPE_BORDER_WIDTH, CTF_SC_TOPBORDERWIDTH ),
    MAP( "UserDefinedAttributes", XML_NAMESPACE_TEXT, XML_XMLNS, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),
    MAP( "ValidationXML", XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION, XML_TYPE_BUILDIN_CMP_ONLY, CTF_SC_VALIDATION ),
    MAP( "VertJustify", XML_NAMESPACE_FO, XML_VERTICAL_ALIGN, XML_SC_TYPE_VERTJUSTIFY, 0),
    MAP( "WritingMode", XML_NAMESPACE_STYLE, XML_WRITING_MODE, XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT, 0 ),
    { 0L }
};

const XMLPropertyMapEntry aXMLScColumnStylesProperties[] =
{
    MAP( "IsManualPageBreak", XML_NAMESPACE_FO, XML_BREAK_BEFORE, XML_SC_TYPE_BREAKBEFORE, 0),
    MAP( "IsVisible", XML_NAMESPACE_TABLE, XML_DISPLAY, XML_SC_TYPE_EQUAL|MID_FLAG_SPECIAL_ITEM, CTF_SC_ISVISIBLE ),
    MAP( "Width", XML_NAMESPACE_STYLE, XML_COLUMN_WIDTH, XML_TYPE_MEASURE, 0 ),
//	MAP( "OptimalWidth", XML_NAMESPACE_STYLE, XML_USE_OPTIMAL_COLUMN_WIDTH, XML_TYPE_BOOL, 0),
    { 0L }
};

const XMLPropertyMapEntry aXMLScRowStylesProperties[] =
{
    MAP( "Height", XML_NAMESPACE_STYLE, XML_ROW_HEIGHT, XML_TYPE_MEASURE, CTF_SC_ROWHEIGHT),
    MAP( "IsManualPageBreak", XML_NAMESPACE_FO, XML_BREAK_BEFORE, XML_SC_TYPE_BREAKBEFORE, CTF_SC_ROWBREAKBEFORE),
    MAP( "OptimalHeight", XML_NAMESPACE_STYLE, XML_USE_OPTIMAL_ROW_HEIGHT, XML_TYPE_BOOL, CTF_SC_ROWOPTIMALHEIGHT),
    { 0L }
};

const XMLPropertyMapEntry aXMLScTableStylesProperties[] =
{
    MAP( "IsVisible", XML_NAMESPACE_TABLE, XML_DISPLAY, XML_TYPE_BOOL, 0 ),
    MAP( "PageStyle", XML_NAMESPACE_STYLE, XML_MASTER_PAGE_NAME, XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM, CTF_SC_MASTERPAGENAME ),
    { 0L }
};

ScXMLCellExportPropertyMapper::ScXMLCellExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper )
            : SvXMLExportPropertyMapper(rMapper)
{
}

ScXMLCellExportPropertyMapper::~ScXMLCellExportPropertyMapper()
{
}

void ScXMLCellExportPropertyMapper::ContextFilter(
    ::std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
    XMLPropertyState* pPadding = NULL;
    XMLPropertyState* pPadding_Bottom = NULL;
    XMLPropertyState* pPadding_Left = NULL;
    XMLPropertyState* pPadding_Right = NULL;
    XMLPropertyState* pPadding_Top = NULL;

    XMLPropertyState* pBorder = NULL;
    XMLPropertyState* pBorder_Bottom = NULL;
    XMLPropertyState* pBorder_Left = NULL;
    XMLPropertyState* pBorder_Right = NULL;
    XMLPropertyState* pBorder_Top = NULL;

    XMLPropertyState* pAllBorderWidthState = NULL;
    XMLPropertyState* pLeftBorderWidthState = NULL;
    XMLPropertyState* pRightBorderWidthState = NULL;
    XMLPropertyState* pTopBorderWidthState = NULL;
    XMLPropertyState* pBottomBorderWidthState = NULL;

    for( ::std::vector< XMLPropertyState >::iterator aIter = rProperties.begin();
         aIter != rProperties.end();
         ++aIter )
    {
        XMLPropertyState *propertie = &(*aIter);
        switch( getPropertySetMapper()->GetEntryContextId( propertie->mnIndex ) )
        {
            case CTF_SC_ALLPADDING:			pPadding = propertie; break;
            case CTF_SC_BOTTOMPADDING:		pPadding_Bottom = propertie; break;
            case CTF_SC_LEFTPADDING:		pPadding_Left = propertie; break;
            case CTF_SC_RIGHTPADDING:		pPadding_Right = propertie; break;
            case CTF_SC_TOPPADDING:			pPadding_Top = propertie; break;
            case CTF_SC_ALLBORDER:			pBorder = propertie; break;
            case CTF_SC_LEFTBORDER:			pBorder_Left = propertie; break;
            case CTF_SC_RIGHTBORDER:		pBorder_Right = propertie; break;
            case CTF_SC_BOTTOMBORDER:		pBorder_Bottom = propertie; break;
            case CTF_SC_TOPBORDER:			pBorder_Top = propertie; break;
            case CTF_SC_ALLBORDERWIDTH:		pAllBorderWidthState = propertie; break;
            case CTF_SC_LEFTBORDERWIDTH:	pLeftBorderWidthState = propertie; break;
            case CTF_SC_RIGHTBORDERWIDTH:	pRightBorderWidthState = propertie; break;
            case CTF_SC_TOPBORDERWIDTH:		pTopBorderWidthState = propertie; break;
            case CTF_SC_BOTTOMBORDERWIDTH:	pBottomBorderWidthState = propertie; break;
        }
    }

    if (pPadding && pPadding_Bottom && pPadding_Left && pPadding_Right && pPadding_Top)
    {
        sal_Int32 nBottom, nTop, nLeft, nRight;
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
            table::BorderLine aLeft, aRight, aTop, aBottom;

            pBorder_Left->maValue >>= aLeft;
            pBorder_Right->maValue >>= aRight;
            pBorder_Top->maValue >>= aTop;
            pBorder_Bottom->maValue >>= aBottom;
            if( aLeft.Color == aRight.Color && aLeft.InnerLineWidth == aRight.InnerLineWidth &&
                aLeft.OuterLineWidth == aRight.OuterLineWidth && aLeft.LineDistance == aRight.LineDistance &&
                aLeft.Color == aTop.Color && aLeft.InnerLineWidth == aTop.InnerLineWidth &&
                aLeft.OuterLineWidth == aTop.OuterLineWidth && aLeft.LineDistance == aTop.LineDistance &&
                aLeft.Color == aBottom.Color && aLeft.InnerLineWidth == aBottom.InnerLineWidth &&
                aLeft.OuterLineWidth == aBottom.OuterLineWidth && aLeft.LineDistance == aBottom.LineDistance )
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
            table::BorderLine aLeft, aRight, aTop, aBottom;

            pLeftBorderWidthState->maValue >>= aLeft;
            pRightBorderWidthState->maValue >>= aRight;
            pTopBorderWidthState->maValue >>= aTop;
            pBottomBorderWidthState->maValue >>= aBottom;
            if( aLeft.InnerLineWidth == aRight.InnerLineWidth && aLeft.OuterLineWidth == aRight.OuterLineWidth &&
                aLeft.LineDistance == aRight.LineDistance && aLeft.InnerLineWidth == aTop.InnerLineWidth &&
                aLeft.OuterLineWidth == aTop.OuterLineWidth && aLeft.LineDistance == aTop.LineDistance &&
                aLeft.InnerLineWidth == aBottom.InnerLineWidth && aLeft.OuterLineWidth == aBottom.OuterLineWidth &&
                aLeft.LineDistance == aBottom.LineDistance )
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
    SvXMLExportPropertyMapper::ContextFilter(rProperties, rPropSet);
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void ScXMLCellExportPropertyMapper::handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties,
            sal_uInt32 nIdx ) const
{
    // the SpecialItem NumberFormat must not be handled by this method
    // the SpecialItem ConditionlaFormat must not be handled by this method
    // the SpecialItem CharBackColor must not be handled by this method
}

ScXMLRowExportPropertyMapper::ScXMLRowExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper )
            : SvXMLExportPropertyMapper(rMapper)
{
}

ScXMLRowExportPropertyMapper::~ScXMLRowExportPropertyMapper()
{
}

void ScXMLRowExportPropertyMapper::ContextFilter(
    ::std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
    //#108550#; don't filter the height, so other applications know the calculated height

/*	XMLPropertyState* pHeight = NULL;
    XMLPropertyState* pOptimalHeight = NULL;

    for( ::std::vector< XMLPropertyState >::iterator propertie = rProperties.begin();
         propertie != rProperties.end();
         propertie++ )
    {
        switch( getPropertySetMapper()->GetEntryContextId( propertie->mnIndex ) )
        {
            case CTF_SC_ROWHEIGHT:				pHeight = propertie; break;
            case CTF_SC_ROWOPTIMALHEIGHT:		pOptimalHeight = propertie; break;
        }
    }
    if ((pHeight && pOptimalHeight && ::cppu::any2bool( pOptimalHeight->maValue )) ||
        (pHeight && !pOptimalHeight))
    {
        pHeight->mnIndex = -1;
        pHeight->maValue.clear();
    }
    if (pOptimalHeight)
    {
        pOptimalHeight->mnIndex = -1;
        pOptimalHeight->maValue.clear();
    }*/
}

ScXMLColumnExportPropertyMapper::ScXMLColumnExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper )
            : SvXMLExportPropertyMapper(rMapper)
{
}

ScXMLColumnExportPropertyMapper::~ScXMLColumnExportPropertyMapper()
{
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void ScXMLColumnExportPropertyMapper::handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties,
            sal_uInt32 nIdx ) const
{
    // the SpecialItem IsVisible must not be handled by this method
}

ScXMLTableExportPropertyMapper::ScXMLTableExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper )
            : SvXMLExportPropertyMapper(rMapper)
{
}

ScXMLTableExportPropertyMapper::~ScXMLTableExportPropertyMapper()
{
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void ScXMLTableExportPropertyMapper::handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties,
            sal_uInt32 nIdx ) const
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
        ::std::vector< XMLPropertyState >::const_iterator i = rProperties.begin();
        for (i; (i != rProperties.end()); i++)
        {
            UniReference< XMLPropertySetMapper > aPropMapper =
                rScXMLExport.GetCellStylesPropertySetMapper();
            sal_Int16 nContextID = aPropMapper->GetEntryContextId(i->mnIndex);
            switch (nContextID)
            {
                case CTF_SC_NUMBERFORMAT :
                {
                    sal_Int32 nNumberFormat;
                    if (i->maValue >>= nNumberFormat)
                    {
                        ::rtl::OUString sAttrValue = rScXMLExport.getDataStyleName(nNumberFormat);
                        if (sAttrValue.getLength())
                        {
                            GetExport().AddAttribute(
                                aPropMapper->GetEntryNameSpace(i->mnIndex),
                                aPropMapper->GetEntryXMLName(i->mnIndex),
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
        ::std::vector< XMLPropertyState >::const_iterator i = rProperties.begin();
        for (i; (i != rProperties.end()); i++)
        {
            UniReference< XMLPropertySetMapper > aPropMapper =
                rScXMLExport.GetTableStylesPropertySetMapper();
            sal_Int16 nContextID = aPropMapper->GetEntryContextId(i->mnIndex);
            switch (nContextID)
            {
                case CTF_SC_MASTERPAGENAME :
                {
                    ::rtl::OUString sName;
                    if (i->maValue >>= sName)
                    {
                        GetExport().AddAttribute(
                            aPropMapper->GetEntryNameSpace(i->mnIndex),
                            aPropMapper->GetEntryXMLName(i->mnIndex),
                            sName );
                    }
                }
                break;
            }
        }
    }
}

void ScXMLAutoStylePoolP::exportStyleContent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
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
        sal_Bool bNotFound = sal_True;
        ::std::vector< XMLPropertyState >::const_iterator i = rProperties.begin();
        for (i; (i != rProperties.end()) && bNotFound; i++)
        {
            sal_Int16 nContextID = rScXMLExport.GetCellStylesPropertySetMapper()->GetEntryContextId(i->mnIndex);
            switch (nContextID)
            {
                case CTF_SC_MAP :
                {
                    uno::Reference <sheet::XSheetConditionalEntries> xSheetConditionalEntries;
                    if (i->maValue >>= xSheetConditionalEntries)
                    {
                        uno::Reference<container::XIndexAccess> xIndex( xSheetConditionalEntries, uno::UNO_QUERY );
                        if ( xIndex.is() )
                        {
                            sal_Int32 nConditionCount = xIndex->getCount();
                            for (sal_Int32 nCondition = 0; nCondition < nConditionCount; nCondition++)
                            {
                                uno::Any aSheetConditionalEntry = xIndex->getByIndex(nCondition);
                                uno::Reference <sheet::XSheetConditionalEntry> xSheetConditionalEntry;
                                if (aSheetConditionalEntry >>= xSheetConditionalEntry)
                                {
                                    ::rtl::OUString sStyleName = xSheetConditionalEntry->getStyleName();
                                    uno::Reference <sheet::XSheetCondition> xSheetCondition(xSheetConditionalEntry, uno::UNO_QUERY);
                                    if (xSheetCondition.is())
                                    {
                                        sheet::ConditionOperator aOperator = xSheetCondition->getOperator();
                                        if (aOperator != sheet::ConditionOperator_NONE)
                                        {
                                            if (aOperator == sheet::ConditionOperator_FORMULA)
                                            {
                                                ::rtl::OUString sCondition(RTL_CONSTASCII_USTRINGPARAM("is-true-formula("));
                                                sCondition += xSheetCondition->getFormula1();
                                                sCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_CONDITION, sCondition);
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_APPLY_STYLE_NAME, sStyleName);
                                                OUString sOUBaseAddress;
                                                ScXMLConverter::GetStringFromAddress( sOUBaseAddress,
                                                    xSheetCondition->getSourcePosition(), rScXMLExport.GetDocument() );
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_BASE_CELL_ADDRESS, sOUBaseAddress);
                                                SvXMLElementExport aMElem(rScXMLExport, XML_NAMESPACE_STYLE, XML_MAP, sal_True, sal_True);
                                            }
                                            else
                                            {
                                                ::rtl::OUString sCondition;
                                                if (aOperator == sheet::ConditionOperator_BETWEEN ||
                                                    aOperator == sheet::ConditionOperator_NOT_BETWEEN)
                                                {
                                                    if (aOperator == sheet::ConditionOperator_BETWEEN)
                                                        sCondition = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-between("));
                                                    else
                                                        sCondition = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-not-between("));
                                                    sCondition += xSheetCondition->getFormula1();
                                                    sCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                                                    sCondition += xSheetCondition->getFormula2();
                                                    sCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
                                                }
                                                else
                                                {
                                                    sCondition = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content()"));
                                                    switch (aOperator)
                                                    {
                                                        case sheet::ConditionOperator_LESS:
                                                            sCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
                                                        break;
                                                        case sheet::ConditionOperator_GREATER:
                                                            sCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
                                                        break;
                                                        case sheet::ConditionOperator_LESS_EQUAL:
                                                            sCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
                                                        break;
                                                        case sheet::ConditionOperator_GREATER_EQUAL:
                                                            sCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
                                                        break;
                                                        case sheet::ConditionOperator_EQUAL:
                                                            sCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
                                                        break;
                                                        case sheet::ConditionOperator_NOT_EQUAL:
                                                            sCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
                                                        break;
                                                    }
                                                    sCondition += xSheetCondition->getFormula1();
                                                }
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_CONDITION, sCondition);
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_APPLY_STYLE_NAME, sStyleName);
                                                OUString sOUBaseAddress;
                                                ScXMLConverter::GetStringFromAddress( sOUBaseAddress,
                                                    xSheetCondition->getSourcePosition(), rScXMLExport.GetDocument() );
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, XML_BASE_CELL_ADDRESS, sOUBaseAddress);
                                                SvXMLElementExport aMElem(rScXMLExport, XML_NAMESPACE_STYLE, XML_MAP, sal_True, sal_True);
                                            }
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

ScXMLAutoStylePoolP::ScXMLAutoStylePoolP(ScXMLExport& rTempScXMLExport):
    SvXMLAutoStylePoolP(rTempScXMLExport),
    rScXMLExport(rTempScXMLExport)
{
}

ScXMLAutoStylePoolP::~ScXMLAutoStylePoolP()
{
}


void ScXMLStyleExport::exportStyleAttributes(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle )
{
    uno::Reference< beans::XPropertySet > xPropSet( rStyle, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
    ::rtl::OUString sNumberFormat(RTL_CONSTASCII_USTRINGPARAM("NumberFormat"));
    if( xPropSetInfo->hasPropertyByName( sNumberFormat ) )
    {
        uno::Reference< beans::XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
        if( xPropState.is() && (beans::PropertyState_DIRECT_VALUE ==
                xPropState->getPropertyState( sNumberFormat )) )
        {
            uno::Any aAny = xPropSet->getPropertyValue( sNumberFormat );
            sal_Int32 nNumberFormat;
            if (aAny >>= nNumberFormat)
            {
                ::rtl::OUString sDataStyleName = GetExport().getDataStyleName(nNumberFormat);
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME,
                                                    sDataStyleName );

            }
        }
    }
}

void ScXMLStyleExport::exportStyleContent(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle )
{
}

ScXMLStyleExport::ScXMLStyleExport(
        SvXMLExport& rExp,
        const ::rtl::OUString& rPoolStyleName,
        SvXMLAutoStylePoolP *pAutoStyleP )
    : XMLStyleExport(rExp, rPoolStyleName, pAutoStyleP)
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

    XMLPropertyHandler* pHdl = (XMLPropertyHandler*)XMLPropertyHandlerFactory::GetPropertyHandler( nType );
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

sal_Bool XmlScPropHdl_CellProtection::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    util::CellProtection aCellProtection1, aCellProtection2;

    if((r1 >>= aCellProtection1) && (r2 >>= aCellProtection2))
    {
        return ((aCellProtection1.IsHidden == aCellProtection2.IsHidden) &&
                (aCellProtection1.IsLocked == aCellProtection2.IsLocked) &&
                (aCellProtection1.IsFormulaHidden == aCellProtection2.IsFormulaHidden));
    }
    return sal_False;
}

sal_Bool XmlScPropHdl_CellProtection::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    util::CellProtection aCellProtection;
    sal_Bool bDefault = sal_False;
    if (!rValue.hasValue())
    {
        aCellProtection.IsHidden = sal_False;
        aCellProtection.IsLocked = sal_True;
        aCellProtection.IsFormulaHidden = sal_False;
        aCellProtection.IsPrintHidden = sal_False;
        bDefault = sal_True;
    }
    if ((rValue >>= aCellProtection) || bDefault)
    {
        if (!IsXMLToken(rStrImpValue, XML_NONE))
        {
            if (!IsXMLToken(rStrImpValue, XML_HIDDEN_AND_PROTECTED))
            {
                if (!IsXMLToken(rStrImpValue, XML_PROTECTED))
                {
                    if (!IsXMLToken(rStrImpValue, XML_FORMULA_HIDDEN))
                    {
                        sal_Int16 i = 0;
                        while (i < rStrImpValue.getLength() && rStrImpValue[i] != ' ')
                            i++;
                        ::rtl::OUString sFirst = rStrImpValue.copy(0, i);
                        ::rtl::OUString sSecond = rStrImpValue.copy(i + 1);
                        aCellProtection.IsFormulaHidden = sal_False;
                        aCellProtection.IsHidden = sal_False;
                        aCellProtection.IsLocked = sal_False;
                        if ((IsXMLToken(sFirst, XML_PROTECTED)) || (IsXMLToken(sSecond, XML_PROTECTED)))
                            aCellProtection.IsLocked = sal_True;
                        if ((IsXMLToken(sFirst, XML_FORMULA_HIDDEN)) || (IsXMLToken(sSecond, XML_FORMULA_HIDDEN)))
                            aCellProtection.IsFormulaHidden = sal_True;
                        rValue <<= aCellProtection;
                        bRetval = sal_True;
                    }
                    else
                    {
                        aCellProtection.IsFormulaHidden = sal_True;
                        aCellProtection.IsHidden = sal_False;
                        aCellProtection.IsLocked = sal_False;
                        rValue <<= aCellProtection;
                        bRetval = sal_True;
                    }
                }
                else
                {
                    aCellProtection.IsFormulaHidden = sal_False;
                    aCellProtection.IsHidden = sal_False;
                    aCellProtection.IsLocked = sal_True;
                    rValue <<= aCellProtection;
                    bRetval = sal_True;
                }
            }
            else
            {
                aCellProtection.IsFormulaHidden = sal_True;
                aCellProtection.IsHidden = sal_True;
                aCellProtection.IsLocked = sal_True;
                rValue <<= aCellProtection;
                bRetval = sal_True;
            }
        }
        else
        {
            aCellProtection.IsFormulaHidden = sal_False;
            aCellProtection.IsHidden = sal_False;
            aCellProtection.IsLocked = sal_False;
            rValue <<= aCellProtection;
            bRetval = sal_True;
        }
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_CellProtection::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);
    util::CellProtection aCellProtection;

    if(rValue >>= aCellProtection)
    {
        if (!(aCellProtection.IsFormulaHidden || aCellProtection.IsHidden || aCellProtection.IsLocked))
        {
            rStrExpValue = GetXMLToken(XML_NONE);
            bRetval = sal_True;
        }
        else if (aCellProtection.IsHidden && aCellProtection.IsLocked)
        {
            rStrExpValue = GetXMLToken(XML_HIDDEN_AND_PROTECTED);
            bRetval = sal_True;
        }
        else if (aCellProtection.IsLocked && !(aCellProtection.IsFormulaHidden || aCellProtection.IsHidden))
        {
            rStrExpValue = GetXMLToken(XML_PROTECTED);
            bRetval = sal_True;
        }
        else if (aCellProtection.IsFormulaHidden && !(aCellProtection.IsLocked || aCellProtection.IsHidden))
        {
            rStrExpValue = GetXMLToken(XML_FORMULA_HIDDEN);
            bRetval = sal_True;
        }
        else if (aCellProtection.IsFormulaHidden && aCellProtection.IsLocked)
        {
            rStrExpValue = GetXMLToken(XML_PROTECTED);
            rStrExpValue += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
            rStrExpValue += GetXMLToken(XML_FORMULA_HIDDEN);
            bRetval = sal_True;
        }
    }

    return bRetval;
}

XmlScPropHdl_PrintContent::~XmlScPropHdl_PrintContent()
{
}

sal_Bool XmlScPropHdl_PrintContent::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    util::CellProtection aCellProtection1, aCellProtection2;

    if((r1 >>= aCellProtection1) && (r2 >>= aCellProtection2))
    {
        return (aCellProtection1.IsPrintHidden == aCellProtection2.IsPrintHidden);
    }
    return sal_False;
}

sal_Bool XmlScPropHdl_PrintContent::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);
    util::CellProtection aCellProtection;
    sal_Bool bDefault = sal_False;
    if (!rValue.hasValue())
    {
        aCellProtection.IsHidden = sal_False;
        aCellProtection.IsLocked = sal_True;
        aCellProtection.IsFormulaHidden = sal_False;
        aCellProtection.IsPrintHidden = sal_False;
        bDefault = sal_True;
    }
    if ((rValue >>= aCellProtection) || bDefault)
    {
        sal_Bool bValue;
        if (rUnitConverter.convertBool(bValue, rStrImpValue))
        {
            aCellProtection.IsPrintHidden = !bValue;
            rValue <<= aCellProtection;
            bRetval = sal_True;
        }
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_PrintContent::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    util::CellProtection aCellProtection;
    if(rValue >>= aCellProtection)
    {
        ::rtl::OUStringBuffer sValue;
        rUnitConverter.convertBool(sValue, !aCellProtection.IsPrintHidden);
        rStrExpValue = sValue.makeStringAndClear();
        bRetval = sal_True;
    }

    return bRetval;
}

XmlScPropHdl_HoriJustify::~XmlScPropHdl_HoriJustify()
{
}

sal_Bool XmlScPropHdl_HoriJustify::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    table::CellHoriJustify aHoriJustify1, aHoriJustify2;

    if((r1 >>= aHoriJustify1) && (r2 >>= aHoriJustify2))
        return (aHoriJustify1 == aHoriJustify2);
    return sal_False;
}

sal_Bool XmlScPropHdl_HoriJustify::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    table::CellHoriJustify nValue;
    if (IsXMLToken(rStrImpValue, XML_START))
    {
         nValue = table::CellHoriJustify_LEFT;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_END))
    {
         nValue = table::CellHoriJustify_RIGHT;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_CENTER))
    {
         nValue = table::CellHoriJustify_CENTER;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_JUSTIFY))
    {
         nValue = table::CellHoriJustify_BLOCK;
        rValue <<= nValue;
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_HoriJustify::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    table::CellHoriJustify nVal;
    sal_Bool bRetval(sal_False);

    if(rValue >>= nVal)
    {
        switch (nVal)
        {
            case table::CellHoriJustify_REPEAT:
            case table::CellHoriJustify_LEFT:
            {
                rStrExpValue = GetXMLToken(XML_START);
                bRetval = sal_True;
            }
            break;
            case table::CellHoriJustify_RIGHT:
            {
                rStrExpValue = GetXMLToken(XML_END);
                bRetval = sal_True;
            }
            break;
            case table::CellHoriJustify_CENTER:
            {
                rStrExpValue = GetXMLToken(XML_CENTER);
                bRetval = sal_True;
            }
            break;
            case table::CellHoriJustify_BLOCK:
            {
                rStrExpValue = GetXMLToken(XML_JUSTIFY);
                bRetval = sal_True;
            }
            break;
        }
    }

    return bRetval;
}

XmlScPropHdl_HoriJustifySource::~XmlScPropHdl_HoriJustifySource()
{
}

sal_Bool XmlScPropHdl_HoriJustifySource::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    table::CellHoriJustify aHoriJustify1, aHoriJustify2;

    if((r1 >>= aHoriJustify1) && (r2 >>= aHoriJustify2))
        return (aHoriJustify1 == aHoriJustify2);
    return sal_False;
}

sal_Bool XmlScPropHdl_HoriJustifySource::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    if (IsXMLToken(rStrImpValue, XML_FIX))
    {
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_VALUE_TYPE))
    {
        table::CellHoriJustify nValue = table::CellHoriJustify_STANDARD;
        rValue <<= nValue;
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_HoriJustifySource::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    table::CellHoriJustify nVal;
    sal_Bool bRetval(sal_False);

    if(rValue >>= nVal)
    {
        if (nVal == table::CellHoriJustify_STANDARD)
        {
            rStrExpValue = GetXMLToken(XML_VALUE_TYPE);
            bRetval = sal_True;
        }
        else
        {
            rStrExpValue = GetXMLToken(XML_FIX);
            bRetval = sal_True;
        }
    }

    return bRetval;
}

XmlScPropHdl_Orientation::~XmlScPropHdl_Orientation()
{
}

sal_Bool XmlScPropHdl_Orientation::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    table::CellOrientation aOrientation1, aOrientation2;

    if((r1 >>= aOrientation1) && (r2 >>= aOrientation2))
        return (aOrientation1 == aOrientation2);
    return sal_False;
}

sal_Bool XmlScPropHdl_Orientation::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    table::CellOrientation nValue;
    if (IsXMLToken(rStrImpValue, XML_LTR))
    {
        nValue = table::CellOrientation_STANDARD;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_TTB))
    {
        nValue = table::CellOrientation_STACKED;
        rValue <<= nValue;
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_Orientation::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    table::CellOrientation nVal;
    sal_Bool bRetval(sal_False);

    if(rValue >>= nVal)
    {
        switch (nVal)
        {
            case table::CellOrientation_STACKED :
            {
                rStrExpValue = GetXMLToken(XML_TTB);
                bRetval = sal_True;
            }
            break;
            default:
            {
                rStrExpValue = GetXMLToken(XML_LTR);
                bRetval = sal_True;
            }
            break;
        }
    }

    return bRetval;
}

XmlScPropHdl_RotateAngle::~XmlScPropHdl_RotateAngle()
{
}

sal_Bool XmlScPropHdl_RotateAngle::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Int32 aAngle1, aAngle2;

    if((r1 >>= aAngle1) && (r2 >>= aAngle2))
        return (aAngle1 == aAngle2);
    return sal_False;
}

sal_Bool XmlScPropHdl_RotateAngle::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    sal_Int32 nValue;
    if (rUnitConverter.convertNumber(nValue, rStrImpValue))
    {
        nValue *= 100;
        rValue <<= nValue;
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_RotateAngle::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int32 nVal;
    sal_Bool bRetval(sal_False);

    if(rValue >>= nVal)
    {
        ::rtl::OUStringBuffer sValue;
        rUnitConverter.convertNumber(sValue, sal_Int32(nVal / 100));
        rStrExpValue = sValue.makeStringAndClear();
        bRetval = sal_True;
    }

    return bRetval;
}

XmlScPropHdl_RotateReference::~XmlScPropHdl_RotateReference()
{
}

sal_Bool XmlScPropHdl_RotateReference::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    table::CellVertJustify aReference1, aReference2;

    if((r1 >>= aReference1) && (r2 >>= aReference2))
        return (aReference1 == aReference2);
    return sal_False;
}

sal_Bool XmlScPropHdl_RotateReference::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    table::CellVertJustify nValue;
    if (IsXMLToken(rStrImpValue, XML_NONE))
    {
        nValue = table::CellVertJustify_STANDARD;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_BOTTOM))
    {
        nValue = table::CellVertJustify_BOTTOM;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_TOP))
    {
        nValue = table::CellVertJustify_TOP;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_CENTER))
    {
        nValue = table::CellVertJustify_CENTER;
        rValue <<= nValue;
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_RotateReference::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    table::CellVertJustify nVal;
    sal_Bool bRetval(sal_False);

    if(rValue >>= nVal)
    {
        switch (nVal)
        {
            case table::CellVertJustify_BOTTOM :
            {
                rStrExpValue = GetXMLToken(XML_BOTTOM);
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_CENTER :
            {
                rStrExpValue = GetXMLToken(XML_CENTER);
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_STANDARD :
            {
                rStrExpValue = GetXMLToken(XML_NONE);
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_TOP :
            {
                rStrExpValue = GetXMLToken(XML_TOP);
                bRetval = sal_True;
            }
            break;
        }
    }

    return bRetval;
}

XmlScPropHdl_VertJustify::~XmlScPropHdl_VertJustify()
{
}

sal_Bool XmlScPropHdl_VertJustify::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    table::CellVertJustify aReference1, aReference2;

    if((r1 >>= aReference1) && (r2 >>= aReference2))
        return (aReference1 == aReference2);
    return sal_False;
}

sal_Bool XmlScPropHdl_VertJustify::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    table::CellVertJustify nValue;
    if (IsXMLToken(rStrImpValue, XML_AUTOMATIC))
    {
        nValue = table::CellVertJustify_STANDARD;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_BOTTOM))
    {
        nValue = table::CellVertJustify_BOTTOM;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_TOP))
    {
        nValue = table::CellVertJustify_TOP;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_MIDDLE))
    {
        nValue = table::CellVertJustify_CENTER;
        rValue <<= nValue;
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_VertJustify::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    table::CellVertJustify nVal;
    sal_Bool bRetval(sal_False);

    if(rValue >>= nVal)
    {
        switch (nVal)
        {
            case table::CellVertJustify_BOTTOM :
            {
                rStrExpValue = GetXMLToken(XML_BOTTOM);
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_CENTER :
            {
                rStrExpValue = GetXMLToken(XML_MIDDLE);
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_STANDARD :
            {
                rStrExpValue = GetXMLToken(XML_AUTOMATIC);
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_TOP :
            {
                rStrExpValue = GetXMLToken(XML_TOP);
                bRetval = sal_True;
            }
            break;
        }
    }

    return bRetval;
}

XmlScPropHdl_BreakBefore::~XmlScPropHdl_BreakBefore()
{
}

sal_Bool XmlScPropHdl_BreakBefore::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool aBreak1, aBreak2;

    if((r1 >>= aBreak1) && (r2 >>= aBreak2))
        return (aBreak1 == aBreak2);
    return sal_False;
}

sal_Bool XmlScPropHdl_BreakBefore::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    sal_Bool bValue;
    if (IsXMLToken(rStrImpValue, XML_AUTO))
    {
        bValue = sal_False;
        rValue = ::cppu::bool2any(bValue);
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_PAGE))
    {
        bValue = sal_True;
        rValue = ::cppu::bool2any(bValue);
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_BreakBefore::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bVal;
    sal_Bool bRetval(sal_False);

    if(rValue >>= bVal)
    {
        if (bVal)
        {
            rStrExpValue = GetXMLToken(XML_PAGE);
            bRetval = sal_True;
        }
        else
        {
            rStrExpValue = GetXMLToken(XML_AUTO);
            bRetval = sal_True;
        }
    }

    return bRetval;
}

XmlScPropHdl_IsTextWrapped::~XmlScPropHdl_IsTextWrapped()
{
}

sal_Bool XmlScPropHdl_IsTextWrapped::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    return (::cppu::any2bool(r1) == ::cppu::any2bool(r2));
}

sal_Bool XmlScPropHdl_IsTextWrapped::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    if (IsXMLToken(rStrImpValue, XML_WRAP))
    {
        rValue = ::cppu::bool2any(sal_True);
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_NO_WRAP))
    {
        rValue = ::cppu::bool2any(sal_False);
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_IsTextWrapped::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    if (::cppu::any2bool(rValue))
    {
        rStrExpValue = GetXMLToken(XML_WRAP);
        bRetval = sal_True;
    }
    else
    {
        rStrExpValue = GetXMLToken(XML_NO_WRAP);
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_IsEqual::importXML( const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ERROR("should never be called");
    return sal_False;
}

sal_Bool XmlScPropHdl_IsEqual::exportXML( ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ERROR("should never be called");
    return sal_False;
}

XmlScPropHdl_Vertical::~XmlScPropHdl_Vertical()
{
}

sal_Bool XmlScPropHdl_Vertical::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    return (::cppu::any2bool(r1) == ::cppu::any2bool(r2));
}

sal_Bool XmlScPropHdl_Vertical::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    if (IsXMLToken(rStrImpValue, XML_AUTO))
    {
        rValue = ::cppu::bool2any(sal_True);
        bRetval = sal_True;
    }
    else if (IsXMLToken(rStrImpValue, XML_0))
    {
        rValue = ::cppu::bool2any(sal_False);
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_Vertical::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    if (::cppu::any2bool(rValue))
    {
        rStrExpValue = GetXMLToken(XML_AUTO);
        bRetval = sal_True;
    }
    else
    {
        rStrExpValue = GetXMLToken(XML_0);
        bRetval = sal_True;
    }

    return bRetval;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
