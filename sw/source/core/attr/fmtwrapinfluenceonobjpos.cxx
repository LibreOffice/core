/*************************************************************************
 *
 *  $RCSfile: fmtwrapinfluenceonobjpos.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:00:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FMTWRAPINFLUENCEONOBJPOS_HXX
#include <fmtwrapinfluenceonobjpos.hxx>
#endif

#ifndef _UNOMID_H
#include <unomid.h>
#endif

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

SfxPoolItem* SwFmtWrapInfluenceOnObjPos::Clone( SfxItemPool *pPool ) const
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
            sal_Int16 nNewWrapInfluence;
            rVal >>= nNewWrapInfluence;
            if ( nNewWrapInfluence == text::WrapInfluenceOnPosition::NONE_SUCCESSIVE_POSITIONED ||
                 nNewWrapInfluence == text::WrapInfluenceOnPosition::NONE_CONCURRENT_POSITIONED )
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
    if ( _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::NONE_SUCCESSIVE_POSITIONED ||
         _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::NONE_CONCURRENT_POSITIONED )
    {
        mnWrapInfluenceOnPosition = _nWrapInfluenceOnPosition;
    }
    else
    {
        ASSERT( false, "<SwFmtWrapInfluenceOnObjPos::SetWrapInfluenceOnObjPos(..)> - invalid attribute value" );
    }
}

sal_Int16 SwFmtWrapInfluenceOnObjPos::GetWrapInfluenceOnObjPos() const
{
    return mnWrapInfluenceOnPosition;
}
