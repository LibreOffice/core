/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SORT_HXX
#define _SORT_HXX

#include <svl/svarray.hxx>
#include <ndindex.hxx>


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


namespace com { namespace sun { namespace star { namespace lang {
    struct Locale;
}}}}

/*--------------------------------------------------------------------
    Beschreibung: Liste aller sortierten Elemente
 --------------------------------------------------------------------*/
typedef const _FndBox*      _FndBoxPtr;
typedef SwSortElement*      SwSortElementPtr;
typedef const SwTableBox*   SwMovedBoxPtr;

SV_DECL_PTRARR_SORT(SwSortElements, SwSortElementPtr, 0, 1 )
SV_DECL_PTRARR(SwMovedBoxes,        SwMovedBoxPtr, 10, 10 )

/*--------------------------------------------------------------------
    Beschreibung: Funktionen zum Moven von Boxen
 --------------------------------------------------------------------*/

void MoveCol(SwDoc* pDoc, const FlatFndBox& rBox,
             sal_uInt16 nS, sal_uInt16 nT, SwMovedBoxes& rMovedList, SwUndoSort* pUD=0);
void MoveRow(SwDoc* pDoc, const FlatFndBox& rBox,
             sal_uInt16 nS, sal_uInt16 nT, SwMovedBoxes& rMovedList, SwUndoSort* pUD=0);
void MoveCell(SwDoc* pDoc, const SwTableBox* pSource,
              const SwTableBox* pTar, sal_Bool bMovedBefore, SwUndoSort* pUD=0);

/*-------------------------------------------------------------------
    Beschreibung: Elemente zum Sortieren von Text und Tabellen-Inhalt
 --------------------------------------------------------------------*/

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

    sal_Bool operator==(const SwSortElement& );
    sal_Bool operator<(const SwSortElement& );

    double StrToDouble(const String& rStr) const;
};

/*--------------------------------------------------------------------
    Beschreibung: Sortieren Text
 --------------------------------------------------------------------*/

struct SwSortTxtElement : public SwSortElement
{
    // fuer Text
    sal_uLong           nOrg;
    SwNodeIndex     aPos;

    SwSortTxtElement( const SwNodeIndex& rPos );
    virtual ~SwSortTxtElement();

    virtual String GetKey( sal_uInt16 nKey ) const;
};

/*--------------------------------------------------------------------
    Beschreibung: Sortieren Tabelle
 --------------------------------------------------------------------*/

struct SwSortBoxElement : public SwSortElement
{
    sal_uInt16                      nRow;

    SwSortBoxElement( sal_uInt16 nRC );
    virtual ~SwSortBoxElement();

    virtual String GetKey( sal_uInt16 nKey ) const;
    virtual double GetValue( sal_uInt16 nKey ) const;
};


/*--------------------------------------------------------------------
    Beschreibung: SymFndBoxes stellt ein zweidimensionales
                  Array von FndBoxes dar
 --------------------------------------------------------------------*/

class FlatFndBox
{
public:
    FlatFndBox(SwDoc* pDocPtr, const _FndBox& rBox);
    ~FlatFndBox();

    sal_Bool                IsSymmetric() const { return bSym;  }
    sal_uInt16              GetRows()     const { return nRows; }
    sal_uInt16              GetCols()     const { return nCols; }

    const _FndBox*      GetBox(sal_uInt16 nCol, sal_uInt16 nRow) const;

    inline sal_Bool         HasItemSets() const;
    const SfxItemSet*   GetItemSet(sal_uInt16 nCol, sal_uInt16 nRow) const;

private:

    sal_Bool                CheckLineSymmetry(const _FndBox& rBox);
    sal_Bool                CheckBoxSymmetry(const _FndLine& rLn);
    sal_uInt16              GetColCount(const _FndBox& rBox);
    sal_uInt16              GetRowCount(const _FndBox& rBox);
    void                FillFlat(const _FndBox&, sal_Bool bLastBox=sal_False);

    SwDoc*              pDoc;
    const _FndBox&      rBoxRef;
    _FndBoxPtr*         pArr;
    SfxItemSet**        ppItemSets;

    sal_uInt16              nRows;
    sal_uInt16              nCols;

    sal_uInt16              nRow;
    sal_uInt16              nCol;

    sal_Bool                bSym;
};


inline sal_Bool FlatFndBox::HasItemSets() const { return 0 != ppItemSets; }

#endif // _NDSORT_HXX
