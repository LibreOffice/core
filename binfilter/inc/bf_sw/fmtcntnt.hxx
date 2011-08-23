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
#ifndef _FMTCNTNT_HXX
#define _FMTCNTNT_HXX

#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
namespace binfilter {

class SwNodeIndex;
class SwStartNode;

//Cntnt, Inhalt des Frame (Header, Footer, Fly) ----

class SwFmtCntnt: public SfxPoolItem
{
    SwNodeIndex *pStartNode;

    SwFmtCntnt &operator=( const SwFmtCntnt & ); //Kopieren verboten

public:
//	SwFmtCntnt( const SwNodeIndex *pStartNodeIdx = 0 );
    SwFmtCntnt( const SwStartNode* pStartNode = 0 );
    SwFmtCntnt( const SwFmtCntnt &rCpy );
    ~SwFmtCntnt();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;

    const SwNodeIndex *GetCntntIdx() const { return pStartNode; }
    void SetNewCntntIdx( const SwNodeIndex *pIdx );
};

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.hxx; revisit with gcc3
inline const SwFmtCntnt &SwAttrSet::GetCntnt(BOOL bInP) const
    { return (const SwFmtCntnt&)Get( RES_CNTNT,bInP); }

inline const SwFmtCntnt &SwFmt::GetCntnt(BOOL bInP) const
    { return aSet.GetCntnt(bInP); }
#endif

} //namespace binfilter
#endif

