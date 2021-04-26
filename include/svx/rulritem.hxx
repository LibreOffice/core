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
#ifndef INCLUDED_SVX_RULRITEM_HXX
#define INCLUDED_SVX_RULRITEM_HXX

#include <tools/gen.hxx>
#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>
#include <vector>

class SVX_DLLPUBLIC SvxLongLRSpaceItem final : public SfxPoolItem
{
    tools::Long    mlLeft;         // nLeft or the negative first-line indentation
    tools::Long    mlRight;        // the unproblematic right edge

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SvxLongLRSpaceItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    const SvxLongLRSpaceItem& operator=(const SvxLongLRSpaceItem &) = delete;

public:
    static SfxPoolItem* CreateDefault();
    SvxLongLRSpaceItem(tools::Long lLeft, tools::Long lRight, sal_uInt16 nId);
    SvxLongLRSpaceItem();
    SvxLongLRSpaceItem(SvxLongLRSpaceItem const &) = default;

    tools::Long GetLeft() const { return mlLeft;}
    tools::Long GetRight() const { return mlRight;}
    void SetLeft(tools::Long lArgLeft);
    void SetRight(tools::Long lArgRight);
};

class SVX_DLLPUBLIC SvxLongULSpaceItem final : public SfxPoolItem
{
    tools::Long    mlLeft;         // nLeft or the negative first-line indentation
    tools::Long    mlRight;        // the unproblematic right edge

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SvxLongULSpaceItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    const SvxLongULSpaceItem& operator=(const SvxLongULSpaceItem &) = delete;

public:
    static SfxPoolItem* CreateDefault();
    SvxLongULSpaceItem(tools::Long lUpper, tools::Long lLower, sal_uInt16 nId);
    SvxLongULSpaceItem();
    SvxLongULSpaceItem(SvxLongULSpaceItem const &) = default;

    tools::Long GetUpper() const { return mlLeft;}
    tools::Long GetLower() const { return mlRight;}
    void SetUpper(tools::Long lArgLeft);
    void SetLower(tools::Long lArgRight);
};

class SVX_DLLPUBLIC SvxPagePosSizeItem final : public SfxPoolItem
{
    Point aPos;
    tools::Long lWidth;
    tools::Long lHeight;

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SvxPagePosSizeItem* Clone( SfxItemPool *pPool = nullptr ) const override;

private:
    const SvxPagePosSizeItem& operator=(const SvxPagePosSizeItem &) = delete;
public:
    static SfxPoolItem* CreateDefault();
    SvxPagePosSizeItem(const Point &rPos, tools::Long lWidth, tools::Long lHeight);
    SvxPagePosSizeItem();
    SvxPagePosSizeItem(SvxPagePosSizeItem const &) = default;

    const Point &GetPos() const { return aPos; }
    tools::Long    GetWidth() const { return lWidth; }
    tools::Long    GetHeight() const { return lHeight; }
};

struct SVX_DLLPUBLIC SvxColumnDescription
{
    tools::Long     nStart;    /* Start of the column */
    tools::Long     nEnd;      /* End of the column */
    bool     bVisible;  /* Visibility */

    tools::Long nEndMin; //min. possible position of end
    tools::Long nEndMax; //max. possible position of end

    SvxColumnDescription(tools::Long start, tools::Long end, bool bVis);

    SvxColumnDescription(tools::Long start, tools::Long end,
                         tools::Long endMin, tools::Long endMax, bool bVis);

    bool operator==(const SvxColumnDescription &rCmp) const;
    bool operator!=(const SvxColumnDescription &rCmp) const;
    tools::Long GetWidth() const;
};

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, SvxColumnDescription const& dsc)
{
    return stream << "{ nStart " << dsc.nStart << " nEnd " << dsc.nEnd
        << " bVisible " << dsc.bVisible  << " nEndMin " << dsc.nEndMin
        << " nEndMax " << dsc.nEndMax << " }";
}


class SVX_DLLPUBLIC SvxColumnItem final : public SfxPoolItem
{
    std::vector<SvxColumnDescription>  aColumns; // Column array

    tools::Long nLeft;             // Left edge for the table
    tools::Long nRight;            // Right edge for the table; for columns always
                            // equal to the surrounding frame
    sal_uInt16 nActColumn;  // the current column
    bool       bTable;      // table?
    bool       bOrtho;      // evenly spread columns

    virtual bool operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& ) const override;

    virtual SvxColumnItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool         QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool         PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

public:
    static SfxPoolItem* CreateDefault();
    // right edge of the surrounding frame
    // nLeft, nRight each the distance to the surrounding frame
    SvxColumnItem(sal_uInt16 nAct = 0);
    SvxColumnItem(sal_uInt16 nActCol, sal_uInt16 nLeft, sal_uInt16 nRight); // Table with borders

    SvxColumnDescription&       operator[](sal_uInt16 index);
    const SvxColumnDescription& operator[](sal_uInt16 index) const;
    SvxColumnDescription&       At(sal_uInt16 index);
    SvxColumnDescription&       GetActiveColumnDescription();

    sal_uInt16  Count() const;
    void        Append(const SvxColumnDescription& rDesc);
    void        SetLeft(tools::Long aLeft);
    void        SetRight(tools::Long aRight);
    sal_uInt16  GetActColumn() const { return nActColumn;}
    bool        IsFirstAct() const;
    bool        IsLastAct() const;
    tools::Long        GetLeft() const { return nLeft;}
    tools::Long        GetRight() const { return nRight;}
    bool        IsTable() const { return bTable;}
    bool        CalcOrtho() const;
    void        SetOrtho(bool bVal);
    bool        IsConsistent() const;
};

class SVX_DLLPUBLIC SvxObjectItem final : public SfxPoolItem
{
    tools::Long nStartX;   /* Start in x direction */
    tools::Long nEndX;     /* End in x direction */
    tools::Long nStartY;   /* Start in y direction */
    tools::Long nEndY;     /* End in y direction */
    bool bLimits;   /* boundary limit control by the application */

    virtual bool             operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SvxObjectItem*   Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const SvxObjectItem &operator=(const SvxObjectItem &) = delete;

public:
    static SfxPoolItem* CreateDefault();
    SvxObjectItem(tools::Long nStartX, tools::Long nEndX,
                  tools::Long nStartY, tools::Long nEndY);
    SvxObjectItem(SvxObjectItem const &) = default;

    tools::Long GetStartX() const { return nStartX;}
    tools::Long GetEndX() const { return nEndX;}
    tools::Long GetStartY() const { return nStartY;}
    tools::Long GetEndY() const { return nEndY;}

    void SetStartX(tools::Long lValue);
    void SetEndX(tools::Long lValue);
    void SetStartY(tools::Long lValue);
    void SetEndY(tools::Long lValue);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
