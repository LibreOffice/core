/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtanchr.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 08:54:26 $
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
#ifndef _FMTANCHR_HXX
#define _FMTANCHR_HXX

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SWTYPES_HXX //autogen
#include <swtypes.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

struct SwPosition;
class IntlWrapper;
#define IVER_FMTANCHOR_LONGIDX ((USHORT)1)

//FlyAnchor, Anker des Freifliegenden Rahmen ----

class SW_DLLPUBLIC SwFmtAnchor: public SfxPoolItem
{
    SwPosition *pCntntAnchor;   //0 Fuer Seitengebundene Rahmen.
                                //Index fuer Absatzgebundene Rahmen.
                                //Position fuer Zeichengebundene Rahmen
    RndStdIds  nAnchorId;
    USHORT     nPageNum;        //Seitennummer bei Seitengeb. Rahmen.

    // OD 2004-05-05 #i28701# - getting anchor positions ordered
    sal_uInt32 mnOrder;
    static sal_uInt32 mnOrderCounter;

public:
    SwFmtAnchor( RndStdIds eRnd = FLY_PAGE, USHORT nPageNum = 0 );
    SwFmtAnchor( const SwFmtAnchor &rCpy );
    ~SwFmtAnchor();

    //  Zuweisungsoperator
    SwFmtAnchor &operator=( const SwFmtAnchor& );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    RndStdIds GetAnchorId() const { return nAnchorId; }
    USHORT GetPageNum() const { return nPageNum; }
    const SwPosition *GetCntntAnchor() const { return pCntntAnchor; }
    // OD 2004-05-05 #i28701#
    sal_uInt32 GetOrder() const;

    void SetType( RndStdIds nRndId ) { nAnchorId = nRndId; }
    void SetPageNum( USHORT nNew ) { nPageNum = nNew; }
    void SetAnchor( const SwPosition *pPos );
};

inline const SwFmtAnchor &SwAttrSet::GetAnchor(BOOL bInP) const
     { return (const SwFmtAnchor&)Get( RES_ANCHOR,bInP); }

 inline const SwFmtAnchor &SwFmt::GetAnchor(BOOL bInP) const
     { return aSet.GetAnchor(bInP); }

#endif

