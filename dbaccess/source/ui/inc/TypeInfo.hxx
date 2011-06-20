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
//========================================================================
// Anhand dieser Ids werden die sprachabhaengigen ::rtl::OUString aus der Resource geholt
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
        ::rtl::OUString aUIName;        // the name which is the user see (a combination of resource text and aTypeName)
        ::rtl::OUString aTypeName;      // Name des Types in der Datenbank
        ::rtl::OUString aLiteralPrefix; // Prefix zum Quoten
        ::rtl::OUString aLiteralSuffix; // Suffix zum Quoten
        ::rtl::OUString aCreateParams;  // Parameter zum Erstellen
        ::rtl::OUString aLocalTypeName;

        sal_Int32       nPrecision;     // Laenge des Types
        sal_Int32       nType;          // Datenbanktyp

        sal_Int16       nMaximumScale;  // Nachkommastellen
        sal_Int16       nMinimumScale;  // Min Nachkommastellen

        sal_Int16       nSearchType;    // kann nach dem Typen gesucht werden


        sal_Bool        bCurrency       : 1,    // Waehrung
                        bAutoIncrement  : 1,    // Ist es ein automatisch incrementierendes Feld
                        bNullable       : 1,    // Kann das Feld NULL annehmen
                        bCaseSensitive  : 1,    // Ist der Type Casesensitive
                        bUnsigned       : 1;    // Ist der Type Unsigned

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
        inline ::rtl::OUString  getDBName() const { return aTypeName; }

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
                               const ::rtl::OUString& _sTypeName,
                               const ::rtl::OUString& _sCreateParams,
                               sal_Int32 _nPrecision,
                               sal_Int32 _nScale,
                               sal_Bool _bAutoIncrement,
                               sal_Bool& _brForceToType);
}

#endif // DBAUI_TYPEINFO_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
