/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtwrapinfluenceonobjpos.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-12 16:22:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _FMTWRAPINFLUENCEONOBJPOS_HXX
#include <fmtwrapinfluenceonobjpos.hxx>
#endif

#ifndef _UNOMID_H
#include <unomid.h>
#endif

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
    ASSERT( SfxPoolItem::operator==( _rAttr ), "keine gleichen Attribute" );
    return ( mnWrapInfluenceOnPosition ==
                    static_cast<const SwFmtWrapInfluenceOnObjPos&>(_rAttr).
                                                GetWrapInfluenceOnObjPos() );
}

SfxPoolItem* SwFmtWrapInfluenceOnObjPos::Clone( SfxItemPool * ) const
{
    return new SwFmtWrapInfluenceOnObjPos(*this);
}

BOOL SwFmtWrapInfluenceOnObjPos::QueryValue( Any& rVal, BYTE nMemberId ) const
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

BOOL SwFmtWrapInfluenceOnObjPos::PutValue( const Any& rVal, BYTE nMemberId )
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
