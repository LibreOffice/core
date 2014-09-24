/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EDITENG_BOXITEM_HXX
#define INCLUDED_EDITENG_BOXITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/editengdllapi.h>
#include <com/sun/star/table/BorderLine2.hpp>


// class SvxBoxItem ------------------------------------------------------

/*  [Description]

    This item describes a border attribute
    (all four edges and the inward distance)
*/

#define BOX_LINE_TOP    ((sal_uInt16)0)
#define BOX_LINE_BOTTOM ((sal_uInt16)1)
#define BOX_LINE_LEFT   ((sal_uInt16)2)
#define BOX_LINE_RIGHT  ((sal_uInt16)3)

class EDITENG_DLLPUBLIC SvxBoxItem : public SfxPoolItem
{
    editeng::SvxBorderLine  *pTop,
                            *pBottom,
                            *pLeft,
                            *pRight;
    sal_uInt16      nTopDist,
                    nBottomDist,
                    nLeftDist,
                    nRightDist;

public:
    TYPEINFO_OVERRIDE();

    explicit SvxBoxItem( const sal_uInt16 nId );
    SvxBoxItem( const SvxBoxItem &rCpy );
    virtual ~SvxBoxItem();
    SvxBoxItem &operator=( const SvxBoxItem& rBox );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const SAL_OVERRIDE;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxPoolItem*     Create(SvStream &) const SAL_OVERRIDE;
    virtual SvStream&        Store(SvStream &) const SAL_OVERRIDE;

    virtual bool             ScaleMetrics( long nMult, long nDiv ) SAL_OVERRIDE;
    virtual bool             HasMetrics() const SAL_OVERRIDE;

    const   editeng::SvxBorderLine* GetTop()    const { return pTop; }
    const   editeng::SvxBorderLine* GetBottom() const { return pBottom; }
    const   editeng::SvxBorderLine* GetLeft()   const { return pLeft; }
    const   editeng::SvxBorderLine* GetRight()  const { return pRight; }

    const   editeng::SvxBorderLine* GetLine( sal_uInt16 nLine ) const;

    //The Pointers are being copied!
    void    SetLine( const editeng::SvxBorderLine* pNew, sal_uInt16 nLine );

    sal_uInt16  GetDistance( sal_uInt16 nLine ) const;
    sal_uInt16  GetDistance() const;

    void    SetDistance( sal_uInt16 nNew, sal_uInt16 nLine );
    inline void SetDistance( sal_uInt16 nNew );

    // Line width plus Space plus inward distance
    // bIgnoreLine = TRUE -> Also return distance, when no Line is set
    sal_uInt16  CalcLineSpace( sal_uInt16 nLine, bool bIgnoreLine = false ) const;
    static com::sun::star::table::BorderLine2 SvxLineToLine( const editeng::SvxBorderLine* pLine, bool bConvert );
    static bool LineToSvxLine(const ::com::sun::star::table::BorderLine& rLine, editeng::SvxBorderLine& rSvxLine, bool bConvert);
    static bool LineToSvxLine(const ::com::sun::star::table::BorderLine2& rLine, editeng::SvxBorderLine& rSvxLine, bool bConvert);
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
    editeng::SvxBorderLine* pHori;   //inner horizontal Line
    editeng::SvxBorderLine* pVert;   //inner vertical Line

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

    bool        bDist      :1;  // TRUE, Unlock Distance.
    bool        bMinDist   :1;  // TRUE, Going below minimum Distance is prohibited

    sal_uInt8   nValidFlags;    // 0000 0000
                                // 0000 0001 VALID_TOP
                                // 0000 0010 VALID_BOTTOM
                                // 0000 0100 VALID_LEFT
                                // 0000 1000 VALID_RIGHT
                                // 0001 0000 VALID_HORI
                                // 0010 0000 VALID_VERT
                                // 0100 0000 VALID_DIST
                                // 1000 0000 VALID_DISABLE

    sal_uInt16  nDefDist;       // The default or minimum distance.

public:
    TYPEINFO_OVERRIDE();

    explicit SvxBoxInfoItem( const sal_uInt16 nId );
    SvxBoxInfoItem( const SvxBoxInfoItem &rCpy );
    virtual ~SvxBoxInfoItem();
    SvxBoxInfoItem &operator=( const SvxBoxInfoItem &rCpy );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool             operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const SAL_OVERRIDE;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Create(SvStream &) const SAL_OVERRIDE;
    virtual SvStream&       Store(SvStream &) const SAL_OVERRIDE;
    virtual bool             ScaleMetrics( long nMult, long nDiv ) SAL_OVERRIDE;
    virtual bool             HasMetrics() const SAL_OVERRIDE;

    const editeng::SvxBorderLine*   GetHori() const { return pHori; }
    const editeng::SvxBorderLine*   GetVert() const { return pVert; }

    //The Pointers are being copied!
    void                    SetLine( const editeng::SvxBorderLine* pNew, sal_uInt16 nLine );

    bool    IsTable() const             { return mbEnableHor && mbEnableVer; }
    void    SetTable( bool bNew )       { mbEnableHor = mbEnableVer = bNew; }

    inline bool         IsHorEnabled() const { return mbEnableHor; }
    inline void         EnableHor( bool bEnable ) { mbEnableHor = bEnable; }
    inline bool         IsVerEnabled() const { return mbEnableVer; }
    inline void         EnableVer( bool bEnable ) { mbEnableVer = bEnable; }

    bool    IsDist() const              { return bDist; }
    void    SetDist( bool bNew )        { bDist = bNew; }
    bool    IsMinDist() const           { return bMinDist; }
    void    SetMinDist( bool bNew )     { bMinDist = bNew; }
    sal_uInt16  GetDefDist() const          { return nDefDist; }
    void    SetDefDist( sal_uInt16 nNew )   { nDefDist = nNew; }

    bool                    IsValid( sal_uInt8 nValid ) const
                                { return ( nValidFlags & nValid ) == nValid; }
    void                    SetValid( sal_uInt8 nValid, bool bValid = true )
                                { bValid ? ( nValidFlags |= nValid )
                                         : ( nValidFlags &= ~nValid ); }
    void                    ResetFlags();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
