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

#include "PageMasterPropHdlFactory.hxx"
#include <enummaps.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmlbahdl.hxx"
#include <xmloff/NamedBoolPropertyHdl.hxx>
#include <XMLTextColumnsPropertyHandler.hxx>
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#include "PageMasterPropHdl.hxx"
#include <xmloff/PageMasterStyleMap.hxx>
#include <com/sun/star/text/TextGridMode.hpp>
#include <xmloff/EnumPropertyHdl.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <XMLFillBitmapSizePropertyHandler.hxx>
#include <XMLBitmapLogicalSizePropertyHandler.hxx>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <XMLBitmapRepeatOffsetPropertyHandler.hxx>

using namespace ::xmloff::token;
using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;

static SvXMLEnumMapEntry<sal_uInt16> const aXML_TextGridMode_ConstantMap[] =
{
    { XML_NONE,         text::TextGridMode::NONE },
    { XML_LINE,         text::TextGridMode::LINES },
    { XML_BOTH,         text::TextGridMode::LINES_AND_CHARS },
    { XML_TOKEN_INVALID, 0 }
};

XMLPageMasterPropHdlFactory::XMLPageMasterPropHdlFactory() :
    XMLPropertyHandlerFactory()
{
}

XMLPageMasterPropHdlFactory::~XMLPageMasterPropHdlFactory()
{
}

const XMLPropertyHandler* XMLPageMasterPropHdlFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    nType &= MID_FLAG_MASK;

    XMLPropertyHandler* pHdl = const_cast<XMLPropertyHandler*>(XMLPropertyHandlerFactory::GetPropertyHandler( nType ));
    if( !pHdl )
    {
        switch( nType )
        {
            case XML_PM_TYPE_PAGESTYLELAYOUT:
                pHdl = new XMLPMPropHdl_PageStyleLayout;
            break;
            case XML_PM_TYPE_NUMFORMAT:
                pHdl = new XMLPMPropHdl_NumFormat;
            break;
            case XML_PM_TYPE_NUMLETTERSYNC:
                pHdl = new XMLPMPropHdl_NumLetterSync;
            break;
            case XML_PM_TYPE_PAPERTRAYNUMBER:
                pHdl = new XMLPMPropHdl_PaperTrayNumber;
            break;
            case XML_PM_TYPE_PRINTORIENTATION:
                pHdl = new XMLNamedBoolPropertyHdl(
                    GetXMLToken( XML_LANDSCAPE ),
                    GetXMLToken( XML_PORTRAIT ) );
            break;
            case XML_PM_TYPE_PRINTANNOTATIONS:
                pHdl = new XMLPMPropHdl_Print( XML_ANNOTATIONS );
            break;
            case XML_PM_TYPE_PRINTCHARTS:
                pHdl = new XMLPMPropHdl_Print( XML_CHARTS );
            break;
            case XML_PM_TYPE_PRINTDRAWING:
                pHdl = new XMLPMPropHdl_Print( XML_DRAWINGS );
            break;
            case XML_PM_TYPE_PRINTFORMULAS:
                pHdl = new XMLPMPropHdl_Print( XML_FORMULAS );
            break;
            case XML_PM_TYPE_PRINTGRID:
                pHdl = new XMLPMPropHdl_Print( XML_GRID );
            break;
            case XML_PM_TYPE_PRINTHEADERS:
                pHdl = new XMLPMPropHdl_Print( XML_HEADERS );
            break;
            case XML_PM_TYPE_PRINTOBJECTS:
                pHdl = new XMLPMPropHdl_Print( XML_OBJECTS );
            break;
            case XML_PM_TYPE_PRINTZEROVALUES:
                pHdl = new XMLPMPropHdl_Print( XML_ZERO_VALUES );
            break;
            case XML_PM_TYPE_PRINTPAGEORDER:
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken( XML_TTB ),
                                                    GetXMLToken( XML_LTR ) );
            break;
            case XML_PM_TYPE_FIRSTPAGENUMBER:
                pHdl = new XMLNumberNonePropHdl( XML_CONTINUE, 2 );
            break;
            case XML_PM_TYPE_CENTER_HORIZONTAL:
                pHdl = new XMLPMPropHdl_CenterHorizontal;
            break;
            case XML_PM_TYPE_CENTER_VERTICAL:
                pHdl = new XMLPMPropHdl_CenterVertical;
            break;
            case XML_TYPE_TEXT_COLUMNS:
                pHdl = new XMLTextColumnsPropertyHandler;
            break;
            case XML_TYPE_LAYOUT_GRID_MODE:
                pHdl = new XMLConstantsPropertyHandler(
                    aXML_TextGridMode_ConstantMap, XML_NONE );
            break;

            case XML_SW_TYPE_FILLSTYLE:
                pHdl = new XMLEnumPropertyHdl( aXML_FillStyle_EnumMap );
                break;
            case XML_SW_TYPE_FILLBITMAPSIZE:
                pHdl = new XMLFillBitmapSizePropertyHandler();
                break;
            case XML_SW_TYPE_LOGICAL_SIZE:
                pHdl = new XMLBitmapLogicalSizePropertyHandler();
                break;
            case XML_SW_TYPE_BITMAP_REFPOINT:
                pHdl = new XMLEnumPropertyHdl( aXML_RefPoint_EnumMap );
                break;
            case XML_SW_TYPE_BITMAP_MODE:
                pHdl = new XMLEnumPropertyHdl( aXML_BitmapMode_EnumMap );
                break;
            case XML_SW_TYPE_BITMAPREPOFFSETX:
            case XML_SW_TYPE_BITMAPREPOFFSETY:
                pHdl = new XMLBitmapRepeatOffsetPropertyHandler(XML_SW_TYPE_BITMAPREPOFFSETX == nType);
                break;

            default:
            {
                OSL_ENSURE(false, "XMLPropertyHandler missing (!)");
                break;
            }
        }

        if( pHdl )
            PutHdlCache( nType, pHdl );
    }
    return pHdl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
