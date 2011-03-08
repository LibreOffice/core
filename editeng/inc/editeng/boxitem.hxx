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

#define BOX_LINE_TOP    ((USHORT)0)
#define BOX_LINE_BOTTOM ((USHORT)1)
#define BOX_LINE_LEFT   ((USHORT)2)
#define BOX_LINE_RIGHT  ((USHORT)3)

#define BOX_4DISTS_VERSION ((USHORT)1)

class EDITENG_DLLPUBLIC SvxBoxItem : public SfxPoolItem
{
    SvxBorderLine  *pTop,
                   *pBottom,
                   *pLeft,
                   *pRight;
    USHORT          nTopDist,
                    nBottomDist,
                    nLeftDist,
                    nRightDist;

public:
    TYPEINFO();

    SvxBoxItem( const USHORT nId );
    SvxBoxItem( const SvxBoxItem &rCpy );
    ~SvxBoxItem();
    SvxBoxItem &operator=( const SvxBoxItem& rBox );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion ) const;
    virtual USHORT           GetVersion( USHORT nFileVersion ) const;

    virtual bool             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    const   SvxBorderLine* GetTop()    const { return pTop; }
    const   SvxBorderLine* GetBottom() const { return pBottom; }
    const   SvxBorderLine* GetLeft()   const { return pLeft; }
    const   SvxBorderLine* GetRight()  const { return pRight; }

    const   SvxBorderLine* GetLine( USHORT nLine ) const;

        //The Pointers are being copied!
    void    SetLine( const SvxBorderLine* pNew, USHORT nLine );

    USHORT  GetDistance( USHORT nLine ) const;
    USHORT  GetDistance() const;

    void    SetDistance( USHORT nNew, USHORT nLine );
    inline void SetDistance( USHORT nNew );

        // Line width plus Space plus inward distance
        //bIgnoreLine = TRUE -> Also return distance, when no Line is set
    USHORT  CalcLineSpace( USHORT nLine, BOOL bIgnoreLine = FALSE ) const;
    static com::sun::star::table::BorderLine2 SvxLineToLine( const SvxBorderLine* pLine, sal_Bool bConvert );
    static sal_Bool LineToSvxLine(const ::com::sun::star::table::BorderLine& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert);
    static sal_Bool LineToSvxLine(const ::com::sun::star::table::BorderLine2& rLine, SvxBorderLine& rSvxLine, sal_Bool bConvert);
};

inline void SvxBoxItem::SetDistance( USHORT nNew )
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

#define BOXINFO_LINE_HORI   ((USHORT)0)
#define BOXINFO_LINE_VERT   ((USHORT)1)

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

    BOOL    bDist      :1;  // TRUE, Unlock Distance.
    BOOL    bMinDist   :1;  // TRUE, Going below minimum Distance is prohibited

    BYTE    nValidFlags;    // 0000 0000
                            // ³³³³ ³³³ÀÄ VALID_TOP
                            // ³³³³ ³³ÀÄÄ VALID_BOTTOM
                            // ³³³³ ³ÀÄÄÄ VALID_LEFT
                            // ³³³³ ÀÄÄÄÄ VALID_RIGHT
                            // ³³³ÀÄÄÄÄÄÄ VALID_HORI
                            // ³³ÀÄÄÄÄÄÄÄ VALID_VERT
                            // ³ÀÄÄÄÄÄÄÄÄ VALID_DIST
                            // ÀÄÄÄÄÄÄÄÄÄ VALID_DISABLE

    USHORT  nDefDist;       // The default or minimum distance.

public:
    TYPEINFO();

    SvxBoxInfoItem( const USHORT nId );
    SvxBoxInfoItem( const SvxBoxInfoItem &rCpy );
    ~SvxBoxInfoItem();
    SvxBoxInfoItem &operator=( const SvxBoxInfoItem &rCpy );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT) const;
    virtual SvStream&       Store(SvStream &, USHORT nItemVersion ) const;
    virtual bool            ScaleMetrics( long nMult, long nDiv );
    virtual bool            HasMetrics() const;

    const SvxBorderLine*    GetHori() const { return pHori; }
    const SvxBorderLine*    GetVert() const { return pVert; }

        //The Pointers are being copied!
    void                    SetLine( const SvxBorderLine* pNew, USHORT nLine );

    BOOL    IsTable() const             { return mbEnableHor && mbEnableVer; }
    void    SetTable( BOOL bNew )       { mbEnableHor = mbEnableVer = bNew; }

    inline bool         IsHorEnabled() const { return mbEnableHor; }
    inline void         EnableHor( bool bEnable ) { mbEnableHor = bEnable; }
    inline bool         IsVerEnabled() const { return mbEnableVer; }
    inline void         EnableVer( bool bEnable ) { mbEnableVer = bEnable; }

    BOOL    IsDist() const              { return bDist; }
    void    SetDist( BOOL bNew )        { bDist = bNew; }
    BOOL    IsMinDist() const           { return bMinDist; }
    void    SetMinDist( BOOL bNew )     { bMinDist = bNew; }
    USHORT  GetDefDist() const          { return nDefDist; }
    void    SetDefDist( USHORT nNew )   { nDefDist = nNew; }

    BOOL                    IsValid( BYTE nValid ) const
                                { return ( nValidFlags & nValid ) == nValid; }
    void                    SetValid( BYTE nValid, BOOL bValid = TRUE )
                                { bValid ? ( nValidFlags |= nValid )
                                         : ( nValidFlags &= ~nValid ); }
    void                    ResetFlags();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
