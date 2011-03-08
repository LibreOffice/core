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
#ifndef _SVX_RULRITEM_HXX
#define _SVX_RULRITEM_HXX

// include ---------------------------------------------------------------


#include <tools/gen.hxx>
#include <svl/poolitem.hxx>
#include "svx/svxdllapi.h"

// class SvxLongLRSpaceItem ----------------------------------------------

class SVX_DLLPUBLIC SvxLongLRSpaceItem : public SfxPoolItem
{
    long    lLeft;         // nLeft or the negativ first-line indentation
    long    lRight;        // the unproblematic right edge

  protected:

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

private:
    SVX_DLLPRIVATE const SvxLongLRSpaceItem& operator=(const SvxLongLRSpaceItem &); // n.i.

public:
    TYPEINFO();
    SvxLongLRSpaceItem(long lLeft, long lRight, USHORT nId);
    SvxLongLRSpaceItem(const SvxLongLRSpaceItem &);
    SvxLongLRSpaceItem();

    long    GetLeft() const { return lLeft; }
    long    GetRight() const { return lRight; }
    void    SetLeft(long lArgLeft) {lLeft=lArgLeft;}
    void    SetRight(long lArgRight) {lRight=lArgRight;}
};

// class SvxLongULSpaceItem ----------------------------------------------

class SVX_DLLPUBLIC SvxLongULSpaceItem : public SfxPoolItem
{
    long    lLeft;         // nLeft or the negative first-line indentation
    long    lRight;        // the unproblematic right edge

  protected:

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

private:
    SVX_DLLPRIVATE const SvxLongULSpaceItem& operator=(const SvxLongULSpaceItem &); // n.i.

public:
    TYPEINFO();
    SvxLongULSpaceItem(long lUpper, long lLower, USHORT nId);
    SvxLongULSpaceItem(const SvxLongULSpaceItem &);
    SvxLongULSpaceItem();

    long    GetUpper() const { return lLeft; }
    long    GetLower() const { return lRight; }
    void    SetUpper(long lArgLeft) {lLeft=lArgLeft;}
    void    SetLower(long lArgRight) {lRight=lArgRight;}
};

// class SvxPagePosSizeItem ----------------------------------------------

class SVX_DLLPUBLIC SvxPagePosSizeItem : public SfxPoolItem
{
    Point aPos;
    long lWidth;
    long lHeight;
protected:
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

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

// struct SvxColumnDescription -------------------------------------------

struct SvxColumnDescription
{
    long nStart;                    /* Start of the column */
    long nEnd;                      /* End of the column */
    BOOL   bVisible;                   /* Visibility */

    long nEndMin;         //min. possible position of end
    long nEndMax;         //max. possible position of end

    SvxColumnDescription():
        nStart(0), nEnd(0), bVisible(TRUE), nEndMin(0), nEndMax(0) {}

    SvxColumnDescription(const SvxColumnDescription &rCopy) :
        nStart(rCopy.nStart), nEnd(rCopy.nEnd),
        bVisible(rCopy.bVisible),
        nEndMin(rCopy.nEndMin), nEndMax(rCopy.nEndMax)
         {}

    SvxColumnDescription(long start, long end, BOOL bVis = TRUE):
        nStart(start), nEnd(end),
        bVisible(bVis),
        nEndMin(0), nEndMax(0) {}

    SvxColumnDescription(long start, long end,
                        long endMin, long endMax, BOOL bVis = TRUE):
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

// class SvxColumnItem ---------------------------------------------------

typedef SvPtrarr SvxColumns;

class SVX_DLLPUBLIC SvxColumnItem : public SfxPoolItem
{
    SvxColumns aColumns;// Column array
    long    nLeft,      // Left edge for the table
           nRight;      // Right edge for the table; for columns always
                                // equal to the surrounding frame
    USHORT nActColumn;  // the current column
    BOOL    bTable;     // table?
    BOOL    bOrtho;     // evenly spread columns

    void DeleteAndDestroyColumns();

protected:
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
public:
    TYPEINFO();
    // right edge of the surrounding frame
    // nLeft, nRight each the distance to the surrounding frame
    SvxColumnItem(USHORT nAct = 0); // columns
    SvxColumnItem(USHORT nActCol,
                  USHORT nLeft, USHORT nRight = 0); // table with borders
    SvxColumnItem(const SvxColumnItem &);
    ~SvxColumnItem();

    const SvxColumnItem &operator=(const SvxColumnItem &);

    USHORT Count() const { return aColumns.Count(); }
    SvxColumnDescription &operator[](USHORT i)
        { return *(SvxColumnDescription*)aColumns[i]; }
    const SvxColumnDescription &operator[](USHORT i) const
        { return *(SvxColumnDescription*)aColumns[i]; }
    void Insert(const SvxColumnDescription &rDesc, USHORT nPos) {
        SvxColumnDescription* pDesc = new SvxColumnDescription(rDesc);
        aColumns.Insert(pDesc, nPos);
    }
    void   Append(const SvxColumnDescription &rDesc) { Insert(rDesc, Count()); }
    void   SetLeft(long left) { nLeft = left; }
    void   SetRight(long right) { nRight = right; }
    void   SetActColumn(USHORT nCol) { nActColumn = nCol; }

    USHORT GetActColumn() const { return nActColumn; }
    BOOL   IsFirstAct() const { return nActColumn == 0; }
    BOOL   IsLastAct() const { return nActColumn == Count()-1; }
    long GetLeft() { return nLeft; }
    long GetRight() { return nRight; }

    BOOL   IsTable() const { return bTable; }

    BOOL   CalcOrtho() const;
    void   SetOrtho(BOOL bVal) { bOrtho = bVal; }
    BOOL   IsOrtho () const { return FALSE ; }

    BOOL IsConsistent() const  { return nActColumn < aColumns.Count(); }
    long   GetVisibleRight() const;// right visible edge of the current column
};

// class SvxObjectItem ---------------------------------------------------

class SVX_DLLPUBLIC SvxObjectItem : public SfxPoolItem
{
private:
    long   nStartX;                    /* Start in x direction */
    long   nEndX;                      /* End in x direction */
    long   nStartY;                    /* Start in y direction */
    long   nEndY;                      /* Ende in y direction */
    BOOL   bLimits;                    /* boundary limit control by the application */
protected:
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
private:
    SVX_DLLPRIVATE const SvxObjectItem &operator=(const SvxObjectItem &); // n.i.
public:
    TYPEINFO();
    SvxObjectItem(long nStartX, long nEndX,
                  long nStartY, long nEndY,
                  BOOL bLimits = FALSE);
    SvxObjectItem(const SvxObjectItem &);

    BOOL   HasLimits() const { return bLimits; }

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
