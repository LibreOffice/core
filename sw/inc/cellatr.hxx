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

#ifndef INCLUDED_SW_INC_CELLATR_HXX
#define INCLUDED_SW_INC_CELLATR_HXX

#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include "swdllapi.h"
#include "format.hxx"
#include "hintids.hxx"
#include "cellfml.hxx"

/** The number formatter's default locale's @ Text format.
    Not necessarily system locale, but the locale the formatter was constructed
    with. For this SvNumberFormatter::IsTextFormat() always returns true.
 */
constexpr sal_uInt32 getSwDefaultTextFormat() { return NF_STANDARD_FORMAT_TEXT; }

class SW_DLLPUBLIC SwTableBoxNumFormat : public SfxUInt32Item
{
public:
    SwTableBoxNumFormat( sal_uInt32 nFormat = getSwDefaultTextFormat() );

    // "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SwTableBoxNumFormat* Clone( SfxItemPool* pPool = nullptr ) const override;

    SwTableBoxNumFormat& operator=( const SwTableBoxNumFormat& rAttr )
    {
        SetValue( rAttr.GetValue() );
        return *this;
    }
};

class SAL_DLLPUBLIC_RTTI SwTableBoxFormula : public SfxPoolItem, public SwTableFormula
{
    SwModify* m_pDefinedIn;   // Modify object where the formula is located
                            // can only be TableBoxFormat

public:
    SwTableBoxFormula( const OUString& rFormula );

    // "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SwTableBoxFormula* Clone( SfxItemPool* pPool = nullptr ) const override;

    const SwModify* GetDefinedIn() const { return m_pDefinedIn; }
    void ChgDefinedIn( const SwModify* pNew )
                                            { m_pDefinedIn = const_cast<SwModify*>(pNew); }
    //  BoxAttribut -> BoxStartNode
    virtual const SwNode* GetNodeOfFormula() const override;

          SwTableBox* GetTableBox();
    const SwTableBox* GetTableBox() const
        { return const_cast<SwTableBoxFormula*>(this)->GetTableBox(); }

    void ChangeState( const SfxPoolItem* pItem );
    void Calc( SwTableCalcPara& rCalcPara, double& rValue );
};

class SW_DLLPUBLIC SwTableBoxValue : public SfxPoolItem
{
    double m_nValue;
public:
    SwTableBoxValue();
    SwTableBoxValue( const double aVal );

    // "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SwTableBoxValue* Clone( SfxItemPool* pPool = nullptr ) const override;

    SwTableBoxValue& operator=( const SwTableBoxValue& rCmp )
    {
        m_nValue = rCmp.m_nValue;
        return *this;
    }

    double GetValue() const                     { return m_nValue; }
};

inline const SwTableBoxNumFormat      &SwAttrSet::GetTableBoxNumFormat(bool bInP) const
    {   return Get( RES_BOXATR_FORMAT,bInP); }
inline const SwTableBoxFormula        &SwAttrSet::GetTableBoxFormula(bool bInP) const
    {   return Get( RES_BOXATR_FORMULA,bInP); }
inline const SwTableBoxValue          &SwAttrSet::GetTableBoxValue(bool bInP) const
    {   return Get( RES_BOXATR_VALUE, bInP); }

inline const SwTableBoxNumFormat      &SwFormat::GetTableBoxNumFormat(bool bInP) const
    {   return m_aSet.GetTableBoxNumFormat(bInP); }
inline const SwTableBoxFormula        &SwFormat::GetTableBoxFormula(bool bInP) const
    {   return m_aSet.GetTableBoxFormula(bInP); }
inline const SwTableBoxValue          &SwFormat::GetTableBoxValue(bool bInP) const
    {   return m_aSet.GetTableBoxValue(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
