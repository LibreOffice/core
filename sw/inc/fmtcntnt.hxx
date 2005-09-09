/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtcntnt.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:48:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FMTCNTNT_HXX
#define _FMTCNTNT_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif

class SwNodeIndex;
class SwStartNode;

//Cntnt, Inhalt des Frame (Header, Footer, Fly) ----

class SwFmtCntnt: public SfxPoolItem
{
    SwNodeIndex *pStartNode;

    SwFmtCntnt &operator=( const SwFmtCntnt & ); //Kopieren verboten

public:
//  SwFmtCntnt( const SwNodeIndex *pStartNodeIdx = 0 );
    SwFmtCntnt( const SwStartNode* pStartNode = 0 );
    SwFmtCntnt( const SwFmtCntnt &rCpy );
    ~SwFmtCntnt();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    const SwNodeIndex *GetCntntIdx() const { return pStartNode; }
    void SetNewCntntIdx( const SwNodeIndex *pIdx );
};

inline const SwFmtCntnt &SwAttrSet::GetCntnt(BOOL bInP) const
    { return (const SwFmtCntnt&)Get( RES_CNTNT,bInP); }

inline const SwFmtCntnt &SwFmt::GetCntnt(BOOL bInP) const
    { return aSet.GetCntnt(bInP); }

#endif

