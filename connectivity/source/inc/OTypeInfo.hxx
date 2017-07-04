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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_OTYPEINFO_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_OTYPEINFO_HXX

#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{
    struct OTypeInfo
    {
        OUString aTypeName;      // Name of the type in the database
        OUString aLocalTypeName;

        sal_Int32       nPrecision;     // Length of the type

        sal_Int16       nMaximumScale;  // Decimal places

        sal_Int16       nType;          // Database type

        OTypeInfo()
                :nPrecision(0)
                ,nMaximumScale(0)
                ,nType( css::sdbc::DataType::OTHER)
        {}

        static void * SAL_CALL operator new( size_t nSize )
            { return ::rtl_allocateMemory( nSize ); }
        static void * SAL_CALL operator new( size_t /*nSize*/,void* _pHint )
            { return _pHint; }
        static void SAL_CALL operator delete( void * pMem )
            { ::rtl_freeMemory( pMem ); }
        static void SAL_CALL operator delete( void * /*pMem*/,void* /*_pHint*/ )
            {  }

        bool operator == (const OTypeInfo& lh) const { return lh.nType == nType; }
        bool operator != (const OTypeInfo& lh) const { return lh.nType != nType; }
    };
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_OTYPEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
