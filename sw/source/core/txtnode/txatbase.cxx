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

#include <svl/itempool.hxx>
#include <txatbase.hxx>
#include <fmtfld.hxx>
#include <docufld.hxx>

SwTxtAttr::SwTxtAttr( SfxPoolItem& rAttr, sal_Int32 nStart )
    : m_pAttr( &rAttr )
    , m_nStart( nStart )
    , m_bDontExpand( false )
    , m_bLockExpandFlag( false )
    , m_bDontMoveAttr( false )
    , m_bCharFmtAttr( false )
    , m_bOverlapAllowedAttr( false )
    , m_bPriorityAttr( false )
    , m_bDontExpandStart( false )
    , m_bNesting( false )
    , m_bHasDummyChar( false )
    , m_bFormatIgnoreStart(false)
    , m_bFormatIgnoreEnd(false)
{
}

SwTxtAttr::~SwTxtAttr( )
{
}

sal_Int32* SwTxtAttr::GetEnd()
{
    return 0;
}

void SwTxtAttr::Destroy( SwTxtAttr * pToDestroy, SfxItemPool& rPool )
{
    if (!pToDestroy) return;
    SfxPoolItem * const pAttr = pToDestroy->m_pAttr;
    delete pToDestroy;
    rPool.Remove( *pAttr );
}

int SwTxtAttr::operator==( const SwTxtAttr& rAttr ) const
{
    return GetAttr() == rAttr.GetAttr();
}

SwTxtAttrEnd::SwTxtAttrEnd( SfxPoolItem& rAttr,
        sal_Int32 nStart, sal_Int32 nEnd ) :
    SwTxtAttr( rAttr, nStart ), m_nEnd( nEnd )
{
}

sal_Int32* SwTxtAttrEnd::GetEnd()
{
    return & m_nEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
