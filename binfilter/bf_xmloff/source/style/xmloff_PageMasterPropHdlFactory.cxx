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

#include "PageMasterPropHdlFactory.hxx"

#include "xmlbahdl.hxx"
#include "NamedBoolPropertyHdl.hxx"
#include "XMLTextColumnsPropertyHandler.hxx"
#include "XMLConstantsPropertyHandler.hxx"

#include "PageMasterPropHdl.hxx"
#include "PageMasterStyleMap.hxx"

#include <com/sun/star/text/TextGridMode.hpp>
namespace binfilter {


using namespace ::rtl;
using namespace ::binfilter::xmloff::token;
using namespace ::com::sun::star;


SvXMLEnumMapEntry aXML_TextGridMode_ConstantMap[] =
{
    { XML_NONE,         text::TextGridMode::NONE },
    { XML_LINE,	        text::TextGridMode::LINES },
    { XML_BOTH,         text::TextGridMode::LINES_AND_CHARS },
    { XML_TOKEN_INVALID, 0 }
};

//______________________________________________________________________________

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

    XMLPropertyHandler* pHdl = (XMLPropertyHandler*) XMLPropertyHandlerFactory::GetPropertyHandler( nType );
    if( !pHdl )
    {
        switch( nType )
        {
            case XML_PM_TYPE_PAGESTYLELAYOUT:
                pHdl = new XMLPMPropHdl_PageStyleLayout();
            break;
            case XML_PM_TYPE_NUMFORMAT:
                pHdl = new XMLPMPropHdl_NumFormat();
            break;
            case XML_PM_TYPE_NUMLETTERSYNC:
                pHdl = new XMLPMPropHdl_NumLetterSync();
            break;
            case XML_PM_TYPE_PAPERTRAYNUMBER:
                pHdl = new XMLPMPropHdl_PaperTrayNumber();
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
                pHdl = new XMLPMPropHdl_CenterHorizontal();
            break;
            case XML_PM_TYPE_CENTER_VERTICAL:
                pHdl = new XMLPMPropHdl_CenterVertical();
            break;
            case XML_TYPE_TEXT_COLUMNS:
                pHdl = new XMLTextColumnsPropertyHandler;
            break;
            case XML_TYPE_LAYOUT_GRID_MODE:
                pHdl = new XMLConstantsPropertyHandler(
                    aXML_TextGridMode_ConstantMap, XML_NONE );
            break;
        }

        if( pHdl )
            PutHdlCache( nType, pHdl );
    }
    return pHdl;
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
