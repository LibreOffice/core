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

#include "formenums.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/form/NavigationBarMode.hpp>
#include <com/sun/star/form/TabulatorCycle.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <vcl/wintypes.hxx>		// for check states
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::awt;
    using namespace ::binfilter::xmloff::token;

    const SvXMLEnumMapEntry*	OEnumMapper::s_pEnumMap[OEnumMapper::KNOWN_ENUM_PROPERTIES] =
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };

    //---------------------------------------------------------------------
    const SvXMLEnumMapEntry*	OEnumMapper::getEnumMap(EnumProperties _eProperty)
    {
        OSL_ENSURE(_eProperty < KNOWN_ENUM_PROPERTIES, "OEnumMapper::getEnumMap: invalid index (this will crash)!");

        const SvXMLEnumMapEntry*& rReturn = s_pEnumMap[_eProperty];
        if (!rReturn)
        {
            // the map for this property is not initialized yet
             switch (_eProperty)
            {
                // FormSubmitEncoding
                case epSubmitEncoding:
                {
                    static SvXMLEnumMapEntry aSubmitEncodingMap[] =
                    {
                        { XML_APPLICATION_X_WWW_FORM_URLENCODED, FormSubmitEncoding_URL },
                        { XML_MULTIPART_FORMDATA, FormSubmitEncoding_MULTIPART },
                        { XML_APPLICATION_TEXT, FormSubmitEncoding_TEXT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aSubmitEncodingMap;
                }
                break;
                // FormSubmitMethod
                case epSubmitMethod:
                {
                    static SvXMLEnumMapEntry aSubmitMethodMap[] =
                    {
                        { XML_GET, FormSubmitMethod_GET },
                        { XML_POST, FormSubmitMethod_POST },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aSubmitMethodMap;
                }
                break;
                // CommandType
                case epCommandType:
                {
                    static SvXMLEnumMapEntry aCommandTypeMap[] =
                    {
                        { XML_TABLE, CommandType::TABLE },
                        { XML_QUERY, CommandType::QUERY },
                        { XML_COMMAND, CommandType::COMMAND },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aCommandTypeMap;
                }
                break;
                // NavigationBarMode
                case epNavigationType:
                {
                    static SvXMLEnumMapEntry aNavigationTypeMap[] =
                    {
                        { XML_NONE, NavigationBarMode_NONE },
                        { XML_CURRENT, NavigationBarMode_CURRENT },
                        { XML_PARENT, NavigationBarMode_PARENT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aNavigationTypeMap;
                };
                break;
                // TabulatorCycle
                case epTabCyle:
                {
                    static SvXMLEnumMapEntry aTabulytorCycleMap[] =
                    {
                        { XML_RECORDS, TabulatorCycle_RECORDS },
                        { XML_CURRENT, TabulatorCycle_CURRENT },
                        { XML_PAGE, TabulatorCycle_PAGE },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aTabulytorCycleMap;
                };
                break;
                // FormButtonType
                case epButtonType:
                {
                    static SvXMLEnumMapEntry aFormButtonTypeMap[] =
                    {
                        { XML_PUSH, FormButtonType_PUSH },
                        { XML_SUBMIT, FormButtonType_SUBMIT },
                        { XML_RESET, FormButtonType_RESET },
                        { XML_URL, FormButtonType_URL },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aFormButtonTypeMap;
                };
                break;
                // ListSourceType
                case epListSourceType:
                {
                    static SvXMLEnumMapEntry aListSourceTypeMap[] =
                    {
                        { XML_VALUE_LIST, ListSourceType_VALUELIST },
                        { XML_TABLE, ListSourceType_TABLE },
                        { XML_QUERY, ListSourceType_QUERY },
                        { XML_SQL, ListSourceType_SQL },
                        { XML_SQL_PASS_THROUGH, ListSourceType_SQLPASSTHROUGH },
                        { XML_TABLE_FIELDS, ListSourceType_TABLEFIELDS },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aListSourceTypeMap;
                };
                break;
                // check state of a checkbox
                case epCheckState:
                {
                    static SvXMLEnumMapEntry aCheckStateMap[] =
                    {
                        { XML_UNCHECKED, STATE_NOCHECK },
                        { XML_CHECKED, STATE_CHECK },
                        { XML_UNKNOWN, STATE_DONTKNOW },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aCheckStateMap;
                };
                break;
                case epTextAlign:
                {
                    static SvXMLEnumMapEntry aTextAlignMap[] =
                    {
                        { XML_START,		TextAlign::LEFT },
                        { XML_CENTER,		TextAlign::CENTER },
                        { XML_END,			TextAlign::RIGHT },
                        { XML_JUSTIFY,		-1 },
                        { XML_JUSTIFIED,	-1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aTextAlignMap;
                };
                break;
                case epBorderWidth:
                {
                    static SvXMLEnumMapEntry aBorderTypeMap[] =
                    {
                        { XML_NONE,	    0 },
                        { XML_HIDDEN, 	0 },
                        { XML_SOLID, 	2 },
                        { XML_DOUBLE,	2 },
                        { XML_DOTTED, 	2 },
                        { XML_DASHED, 	2 },
                        { XML_GROOVE, 	1 },
                        { XML_RIDGE,  	1 },
                        { XML_INSET,  	1 },
                        { XML_OUTSET, 	1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aBorderTypeMap;
                };
                break;

                case epFontEmphasis:
                {
                    static SvXMLEnumMapEntry aFontEmphasisMap[] =
                    {
                        { XML_NONE,	    FontEmphasisMark::NONE },
                        { XML_DOT,		FontEmphasisMark::DOT },
                        { XML_CIRCLE,	FontEmphasisMark::CIRCLE },
                        { XML_DISC,	    FontEmphasisMark::DISC },
                        { XML_ACCENT,	FontEmphasisMark::ACCENT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aFontEmphasisMap;
                }
                break;

                case epFontRelief:
                {
                    static SvXMLEnumMapEntry aFontReliefMap[] =
                    {
                        { XML_NONE,		FontRelief::NONE },
                        { XML_ENGRAVED,	FontRelief::ENGRAVED },
                        { XML_EMBOSSED,	FontRelief::EMBOSSED },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aFontReliefMap;
                }
                break;

                case epListLinkageType:
                {
                    static SvXMLEnumMapEntry aListLinkageMap[] =
                    {
                        { XML_SELECTION,            0 },
                        { XML_SELECTION_INDEXES,    1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aListLinkageMap;
                }
                break;
            }
        }

        return rReturn;
    }

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
