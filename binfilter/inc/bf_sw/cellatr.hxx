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

#include <bf_svtools/intitem.hxx>
#include <bf_svtools/zforlist.hxx>

#include "format.hxx"
#include "cellfml.hxx"
namespace binfilter {


class SwTblBoxNumFormat : public SfxUInt32Item
{
    BOOL bAuto;		// automatisch vergebenes Flag
public:
    SwTblBoxNumFormat( UINT32 nFormat = NUMBERFORMAT_TEXT,
                        BOOL bAuto = FALSE );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
    virtual USHORT			GetVersion( USHORT nFileVersion) const;

    inline SwTblBoxNumFormat& operator=( const SwTblBoxNumFormat& rAttr )
    {
        SetValue( rAttr.GetValue() );
        SetAutoFlag( rAttr.GetAutoFlag() );
        return *this;
    }

    BOOL GetAutoFlag() const 					{ return bAuto; }
    void SetAutoFlag( BOOL bFlag = TRUE )		{ bAuto = bFlag; }
};

class SwTblBoxFormula : public SfxPoolItem, public SwTableFormula
{
    SwModify* pDefinedIn;	// Modify-Object, in dem die Formel steht
                            // kann nur TablenBoxFormat sein

public:
    SwTblBoxFormula( const String& rFormula );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;
    virtual USHORT			GetVersion( USHORT nFileVersion) const;

    // erfrage und setze den Modify-Pointer
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    inline void ChgDefinedIn( const SwModify* pNew )
                                            { pDefinedIn = (SwModify*)pNew; }
    // suche den Node, in dem die Formel steht:
    //	BoxAttribut	-> BoxStartNode
    virtual const SwNode* GetNodeOfFormula() const;

          SwTableBox* GetTableBox();
    const SwTableBox* GetTableBox() const
        { return ((SwTblBoxFormula*)this)->GetTableBox(); }
};

class SwTblBoxValue : public SfxPoolItem
{
    double nValue;
public:
    SwTblBoxValue();
    SwTblBoxValue( const double aVal );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
    virtual USHORT			GetVersion( USHORT nFileVersion) const;

    inline SwTblBoxValue& operator=( const SwTblBoxValue& rCmp )
    {
        nValue = rCmp.nValue;
        return *this;
    }

    double GetValue() const 					{ return nValue; }
    void SetValue( const double nVal )			{ nValue = nVal; }
};



//***************************************************************************

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwTblBoxNumFormat      &SwAttrSet::GetTblBoxNumFmt(BOOL bInP) const
    {   return (const SwTblBoxNumFormat&)Get( RES_BOXATR_FORMAT,bInP); }
inline const SwTblBoxFormula		&SwAttrSet::GetTblBoxFormula(BOOL bInP) const
    {   return (const SwTblBoxFormula&)Get( RES_BOXATR_FORMULA,bInP); }
inline const SwTblBoxValue			&SwAttrSet::GetTblBoxValue(BOOL bInP) const
    {   return (const SwTblBoxValue&)Get( RES_BOXATR_VALUE, bInP); }
#endif

//***************************************************************************

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwTblBoxNumFormat		&SwFmt::GetTblBoxNumFmt(BOOL bInP) const
    {   return aSet.GetTblBoxNumFmt(bInP); }
inline const SwTblBoxFormula		&SwFmt::GetTblBoxFormula(BOOL bInP) const
    {   return aSet.GetTblBoxFormula(bInP); }
inline const SwTblBoxValue			&SwFmt::GetTblBoxValue(BOOL bInP) const
    {   return aSet.GetTblBoxValue(bInP); }
#endif

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
