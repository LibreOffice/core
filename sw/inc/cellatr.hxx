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
#include "cellfml.hxx"

namespace rtl { class OUString; }

class SW_DLLPUBLIC SwTblBoxNumFormat : public SfxUInt32Item
{
    bool bAuto;     ///< automatically given flag
public:
    SwTblBoxNumFormat( sal_uInt32 nFormat = css::util::NumberFormat::TEXT,
                        bool bAuto = false );

    // "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;

    inline SwTblBoxNumFormat& operator=( const SwTblBoxNumFormat& rAttr )
    {
        SetValue( rAttr.GetValue() );
        SetAutoFlag( rAttr.GetAutoFlag() );
        return *this;
    }

    bool GetAutoFlag() const                    { return bAuto; }
    void SetAutoFlag( bool bFlag = true )       { bAuto = bFlag; }
};

class SwTblBoxFormula : public SfxPoolItem, public SwTableFormula
{
    SwModify* pDefinedIn;   // Modify object where the formula is located
                            // can only be TableBoxFormat

public:
    SwTblBoxFormula( const OUString& rFormula );
    virtual ~SwTblBoxFormula() {};

    // "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;

    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    inline void ChgDefinedIn( const SwModify* pNew )
                                            { pDefinedIn = const_cast<SwModify*>(pNew); }
    //  BoxAttribut -> BoxStartNode
    virtual const SwNode* GetNodeOfFormula() const SAL_OVERRIDE;

          SwTableBox* GetTableBox();
    const SwTableBox* GetTableBox() const
        { return const_cast<SwTblBoxFormula*>(this)->GetTableBox(); }

    void ChangeState( const SfxPoolItem* pItem );
    void Calc( SwTblCalcPara& rCalcPara, double& rValue );
};

class SW_DLLPUBLIC SwTblBoxValue : public SfxPoolItem
{
    double nValue;
public:
    SwTblBoxValue();
    SwTblBoxValue( const double aVal );

    // "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;

    inline SwTblBoxValue& operator=( const SwTblBoxValue& rCmp )
    {
        nValue = rCmp.nValue;
        return *this;
    }

    double GetValue() const                     { return nValue; }
};

inline const SwTblBoxNumFormat      &SwAttrSet::GetTblBoxNumFmt(bool bInP) const
    {   return static_cast<const SwTblBoxNumFormat&>(Get( RES_BOXATR_FORMAT,bInP)); }
inline const SwTblBoxFormula        &SwAttrSet::GetTblBoxFormula(bool bInP) const
    {   return static_cast<const SwTblBoxFormula&>(Get( RES_BOXATR_FORMULA,bInP)); }
inline const SwTblBoxValue          &SwAttrSet::GetTblBoxValue(bool bInP) const
    {   return static_cast<const SwTblBoxValue&>(Get( RES_BOXATR_VALUE, bInP)); }

inline const SwTblBoxNumFormat      &SwFmt::GetTblBoxNumFmt(bool bInP) const
    {   return m_aSet.GetTblBoxNumFmt(bInP); }
inline const SwTblBoxFormula        &SwFmt::GetTblBoxFormula(bool bInP) const
    {   return m_aSet.GetTblBoxFormula(bInP); }
inline const SwTblBoxValue          &SwFmt::GetTblBoxValue(bool bInP) const
    {   return m_aSet.GetTblBoxValue(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
