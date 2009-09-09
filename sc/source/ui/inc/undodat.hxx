/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: undodat.hxx,v $
 * $Revision: 1.8.32.2 $
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

#ifndef SC_UNDODAT_HXX
#define SC_UNDODAT_HXX

#include "undobase.hxx"
#include "rangeutl.hxx"     // ScArea
#include "rangelst.hxx"     // ScRangeListRef
#include "markdata.hxx"
#include "sortparam.hxx"
#include "queryparam.hxx"
#include "pivot.hxx"

class ScDocShell;
class ScDocument;
class ScOutlineTable;
class ScRangeName;
class ScDBData;
class ScDBCollection;
class ScDPObject;
class SdrUndoAction;

//----------------------------------------------------------------------------

class ScUndoDoOutline: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoDoOutline( ScDocShell* pNewDocShell,
                            SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                            ScDocument* pNewUndoDoc, BOOL bNewColumns,
                            USHORT nNewLevel, USHORT nNewEntry, BOOL bNewShow );
    virtual         ~ScUndoDoOutline();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nTab;
    ScDocument*     pUndoDoc;
    BOOL            bColumns;
    USHORT          nLevel;
    USHORT          nEntry;
    BOOL            bShow;
};


class ScUndoMakeOutline: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoMakeOutline( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScOutlineTable* pNewUndoTab,
                            BOOL bNewColumns, BOOL bNewMake );
    virtual         ~ScUndoMakeOutline();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScOutlineTable* pUndoTable;
    BOOL            bColumns;
    BOOL            bMake;
};


class ScUndoOutlineLevel: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoOutlineLevel( ScDocShell* pNewDocShell,
                            SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                            BOOL bNewColumns, USHORT nNewLevel );
    virtual         ~ScUndoOutlineLevel();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nTab;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
    BOOL            bColumns;
    USHORT          nLevel;
};


class ScUndoOutlineBlock: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoOutlineBlock( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                            BOOL bNewShow );
    virtual         ~ScUndoOutlineBlock();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
    BOOL            bShow;
};


class ScUndoRemoveAllOutlines: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRemoveAllOutlines( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab );
    virtual         ~ScUndoRemoveAllOutlines();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
};


class ScUndoAutoOutline: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoAutoOutline( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab );
    virtual         ~ScUndoAutoOutline();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
};


class ScUndoSubTotals: public ScDBFuncUndo
{
public:
                    TYPEINFO();
                    ScUndoSubTotals( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScSubTotalParam& rNewParam, SCROW nNewEndY,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
//                          ScDBData* pNewData,
                            ScRangeName* pNewUndoRange, ScDBCollection* pNewUndoDB );
    virtual         ~ScUndoSubTotals();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB           nTab;
    ScSubTotalParam aParam;                         // Original uebergebener Parameter
    SCROW           nNewEndRow;                     // Ergebnis-Groesse
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
//  ScDBData*       pUndoDBData;
    ScRangeName*    pUndoRange;
    ScDBCollection* pUndoDB;
};


class ScUndoSort: public ScDBFuncUndo
{
public:
                    TYPEINFO();
                    ScUndoSort( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScSortParam& rParam,
                            BOOL bQuery, ScDocument* pNewUndoDoc,
                            ScDBCollection* pNewUndoDB, const ScRange* pDest = NULL );
    virtual         ~ScUndoSort();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB           nTab;
    ScSortParam     aSortParam;
    BOOL            bRepeatQuery;
    ScDocument*     pUndoDoc;
    ScDBCollection* pUndoDB;                // wegen Quell- und Zielbereich
    BOOL            bDestArea;
    ScRange         aDestRange;
};


class ScUndoQuery: public ScDBFuncUndo
{
public:
                    TYPEINFO();
                    ScUndoQuery( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScQueryParam& rParam, ScDocument* pNewUndoDoc,
                            ScDBCollection* pNewUndoDB, const ScRange* pOld,
                            BOOL bSize, const ScRange* pAdvSrc );
    virtual         ~ScUndoQuery();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SdrUndoAction*  pDrawUndo;
    SCTAB           nTab;
    ScQueryParam    aQueryParam;
    ScDocument*     pUndoDoc;
    ScDBCollection* pUndoDB;                // wegen Quell- und Zielbereich
    ScRange         aOldDest;
    ScRange         aAdvSource;
    BOOL            bIsAdvanced;
    BOOL            bDestArea;
    BOOL            bDoSize;
};


class ScUndoAutoFilter: public ScDBFuncUndo
{
private:
    String          aDBName;
    BOOL            bFilterSet;

    void            DoChange( BOOL bUndo );

public:
                    TYPEINFO();
                    ScUndoAutoFilter( ScDocShell* pNewDocShell, const ScRange& rRange,
                            const String& rName, BOOL bSet );
    virtual         ~ScUndoAutoFilter();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;
};


class ScUndoDBData: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoDBData( ScDocShell* pNewDocShell,
                            ScDBCollection* pNewUndoColl, ScDBCollection* pNewRedoColl );
    virtual         ~ScUndoDBData();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDBCollection* pUndoColl;
    ScDBCollection* pRedoColl;
};


class ScUndoImportData: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoImportData( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScImportParam& rParam, SCCOL nNewEndX, SCROW nNewEndY,
                            SCCOL nNewFormula,
                            ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                            ScDBData* pNewUndoData, ScDBData* pNewRedoData );
    virtual         ~ScUndoImportData();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB           nTab;
    ScImportParam   aImportParam;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
    ScDBData*       pUndoDBData;
    ScDBData*       pRedoDBData;
    SCCOL           nFormulaCols;
    BOOL            bRedoFilled;
};


class ScUndoRepeatDB: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRepeatDB( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            SCCOL nStartX, SCROW nStartY, SCCOL nEndX, SCROW nEndY,
                            SCROW nResultEndRow, SCCOL nCurX, SCROW nCurY,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                            ScRangeName* pNewUndoRange, ScDBCollection* pNewUndoDB,
                            const ScRange* pOldQ, const ScRange* pNewQ );
    virtual         ~ScUndoRepeatDB();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    SCROW           nNewEndRow;
    ScAddress       aCursorPos;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
    ScRangeName*    pUndoRange;
    ScDBCollection* pUndoDB;
    ScRange         aOldQuery;
    ScRange         aNewQuery;
    BOOL            bQuerySize;
};

//UNUSED2008-05  class ScUndoPivot: public ScSimpleUndo
//UNUSED2008-05  {
//UNUSED2008-05  public:
//UNUSED2008-05                      TYPEINFO();
//UNUSED2008-05                      ScUndoPivot( ScDocShell* pNewDocShell,
//UNUSED2008-05                              const ScArea& rOld, const ScArea& rNew,
//UNUSED2008-05                              ScDocument* pOldDoc, ScDocument* pNewDoc,
//UNUSED2008-05                              const ScPivot* pOldPivot, const ScPivot* pNewPivot );
//UNUSED2008-05      virtual         ~ScUndoPivot();
//UNUSED2008-05
//UNUSED2008-05      virtual void    Undo();
//UNUSED2008-05      virtual void    Redo();
//UNUSED2008-05      virtual void    Repeat(SfxRepeatTarget& rTarget);
//UNUSED2008-05      virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;
//UNUSED2008-05
//UNUSED2008-05      virtual String  GetComment() const;
//UNUSED2008-05
//UNUSED2008-05  private:
//UNUSED2008-05      ScArea          aOldArea;
//UNUSED2008-05      ScArea          aNewArea;
//UNUSED2008-05      ScDocument*     pOldUndoDoc;
//UNUSED2008-05      ScDocument*     pNewUndoDoc;
//UNUSED2008-05      ScPivotParam    aOldParam;              // fuer Redo
//UNUSED2008-05      ScQueryParam    aOldQuery;
//UNUSED2008-05      ScArea          aOldSrc;
//UNUSED2008-05      ScPivotParam    aNewParam;              // fuer Undo in Collection
//UNUSED2008-05      ScQueryParam    aNewQuery;
//UNUSED2008-05      ScArea          aNewSrc;
//UNUSED2008-05      String          aOldName;
//UNUSED2008-05      String          aOldTag;
//UNUSED2008-05      String          aNewName;
//UNUSED2008-05      String          aNewTag;
//UNUSED2008-05  };

class ScUndoDataPilot: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoDataPilot( ScDocShell* pNewDocShell,
                            ScDocument* pOldDoc, ScDocument* pNewDoc,
                            const ScDPObject* pOldObj, const ScDPObject* pNewObj,
                            BOOL bMove );
    virtual         ~ScUndoDataPilot();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDocument*     pOldUndoDoc;
    ScDocument*     pNewUndoDoc;
    ScDPObject*     pOldDPObject;
    ScDPObject*     pNewDPObject;
    BOOL            bAllowMove;
};


class ScUndoConsolidate: public ScSimpleUndo
{
public:
                        TYPEINFO();
                        ScUndoConsolidate( ScDocShell* pNewDocShell,
                                const ScArea& rArea, const ScConsolidateParam& rPar,
                                ScDocument* pNewUndoDoc, BOOL bReference,
                                SCROW nInsCount, ScOutlineTable* pTab,
                                ScDBData* pData );
    virtual             ~ScUndoConsolidate();

    virtual void        Undo();
    virtual void        Redo();
    virtual void        Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL        CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String      GetComment() const;

private:
    ScArea              aDestArea;
    ScDocument*         pUndoDoc;
    ScConsolidateParam  aParam;
    BOOL                bInsRef;
    SCSIZE              nInsertCount;
    ScOutlineTable*     pUndoTab;
    ScDBData*           pUndoData;
};


class ScUndoChartData: public ScSimpleUndo
{
public:
                        TYPEINFO();
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const String& rName, const ScRange& rNew,
                                BOOL bColHdr, BOOL bRowHdr, BOOL bAdd );
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const String& rName, const ScRangeListRef& rNew,
                                BOOL bColHdr, BOOL bRowHdr, BOOL bAdd );
    virtual             ~ScUndoChartData();

    virtual void        Undo();
    virtual void        Redo();
    virtual void        Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL        CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String      GetComment() const;

private:
    String              aChartName;
    ScRangeListRef      aOldRangeListRef;
    BOOL                bOldColHeaders;
    BOOL                bOldRowHeaders;
//  ScRange             aNewRange;
    ScRangeListRef      aNewRangeListRef;
    BOOL                bNewColHeaders;
    BOOL                bNewRowHeaders;
    BOOL                bAddRange;

    void                Init();
};




#endif

