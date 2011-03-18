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

#ifndef _CELLATR_HXX
#define _CELLATR_HXX

#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include "swdllapi.h"
#include "format.hxx"
#include "cellfml.hxx"

class SW_DLLPUBLIC SwTblBoxNumFormat : public SfxUInt32Item
{
    sal_Bool bAuto;     // automatically given flag
public:
    SwTblBoxNumFormat( sal_uInt32 nFormat = NUMBERFORMAT_TEXT,
                        sal_Bool bAuto = sal_False );

    // "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    inline SwTblBoxNumFormat& operator=( const SwTblBoxNumFormat& rAttr )
    {
        SetValue( rAttr.GetValue() );
        SetAutoFlag( rAttr.GetAutoFlag() );
        return *this;
    }

    sal_Bool GetAutoFlag() const                    { return bAuto; }
    void SetAutoFlag( sal_Bool bFlag = sal_True )       { bAuto = bFlag; }
};

class SwTblBoxFormula : public SfxPoolItem, public SwTableFormula
{
    SwModify* pDefinedIn;   // Modify object where the formula is located
                            // can only be TableBoxFormat

public:
    SwTblBoxFormula( const String& rFormula );

    // "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    inline void ChgDefinedIn( const SwModify* pNew )
                                            { pDefinedIn = (SwModify*)pNew; }
    //  BoxAttribut -> BoxStartNode
    virtual const SwNode* GetNodeOfFormula() const;

          SwTableBox* GetTableBox();
    const SwTableBox* GetTableBox() const
        { return ((SwTblBoxFormula*)this)->GetTableBox(); }

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
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    inline SwTblBoxValue& operator=( const SwTblBoxValue& rCmp )
    {
        nValue = rCmp.nValue;
        return *this;
    }

    double GetValue() const                     { return nValue; }
};



//***************************************************************************

inline const SwTblBoxNumFormat      &SwAttrSet::GetTblBoxNumFmt(sal_Bool bInP) const
    {   return (const SwTblBoxNumFormat&)Get( RES_BOXATR_FORMAT,bInP); }
inline const SwTblBoxFormula        &SwAttrSet::GetTblBoxFormula(sal_Bool bInP) const
    {   return (const SwTblBoxFormula&)Get( RES_BOXATR_FORMULA,bInP); }
inline const SwTblBoxValue          &SwAttrSet::GetTblBoxValue(sal_Bool bInP) const
    {   return (const SwTblBoxValue&)Get( RES_BOXATR_VALUE, bInP); }

//***************************************************************************

inline const SwTblBoxNumFormat      &SwFmt::GetTblBoxNumFmt(sal_Bool bInP) const
    {   return aSet.GetTblBoxNumFmt(bInP); }
inline const SwTblBoxFormula        &SwFmt::GetTblBoxFormula(sal_Bool bInP) const
    {   return aSet.GetTblBoxFormula(bInP); }
inline const SwTblBoxValue          &SwFmt::GetTblBoxValue(sal_Bool bInP) const
    {   return aSet.GetTblBoxValue(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
