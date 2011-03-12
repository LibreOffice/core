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

#ifndef SC_DBCOLECT_HXX
#define SC_DBCOLECT_HXX

#include "scdllapi.h"
#include "collect.hxx"
#include "global.hxx"       // MAXQUERY
#include "sortparam.hxx"    // MAXSORT
#include "refreshtimer.hxx"
#include "address.hxx"
#include "scdllapi.h"

class ScDocument;

class ScDBData : public ScDataObject, public ScRefreshTimer
{

private:
    // DBParam
    String          aName;
    SCTAB           nTable;
    SCCOL           nStartCol;
    SCROW           nStartRow;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    BOOL            bByRow;
    BOOL            bHasHeader;
    BOOL            bDoSize;
    BOOL            bKeepFmt;
    BOOL            bStripData;
    // SortParam
    BOOL            bSortCaseSens;
    BOOL            bSortNaturalSort;
    BOOL            bIncludePattern;
    BOOL            bSortInplace;
    BOOL            bSortUserDef;
    USHORT          nSortUserIndex;
    SCTAB           nSortDestTab;
    SCCOL           nSortDestCol;
    SCROW           nSortDestRow;
    BOOL            bDoSort[MAXSORT];
    SCCOLROW        nSortField[MAXSORT];
    BOOL            bAscending[MAXSORT];
    ::com::sun::star::lang::Locale aSortLocale;
    String          aSortAlgorithm;
    // QueryParam
    BOOL            bQueryInplace;
    BOOL            bQueryCaseSens;
    BOOL            bQueryRegExp;
    BOOL            bQueryDuplicate;
    SCTAB           nQueryDestTab;
    SCCOL           nQueryDestCol;
    SCROW           nQueryDestRow;
    BOOL            bDoQuery[MAXQUERY];
    SCCOLROW        nQueryField[MAXQUERY];
    ScQueryOp       eQueryOp[MAXQUERY];
    BOOL            bQueryByString[MAXQUERY];
    bool            bQueryByDate[MAXQUERY];
    String*         pQueryStr[MAXQUERY];
    double          nQueryVal[MAXQUERY];
    ScQueryConnect  eQueryConnect[MAXQUERY];
    BOOL            bIsAdvanced;        // TRUE if created by advanced filter
    ScRange         aAdvSource;         // source range
    SCROW           nDynamicEndRow;
    // SubTotalParam
    BOOL            bSubRemoveOnly;
    BOOL            bSubReplace;
    BOOL            bSubPagebreak;
    BOOL            bSubCaseSens;
    BOOL            bSubDoSort;
    BOOL            bSubAscending;
    BOOL            bSubIncludePattern;
    BOOL            bSubUserDef;
    USHORT          nSubUserIndex;
    BOOL            bDoSubTotal[MAXSUBTOTAL];
    SCCOL           nSubField[MAXSUBTOTAL];
    SCCOL           nSubTotals[MAXSUBTOTAL];
    SCCOL*          pSubTotals[MAXSUBTOTAL];
    ScSubTotalFunc* pFunctions[MAXSUBTOTAL];
    // data base import
    BOOL            bDBImport;
    String          aDBName;
    String          aDBStatement;
    BOOL            bDBNative;
    BOOL            bDBSelection;       // not in Param: if selection, block update
    BOOL            bDBSql;             // aDBStatement is SQL not a name
    BYTE            nDBType;            // enum DBObject (up to now only dbTable, dbQuery)

    USHORT          nIndex;             // unique index formulas
    BOOL            bAutoFilter;        // AutoFilter? (not saved)
    BOOL            bModified;          // is set/cleared for/by(?) UpdateReference

    using ScRefreshTimer::operator==;

public:
            SC_DLLPUBLIC ScDBData(const String& rName,
                     SCTAB nTab,
                     SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                     BOOL bByR = TRUE, BOOL bHasH = TRUE);
            ScDBData(const ScDBData& rData);
            ~ScDBData();

    virtual ScDataObject*   Clone() const;

            ScDBData&   operator= (const ScDBData& rData);

            BOOL        operator== (const ScDBData& rData) const;

            const String& GetName() const               { return aName; }
            void        GetName(String& rName) const    { rName = aName; }
            void        SetName(const String& rName)    { aName = rName; }
            void        GetArea(SCTAB& rTab, SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2, bool bUseDynamicRange = false) const;
            SC_DLLPUBLIC void       GetArea(ScRange& rRange, bool bUseDynamicRange = false) const;
            void        SetArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
            void        SetDynamicEndRow(SCROW nRow);
            void        MoveTo(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
            BOOL        IsByRow() const                 { return bByRow; }
            void        SetByRow(BOOL bByR)             { bByRow = bByR; }
            BOOL        HasHeader() const               { return bHasHeader; }
            void        SetHeader(BOOL bHasH)           { bHasHeader = bHasH; }
            void        SetIndex(USHORT nInd)           { nIndex = nInd; }
            USHORT      GetIndex() const                { return nIndex; }
            BOOL        IsDoSize() const                { return bDoSize; }
            void        SetDoSize(BOOL bSet)            { bDoSize = bSet; }
            BOOL        IsKeepFmt() const               { return bKeepFmt; }
            void        SetKeepFmt(BOOL bSet)           { bKeepFmt = bSet; }
            BOOL        IsStripData() const             { return bStripData; }
            void        SetStripData(BOOL bSet)         { bStripData = bSet; }

            String      GetSourceString() const;
            String      GetOperations() const;

            void        GetSortParam(ScSortParam& rSortParam) const;
            void        SetSortParam(const ScSortParam& rSortParam);

            SC_DLLPUBLIC void       GetQueryParam(ScQueryParam& rQueryParam) const;
            SC_DLLPUBLIC void       SetQueryParam(const ScQueryParam& rQueryParam);
            SC_DLLPUBLIC BOOL       GetAdvancedQuerySource(ScRange& rSource) const;
            SC_DLLPUBLIC void       SetAdvancedQuerySource(const ScRange* pSource);

            void        GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const;
            void        SetSubTotalParam(const ScSubTotalParam& rSubTotalParam);

            void        GetImportParam(ScImportParam& rImportParam) const;
            void        SetImportParam(const ScImportParam& rImportParam);

            BOOL        IsDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, BOOL bStartOnly) const;
            BOOL        IsDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;

            BOOL        HasImportParam() const   { return bDBImport; }
            BOOL        HasQueryParam() const    { return bDoQuery[0]; }
            BOOL        HasSortParam() const     { return bDoSort[0]; }
            BOOL        HasSubTotalParam() const { return bDoSubTotal[0]; }

            BOOL        HasImportSelection() const      { return bDBSelection; }
            void        SetImportSelection(BOOL bSet)   { bDBSelection = bSet; }

            BOOL        HasAutoFilter() const       { return bAutoFilter; }
            void        SetAutoFilter(BOOL bSet)    { bAutoFilter = bSet; }

            BOOL        IsModified() const          { return bModified; }
            void        SetModified(BOOL bMod)      { bModified = bMod; }
};


//------------------------------------------------------------------------
class SC_DLLPUBLIC ScDBCollection : public ScSortedCollection
{

private:
    Link        aRefreshHandler;
    ScDocument* pDoc;
    USHORT nEntryIndex;         // counter for unique indices

public:
    ScDBCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE, ScDocument* pDocument = NULL) :
                    ScSortedCollection  ( nLim, nDel, bDup ),
                    pDoc                ( pDocument ),
                    nEntryIndex         ( SC_START_INDEX_DB_COLL )  // see above for the names
                    {}
    ScDBCollection(const ScDBCollection& rScDBCollection) :
                    ScSortedCollection  ( rScDBCollection ),
                    pDoc                ( rScDBCollection.pDoc ),
                    nEntryIndex         ( rScDBCollection.nEntryIndex)
                    {}

    virtual ScDataObject*   Clone() const { return new ScDBCollection(*this); }
            ScDBData*   operator[]( const USHORT nIndex) const {return (ScDBData*)At(nIndex);}
    virtual short       Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;
    virtual BOOL        IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const;
            ScDBData*   GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, BOOL bStartOnly) const;
            ScDBData*   GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;

    BOOL    SearchName( const String& rName, USHORT& rIndex ) const;

    void    DeleteOnTab( SCTAB nTab );
    void    UpdateReference(UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void    UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

    ScDBData* FindIndex(USHORT nIndex);
    USHORT  GetEntryIndex()                 { return nEntryIndex; }
    void    SetEntryIndex(USHORT nInd)      { nEntryIndex = nInd; }
    virtual BOOL Insert(ScDataObject* pScDataObject);

    void            SetRefreshHandler( const Link& rLink )
                        { aRefreshHandler = rLink; }
    const Link&     GetRefreshHandler() const   { return aRefreshHandler; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
