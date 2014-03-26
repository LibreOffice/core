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
#ifndef INCLUDED_SW_INC_FMTWRAPINFLUENCEONOBJPOS_HXX
#define INCLUDED_SW_INC_FMTWRAPINFLUENCEONOBJPOS_HXX

#include <hintids.hxx>
#include <format.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/text/WrapInfluenceOnPosition.hpp>

class SW_DLLPUBLIC SwFmtWrapInfluenceOnObjPos: public SfxPoolItem
{
private:
    sal_Int16 mnWrapInfluenceOnPosition;

public:
    TYPEINFO_OVERRIDE();

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
    virtual bool operator==( const SfxPoolItem& _rAttr ) const;
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
