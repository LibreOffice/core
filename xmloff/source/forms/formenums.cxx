/*************************************************************************
 *
 *  $RCSfile: formenums.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 16:09:05 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _XMLOFF_FORMENUMS_HXX_
#include "formenums.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_FORM_FORMSUBMITENCODING_HPP_
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITMETHOD_HPP_
#include <com/sun/star/form/FormSubmitMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_NAVIGATIONBARMODE_HPP_
#include <com/sun/star/form/NavigationBarMode.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_TABULATORCYCLE_HPP_
#include <com/sun/star/form/TabulatorCycle.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_LISTSOURCETYPE_HPP_
#include <com/sun/star/form/ListSourceType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWIDTH_HPP_
#include <com/sun/star/awt/FontWidth.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTEMPHASISMARK_HPP_
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTRELIEF_HPP_
#include <com/sun/star/awt/FontRelief.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_VISUALEFFECT_HPP_
#include <com/sun/star/awt/VisualEffect.hpp>
#endif
#ifndef _SV_WINTYPES_HXX
#include <vcl/wintypes.hxx>     // for check states
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::awt;
    using namespace ::xmloff::token;

    const SvXMLEnumMapEntry*    OEnumMapper::s_pEnumMap[OEnumMapper::KNOWN_ENUM_PROPERTIES] =
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
        NULL,
        NULL,
        NULL,
        NULL
    };

    //---------------------------------------------------------------------
    const SvXMLEnumMapEntry*    OEnumMapper::getEnumMap(EnumProperties _eProperty)
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
                        { XML_START,        TextAlign::LEFT },
                        { XML_CENTER,       TextAlign::CENTER },
                        { XML_END,          TextAlign::RIGHT },
                        { XML_JUSTIFY,      -1 },
                        { XML_JUSTIFIED,    -1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aTextAlignMap;
                };
                break;
                case epBorderWidth:
                {
                    static SvXMLEnumMapEntry aBorderTypeMap[] =
                    {
                        { XML_NONE,     0 },
                        { XML_HIDDEN,   0 },
                        { XML_SOLID,    2 },
                        { XML_DOUBLE,   2 },
                        { XML_DOTTED,   2 },
                        { XML_DASHED,   2 },
                        { XML_GROOVE,   1 },
                        { XML_RIDGE,    1 },
                        { XML_INSET,    1 },
                        { XML_OUTSET,   1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aBorderTypeMap;
                };
                break;

                case epFontEmphasis:
                {
                    static SvXMLEnumMapEntry aFontEmphasisMap[] =
                    {
                        { XML_NONE,     FontEmphasisMark::NONE },
                        { XML_DOT,      FontEmphasisMark::DOT },
                        { XML_CIRCLE,   FontEmphasisMark::CIRCLE },
                        { XML_DISC,     FontEmphasisMark::DISC },
                        { XML_ACCENT,   FontEmphasisMark::ACCENT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aFontEmphasisMap;
                }
                break;

                case epFontRelief:
                {
                    static SvXMLEnumMapEntry aFontReliefMap[] =
                    {
                        { XML_NONE,     FontRelief::NONE },
                        { XML_ENGRAVED, FontRelief::ENGRAVED },
                        { XML_EMBOSSED, FontRelief::EMBOSSED },
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

                case epOrientation:
                {
                    static SvXMLEnumMapEntry aOrientationMap[] =
                    {
                        { XML_HORIZONTAL,   ScrollBarOrientation::HORIZONTAL },
                        { XML_VERTICAL,     ScrollBarOrientation::VERTICAL },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aOrientationMap;
                }
                break;

                case epVisualEffect:
                {
                    static SvXMLEnumMapEntry aVisualEffectMap[] =
                    {
                        { XML_NONE, VisualEffect::NONE },
                        { XML_3D,   VisualEffect::LOOK3D },
                        { XML_FLAT, VisualEffect::FLAT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aVisualEffectMap;
                }
                break;

                case epImagePosition:
                {
                    static SvXMLEnumMapEntry aImagePositionMap[] =
                    {
                        { XML_START,  0 },
                        { XML_END,    1 },
                        { XML_TOP,    2 },
                        { XML_BOTTOM, 3 },
                        { XML_CENTER, -1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aImagePositionMap;
                }
                break;

                case epImageAlign:
                {
                    static SvXMLEnumMapEntry aImageAlignMap[] =
                    {
                        { XML_START,  0 },
                        { XML_CENTER, 1 },
                        { XML_END, 2 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aImageAlignMap;
                }
                break;
            }
        }

        return rReturn;
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

