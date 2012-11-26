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
#include <fmtwrapinfluenceonobjpos.hxx>

#ifndef _UNOMID_H
#include <unomid.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SwFmtWrapInfluenceOnObjPos::SwFmtWrapInfluenceOnObjPos(
                                            sal_Int16 _nWrapInfluenceOnPosition )
    : SfxPoolItem( RES_WRAP_INFLUENCE_ON_OBJPOS ),
    mnWrapInfluenceOnPosition( _nWrapInfluenceOnPosition )
{
}

SwFmtWrapInfluenceOnObjPos::SwFmtWrapInfluenceOnObjPos(
                                        const SwFmtWrapInfluenceOnObjPos& _rCpy )
    : SfxPoolItem( RES_WRAP_INFLUENCE_ON_OBJPOS ),
    mnWrapInfluenceOnPosition( _rCpy.GetWrapInfluenceOnObjPos() )
{
}

SwFmtWrapInfluenceOnObjPos::~SwFmtWrapInfluenceOnObjPos()
{
}

SwFmtWrapInfluenceOnObjPos& SwFmtWrapInfluenceOnObjPos::operator=(
                                    const SwFmtWrapInfluenceOnObjPos& _rSource )
{
    mnWrapInfluenceOnPosition = _rSource.GetWrapInfluenceOnObjPos();

    return *this;
}

int SwFmtWrapInfluenceOnObjPos::operator==( const SfxPoolItem& _rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( _rAttr ), "keine gleichen Attribute" );
    return ( mnWrapInfluenceOnPosition ==
                    static_cast<const SwFmtWrapInfluenceOnObjPos&>(_rAttr).
                                                GetWrapInfluenceOnObjPos() );
}

SfxPoolItem* SwFmtWrapInfluenceOnObjPos::Clone( SfxItemPool * ) const
{
    return new SwFmtWrapInfluenceOnObjPos(*this);
}

sal_Bool SwFmtWrapInfluenceOnObjPos::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch ( nMemberId )
    {
        case MID_WRAP_INFLUENCE:
        {
            rVal <<= GetWrapInfluenceOnObjPos();
        }
        break;
        default:
            ASSERT( false, "<SwFmtWrapInfluenceOnObjPos::QueryValue()> - unknown MemberId" );
            bRet = sal_False;
    }

    return bRet;
}

sal_Bool SwFmtWrapInfluenceOnObjPos::PutValue( const Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;

    switch ( nMemberId )
    {
        case MID_WRAP_INFLUENCE:
        {
            sal_Int16 nNewWrapInfluence = 0;
            rVal >>= nNewWrapInfluence;
            // --> OD 2004-10-18 #i35017# - constant names have changed and
            // <ITERATIVE> has been added
            if ( nNewWrapInfluence == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
                 nNewWrapInfluence == text::WrapInfluenceOnPosition::ONCE_CONCURRENT ||
                 nNewWrapInfluence == text::WrapInfluenceOnPosition::ITERATIVE )
            // <--
            {
                SetWrapInfluenceOnObjPos( nNewWrapInfluence );
            }
            else
            {
                ASSERT( false, "<SwFmtWrapInfluenceOnObjPos::PutValue(..)> - invalid attribute value" );
                bRet = sal_False;
            }
        }
        break;
        default:
            ASSERT( false, "<SwFmtWrapInfluenceOnObjPos::QueryValue()> - unknown MemberId" );
            bRet = sal_False;
    }

    return bRet;
}

void SwFmtWrapInfluenceOnObjPos::SetWrapInfluenceOnObjPos( sal_Int16 _nWrapInfluenceOnPosition )
{
    // --> OD 2004-10-18 #i35017# - constant names have changed and consider
    // new value <ITERATIVE>
    if ( _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
         _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_CONCURRENT ||
         _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ITERATIVE )
    // <--
    {
        mnWrapInfluenceOnPosition = _nWrapInfluenceOnPosition;
    }
    else
    {
        ASSERT( false, "<SwFmtWrapInfluenceOnObjPos::SetWrapInfluenceOnObjPos(..)> - invalid attribute value" );
    }
}

// --> OD 2004-10-18 #i35017# - add parameter <_bIterativeAsOnceConcurrent>
// to control, if value <ITERATIVE> has to be treated as <ONCE_CONCURRENT>
sal_Int16 SwFmtWrapInfluenceOnObjPos::GetWrapInfluenceOnObjPos(
                                const bool _bIterativeAsOnceConcurrent ) const
{
    sal_Int16 nWrapInfluenceOnPosition( mnWrapInfluenceOnPosition );

    if ( _bIterativeAsOnceConcurrent &&
         nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ITERATIVE )
    {
        nWrapInfluenceOnPosition = text::WrapInfluenceOnPosition::ONCE_CONCURRENT;
    }

    return nWrapInfluenceOnPosition;
}
// <--
