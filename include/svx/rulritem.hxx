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

class SVX_DLLPUBLIC SvxLongLRSpaceItem : public SfxPoolItem
{
    long    mlLeft;         // nLeft or the negativ first-line indentation
    long    mlRight;        // the unproblematic right edge

  protected:

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;

private:
    const SvxLongLRSpaceItem& operator=(const SvxLongLRSpaceItem &) = delete;

public:
    static SfxPoolItem* CreateDefault();
    SvxLongLRSpaceItem(long lLeft, long lRight, sal_uInt16 nId);
    SvxLongLRSpaceItem(const SvxLongLRSpaceItem &);
    SvxLongLRSpaceItem();

    long GetLeft() const { return mlLeft;}
    long GetRight() const { return mlRight;}
    void SetLeft(long lArgLeft);
    void SetRight(long lArgRight);
};

class SVX_DLLPUBLIC SvxLongULSpaceItem : public SfxPoolItem
{
    long    mlLeft;         // nLeft or the negative first-line indentation
    long    mlRight;        // the unproblematic right edge

  protected:

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;

private:
    const SvxLongULSpaceItem& operator=(const SvxLongULSpaceItem &) = delete;

public:
    static SfxPoolItem* CreateDefault();
    SvxLongULSpaceItem(long lUpper, long lLower, sal_uInt16 nId);
    SvxLongULSpaceItem(const SvxLongULSpaceItem &);
    SvxLongULSpaceItem();

    long GetUpper() const { return mlLeft;}
    long GetLower() const { return mlRight;}
    void SetUpper(long lArgLeft);
    void SetLower(long lArgRight);
};

class SVX_DLLPUBLIC SvxPagePosSizeItem : public SfxPoolItem
{
    Point aPos;
    long lWidth;
    long lHeight;
protected:
    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;

private:
    const SvxPagePosSizeItem& operator=(const SvxPagePosSizeItem &) = delete;
public:
    static SfxPoolItem* CreateDefault();
    SvxPagePosSizeItem(const Point &rPos, long lWidth, long lHeight);
    SvxPagePosSizeItem(const SvxPagePosSizeItem &);
    SvxPagePosSizeItem();

    const Point &GetPos() const { return aPos; }
    long    GetWidth() const { return lWidth; }
    long    GetHeight() const { return lHeight; }
};

struct SVX_DLLPUBLIC SvxColumnDescription
{
    long     nStart;    /* Start of the column */
    long     nEnd;      /* End of the column */
    bool     bVisible;  /* Visibility */

    long nEndMin; //min. possible position of end
    long nEndMax; //max. possible position of end

    SvxColumnDescription();

    SvxColumnDescription(const SvxColumnDescription &rCopy);

    SvxColumnDescription(long start, long end, bool bVis = true);

    SvxColumnDescription(long start, long end,
                         long endMin, long endMax, bool bVis = true);

    bool operator==(const SvxColumnDescription &rCmp) const;
    bool operator!=(const SvxColumnDescription &rCmp) const;
    long GetWidth() const;
};

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, SvxColumnDescription const& dsc)
{
    return stream << "{ nStart " << dsc.nStart << " nEnd " << dsc.nEnd
        << " bVisible " << dsc.bVisible  << " nEndMin " << dsc.nEndMin
        << " nEndMax " << dsc.nEndMax << " }";
}


class SVX_DLLPUBLIC SvxColumnItem : public SfxPoolItem
{
    typedef std::vector<SvxColumnDescription> SvxColumnDescriptionVector;
    SvxColumnDescriptionVector aColumns; // Column array

    long nLeft;             // Left edge for the table
    long nRight;            // Right edge for the table; for columns always
                            // equal to the surrounding frame
    sal_uInt16 nActColumn;  // the current column
    bool       bTable;      // table?
    bool       bOrtho;      // evenly spread columns

protected:
    virtual bool operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 OUString &rText,
                                                 const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool         QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool         PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

public:
    static SfxPoolItem* CreateDefault();
    // right edge of the surrounding frame
    // nLeft, nRight each the distance to the surrounding frame
    SvxColumnItem(sal_uInt16 nAct = 0);
    SvxColumnItem(sal_uInt16 nActCol, sal_uInt16 nLeft, sal_uInt16 nRight = 0); // Table with borders
    SvxColumnItem(const SvxColumnItem& aItem);
    virtual ~SvxColumnItem();

    const SvxColumnItem &operator=(const SvxColumnItem &);

    SvxColumnDescription&       operator[](sal_uInt16 index);
    const SvxColumnDescription& operator[](sal_uInt16 index) const;
    SvxColumnDescription&       At(sal_uInt16 index);
    SvxColumnDescription&       GetActiveColumnDescription();

    sal_uInt16  Count() const;
    void        Insert(const SvxColumnDescription& rDesc, sal_uInt16 nPos);
    void        Append(const SvxColumnDescription& rDesc);
    void        SetLeft(long aLeft);
    void        SetRight(long aRight);
    sal_uInt16  GetActColumn() const { return nActColumn;}
    bool        IsFirstAct() const;
    bool        IsLastAct() const;
    long        GetLeft() { return nLeft;}
    long        GetRight() { return nRight;}
    bool        IsTable() const { return bTable;}
    bool        CalcOrtho() const;
    void        SetOrtho(bool bVal);
    bool        IsConsistent() const;
};

// class SvxObjectItem ---------------------------------------------------

class SVX_DLLPUBLIC SvxObjectItem : public SfxPoolItem
{
private:
    long nStartX;   /* Start in x direction */
    long nEndX;     /* End in x direction */
    long nStartY;   /* Start in y direction */
    long nEndY;     /* Ende in y direction */
    bool bLimits; /* boundary limit control by the application */

protected:
    virtual bool             operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

private:
    const SvxObjectItem &operator=(const SvxObjectItem &) = delete;

public:
    static SfxPoolItem* CreateDefault();
    SvxObjectItem(long nStartX, long nEndX,
                  long nStartY, long nEndY,
                  bool bLimits = false);
    SvxObjectItem(const SvxObjectItem& rCopy);

    bool HasLimits() const { return bLimits;}

    long GetStartX() const { return nStartX;}
    long GetEndX() const { return nEndX;}
    long GetStartY() const { return nStartY;}
    long GetEndY() const { return nEndY;}

    void SetStartX(long lValue);
    void SetEndX(long lValue);
    void SetStartY(long lValue);
    void SetEndY(long lValue);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
