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
#ifndef _FMTORNT_HXX
#define _FMTORNT_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include "swdllapi.h"
#include <hintids.hxx>
#include <swtypes.hxx>
#include <format.hxx>
#include <svl/poolitem.hxx>


class IntlWrapper;

class SW_DLLPUBLIC SwFmtVertOrient: public SfxPoolItem
{
    SwTwips         nYPos;  ///< Contains *always* the current RelPos.
    sal_Int16       eOrient;
    sal_Int16       eRelation;
public:
    TYPEINFO();
    SwFmtVertOrient( SwTwips nY = 0, sal_Int16 eVert = com::sun::star::text::VertOrientation::NONE,
                     sal_Int16 eRel = com::sun::star::text::RelOrientation::PRINT_AREA );
    inline SwFmtVertOrient &operator=( const SwFmtVertOrient &rCpy );

    /// "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    SvStream& Store(SvStream &rStream, sal_uInt16 itemVersion) const;
    SfxPoolItem* Create(SvStream &rStream, sal_uInt16 itemVersion) const;

    sal_Int16 GetVertOrient() const { return eOrient; }
    sal_Int16 GetRelationOrient() const { return eRelation; }
    void   SetVertOrient( sal_Int16 eNew ) { eOrient = eNew; }
    void   SetRelationOrient( sal_Int16 eNew ) { eRelation = eNew; }

    SwTwips GetPos() const { return nYPos; }
    void    SetPos( SwTwips nNew ) { nYPos = nNew; }
};


class SW_DLLPUBLIC SwFmtHoriOrient: public SfxPoolItem
{
    SwTwips         nXPos;              ///< Contains *always* the current RelPos.
    sal_Int16       eOrient;
    sal_Int16       eRelation;
    sal_Bool            bPosToggle : 1; ///< Flip position on even pages.
public:
    TYPEINFO();
    SwFmtHoriOrient( SwTwips nX = 0, sal_Int16 eHori = com::sun::star::text::HoriOrientation::NONE,
        sal_Int16 eRel = com::sun::star::text::RelOrientation::PRINT_AREA, sal_Bool bPos = sal_False );
    inline SwFmtHoriOrient &operator=( const SwFmtHoriOrient &rCpy );

    /// "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    sal_Int16 GetHoriOrient() const { return eOrient; }
    sal_Int16 GetRelationOrient() const { return eRelation; }
    void SetHoriOrient( sal_Int16 eNew ) { eOrient = eNew; }
    void SetRelationOrient( sal_Int16 eNew ) { eRelation = eNew; }

    SwTwips GetPos() const { return nXPos; }
    void    SetPos( SwTwips nNew ) { nXPos = nNew; }

    sal_Bool IsPosToggle() const { return bPosToggle; }
    void SetPosToggle( sal_Bool bNew ) { bPosToggle = bNew; }
};

inline SwFmtVertOrient &SwFmtVertOrient::operator=( const SwFmtVertOrient &rCpy )
{
    nYPos = rCpy.GetPos();
    eOrient = rCpy.GetVertOrient();
    eRelation = rCpy.GetRelationOrient();
    return *this;
}
inline SwFmtHoriOrient &SwFmtHoriOrient::operator=( const SwFmtHoriOrient &rCpy )
{
    nXPos = rCpy.GetPos();
    eOrient = rCpy.GetHoriOrient();
    eRelation = rCpy.GetRelationOrient();
    bPosToggle = rCpy.IsPosToggle();
    return *this;
}

inline const SwFmtVertOrient &SwAttrSet::GetVertOrient(sal_Bool bInP) const
    { return (const SwFmtVertOrient&)Get( RES_VERT_ORIENT,bInP); }
inline const SwFmtHoriOrient &SwAttrSet::GetHoriOrient(sal_Bool bInP) const
    { return (const SwFmtHoriOrient&)Get( RES_HORI_ORIENT,bInP); }

inline const SwFmtVertOrient &SwFmt::GetVertOrient(sal_Bool bInP) const
    { return aSet.GetVertOrient(bInP); }
inline const SwFmtHoriOrient &SwFmt::GetHoriOrient(sal_Bool bInP) const
    { return aSet.GetHoriOrient(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
