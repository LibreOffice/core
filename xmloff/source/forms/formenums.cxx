/*************************************************************************
 *
 *  $RCSfile: formenums.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-17 19:01:56 $
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

#ifndef _SV_WINTYPES_HXX
#include <vcl/wintypes.hxx>     // for check states
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;

    SvXMLEnumMapEntry*  OEnumMapper::s_pEnumMap[OEnumMapper::KNOWN_ENUM_PROPERTIES] =
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };

    //---------------------------------------------------------------------
    SvXMLEnumMapEntry*  OEnumMapper::getEnumMap(EnumProperties _eProperty)
    {
        OSL_ENSURE(_eProperty < KNOWN_ENUM_PROPERTIES, "OEnumMapper::getEnumMap: invalid index (this will crash)!");

        SvXMLEnumMapEntry*& rReturn = s_pEnumMap[_eProperty];
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
                        { "application/x-www-form-urlencoded", FormSubmitEncoding_URL },
                        { "multipart/formdata", FormSubmitEncoding_MULTIPART },
                        { "application/text", FormSubmitEncoding_TEXT },
                        { NULL, 0 }
                    };
                    rReturn = aSubmitEncodingMap;
                }
                break;
                // FormSubmitMethod
                case epSubmitMethod:
                {
                    static SvXMLEnumMapEntry aSubmitMethodMap[] =
                    {
                        { "get", FormSubmitMethod_GET },
                        { "post", FormSubmitMethod_POST },
                        { NULL, 0 }
                    };
                    rReturn = aSubmitMethodMap;
                }
                break;
                // CommandType
                case epCommandType:
                {
                    static SvXMLEnumMapEntry aCommandTypeMap[] =
                    {
                        { "table", CommandType::TABLE },
                        { "query", CommandType::QUERY },
                        { "command", CommandType::COMMAND },
                        { NULL, 0 }
                    };
                    rReturn = aCommandTypeMap;
                }
                break;
                // NavigationBarMode
                case epNavigationType:
                {
                    static SvXMLEnumMapEntry aNavigationTypeMap[] =
                    {
                        { "none", NavigationBarMode_NONE },
                        { "current", NavigationBarMode_CURRENT },
                        { "parent", NavigationBarMode_PARENT },
                        { NULL, 0 }
                    };
                    rReturn = aNavigationTypeMap;
                };
                break;
                // TabulatorCycle
                case epTabCyle:
                {
                    static SvXMLEnumMapEntry aTabulytorCycleMap[] =
                    {
                        { "records", TabulatorCycle_RECORDS },
                        { "current", TabulatorCycle_CURRENT },
                        { "page", TabulatorCycle_PAGE },
                        { NULL, 0 }
                    };
                    rReturn = aTabulytorCycleMap;
                };
                break;
                // FormButtonType
                case epButtonType:
                {
                    static SvXMLEnumMapEntry aFormButtonTypeMap[] =
                    {
                        { "push", FormButtonType_PUSH },
                        { "submit", FormButtonType_SUBMIT },
                        { "reset", FormButtonType_RESET },
                        { "url", FormButtonType_URL },
                        { NULL, 0 }
                    };
                    rReturn = aFormButtonTypeMap;
                };
                break;
                // ListSourceType
                case epListSourceType:
                {
                    static SvXMLEnumMapEntry aListSourceTypeMap[] =
                    {
                        { "value-list", ListSourceType_VALUELIST },
                        { "table", ListSourceType_TABLE },
                        { "query", ListSourceType_QUERY },
                        { "sql", ListSourceType_SQL },
                        { "sql-pass-through", ListSourceType_SQLPASSTHROUGH },
                        { "table-fields", ListSourceType_TABLEFIELDS },
                        { NULL, 0 }
                    };
                    rReturn = aListSourceTypeMap;
                };
                break;
                // check state of a checkbox
                case epCheckState:
                {
                    static SvXMLEnumMapEntry aCheckStateMap[] =
                    {
                        { "unchecked", STATE_NOCHECK },
                        { "checked", STATE_CHECK },
                        { "unknown", STATE_DONTKNOW },
                        { NULL, 0 }
                    };
                    rReturn = aCheckStateMap;
                };
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
 *
 *  Revision 1.0 15.11.00 11:54:17  fs
 ************************************************************************/

