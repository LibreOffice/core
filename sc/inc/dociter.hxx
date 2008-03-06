/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dociter.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:15:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_DOCITER_HXX
#define SC_DOCITER_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

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
class ScRange;

class ScDocumentIterator                // alle nichtleeren Zellen durchgehen
{
private:
    ScDocument*             pDoc;
    SCTAB                   nStartTab;
    SCTAB                   nEndTab;

    const ScPatternAttr*    pDefPattern;

    SCCOL                   nCol;
    SCROW                   nRow;
    SCTAB                   nTab;
    ScBaseCell*             pCell;
    const ScPatternAttr*    pPattern;


    SCSIZE                  nColPos;
    SCSIZE                  nAttrPos;

    BOOL                    GetThis();
    BOOL                    GetThisCol();

public:
            ScDocumentIterator( ScDocument* pDocument, SCTAB nStartTable, SCTAB nEndTable );
            ~ScDocumentIterator();

    BOOL                    GetFirst();
    BOOL                    GetNext();

    ScBaseCell*             GetCell();
    const ScPatternAttr*    GetPattern();
    void                    GetPos( SCCOL& rCol, SCROW& rRow, SCTAB& rTab );
};

class ScValueIterator            // alle Zahlenwerte in einem Bereich durchgehen
{
private:
    double          fNextValue;
    ScDocument*     pDoc;
    const ScAttrArray*  pAttrArray;
    ULONG           nNumFormat;     // fuer CalcAsShown
    ULONG           nNumFmtIndex;
    SCCOL           nStartCol;
    SCROW           nStartRow;
    SCTAB           nStartTab;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    SCTAB           nEndTab;
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    SCSIZE          nColRow;
    SCROW           nNextRow;
    SCROW           nAttrEndRow;
    short           nNumFmtType;
    BOOL            bNumValid;
    BOOL            bSubTotal;
    BOOL            bNextValid;
    BOOL            bCalcAsShown;
    BOOL            bTextAsZero;

    BOOL            GetThis(double& rValue, USHORT& rErr);
public:
                    ScValueIterator(ScDocument* pDocument,
                                    SCCOL nSCol, SCROW nSRow, SCTAB nSTab,
                                    SCCOL nECol, SCROW nERow, SCTAB nETab,
                                    BOOL bSTotal = FALSE, BOOL bTextAsZero = FALSE);
                    ScValueIterator(ScDocument* pDocument,
                                    const ScRange& rRange, BOOL bSTotal = FALSE,
                                    BOOL bTextAsZero = FALSE );
    void            GetCurNumFmtInfo( short& nType, ULONG& nIndex );
    /// Does NOT reset rValue if no value found!
    BOOL            GetFirst(double& rValue, USHORT& rErr);
    /// Does NOT reset rValue if no value found!
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
    const ScAttrArray*  pAttrArray;
    ULONG           nNumFormat;     // fuer CalcAsShown
    ULONG           nNumFmtIndex;
    SCCOL           nCol;
    SCROW           nRow;
    SCSIZE          nColRow;
    SCROW           nAttrEndRow;
    SCTAB           nTab;
    short           nNumFmtType;
    BOOL            bCalcAsShown;

    BOOL            GetThis(double& rValue, USHORT& rErr);
public:
                    ScQueryValueIterator(ScDocument* pDocument, SCTAB nTable,
                                         const ScQueryParam& aParam);
    /// Does NOT reset rValue if no value found!
    BOOL            GetFirst(double& rValue, USHORT& rErr);
    /// Does NOT reset rValue if no value found!
    BOOL            GetNext(double& rValue, USHORT& rErr);
    void            GetCurNumFmtInfo( short& nType, ULONG& nIndex )
                        { nType = nNumFmtType; nIndex = nNumFmtIndex; }
};

class ScCellIterator            // alle Zellen in einem Bereich durchgehen
{                               // bei SubTotal aber keine ausgeblendeten und
private:                        // SubTotalZeilen
    ScDocument*     pDoc;
    SCCOL           nStartCol;
    SCROW           nStartRow;
    SCTAB           nStartTab;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    SCTAB           nEndTab;
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    SCSIZE          nColRow;
    BOOL            bSubTotal;

    ScBaseCell*     GetThis();
public:
                    ScCellIterator(ScDocument* pDocument,
                                   SCCOL nSCol, SCROW nSRow, SCTAB nSTab,
                                   SCCOL nECol, SCROW nERow, SCTAB nETab,
                                   BOOL bSTotal = FALSE);
                    ScCellIterator(ScDocument* pDocument,
                                   const ScRange& rRange, BOOL bSTotal = FALSE);
    ScBaseCell*     GetFirst();
    ScBaseCell*     GetNext();
    SCCOL          GetCol() { return nCol; }
    SCROW          GetRow() { return nRow; }
    SCTAB          GetTab() { return nTab; }
};

class ScQueryCellIterator           // alle nichtleeren Zellen in einem Bereich
{                                   // durchgehen
    enum StopOnMismatchBits
    {
        nStopOnMismatchDisabled = 0x00,
        nStopOnMismatchEnabled  = 0x01,
        nStopOnMismatchOccured  = 0x02,
        nStopOnMismatchExecuted = nStopOnMismatchEnabled | nStopOnMismatchOccured
    };

    enum TestEqualConditionBits
    {
        nTestEqualConditionDisabled = 0x00,
        nTestEqualConditionEnabled  = 0x01,
        nTestEqualConditionMatched  = 0x02,
        nTestEqualConditionFulfilled = nTestEqualConditionEnabled | nTestEqualConditionMatched
    };

private:
    ScQueryParam    aParam;
    ScDocument*     pDoc;
    const ScAttrArray*  pAttrArray;
    ULONG           nNumFormat;
    SCTAB           nTab;
    SCCOL           nCol;
    SCROW           nRow;
    SCSIZE          nColRow;
    SCROW           nAttrEndRow;
    BYTE            nStopOnMismatch;
    BYTE            nTestEqualCondition;
    BOOL            bAdvanceQuery;
    BOOL            bIgnoreMismatchOnLeadingStrings;

    ScBaseCell*     GetThis();

                    /* Only works if no regular expression is involved, only
                       searches for rows in one column, and only the first
                       query entry is considered with simple conditions
                       SC_LESS_EQUAL (sorted ascending) or SC_GREATER_EQUAL
                       (sorted descending). Check these things before
                       invocation! Delivers a starting point, continue with
                       GetThis() and GetNext() afterwards. Introduced for
                       FindEqualOrSortedLastInRange()
                     */
    ScBaseCell*     BinarySearch();

public:
                    ScQueryCellIterator(ScDocument* pDocument, SCTAB nTable,
                                        const ScQueryParam& aParam, BOOL bMod = TRUE);
                                        // fuer bMod = FALSE muss der QueryParam
                                        // weiter aufgefuellt sein (bIsString)
    ScBaseCell*     GetFirst();
    ScBaseCell*     GetNext();
    SCCOL           GetCol() { return nCol; }
    SCROW           GetRow() { return nRow; }
    ULONG           GetNumberFormat();

                    // setzt alle Entry.nField einen weiter, wenn Spalte
                    // wechselt, fuer ScInterpreter ScHLookup()
    void            SetAdvanceQueryParamEntryField( BOOL bVal )
                        { bAdvanceQuery = bVal; }
    void            AdvanceQueryParamEntryField();

                    /** If set, iterator stops on first non-matching cell
                        content. May be used in SC_LESS_EQUAL queries where a
                        cell range is assumed to be sorted; stops on first
                        value being greater than the queried value and
                        GetFirst()/GetNext() return NULL. StoppedOnMismatch()
                        returns TRUE then.
                        However, the iterator's conditions are not set to end
                        all queries, GetCol() and GetRow() return values for
                        the non-matching cell, further GetNext() calls may be
                        executed. */
    void            SetStopOnMismatch( BOOL bVal )
                        {
                            nStopOnMismatch = sal::static_int_cast<BYTE>(bVal ? nStopOnMismatchEnabled :
                                nStopOnMismatchDisabled);
                        }
    BOOL            StoppedOnMismatch() const
                        { return nStopOnMismatch == nStopOnMismatchExecuted; }

                    /** If set, an additional test for SC_EQUAL condition is
                        executed in ScTable::ValidQuery() if SC_LESS_EQUAL or
                        SC_GREATER_EQUAL conditions are to be tested. May be
                        used where a cell range is assumed to be sorted to stop
                        if an equal match is found. */
    void            SetTestEqualCondition( BOOL bVal )
                        {
                            nTestEqualCondition = sal::static_int_cast<BYTE>(bVal ?
                                nTestEqualConditionEnabled :
                                nTestEqualConditionDisabled);
                        }
    BOOL            IsEqualConditionFulfilled() const
                        { return nTestEqualCondition == nTestEqualConditionFulfilled; }

                    /** In a range assumed to be sorted find either the last of
                        a sequence of equal entries or the last being less than
                        (or greater than) the queried value. Used by the
                        interpreter for [HV]?LOOKUP() and MATCH(). Column and
                        row position of the found entry are returned, otherwise
                        invalid.

                        @param bSearchForEqualAfterMismatch
                            Continue searching for an equal entry even if the
                            last entry matching the range was found, in case
                            the data is not sorted. Is always done if regular
                            expressions are involved.

                        @param bIgnoreMismatchOnLeadingStrings
                            Normally strings are sorted behind numerical
                            values. If this parameter is TRUE, the search does
                            not stop when encountering a string and does not
                            assume that no values follow anymore.
                            If querying for a string a mismatch on the first
                            entry, e.g. column header, is ignored.

                        @ATTENTION! StopOnMismatch, TestEqualCondition and
                        the internal IgnoreMismatchOnLeadingStrings and query
                        params are in an undefined state upon return! The
                        iterator is not usable anymore except for obtaining the
                        number format!
                      */
    BOOL            FindEqualOrSortedLastInRange( SCCOL& nFoundCol,
                        SCROW& nFoundRow, BOOL bSearchForEqualAfterMismatch = FALSE,
                        BOOL bIgnoreMismatchOnLeadingStrings = TRUE );
};

class ScDocAttrIterator             // alle Attribut-Bereiche
{
private:
    ScDocument*     pDoc;
    SCTAB           nTab;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCCOL           nCol;
    ScAttrIterator* pColIter;

public:
                    ScDocAttrIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
                    ~ScDocAttrIterator();

    const ScPatternAttr*    GetNext( SCCOL& rCol, SCROW& rRow1, SCROW& rRow2 );
};

class ScAttrRectIterator            // alle Attribut-Bereiche, auch Bereiche ueber mehrere Spalten
{
private:
    ScDocument*     pDoc;
    SCTAB           nTab;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCCOL           nIterStartCol;
    SCCOL           nIterEndCol;
    ScAttrIterator* pColIter;

public:
                    ScAttrRectIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
                    ~ScAttrRectIterator();

    void                    DataChanged();
    const ScPatternAttr*    GetNext( SCCOL& rCol1, SCCOL& rCol2, SCROW& rRow1, SCROW& rRow2 );
};

class ScHorizontalCellIterator      // alle nichtleeren Zellen in einem Bereich
{                                   // zeilenweise durchgehen
private:
    ScDocument*     pDoc;
    SCTAB           nTab;
    SCCOL           nStartCol;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    SCROW*          pNextRows;
    SCSIZE*         pNextIndices;
    SCCOL           nCol;
    SCROW           nRow;
    BOOL            bMore;

public:
                    ScHorizontalCellIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
                    ~ScHorizontalCellIterator();

    ScBaseCell*     GetNext( SCCOL& rCol, SCROW& rRow );
    BOOL            ReturnNext( SCCOL& rCol, SCROW& rRow );

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
    SCTAB                   nTab;
    SCCOL                   nStartCol;
    SCROW                   nStartRow;
    SCCOL                   nEndCol;
    SCROW                   nEndRow;

    SCROW*                  pNextEnd;
    SCSIZE*                 pIndices;
    const ScPatternAttr**   ppPatterns;
    SCCOL                   nCol;
    SCROW                   nRow;
    BOOL                    bRowEmpty;

public:
            ScHorizontalAttrIterator( ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
            ~ScHorizontalAttrIterator();

    const ScPatternAttr*    GetNext( SCCOL& rCol1, SCCOL& rCol2, SCROW& rRow );
};

//
//  gibt nichtleere Zellen und Bereiche mit Formatierung zurueck (horizontal)
//

class ScUsedAreaIterator
{
private:
    ScHorizontalCellIterator    aCellIter;
    ScHorizontalAttrIterator    aAttrIter;

    SCCOL                   nNextCol;
    SCROW                   nNextRow;

    SCCOL                   nCellCol;
    SCROW                   nCellRow;
    const ScBaseCell*       pCell;
    SCCOL                   nAttrCol1;
    SCCOL                   nAttrCol2;
    SCROW                   nAttrRow;
    const ScPatternAttr*    pPattern;

    SCCOL                   nFoundStartCol;         // Ergebnisse nach GetNext
    SCCOL                   nFoundEndCol;
    SCROW                   nFoundRow;
    const ScPatternAttr*    pFoundPattern;
    const ScBaseCell*       pFoundCell;

public:
            ScUsedAreaIterator( ScDocument* pDocument, SCTAB nTable,
                                SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
            ~ScUsedAreaIterator();

    BOOL    GetNext();

    SCCOL                   GetStartCol() const     { return nFoundStartCol; }
    SCCOL                   GetEndCol() const       { return nFoundEndCol; }
    SCROW                   GetRow() const          { return nFoundRow; }
    const ScPatternAttr*    GetPattern() const      { return pFoundPattern; }
    const ScBaseCell*       GetCell() const         { return pFoundCell; }
};

#endif


