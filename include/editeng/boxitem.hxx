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
#include <editeng/editengdllapi.h>
#include <com/sun/star/table/BorderLine2.hpp>
#include <o3tl/typed_flags_set.hxx>
#include <memory>


namespace editeng { class SvxBorderLine; }

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
constexpr sal_uInt16 BOX_4DISTS_VERSION = 1;
/**
This version causes SvxBoxItem to store the styles for its border lines when
serializing to stream.
*/
constexpr sal_uInt16 BOX_BORDER_STYLE_VERSION = 2;

class EDITENG_DLLPUBLIC SvxBoxItem final : public SfxPoolItem
{
    std::unique_ptr<editeng::SvxBorderLine>
                    pTop,
                    pBottom,
                    pLeft,
                    pRight;
    sal_uInt16      nTopDist,
                    nBottomDist,
                    nLeftDist,
                    nRightDist;
    bool            bRemoveAdjCellBorder;

public:
    static SfxPoolItem* CreateDefault();

    explicit SvxBoxItem( const sal_uInt16 nId );
    SvxBoxItem( const SvxBoxItem &rCpy );
    virtual ~SvxBoxItem() override;

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    MapUnit eCoreMetric,
                                    MapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper& ) const override;

    virtual SvxBoxItem*      Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual void             ScaleMetrics( tools::Long nMult, tools::Long nDiv ) override;
    virtual bool             HasMetrics() const override;

    const   editeng::SvxBorderLine* GetTop()    const { return pTop.get(); }
    const   editeng::SvxBorderLine* GetBottom() const { return pBottom.get(); }
    const   editeng::SvxBorderLine* GetLeft()   const { return pLeft.get(); }
    const   editeng::SvxBorderLine* GetRight()  const { return pRight.get(); }

    const   editeng::SvxBorderLine* GetLine( SvxBoxItemLine nLine ) const;

    //The Pointers are being copied!
    void    SetLine( const editeng::SvxBorderLine* pNew, SvxBoxItemLine nLine );

    sal_uInt16  GetDistance( SvxBoxItemLine nLine ) const;
    sal_uInt16  GetSmallestDistance() const;

    bool IsRemoveAdjacentCellBorder() const { return bRemoveAdjCellBorder; }

    void    SetDistance( sal_uInt16 nNew, SvxBoxItemLine nLine );
    inline void SetAllDistances( sal_uInt16 nNew );

    void SetRemoveAdjacentCellBorder( bool bSet ) { bRemoveAdjCellBorder = bSet; }

    // Line width plus Space plus inward distance
    // bEvenIfNoLine = TRUE -> Also return distance, when no Line is set
    sal_uInt16  CalcLineWidth( SvxBoxItemLine nLine ) const;
    sal_uInt16  CalcLineSpace( SvxBoxItemLine nLine, bool bEvenIfNoLine = false ) const;
    bool HasBorder( bool bTreatPaddingAsBorder ) const;
    static css::table::BorderLine2 SvxLineToLine( const editeng::SvxBorderLine* pLine, bool bConvert );
    static bool LineToSvxLine(const css::table::BorderLine& rLine, editeng::SvxBorderLine& rSvxLine, bool bConvert);
    static bool LineToSvxLine(const css::table::BorderLine2& rLine, editeng::SvxBorderLine& rSvxLine, bool bConvert);

    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

inline void SvxBoxItem::SetAllDistances(sal_uInt16 const nNew)
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

class EDITENG_DLLPUBLIC SvxBoxInfoItem final : public SfxPoolItem
{
    std::unique_ptr<editeng::SvxBorderLine> pHori;   //inner horizontal Line
    std::unique_ptr<editeng::SvxBorderLine> pVert;   //inner vertical Line

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
    virtual ~SvxBoxInfoItem() override;

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            GetPresentation( SfxItemPresentation ePres,
                                    MapUnit eCoreMetric,
                                    MapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper& ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual SvxBoxInfoItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual void            ScaleMetrics( tools::Long nMult, tools::Long nDiv ) override;
    virtual bool            HasMetrics() const override;

    const editeng::SvxBorderLine*   GetHori() const { return pHori.get(); }
    const editeng::SvxBorderLine*   GetVert() const { return pVert.get(); }

    //The Pointers are being copied!
    void                    SetLine( const editeng::SvxBorderLine* pNew, SvxBoxInfoItemLine nLine );

    bool                    IsTable() const             { return mbEnableHor && mbEnableVer; }
    void                    SetTable( bool bNew )       { mbEnableHor = mbEnableVer = bNew; }

    bool             IsHorEnabled() const { return mbEnableHor; }
    void             EnableHor( bool bEnable ) { mbEnableHor = bEnable; }
    bool             IsVerEnabled() const { return mbEnableVer; }
    void             EnableVer( bool bEnable ) { mbEnableVer = bEnable; }

    bool                    IsDist() const              { return bDist; }
    void                    SetDist( bool bNew )        { bDist = bNew; }
    bool                    IsMinDist() const           { return bMinDist; }
    void                    SetMinDist( bool bNew )     { bMinDist = bNew; }
    sal_uInt16              GetDefDist() const            { return nDefDist; }
    void                    SetDefDist( sal_uInt16 nNew ) { nDefDist = nNew; }

    bool                    IsValid( SvxBoxInfoItemValidFlags nValid ) const
                                { return bool( nValidFlags & nValid ); }
    void                    SetValid( SvxBoxInfoItemValidFlags nValid, bool bValid = true )
    {
        if (bValid)
            nValidFlags |= nValid;
        else
            nValidFlags &= ~nValid;
    }
    void                    ResetFlags();

    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

namespace editeng
{

void EDITENG_DLLPUBLIC BorderDistanceFromWord(bool bFromEdge, sal_Int32& nMargin,
    sal_Int32& nBorderDistance, sal_Int32 nBorderWidth);

struct EDITENG_DLLPUBLIC WordPageMargins final
{
    sal_uInt16 nLeft = 0;
    sal_uInt16 nRight = 0;
    sal_uInt16 nTop = 0;
    sal_uInt16 nBottom = 0;
};

struct EDITENG_DLLPUBLIC WordBorderDistances final
{
    bool bFromEdge = false;
    sal_uInt16 nLeft = 0;
    sal_uInt16 nRight = 0;
    sal_uInt16 nTop = 0;
    sal_uInt16 nBottom = 0;
};

// Heuristics to decide if we need to use "from edge" offset of borders. All sizes in twips
void EDITENG_DLLPUBLIC BorderDistancesToWord(const SvxBoxItem& rBox, const WordPageMargins& rMargins,
    WordBorderDistances& rDistances);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
