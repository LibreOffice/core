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

#ifndef INCLUDED_SC_SOURCE_UI_INC_UNDODAT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_UNDODAT_HXX

#include "undobase.hxx"
#include "rangeutl.hxx"
#include "rangelst.hxx"
#include "sortparam.hxx"
#include "queryparam.hxx"
#include "subtotalparam.hxx"
#include "pivot.hxx"

#include <boost/scoped_ptr.hpp>

class ScDocShell;
class ScDocument;
class ScOutlineTable;
class ScRangeName;
class ScDBData;
class ScDBCollection;
class ScDPObject;
class SdrUndoAction;
class ScMarkData;

class ScUndoDoOutline: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoDoOutline( ScDocShell* pNewDocShell,
                            SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                            ScDocument* pNewUndoDoc, bool bNewColumns,
                            sal_uInt16 nNewLevel, sal_uInt16 nNewEntry, bool bNewShow );
    virtual         ~ScUndoDoOutline();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nTab;
    ScDocument*     pUndoDoc;
    bool            bColumns;
    sal_uInt16      nLevel;
    sal_uInt16      nEntry;
    bool            bShow;
};

class ScUndoMakeOutline: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoMakeOutline( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScOutlineTable* pNewUndoTab,
                            bool bNewColumns, bool bNewMake );
    virtual         ~ScUndoMakeOutline();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScOutlineTable* pUndoTable;
    bool            bColumns;
    bool            bMake;
};

class ScUndoOutlineLevel: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoOutlineLevel( ScDocShell* pNewDocShell,
                            SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                            bool bNewColumns, sal_uInt16 nNewLevel );
    virtual         ~ScUndoOutlineLevel();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nTab;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
    bool            bColumns;
    sal_uInt16      nLevel;
};

class ScUndoOutlineBlock: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoOutlineBlock( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                            bool bNewShow );
    virtual         ~ScUndoOutlineBlock();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
    bool            bShow;
};

class ScUndoRemoveAllOutlines: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoRemoveAllOutlines( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab );
    virtual         ~ScUndoRemoveAllOutlines();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
};

class ScUndoAutoOutline: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoAutoOutline( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab );
    virtual         ~ScUndoAutoOutline();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
};

class ScUndoSubTotals: public ScDBFuncUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoSubTotals( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScSubTotalParam& rNewParam, SCROW nNewEndY,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
//                          ScDBData* pNewData,
                            ScRangeName* pNewUndoRange, ScDBCollection* pNewUndoDB );
    virtual         ~ScUndoSubTotals();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    SCTAB           nTab;
    ScSubTotalParam aParam;                         // The original passed parameter
    SCROW           nNewEndRow;                     // Size of result
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTable;
//  ScDBData*       pUndoDBData;
    ScRangeName*    pUndoRange;
    ScDBCollection* pUndoDB;
};

class ScUndoQuery: public ScDBFuncUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoQuery( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScQueryParam& rParam, ScDocument* pNewUndoDoc,
                            ScDBCollection* pNewUndoDB, const ScRange* pOld,
                            bool bSize, const ScRange* pAdvSrc );
    virtual         ~ScUndoQuery();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    SdrUndoAction*  pDrawUndo;
    SCTAB           nTab;
    ScQueryParam    aQueryParam;
    ScDocument*     pUndoDoc;
    ScDBCollection* pUndoDB;                // due to source and target range
    ScRange         aOldDest;
    ScRange         aAdvSource;
    bool            bIsAdvanced;
    bool            bDestArea;
    bool            bDoSize;
};

class ScUndoAutoFilter: public ScDBFuncUndo
{
private:
    OUString aDBName;
    bool            bFilterSet;

    void            DoChange( bool bUndo );

public:
                    TYPEINFO_OVERRIDE();
                    ScUndoAutoFilter( ScDocShell* pNewDocShell, const ScRange& rRange,
                                      const OUString& rName, bool bSet );
    virtual         ~ScUndoAutoFilter();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;
};

class ScUndoDBData: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoDBData( ScDocShell* pNewDocShell,
                            ScDBCollection* pNewUndoColl, ScDBCollection* pNewRedoColl );
    virtual         ~ScUndoDBData();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    ScDBCollection* pUndoColl;
    ScDBCollection* pRedoColl;
};

class ScUndoImportData: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoImportData( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            const ScImportParam& rParam, SCCOL nNewEndX, SCROW nNewEndY,
                            SCCOL nNewFormula,
                            ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                            ScDBData* pNewUndoData, ScDBData* pNewRedoData );
    virtual         ~ScUndoImportData();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

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
    bool            bRedoFilled;
};

class ScUndoRepeatDB: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoRepeatDB( ScDocShell* pNewDocShell, SCTAB nNewTab,
                            SCCOL nStartX, SCROW nStartY, SCCOL nEndX, SCROW nEndY,
                            SCROW nResultEndRow, SCCOL nCurX, SCROW nCurY,
                            ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                            ScRangeName* pNewUndoRange, ScDBCollection* pNewUndoDB,
                            const ScRange* pOldQ, const ScRange* pNewQ );
    virtual         ~ScUndoRepeatDB();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

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
    bool            bQuerySize;
};

class ScUndoDataPilot: public ScSimpleUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoDataPilot( ScDocShell* pNewDocShell,
                            ScDocument* pOldDoc, ScDocument* pNewDoc,
                            const ScDPObject* pOldObj, const ScDPObject* pNewObj,
                            bool bMove );
    virtual         ~ScUndoDataPilot();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    ScDocument*     pOldUndoDoc;
    ScDocument*     pNewUndoDoc;
    ScDPObject*     pOldDPObject;
    ScDPObject*     pNewDPObject;
    bool            bAllowMove;
};

class ScUndoConsolidate: public ScSimpleUndo
{
public:
                        TYPEINFO_OVERRIDE();
                        ScUndoConsolidate( ScDocShell* pNewDocShell,
                                const ScArea& rArea, const ScConsolidateParam& rPar,
                                ScDocument* pNewUndoDoc, bool bReference,
                                SCROW nInsCount, ScOutlineTable* pTab,
                                ScDBData* pData );
    virtual             ~ScUndoConsolidate();

    virtual void        Undo() SAL_OVERRIDE;
    virtual void        Redo() SAL_OVERRIDE;
    virtual void        Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool        CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    ScArea              aDestArea;
    ScDocument*         pUndoDoc;
    ScConsolidateParam  aParam;
    bool                bInsRef;
    SCSIZE              nInsertCount;
    ScOutlineTable*     pUndoTab;
    ScDBData*           pUndoData;
};

class ScUndoChartData: public ScSimpleUndo
{
public:
                        TYPEINFO_OVERRIDE();
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const OUString& rName, const ScRange& rNew,
                                bool bColHdr, bool bRowHdr, bool bAdd );
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const OUString& rName, const ScRangeListRef& rNew,
                                bool bColHdr, bool bRowHdr, bool bAdd );
    virtual             ~ScUndoChartData();

    virtual void        Undo() SAL_OVERRIDE;
    virtual void        Redo() SAL_OVERRIDE;
    virtual void        Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool        CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    OUString            aChartName;
    ScRangeListRef      aOldRangeListRef;
    bool                bOldColHeaders;
    bool                bOldRowHeaders;
//  ScRange             aNewRange;
    ScRangeListRef      aNewRangeListRef;
    bool                bNewColHeaders;
    bool                bNewRowHeaders;
    bool                bAddRange;

    void                Init();
};

class ScUndoDataForm: public ScBlockUndo
{
public:
                    TYPEINFO_OVERRIDE();
                    ScUndoDataForm( ScDocShell* pNewDocShell,
                                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                                const ScMarkData& rMark,
                                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                                InsertDeleteFlags nNewFlags,
                                ScRefUndoData* pRefData, void* pFill1, void* pFill2, void* pFill3,
                                bool bRedoIsFilled = true
                                 );
    virtual     ~ScUndoDataForm();

    virtual void    Undo() SAL_OVERRIDE;
    virtual void    Redo() SAL_OVERRIDE;
    virtual void    Repeat(SfxRepeatTarget& rTarget) SAL_OVERRIDE;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    boost::scoped_ptr<ScMarkData> mpMarkData;
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
    InsertDeleteFlags nFlags;
    ScRefUndoData*  pRefUndoData;
    ScRefUndoData*  pRefRedoData;
    sal_uLong       nStartChangeAction;
    sal_uLong       nEndChangeAction;
    bool            bRedoFilled;

    void            DoChange( const bool bUndo );
    void            SetChangeTrack();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
