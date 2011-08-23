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

// MARKER(update_precomp.py): autogen include statement, do not remove


#define UNICODE

#include "ddeimp.hxx"
#include <bf_svtools/svdde.hxx>

#if defined( WIN ) && defined( MSC )
#pragma code_seg( "SVDDE_MISC_CODE" )
#endif

namespace binfilter
{

// --- DdeString::DdeString() --------------------------------------

DdeString::DdeString( DWORD hDdeInst, const sal_Unicode* p ) :
                String( p )
{
    hString = DdeCreateStringHandle( hDdeInst, (LPTSTR)p, CP_WINUNICODE );
    hInst = hDdeInst;
}

// --- DdeString::DdeString() --------------------------------------

DdeString::DdeString( DWORD hDdeInst, const String& r) :
                String( r )
{
    hString = DdeCreateStringHandle( hDdeInst, (LPTSTR)r.GetBuffer(), CP_WINUNICODE );
    hInst = hDdeInst;
}

// --- DdeString::~DdeString() -------------------------------------

DdeString::~DdeString()
{
    if ( hString )
        DdeFreeStringHandle( hInst, hString );
}

// --- DdeString::operator==() -------------------------------------

int DdeString::operator==( HSZ h )
{
    return( !DdeCmpStringHandles( hString, h ) );
}

// --- DdeString::operator HSZ() -----------------------------------

DdeString::operator HSZ()
{
    return hString;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
