/*************************************************************************
 *
 *  $RCSfile: docsort.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SORT_HXX
#define _SORT_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

class SwDoc;
class SwTableBox;
class SwUndoSort;
class FlatFndBox;
struct SwSortOptions;
struct SwSortElement;
class _FndBox;
class _FndLine;
class International;

/*--------------------------------------------------------------------
    Beschreibung: Liste aller sortierten Elemente
 --------------------------------------------------------------------*/
typedef const _FndBox*      _FndBoxPtr;
typedef SwSortElement*      SwSortElementPtr;
typedef const SwTableBox*   SwMovedBoxPtr;

SV_DECL_PTRARR_SORT(SwSortElements, SwSortElementPtr, 0, 1 );
SV_DECL_PTRARR(SwMovedBoxes,        SwMovedBoxPtr, 10, 10 );

/*--------------------------------------------------------------------
    Beschreibung: Funktionen zum Moven von Boxen
 --------------------------------------------------------------------*/

void MoveCol(SwDoc* pDoc, const FlatFndBox& rBox,
             USHORT nS, USHORT nT, SwMovedBoxes& rMovedList, SwUndoSort* pUD=0);
void MoveRow(SwDoc* pDoc, const FlatFndBox& rBox,
             USHORT nS, USHORT nT, SwMovedBoxes& rMovedList, SwUndoSort* pUD=0);
void MoveCell(SwDoc* pDoc, const SwTableBox* pSource,
              const SwTableBox* pTar, BOOL bMovedBefore, SwUndoSort* pUD=0);

/*-------------------------------------------------------------------
    Beschreibung: Elemente zum Sortieren von Text und Tabellen-Inhalt
 --------------------------------------------------------------------*/

struct SwSortElement
{
    static  SwSortOptions*   pOptions;
    static  SwDoc*           pDoc;
    static const FlatFndBox* pBox;
    static  International*   pIntl;

    static void Init( SwDoc*, const SwSortOptions& rOpt, FlatFndBox* = 0 );
    static void Finit();

    virtual ~SwSortElement();

    virtual String GetKey(USHORT nKey ) const = 0;
    virtual double GetValue(USHORT nKey ) const;

    BOOL operator==(const SwSortElement& );
    BOOL operator<(const SwSortElement& );

    double StrToDouble(const String& rStr) const;
};

/*--------------------------------------------------------------------
    Beschreibung: Sortieren Text
 --------------------------------------------------------------------*/

struct SwSortTxtElement : public SwSortElement
{
    // fuer Text
    ULONG           nOrg;
    SwNodeIndex     aPos;

    SwSortTxtElement( const SwNodeIndex& rPos );
    virtual ~SwSortTxtElement();

    virtual String GetKey( USHORT nKey ) const;
};

/*--------------------------------------------------------------------
    Beschreibung: Sortieren Tabelle
 --------------------------------------------------------------------*/

struct SwSortBoxElement : public SwSortElement
{
    USHORT                      nRow;

    SwSortBoxElement( USHORT nRC );
    virtual ~SwSortBoxElement();

    virtual String GetKey( USHORT nKey ) const;
    virtual double GetValue( USHORT nKey ) const;
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

    BOOL                IsSymmetric() const { return bSym;  }
    USHORT              GetRows()     const { return nRows; }
    USHORT              GetCols()     const { return nCols; }

    const _FndBox*      GetBox(USHORT nCol, USHORT nRow) const;

    inline BOOL         HasItemSets() const;
    const SfxItemSet*   GetItemSet(USHORT nCol, USHORT nRow) const;

private:

    BOOL                CheckLineSymmetry(const _FndBox& rBox);
    BOOL                CheckBoxSymmetry(const _FndLine& rLn);
    USHORT              GetColCount(const _FndBox& rBox);
    USHORT              GetRowCount(const _FndBox& rBox);
    void                FillFlat(const _FndBox&, BOOL bLastBox=FALSE);

    SwDoc*              pDoc;
    const _FndBox&      rBoxRef;
    _FndBoxPtr*         pArr;
    SfxItemSet**        ppItemSets;

    USHORT              nRows;
    USHORT              nCols;

    USHORT              nRow;
    USHORT              nCol;

    BOOL                bSym;
};


inline BOOL FlatFndBox::HasItemSets() const { return 0 != ppItemSets; }

#endif // _NDSORT_HXX
