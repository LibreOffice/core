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
                    ScUndoDoOutline( ScDocShell* pNewDocShell,
                            SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                            ScDocument* pNewUndoDoc, sal_Bool bNewColumns,
                            sal_uInt16 nNewLevel, sal_uInt16 nNewEntry, sal_Bool bNewShow );
    virtual         ~ScUndoDoOutline();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nTab;
    ScDocument*     pUndoDoc;
    sal_Bool            bColumns;
    sal_uInt16          nLevel;
    sal_uInt16          nEntry;
    sal_Bool            bShow;
};


class ScUndoMakeOutline: public ScSimpleUndo
{
public:
                    ScUndoMakeOutline( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScOutlineTable* pNewUndoTab,
                            sal_Bool bNewColumns, sal_Bool bNewMake );
    virtual         ~ScUndoMakeOutline();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScOutlineTable* pUndoTable;
    sal_Bool            bColumns;
    sal_Bool            bMake;
};


class ScUndoOutlineLevel: public ScSimpleUndo
{
public:
                    ScUndoOutlineLevel( ScDocShell* pNewDocShell,
                            SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                            sal_Bool bNewColumns, sal_uInt16 nNewLevel );
    virtual         ~ScUndoOutlineLevel();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nTab;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
    sal_Bool            bColumns;
    sal_uInt16          nLevel;
};


class ScUndoOutlineBlock: public ScSimpleUndo
{
public:
                    ScUndoOutlineBlock( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                            sal_Bool bNewShow );
    virtual         ~ScUndoOutlineBlock();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
    sal_Bool            bShow;
};


class ScUndoRemoveAllOutlines: public ScSimpleUndo
{
public:
                    ScUndoRemoveAllOutlines( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab );
    virtual         ~ScUndoRemoveAllOutlines();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

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
                    ScUndoAutoOutline( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab );
    virtual         ~ScUndoAutoOutline();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

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
                    ScUndoSubTotals( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScSubTotalParam& rNewParam, SCROW nNewEndY,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
//                          ScDBData* pNewData,
                            ScRangeName* pNewUndoRange, ScDBCollection* pNewUndoDB );
    virtual         ~ScUndoSubTotals();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

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
                    ScUndoSort( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScSortParam& rParam,
                            sal_Bool bQuery, ScDocument* pNewUndoDoc,
                            ScDBCollection* pNewUndoDB, const ScRange* pDest = NULL );
    virtual         ~ScUndoSort();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB           nTab;
    ScSortParam     aSortParam;
    sal_Bool            bRepeatQuery;
    ScDocument*     pUndoDoc;
    ScDBCollection* pUndoDB;                // wegen Quell- und Zielbereich
    sal_Bool            bDestArea;
    ScRange         aDestRange;
};


class ScUndoQuery: public ScDBFuncUndo
{
public:
                    ScUndoQuery( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScQueryParam& rParam, ScDocument* pNewUndoDoc,
                            ScDBCollection* pNewUndoDB, const ScRange* pOld,
                            sal_Bool bSize, const ScRange* pAdvSrc );
    virtual         ~ScUndoQuery();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SdrUndoAction*  pDrawUndo;
    SCTAB           nTab;
    ScQueryParam    aQueryParam;
    ScDocument*     pUndoDoc;
    ScDBCollection* pUndoDB;                // wegen Quell- und Zielbereich
    ScRange         aOldDest;
    ScRange         aAdvSource;
    sal_Bool            bIsAdvanced;
    sal_Bool            bDestArea;
    sal_Bool            bDoSize;
};


class ScUndoAutoFilter: public ScDBFuncUndo
{
private:
    String          aDBName;
    sal_Bool            bFilterSet;

    void            DoChange( sal_Bool bUndo );

public:
                    ScUndoAutoFilter( ScDocShell* pNewDocShell, const ScRange& rRange,
                            const String& rName, sal_Bool bSet );
    virtual         ~ScUndoAutoFilter();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;
};


class ScUndoDBData: public ScSimpleUndo
{
public:
                    ScUndoDBData( ScDocShell* pNewDocShell,
                            ScDBCollection* pNewUndoColl, ScDBCollection* pNewRedoColl );
    virtual         ~ScUndoDBData();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDBCollection* pUndoColl;
    ScDBCollection* pRedoColl;
};


class ScUndoImportData: public ScSimpleUndo
{
public:
                    ScUndoImportData( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScImportParam& rParam, SCCOL nNewEndX, SCROW nNewEndY,
                            SCCOL nNewFormula,
                            ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                            ScDBData* pNewUndoData, ScDBData* pNewRedoData );
    virtual         ~ScUndoImportData();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

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
    sal_Bool            bRedoFilled;
};


class ScUndoRepeatDB: public ScSimpleUndo
{
public:
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
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

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
    sal_Bool            bQuerySize;
};

//UNUSED2008-05  class ScUndoPivot: public ScSimpleUndo
//UNUSED2008-05  {
//UNUSED2008-05  public:
//UNUSED2008-05                      ScUndoPivot( ScDocShell* pNewDocShell,
//UNUSED2008-05                              const ScArea& rOld, const ScArea& rNew,
//UNUSED2008-05                              ScDocument* pOldDoc, ScDocument* pNewDoc,
//UNUSED2008-05                              const ScPivot* pOldPivot, const ScPivot* pNewPivot );
//UNUSED2008-05      virtual         ~ScUndoPivot();
//UNUSED2008-05
//UNUSED2008-05      virtual void    Undo();
//UNUSED2008-05      virtual void    Redo();
//UNUSED2008-05      virtual void    Repeat(SfxRepeatTarget& rTarget);
//UNUSED2008-05      virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;
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
                    ScUndoDataPilot( ScDocShell* pNewDocShell,
                            ScDocument* pOldDoc, ScDocument* pNewDoc,
                            const ScDPObject* pOldObj, const ScDPObject* pNewObj,
                            sal_Bool bMove );
    virtual         ~ScUndoDataPilot();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDocument*     pOldUndoDoc;
    ScDocument*     pNewUndoDoc;
    ScDPObject*     pOldDPObject;
    ScDPObject*     pNewDPObject;
    sal_Bool            bAllowMove;
};


class ScUndoConsolidate: public ScSimpleUndo
{
public:
                        ScUndoConsolidate( ScDocShell* pNewDocShell,
                                const ScArea& rArea, const ScConsolidateParam& rPar,
                                ScDocument* pNewUndoDoc, sal_Bool bReference,
                                SCROW nInsCount, ScOutlineTable* pTab,
                                ScDBData* pData );
    virtual             ~ScUndoConsolidate();

    virtual void        Undo();
    virtual void        Redo();
    virtual void        Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool        CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String      GetComment() const;

private:
    ScArea              aDestArea;
    ScDocument*         pUndoDoc;
    ScConsolidateParam  aParam;
    sal_Bool                bInsRef;
    SCSIZE              nInsertCount;
    ScOutlineTable*     pUndoTab;
    ScDBData*           pUndoData;
};


class ScUndoChartData: public ScSimpleUndo
{
public:
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const String& rName, const ScRange& rNew,
                                sal_Bool bColHdr, sal_Bool bRowHdr, sal_Bool bAdd );
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const String& rName, const ScRangeListRef& rNew,
                                sal_Bool bColHdr, sal_Bool bRowHdr, sal_Bool bAdd );
    virtual             ~ScUndoChartData();

    virtual void        Undo();
    virtual void        Redo();
    virtual void        Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool        CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String      GetComment() const;

private:
    String              aChartName;
    ScRangeListRef      aOldRangeListRef;
    sal_Bool                bOldColHeaders;
    sal_Bool                bOldRowHeaders;
//  ScRange             aNewRange;
    ScRangeListRef      aNewRangeListRef;
    sal_Bool                bNewColHeaders;
    sal_Bool                bNewRowHeaders;
    sal_Bool                bAddRange;

    void                Init();
};




#endif

