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
#ifndef SW_FMTLINE_HXX
#define SW_FMTLINE_HXX

#include <format.hxx>

class IntlWrapper;

namespace binfilter {

class SwFmtLineNumber: public SfxPoolItem
{
    ULONG nStartValue	:24; //Startwert fuer den Absatz, 0 == kein Startwert
    ULONG bCountLines	:1;	 //Zeilen des Absatzes sollen mitgezaehlt werden.

public:
    SwFmtLineNumber();
    ~SwFmtLineNumber();

    TYPEINFO();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
    virtual USHORT			GetVersion( USHORT nFFVer ) const;
    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    ULONG GetStartValue() const { return nStartValue; }
    BOOL  IsCount()		  const { return bCountLines != 0; }

    void SetStartValue( ULONG nNew ) { nStartValue = nNew; }
    void SetCountLines( BOOL b )     { bCountLines = b;	   }
};

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwFmtLineNumber &SwAttrSet::GetLineNumber(BOOL bInP) const
    { return (const SwFmtLineNumber&)Get( RES_LINENUMBER,bInP); }

inline const SwFmtLineNumber &SwFmt::GetLineNumber(BOOL bInP) const
    { return aSet.GetLineNumber(bInP); }
#endif

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
