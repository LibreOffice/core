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
#ifndef _FMTWRAPINFLUENCEONOBJPOS_HXX
#define _FMTWRAPINFLUENCEONOBJPOS_HXX

#include <hintids.hxx>
#include <format.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/text/WrapInfluenceOnPosition.hpp>

class SW_DLLPUBLIC SwFmtWrapInfluenceOnObjPos: public SfxPoolItem
{
private:
    sal_Int16 mnWrapInfluenceOnPosition;

public:
    TYPEINFO();

    // #i35017# - constant name has changed
    SwFmtWrapInfluenceOnObjPos(
            sal_Int16 _nWrapInfluenceOnPosition =
            com::sun::star::text::WrapInfluenceOnPosition::ONCE_CONCURRENT );
    SwFmtWrapInfluenceOnObjPos(
            const SwFmtWrapInfluenceOnObjPos& _rCpy );
    ~SwFmtWrapInfluenceOnObjPos();

    SwFmtWrapInfluenceOnObjPos& operator=(
            const SwFmtWrapInfluenceOnObjPos& _rSource );

    /// pure virtual methods of class <SfxPoolItem>
    virtual int operator==( const SfxPoolItem& _rAttr ) const;
    virtual SfxPoolItem* Clone( SfxItemPool* pPool = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    /// direct accessors to data
    void SetWrapInfluenceOnObjPos( sal_Int16 _nWrapInfluenceOnPosition );
    // #i35017# - add parameter <_bIterativeAsOnceConcurrent>
    /// to control, if value <ITERATIVE> has to be treated as <ONCE_CONCURRENT>
    sal_Int16 GetWrapInfluenceOnObjPos(
                        const bool _bIterativeAsOnceConcurrent = false ) const;
};

inline const SwFmtWrapInfluenceOnObjPos& SwAttrSet::GetWrapInfluenceOnObjPos(sal_Bool bInP) const
    { return (const SwFmtWrapInfluenceOnObjPos&)Get( RES_WRAP_INFLUENCE_ON_OBJPOS,bInP); }

 inline const SwFmtWrapInfluenceOnObjPos& SwFmt::GetWrapInfluenceOnObjPos(sal_Bool bInP) const
    { return aSet.GetWrapInfluenceOnObjPos(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
