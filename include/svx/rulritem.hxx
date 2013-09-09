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
#ifndef _SVX_RULRITEM_HXX
#define _SVX_RULRITEM_HXX

#include <tools/gen.hxx>
#include <svl/poolitem.hxx>
#include "svx/svxdllapi.h"
#include <vector>

class SVX_DLLPUBLIC SvxLongLRSpaceItem : public SfxPoolItem
{
    long    lLeft;         // nLeft or the negativ first-line indentation
    long    lRight;        // the unproblematic right edge

  protected:

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual OUString         GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

private:
    SVX_DLLPRIVATE const SvxLongLRSpaceItem& operator=(const SvxLongLRSpaceItem &); // n.i.

public:
    TYPEINFO();
    SvxLongLRSpaceItem(long lLeft, long lRight, sal_uInt16 nId);
    SvxLongLRSpaceItem(const SvxLongLRSpaceItem &);
    SvxLongLRSpaceItem();

    long    GetLeft() const { return lLeft; }
    long    GetRight() const { return lRight; }
    void    SetLeft(long lArgLeft) {lLeft=lArgLeft;}
    void    SetRight(long lArgRight) {lRight=lArgRight;}
};

class SVX_DLLPUBLIC SvxLongULSpaceItem : public SfxPoolItem
{
    long    lLeft;         // nLeft or the negative first-line indentation
    long    lRight;        // the unproblematic right edge

  protected:

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual OUString         GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

private:
    SVX_DLLPRIVATE const SvxLongULSpaceItem& operator=(const SvxLongULSpaceItem &); // n.i.

public:
    TYPEINFO();
    SvxLongULSpaceItem(long lUpper, long lLower, sal_uInt16 nId);
    SvxLongULSpaceItem(const SvxLongULSpaceItem &);
    SvxLongULSpaceItem();

    long    GetUpper() const { return lLeft; }
    long    GetLower() const { return lRight; }
    void    SetUpper(long lArgLeft) {lLeft=lArgLeft;}
    void    SetLower(long lArgRight) {lRight=lArgRight;}
};

class SVX_DLLPUBLIC SvxPagePosSizeItem : public SfxPoolItem
{
    Point aPos;
    long lWidth;
    long lHeight;
protected:
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual OUString         GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

private:
    SVX_DLLPRIVATE const SvxPagePosSizeItem& operator=(const SvxPagePosSizeItem &); // n.i.
public:
    TYPEINFO();
    SvxPagePosSizeItem(const Point &rPos, long lWidth, long lHeight);
    SvxPagePosSizeItem(const SvxPagePosSizeItem &);
    SvxPagePosSizeItem();

    const Point &GetPos() const { return aPos; }
    long    GetWidth() const { return lWidth; }
    long    GetHeight() const { return lHeight; }
};

struct SvxColumnDescription
{
    long nStart;                    /* Start of the column */
    long nEnd;                      /* End of the column */
    sal_Bool bVisible;              /* Visibility */

    long nEndMin;         //min. possible position of end
    long nEndMax;         //max. possible position of end

    SvxColumnDescription():
        nStart(0), nEnd(0), bVisible(sal_True), nEndMin(0), nEndMax(0) {}

    SvxColumnDescription(const SvxColumnDescription &rCopy) :
        nStart(rCopy.nStart), nEnd(rCopy.nEnd),
        bVisible(rCopy.bVisible),
        nEndMin(rCopy.nEndMin), nEndMax(rCopy.nEndMax)
         {}

    SvxColumnDescription(long start, long end, sal_Bool bVis = sal_True):
        nStart(start), nEnd(end),
        bVisible(bVis),
        nEndMin(0), nEndMax(0) {}

    SvxColumnDescription(long start, long end,
                        long endMin, long endMax, sal_Bool bVis = sal_True):
        nStart(start), nEnd(end),
        bVisible(bVis),
        nEndMin(endMin), nEndMax(endMax)
         {}

    int operator==(const SvxColumnDescription &rCmp) const {
        return nStart == rCmp.nStart &&
            bVisible == rCmp.bVisible &&
            nEnd == rCmp.nEnd &&
            nEndMin == rCmp.nEndMin &&
                nEndMax == rCmp.nEndMax;
    }
    int operator!=(const SvxColumnDescription &rCmp) const {
        return !operator==(rCmp);
    }
    long GetWidth() const { return nEnd - nStart; }
};

class SVX_DLLPUBLIC SvxColumnItem : public SfxPoolItem
{
    typedef std::vector<SvxColumnDescription> SvxColumnDescriptionVector;
    SvxColumnDescriptionVector aColumns; // Column array

    long nLeft;             // Left edge for the table
    long nRight;            // Right edge for the table; for columns always
                            // equal to the surrounding frame
    sal_uInt16 nActColumn;  // the current column
    sal_uInt8  bTable;      // table?
    sal_uInt8  bOrtho;      // evenly spread columns

protected:
    virtual int operator==( const SfxPoolItem& ) const;

    virtual OUString GetValueText() const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 OUString &rText,
                                                 const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool         QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool         PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

public:
    TYPEINFO();
    // right edge of the surrounding frame
    // nLeft, nRight each the distance to the surrounding frame
    SvxColumnItem(sal_uInt16 nAct = 0);
    SvxColumnItem(sal_uInt16 nActCol, sal_uInt16 nLeft, sal_uInt16 nRight = 0); // Table with borders
    SvxColumnItem(const SvxColumnItem& aItem);
    ~SvxColumnItem();

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
    void        SetActColumn(sal_uInt16 nCol);
    sal_uInt16  GetActColumn() const;
    sal_uInt16  GetColumnDescription() const;
    sal_Bool    IsFirstAct() const;
    sal_Bool    IsLastAct() const;
    long        GetLeft();
    long        GetRight();
    sal_Bool    IsTable() const;
    sal_Bool    CalcOrtho() const;
    void        SetOrtho(sal_Bool bVal);
    sal_Bool    IsOrtho () const;
    sal_Bool    IsConsistent() const;
};

// class SvxObjectItem ---------------------------------------------------

class SVX_DLLPUBLIC SvxObjectItem : public SfxPoolItem
{
private:
    long   nStartX;                    /* Start in x direction */
    long   nEndX;                      /* End in x direction */
    long   nStartY;                    /* Start in y direction */
    long   nEndY;                      /* Ende in y direction */
    sal_Bool bLimits;                  /* boundary limit control by the application */
protected:
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual OUString         GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
private:
    SVX_DLLPRIVATE const SvxObjectItem &operator=(const SvxObjectItem &); // n.i.
public:
    TYPEINFO();
    SvxObjectItem(long nStartX, long nEndX,
                  long nStartY, long nEndY,
                  sal_Bool bLimits = sal_False);
    SvxObjectItem(const SvxObjectItem &);

    sal_Bool   HasLimits() const { return bLimits; }

    long   GetStartX() const { return nStartX; }
    long   GetEndX() const { return nEndX; }
    long   GetStartY() const { return nStartY; }
    long   GetEndY() const { return nEndY; }

    void   SetStartX(long l) { nStartX = l; }
    void   SetEndX(long l) { nEndX = l; }
    void   SetStartY(long l) { nStartY = l; }
    void   SetEndY(long l) { nEndY = l; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
