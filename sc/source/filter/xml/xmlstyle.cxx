/*************************************************************************
 *
 *  $RCSfile: xmlstyle.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: sab $ $Date: 2000-10-20 06:24:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif
#include "xmlexprt.hxx"
#include "xmlimprt.hxx"

/*#ifndef _PROPIMP0_HXX
#include "propimp0.hxx"
#endif*/

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlkywd.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/families.hxx>
#ifndef _XMLOFF_XMLNUMFE_HXX
#include <xmloff/xmlnumfe.hxx>
#endif
#ifndef _XMLOFF_XMLNUMFI_HXX
#include <xmloff/xmlnumfi.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_CELLPROTECTION_HPP_
#include <com/sun/star/util/CellProtection.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLORIENTATION_HPP_
#include <com/sun/star/table/CellOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLVERTJUSTIFY_HPP_
#include <com/sun/star/table/CellVertJustify.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLHORIJUSTIFY_HPP_
#include <com/sun/star/table/CellHoriJustify.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEBORDER_HPP_
#include <com/sun/star/table/TableBorder.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCONDITIONALENTRIES_HPP_
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCONDITIONALENTRY_HPP_
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCONDITION_HPP_
#include <com/sun/star/sheet/XSheetCondition.hpp>
#endif

#include <rtl/ustrbuf.hxx>

#ifdef _USE_NAMESPACE
using namespace com::sun::star;
#endif

const XMLPropertyMapEntry aXMLScCellStylesProperties[] =
{
    { "CellBackColor", XML_NAMESPACE_FO, sXML_background_color, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 },
    { "CellProtection", XML_NAMESPACE_STYLE, sXML_cell_protect, XML_SC_TYPE_CELLPROTECTION|MID_FLAG_MERGE_PROPERTY, 0 },
    { "CellProtection", XML_NAMESPACE_STYLE, sXML_print_content, XML_SC_TYPE_PRINTCONTENT|MID_FLAG_MERGE_PROPERTY, 0 },
    { "NumberFormat", XML_NAMESPACE_STYLE, sXML_data_style_name, XML_TYPE_NUMBER|MID_FLAG_SPECIAL_ITEM, CTF_NUMBERFORMAT},
    { "HoriJustify", XML_NAMESPACE_FO, sXML_text_align, XML_SC_TYPE_HORIJUSTIFY|MID_FLAG_MERGE_PROPERTY, 0 },
    { "HoriJustify", XML_NAMESPACE_STYLE, sXML_text_align_source, XML_SC_TYPE_HORIJUSTIFYSOURCE|MID_FLAG_MERGE_PROPERTY, 0 },
//  { "HoriJustify", XML_NAMESPACE_FO, sXML_text_align, XML_SC_TYPE_HORIJUSTIFY|MID_FLAG_MULTI_PROPERTY, CTF_HORIJUSTIFY },
//  { "HoriJustify", XML_NAMESPACE_STYLE, sXML_text_align_source, XML_SC_TYPE_HORIJUSTIFYSOURCE|MID_FLAG_MULTI_PROPERTY, CTF_HORIJUSTIFY_SOURCE },
    { "IsCellBackgroundTransparent", XML_NAMESPACE_FO, sXML_background_color, XML_TYPE_ISTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 },
    { "IsTextWrapped", XML_NAMESPACE_FO, sXML_wrap_option, XML_SC_ISTEXTWRAPPED, 0 },
    { "Orientation", XML_NAMESPACE_FO, sXML_direction, XML_SC_TYPE_ORIENTATION, 0 },
    { "ParaIndent", XML_NAMESPACE_FO, sXML_margin_left, XML_TYPE_MEASURE16, 0 },
    { "ParaBottomMargin", XML_NAMESPACE_FO, sXML_padding, XML_TYPE_MEASURE, CTF_ALLPADDING },
    { "ParaBottomMargin", XML_NAMESPACE_FO, sXML_padding_bottom, XML_TYPE_MEASURE, CTF_BOTTOMPADDING },
    { "ParaLeftMargin", XML_NAMESPACE_FO, sXML_padding_left, XML_TYPE_MEASURE, CTF_LEFTPADDING },
    { "ParaRightMargin", XML_NAMESPACE_FO, sXML_padding_right, XML_TYPE_MEASURE, CTF_RIGHTPADDING },
    { "ParaTopMargin", XML_NAMESPACE_FO, sXML_padding_top, XML_TYPE_MEASURE, CTF_TOPPADDING },
    { "RotateAngle", XML_NAMESPACE_STYLE, sXML_rotation_angle, XML_SC_TYPE_ROTATEANGLE, 0 },
    { "RotateReference", XML_NAMESPACE_STYLE, sXML_rotation_align, XML_SC_TYPE_ROTATEREFERENCE, 0},
    { "ShadowFormat", XML_NAMESPACE_STYLE, sXML_shadow, XML_TYPE_TEXT_SHADOW, 0 },
    { "ConditionalFormat", XML_NAMESPACE_STYLE, sXML_map, XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM, CTF_MAP },
    { "LeftBorder", XML_NAMESPACE_FO, sXML_border, XML_TYPE_BORDER, CTF_ALLBORDER },
    { "LeftBorder", XML_NAMESPACE_FO, sXML_border_left, XML_TYPE_BORDER, CTF_LEFTBORDER },
    { "RightBorder", XML_NAMESPACE_FO, sXML_border_right, XML_TYPE_BORDER, CTF_RIGHTBORDER },
    { "TopBorder", XML_NAMESPACE_FO, sXML_border_top, XML_TYPE_BORDER, CTF_TOPBORDER },
    { "BottomBorder", XML_NAMESPACE_FO, sXML_border_bottom, XML_TYPE_BORDER, CTF_BOTTOMBORDER },
    { "LeftBorder", XML_NAMESPACE_STYLE, sXML_border_line_width, XML_TYPE_BORDER_WIDTH, CTF_ALLBORDERWIDTH },
    { "LeftBorder", XML_NAMESPACE_STYLE, sXML_border_line_width_left, XML_TYPE_BORDER_WIDTH, CTF_LEFTBORDERWIDTH },
    { "RightBorder", XML_NAMESPACE_STYLE, sXML_border_line_width_right, XML_TYPE_BORDER_WIDTH, CTF_RIGHTBORDERWIDTH },
    { "TopBorder", XML_NAMESPACE_STYLE, sXML_border_line_width_top, XML_TYPE_BORDER_WIDTH, CTF_TOPBORDERWIDTH },
    { "BottomBorder", XML_NAMESPACE_STYLE, sXML_border_line_width_bottom, XML_TYPE_BORDER_WIDTH, CTF_BOTTOMBORDERWIDTH },
    { "VertJustify", XML_NAMESPACE_FO, sXML_vertical_align, XML_SC_TYPE_VERTJUSTIFY, 0},

// CharacterProperties
    { "CharAutoKerning", XML_NAMESPACE_STYLE, sXML_letter_kerning, XML_TYPE_BOOL, 0 },
    { "CharBackColor", XML_NAMESPACE_FO, sXML_text_background_color, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 },
    { "CharCaseMap", XML_NAMESPACE_FO, sXML_font_variant, XML_TYPE_TEXT_CASEMAP_VAR, 0 },
    { "CharCaseMap", XML_NAMESPACE_FO, sXML_text_transform, XML_TYPE_TEXT_CASEMAP, 0 },
    { "CharColor", XML_NAMESPACE_FO, sXML_color, XML_TYPE_COLOR, 0 },
    { "CharContoured", XML_NAMESPACE_STYLE, sXML_text_outline, XML_TYPE_BOOL, 0 },
    { "CharCrossedOut", XML_NAMESPACE_STYLE, sXML_text_crossing_out, XML_TYPE_TEXT_BOOLCROSSEDOUT, 0 },
//  { "CharEscapement", XML_NAMESPACE_STYLE, sXML_text_position, XML_TYPE_TEXT_ESCAPEMENT|MID_FLAG_MERGE_ATTRIBUTE, 0 },
//  { "CharEscapementHeight", XML_NAMESPACE_STYLE, sXML_text_position, XML_TYPE_TEXT_ESCAPEMENT_HEIGHT|MID_FLAG_MERGE_ATTRIBUTE, 0 },
    { "CharFlash", XML_NAMESPACE_STYLE, sXML_text_blinking, XML_TYPE_BOOL, 0},
    { "CharFontName",XML_NAMESPACE_FO, sXML_font_family, XML_TYPE_TEXT_FONTFAMILYNAME, 0 },
//  { "CharFontType", ...},
    { "CharFontStyleName", XML_NAMESPACE_STYLE, sXML_font_style_name, XML_TYPE_STRING, 0 },
    { "CharFontFamily", XML_NAMESPACE_STYLE, sXML_font_family_generic, XML_TYPE_TEXT_FONTFAMILY, 0 },
    { "CharFontPitch", XML_NAMESPACE_STYLE, sXML_font_pitch, XML_TYPE_TEXT_FONTPITCH, 0 },
    { "CharFontCharSet", XML_NAMESPACE_STYLE, sXML_font_charset, XML_TYPE_TEXT_FONTENCODING, 0 },
    { "CharHeight", XML_NAMESPACE_FO, sXML_font_size, XML_TYPE_CHAR_HEIGHT, 0 },
    { "CharKeepTogether", XML_NAMESPACE_FO, sXML_keep_with_next, XML_TYPE_BOOL, 0 },
    { "CharKerning", XML_NAMESPACE_FO, sXML_letter_spacing, XML_TYPE_TEXT_KERNING, 0 },
    { "CharLocale", XML_NAMESPACE_FO, sXML_language, XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 },
    { "CharLocale", XML_NAMESPACE_FO, sXML_country, XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 },
//  { "CharNoLineBreak, ...},
    { "CharPosture",XML_NAMESPACE_FO, sXML_font_style, XML_TYPE_TEXT_POSTURE, 0 },
    { "CharShadowed",XML_NAMESPACE_FO, sXML_text_shadow, XML_TYPE_TEXT_SHADOWED, 0 },
//  { "CharStyle", ... },
//  { "CharBackTransparent", ... },
    { "CharBackTransparent", XML_NAMESPACE_FO, sXML_text_background_color, XML_TYPE_ISTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 },
    { "CharUnderline", XML_NAMESPACE_STYLE, sXML_text_underline, XML_TYPE_TEXT_UNDERLINE, 0 },
    { "CharWeight", XML_NAMESPACE_FO, sXML_font_weight, XML_TYPE_TEXT_WEIGHT, 0 },
    { "CharWordMode", XML_NAMESPACE_STYLE, sXML_score_spaces, XML_TYPE_NBOOL, 0 },
    { 0L }
};

const XMLPropertyMapEntry aXMLScColumnStylesProperties[] =
{
    { "Width", XML_NAMESPACE_STYLE, sXML_column_width, XML_TYPE_MEASURE, 0 },
    { "IsManualPageBreak", XML_NAMESPACE_FO, sXML_break_before, XML_SC_TYPE_BREAKBEFORE, 0},
//  { "OptimalWidth", XML_NAMESPACE_STYLE, sXML_use_optimal_column_width, XML_TYPE_BOOL, 0},
    { 0L }
};

const XMLPropertyMapEntry aXMLScRowStylesProperties[] =
{
    { "Height", XML_NAMESPACE_STYLE, sXML_row_height, XML_TYPE_MEASURE, 0 },
    { "IsManualPageBreak", XML_NAMESPACE_FO, sXML_break_before, XML_SC_TYPE_BREAKBEFORE, 0},
    { "OptimalHeight", XML_NAMESPACE_STYLE, sXML_use_optimal_row_height, XML_TYPE_BOOL, 0},
    { 0L }
};

const XMLPropertyMapEntry aXMLScTableStylesProperties[] =
{
    { "IsVisible", XML_NAMESPACE_TABLE, sXML_display, XML_TYPE_BOOL, 0 },
    { "PageStyle", XML_NAMESPACE_TABLE, sXML_page_style_name, XML_TYPE_STRING, 0 },
    { 0L }
};

ScXMLExportPropertyMapper::ScXMLExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper )
            : SvXMLExportPropertyMapper(rMapper)
{
}
ScXMLExportPropertyMapper::~ScXMLExportPropertyMapper()
{
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void ScXMLExportPropertyMapper::handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties,
            sal_uInt32 nIdx ) const
{
    // the SpecialItem NumberFormat must not be handled by this method
    // the SpecialItem ConditionlaFormat must not be handled by this method
}

void ScXMLAutoStylePoolP::exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap) const
{
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties, rPropExp, rUnitConverter, rNamespaceMap );
    if (nFamily == XML_STYLE_FAMILY_TABLE_CELL)
    {
        ::std::vector< XMLPropertyState >::const_iterator i = rProperties.begin();
        for (i; (i != rProperties.end()); i++)
        {
            UniReference< XMLCellStylesPropertySetMapper > aPropMapper =
                rScXMLExport.GetCellStylesPropertySetMapper();
            sal_Int16 nContextID = aPropMapper->GetEntryContextId(i->mnIndex);
            switch (nContextID)
            {
                case CTF_NUMBERFORMAT :
                {
                    sal_Int32 nNumberFormat;
                    if (i->maValue >>= nNumberFormat)
                    {
                        rtl::OUString sAttrValue = rScXMLExport.getDataStyleName(nNumberFormat);
                        if (sAttrValue.len())
                        {
                            rtl::OUString sAttrName( rNamespaceMap.GetQNameByKey(
                                aPropMapper->GetEntryNameSpace(i->mnIndex), aPropMapper->GetEntryXMLName(i->mnIndex) ) );
                            rtl::OUString sCDATA( RTL_CONSTASCII_USTRINGPARAM(sXML_CDATA ) );
                            rAttrList.AddAttribute( sAttrName, sCDATA, sAttrValue );
                        }
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
        const SvXMLExportPropertyMapper& rPropExp,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
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
                case CTF_MAP :
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
                                    rtl::OUString sStyleName = xSheetConditionalEntry->getStyleName();
                                    uno::Reference <sheet::XSheetCondition> xSheetCondition(xSheetConditionalEntry, uno::UNO_QUERY);
                                    if (xSheetCondition.is())
                                    {
                                        sheet::ConditionOperator aOperator = xSheetCondition->getOperator();
                                        if (aOperator != sheet::ConditionOperator_NONE)
                                        {
                                            if (aOperator == sheet::ConditionOperator_FORMULA)
                                            {
                                                rtl::OUString sCondition(RTL_CONSTASCII_USTRINGPARAM("is-true-formula("));
                                                sCondition += xSheetCondition->getFormula1();
                                                sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_condition, sCondition);
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_apply_style_name, sStyleName);
                                                table::CellAddress aBase = xSheetCondition->getSourcePosition();
                                                ScAddress aBaseAddress(aBase.Column, aBase.Row, aBase.Sheet);
                                                String sBaseAddress;
                                                aBaseAddress.Format(sBaseAddress, SCA_VALID | SCA_TAB_3D, rScXMLExport.GetDocument());
                                                rtl::OUString sOUBaseAddress(sBaseAddress);
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_base_cell_address, sOUBaseAddress);
                                                SvXMLElementExport aMElem(rScXMLExport, XML_NAMESPACE_STYLE, sXML_map, sal_True, sal_True);
                                            }
                                            else
                                            {
                                                rtl::OUString sCondition;
                                                if (aOperator == sheet::ConditionOperator_BETWEEN ||
                                                    aOperator == sheet::ConditionOperator_NOT_BETWEEN)
                                                {
                                                    if (aOperator == sheet::ConditionOperator_BETWEEN)
                                                        sCondition = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-between("));
                                                    else
                                                        sCondition = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-not-between("));
                                                    sCondition += xSheetCondition->getFormula1();
                                                    sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                                                    sCondition += xSheetCondition->getFormula2();
                                                    sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
                                                }
                                                else
                                                {
                                                    sCondition = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content()"));
                                                    switch (aOperator)
                                                    {
                                                        case sheet::ConditionOperator_LESS:
                                                            sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
                                                        break;
                                                        case sheet::ConditionOperator_GREATER:
                                                            sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
                                                        break;
                                                        case sheet::ConditionOperator_LESS_EQUAL:
                                                            sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
                                                        break;
                                                        case sheet::ConditionOperator_GREATER_EQUAL:
                                                            sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
                                                        break;
                                                        case sheet::ConditionOperator_EQUAL:
                                                            sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
                                                        break;
                                                        case sheet::ConditionOperator_NOT_EQUAL:
                                                            sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
                                                        break;
                                                    }
                                                    sCondition += xSheetCondition->getFormula1();
                                                }
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_condition, sCondition);
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_apply_style_name, sStyleName);
                                                table::CellAddress aBase = xSheetCondition->getSourcePosition();
                                                ScAddress aBaseAddress(aBase.Column, aBase.Row, aBase.Sheet);
                                                String sBaseAddress;
                                                aBaseAddress.Format(sBaseAddress, SCA_VALID | SCA_TAB_3D, rScXMLExport.GetDocument());
                                                rtl::OUString sOUBaseAddress(sBaseAddress);
                                                rScXMLExport.AddAttribute(XML_NAMESPACE_STYLE, sXML_base_cell_address, sOUBaseAddress);
                                                SvXMLElementExport aMElem(rScXMLExport, XML_NAMESPACE_STYLE, sXML_map, sal_True, sal_True);
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
    SvXMLAutoStylePoolP(),
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
    if( xPropSetInfo->hasPropertyByName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")) ) )
    {
        uno::Any aAny = xPropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")) );
        sal_Int32 nNumberFormat;
        if (aAny >>= nNumberFormat)
        {
            rtl::OUString sDataStyleName = GetExport().getDataStyleName(nNumberFormat);
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_data_style_name,
                                                sDataStyleName );

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

/*XMLScPropHdlFactory::XMLScPropHdlFactory(const ScXMLExport* pScTempXMLExport)
    : XMLPropertyHandlerFactory(),
    pScXMLExport(pScTempXMLExport),
    bIsExport(sal_True)
{
}

XMLScPropHdlFactory::XMLScPropHdlFactory(const ScXMLImport* pScTempXMLImport)
    : XMLPropertyHandlerFactory(),
    pScXMLImport(pScTempXMLImport),
    bIsExport(sal_False)
{
}*/

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
  /*            case XML_SC_TYPE_NUMBERFORMAT :
            {
                if (bIsExport)
                    pHdl = new XmlScPropHdl_NumberFormat(pScXMLExport);
                else
                    pHdl = new XmlScPropHdl_NumberFormat(pScXMLImport);
            }
            break;*/
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
        }

        if(pHdl)
            PutHdlCache(nType, pHdl);
    }

    return pHdl;
}

void XMLCellStylesPropertySetMapper::ContextFilter(
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

    for( ::std::vector< XMLPropertyState >::iterator propertie = rProperties.begin();
         propertie != rProperties.end();
         propertie++ )
    {
        switch( GetEntryContextId( propertie->mnIndex ) )
        {
            case CTF_ALLPADDING:        pPadding = propertie; break;
            case CTF_BOTTOMPADDING:     pPadding_Bottom = propertie; break;
            case CTF_LEFTPADDING:       pPadding_Left = propertie; break;
            case CTF_RIGHTPADDING:      pPadding_Right = propertie; break;
            case CTF_TOPPADDING:        pPadding_Top = propertie; break;
            case CTF_ALLBORDER:         pBorder = propertie; break;
            case CTF_LEFTBORDER:        pBorder_Left = propertie; break;
            case CTF_RIGHTBORDER:       pBorder_Right = propertie; break;
            case CTF_BOTTOMBORDER:      pBorder_Bottom = propertie; break;
            case CTF_TOPBORDER:         pBorder_Top = propertie; break;
            case CTF_ALLBORDERWIDTH:    pAllBorderWidthState = propertie; break;
            case CTF_LEFTBORDERWIDTH:   pLeftBorderWidthState = propertie; break;
            case CTF_RIGHTBORDERWIDTH:  pRightBorderWidthState = propertie; break;
            case CTF_TOPBORDERWIDTH:    pTopBorderWidthState = propertie; break;
            case CTF_BOTTOMBORDERWIDTH: pBottomBorderWidthState = propertie; break;
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
}

XMLCellStylesPropertySetMapper::XMLCellStylesPropertySetMapper(const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory) :
    XMLPropertySetMapper( pEntries, rFactory )
{
}

XMLCellStylesPropertySetMapper::~XMLCellStylesPropertySetMapper()
{
}

void XMLColumnStylesPropertySetMapper::ContextFilter(
    ::std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
}

XMLColumnStylesPropertySetMapper::XMLColumnStylesPropertySetMapper(const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory) :
    XMLPropertySetMapper( pEntries, rFactory )
{
}

XMLColumnStylesPropertySetMapper::~XMLColumnStylesPropertySetMapper()
{
}

void XMLRowStylesPropertySetMapper::ContextFilter(
    ::std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
}

XMLRowStylesPropertySetMapper::XMLRowStylesPropertySetMapper(const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory) :
    XMLPropertySetMapper( pEntries, rFactory )
{
}

XMLRowStylesPropertySetMapper::~XMLRowStylesPropertySetMapper()
{
}

void XMLTableStylesPropertySetMapper::ContextFilter(
    ::std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
}

XMLTableStylesPropertySetMapper::XMLTableStylesPropertySetMapper(const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory) :
    XMLPropertySetMapper( pEntries, rFactory )
{
}

XMLTableStylesPropertySetMapper::~XMLTableStylesPropertySetMapper()
{
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
        if (rStrImpValue.compareToAscii(sXML_none) != 0)
        {
            if (rStrImpValue.compareToAscii(sXML_hidden_and_protected) != 0)
            {
                if (rStrImpValue.compareToAscii(sXML_protected) != 0)
                {
                    if (rStrImpValue.compareToAscii(sXML_formula_hidden) != 0)
                    {
                        sal_Int16 i = 0;
                        while (i < rStrImpValue.getLength() && rStrImpValue[i] != ' ')
                            i++;
                        rtl::OUString sFirst = rStrImpValue.copy(0, i);
                        rtl::OUString sSecond = rStrImpValue.copy(i + 1);
                        aCellProtection.IsFormulaHidden = sal_False;
                        aCellProtection.IsHidden = sal_False;
                        aCellProtection.IsLocked = sal_False;
                        if ((sFirst.compareToAscii(sXML_protected) == 0) || (sSecond.compareToAscii(sXML_protected) == 0))
                            aCellProtection.IsLocked = sal_True;
                        if ((sFirst.compareToAscii(sXML_formula_hidden) == 0) || (sSecond.compareToAscii(sXML_formula_hidden) == 0))
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
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_none));
            bRetval = sal_True;
        }
        else if (aCellProtection.IsHidden && aCellProtection.IsLocked)
        {
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_hidden_and_protected));
            bRetval = sal_True;
        }
        else if (aCellProtection.IsLocked && !(aCellProtection.IsFormulaHidden || aCellProtection.IsHidden))
        {
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_protected));
            bRetval = sal_True;
        }
        else if (aCellProtection.IsFormulaHidden && !(aCellProtection.IsLocked || aCellProtection.IsHidden))
        {
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_formula_hidden));
            bRetval = sal_True;
        }
        else if (aCellProtection.IsFormulaHidden && aCellProtection.IsLocked)
        {
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_protected));
            rStrExpValue += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
            rStrExpValue += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_formula_hidden));
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
        rtl::OUStringBuffer sValue;
        rUnitConverter.convertBool(sValue, !aCellProtection.IsPrintHidden);
        rStrExpValue = sValue.makeStringAndClear();
        bRetval = sal_True;
    }

    return bRetval;
}

/*XmlScPropHdl_NumberFormat::XmlScPropHdl_NumberFormat(const ScXMLExport* pScTempXMLExport)
{
    pScXMLExport = pScTempXMLExport;
}

XmlScPropHdl_NumberFormat::XmlScPropHdl_NumberFormat(const ScXMLImport* pScTempXMLImport)
{
    pScXMLImport = pScTempXMLImport;
}

XmlScPropHdl_NumberFormat::~XmlScPropHdl_NumberFormat()
{
}

sal_Bool XmlScPropHdl_NumberFormat::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Int32 aNumberFormat1, aNumberFormat2;

    if((r1 >>= aNumberFormat1) && (r2 >>= aNumberFormat2))
    {
        return (aNumberFormat1 == aNumberFormat2);
    }
    return sal_False;
}

sal_Bool XmlScPropHdl_NumberFormat::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    sal_Int32 nNumberFormat = pScXMLImport->GetNumImport()->GetKeyForName(rStrImpValue);
    if (nNumberFormat >= 0)
    {
        rValue <<= nNumberFormat;
        bRetval = sal_True;
    }
    else
        bRetval = sal_False;

    return bRetval;
}

sal_Bool XmlScPropHdl_NumberFormat::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    sal_Int32 nNumberFormat;
    if (rValue >>= nNumberFormat)
    {
        rStrExpValue = pScXMLExport->GetNumExport()->GetStyleName(nNumberFormat);
        bRetval = sal_True;
    }

    return bRetval;
}*/

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
    if (rStrImpValue.compareToAscii(sXML_start) == 0)
    {
         nValue = table::CellHoriJustify_LEFT;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_end) == 0)
    {
         nValue = table::CellHoriJustify_RIGHT;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_center) == 0)
    {
         nValue = table::CellHoriJustify_CENTER;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_justify) == 0)
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
            case table::CellHoriJustify_LEFT:
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_start));
                bRetval = sal_True;
            }
            break;
            case table::CellHoriJustify_RIGHT:
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_end));
                bRetval = sal_True;
            }
            break;
            case table::CellHoriJustify_CENTER:
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_center));
                bRetval = sal_True;
            }
            break;
            case table::CellHoriJustify_BLOCK:
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_justify));
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

    if (rStrImpValue.compareToAscii(sXML_fix) == 0)
    {
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_value_type) == 0)
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
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_value_type));
            bRetval = sal_True;
        }
        else
        {
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_fix));
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
    if (rStrImpValue.compareToAscii(sXML_ltr) == 0)
    {
        nValue = table::CellOrientation_STANDARD;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_ttb) == 0)
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
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_ttb));
                bRetval = sal_True;
            }
            break;
            default:
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_ltr));
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
        rtl::OUStringBuffer sValue;
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
    if (rStrImpValue.compareToAscii(sXML_none) == 0)
    {
        nValue = table::CellVertJustify_STANDARD;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_bottom) == 0)
    {
        nValue = table::CellVertJustify_BOTTOM;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_top) == 0)
    {
        nValue = table::CellVertJustify_TOP;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_center) == 0)
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
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom));
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_CENTER :
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_center));
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_STANDARD :
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_none));
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_TOP :
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top));
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
    if (rStrImpValue.compareToAscii(sXML_automatic) == 0)
    {
        nValue = table::CellVertJustify_STANDARD;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_bottom) == 0)
    {
        nValue = table::CellVertJustify_BOTTOM;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_top) == 0)
    {
        nValue = table::CellVertJustify_TOP;
        rValue <<= nValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_middle) == 0)
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
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom));
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_CENTER :
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_middle));
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_STANDARD :
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_automatic));
                bRetval = sal_True;
            }
            break;
            case table::CellVertJustify_TOP :
            {
                rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top));
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
    if (rStrImpValue.compareToAscii(sXML_auto) == 0)
    {
        bValue = sal_False;
        rValue <<= bValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_page) == 0)
    {
        bValue = sal_True;
        rValue <<= bValue;
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
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_page));
            bRetval = sal_True;
        }
        else
        {
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_auto));
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
    sal_Bool aBreak1, aBreak2;

    if((r1 >>= aBreak1) && (r2 >>= aBreak2))
        return (aBreak1 == aBreak2);
    return sal_False;
}

sal_Bool XmlScPropHdl_IsTextWrapped::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);

    sal_Bool bValue;
    if (rStrImpValue.compareToAscii(sXML_wrap) == 0)
    {
        bValue = sal_True;
        rValue <<= bValue;
        bRetval = sal_True;
    }
    else if (rStrImpValue.compareToAscii(sXML_no_wrap) == 0)
    {
        bValue = sal_False;
        rValue <<= bValue;
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XmlScPropHdl_IsTextWrapped::exportXML(
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
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_wrap));
            bRetval = sal_True;
        }
        else
        {
            rStrExpValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_no_wrap));
            bRetval = sal_True;
        }
    }

    return bRetval;
}
