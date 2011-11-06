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


#include <hintids.hxx>
#include <txtrfmrk.hxx>
#include <fmtrfmrk.hxx>
#include <swfont.hxx>


/****************************************************************************
 *
 *  class SwFmtRefMark
 *
 ****************************************************************************/

SwFmtRefMark::~SwFmtRefMark( )
{
}

SwFmtRefMark::SwFmtRefMark( const XubString& rName )
    : SfxPoolItem( RES_TXTATR_REFMARK ),
    pTxtAttr( 0 ),
    aRefName( rName )
{
}

SwFmtRefMark::SwFmtRefMark( const SwFmtRefMark& rAttr )
    : SfxPoolItem( RES_TXTATR_REFMARK ),
    pTxtAttr( 0 ),
    aRefName( rAttr.aRefName )
{
}

int SwFmtRefMark::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return aRefName == ((SwFmtRefMark&)rAttr).aRefName;
}

SfxPoolItem* SwFmtRefMark::Clone( SfxItemPool* ) const
{
    return new SwFmtRefMark( *this );
}

/*************************************************************************
 *                      class SwTxtRefMark
 *************************************************************************/

// Attribut fuer Inhalts-/Positions-Referenzen im Text

SwTxtRefMark::SwTxtRefMark( SwFmtRefMark& rAttr,
            xub_StrLen const nStartPos, xub_StrLen const*const pEnd)
    : SwTxtAttrEnd( rAttr, nStartPos, nStartPos )
    , m_pTxtNode( 0 )
    , m_pEnd( 0 )
{
    rAttr.pTxtAttr = this;
    if ( pEnd )
    {
        m_nEnd = *pEnd;
        m_pEnd = & m_nEnd;
    }
    else
    {
        SetHasDummyChar(true);
    }
    SetDontMoveAttr( true );
    SetOverlapAllowedAttr( true );
}

xub_StrLen* SwTxtRefMark::GetEnd()
{
    return m_pEnd;
}

