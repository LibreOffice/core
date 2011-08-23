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
#ifndef _FMTANCHR_HXX
#define _FMTANCHR_HXX

#include <swtypes.hxx>
#include <format.hxx>

class IntlWrapper;
namespace binfilter {

struct SwPosition;

#define IVER_FMTANCHOR_LONGIDX ((USHORT)1)

//FlyAnchor, Anker des Freifliegenden Rahmen ----

class SwFmtAnchor: public SfxPoolItem
{
    SwPosition *pCntntAnchor;	//0 Fuer Seitengebundene Rahmen.
                                //Index fuer Absatzgebundene Rahmen.
                                //Position fuer Zeichengebundene Rahmen
    RndStdIds  nAnchorId;
    USHORT	   nPageNum;		//Seitennummer bei Seitengeb. Rahmen.

public:
    SwFmtAnchor( RndStdIds eRnd = FLY_PAGE, USHORT nPageNum = 0 );
    SwFmtAnchor( const SwFmtAnchor &rCpy );
    ~SwFmtAnchor();

    //	Zuweisungsoperator
    SwFmtAnchor &operator=( const SwFmtAnchor& );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
    virtual USHORT			GetVersion( USHORT nFFVer ) const;

    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    RndStdIds GetAnchorId() const { return nAnchorId; }
    USHORT GetPageNum() const { return nPageNum; }
    const SwPosition *GetCntntAnchor() const { return pCntntAnchor; }

    void SetPageNum( USHORT nNew ) { nPageNum = nNew; }

    void SetType( RndStdIds nRndId ) { nAnchorId = nRndId; }
    void SetAnchor( const SwPosition *pPos );
};

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwFmtAnchor &SwAttrSet::GetAnchor(BOOL bInP) const
     { return (const SwFmtAnchor&)Get( RES_ANCHOR,bInP); }

 inline const SwFmtAnchor &SwFmt::GetAnchor(BOOL bInP) const
     { return aSet.GetAnchor(bInP); }
#endif

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
