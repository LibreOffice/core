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
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#define _CONNECTIVITY_PROPERTYIDS_HXX_

// this define has to be set to split the names into different dll's or so's
// every dll has his own set of property names
#include <rtl/ustring.hxx>
#include <map>
#include "connectivity/dbtoolsdllapi.hxx"

namespace dbtools
{
    class OOO_DLLPUBLIC_DBTOOLS OPropertyMap
    {
        ::std::map<sal_Int32 , rtl_uString*> m_aPropertyMap;

        OUString fillValue(sal_Int32 _nIndex);
    public:
        OPropertyMap()
        {
        }
        ~OPropertyMap();
        OUString getNameByIndex(sal_Int32 _nIndex) const;
    };
}

namespace connectivity
{
    namespace CONNECTIVITY_PROPERTY_NAME_SPACE
    {
        typedef const sal_Char* (*PVFN)();

        struct OOO_DLLPUBLIC_DBTOOLS UStringDescription
        {
            const sal_Char* pZeroTerminatedName;
            sal_Int32 nLength;

            UStringDescription(PVFN _fCharFkt);
            operator OUString() const { return OUString(pZeroTerminatedName,nLength,RTL_TEXTENCODING_ASCII_US); }
            ~UStringDescription();
        private:
            UStringDescription();
        };
    }
}


//------------------------------------------------------------------------------
#define DECL_PROP1IMPL(varname, type) \
pProperties[nPos++] = ::com::sun::star::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_##varname), PROPERTY_ID_##varname, ::getCppuType(static_cast< type*>(0)),
//------------------------------------------------------------------------------
#define DECL_PROP0(varname, type)   \
    DECL_PROP1IMPL(varname, type) 0)
//------------------------------------------------------------------------------
#define DECL_BOOL_PROP1IMPL(varname) \
        pProperties[nPos++] = ::com::sun::star::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_##varname), PROPERTY_ID_##varname, ::getBooleanCppuType(),
//------------------------------------------------------------------------------
#define DECL_BOOL_PROP0(varname)    \
    DECL_BOOL_PROP1IMPL(varname) 0)


#define PROPERTY_ID_QUERYTIMEOUT                    1
#define PROPERTY_ID_MAXFIELDSIZE                    2
#define PROPERTY_ID_MAXROWS                         3
#define PROPERTY_ID_CURSORNAME                      4
#define PROPERTY_ID_RESULTSETCONCURRENCY            5
#define PROPERTY_ID_RESULTSETTYPE                   6
#define PROPERTY_ID_FETCHDIRECTION                  7
#define PROPERTY_ID_FETCHSIZE                       8
#define PROPERTY_ID_ESCAPEPROCESSING                9
#define PROPERTY_ID_USEBOOKMARKS                    10
// Column
#define PROPERTY_ID_NAME                            11
#define PROPERTY_ID_TYPE                            12
#define PROPERTY_ID_TYPENAME                        13
#define PROPERTY_ID_PRECISION                       14
#define PROPERTY_ID_SCALE                           15
#define PROPERTY_ID_ISNULLABLE                      16
#define PROPERTY_ID_ISAUTOINCREMENT                 17
#define PROPERTY_ID_ISROWVERSION                    18
#define PROPERTY_ID_DESCRIPTION                     19
#define PROPERTY_ID_DEFAULTVALUE                    20

#define PROPERTY_ID_REFERENCEDTABLE                 21
#define PROPERTY_ID_UPDATERULE                      22
#define PROPERTY_ID_DELETERULE                      23
#define PROPERTY_ID_CATALOG                         24
#define PROPERTY_ID_ISUNIQUE                        25
#define PROPERTY_ID_ISPRIMARYKEYINDEX               26
#define PROPERTY_ID_ISCLUSTERED                     27
#define PROPERTY_ID_ISASCENDING                     28
#define PROPERTY_ID_SCHEMANAME                      29
#define PROPERTY_ID_CATALOGNAME                     30

#define PROPERTY_ID_COMMAND                         31
#define PROPERTY_ID_CHECKOPTION                     32
#define PROPERTY_ID_PASSWORD                        33
#define PROPERTY_ID_RELATEDCOLUMN                   34

#define PROPERTY_ID_FUNCTION                        35
#define PROPERTY_ID_TABLENAME                       36
#define PROPERTY_ID_REALNAME                        37
#define PROPERTY_ID_DBASEPRECISIONCHANGED           38
#define PROPERTY_ID_ISCURRENCY                      39
#define PROPERTY_ID_ISBOOKMARKABLE                  40

#define PROPERTY_ID_INVALID_INDEX                   41
#define PROPERTY_ID_HY010                           43
#define PROPERTY_ID_LABEL                           44
#define PROPERTY_ID_DELIMITER                       45
#define PROPERTY_ID_FORMATKEY                       46
#define PROPERTY_ID_LOCALE                          47
#define PROPERTY_ID_IM001                           48

#define PROPERTY_ID_AUTOINCREMENTCREATION           49

#define PROPERTY_ID_PRIVILEGES                      50
#define PROPERTY_ID_HAVINGCLAUSE                    51

#define PROPERTY_ID_ISSIGNED                        52
#define PROPERTY_ID_AGGREGATEFUNCTION               53
#define PROPERTY_ID_ISSEARCHABLE                    54

#define PROPERTY_ID_APPLYFILTER                     55
#define PROPERTY_ID_FILTER                          56
#define PROPERTY_ID_MASTERFIELDS                    57
#define PROPERTY_ID_DETAILFIELDS                    58
#define PROPERTY_ID_FIELDTYPE                       59
#define PROPERTY_ID_VALUE                           60
#define PROPERTY_ID_ACTIVE_CONNECTION               61

#endif // _CONNECTIVITY_PROPERTYIDS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
