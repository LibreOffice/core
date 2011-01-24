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
#ifndef _ERRHDL_HXX
#define _ERRHDL_HXX

#if OSL_DEBUG_LEVEL > 1

#include <tools/solar.h>
#include <sal/types.h>
#include "swdllapi.h"

extern BOOL bAssert;                // TRUE, if an ASSERT Box popped up.


// -----------------------------------------------------------------------
// Return an error message including file name and line number indicating
// where the error occured.
// The function may not be used directly!
// -----------------------------------------------------------------------
SW_DLLPUBLIC void AssertFail( USHORT, const sal_Char* );

// -----------------------------------------------------------------------
// Check if the given condition is true. If not, an error message,
// identified via the ID, will be returned.
// -----------------------------------------------------------------------
#define ASSERT_ID( cond, id ) \
    if( !(cond) ) { \
        const char   *_pFileName  = __FILE__; \
       ::AssertFail( (USHORT)id, _pFileName ); \
    }


// -----------------------------------------------------------------------
// When building for production all debugging utilities are ignored automatically.
// -----------------------------------------------------------------------
#else
#define ASSERT_ID( cond, id )       ;
#endif // PRODUCT



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
