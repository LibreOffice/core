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

#pragma once

#include "undobase.hxx"
#include "refundo.hxx"
#include <dpobject.hxx>
#include <olinetab.hxx>
#include <rangeutl.hxx>
#include <rangelst.hxx>
#include <queryparam.hxx>
#include <subtotalparam.hxx>

#include <memory>

class ScDocShell;
class SdrUndoAction;

class ScUndoDoOutline: public ScSimpleUndo
{
public:
                    ScUndoDoOutline( ScDocShell& rNewDocShell,
                            SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                            ScDocumentUniquePtr pNewUndoDoc, bool bNewColumns,
                            sal_uInt16 nNewLevel, sal_uInt16 nNewEntry, bool bNewShow );
    virtual         ~ScUndoDoOutline() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nTab;
    ScDocumentUniquePtr
                    pUndoDoc;
    bool            bColumns;
    sal_uInt16      nLevel;
    sal_uInt16      nEntry;
    bool            bShow;
};

class ScUndoMakeOutline: public ScSimpleUndo
{
public:
                    ScUndoMakeOutline( ScDocShell& rNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            std::unique_ptr<ScOutlineTable> pNewUndoTab,
                            bool bNewColumns, bool bNewMake );
    virtual         ~ScUndoMakeOutline() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    std::unique_ptr<ScOutlineTable>
                    pUndoTable;
    bool            bColumns;
    bool            bMake;
};

class ScUndoOutlineLevel: public ScSimpleUndo
{
public:
    ScUndoOutlineLevel(ScDocShell& rNewDocShell,
                       SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                       ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab,
                       bool bNewColumns, sal_uInt16 nNewLevel);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nTab;
    ScDocumentUniquePtr xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
    bool            bColumns;
    sal_uInt16      nLevel;
};

class ScUndoOutlineBlock: public ScSimpleUndo
{
public:
    ScUndoOutlineBlock(ScDocShell& rNewDocShell,
                       SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                       SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                       ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab,
                       bool bNewShow);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocumentUniquePtr xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
    bool            bShow;
};

class ScUndoRemoveAllOutlines: public ScSimpleUndo
{
public:
    ScUndoRemoveAllOutlines(ScDocShell& rNewDocShell,
            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
            ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocumentUniquePtr xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
};

class ScUndoAutoOutline: public ScSimpleUndo
{
public:
    ScUndoAutoOutline(ScDocShell& rNewDocShell,
                      SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                      SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                      ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    ScDocumentUniquePtr xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
};

class ScUndoSubTotals: public ScDBFuncUndo
{
public:
    ScUndoSubTotals(ScDocShell& rNewDocShell, SCTAB nNewTab,
                    const ScSubTotalParam& rNewParam, SCROW nNewEndY,
                    ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab,
                    std::unique_ptr<ScRangeName> pNewUndoRange, std::unique_ptr<ScDBCollection> pNewUndoDB);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB           nTab;
    ScSubTotalParam aParam;                         // The original passed parameter
    SCROW           nNewEndRow;                     // Size of result
    ScDocumentUniquePtr xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
    std::unique_ptr<ScRangeName> xUndoRange;
    std::unique_ptr<ScDBCollection> xUndoDB;
};

class ScUndoQuery: public ScDBFuncUndo
{
public:
                    ScUndoQuery( ScDocShell& rNewDocShell, SCTAB nNewTab,
                            const ScQueryParam& rParam, ScDocumentUniquePtr pNewUndoDoc,
                            std::unique_ptr<ScDBCollection> pNewUndoDB, const ScRange* pOld,
                            bool bSize, const ScRange* pAdvSrc );
    virtual         ~ScUndoQuery() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    std::unique_ptr<SdrUndoAction> pDrawUndo;
    SCTAB           nTab;
    ScQueryParam    aQueryParam;
    ScDocumentUniquePtr xUndoDoc;
    std::unique_ptr<ScDBCollection> xUndoDB;                // due to source and target range
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
                    ScUndoAutoFilter( ScDocShell& rNewDocShell, const ScRange& rRange,
                                      OUString aName, bool bSet );
    virtual         ~ScUndoAutoFilter() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;
};

class ScUndoDBData: public ScSimpleUndo
{
public:
                    ScUndoDBData( ScDocShell& rNewDocShell,
                            std::unique_ptr<ScDBCollection> pNewUndoColl,
                            std::unique_ptr<ScDBCollection> pNewRedoColl );
    virtual         ~ScUndoDBData() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    std::unique_ptr<ScDBCollection> pUndoColl;
    std::unique_ptr<ScDBCollection> pRedoColl;
};

class ScUndoImportData: public ScSimpleUndo
{
public:
    ScUndoImportData(ScDocShell& rNewDocShell, SCTAB nNewTab,
                     const ScImportParam& rParam, SCCOL nNewEndX, SCROW nNewEndY,
                     SCCOL nNewFormula,
                     ScDocumentUniquePtr pNewUndoDoc, ScDocumentUniquePtr pNewRedoDoc,
                     std::unique_ptr<ScDBData> pNewUndoData, std::unique_ptr<ScDBData> pNewRedoData);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB           nTab;
    ScImportParam   aImportParam;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    ScDocumentUniquePtr xUndoDoc;
    ScDocumentUniquePtr xRedoDoc;
    std::unique_ptr<ScDBData> xUndoDBData;
    std::unique_ptr<ScDBData> xRedoDBData;
    SCCOL           nFormulaCols;
    bool            bRedoFilled;
};

class ScUndoRepeatDB: public ScSimpleUndo
{
public:
    ScUndoRepeatDB(ScDocShell& rNewDocShell, SCTAB nNewTab,
                   SCCOL nStartX, SCROW nStartY, SCCOL nEndX, SCROW nEndY,
                   SCROW nResultEndRow, SCCOL nCurX, SCROW nCurY,
                   ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab,
                   std::unique_ptr<ScRangeName> pNewUndoRange, std::unique_ptr<ScDBCollection> pNewUndoDB,
                   const ScRange* pOldQ, const ScRange* pNewQ);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress       aBlockStart;
    ScAddress       aBlockEnd;
    SCROW           nNewEndRow;
    ScAddress       aCursorPos;
    ScDocumentUniquePtr xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
    std::unique_ptr<ScRangeName> xUndoRange;
    std::unique_ptr<ScDBCollection> xUndoDB;
    ScRange         aOldQuery;
    ScRange         aNewQuery;
    bool            bQuerySize;
};

class ScUndoDataPilot: public ScSimpleUndo
{
public:
    ScUndoDataPilot(ScDocShell& rNewDocShell,
                    ScDocumentUniquePtr pOldDoc, ScDocumentUniquePtr pNewDoc,
                    const ScDPObject* pOldObj, const ScDPObject* pNewObj,
                    bool bMove);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocumentUniquePtr xOldUndoDoc;
    ScDocumentUniquePtr xNewUndoDoc;
    std::unique_ptr<ScDPObject> xOldDPObject;
    std::unique_ptr<ScDPObject> xNewDPObject;
    bool            bAllowMove;
};

class ScUndoConsolidate: public ScSimpleUndo
{
public:
    ScUndoConsolidate(ScDocShell& rNewDocShell,
                      const ScArea& rArea, const ScConsolidateParam& rPar,
                      ScDocumentUniquePtr pNewUndoDoc, bool bReference,
                      SCROW nInsCount, std::unique_ptr<ScOutlineTable> pTab,
                      std::unique_ptr<ScDBData> pData);

    virtual void        Undo() override;
    virtual void        Redo() override;
    virtual void        Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool        CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScArea              aDestArea;
    ScDocumentUniquePtr xUndoDoc;
    ScConsolidateParam  aParam;
    bool                bInsRef;
    SCSIZE              nInsertCount;
    std::unique_ptr<ScOutlineTable> xUndoTab;
    std::unique_ptr<ScDBData> xUndoData;
};

class ScUndoChartData: public ScSimpleUndo
{
public:
                        ScUndoChartData( ScDocShell& rNewDocShell,
                                OUString aName, const ScRange& rNew,
                                bool bColHdr, bool bRowHdr, bool bAdd );
                        ScUndoChartData( ScDocShell& rNewDocShell,
                                OUString aName, ScRangeListRef xNew,
                                bool bColHdr, bool bRowHdr, bool bAdd );
    virtual             ~ScUndoChartData() override;

    virtual void        Undo() override;
    virtual void        Redo() override;
    virtual void        Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool        CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

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
    ScUndoDataForm(ScDocShell& rNewDocShell,
                   SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                   SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                   const ScMarkData& rMark,
                   ScDocumentUniquePtr pNewUndoDoc, ScDocumentUniquePtr pNewRedoDoc,
                   std::unique_ptr<ScRefUndoData> pRefData);
    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    std::unique_ptr<ScMarkData> mxMarkData;
    ScDocumentUniquePtr xUndoDoc;
    ScDocumentUniquePtr xRedoDoc;
    std::unique_ptr<ScRefUndoData> xRefUndoData;
    std::unique_ptr<ScRefUndoData> xRefRedoData;
    bool            bRedoFilled;

    void            DoChange( const bool bUndo );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
