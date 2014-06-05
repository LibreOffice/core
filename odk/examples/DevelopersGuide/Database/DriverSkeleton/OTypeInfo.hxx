/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#ifndef INCLUDED_EXAMPLES_DATABASE_DRIVERSKELETON_OTYPEINFO_HXX
#define INCLUDED_EXAMPLES_DATABASE_DRIVERSKELETON_OTYPEINFO_HXX

#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

namespace connectivity
{
    struct OTypeInfo
    {
        ::rtl::OUString aTypeName;      // Name of the type in the database
        ::rtl::OUString aLiteralPrefix; // Prefix for literals
        ::rtl::OUString aLiteralSuffix; // Suffix for literals
        ::rtl::OUString aCreateParams;  // Parameters to create
        ::rtl::OUString aLocalTypeName;

        sal_Int32       nPrecision;     // Length of the types

        sal_Int16       nMaximumScale;  // Decimal places (precision)
        sal_Int16       nMinimumScale;  // Min decimal places (precision)

        sal_Int16       nType;          // Database type
        sal_Int16       nSearchType;    // Can search for the type
        sal_Int16       nNumPrecRadix;  // indicating the radix, which is usually 2 or 10

        sal_Bool        bCurrency       : 1,    // Currency
                        bAutoIncrement  : 1,    // Is this field auto incrementing?
                        bNullable       : 1,    // Can this field assume a NULL value?
                        bCaseSensitive  : 1,    // Is this type case-sensitive?
                        bUnsigned       : 1,    // Is this type unsigned?
                        bEmpty_1        : 1,    // for later use
                        bEmpty_2        : 1;

        OTypeInfo()
                :bCurrency(sal_False)
                ,bAutoIncrement(sal_False)
                ,bNullable(sal_True)
                ,bCaseSensitive(sal_False)
                ,bUnsigned(sal_False)
                ,nMaximumScale(0)
                ,nMinimumScale(0)
                ,nType( ::com::sun::star::sdbc::DataType::OTHER)
                ,nPrecision(0)
                ,nSearchType( ::com::sun::star::sdbc::ColumnSearch::FULL)
        {}

        inline static void * SAL_CALL operator new( size_t nSize )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t nSize,void* _pHint )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void * pMem,void* _pHint )
            {  }

        sal_Bool operator == (const OTypeInfo& lh) const { return lh.nType == nType; }
        sal_Bool operator != (const OTypeInfo& lh) const { return lh.nType != nType; }

        inline ::rtl::OUString getDBName() const { return aTypeName; }
    };
}

#endif // INCLUDED_EXAMPLES_DATABASE_DRIVERSKELETON_OTYPEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
