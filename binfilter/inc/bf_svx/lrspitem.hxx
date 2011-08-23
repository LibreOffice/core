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
#ifndef _SVX_LRSPITEM_HXX
#define _SVX_LRSPITEM_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/poolitem.hxx>
#include <bf_svx/svxids.hrc>

namespace rtl
{
    class OUString;
}
namespace binfilter {

// class SvxLRSpaceItem --------------------------------------------------

/*
[Beschreibung]
Linker/Rechter Rand sowie Erstzeileneinzug

SvxLRSpaceItem bietet zwei Schnittstellen zur Befragung des linken
Randes und des Erstzeileneinzuges an. Die Get-Methoden liefern
die Member zurueck, wie sie das Layout bisher auch erwartete:
Der linke Rand verschiebt sich beim negativem Erstzeileneinzug
nach links. Die SetTxt/GetTxt-Methoden setzen voraus, dass der
linke Rand der 0-Punkt des Erstzeileneinzugs darstellt:

     UI         UI       LAYOUT   UI/TEXT      UI/TEXT    (Wo?)
SetTxtLeft SetTxtFirst GetLeft  GetTxtLeft  GetTxtFirst  (Was?)
    500       -500        0        500         -500      (Wieviel?)
    500         0        500       500           0
    500       +500       500       500         +500
    700       -500       200       700         -500
*/

#define	LRSPACE_16_VERSION		((USHORT)0x0001)
#define	LRSPACE_TXTLEFT_VERSION	((USHORT)0x0002)
#define	LRSPACE_AUTOFIRST_VERSION ((USHORT)0x0003)
#define	LRSPACE_NEGATIVE_VERSION ((USHORT)0x0004)

class SvxLRSpaceItem : public SfxPoolItem
{
    short	nFirstLineOfst;     // Erstzeileneinzug _immer_ relativ zu nTxtLeft
    long	nTxtLeft;           // wir spendieren einen USHORT
    long	nLeftMargin;        // nLeft oder der neg. Erstzeileneinzug
    long	nRightMargin;		// der unproblematische rechte Rand

    USHORT	nPropFirstLineOfst, nPropLeftMargin, nPropRightMargin;
    BOOL 	bAutoFirst	: 1;	// Automatische Berechnung der Erstzeileneinzugs
    BOOL 	bBulletFI 	: 1;	// FI = Bullet...

    void   AdjustLeft();		// nLeftMargin und nTxtLeft werden angepasst.

public:
    TYPEINFO();

    SvxLRSpaceItem( const USHORT nId = ITEMID_LRSPACE );
    SvxLRSpaceItem( const long nLeft, const long nRight,
                    const long nTLeft = 0, const short nOfset = 0,
                    const USHORT nId = ITEMID_LRSPACE );
    inline SvxLRSpaceItem& operator=( const SvxLRSpaceItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 			 operator==( const SfxPoolItem& ) const;

    virtual	bool             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 Create(SvStream &, USHORT) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion ) const;
    virtual USHORT			 GetVersion( USHORT nFileVersion ) const;

    // Die "Layout-Schnittstelle":
    inline void   SetLeft ( const long nL, const USHORT nProp = 100 );
    inline void   SetRight( const long nR, const USHORT nProp = 100 );

    // abfragen / direktes setzen der absoluten Werte
    inline long GetLeft()  const { return nLeftMargin; }
    inline long GetRight() const { return nRightMargin;}
    inline void SetLeftValue( const long nL ) { nTxtLeft = nLeftMargin = nL; }
    inline void SetRightValue( const long nR ) { nRightMargin = nR; }
    inline BOOL IsAutoFirst()  const { return bAutoFirst; }
    inline void SetAutoFirst( const BOOL bNew ) { bAutoFirst = bNew; }

    // abfragen / setzen der Prozent-Werte
    inline void SetPropLeft( const USHORT nProp = 100 )
                    { nPropLeftMargin = nProp; }
    inline void SetPropRight( const USHORT nProp = 100 )
                    { nPropRightMargin = nProp;}
    inline USHORT GetPropLeft()  const { return nPropLeftMargin; }
    inline USHORT GetPropRight() const { return nPropRightMargin;}

    // Die "UI/Text-Schnittstelle":
    inline void SetTxtLeft( const long nL, const USHORT nProp = 100 );
    inline long GetTxtLeft() const { return nTxtLeft; }

    inline void   SetTxtFirstLineOfst( const short nF, const USHORT nProp = 100 );
    inline short  GetTxtFirstLineOfst() const { return nFirstLineOfst; }
    inline void SetPropTxtFirstLineOfst( const USHORT nProp = 100 )
                    { nPropFirstLineOfst = nProp; }
    inline USHORT GetPropTxtFirstLineOfst() const
                    { return nPropFirstLineOfst; }
    inline void SetTxtFirstLineOfstValue( const short nValue )
                    { nFirstLineOfst = nValue; }

    // Outliner-Umstellung...
    void		SetBulletFI( BOOL b ) { bBulletFI = b; }
    BOOL		IsBulletFI() const { return bBulletFI; }
};

inline SvxLRSpaceItem &SvxLRSpaceItem::operator=( const SvxLRSpaceItem &rCpy )
{
    nFirstLineOfst = rCpy.nFirstLineOfst;
    nTxtLeft = rCpy.nTxtLeft;
    nLeftMargin = rCpy.nLeftMargin;
    nRightMargin = rCpy.nRightMargin;
    nPropFirstLineOfst = rCpy.nPropFirstLineOfst;
    nPropLeftMargin = rCpy.nPropLeftMargin;
    nPropRightMargin = rCpy.nPropRightMargin;
    bBulletFI = rCpy.bBulletFI;
    bAutoFirst = rCpy.bAutoFirst;
    return *this;
}

inline void SvxLRSpaceItem::SetLeft( const long nL, const USHORT nProp )
{
    nLeftMargin = (nL * nProp) / 100;
    nTxtLeft = nLeftMargin;
    nPropLeftMargin = nProp;
}
inline void SvxLRSpaceItem::SetRight( const long nR, const USHORT nProp )
{
    nRightMargin = (nR * nProp) / 100;
    nPropRightMargin = nProp;
}
inline void SvxLRSpaceItem::SetTxtFirstLineOfst( const short nF,
                                                 const USHORT nProp )
{
    nFirstLineOfst = short((long(nF) * nProp ) / 100);
    nPropFirstLineOfst = nProp;
    AdjustLeft();
}

inline void SvxLRSpaceItem::SetTxtLeft( const long nL, const USHORT nProp )
{
    nTxtLeft = (nL * nProp) / 100;
    nPropLeftMargin = nProp;
    AdjustLeft();
}

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
