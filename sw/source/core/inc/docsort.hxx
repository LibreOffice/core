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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCSORT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCSORT_HXX

#include <ndindex.hxx>

#include <memory>
#include <vector>

class SwDoc;
class SwTableBox;
class SwUndoSort;
class FlatFndBox;
struct SwSortOptions;
class FndBox_;
class FndLine_;
class CollatorWrapper;
class LocaleDataWrapper;
namespace com::sun::star::lang { struct Locale; }

class SwMovedBoxes
{
private:
    std::vector<const SwTableBox*> mBoxes;

public:
    void push_back(const SwTableBox* &rpTableBox) {mBoxes.push_back(rpTableBox);}

    sal_uInt16 GetPos(const SwTableBox* pTableBox) const;
};

// Functions for moving boxes
void MoveCol(SwDoc* pDoc, const FlatFndBox& rBox,
             sal_uInt16 nS, sal_uInt16 nT, SwMovedBoxes& rMovedList, SwUndoSort* pUD);
void MoveRow(SwDoc* pDoc, const FlatFndBox& rBox,
             sal_uInt16 nS, sal_uInt16 nT, SwMovedBoxes& rMovedList, SwUndoSort* pUD);
void MoveCell(SwDoc* pDoc, const SwTableBox* pSource,
              const SwTableBox* pTar, bool bMovedBefore, SwUndoSort* pUD=nullptr);

// Elements for sorting text and table content
struct SwSortElement
{
    static SwSortOptions*       pOptions;
    static SwDoc*               pDoc;
    static const FlatFndBox*    pBox;
    static CollatorWrapper*     pSortCollator;
    static css::lang::Locale*   pLocale;
    static OUString*            pLastAlgorithm;
    static LocaleDataWrapper*   pLclData;

    static void Init( SwDoc*, const SwSortOptions& rOpt, FlatFndBox const * = nullptr );
    static void Finit();

    SwSortElement() = default;
    SwSortElement(SwSortElement const &) = default;
    SwSortElement(SwSortElement &&) = default;
    SwSortElement & operator =(SwSortElement const &) = default;
    SwSortElement & operator =(SwSortElement &&) = default;

    virtual ~SwSortElement();

    virtual OUString GetKey(sal_uInt16 nKey ) const = 0;
    virtual double GetValue(sal_uInt16 nKey ) const;

    bool operator<(const SwSortElement& ) const;

    static double StrToDouble(const OUString& rStr);
private:
    int keycompare(const SwSortElement& rCmp, sal_uInt16 nKey) const;
};

// sort text
struct SwSortTextElement final : public SwSortElement
{
    sal_uLong   nOrg;
    SwNodeIndex aPos;

    SwSortTextElement( const SwNodeIndex& rPos );

    virtual OUString GetKey( sal_uInt16 nKey ) const override;
};

// sort table
struct SwSortBoxElement final : public SwSortElement
{
    sal_uInt16 nRow;

    SwSortBoxElement( sal_uInt16 nRC );

    virtual OUString GetKey( sal_uInt16 nKey ) const override;
    virtual double GetValue( sal_uInt16 nKey ) const override;
};

// two-dimensional array of FndBoxes
class FlatFndBox
{
public:
    FlatFndBox(SwDoc* pDocPtr, const FndBox_& rBox);
    ~FlatFndBox();

    bool            IsSymmetric() const { return m_bSym;  }
    sal_uInt16          GetRows()     const { return m_nRows; }
    sal_uInt16          GetCols()     const { return m_nCols; }

    const FndBox_*      GetBox(sal_uInt16 nCol, sal_uInt16 nRow) const;

    inline bool     HasItemSets() const;
    const SfxItemSet*   GetItemSet(sal_uInt16 nCol, sal_uInt16 nRow) const;

private:
    bool            CheckLineSymmetry(const FndBox_& rBox);
    bool            CheckBoxSymmetry(const FndLine_& rLn);
    sal_uInt16          GetColCount(const FndBox_& rBox);
    sal_uInt16          GetRowCount(const FndBox_& rBox);
    void                FillFlat(const FndBox_&, bool bLastBox=false);

    SwDoc*              m_pDoc;
    std::unique_ptr<FndBox_ const *[]> m_pArr;
    std::vector<std::unique_ptr<SfxItemSet>> m_ppItemSets;

    sal_uInt16          m_nRows;
    sal_uInt16          m_nCols;
    sal_uInt16          m_nRow;
    sal_uInt16          m_nCol;

    bool            m_bSym;
};

inline bool FlatFndBox::HasItemSets() const { return !m_ppItemSets.empty(); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
