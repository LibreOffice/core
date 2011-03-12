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
#ifndef _SVX_BOXITEM_HXX
#define _SVX_BOXITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/editengdllapi.h>
#include <com/sun/star/table/BorderLine2.hpp>

namespace rtl { class OUString; }

// class SvxBoxItem ------------------------------------------------------

/*  [Description]

    This item describes a border attribute
    (all four edges and the inward distance)
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

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual sal_uInt16           GetVersion( sal_uInt16 nFileVersion ) const;

    virtual bool             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    const   SvxBorderLine* GetTop()    const { return pTop; }
    const   SvxBorderLine* GetBottom() const { return pBottom; }
    const   SvxBorderLine* GetLeft()   const { return pLeft; }
    const   SvxBorderLine* GetRight()  const { return pRight; }

    const   SvxBorderLine* GetLine( sal_uInt16 nLine ) const;

        //The Pointers are being copied!
    void    SetLine( const SvxBorderLine* pNew, sal_uInt16 nLine );

    sal_uInt16  GetDistance( sal_uInt16 nLine ) const;
    sal_uInt16  GetDistance() const;

    void    SetDistance( sal_uInt16 nNew, sal_uInt16 nLine );
    inline void SetDistance( sal_uInt16 nNew );

        // Line width plus Space plus inward distance
        //bIgnoreLine = TRUE -> Also return distance, when no Line is set
    sal_uInt16  CalcLineSpace( sal_uInt16 nLine, sal_Bool bIgnoreLine = sal_False ) const;
    static com::sun::star::table::BorderLine SvxLineToLine( const SvxBorderLine* pLine, sal_Bool bConvert );
    static sal_Bool LineToSvxLine(const ::com::sun::star::table::BorderLine& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert);
    static sal_Bool LineToSvxLine(const ::com::sun::star::table::BorderLine2& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert);
};

inline void SvxBoxItem::SetDistance( sal_uInt16 nNew )
{
    nTopDist = nBottomDist = nLeftDist = nRightDist = nNew;
}

// class SvxBoxInfoItem --------------------------------------------------

/*  [Description]

    Another item for the border. This item has only limited functionality.
    On one hand, the general Dialog is told by the item which options it
    should offer. On the other hand, this attribute  may be used to
    transported the borderline for the inner horizontal and vertical lines.
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
    SvxBorderLine* pHori;   //inner horizontal Line
    SvxBorderLine* pVert;   //inner vertical Line

    bool                mbEnableHor;   /// true = Enable inner horizontal line.
    bool                mbEnableVer;   /// true = Enable inner vertical line.

    /*
     Currently only for StarWriter: distance inward from SvxBoxItem. If the
     distance is requested, then the field for the distance from the dialog be
     activated. nDefDist is regarded as a default value. If any line is
     turned on or will be turned on it must this distance be set to default.
     bMinDist indicates whether the user can go below this value or not.
     With NDIST is the current distance from the app transported back and
     forth to the dialogue.
    */

    sal_Bool    bDist      :1;  // TRUE, Unlock Distance.
    sal_Bool    bMinDist   :1;  // TRUE, Going below minimum Distance is prohibited

    sal_uInt8   nValidFlags;    // 0000 0000
                            // ³³³³ ³³³ÀÄ VALID_TOP
                            // ³³³³ ³³ÀÄÄ VALID_BOTTOM
                            // ³³³³ ³ÀÄÄÄ VALID_LEFT
                            // ³³³³ ÀÄÄÄÄ VALID_RIGHT
                            // ³³³ÀÄÄÄÄÄÄ VALID_HORI
                            // ³³ÀÄÄÄÄÄÄÄ VALID_VERT
                            // ³ÀÄÄÄÄÄÄÄÄ VALID_DIST
                            // ÀÄÄÄÄÄÄÄÄÄ VALID_DISABLE

    sal_uInt16  nDefDist;       // The default or minimum distance.

public:
    TYPEINFO();

    SvxBoxInfoItem( const sal_uInt16 nId );
    SvxBoxInfoItem( const SvxBoxInfoItem &rCpy );
    ~SvxBoxInfoItem();
    SvxBoxInfoItem &operator=( const SvxBoxInfoItem &rCpy );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual int              ScaleMetrics( long nMult, long nDiv );
    virtual int              HasMetrics() const;

    const SvxBorderLine*    GetHori() const { return pHori; }
    const SvxBorderLine*    GetVert() const { return pVert; }

        //The Pointers are being copied!
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
