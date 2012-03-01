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

#ifndef _CONNECTIVITY_OTYPEINFO_HXX_
#define _CONNECTIVITY_OTYPEINFO_HXX_

#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    struct OTypeInfo
    {
        ::rtl::OUString aTypeName;      // Name of the type in the database
        ::rtl::OUString aLiteralPrefix; // Prefix for quoting
        ::rtl::OUString aLiteralSuffix; // Suffix for quoting
        ::rtl::OUString aCreateParams;  // Parameter for creating
        ::rtl::OUString aLocalTypeName;

        sal_Int32       nPrecision;     // Length of the type

        sal_Int16       nMaximumScale;  // Decimal places
        sal_Int16       nMinimumScale;  // Minimum decimal places

        sal_Int16       nType;          // Database type
        sal_Int16       nSearchType;    // Can we search for the type?
        sal_Int16       nNumPrecRadix;  // indicating the radix, which is usually 2 or 10

        sal_Bool        bCurrency       : 1,    // Currency
                        bAutoIncrement  : 1,    // Is it an autoincrementing field?
                        bNullable       : 1,    // Can the field be NULL?
                        bCaseSensitive  : 1,    // Is the type case sensitive?
                        bUnsigned       : 1,    // Is the type unsigned?
                        bEmpty_1        : 1,    // for later use
                        bEmpty_2        : 1;

        OTypeInfo()
                :nPrecision(0)
                ,nMaximumScale(0)
                ,nMinimumScale(0)
                ,nType( ::com::sun::star::sdbc::DataType::OTHER)
                ,nSearchType( ::com::sun::star::sdbc::ColumnSearch::FULL)
                ,bCurrency(sal_False)
                ,bAutoIncrement(sal_False)
                ,bNullable(sal_True)
                ,bCaseSensitive(sal_False)
                ,bUnsigned(sal_False)
        {}

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t /*nSize*/,void* _pHint ) SAL_THROW(())
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void * /*pMem*/,void* /*_pHint*/ ) SAL_THROW(())
            {  }

        sal_Bool operator == (const OTypeInfo& lh) const { return lh.nType == nType; }
        sal_Bool operator != (const OTypeInfo& lh) const { return lh.nType != nType; }

        inline ::rtl::OUString getDBName() const { return aTypeName; }
    };
}
#endif // _CONNECTIVITY_OTYPEINFO_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
