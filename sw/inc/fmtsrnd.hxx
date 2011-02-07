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
#ifndef _FMTSRND_HXX
#define _FMTSRND_HXX

#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>
#include <svl/eitem.hxx>

#include <fmtsrndenum.hxx>
class IntlWrapper;

// SwFmtSurround: How document content under the frame shall behave.
class SW_DLLPUBLIC SwFmtSurround: public SfxEnumItem
{
    BOOL    bAnchorOnly :1;
    BOOL    bContour    :1;
    BOOL    bOutside    :1;
public:
    SwFmtSurround( SwSurround eNew = SURROUND_PARALLEL );
    SwFmtSurround( const SwFmtSurround & );
    inline SwFmtSurround &operator=( const SwFmtSurround &rCpy );

    // "Pure virtual Methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual USHORT          GetValueCount() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    SwSurround GetSurround()const { return SwSurround( GetValue() ); }
    BOOL    IsAnchorOnly()  const { return bAnchorOnly; }
    BOOL    IsContour()     const { return bContour; }
    BOOL    IsOutside()     const { return bOutside; }
    void    SetSurround  ( SwSurround eNew ){ SfxEnumItem::SetValue( USHORT( eNew ) ); }
    void    SetAnchorOnly( BOOL bNew )      { bAnchorOnly = bNew; }
    void    SetContour( BOOL bNew )         { bContour = bNew; }
    void    SetOutside( BOOL bNew )         { bOutside = bNew; }
};

inline SwFmtSurround &SwFmtSurround::operator=( const SwFmtSurround &rCpy )
{
    bAnchorOnly = rCpy.IsAnchorOnly();
    bContour = rCpy.IsContour();
    bOutside = rCpy.IsOutside();
    SfxEnumItem::SetValue( rCpy.GetValue() );
    return *this;
}

inline const SwFmtSurround &SwAttrSet::GetSurround(BOOL bInP) const
    { return (const SwFmtSurround&)Get( RES_SURROUND,bInP); }

inline const SwFmtSurround &SwFmt::GetSurround(BOOL bInP) const
    { return aSet.GetSurround(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
