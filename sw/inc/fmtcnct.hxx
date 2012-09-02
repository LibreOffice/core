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
#ifndef _FMTCNCT_HXX
#define _FMTCNCT_HXX

#include <hintids.hxx>
#include <svl/poolitem.hxx>
#include <format.hxx>
#include <calbck.hxx>


class SwFlyFrmFmt;
class IntlWrapper;

/// Connection (text flow) between two FlyFrms.
class SW_DLLPUBLIC SwFmtChain: public SfxPoolItem
{
    SwClient aPrev, ///< Previous SwFlyFrmFmt (if existent).
             aNext; ///< Next SwFlyFrmFmt (if existent).


public:
    SwFmtChain() : SfxPoolItem( RES_CHAIN ) {}
    SwFmtChain( const SwFmtChain &rCpy );

    inline SwFmtChain &operator=( const SwFmtChain& );

    /// "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;

    SwFlyFrmFmt* GetPrev() const { return (SwFlyFrmFmt*)aPrev.GetRegisteredIn(); }
    SwFlyFrmFmt* GetNext() const { return (SwFlyFrmFmt*)aNext.GetRegisteredIn(); }


    void SetPrev( SwFlyFrmFmt *pFmt );
    void SetNext( SwFlyFrmFmt *pFmt );
};

SwFmtChain &SwFmtChain::operator=( const SwFmtChain &rCpy )
{
    SetPrev( rCpy.GetPrev() );
    SetNext( rCpy.GetNext() );
    return *this;
}


inline const SwFmtChain &SwAttrSet::GetChain(sal_Bool bInP) const
    { return (const SwFmtChain&)Get( RES_CHAIN,bInP); }

inline const SwFmtChain &SwFmt::GetChain(sal_Bool bInP) const
    { return aSet.GetChain(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
