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

#ifndef _CONNECTIVITY_OTYPEINFO_HXX_
#define _CONNECTIVITY_OTYPEINFO_HXX_

#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    struct OTypeInfo
    {
        OUString aTypeName;      // Name of the type in the database
        OUString aLiteralPrefix; // Prefix for quoting
        OUString aLiteralSuffix; // Suffix for quoting
        OUString aCreateParams;  // Parameter for creating
        OUString aLocalTypeName;

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

        inline OUString getDBName() const { return aTypeName; }
    };
}
#endif // _CONNECTIVITY_OTYPEINFO_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
