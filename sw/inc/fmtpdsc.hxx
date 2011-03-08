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
#ifndef _FMTPDSC_HXX
#define _FMTPDSC_HXX


#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>
#include <calbck.hxx>

class SwPageDesc;
class SwHistory;
class SwPaM;
class IntlWrapper;

// Pagedescriptor
// Client of SwPageDesc that is "described" by the attribute.

#define IVER_FMTPAGEDESC_NOAUTO ((USHORT)0x0001)
#define IVER_FMTPAGEDESC_LONGPAGE   ((USHORT)0x0002)

class SW_DLLPUBLIC SwFmtPageDesc : public SfxPoolItem, public SwClient
{
    // This "Doc"-function is made friend in order to be able
    // to set the auto-flag after copying!!
    friend BOOL InsAttr( SwDoc*, const SwPaM &, const SfxItemSet&, USHORT,
                        SwHistory* );
    USHORT nNumOffset;          // Offset page number.
    USHORT nDescNameIdx;        // SW3-Reader: stringpool-index of style name.
    SwModify* pDefinedIn;       // Points to the object in which the
                                // attribute was set (CntntNode/Format).

public:
    SwFmtPageDesc( const SwPageDesc *pDesc = 0 );
    SwFmtPageDesc( const SwFmtPageDesc &rCpy );
    SwFmtPageDesc &operator=( const SwFmtPageDesc &rCpy );
    ~SwFmtPageDesc();

    TYPEINFO();

    // "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

          SwPageDesc *GetPageDesc() { return (SwPageDesc*)GetRegisteredIn(); }
    const SwPageDesc *GetPageDesc() const { return (SwPageDesc*)GetRegisteredIn(); }

    USHORT  GetNumOffset() const        { return nNumOffset; }
    void    SetNumOffset( USHORT nNum ) { nNumOffset = nNum; }

    // Query / set where attribute is anchored.
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    void ChgDefinedIn( const SwModify* pNew ) { pDefinedIn = (SwModify*)pNew; }
};


inline const SwFmtPageDesc &SwAttrSet::GetPageDesc(BOOL bInP) const
    { return (const SwFmtPageDesc&)Get( RES_PAGEDESC,bInP); }

inline const SwFmtPageDesc &SwFmt::GetPageDesc(BOOL bInP) const
    { return aSet.GetPageDesc(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
