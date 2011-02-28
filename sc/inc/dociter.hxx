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

#ifndef SC_DOCITER_HXX
#define SC_DOCITER_HXX

#include "address.hxx"
#include <tools/solar.h>
#include "global.hxx"
#include "scdllapi.h"
#include "queryparam.hxx"

#include <memory>

#include <set>

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

    sal_Bool                    GetThis();
    sal_Bool                    GetThisCol();

public:
            ScDocumentIterator( ScDocument* pDocument, SCTAB nStartTable, SCTAB nEndTable );
            ~ScDocumentIterator();

    sal_Bool                    GetFirst();
    sal_Bool                    GetNext();

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
    sal_uLong           nNumFormat;     // fuer CalcAsShown
    sal_uLong           nNumFmtIndex;
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
    sal_Bool            bNumValid;
    sal_Bool            bSubTotal;
    sal_Bool            bNextValid;
    sal_Bool            bCalcAsShown;
    sal_Bool            bTextAsZero;

    sal_Bool            GetThis(double& rValue, sal_uInt16& rErr);
public:
//UNUSED2008-05  ScValueIterator(ScDocument* pDocument,
//UNUSED2008-05                  SCCOL nSCol, SCROW nSRow, SCTAB nSTab,
//UNUSED2008-05                  SCCOL nECol, SCROW nERow, SCTAB nETab,
//UNUSED2008-05                  sal_Bool bSTotal = sal_False, sal_Bool bTextAsZero = sal_False);

                    ScValueIterator(ScDocument* pDocument,
                                    const ScRange& rRange, sal_Bool bSTotal = sal_False,
                                    sal_Bool bTextAsZero = sal_False );
    void            GetCurNumFmtInfo( short& nType, sal_uLong& nIndex );
    /// Does NOT reset rValue if no value found!
    sal_Bool            GetFirst(double& rValue, sal_uInt16& rErr);
    /// Does NOT reset rValue if no value found!
    sal_Bool            GetNext(double& rValue, sal_uInt16& rErr)
                    {
                        return bNextValid ? ( bNextValid = sal_False, rValue = fNextValue,
                                                rErr = 0, nRow = nNextRow,
                                                ++nColRow, bNumValid = sal_False, sal_True )
                                          : ( ++nRow, GetThis(rValue, rErr) );
                    }
};

// ============================================================================

class ScDBQueryDataIterator
{
public:
    struct Value
    {
        ::rtl::OUString maString;
        double          mfValue;
        sal_uInt16      mnError;
        bool            mbIsNumber;

        Value();
    };

private:
    static SCROW        GetRowByColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCCOL nCol, SCSIZE nColRow);
    static ScBaseCell*  GetCellByColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCCOL nCol, SCSIZE nColRow);
    static ScAttrArray* GetAttrArrayByCol(ScDocument& rDoc, SCTAB nTab, SCCOL nCol);
    static bool         IsQueryValid(ScDocument& rDoc, const ScQueryParam& rParam, SCTAB nTab, SCROW nRow, ScBaseCell* pCell);
    static SCSIZE       SearchColEntryIndex(ScDocument& rDoc, SCTAB nTab, SCROW nRow, SCCOL nCol);

    class DataAccess
    {
    public:
        DataAccess(const ScDBQueryDataIterator* pParent);
        virtual ~DataAccess() = 0;
        virtual bool getCurrent(Value& rValue) = 0;
        virtual bool getFirst(Value& rValue) = 0;
        virtual bool getNext(Value& rValue) = 0;
    protected:
        const ScDBQueryDataIterator* mpParent;
    };

    class DataAccessInternal : public DataAccess
    {
    public:
        DataAccessInternal(const ScDBQueryDataIterator* pParent, ScDBQueryParamInternal* pParam, ScDocument* pDoc);
        virtual ~DataAccessInternal();
        virtual bool getCurrent(Value& rValue);
        virtual bool getFirst(Value& rValue);
        virtual bool getNext(Value& rValue);

    private:
        ScDBQueryParamInternal* mpParam;
        ScDocument*         mpDoc;
        const ScAttrArray*  pAttrArray;
        sal_uLong               nNumFormat;     // for CalcAsShown
        sal_uLong               nNumFmtIndex;
        SCCOL               nCol;
        SCROW               nRow;
        SCSIZE              nColRow;
        SCROW               nAttrEndRow;
        SCTAB               nTab;
        short               nNumFmtType;
        bool                bCalcAsShown;
    };

    class DataAccessMatrix : public DataAccess
    {
    public:
        DataAccessMatrix(const ScDBQueryDataIterator* pParent, ScDBQueryParamMatrix* pParam);
        virtual ~DataAccessMatrix();
        virtual bool getCurrent(Value& rValue);
        virtual bool getFirst(Value& rValue);
        virtual bool getNext(Value& rValue);

    private:
        bool isValidQuery(SCROW mnRow, const ScMatrix& rMat) const;

        ScDBQueryParamMatrix* mpParam;
        SCROW mnCurRow;
        SCROW mnRows;
        SCCOL mnCols;
    };

    ::std::auto_ptr<ScDBQueryParamBase> mpParam;
    ::std::auto_ptr<DataAccess>         mpData;

public:
                    ScDBQueryDataIterator(ScDocument* pDocument, ScDBQueryParamBase* pParam);
    /// Does NOT reset rValue if no value found!
    bool            GetFirst(Value& rValue);
    /// Does NOT reset rValue if no value found!
    bool            GetNext(Value& rValue);
};

// ============================================================================

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
    sal_Bool            bSubTotal;

    ScBaseCell*     GetThis();
public:
                    ScCellIterator(ScDocument* pDocument,
                                   SCCOL nSCol, SCROW nSRow, SCTAB nSTab,
                                   SCCOL nECol, SCROW nERow, SCTAB nETab,
                                   sal_Bool bSTotal = sal_False);
                    ScCellIterator(ScDocument* pDocument,
                                   const ScRange& rRange, sal_Bool bSTotal = sal_False);
    ScBaseCell*     GetFirst();
    ScBaseCell*     GetNext();
    SCCOL           GetCol() const { return nCol; }
    SCROW           GetRow() const { return nRow; }
    SCTAB           GetTab() const { return nTab; }
    ScAddress       GetPos() const { return ScAddress( nCol, nRow, nTab ); }
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
    sal_uLong           nNumFormat;
    SCTAB           nTab;
    SCCOL           nCol;
    SCROW           nRow;
    SCSIZE          nColRow;
    SCROW           nAttrEndRow;
    sal_uInt8            nStopOnMismatch;
    sal_uInt8            nTestEqualCondition;
    sal_Bool            bAdvanceQuery;
    sal_Bool            bIgnoreMismatchOnLeadingStrings;

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
                                        const ScQueryParam& aParam, sal_Bool bMod = sal_True);
                                        // fuer bMod = sal_False muss der QueryParam
                                        // weiter aufgefuellt sein (bIsString)
    ScBaseCell*     GetFirst();
    ScBaseCell*     GetNext();
    SCCOL           GetCol() { return nCol; }
    SCROW           GetRow() { return nRow; }

                    // setzt alle Entry.nField einen weiter, wenn Spalte
                    // wechselt, fuer ScInterpreter ScHLookup()
    void            SetAdvanceQueryParamEntryField( sal_Bool bVal )
                        { bAdvanceQuery = bVal; }
    void            AdvanceQueryParamEntryField();

                    /** If set, iterator stops on first non-matching cell
                        content. May be used in SC_LESS_EQUAL queries where a
                        cell range is assumed to be sorted; stops on first
                        value being greater than the queried value and
                        GetFirst()/GetNext() return NULL. StoppedOnMismatch()
                        returns sal_True then.
                        However, the iterator's conditions are not set to end
                        all queries, GetCol() and GetRow() return values for
                        the non-matching cell, further GetNext() calls may be
                        executed. */
    void            SetStopOnMismatch( sal_Bool bVal )
                        {
                            nStopOnMismatch = sal::static_int_cast<sal_uInt8>(bVal ? nStopOnMismatchEnabled :
                                nStopOnMismatchDisabled);
                        }
    sal_Bool            StoppedOnMismatch() const
                        { return nStopOnMismatch == nStopOnMismatchExecuted; }

                    /** If set, an additional test for SC_EQUAL condition is
                        executed in ScTable::ValidQuery() if SC_LESS_EQUAL or
                        SC_GREATER_EQUAL conditions are to be tested. May be
                        used where a cell range is assumed to be sorted to stop
                        if an equal match is found. */
    void            SetTestEqualCondition( sal_Bool bVal )
                        {
                            nTestEqualCondition = sal::static_int_cast<sal_uInt8>(bVal ?
                                nTestEqualConditionEnabled :
                                nTestEqualConditionDisabled);
                        }
    sal_Bool            IsEqualConditionFulfilled() const
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
                            values. If this parameter is sal_True, the search does
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
    sal_Bool            FindEqualOrSortedLastInRange( SCCOL& nFoundCol,
                        SCROW& nFoundRow, sal_Bool bSearchForEqualAfterMismatch = sal_False,
                        sal_Bool bIgnoreMismatchOnLeadingStrings = sal_True );
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
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCROW*          pNextRows;
    SCSIZE*         pNextIndices;
    SCCOL           nCol;
    SCROW           nRow;
    sal_Bool            bMore;

public:
                    ScHorizontalCellIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
                    ~ScHorizontalCellIterator();

    ScBaseCell*     GetNext( SCCOL& rCol, SCROW& rRow );
    sal_Bool            ReturnNext( SCCOL& rCol, SCROW& rRow );
    /// Set a(nother) sheet and (re)init.
    void            SetTab( SCTAB nTab );

private:
    void            Advance();
};


/** Row-wise value iterator. */
class ScHorizontalValueIterator
{
private:
    ScDocument               *pDoc;
    const ScAttrArray        *pAttrArray;
    ScHorizontalCellIterator *pCellIter;
    ULONG                     nNumFormat;     // for CalcAsShown
    ULONG                     nNumFmtIndex;
    SCTAB                     nEndTab;
    SCCOL                     nCurCol;
    SCROW                     nCurRow;
    SCTAB                     nCurTab;
    SCROW                     nAttrEndRow;
    short                     nNumFmtType;
    bool                      bNumValid;
    bool                      bSubTotal;
    bool                      bCalcAsShown;
    bool                      bTextAsZero;

public:

                    ScHorizontalValueIterator( ScDocument* pDocument,
                                               const ScRange& rRange,
                                               bool bSTotal = false,
                                               bool bTextAsZero = false );
                    ~ScHorizontalValueIterator();
    void            GetCurNumFmtInfo( short& nType, ULONG& nIndex );
    /// Does NOT reset rValue if no value found!
    bool            GetNext( double& rValue, USHORT& rErr );
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
    sal_Bool                    bRowEmpty;

public:
            ScHorizontalAttrIterator( ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
            ~ScHorizontalAttrIterator();

    const ScPatternAttr*    GetNext( SCCOL& rCol1, SCCOL& rCol2, SCROW& rRow );
};

//
//  gibt nichtleere Zellen und Bereiche mit Formatierung zurueck (horizontal)
//

class SC_DLLPUBLIC ScUsedAreaIterator
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

    sal_Bool    GetNext();

    SCCOL                   GetStartCol() const     { return nFoundStartCol; }
    SCCOL                   GetEndCol() const       { return nFoundEndCol; }
    SCROW                   GetRow() const          { return nFoundRow; }
    const ScPatternAttr*    GetPattern() const      { return pFoundPattern; }
    const ScBaseCell*       GetCell() const         { return pFoundCell; }
};

// ============================================================================

class ScRowBreakIterator
{
public:
    static SCROW NOT_FOUND;

    explicit ScRowBreakIterator(::std::set<SCROW>& rBreaks);
    SCROW first();
    SCROW next();

private:
    ::std::set<SCROW>& mrBreaks;
    ::std::set<SCROW>::const_iterator maItr;
    ::std::set<SCROW>::const_iterator maEnd;
};

#endif


