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
#ifndef DBAUI_TYPEINFO_HXX
#define DBAUI_TYPEINFO_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <boost/shared_ptr.hpp>
#include <map>

namespace dbaui
{
// Based on these ids the language dependent OUString are fetched from the resource
const sal_uInt16 TYPE_UNKNOWN   = 0;
const sal_uInt16 TYPE_TEXT      = 1;
const sal_uInt16 TYPE_NUMERIC   = 2;
const sal_uInt16 TYPE_DATETIME  = 3;
const sal_uInt16 TYPE_DATE      = 4;
const sal_uInt16 TYPE_TIME      = 5;
const sal_uInt16 TYPE_BOOL      = 6;
const sal_uInt16 TYPE_CURRENCY  = 7;
const sal_uInt16 TYPE_MEMO      = 8;
const sal_uInt16 TYPE_COUNTER   = 9;
const sal_uInt16 TYPE_IMAGE     = 10;
const sal_uInt16 TYPE_CHAR      = 11;
const sal_uInt16 TYPE_DECIMAL   = 12;
const sal_uInt16 TYPE_BINARY    = 13;
const sal_uInt16 TYPE_VARBINARY = 14;
const sal_uInt16 TYPE_BIGINT    = 15;
const sal_uInt16 TYPE_DOUBLE    = 16;
const sal_uInt16 TYPE_FLOAT     = 17;
const sal_uInt16 TYPE_REAL      = 18;
const sal_uInt16 TYPE_INTEGER   = 19;
const sal_uInt16 TYPE_SMALLINT  = 20;
const sal_uInt16 TYPE_TINYINT   = 21;
const sal_uInt16 TYPE_SQLNULL   = 22;
const sal_uInt16 TYPE_OBJECT    = 23;
const sal_uInt16 TYPE_DISTINCT  = 24;
const sal_uInt16 TYPE_STRUCT    = 25;
const sal_uInt16 TYPE_ARRAY     = 26;
const sal_uInt16 TYPE_BLOB      = 27;
const sal_uInt16 TYPE_CLOB      = 28;
const sal_uInt16 TYPE_REF       = 29;
const sal_uInt16 TYPE_OTHER     = 30;
const sal_uInt16 TYPE_BIT       = 31;

    class OTypeInfo
    {
    public:
        OUString aUIName;        // the name which is the user see (a combination of resource text and aTypeName)
        OUString aTypeName;      // name of type in database
        OUString aLiteralPrefix; // prefix for quoting
        OUString aLiteralSuffix; // suffix for quoting
        OUString aCreateParams;  // parameter for creation
        OUString aLocalTypeName;

        sal_Int32       nPrecision;     // length of type
        sal_Int32       nType;          // database type

        sal_Int16       nMaximumScale;  // decimal places after decimal point
        sal_Int16       nMinimumScale;  // min decimal places after decimal point

        sal_Int16       nSearchType;    // if it is possible to search for type

        sal_Bool        bCurrency       : 1,    // currency
                        bAutoIncrement  : 1,    // if automatic incrementing field
                        bNullable       : 1,    // if field can be NULL
                        bCaseSensitive  : 1,    // if type is case sensitive
                        bUnsigned       : 1;    // if type is unsigned

        OTypeInfo()
                :nPrecision(0)
                ,nType(::com::sun::star::sdbc::DataType::OTHER)
                ,nMaximumScale(0)
                ,nMinimumScale(0)
                ,nSearchType(::com::sun::star::sdbc::ColumnSearch::FULL)
                ,bCurrency(sal_False)
                ,bAutoIncrement(sal_False)
                ,bNullable(sal_True)
                ,bCaseSensitive(sal_False)
                ,bUnsigned(sal_False)
        {}
        sal_Bool operator == (const OTypeInfo& lh) const { return lh.nType == nType; }
        sal_Bool operator != (const OTypeInfo& lh) const { return lh.nType != nType; }
        inline OUString  getDBName() const { return aTypeName; }

    };

    typedef ::boost::shared_ptr<OTypeInfo>          TOTypeInfoSP;
    typedef ::std::multimap<sal_Int32,TOTypeInfoSP> OTypeInfoMap;
    /** return the most suitable typeinfo for a requested type
        @param  _rTypeInfo      contains a map of type to typeinfo
        @param  _nType          the requested type
        @param  _sTypeName      the typename
        @param  _sCreateParams  the create params
        @param  _nPrecision     the precision
        @param  _nScale         the scale
        @param  _bAutoIncrement if it is a auto increment
        @param  _brForceToType  true when type was found which has some differenes
    */
    TOTypeInfoSP getTypeInfoFromType(const OTypeInfoMap& _rTypeInfo,
                               sal_Int32 _nType,
                               const OUString& _sTypeName,
                               const OUString& _sCreateParams,
                               sal_Int32 _nPrecision,
                               sal_Int32 _nScale,
                               sal_Bool _bAutoIncrement,
                               sal_Bool& _brForceToType);
}

#endif // DBAUI_TYPEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
