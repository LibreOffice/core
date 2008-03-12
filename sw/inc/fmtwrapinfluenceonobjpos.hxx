/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtwrapinfluenceonobjpos.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:11:56 $
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
#ifndef _FMTWRAPINFLUENCEONOBJPOS_HXX
#define _FMTWRAPINFLUENCEONOBJPOS_HXX

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _FORMAT_HXX
#include <format.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPINFLUENCEONPOSITION_HPP_
#include <com/sun/star/text/WrapInfluenceOnPosition.hpp>
#endif

class SwFmtWrapInfluenceOnObjPos: public SfxPoolItem
{
private:
    sal_Int16 mnWrapInfluenceOnPosition;

public:
    TYPEINFO();

    // --> OD 2004-10-18 #i35017# - constant name has changed
    SwFmtWrapInfluenceOnObjPos(
            sal_Int16 _nWrapInfluenceOnPosition =
            com::sun::star::text::WrapInfluenceOnPosition::ONCE_CONCURRENT );
    // <--
    SwFmtWrapInfluenceOnObjPos(
            const SwFmtWrapInfluenceOnObjPos& _rCpy );
    ~SwFmtWrapInfluenceOnObjPos();

    SwFmtWrapInfluenceOnObjPos& operator=(
            const SwFmtWrapInfluenceOnObjPos& _rSource );

    // pure virtual methods of class <SfxPoolItem>
    virtual int operator==( const SfxPoolItem& _rAttr ) const;
    virtual SfxPoolItem* Clone( SfxItemPool* pPool = 0 ) const;

    virtual BOOL QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    // direct accessors to data
    void SetWrapInfluenceOnObjPos( sal_Int16 _nWrapInfluenceOnPosition );
    // --> OD 2004-10-18 #i35017# - add parameter <_bIterativeAsOnceConcurrent>
    // to control, if value <ITERATIVE> has to be treated as <ONCE_CONCURRENT>
    sal_Int16 GetWrapInfluenceOnObjPos(
                        const bool _bIterativeAsOnceConcurrent = false ) const;
    // <--
};

inline const SwFmtWrapInfluenceOnObjPos& SwAttrSet::GetWrapInfluenceOnObjPos(BOOL bInP) const
    { return (const SwFmtWrapInfluenceOnObjPos&)Get( RES_WRAP_INFLUENCE_ON_OBJPOS,bInP); }

 inline const SwFmtWrapInfluenceOnObjPos& SwFmt::GetWrapInfluenceOnObjPos(BOOL bInP) const
    { return aSet.GetWrapInfluenceOnObjPos(bInP); }

#endif
