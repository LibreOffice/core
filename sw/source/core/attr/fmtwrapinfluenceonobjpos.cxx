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
#include "precompiled_sw.hxx"
#include <fmtwrapinfluenceonobjpos.hxx>

#include <unomid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

TYPEINIT1(SwFmtWrapInfluenceOnObjPos, SfxPoolItem);

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
    OSL_ENSURE( SfxPoolItem::operator==( _rAttr ), "keine gleichen Attribute" );
    return ( mnWrapInfluenceOnPosition ==
                    static_cast<const SwFmtWrapInfluenceOnObjPos&>(_rAttr).
                                                GetWrapInfluenceOnObjPos() );
}

SfxPoolItem* SwFmtWrapInfluenceOnObjPos::Clone( SfxItemPool * ) const
{
    return new SwFmtWrapInfluenceOnObjPos(*this);
}

bool SwFmtWrapInfluenceOnObjPos::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch ( nMemberId )
    {
        case MID_WRAP_INFLUENCE:
        {
            rVal <<= GetWrapInfluenceOnObjPos();
        }
        break;
        default:
            OSL_ENSURE( false, "<SwFmtWrapInfluenceOnObjPos::QueryValue()> - unknown MemberId" );
            bRet = false;
    }

    return bRet;
}

bool SwFmtWrapInfluenceOnObjPos::PutValue( const Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;

    switch ( nMemberId )
    {
        case MID_WRAP_INFLUENCE:
        {
            sal_Int16 nNewWrapInfluence = 0;
            rVal >>= nNewWrapInfluence;
            // #i35017# - constant names have changed and
            // <ITERATIVE> has been added
            if ( nNewWrapInfluence == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
                 nNewWrapInfluence == text::WrapInfluenceOnPosition::ONCE_CONCURRENT ||
                 nNewWrapInfluence == text::WrapInfluenceOnPosition::ITERATIVE )
            {
                SetWrapInfluenceOnObjPos( nNewWrapInfluence );
            }
            else
            {
                OSL_ENSURE( false, "<SwFmtWrapInfluenceOnObjPos::PutValue(..)> - invalid attribute value" );
                bRet = false;
            }
        }
        break;
        default:
            OSL_ENSURE( false, "<SwFmtWrapInfluenceOnObjPos::QueryValue()> - unknown MemberId" );
            bRet = false;
    }

    return bRet;
}

void SwFmtWrapInfluenceOnObjPos::SetWrapInfluenceOnObjPos( sal_Int16 _nWrapInfluenceOnPosition )
{
    // #i35017# - constant names have changed and consider
    // new value <ITERATIVE>
    if ( _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
         _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_CONCURRENT ||
         _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ITERATIVE )
    {
        mnWrapInfluenceOnPosition = _nWrapInfluenceOnPosition;
    }
    else
    {
        OSL_ENSURE( false, "<SwFmtWrapInfluenceOnObjPos::SetWrapInfluenceOnObjPos(..)> - invalid attribute value" );
    }
}

// #i35017# - add parameter <_bIterativeAsOnceConcurrent>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
