/*************************************************************************
 *
 *  $RCSfile: dociter.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:48 $
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

#ifndef SC_DOCITER_HXX
#define SC_DOCITER_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class ScDocument;
class ScBaseCell;
class ScPatternAttr;
class ScAttrArray;
class ScAttrIterator;

class ScDocumentIterator                // alle nichtleeren Zellen durchgehen
{
private:
    ScDocument*             pDoc;
    USHORT                  nStartTab;
    USHORT                  nEndTab;

    const ScPatternAttr*    pDefPattern;

    USHORT                  nCol;
    USHORT                  nRow;
    USHORT                  nTab;
    ScBaseCell*             pCell;
    const ScPatternAttr*    pPattern;


    USHORT                  nColPos;
    USHORT                  nAttrPos;

    BOOL                    GetThis();
    BOOL                    GetThisCol();

public:
            ScDocumentIterator( ScDocument* pDocument, USHORT nStartTable, USHORT nEndTable );
            ~ScDocumentIterator();

    BOOL                    GetFirst();
    BOOL                    GetNext();

    ScBaseCell*             GetCell();
    const ScPatternAttr*    GetPattern();
    void                    GetPos( USHORT& rCol, USHORT& rRow, USHORT& rTab );
};

class ScValueIterator            // alle Zahlenwerte in einem Bereich durchgehen
{
private:
    double          fNextValue;
    ScDocument*     pDoc;
    const ScAttrArray*  pAttrArray;
    ULONG           nNumFormat;     // fuer CalcAsShown
    ULONG           nNumFmtIndex;
    USHORT          nStartCol;
    USHORT          nStartRow;
    USHORT          nStartTab;
    USHORT          nEndCol;
    USHORT          nEndRow;
    USHORT          nEndTab;
    USHORT          nCol;
    USHORT          nRow;
    USHORT          nTab;
    USHORT          nColRow;
    USHORT          nNextRow;
    USHORT          nAttrEndRow;
    short           nNumFmtType;
    BOOL            bNumValid;
    BOOL            bSubTotal;
    BOOL            bNextValid;
    BOOL            bCalcAsShown;
    BOOL            bTextAsZero;

    BOOL            GetThis(double& rValue, USHORT& rErr);
public:
                    ScValueIterator(ScDocument* pDocument,
                                    USHORT nSCol, USHORT nSRow, USHORT nSTab,
                                    USHORT nECol, USHORT nERow, USHORT nETab,
                                    BOOL bSTotal = FALSE, BOOL bTextAsZero = FALSE);
                    ScValueIterator(ScDocument* pDocument,
                                    const ScRange& rRange, BOOL bSTotal = FALSE,
                                    BOOL bTextAsZero = FALSE );
    void            GetCurNumFmtInfo( short& nType, ULONG& nIndex );
    BOOL            GetFirst(double& rValue, USHORT& rErr);
    BOOL            GetNext(double& rValue, USHORT& rErr)
                    {
                        return bNextValid ? ( bNextValid = FALSE, rValue = fNextValue,
                                                rErr = 0, nRow = nNextRow,
                                                ++nColRow, bNumValid = FALSE, TRUE )
                                          : ( ++nRow, GetThis(rValue, rErr) );
                    }
};

class ScQueryValueIterator            // alle Zahlenwerte in einem Bereich durchgehen
{
private:
    ScQueryParam    aParam;
    ScDocument*     pDoc;
    ScAttrArray*    pAttrArray;
    ULONG           nNumFormat;     // fuer CalcAsShown
    ULONG           nNumFmtIndex;
    USHORT          nCol;
    USHORT          nRow;
    USHORT          nColRow;
    USHORT          nAttrEndRow;
    USHORT          nTab;
    short           nNumFmtType;
    BOOL            bCalcAsShown;

    BOOL            GetThis(double& rValue, USHORT& rErr);
public:
                    ScQueryValueIterator(ScDocument* pDocument, USHORT nTable,
                                         const ScQueryParam& aParam);
    BOOL            GetFirst(double& rValue, USHORT& rErr);
    BOOL            GetNext(double& rValue, USHORT& rErr);
    void            GetCurNumFmtInfo( short& nType, ULONG& nIndex )
                        { nType = nNumFmtType; nIndex = nNumFmtIndex; }
};

class ScCellIterator            // alle Zellen in einem Bereich durchgehen
{                               // bei SubTotal aber keine ausgeblendeten und
private:                        // SubTotalZeilen
    ScDocument*     pDoc;
    USHORT          nStartCol;
    USHORT          nStartRow;
    USHORT          nStartTab;
    USHORT          nEndCol;
    USHORT          nEndRow;
    USHORT          nEndTab;
    USHORT          nCol;
    USHORT          nRow;
    USHORT          nTab;
    USHORT          nColRow;
    BOOL            bSubTotal;

    ScBaseCell*     GetThis();
public:
                    ScCellIterator(ScDocument* pDocument,
                                   USHORT nSCol, USHORT nSRow, USHORT nSTab,
                                   USHORT nECol, USHORT nERow, USHORT nETab,
                                   BOOL bSTotal = FALSE);
                    ScCellIterator(ScDocument* pDocument,
                                   const ScRange& rRange, BOOL bSTotal = FALSE);
    ScBaseCell*     GetFirst();
    ScBaseCell*     GetNext();
    USHORT          GetCol() { return nCol; }
    USHORT          GetRow() { return nRow; }
    USHORT          GetTab() { return nTab; }
};

class ScQueryCellIterator           // alle nichtleeren Zellen in einem Bereich
{                                   // durchgehen
private:
    ScQueryParam    aParam;
    ScDocument*     pDoc;
    ScAttrArray*    pAttrArray;
    ULONG           nNumFormat;
    USHORT          nTab;
    USHORT          nCol;
    USHORT          nRow;
    USHORT          nColRow;
    USHORT          nAttrEndRow;
    BOOL            bAdvanceQuery;

    ScBaseCell*     GetThis();
public:
                    ScQueryCellIterator(ScDocument* pDocument, USHORT nTable,
                                        const ScQueryParam& aParam, BOOL bMod = TRUE);
                                        // fuer bMod = FALSE muss der QueryParam
                                        // weiter aufgefuellt sein (bIsString)
    ScBaseCell*     GetFirst();
    ScBaseCell*     GetNext();
    USHORT          GetCol() { return nCol; }
    USHORT          GetRow() { return nRow; }
    ULONG           GetNumberFormat();

                    // setzt alle Entry.nField einen weiter, wenn Spalte
                    // wechselt, fuer ScInterpreter ScHLookup()
    void            SetAdvanceQueryParamEntryField( BOOL bVal )
                        { bAdvanceQuery = bVal; }
    void            AdvanceQueryParamEntryField();
};

class ScDocAttrIterator             // alle Attribut-Bereiche
{
private:
    ScDocument*     pDoc;
    USHORT          nTab;
    USHORT          nEndCol;
    USHORT          nStartRow;
    USHORT          nEndRow;
    USHORT          nCol;
    ScAttrIterator* pColIter;

public:
                    ScDocAttrIterator(ScDocument* pDocument, USHORT nTable,
                                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
                    ~ScDocAttrIterator();

    const ScPatternAttr*    GetNext( USHORT& rCol, USHORT& rRow1, USHORT& rRow2 );
};

class ScAttrRectIterator            // alle Attribut-Bereiche, auch Bereiche ueber mehrere Spalten
{
private:
    ScDocument*     pDoc;
    USHORT          nTab;
    USHORT          nEndCol;
    USHORT          nStartRow;
    USHORT          nEndRow;
    USHORT          nIterStartCol;
    USHORT          nIterEndCol;
    ScAttrIterator* pColIter;

public:
                    ScAttrRectIterator(ScDocument* pDocument, USHORT nTable,
                                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
                    ~ScAttrRectIterator();

    void                    DataChanged();
    const ScPatternAttr*    GetNext( USHORT& rCol1, USHORT& rCol2, USHORT& rRow1, USHORT& rRow2 );
};

class ScHorizontalCellIterator      // alle nichtleeren Zellen in einem Bereich
{                                   // zeilenweise durchgehen
private:
    ScDocument*     pDoc;
    USHORT          nTab;
    USHORT          nStartCol;
    USHORT          nEndCol;
    USHORT          nEndRow;
    USHORT*         pNextRows;
    USHORT*         pNextIndices;
    USHORT          nCol;
    USHORT          nRow;
    BOOL            bMore;

public:
                    ScHorizontalCellIterator(ScDocument* pDocument, USHORT nTable,
                                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
                    ~ScHorizontalCellIterator();

    ScBaseCell*     GetNext( USHORT& rCol, USHORT& rRow );
    BOOL            ReturnNext( USHORT& rCol, USHORT& rRow );

private:
    void            Advance();
};


//
//  gibt alle Bereiche mit nicht-Default-Formatierung zurueck (horizontal)
//

class ScHorizontalAttrIterator
{
private:
    ScDocument*             pDoc;
    USHORT                  nTab;
    USHORT                  nStartCol;
    USHORT                  nStartRow;
    USHORT                  nEndCol;
    USHORT                  nEndRow;

    USHORT*                 pNextEnd;
    USHORT*                 pIndices;
    const ScPatternAttr**   ppPatterns;
    USHORT                  nCol;
    USHORT                  nRow;
    BOOL                    bRowEmpty;

public:
            ScHorizontalAttrIterator( ScDocument* pDocument, USHORT nTable,
                                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 );
            ~ScHorizontalAttrIterator();

    const ScPatternAttr*    GetNext( USHORT& rCol1, USHORT& rCol2, USHORT& rRow );
};

//
//  gibt nichtleere Zellen und Bereiche mit Formatierung zurueck (horizontal)
//

class ScUsedAreaIterator
{
private:
    ScHorizontalCellIterator    aCellIter;
    ScHorizontalAttrIterator    aAttrIter;

    USHORT                  nNextCol;
    USHORT                  nNextRow;

    USHORT                  nCellCol;
    USHORT                  nCellRow;
    const ScBaseCell*       pCell;
    USHORT                  nAttrCol1;
    USHORT                  nAttrCol2;
    USHORT                  nAttrRow;
    const ScPatternAttr*    pPattern;

    USHORT                  nFoundStartCol;         // Ergebnisse nach GetNext
    USHORT                  nFoundEndCol;
    USHORT                  nFoundRow;
    const ScPatternAttr*    pFoundPattern;
    const ScBaseCell*       pFoundCell;

public:
            ScUsedAreaIterator( ScDocument* pDocument, USHORT nTable,
                                USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 );
            ~ScUsedAreaIterator();

    BOOL    GetNext();

    USHORT                  GetStartCol() const     { return nFoundStartCol; }
    USHORT                  GetEndCol() const       { return nFoundEndCol; }
    USHORT                  GetRow() const          { return nFoundRow; }
    const ScPatternAttr*    GetPattern() const      { return pFoundPattern; }
    const ScBaseCell*       GetCell() const         { return pFoundCell; }
};

#endif


