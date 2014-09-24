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
#ifndef INCLUDED_SW_INC_FMTORNT_HXX
#define INCLUDED_SW_INC_FMTORNT_HXX

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
    SwTwips         m_nYPos;  ///< Contains *always* the current RelPos.
    sal_Int16       m_eOrient;
    sal_Int16       m_eRelation;
public:
    TYPEINFO_OVERRIDE();
    SwFmtVertOrient( SwTwips nY = 0, sal_Int16 eVert = com::sun::star::text::VertOrientation::NONE,
                     sal_Int16 eRel = com::sun::star::text::RelOrientation::PRINT_AREA );
    inline SwFmtVertOrient &operator=( const SwFmtVertOrient &rCpy );

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    SvStream& Store(SvStream &rStream) const SAL_OVERRIDE;
    SfxPoolItem* Create(SvStream &rStream) const SAL_OVERRIDE;

    sal_Int16 GetVertOrient() const { return m_eOrient; }
    sal_Int16 GetRelationOrient() const { return m_eRelation; }
    void   SetVertOrient( sal_Int16 eNew ) { m_eOrient = eNew; }
    void   SetRelationOrient( sal_Int16 eNew ) { m_eRelation = eNew; }

    SwTwips GetPos() const { return m_nYPos; }
    void    SetPos( SwTwips nNew ) { m_nYPos = nNew; }
};

class SW_DLLPUBLIC SwFmtHoriOrient: public SfxPoolItem
{
    SwTwips         nXPos;              ///< Contains *always* the current RelPos.
    sal_Int16       eOrient;
    sal_Int16       eRelation;
    bool            bPosToggle : 1; ///< Flip position on even pages.
public:
    TYPEINFO_OVERRIDE();
    SwFmtHoriOrient( SwTwips nX = 0, sal_Int16 eHori = com::sun::star::text::HoriOrientation::NONE,
        sal_Int16 eRel = com::sun::star::text::RelOrientation::PRINT_AREA, bool bPos = false );
    inline SwFmtHoriOrient &operator=( const SwFmtHoriOrient &rCpy );

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    sal_Int16 GetHoriOrient() const { return eOrient; }
    sal_Int16 GetRelationOrient() const { return eRelation; }
    void SetHoriOrient( sal_Int16 eNew ) { eOrient = eNew; }
    void SetRelationOrient( sal_Int16 eNew ) { eRelation = eNew; }

    SwTwips GetPos() const { return nXPos; }
    void    SetPos( SwTwips nNew ) { nXPos = nNew; }

    bool IsPosToggle() const { return bPosToggle; }
    void SetPosToggle( bool bNew ) { bPosToggle = bNew; }
};

inline SwFmtVertOrient &SwFmtVertOrient::operator=( const SwFmtVertOrient &rCpy )
{
    m_nYPos = rCpy.GetPos();
    m_eOrient = rCpy.GetVertOrient();
    m_eRelation = rCpy.GetRelationOrient();
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

inline const SwFmtVertOrient &SwAttrSet::GetVertOrient(bool bInP) const
    { return (const SwFmtVertOrient&)Get( RES_VERT_ORIENT,bInP); }
inline const SwFmtHoriOrient &SwAttrSet::GetHoriOrient(bool bInP) const
    { return (const SwFmtHoriOrient&)Get( RES_HORI_ORIENT,bInP); }

inline const SwFmtVertOrient &SwFmt::GetVertOrient(bool bInP) const
    { return aSet.GetVertOrient(bInP); }
inline const SwFmtHoriOrient &SwFmt::GetHoriOrient(bool bInP) const
    { return aSet.GetHoriOrient(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
