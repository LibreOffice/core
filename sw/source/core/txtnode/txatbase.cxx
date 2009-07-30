/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: txatbase.cxx,v $
 * $Revision: 1.12 $
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
#include "precompiled_sw.hxx"


#include <svtools/itempool.hxx>
#include <txatbase.hxx>
#include <fmtfld.hxx>
#include <docufld.hxx>

SwTxtAttr::SwTxtAttr( const SfxPoolItem& rAttr, xub_StrLen nStart )
    : m_pAttr( &rAttr )
    , m_nStart( nStart )
    , m_bDontExpand( false )
    , m_bLockExpandFlag( false )
    , m_bDontMergeAttr( false )
    , m_bDontMoveAttr( false )
    , m_bCharFmtAttr( false )
    , m_bOverlapAllowedAttr( false )
    , m_bPriorityAttr( false )
    , m_bDontExpandStart( false )
{
}

SwTxtAttr::~SwTxtAttr( )
{
}

xub_StrLen* SwTxtAttr::GetEnd()
{
    return 0;
}

// RemoveFromPool must be called before destructor!
void SwTxtAttr::RemoveFromPool( SfxItemPool& rPool )
{
    rPool.Remove( GetAttr() );
    m_pAttr = 0;
}

int SwTxtAttr::operator==( const SwTxtAttr& rAttr ) const
{
    return GetAttr() == rAttr.GetAttr();
}

SwTxtAttrEnd::SwTxtAttrEnd( const SfxPoolItem& rAttr,
        xub_StrLen nStart, xub_StrLen nEnd ) :
    SwTxtAttr( rAttr, nStart ), m_nEnd( nEnd )
{
}

xub_StrLen* SwTxtAttrEnd::GetEnd()
{
    return & m_nEnd;
}

