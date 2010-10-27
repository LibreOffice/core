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
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#define _CONNECTIVITY_PROPERTYIDS_HXX_

// this define has to be set to split the names into different dll's or so's
// every dll has his own set of property names
#include <rtl/ustring.hxx>
#ifndef _MAP_
#include <map>
#endif
#include "connectivity/dbtoolsdllapi.hxx"

namespace dbtools
{
    class OOO_DLLPUBLIC_DBTOOLS OPropertyMap
    {
        ::std::map<sal_Int32 , rtl_uString*> m_aPropertyMap;

        ::rtl::OUString fillValue(sal_Int32 _nIndex);
    public:
        OPropertyMap()
        {
        }
        ~OPropertyMap();
        ::rtl::OUString getNameByIndex(sal_Int32 _nIndex) const;
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
            operator ::rtl::OUString() const { return ::rtl::OUString(pZeroTerminatedName,nLength,RTL_TEXTENCODING_ASCII_US); }
            ~UStringDescription();
        private:
            UStringDescription();
        };
    }
}


//------------------------------------------------------------------------------
#define DECL_PROP1IMPL(varname, type) \
pProperties[nPos++] = ::com::sun::star::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_##varname), PROPERTY_ID_##varname, ::getCppuType(reinterpret_cast< type*>(NULL)),
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
