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

#ifdef DBG_UTIL

#include <tools/solar.h>
#include <sal/types.h>

namespace binfilter {
// -----------------------------------------------------------------------
// Ausgabe einer Fehlermeldung inkl. Dateiname und Zeilennummer
// wo der Fehler auftrat.
// Die Funktion darf nicht direkt benutzt werden!
// -----------------------------------------------------------------------
extern void AssertFail( const sal_Char*, const sal_Char*, USHORT );
extern void AssertFail( USHORT, const sal_Char*, USHORT );

extern BOOL bAssert;				// TRUE, wenn eine ASSERT-Box hochkam
#ifdef WIN
//MSC (7.0) machts nur mit static, ZTC in inlines nur ohne...
#define ASSERT( cond, message ) \
    if( !(cond) ) { \
        static const char __FAR_DATA _pErrorText[] = #message; \
        static const char __FAR_DATA _pFileName[]  = __FILE__; \
        ::binfilter::AssertFail( _pErrorText, _pFileName, __LINE__ ); \
    }
#else
#define ASSERT( cond, message ) \
    if( !(cond) ) { \
        const char	 *_pErrorText = #message; \
        const char	 *_pFileName  = __FILE__; \
       ::binfilter::AssertFail( _pErrorText, _pFileName, __LINE__ ); \
    }
#endif

// -----------------------------------------------------------------------
// Prueft ob die angegebene Bedingung wahr ist, wenn nicht wird eine
// Fehlermeldung die ueber die ID Identifiziert wird, ausgegeben.
// -----------------------------------------------------------------------
#ifdef WIN
//MSC (7.0) machts nur mit static, ZTC in inlines nur ohne...
#define ASSERT_ID( cond, id )	\
    if( !(cond) ) { \
        static const char __FAR_DATA _pFileName[]  = __FILE__;	\
        ::binfilter::AssertFail( (USHORT)id, _pFileName, __LINE__ );	\
    }
#else
#define ASSERT_ID( cond, id ) \
    if( !(cond) ) { \
        const char	 *_pFileName  = __FILE__; \
       ::binfilter::AssertFail( (USHORT)id, _pFileName, __LINE__ ); \
    }
#endif
} //namespace binfilter

// -----------------------------------------------------------------------
// Beim Bilden der Produktversion werden alle Debug-Utilities automatisch
// ignoriert
// -----------------------------------------------------------------------
#else // PRODUCT

namespace binfilter {
#define ASSERT( cond, message ) 	;
#define ASSERT_ID( cond, id )		;
} //namespace binfilter

#endif // PRODUCT

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
