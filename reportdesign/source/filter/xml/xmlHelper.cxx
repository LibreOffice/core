/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlHelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:45:50 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPT_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#include <connectivity/dbtools.hxx>
#include <comphelper/propertysethelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_IMAGEPOSITION_HPP_
#include <com/sun/star/awt/ImagePosition.hpp>
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <xmloff/prstylei.hxx>
#endif
#include "xmlstrings.hrc"
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef _XMLOFF_CONTEXTID_HXX_
#include <xmloff/contextid.hxx>
#endif
#ifndef _XMLOFF_TEXTPRMAP_HXX_
#include <xmloff/txtprmap.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XOFFICEDATABASEDOCUMENT_HPP_
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#endif
#include <com/sun/star/table/BorderLine.hpp>
#ifndef _XMLOFF_XMLCONSTANTSPROPERTYHANDLER_HXX
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_FORCENEWPAGE_HPP_
#include <com/sun/star/report/ForceNewPage.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_REPORTPRINTOPTION_HPP_
#include <com/sun/star/report/ReportPrintOption.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_GROUPON_HPP_
#include <com/sun/star/report/GroupOn.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_KEEPTOGETHER_HPP_
#include <com/sun/star/report/KeepTogether.hpp>
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlement.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCONTROLFORMAT_HPP_
#include <com/sun/star/report/XReportControlFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_LISTSOURCETYPE_HPP_
#include <com/sun/star/form/ListSourceType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif
#ifndef _XMLOFF_ENUMPROPERTYHANDLER_HXX
#include <xmloff/EnumPropertyHdl.hxx>
#endif

#define XML_RPT_ALGINMENT   (XML_DB_TYPES_START+1)
namespace rptxml
{
    using namespace ::xmloff::token;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::beans;
DBG_NAME(rpt_OPropertyHandlerFactory)
OPropertyHandlerFactory::OPropertyHandlerFactory()
{
    DBG_CTOR(rpt_OPropertyHandlerFactory,NULL);
}
// -----------------------------------------------------------------------------
OPropertyHandlerFactory::~OPropertyHandlerFactory()
{
    DBG_DTOR(rpt_OPropertyHandlerFactory,NULL);
}
// -----------------------------------------------------------------------------
const XMLPropertyHandler* OPropertyHandlerFactory::GetPropertyHandler(sal_Int32 _nType) const
{
    const XMLPropertyHandler* pHandler = NULL;

    switch(_nType)
    {
        case XML_RPT_ALGINMENT:
            {
                static SvXMLEnumMapEntry __READONLY_DATA pXML_VerticalAlign_Enum[] =
                {
                    { XML_TOP,          drawing::TextVerticalAdjust_TOP },
                    { XML_MIDDLE,       drawing::TextVerticalAdjust_CENTER },
                    { XML_BOTTOM,       drawing::TextVerticalAdjust_BOTTOM },
                    { XML_JUSTIFY,      drawing::TextVerticalAdjust_BLOCK },
                    { XML_TOKEN_INVALID, 0 }
                };

                pHandler = new XMLEnumPropertyHdl( pXML_VerticalAlign_Enum, ::getCppuType((const com::sun::star::drawing::TextVerticalAdjust*)0) );
            }
            break;
        default:
            ;
    }

    if ( !pHandler )
        pHandler = OControlPropertyHandlerFactory::GetPropertyHandler(_nType);
    return pHandler;
}
// -----------------------------------------------------------------------------
#define MAP_CONST( name, prefix, token, type, context )  { name.ascii, name.length,     XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TEXT,       context }
#define MAP_CONST_T( name, prefix, token, type, context )  { name.ascii, name.length,   XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TABLE,      context }
#define MAP_CONST_ASCII_T( name, prefix, token, type, context )  { name, sizeof(name)-1,XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TABLE,      context }
#define MAP_CONST_P( name, prefix, token, type, context )  { name.ascii, name.length,   XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_PARAGRAPH,  context }
#define MAP_CONST_S( name, prefix, token, type, context )  { name, sizeof(name)-1,      XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_SECTION,    context }
#define MAP_CONST_ASCII( name, prefix, token, type, context )  { name, sizeof(name)-1,  XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TEXT,       context }
#define GMAP( name, prefix, token, type, context )  { name.ascii, name.length,          XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_GRAPHIC,    context }
#define MAP_CONST_C_ASCII( name, prefix, token, type, context ) { name, sizeof(name)-1, XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TABLE_CELL, context }
#define MAP_CONST_C( name, prefix, token, type, context )  { name, name.length,         XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TABLE_CELL, context }
#define MAP_END()   { NULL, 0, 0, XML_TOKEN_INVALID, 0 ,0}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > OXMLHelper::GetCellStylePropertyMap(bool _bOldFormat)
{
    if ( _bOldFormat )
    {
        static const XMLPropertyMapEntry s_aXMLCellStylesProperties[] =
        {
            MAP_CONST_C(      PROPERTY_FORMATKEY,      STYLE,     DATA_STYLE_NAME,        XML_TYPE_NUMBER | MID_FLAG_SPECIAL_ITEM, CTF_RPT_NUMBERFORMAT ),

            MAP_CONST_C(      PROPERTY_CONTROLBACKGROUND,
                                                FO,   BACKGROUND_COLOR,     XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
            MAP_CONST_C(      PROPERTY_CONTROLBACKGROUNDTRANSPARENT,
                                                FO,   BACKGROUND_COLOR,     XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
            MAP_CONST_P(      PROPERTY_CONTROLBACKGROUND,
                                                FO,   BACKGROUND_COLOR,     XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
            MAP_CONST_P(      PROPERTY_CONTROLBACKGROUNDTRANSPARENT,
                                                FO,   BACKGROUND_COLOR,     XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
            MAP_CONST_C_ASCII(      "BorderLeft",       FO,     BORDER_LEFT,           XML_TYPE_BORDER, 0 ),
            MAP_CONST_C_ASCII(      "BorderRight",      FO,     BORDER_RIGHT,          XML_TYPE_BORDER, 0 ),
            MAP_CONST_C_ASCII(      "BorderTop",        FO,     BORDER_TOP,            XML_TYPE_BORDER, 0 ),
            MAP_CONST_C_ASCII(      "BorderBottom",     FO,     BORDER_BOTTOM,         XML_TYPE_BORDER, 0 ),
            MAP_END()
        };
        return new XMLPropertySetMapper((XMLPropertyMapEntry*)s_aXMLCellStylesProperties,new OPropertyHandlerFactory());
    }
    else
    {
        static const XMLPropertyMapEntry s_aXMLCellStylesProperties[] =
        {
            MAP_CONST_C(      PROPERTY_FORMATKEY,      STYLE,     DATA_STYLE_NAME,        XML_TYPE_NUMBER | MID_FLAG_SPECIAL_ITEM, CTF_RPT_NUMBERFORMAT ),

            MAP_CONST_C(      PROPERTY_CONTROLBACKGROUND,
                                                FO,   BACKGROUND_COLOR,     XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
            MAP_CONST_C(      PROPERTY_CONTROLBACKGROUNDTRANSPARENT,
                                                FO,   BACKGROUND_COLOR,     XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
            MAP_CONST_C_ASCII(      "BorderLeft",       FO,     BORDER_LEFT,           XML_TYPE_BORDER, 0 ),
            MAP_CONST_C_ASCII(      "BorderRight",      FO,     BORDER_RIGHT,          XML_TYPE_BORDER, 0 ),
            MAP_CONST_C_ASCII(      "BorderTop",        FO,     BORDER_TOP,            XML_TYPE_BORDER, 0 ),
            MAP_CONST_C_ASCII(      "BorderBottom",     FO,     BORDER_BOTTOM,         XML_TYPE_BORDER, 0 ),
            MAP_END()
        };
        return new XMLPropertySetMapper((XMLPropertyMapEntry*)s_aXMLCellStylesProperties,new OPropertyHandlerFactory());
    }
}
// -----------------------------------------------------------------------------
const XMLPropertyMapEntry* OXMLHelper::GetTableStyleProps()
{
    static const XMLPropertyMapEntry aXMLTableStylesProperties[] =
    {
        MAP_CONST_T(  PROPERTY_BACKCOLOR,      FO,     BACKGROUND_COLOR,        XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
        MAP_CONST_T(  PROPERTY_BACKTRANSPARENT,FO,     BACKGROUND_COLOR,        XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE, 0 ),
        // MAP_CONST_T(  PROPERTY_KEEPTOGETHER,   STYLE,  MAY_BREAK_BETWEEN_ROWS,  XML_TYPE_BOOL     , 0 ),
        MAP_END()
    };
    return aXMLTableStylesProperties;
}
// -----------------------------------------------------------------------------
const XMLPropertyMapEntry* OXMLHelper::GetRowStyleProps()
{
    static const XMLPropertyMapEntry aXMLStylesProperties[] =
    {
        MAP_CONST_S( "Height", STYLE, ROW_HEIGHT, XML_TYPE_PROP_TABLE_ROW|XML_TYPE_MEASURE, 0),
        MAP_END()
    };
    return aXMLStylesProperties;
}
// -----------------------------------------------------------------------------
const XMLPropertyMapEntry* OXMLHelper::GetColumnStyleProps()
{
    static const XMLPropertyMapEntry aXMLColumnStylesProperties[] =
    {
        //MAP( "IsManualPageBreak", XML_NAMESPACE_FO, XML_BREAK_BEFORE, XML_TYPE_PROP_TABLE_COLUMN|XML_SC_TYPE_BREAKBEFORE, 0),
        //MAP( "IsVisible", XML_NAMESPACE_TABLE, XML_DISPLAY, XML_TYPE_PROP_TABLE_COLUMN|XML_SC_TYPE_EQUAL|MID_FLAG_SPECIAL_ITEM, CTF_SC_ISVISIBLE ),
        MAP_CONST_S(    "Width",                 STYLE,     COLUMN_WIDTH,           XML_TYPE_PROP_TABLE_COLUMN|XML_TYPE_MEASURE, 0 ),
    //  MAP( "OptimalWidth", XML_NAMESPACE_STYLE, XML_USE_OPTIMAL_COLUMN_WIDTH, XML_TYPE_PROP_TABLE_COLUMN|XML_TYPE_BOOL, 0),
        MAP_END()
    };
    return aXMLColumnStylesProperties;
}
// -----------------------------------------------------------------------------
const SvXMLEnumMapEntry* OXMLHelper::GetReportPrintOptions()
{
    static SvXMLEnumMapEntry s_aXML_EnumMap[] =
    {
         // { XML_ALL_PAGES,                            report::ReportPrintOption::ALL_PAGES }, // default
        { XML_NOT_WITH_REPORT_HEADER,               report::ReportPrintOption::NOT_WITH_REPORT_HEADER },
        { XML_NOT_WITH_REPORT_FOOTER,               report::ReportPrintOption::NOT_WITH_REPORT_FOOTER },
        { XML_NOT_WITH_REPORT_HEADER_NOR_FOOTER,    report::ReportPrintOption::NOT_WITH_REPORT_HEADER_FOOTER },
        { XML_TOKEN_INVALID, 0 }
    };
    return s_aXML_EnumMap;
}
// -----------------------------------------------------------------------------
const SvXMLEnumMapEntry* OXMLHelper::GetForceNewPageOptions()
{
    static SvXMLEnumMapEntry s_aXML_EnumMap[] =
    {
        // { XML_NONE,                  report::ForceNewPage::NONE }, // default
        { XML_BEFORE_SECTION,       report::ForceNewPage::BEFORE_SECTION },
        { XML_AFTER_SECTION,        report::ForceNewPage::AFTER_SECTION },
        { XML_BEFORE_AFTER_SECTION, report::ForceNewPage::BEFORE_AFTER_SECTION },
        { XML_TOKEN_INVALID, 0 }
    };
    return s_aXML_EnumMap;
}
// -----------------------------------------------------------------------------
//// -----------------------------------------------------------------------------
//const SvXMLEnumMapEntry* OXMLHelper::GetGroupOnOptions()
//{
//  static SvXMLEnumMapEntry s_aXML_EnumMap[] =
//  {
//      // { XML_DEFAULT ,report::GroupOn::DEFAULT }, // default
//      { XML_PREFIX_CHARACTERS ,report::GroupOn::PREFIX_CHARACTERS },
//      { XML_YEAR ,report::GroupOn::YEAR },
//      { XML_QUARTAL , report::GroupOn::QUARTAL },
//      { XML_MONTH ,   report::GroupOn::MONTH },
//      { XML_WEEK ,    report::GroupOn::WEEK },
//      { XML_DAY , report::GroupOn::DAY },
//      { XML_HOUR ,    report::GroupOn::HOUR },
//      { XML_MINUTE ,  report::GroupOn::MINUTE },
//      { XML_INTERVAL ,    report::GroupOn::INTERVAL },
//      { XML_TOKEN_INVALID, 0 }
//  };
//  return s_aXML_EnumMap;
//}
// -----------------------------------------------------------------------------
const SvXMLEnumMapEntry* OXMLHelper::GetKeepTogetherOptions()
{
    static SvXMLEnumMapEntry s_aXML_EnumMap[] =
    {
        // { XML_NO,                    report::KeepTogether::NO }, // default
        { XML_WHOLE_GROUP,          report::KeepTogether::WHOLE_GROUP },
        { XML_WITH_FIRST_DETAIL,    report::KeepTogether::WITH_FIRST_DETAIL },
        { XML_TOKEN_INVALID, 0 }
    };
    return s_aXML_EnumMap;
}
// -----------------------------------------------------------------------------
const SvXMLEnumMapEntry* OXMLHelper::GetImagePositionOptions()
{
    static SvXMLEnumMapEntry s_aXML_EnumMap[] =
    {
        { XML_START,  0 },
        { XML_END,    1 },
        { XML_TOP,    2 },
        { XML_BOTTOM, 3 },
        { XML_CENTER, (sal_uInt16)-1 },
        { XML_TOKEN_INVALID, 0 }
    };
    return s_aXML_EnumMap;
}
// -----------------------------------------------------------------------------
const SvXMLEnumMapEntry* OXMLHelper::GetImageAlignOptions()
{
    static SvXMLEnumMapEntry s_aXML_EnumMap[] =
    {
        { XML_START,  0 },
        { XML_CENTER, 1 },
        { XML_END, 2 },
        { XML_TOKEN_INVALID, 0 }
    };
    return s_aXML_EnumMap;
}
// -----------------------------------------------------------------------------
const SvXMLEnumMapEntry* OXMLHelper::GetCommandTypeOptions()
{
    static SvXMLEnumMapEntry s_aXML_EnumMap[] =
    {
        { XML_TABLE, CommandType::TABLE },
        { XML_QUERY, CommandType::QUERY },
        // { XML_COMMAND, CommandType::COMMAND }, // default
        { XML_TOKEN_INVALID, 0 }
    };
    return s_aXML_EnumMap;
}
// -----------------------------------------------------------------------------
uno::Reference< util::XNumberFormatsSupplier > OXMLHelper::GetNumberFormatsSupplier(const uno::Reference<report::XReportDefinition>& _xReportDefinition)
{
    uno::Reference< util::XNumberFormatsSupplier > xSupplier;
    uno::Reference< uno::XInterface> xParent = _xReportDefinition->getParent();
    if ( xParent.is() )
    {
        uno::Reference< sdb::XOfficeDatabaseDocument > xDatabaseDocument(xParent,uno::UNO_QUERY);
        if ( !xDatabaseDocument.is() )
        {
            uno::Reference< container::XChild> xChild(xParent,uno::UNO_QUERY);
            while( !xDatabaseDocument.is() && xChild.is() )
            {
                xParent = xChild->getParent();
                xDatabaseDocument.set(xParent,uno::UNO_QUERY);
                xChild.set(xParent,uno::UNO_QUERY);
            }
        }
        if ( xDatabaseDocument.is() )
        {
            uno::Reference<beans::XPropertySet> xProp(xDatabaseDocument->getDataSource(),uno::UNO_QUERY);
            if ( xProp.is() )
                xSupplier.set(xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormatsSupplier"))),uno::UNO_QUERY);
        }
    }
    else
    {
        ::comphelper::MediaDescriptor aDescriptor( _xReportDefinition->getArgs() );
        uno::Sequence<beans::PropertyValue> aComponentData;
        aComponentData = aDescriptor.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ComponentData")),aComponentData);
        if ( aComponentData.getLength() )
        {
            ::comphelper::SequenceAsHashMap aComponentDataMap( aComponentData );
            uno::Reference<sdbc::XConnection> xConnection;
            xConnection = aComponentDataMap.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ActiveConnection")),xConnection);
            xSupplier = dbtools::getNumberFormats(xConnection);
        }
    }
    return xSupplier;
}
// -----------------------------------------------------------------------------
#define PROPERTY_ID_FONTNAME         1
#define PROPERTY_ID_FONTHEIGHT       2
#define PROPERTY_ID_FONTWIDTH        3
#define PROPERTY_ID_FONTSTYLENAME    4
#define PROPERTY_ID_FONTFAMILY       5
#define PROPERTY_ID_FONTCHARSET      6
#define PROPERTY_ID_FONTPITCH        7
#define PROPERTY_ID_FONTCHARWIDTH    8
#define PROPERTY_ID_FONTWEIGHT       9
#define PROPERTY_ID_FONTSLANT        10
#define PROPERTY_ID_FONTUNDERLINE    11
#define PROPERTY_ID_FONTSTRIKEOUT    12
#define PROPERTY_ID_FONTORIENTATION  13
#define PROPERTY_ID_FONTKERNING      14
#define PROPERTY_ID_FONTWORDLINEMODE 15
#define PROPERTY_ID_FONTTYPE         16
void OXMLHelper::copyStyleElements(const ::rtl::OUString& _sStyleName,const SvXMLStylesContext* _pAutoStyles,const uno::Reference<beans::XPropertySet>& _xProp)
{
    if ( !_xProp.is() || !_sStyleName.getLength() || !_pAutoStyles )
        return;
    XMLPropStyleContext* pAutoStyle = PTR_CAST(XMLPropStyleContext,_pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL,_sStyleName));
    if ( pAutoStyle )
    {
        ::com::sun::star::awt::FontDescriptor aFont;
        static comphelper::PropertyMapEntry pMap[] =
        {
            {PROPERTY_FONTNAME,         static_cast<sal_uInt16>(PROPERTY_FONTNAME.length),          PROPERTY_ID_FONTNAME,           &::getCppuType(&aFont.Name)         ,PropertyAttribute::BOUND,0},
            {PROPERTY_CHARFONTHEIGHT,   static_cast<sal_uInt16>(PROPERTY_CHARFONTHEIGHT.length),    PROPERTY_ID_FONTHEIGHT,         &::getCppuType(&aFont.Height)       ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTWIDTH,        static_cast<sal_uInt16>(PROPERTY_FONTWIDTH.length),         PROPERTY_ID_FONTWIDTH,          &::getCppuType(&aFont.Width)        ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTSTYLENAME,    static_cast<sal_uInt16>(PROPERTY_FONTSTYLENAME.length),     PROPERTY_ID_FONTSTYLENAME,      &::getCppuType(&aFont.StyleName)    ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTFAMILY,       static_cast<sal_uInt16>(PROPERTY_FONTFAMILY.length),        PROPERTY_ID_FONTFAMILY,         &::getCppuType(&aFont.Family)       ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTCHARSET,      static_cast<sal_uInt16>(PROPERTY_FONTCHARSET.length),       PROPERTY_ID_FONTCHARSET,        &::getCppuType(&aFont.CharSet)      ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTPITCH,        static_cast<sal_uInt16>(PROPERTY_FONTPITCH.length),         PROPERTY_ID_FONTPITCH,          &::getCppuType(&aFont.Pitch)        ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTCHARWIDTH,    static_cast<sal_uInt16>(PROPERTY_FONTCHARWIDTH.length),     PROPERTY_ID_FONTCHARWIDTH,      &::getCppuType(&aFont.CharacterWidth),PropertyAttribute::BOUND,0},
            {PROPERTY_FONTWEIGHT,       static_cast<sal_uInt16>(PROPERTY_FONTWEIGHT.length),        PROPERTY_ID_FONTWEIGHT,         &::getCppuType(&aFont.Weight)       ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTSLANT,        static_cast<sal_uInt16>(PROPERTY_FONTSLANT.length),         PROPERTY_ID_FONTSLANT,          &::getCppuType(&aFont.Slant)        ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTUNDERLINE,    static_cast<sal_uInt16>(PROPERTY_FONTUNDERLINE.length),     PROPERTY_ID_FONTUNDERLINE,      &::getCppuType(&aFont.Underline)    ,PropertyAttribute::BOUND,0},
            {PROPERTY_CHARSTRIKEOUT,    static_cast<sal_uInt16>(PROPERTY_CHARSTRIKEOUT.length),     PROPERTY_ID_FONTSTRIKEOUT,      &::getCppuType(&aFont.Strikeout)    ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTORIENTATION,  static_cast<sal_uInt16>(PROPERTY_FONTORIENTATION.length),   PROPERTY_ID_FONTORIENTATION,    &::getCppuType(&aFont.Orientation)  ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTKERNING,      static_cast<sal_uInt16>(PROPERTY_FONTKERNING.length),       PROPERTY_ID_FONTKERNING,        &::getCppuType(&aFont.Kerning)      ,PropertyAttribute::BOUND,0},
            {PROPERTY_CHARWORDMODE,     static_cast<sal_uInt16>(PROPERTY_CHARWORDMODE.length),      PROPERTY_ID_FONTWORDLINEMODE,   &::getCppuType(&aFont.WordLineMode) ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTTYPE,         static_cast<sal_uInt16>(PROPERTY_FONTTYPE.length),          PROPERTY_ID_FONTTYPE,           &::getCppuType(&aFont.Type)         ,PropertyAttribute::BOUND,0},
            //{PROPERTY_BACKGROUNDCOLOR,    static_cast<sal_uInt16>(PROPERTY_BACKGROUNDCOLOR.length),   PROPERTY_ID_BACKGROUNDCOLOR,    &::getCppuType(&nBackgroundColor)   ,PropertyAttribute::BOUND,0},,
            //{PROPERTY_PARAADJUST,         static_cast<sal_uInt16>(PROPERTY_PARAADJUST.length),                PROPERTY_ID_ALIGN,              &::getCppuType(&nAlign)             ,PropertyAttribute::BOUND,0},,
            //{PROPERTY_CONTROLBORDER,          static_cast<sal_uInt16>(PROPERTY_CONTROLBORDER.length),         PROPERTY_ID_BORDER,             &::getCppuType(&aFont.Type)         ,PropertyAttribute::BOUND,0},,
            //{PROPERTY_CONTROLBORDERCOLOR,     static_cast<sal_uInt16>(PROPERTY_CONTROLBORDERCOLOR.length),        PROPERTY_ID_BORDERCOLOR,        &::getCppuType(&aFont.Type)         ,PropertyAttribute::BOUND,0},,
            //{PROPERTY_CHARCOLOR,      static_cast<sal_uInt16>(PROPERTY_TEXTCOLOR.length),         PROPERTY_ID_TEXTCOLOR,          &::getCppuType(&aFont.Type)         ,PropertyAttribute::BOUND,0},,
            //{PROPERTY_FORMATKEY,      static_cast<sal_uInt16>(PROPERTY_FORMATKEY.length),         PROPERTY_ID_FORMATKEY,          &::getCppuType(&aFont.Type)         ,PropertyAttribute::BOUND,0},,
            //{PROPERTY_CHARUNDERLINECOLOR, static_cast<sal_uInt16>(PROPERTY_CHARUNDERLINECOLOR.length),        PROPERTY_ID_TEXTLINECOLOR,      &::getCppuType(&aFont.Type)         ,PropertyAttribute::BOUND,0},,
            { NULL, 0, 0, NULL, 0, 0 }
        };
        try
        {
            pAutoStyle->FillPropertySet(_xProp);

            uno::Reference<beans::XPropertySet> xProp = comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(pMap));
            pAutoStyle->FillPropertySet(xProp);
            xProp->getPropertyValue(PROPERTY_FONTNAME) >>=          aFont.Name;
            xProp->getPropertyValue(PROPERTY_CHARFONTHEIGHT) >>=        aFont.Height;
            xProp->getPropertyValue(PROPERTY_FONTWIDTH) >>=             aFont.Width;
            xProp->getPropertyValue(PROPERTY_FONTSTYLENAME) >>=         aFont.StyleName;
            xProp->getPropertyValue(PROPERTY_FONTFAMILY) >>=        aFont.Family;
            xProp->getPropertyValue(PROPERTY_FONTCHARSET) >>=       aFont.CharSet;
            xProp->getPropertyValue(PROPERTY_FONTPITCH) >>=             aFont.Pitch;
            xProp->getPropertyValue(PROPERTY_FONTCHARWIDTH) >>=         aFont.CharacterWidth;
            xProp->getPropertyValue(PROPERTY_FONTWEIGHT) >>=        aFont.Weight;
            xProp->getPropertyValue(PROPERTY_FONTSLANT) >>=             aFont.Slant;
            xProp->getPropertyValue(PROPERTY_FONTUNDERLINE) >>=         aFont.Underline;
            xProp->getPropertyValue(PROPERTY_CHARSTRIKEOUT) >>=         aFont.Strikeout;
            xProp->getPropertyValue(PROPERTY_FONTORIENTATION) >>=   aFont.Orientation;
            xProp->getPropertyValue(PROPERTY_FONTKERNING) >>=       aFont.Kerning;
            xProp->getPropertyValue(PROPERTY_CHARWORDMODE) >>=  aFont.WordLineMode;
            xProp->getPropertyValue(PROPERTY_FONTTYPE) >>=          aFont.Type;
            uno::Reference<report::XReportControlFormat> xReportControlModel(_xProp,uno::UNO_QUERY);
            if ( xReportControlModel.is() && aFont.Name.getLength() )
                try
                {
                    xReportControlModel->setFontDescriptor(aFont);
                }
                catch(beans::UnknownPropertyException){}

            if ( xReportControlModel.is() )
            {
                sal_Int16 nTextAlign = xReportControlModel->getParaAdjust();
                switch(nTextAlign)
                {
                    case style::ParagraphAdjust_LEFT:
                        nTextAlign = awt::TextAlign::LEFT;
                        break;
                    case style::ParagraphAdjust_CENTER:
                        nTextAlign = awt::TextAlign::CENTER;
                        break;
                    case style::ParagraphAdjust_RIGHT:
                        nTextAlign = awt::TextAlign::RIGHT;
                        break;
                    default:
                        OSL_ENSURE(0,"Illegal text alignment value!");
                        break;
                }
                xReportControlModel->setParaAdjust(nTextAlign);
            }
           }
        catch(uno::Exception&)
        {
            OSL_ENSURE(0,"OXMLHelper::copyStyleElements -> exception catched");
        }
    }
}
// -----------------------------------------------------------------------------
uno::Reference<beans::XPropertySet> OXMLHelper::createBorderPropertySet()
{
    static comphelper::PropertyMapEntry pMap[] =
    {
        {PROPERTY_BORDERLEFT,   static_cast<sal_uInt16>(PROPERTY_BORDERLEFT.length),        0,          &::getCppuType((const table::BorderLine*)0)         ,PropertyAttribute::BOUND,0},
        {PROPERTY_BORDERRIGHT,  static_cast<sal_uInt16>(PROPERTY_BORDERRIGHT.length),       1,          &::getCppuType((const table::BorderLine*)0)         ,PropertyAttribute::BOUND,0},
        {PROPERTY_BORDERTOP,    static_cast<sal_uInt16>(PROPERTY_BORDERTOP.length),         2,          &::getCppuType((const table::BorderLine*)0)         ,PropertyAttribute::BOUND,0},
        {PROPERTY_BORDERBOTTOM, static_cast<sal_uInt16>(PROPERTY_BORDERBOTTOM.length),      3,          &::getCppuType((const table::BorderLine*)0)         ,PropertyAttribute::BOUND,0},
        { NULL, 0, 0, NULL, 0, 0 }
    };
    return comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(pMap));
}
// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------
