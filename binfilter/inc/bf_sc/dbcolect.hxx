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

#include "collect.hxx"

#include "global.hxx"		// MAXQUERY

#include "sortparam.hxx"	// MAXSORT

#include "refreshtimer.hxx"
namespace binfilter {
class ScDocument;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;

//------------------------------------------------------------------------

class ScDBData : public DataObject, public ScRefreshTimer
{

private:
    // DBParam
    String			aName;
    USHORT			nTable;
    USHORT      	nStartCol;
    USHORT			nStartRow;
    USHORT			nEndCol;
    USHORT			nEndRow;
    BOOL			bByRow;
    BOOL			bHasHeader;
    BOOL			bDoSize;
    BOOL			bKeepFmt;
    BOOL			bStripData;
    // SortParam
    BOOL			bSortCaseSens;
    BOOL			bIncludePattern;
    BOOL			bSortInplace;
    BOOL			bSortUserDef;
    USHORT			nSortUserIndex;
    USHORT			nSortDestTab;
    USHORT			nSortDestCol;
    USHORT			nSortDestRow;
    BOOL			bDoSort[MAXSORT];
    USHORT			nSortField[MAXSORT];
    BOOL			bAscending[MAXSORT];
    ::com::sun::star::lang::Locale aSortLocale;
    String			aSortAlgorithm;
    // QueryParam
    BOOL			bQueryInplace;
    BOOL			bQueryCaseSens;
    BOOL			bQueryRegExp;
    BOOL			bQueryDuplicate;
    USHORT			nQueryDestTab;
    USHORT			nQueryDestCol;
    USHORT			nQueryDestRow;
    BOOL			bDoQuery[MAXQUERY];
    USHORT			nQueryField[MAXQUERY];
    ScQueryOp		eQueryOp[MAXQUERY];
    BOOL			bQueryByString[MAXQUERY];
    String*			pQueryStr[MAXQUERY];
    double			nQueryVal[MAXQUERY];
    ScQueryConnect  eQueryConnect[MAXQUERY];
    BOOL			bIsAdvanced;		// TRUE if created by advanced filter
    ScRange			aAdvSource;			// source range
    // SubTotalParam
    BOOL			bSubRemoveOnly;
    BOOL			bSubReplace;
    BOOL			bSubPagebreak;
    BOOL			bSubCaseSens;
    BOOL			bSubDoSort;
    BOOL			bSubAscending;
    BOOL			bSubIncludePattern;
    BOOL			bSubUserDef;
    USHORT			nSubUserIndex;
    BOOL			bDoSubTotal[MAXSUBTOTAL];
    USHORT			nSubField[MAXSUBTOTAL];
    USHORT			nSubTotals[MAXSUBTOTAL];
    USHORT*			pSubTotals[MAXSUBTOTAL];
    ScSubTotalFunc*	pFunctions[MAXSUBTOTAL];
    // Datenbank-Import
    BOOL			bDBImport;
    String			aDBName;
    String			aDBStatement;
    BOOL			bDBNative;
    BOOL			bDBSelection;		// nicht im Param: Wenn Selektion, Update sperren
    BOOL			bDBSql;				// aDBStatement ist SQL und kein Name
    BYTE			nDBType;			// enum DBObject (bisher nur dbTable, dbQuery)

    USHORT			nIndex; 			// eindeutiger Index fuer Formeln
    USHORT			nExportIndex;		// used by Xcl export
    BOOL			bAutoFilter;		// AutoFilter? (nicht gespeichert)
    BOOL			bModified;			// wird bei UpdateReference gesetzt/geloescht

public:
            ScDBData(const String& rName,
                     USHORT nTab,
                     USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                     BOOL bByR = TRUE, BOOL bHasH = TRUE);
            ScDBData(const ScDBData& rData);
            ScDBData( SvStream& rStream, ScMultipleReadHeader& rHdr );
            ~ScDBData();

            virtual	DataObject*	Clone() const;

            ScDBData&	operator= (const ScDBData& rData);


            BOOL		Store( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

            const String& GetName() const				{ return aName; }
            void		GetName(String& rName) const	{ rName = aName; }
            void		SetName(const String& rName)	{ aName = rName; }
            void		GetArea(USHORT& rTab, USHORT& rCol1, USHORT& rRow1, USHORT& rCol2, USHORT& rRow2) const;
            void		GetArea(ScRange& rRange) const;
            void		SetArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2);
            BOOL		IsByRow() const 				{ return bByRow; }
            void		SetByRow(BOOL bByR) 			{ bByRow = bByR; }
            BOOL		HasHeader() const 				{ return bHasHeader; }
            void		SetHeader(BOOL bHasH) 			{ bHasHeader = bHasH; }
            void		SetIndex(USHORT nInd) 			{ nIndex = nExportIndex = nInd; }
            USHORT		GetIndex() const				{ return nIndex; }
            BOOL		IsDoSize() const				{ return bDoSize; }
            void		SetDoSize(BOOL bSet)			{ bDoSize = bSet; }
            BOOL		IsKeepFmt() const				{ return bKeepFmt; }
            void		SetKeepFmt(BOOL bSet)			{ bKeepFmt = bSet; }
            BOOL		IsStripData() const				{ return bStripData; }
            void		SetStripData(BOOL bSet)			{ bStripData = bSet; }

            BOOL		IsBeyond(USHORT nMaxRow) const;


            void		GetSortParam(ScSortParam& rSortParam) const;
            void		SetSortParam(const ScSortParam& rSortParam);

            void		GetQueryParam(ScQueryParam& rQueryParam) const;
            void		SetQueryParam(const ScQueryParam& rQueryParam);
            BOOL		GetAdvancedQuerySource(ScRange& rSource) const;
            void		SetAdvancedQuerySource(const ScRange* pSource);

            void		GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const;
            void		SetSubTotalParam(const ScSubTotalParam& rSubTotalParam);

            void		GetImportParam(ScImportParam& rImportParam) const;
            void		SetImportParam(const ScImportParam& rImportParam);



            BOOL		HasImportParam() const	 { return bDBImport; }
            BOOL		HasQueryParam() const	 { return bDoQuery[0]; }
            BOOL		HasSortParam() const	 { return bDoSort[0]; }
            BOOL		HasSubTotalParam() const { return bDoSubTotal[0]; }

            BOOL		HasImportSelection() const		{ return bDBSelection; }
            void		SetImportSelection(BOOL bSet)	{ bDBSelection = bSet; }

            BOOL		HasAutoFilter() const 		{ return bAutoFilter; }
            void		SetAutoFilter(BOOL bSet)	{ bAutoFilter = bSet; }

            BOOL		IsModified() const			{ return bModified; }
            void		SetModified(BOOL bMod)		{ bModified = bMod; }

            void		SetExportIndex( USHORT nIdx )	{ nExportIndex = nIdx; }
            USHORT		GetExportIndex() const		{ return nExportIndex; }

};


//------------------------------------------------------------------------
class ScDBCollection : public SortedCollection
{

private:
    Link		aRefreshHandler;
    ScDocument* pDoc;
    USHORT nEntryIndex;			// Zaehler fuer die eindeutigen Indizes

public:
    ScDBCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE, ScDocument* pDocument = NULL) :
                    SortedCollection	( nLim, nDel, bDup ),
                    pDoc				( pDocument ),
                    nEntryIndex			( SC_START_INDEX_DB_COLL )	// oberhalb der Namen
                    {}
    ScDBCollection(const ScDBCollection& rScDBCollection) :
                    SortedCollection	( rScDBCollection ),
                    pDoc 				( rScDBCollection.pDoc ),
                    nEntryIndex			( rScDBCollection.nEntryIndex)
                    {}

    virtual	DataObject*	Clone() const { return new ScDBCollection(*this); }
            ScDBData*	operator[]( const USHORT nIndex) const {return (ScDBData*)At(nIndex);}
    virtual	short		Compare(DataObject* pKey1, DataObject* pKey2) const;
            ScDBData*	GetDBAtCursor(USHORT nCol, USHORT nRow, USHORT nTab, BOOL bStartOnly) const;
            ScDBData*	GetDBAtArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2) const;

    BOOL	Load( SvStream& rStream );
    BOOL	Store( SvStream& rStream ) const;

    BOOL	SearchName( const String& rName, USHORT& rIndex ) const;

    void	UpdateReference(UpdateRefMode eUpdateRefMode,
                                USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                short nDx, short nDy, short nDz );

    ScDBData* FindIndex(USHORT nIndex);
    USHORT 	GetEntryIndex()					{ return nEntryIndex; }
    void 	SetEntryIndex(USHORT nInd)		{ nEntryIndex = nInd; }
    virtual BOOL Insert(DataObject* pDataObject);

    void			SetRefreshHandler( const Link& rLink )
                        { aRefreshHandler = rLink; }
    const Link&		GetRefreshHandler() const	{ return aRefreshHandler; }
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
