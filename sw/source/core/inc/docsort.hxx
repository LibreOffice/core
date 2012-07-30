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

#ifndef _SORT_HXX
#define _SORT_HXX

#include <ndindex.hxx>
#include <vector>

class SwDoc;
class SwTableBox;
class SwUndoSort;
class FlatFndBox;
struct SwSortOptions;
struct SwSortElement;
class _FndBox;
class _FndLine;
class CollatorWrapper;
class LocaleDataWrapper;

namespace com {
    namespace sun {
        namespace star {
            namespace lang {
                struct Locale;
            }
        }
    }
}

class SwMovedBoxes : public std::vector<const SwTableBox*>
{
public:
    sal_uInt16 GetPos(const SwTableBox* pTableBox) const;
};

// Functions for moving boxes
void MoveCol(SwDoc* pDoc, const FlatFndBox& rBox,
             sal_uInt16 nS, sal_uInt16 nT, SwMovedBoxes& rMovedList, SwUndoSort* pUD=0);
void MoveRow(SwDoc* pDoc, const FlatFndBox& rBox,
             sal_uInt16 nS, sal_uInt16 nT, SwMovedBoxes& rMovedList, SwUndoSort* pUD=0);
void MoveCell(SwDoc* pDoc, const SwTableBox* pSource,
              const SwTableBox* pTar, sal_Bool bMovedBefore, SwUndoSort* pUD=0);

// Elements for sorting text and table content
struct SwSortElement
{
    static SwSortOptions*       pOptions;
    static SwDoc*               pDoc;
    static const FlatFndBox*    pBox;
    static CollatorWrapper*     pSortCollator;
    static ::com::sun::star::lang::Locale* pLocale;
    static String*              pLastAlgorithm;
    static LocaleDataWrapper*   pLclData;

    static void Init( SwDoc*, const SwSortOptions& rOpt, FlatFndBox* = 0 );
    static void Finit();

    virtual ~SwSortElement();

    virtual String GetKey(sal_uInt16 nKey ) const = 0;
    virtual double GetValue(sal_uInt16 nKey ) const;

    sal_Bool operator==(const SwSortElement& ) const;
    sal_Bool operator<(const SwSortElement& ) const;

    double StrToDouble(const String& rStr) const;
private:
    int keycompare(const SwSortElement& rCmp, sal_uInt16 nKey) const;
};

// sort text
struct SwSortTxtElement : public SwSortElement
{
    sal_uLong   nOrg;
    SwNodeIndex aPos;

    SwSortTxtElement( const SwNodeIndex& rPos );
    virtual ~SwSortTxtElement();

    virtual String GetKey( sal_uInt16 nKey ) const;
};

// sort table
struct SwSortBoxElement : public SwSortElement
{
    sal_uInt16 nRow;

    SwSortBoxElement( sal_uInt16 nRC );
    virtual ~SwSortBoxElement();

    virtual String GetKey( sal_uInt16 nKey ) const;
    virtual double GetValue( sal_uInt16 nKey ) const;
};

// two-dimensional array of FndBoxes
class FlatFndBox
{
public:
    FlatFndBox(SwDoc* pDocPtr, const _FndBox& rBox);
    ~FlatFndBox();

    sal_Bool            IsSymmetric() const { return bSym;  }
    sal_uInt16          GetRows()     const { return nRows; }
    sal_uInt16          GetCols()     const { return nCols; }

    const _FndBox*      GetBox(sal_uInt16 nCol, sal_uInt16 nRow) const;

    inline sal_Bool     HasItemSets() const;
    const SfxItemSet*   GetItemSet(sal_uInt16 nCol, sal_uInt16 nRow) const;

private:
    sal_Bool            CheckLineSymmetry(const _FndBox& rBox);
    sal_Bool            CheckBoxSymmetry(const _FndLine& rLn);
    sal_uInt16          GetColCount(const _FndBox& rBox);
    sal_uInt16          GetRowCount(const _FndBox& rBox);
    void                FillFlat(const _FndBox&, sal_Bool bLastBox=sal_False);

    SwDoc*              pDoc;
    const _FndBox&      rBoxRef;
    const _FndBox**     pArr;
    SfxItemSet**        ppItemSets;

    sal_uInt16          nRows;
    sal_uInt16          nCols;
    sal_uInt16          nRow;
    sal_uInt16          nCol;

    sal_Bool            bSym;
};

inline sal_Bool FlatFndBox::HasItemSets() const { return 0 != ppItemSets; }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
