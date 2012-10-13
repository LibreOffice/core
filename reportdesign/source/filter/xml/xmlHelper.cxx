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
#include "xmlHelper.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/families.hxx>
#include <xmloff/controlpropertyhdl.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/propertysethelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <xmloff/prstylei.hxx>
#include "xmlstrings.hrc"
#include "xmlEnums.hxx"
#include <xmloff/contextid.hxx>
#include <xmloff/txtprmap.hxx>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#include <com/sun/star/report/ForceNewPage.hpp>
#include <com/sun/star/report/ReportPrintOption.hpp>
#include <com/sun/star/report/GroupOn.hpp>
#include <com/sun/star/report/KeepTogether.hpp>
#include <xmloff/xmlement.hxx>
#include <com/sun/star/report/XReportControlFormat.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <xmloff/EnumPropertyHdl.hxx>

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
    sal_Int32 nType = _nType;
    nType &= MID_FLAG_MASK;

    switch(nType)
    {
        case XML_RPT_ALGINMENT:
            {
                static SvXMLEnumMapEntry const pXML_VerticalAlign_Enum[] =
                {
                    { XML_TOP,          style::VerticalAlignment_TOP },
                    { XML_MIDDLE,       style::VerticalAlignment_MIDDLE },
                    { XML_BOTTOM,       style::VerticalAlignment_BOTTOM },
                    { XML_TOKEN_INVALID, 0 }
                };

                pHandler = new XMLEnumPropertyHdl( pXML_VerticalAlign_Enum, ::getCppuType((const com::sun::star::style::VerticalAlignment*)0) );
            }
            break;
        case (XML_SD_TYPES_START+34):
            pHandler = new xmloff::ImageScaleModeHandler();
        default:
            ;
    }

    if ( !pHandler )
        pHandler = OControlPropertyHandlerFactory::GetPropertyHandler(_nType);
    else
        PutHdlCache(nType, pHandler);
    return pHandler;
}
// -----------------------------------------------------------------------------
#define MAP_CONST( name, prefix, token, type, context )  { name.ascii, name.length,     XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TEXT,       context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_T( name, prefix, token, type, context )  { name.ascii, name.length,   XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TABLE,      context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_ASCII_T( name, prefix, token, type, context )  { name, sizeof(name)-1,XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TABLE,      context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_P( name, prefix, token, type, context )  { name.ascii, name.length,   XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_PARAGRAPH,  context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_S( name, prefix, token, type, context )  { name, sizeof(name)-1,      XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_SECTION,    context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_ASCII( name, prefix, token, type, context )  { name, sizeof(name)-1,  XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TEXT,       context, SvtSaveOptions::ODFVER_010 }
#define GMAP( name, prefix, token, type, context )  { name.ascii, name.length,          XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_GRAPHIC,    context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_C_ASCII( name, prefix, token, type, context ) { name, sizeof(name)-1, XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TABLE_CELL, context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_C( name, prefix, token, type, context )  { name.ascii, name.length,         XML_NAMESPACE_##prefix, XML_##token, type|XML_TYPE_PROP_TABLE_CELL, context, SvtSaveOptions::ODFVER_010 }
#define MAP_END() { NULL, 0, 0, XML_TOKEN_INVALID, 0 ,0, SvtSaveOptions::ODFVER_010}
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
            MAP_CONST_C(      PROPERTY_VERTICALALIGN,   STYLE,    VERTICAL_ALIGN,       XML_RPT_ALGINMENT, 0 ),
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
            MAP_CONST_C(      PROPERTY_VERTICALALIGN,
                                                STYLE,    VERTICAL_ALIGN,       XML_RPT_ALGINMENT, 0 ),
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
        MAP_CONST_S(    "Width",                 STYLE,     COLUMN_WIDTH,           XML_TYPE_PROP_TABLE_COLUMN|XML_TYPE_MEASURE, 0 ),
        MAP_END()
    };
    return aXMLColumnStylesProperties;
}
// -----------------------------------------------------------------------------
const SvXMLEnumMapEntry* OXMLHelper::GetReportPrintOptions()
{
    static SvXMLEnumMapEntry s_aXML_EnumMap[] =
    {
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
        { XML_BEFORE_SECTION,       report::ForceNewPage::BEFORE_SECTION },
        { XML_AFTER_SECTION,        report::ForceNewPage::AFTER_SECTION },
        { XML_BEFORE_AFTER_SECTION, report::ForceNewPage::BEFORE_AFTER_SECTION },
        { XML_TOKEN_INVALID, 0 }
    };
    return s_aXML_EnumMap;
}
// -----------------------------------------------------------------------------
const SvXMLEnumMapEntry* OXMLHelper::GetKeepTogetherOptions()
{
    static SvXMLEnumMapEntry s_aXML_EnumMap[] =
    {
        { XML_WHOLE_GROUP,          report::KeepTogether::WHOLE_GROUP },
        { XML_WITH_FIRST_DETAIL,    report::KeepTogether::WITH_FIRST_DETAIL },
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
        { XML_TOKEN_INVALID, 0 }
    };
    return s_aXML_EnumMap;
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
void OXMLHelper::copyStyleElements(const bool _bOld,const ::rtl::OUString& _sStyleName,const SvXMLStylesContext* _pAutoStyles,const uno::Reference<beans::XPropertySet>& _xProp)
{
    if ( !_xProp.is() || _sStyleName.isEmpty() || !_pAutoStyles )
        return;
    XMLPropStyleContext* pAutoStyle = PTR_CAST(XMLPropStyleContext,_pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL,_sStyleName));
    if ( pAutoStyle )
    {
        ::com::sun::star::awt::FontDescriptor aFont;
        static comphelper::PropertyMapEntry pMap[] =
        {
            {PROPERTY_FONTNAME.ascii,         static_cast<sal_uInt16>(PROPERTY_FONTNAME.length),          PROPERTY_ID_FONTNAME,           &::getCppuType(&aFont.Name)         ,PropertyAttribute::BOUND,0},
            {PROPERTY_CHARFONTHEIGHT.ascii,   static_cast<sal_uInt16>(PROPERTY_CHARFONTHEIGHT.length),    PROPERTY_ID_FONTHEIGHT,         &::getCppuType(&aFont.Height)       ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTWIDTH.ascii,        static_cast<sal_uInt16>(PROPERTY_FONTWIDTH.length),         PROPERTY_ID_FONTWIDTH,          &::getCppuType(&aFont.Width)        ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTSTYLENAME.ascii,    static_cast<sal_uInt16>(PROPERTY_FONTSTYLENAME.length),     PROPERTY_ID_FONTSTYLENAME,      &::getCppuType(&aFont.StyleName)    ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTFAMILY.ascii,       static_cast<sal_uInt16>(PROPERTY_FONTFAMILY.length),        PROPERTY_ID_FONTFAMILY,         &::getCppuType(&aFont.Family)       ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTCHARSET.ascii,      static_cast<sal_uInt16>(PROPERTY_FONTCHARSET.length),       PROPERTY_ID_FONTCHARSET,        &::getCppuType(&aFont.CharSet)      ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTPITCH.ascii,        static_cast<sal_uInt16>(PROPERTY_FONTPITCH.length),         PROPERTY_ID_FONTPITCH,          &::getCppuType(&aFont.Pitch)        ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTCHARWIDTH.ascii,    static_cast<sal_uInt16>(PROPERTY_FONTCHARWIDTH.length),     PROPERTY_ID_FONTCHARWIDTH,      &::getCppuType(&aFont.CharacterWidth),PropertyAttribute::BOUND,0},
            {PROPERTY_FONTWEIGHT.ascii,       static_cast<sal_uInt16>(PROPERTY_FONTWEIGHT.length),        PROPERTY_ID_FONTWEIGHT,         &::getCppuType(&aFont.Weight)       ,PropertyAttribute::BOUND,0},
            {PROPERTY_CHARPOSTURE.ascii,      static_cast<sal_uInt16>(PROPERTY_CHARPOSTURE.length),       PROPERTY_ID_FONTSLANT,          &::getCppuType(&aFont.Slant)        ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTUNDERLINE.ascii,    static_cast<sal_uInt16>(PROPERTY_FONTUNDERLINE.length),     PROPERTY_ID_FONTUNDERLINE,      &::getCppuType(&aFont.Underline)    ,PropertyAttribute::BOUND,0},
            {PROPERTY_CHARSTRIKEOUT.ascii,    static_cast<sal_uInt16>(PROPERTY_CHARSTRIKEOUT.length),     PROPERTY_ID_FONTSTRIKEOUT,      &::getCppuType(&aFont.Strikeout)    ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTORIENTATION.ascii,  static_cast<sal_uInt16>(PROPERTY_FONTORIENTATION.length),   PROPERTY_ID_FONTORIENTATION,    &::getCppuType(&aFont.Orientation)  ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTKERNING.ascii,      static_cast<sal_uInt16>(PROPERTY_FONTKERNING.length),       PROPERTY_ID_FONTKERNING,        &::getCppuType(&aFont.Kerning)      ,PropertyAttribute::BOUND,0},
            {PROPERTY_CHARWORDMODE.ascii,     static_cast<sal_uInt16>(PROPERTY_CHARWORDMODE.length),      PROPERTY_ID_FONTWORDLINEMODE,   &::getCppuType(&aFont.WordLineMode) ,PropertyAttribute::BOUND,0},
            {PROPERTY_FONTTYPE.ascii,         static_cast<sal_uInt16>(PROPERTY_FONTTYPE.length),          PROPERTY_ID_FONTTYPE,           &::getCppuType(&aFont.Type)         ,PropertyAttribute::BOUND,0},
            { NULL, 0, 0, NULL, 0, 0 }
        };
        try
        {
            pAutoStyle->FillPropertySet(_xProp);
            if ( _bOld && _xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_CHARHIDDEN) )
                _xProp->setPropertyValue(PROPERTY_CHARHIDDEN,uno::makeAny(sal_False));

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
            xProp->getPropertyValue(PROPERTY_CHARPOSTURE) >>=           aFont.Slant;
            xProp->getPropertyValue(PROPERTY_FONTUNDERLINE) >>=         aFont.Underline;
            xProp->getPropertyValue(PROPERTY_CHARSTRIKEOUT) >>=         aFont.Strikeout;
            xProp->getPropertyValue(PROPERTY_FONTORIENTATION) >>=   aFont.Orientation;
            xProp->getPropertyValue(PROPERTY_FONTKERNING) >>=       aFont.Kerning;
            xProp->getPropertyValue(PROPERTY_CHARWORDMODE) >>=  aFont.WordLineMode;
            xProp->getPropertyValue(PROPERTY_FONTTYPE) >>=          aFont.Type;
            uno::Reference<report::XReportControlFormat> xReportControlModel(_xProp,uno::UNO_QUERY);
            if ( xReportControlModel.is() && !aFont.Name.isEmpty() )
            {
                try
                {
                    xReportControlModel->setFontDescriptor(aFont);
                }
                catch(const beans::UnknownPropertyException &){}
            }
           }
        catch(uno::Exception&)
        {
            OSL_FAIL("OXMLHelper::copyStyleElements -> exception catched");
        }
    }
}
// -----------------------------------------------------------------------------
uno::Reference<beans::XPropertySet> OXMLHelper::createBorderPropertySet()
{
    static comphelper::PropertyMapEntry pMap[] =
    {
        {PROPERTY_BORDERLEFT.ascii,   static_cast<sal_uInt16>(PROPERTY_BORDERLEFT.length),        0,          &::getCppuType((const table::BorderLine*)0)         ,PropertyAttribute::BOUND,0},
        {PROPERTY_BORDERRIGHT.ascii,  static_cast<sal_uInt16>(PROPERTY_BORDERRIGHT.length),       1,          &::getCppuType((const table::BorderLine*)0)         ,PropertyAttribute::BOUND,0},
        {PROPERTY_BORDERTOP.ascii,    static_cast<sal_uInt16>(PROPERTY_BORDERTOP.length),         2,          &::getCppuType((const table::BorderLine*)0)         ,PropertyAttribute::BOUND,0},
        {PROPERTY_BORDERBOTTOM.ascii, static_cast<sal_uInt16>(PROPERTY_BORDERBOTTOM.length),      3,          &::getCppuType((const table::BorderLine*)0)         ,PropertyAttribute::BOUND,0},
        { NULL, 0, 0, NULL, 0, 0 }
    };
    return comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(pMap));
}
// -----------------------------------------------------------------------------
SvXMLTokenMap* OXMLHelper::GetReportElemTokenMap()
{
    static SvXMLTokenMapEntry aElemTokenMap[]=
    {
        { XML_NAMESPACE_REPORT, XML_REPORT_HEADER,              XML_TOK_REPORT_HEADER           },
        { XML_NAMESPACE_REPORT, XML_PAGE_HEADER ,               XML_TOK_PAGE_HEADER             },
        { XML_NAMESPACE_REPORT, XML_GROUP,                      XML_TOK_GROUP                   },
        { XML_NAMESPACE_REPORT, XML_DETAIL      ,               XML_TOK_DETAIL                  },
        { XML_NAMESPACE_REPORT, XML_PAGE_FOOTER ,               XML_TOK_PAGE_FOOTER             },
        { XML_NAMESPACE_REPORT, XML_REPORT_FOOTER,              XML_TOK_REPORT_FOOTER           },
        { XML_NAMESPACE_REPORT, XML_HEADER_ON_NEW_PAGE,         XML_TOK_HEADER_ON_NEW_PAGE      },
        { XML_NAMESPACE_REPORT, XML_FOOTER_ON_NEW_PAGE,         XML_TOK_FOOTER_ON_NEW_PAGE      },
        { XML_NAMESPACE_REPORT, XML_COMMAND_TYPE,               XML_TOK_COMMAND_TYPE            },
        { XML_NAMESPACE_REPORT, XML_COMMAND,                    XML_TOK_COMMAND                 },
        { XML_NAMESPACE_REPORT, XML_FILTER,                     XML_TOK_FILTER                  },
        { XML_NAMESPACE_REPORT, XML_CAPTION,                    XML_TOK_CAPTION                 },
        { XML_NAMESPACE_REPORT, XML_ESCAPE_PROCESSING,          XML_TOK_ESCAPE_PROCESSING       },
        { XML_NAMESPACE_REPORT, XML_FUNCTION,                   XML_TOK_REPORT_FUNCTION         },
        { XML_NAMESPACE_OFFICE, XML_MIMETYPE,                   XML_TOK_REPORT_MIMETYPE         },
        { XML_NAMESPACE_DRAW,   XML_NAME,                       XML_TOK_REPORT_NAME             },
        { XML_NAMESPACE_REPORT, XML_MASTER_DETAIL_FIELDS,       XML_TOK_MASTER_DETAIL_FIELDS    },
        { XML_NAMESPACE_DRAW,   XML_FRAME,                      XML_TOK_SUB_FRAME               },
        XML_TOKEN_MAP_END
    };
    return new SvXMLTokenMap( aElemTokenMap );
}
// -----------------------------------------------------------------------------
SvXMLTokenMap* OXMLHelper::GetSubDocumentElemTokenMap()
{
    static SvXMLTokenMapEntry aElemTokenMap[]=
    {
        { XML_NAMESPACE_REPORT, XML_MASTER_DETAIL_FIELD,    XML_TOK_MASTER_DETAIL_FIELD},
        { XML_NAMESPACE_REPORT, XML_MASTER,                 XML_TOK_MASTER},
        { XML_NAMESPACE_REPORT, XML_DETAIL,                 XML_TOK_SUB_DETAIL},
        XML_TOKEN_MAP_END
    };
    return new SvXMLTokenMap( aElemTokenMap );
}
// -----------------------------------------------------------------------------
const SvXMLEnumMapEntry* OXMLHelper::GetImageScaleOptions()
{
       static SvXMLEnumMapEntry s_aXML_EnumMap[] =
       {
               { XML_ISOTROPIC,        awt::ImageScaleMode::Isotropic },
               { XML_ANISOTROPIC,      awt::ImageScaleMode::Anisotropic },
               { XML_TOKEN_INVALID, 0 }
       };
       return s_aXML_EnumMap;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
