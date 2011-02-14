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
#ifndef _SVX_BOXITEM_HXX
#define _SVX_BOXITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/editengdllapi.h>
#include <com/sun/star/table/BorderLine.hpp>

namespace rtl { class OUString; }

// class SvxBoxItem ------------------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt ein Umrandungsattribut (alle vier Kanten und
Abstand nach innen.
*/

#define BOX_LINE_TOP    ((sal_uInt16)0)
#define BOX_LINE_BOTTOM ((sal_uInt16)1)
#define BOX_LINE_LEFT   ((sal_uInt16)2)
#define BOX_LINE_RIGHT  ((sal_uInt16)3)

#define BOX_4DISTS_VERSION ((sal_uInt16)1)

class EDITENG_DLLPUBLIC SvxBoxItem : public SfxPoolItem
{
    SvxBorderLine  *pTop,
                   *pBottom,
                   *pLeft,
                   *pRight;
    sal_uInt16          nTopDist,
                    nBottomDist,
                    nLeftDist,
                    nRightDist;

public:
    TYPEINFO();

    SvxBoxItem( const sal_uInt16 nId );
    SvxBoxItem( const SvxBoxItem &rCpy );
    ~SvxBoxItem();
    SvxBoxItem &operator=( const SvxBoxItem& rBox );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual sal_uInt16           GetVersion( sal_uInt16 nFileVersion ) const;

    virtual int              ScaleMetrics( long nMult, long nDiv );
    virtual int              HasMetrics() const;

    const   SvxBorderLine* GetTop()    const { return pTop; }
    const   SvxBorderLine* GetBottom() const { return pBottom; }
    const   SvxBorderLine* GetLeft()   const { return pLeft; }
    const   SvxBorderLine* GetRight()  const { return pRight; }

    const   SvxBorderLine* GetLine( sal_uInt16 nLine ) const;

        //Die Pointer werden kopiert!
    void    SetLine( const SvxBorderLine* pNew, sal_uInt16 nLine );

    sal_uInt16  GetDistance( sal_uInt16 nLine ) const;
    sal_uInt16  GetDistance() const;

    void    SetDistance( sal_uInt16 nNew, sal_uInt16 nLine );
    inline void SetDistance( sal_uInt16 nNew );

        //Breite der Linien plus Zwischenraum plus Abstand nach innen.
        //JP 09.06.99: bIgnoreLine = sal_True -> Distance auch returnen, wenn
        //                          keine Line gesetzt ist
    sal_uInt16  CalcLineSpace( sal_uInt16 nLine, sal_Bool bIgnoreLine = sal_False ) const;
    static com::sun::star::table::BorderLine SvxLineToLine( const SvxBorderLine* pLine, sal_Bool bConvert );
    static sal_Bool LineToSvxLine(const ::com::sun::star::table::BorderLine& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert);
};

inline void SvxBoxItem::SetDistance( sal_uInt16 nNew )
{
    nTopDist = nBottomDist = nLeftDist = nRightDist = nNew;
}

// class SvxBoxInfoItem --------------------------------------------------

/*
[Beschreibung]
Noch ein Item fuer die Umrandung. Dieses Item hat lediglich SS-Funktionalitaet.
Einerseits wird dem allgemeinen Dialog mit diesem Item mitgeteilt, welche
Moeglichkeiten er anbieten soll.
Andererseits werden ueber dieses Attribut ggf. die BorderLines fuer die
horizontalen und vertikalen innerern Linien transportiert.
*/

#define BOXINFO_LINE_HORI   ((sal_uInt16)0)
#define BOXINFO_LINE_VERT   ((sal_uInt16)1)

#define VALID_TOP           0x01
#define VALID_BOTTOM        0x02
#define VALID_LEFT          0x04
#define VALID_RIGHT         0x08
#define VALID_HORI          0x10
#define VALID_VERT          0x20
#define VALID_DISTANCE      0x40
#define VALID_DISABLE       0x80

class EDITENG_DLLPUBLIC SvxBoxInfoItem : public SfxPoolItem
{
    SvxBorderLine* pHori;   //innere horizontale Linie
    SvxBorderLine* pVert;   //innere vertikale Linie

    bool                mbEnableHor;   /// true = Enable inner horizonal line.
    bool                mbEnableVer;   /// true = Enable inner vertical line.

    /*
     z.Z. nur fuer StarWriter: Abstand nach innen von SvxBoxItem.
     Wenn der Abstand gewuenscht ist, so muss das Feld fuer den Abstand vom
     Dialog freigeschaltet werden. nDefDist ist als Defaultwert anzusehen.
     Wenn irgendeine    Linie eingeschalt ist oder wird, so muss dieser
     Abstand defaultet werden. bMinDist gibt an, ob der Wert durch den
     Anwender unterschritten werden darf. Mit nDist wird der aktuelle
     Abstand von der App zum Dialog und zurueck transportiert.
    */

    sal_Bool    bDist      :1;  // sal_True, Abstand freischalten.
    sal_Bool    bMinDist   :1;  // sal_True, Abstand darf nicht unterschritten werden.

    sal_uInt8   nValidFlags;    // 0000 0000
                            // ³³³³ ³³³ÀÄ VALID_TOP
                            // ³³³³ ³³ÀÄÄ VALID_BOTTOM
                            // ³³³³ ³ÀÄÄÄ VALID_LEFT
                            // ³³³³ ÀÄÄÄÄ VALID_RIGHT
                            // ³³³ÀÄÄÄÄÄÄ VALID_HORI
                            // ³³ÀÄÄÄÄÄÄÄ VALID_VERT
                            // ³ÀÄÄÄÄÄÄÄÄ VALID_DIST
                            // ÀÄÄÄÄÄÄÄÄÄ VALID_DISABLE

    sal_uInt16  nDefDist;       // Der Default- bzw. Minimalabstand.

public:
    TYPEINFO();

    SvxBoxInfoItem( const sal_uInt16 nId );
    SvxBoxInfoItem( const SvxBoxInfoItem &rCpy );
    ~SvxBoxInfoItem();
    SvxBoxInfoItem &operator=( const SvxBoxInfoItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual int              ScaleMetrics( long nMult, long nDiv );
    virtual int              HasMetrics() const;

    const SvxBorderLine*    GetHori() const { return pHori; }
    const SvxBorderLine*    GetVert() const { return pVert; }

    //Die Pointer werden kopiert!
    void                    SetLine( const SvxBorderLine* pNew, sal_uInt16 nLine );

    sal_Bool    IsTable() const             { return mbEnableHor && mbEnableVer; }
    void    SetTable( sal_Bool bNew )       { mbEnableHor = mbEnableVer = bNew; }

    inline bool         IsHorEnabled() const { return mbEnableHor; }
    inline void         EnableHor( bool bEnable ) { mbEnableHor = bEnable; }
    inline bool         IsVerEnabled() const { return mbEnableVer; }
    inline void         EnableVer( bool bEnable ) { mbEnableVer = bEnable; }

    sal_Bool    IsDist() const              { return bDist; }
    void    SetDist( sal_Bool bNew )        { bDist = bNew; }
    sal_Bool    IsMinDist() const           { return bMinDist; }
    void    SetMinDist( sal_Bool bNew )     { bMinDist = bNew; }
    sal_uInt16  GetDefDist() const          { return nDefDist; }
    void    SetDefDist( sal_uInt16 nNew )   { nDefDist = nNew; }

    sal_Bool                    IsValid( sal_uInt8 nValid ) const
                                { return ( nValidFlags & nValid ) == nValid; }
    void                    SetValid( sal_uInt8 nValid, sal_Bool bValid = sal_True )
                                { bValid ? ( nValidFlags |= nValid )
                                         : ( nValidFlags &= ~nValid ); }
    void                    ResetFlags();
};
#endif

