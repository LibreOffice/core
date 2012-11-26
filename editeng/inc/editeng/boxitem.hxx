/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    POOLITEM_FACTORY()
    SvxBoxItem( const sal_uInt16 nId = 0 );
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

    virtual void             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

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
    POOLITEM_FACTORY()
    SvxBoxInfoItem( const sal_uInt16 nId = 0 );
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
    virtual void            ScaleMetrics( long nMult, long nDiv );
    virtual bool            HasMetrics() const;

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

