/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _CELLATR_HXX
#define _CELLATR_HXX

#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include "swdllapi.h"
#include "format.hxx"
#include "cellfml.hxx"

class SW_DLLPUBLIC SwTblBoxNumFormat : public SfxUInt32Item
{
    sal_Bool bAuto;     // automatisch vergebenes Flag
public:
    SwTblBoxNumFormat( sal_uInt32 nFormat = NUMBERFORMAT_TEXT,
                        sal_Bool bAuto = sal_False );

    // "pure virtual Methoden" vom SfxPoolItem
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
    SwModify* pDefinedIn;   // Modify-Object, in dem die Formel steht
                            // kann nur TablenBoxFormat sein

public:
    SwTblBoxFormula( const String& rFormula );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    // erfrage und setze den Modify-Pointer
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    inline void ChgDefinedIn( const SwModify* pNew )
                                            { pDefinedIn = (SwModify*)pNew; }
    // suche den Node, in dem die Formel steht:
    //  BoxAttribut -> BoxStartNode
    virtual const SwNode* GetNodeOfFormula() const;

          SwTableBox* GetTableBox();
    const SwTableBox* GetTableBox() const
        { return ((SwTblBoxFormula*)this)->GetTableBox(); }

    // Status aendern
    void ChangeState( const SfxPoolItem* pItem );
    // berechne die Formel
    void Calc( SwTblCalcPara& rCalcPara, double& rValue );
};

class SW_DLLPUBLIC SwTblBoxValue : public SfxPoolItem
{
    double nValue;
public:
    SwTblBoxValue();
    SwTblBoxValue( const double aVal );

    // "pure virtual Methoden" vom SfxPoolItem
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
