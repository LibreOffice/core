/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <svl/itempool.hxx>
#include <txatbase.hxx>
#include <fmtfld.hxx>
#include <docufld.hxx>

SwTxtAttr::SwTxtAttr( SfxPoolItem& rAttr, xub_StrLen nStart )
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
    , m_bHasContent( false )
{
}

SwTxtAttr::~SwTxtAttr( )
{
}

xub_StrLen* SwTxtAttr::GetEnd()
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
        xub_StrLen nStart, xub_StrLen nEnd ) :
    SwTxtAttr( rAttr, nStart ), m_nEnd( nEnd )
{
}

xub_StrLen* SwTxtAttrEnd::GetEnd()
{
    return & m_nEnd;
}

