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
#include <o3tl/typed_flags_set.hxx>


// class SvxBoxItem ------------------------------------------------------

/*  [Description]

    This item describes a border attribute
    (all four edges and the inward distance)
*/
enum class SvxBoxItemLine
{
    TOP, BOTTOM, LEFT, RIGHT, LAST = RIGHT
};

/**
This version causes SvxBoxItem to store the 4 cell spacing distances separately
when serializing to stream.
*/
#define BOX_4DISTS_VERSION ((sal_uInt16)1)
/**
This version causes SvxBoxItem to store the styles for its border lines when
serializing to stream.
*/
#define BOX_BORDER_STYLE_VERSION ((sal_uInt16)2)

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
    static SfxPoolItem* CreateDefault();

    explicit SvxBoxItem( const sal_uInt16 nId );
    SvxBoxItem( const SvxBoxItem &rCpy );
    virtual ~SvxBoxItem();
    SvxBoxItem &operator=( const SvxBoxItem& rBox );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const override;
    virtual sal_uInt16           GetVersion( sal_uInt16 nFileVersion ) const override;

    virtual bool             ScaleMetrics( long nMult, long nDiv ) override;
    virtual bool             HasMetrics() const override;

    const   editeng::SvxBorderLine* GetTop()    const { return pTop; }
    const   editeng::SvxBorderLine* GetBottom() const { return pBottom; }
    const   editeng::SvxBorderLine* GetLeft()   const { return pLeft; }
    const   editeng::SvxBorderLine* GetRight()  const { return pRight; }

    const   editeng::SvxBorderLine* GetLine( SvxBoxItemLine nLine ) const;

    //The Pointers are being copied!
    void    SetLine( const editeng::SvxBorderLine* pNew, SvxBoxItemLine nLine );

    sal_uInt16  GetDistance( SvxBoxItemLine nLine ) const;
    sal_uInt16  GetDistance() const;

    void    SetDistance( sal_uInt16 nNew, SvxBoxItemLine nLine );
    inline void SetDistance( sal_uInt16 nNew );

    // Line width plus Space plus inward distance
    // bEvenIfNoLine = TRUE -> Also return distance, when no Line is set
    sal_uInt16  CalcLineSpace( SvxBoxItemLine nLine, bool bEvenIfNoLine = false ) const;
    bool HasBorder( bool bTreatPaddingAsBorder = false ) const;
    static css::table::BorderLine2 SvxLineToLine( const editeng::SvxBorderLine* pLine, bool bConvert );
    static bool LineToSvxLine(const css::table::BorderLine& rLine, editeng::SvxBorderLine& rSvxLine, bool bConvert);
    static bool LineToSvxLine(const css::table::BorderLine2& rLine, editeng::SvxBorderLine& rSvxLine, bool bConvert);
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

enum class SvxBoxInfoItemLine
{
    HORI, VERT, LAST = VERT
};

enum class SvxBoxInfoItemValidFlags
{
    NONE          = 0x00,
    TOP           = 0x01,
    BOTTOM        = 0x02,
    LEFT          = 0x04,
    RIGHT         = 0x08,
    HORI          = 0x10,
    VERT          = 0x20,
    DISTANCE      = 0x40,
    DISABLE       = 0x80,
    ALL           = 0xff
};
namespace o3tl
{
    template<> struct typed_flags<SvxBoxInfoItemValidFlags> : is_typed_flags<SvxBoxInfoItemValidFlags, 0xff> {};
}

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

    SvxBoxInfoItemValidFlags nValidFlags;
    sal_uInt16  nDefDist;       // The default or minimum distance.

public:
    static SfxPoolItem* CreateDefault();

    explicit SvxBoxInfoItem( const sal_uInt16 nId );
    SvxBoxInfoItem( const SvxBoxInfoItem &rCpy );
    virtual ~SvxBoxInfoItem();
    SvxBoxInfoItem &operator=( const SvxBoxInfoItem &rCpy );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const override;
    virtual bool            ScaleMetrics( long nMult, long nDiv ) override;
    virtual bool            HasMetrics() const override;

    const editeng::SvxBorderLine*   GetHori() const { return pHori; }
    const editeng::SvxBorderLine*   GetVert() const { return pVert; }

    //The Pointers are being copied!
    void                    SetLine( const editeng::SvxBorderLine* pNew, SvxBoxInfoItemLine nLine );

    bool                    IsTable() const             { return mbEnableHor && mbEnableVer; }
    void                    SetTable( bool bNew )       { mbEnableHor = mbEnableVer = bNew; }

    inline bool             IsHorEnabled() const { return mbEnableHor; }
    inline void             EnableHor( bool bEnable ) { mbEnableHor = bEnable; }
    inline bool             IsVerEnabled() const { return mbEnableVer; }
    inline void             EnableVer( bool bEnable ) { mbEnableVer = bEnable; }

    bool                    IsDist() const              { return bDist; }
    void                    SetDist( bool bNew )        { bDist = bNew; }
    bool                    IsMinDist() const           { return bMinDist; }
    void                    SetMinDist( bool bNew )     { bMinDist = bNew; }
    sal_uInt16              GetDefDist() const            { return nDefDist; }
    void                    SetDefDist( sal_uInt16 nNew ) { nDefDist = nNew; }

    bool                    IsValid( SvxBoxInfoItemValidFlags nValid ) const
                                { return bool( nValidFlags & nValid ); }
    void                    SetValid( SvxBoxInfoItemValidFlags nValid, bool bValid = true )
                                { bValid ? ( nValidFlags |= nValid )
                                         : ( nValidFlags &= ~nValid ); }
    void                    ResetFlags();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
