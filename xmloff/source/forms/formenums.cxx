/*************************************************************************
 *
 *  $RCSfile: formenums.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:39:38 $
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
            }
        }

        return rReturn;
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.8.188.1  2003/10/01 09:55:21  fs
 *  #i18994# merging the changes from the CWS fs002
 *
 *  Revision 1.8.184.1  2003/09/25 14:28:39  fs
 *  #18994# merging the changes from cws_srx645_fs002 branch
 *
 *  Revision 1.8.180.1  2003/09/18 14:00:38  fs
 *  #18995# changes for binding list boxes to cells, while exchanging selection indexes instead of strings
 *
 *  Revision 1.8  2001/06/29 21:07:14  dvo
 *  #86004# changes sXML_* strings to XML_* tokens
 *
 *  Revision 1.7  2001/06/15 10:37:07  dvo
 *  #86004# #88312#
 *  - changed convertEnum and appropriate maps to use token constants (rahter than sal_Char*)
 *  - user index marks in the process
 *
 *  Revision 1.6  2001/06/07 12:25:52  fs
 *  #86096# enums for FontEmphasis and FontRelief
 *
 *  Revision 1.5  2000/12/19 12:13:57  fs
 *  some changes ... now the exported styles are XSL conform
 *
 *  Revision 1.4  2000/12/19 08:42:19  fs
 *  removed the epFontWidth
 *
 *  Revision 1.3  2000/12/18 15:14:35  fs
 *  some changes ... now exporting/importing styles
 *
 *  Revision 1.2  2000/12/06 17:28:05  fs
 *  changes for the formlayer import - still under construction
 *
 *  Revision 1.1  2000/11/17 19:01:56  fs
 *  initial checkin - export and/or import the applications form layer
 *
 *
 *  Revision 1.0 15.11.00 11:54:17  fs
 ************************************************************************/

