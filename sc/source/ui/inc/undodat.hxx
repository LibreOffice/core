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
#include <rangeutl.hxx>
#include <rangelst.hxx>
#include <sortparam.hxx>
#include <queryparam.hxx>
#include <subtotalparam.hxx>
#include <pivot.hxx>

#include <memory>

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
                    ScUndoDoOutline( ScDocShell* pNewDocShell,
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
    SCCOLROW const        nStart;
    SCCOLROW const        nEnd;
    SCTAB const           nTab;
    ScDocumentUniquePtr
                          pUndoDoc;
    bool const            bColumns;
    sal_uInt16 const      nLevel;
    sal_uInt16 const      nEntry;
    bool const            bShow;
};

class ScUndoMakeOutline: public ScSimpleUndo
{
public:
                    ScUndoMakeOutline( ScDocShell* pNewDocShell,
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
    ScAddress const       aBlockStart;
    ScAddress const       aBlockEnd;
    std::unique_ptr<ScOutlineTable>
                          pUndoTable;
    bool const            bColumns;
    bool const            bMake;
};

class ScUndoOutlineLevel: public ScSimpleUndo
{
public:
    ScUndoOutlineLevel(ScDocShell* pNewDocShell,
                       SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                       ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab,
                       bool bNewColumns, sal_uInt16 nNewLevel);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCCOLROW const        nStart;
    SCCOLROW const        nEnd;
    SCTAB const           nTab;
    ScDocumentUniquePtr   xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
    bool const            bColumns;
    sal_uInt16 const      nLevel;
};

class ScUndoOutlineBlock: public ScSimpleUndo
{
public:
    ScUndoOutlineBlock(ScDocShell* pNewDocShell,
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
    ScAddress const       aBlockStart;
    ScAddress const       aBlockEnd;
    ScDocumentUniquePtr   xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
    bool const            bShow;
};

class ScUndoRemoveAllOutlines: public ScSimpleUndo
{
public:
    ScUndoRemoveAllOutlines(ScDocShell* pNewDocShell,
            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
            ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress const       aBlockStart;
    ScAddress const       aBlockEnd;
    ScDocumentUniquePtr   xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
};

class ScUndoAutoOutline: public ScSimpleUndo
{
public:
    ScUndoAutoOutline(ScDocShell* pNewDocShell,
                      SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                      SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                      ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress const       aBlockStart;
    ScAddress const       aBlockEnd;
    ScDocumentUniquePtr   xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
};

class ScUndoSubTotals: public ScDBFuncUndo
{
public:
    ScUndoSubTotals(ScDocShell* pNewDocShell, SCTAB nNewTab,
                    const ScSubTotalParam& rNewParam, SCROW nNewEndY,
                    ScDocumentUniquePtr pNewUndoDoc, std::unique_ptr<ScOutlineTable> pNewUndoTab,
                    std::unique_ptr<ScRangeName> pNewUndoRange, std::unique_ptr<ScDBCollection> pNewUndoDB);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB const           nTab;
    ScSubTotalParam const aParam;                         // The original passed parameter
    SCROW const           nNewEndRow;                     // Size of result
    ScDocumentUniquePtr   xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
    std::unique_ptr<ScRangeName> xUndoRange;
    std::unique_ptr<ScDBCollection> xUndoDB;
};

class ScUndoQuery: public ScDBFuncUndo
{
public:
                    ScUndoQuery( ScDocShell* pNewDocShell, SCTAB nNewTab,
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
    SCTAB const           nTab;
    ScQueryParam const    aQueryParam;
    ScDocumentUniquePtr   xUndoDoc;
    std::unique_ptr<ScDBCollection> xUndoDB;                // due to source and target range
    ScRange               aOldDest;
    ScRange               aAdvSource;
    bool                  bIsAdvanced;
    bool                  bDestArea;
    bool const            bDoSize;
};

class ScUndoAutoFilter: public ScDBFuncUndo
{
private:
    OUString const  aDBName;
    bool const      bFilterSet;

    void            DoChange( bool bUndo );

public:
                    ScUndoAutoFilter( ScDocShell* pNewDocShell, const ScRange& rRange,
                                      const OUString& rName, bool bSet );
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
                    ScUndoDBData( ScDocShell* pNewDocShell,
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
    ScUndoImportData(ScDocShell* pNewDocShell, SCTAB nNewTab,
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
    SCTAB const           nTab;
    ScImportParam const   aImportParam;
    SCCOL const           nEndCol;
    SCROW const           nEndRow;
    ScDocumentUniquePtr   xUndoDoc;
    ScDocumentUniquePtr   xRedoDoc;
    std::unique_ptr<ScDBData> xUndoDBData;
    std::unique_ptr<ScDBData> xRedoDBData;
    SCCOL const           nFormulaCols;
    bool                  bRedoFilled;
};

class ScUndoRepeatDB: public ScSimpleUndo
{
public:
    ScUndoRepeatDB(ScDocShell* pNewDocShell, SCTAB nNewTab,
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
    ScAddress const       aBlockStart;
    ScAddress const       aBlockEnd;
    SCROW const           nNewEndRow;
    ScAddress const       aCursorPos;
    ScDocumentUniquePtr   xUndoDoc;
    std::unique_ptr<ScOutlineTable> xUndoTable;
    std::unique_ptr<ScRangeName> xUndoRange;
    std::unique_ptr<ScDBCollection> xUndoDB;
    ScRange               aOldQuery;
    ScRange               aNewQuery;
    bool                  bQuerySize;
};

class ScUndoDataPilot: public ScSimpleUndo
{
public:
    ScUndoDataPilot(ScDocShell* pNewDocShell,
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
    bool const            bAllowMove;
};

class ScUndoConsolidate: public ScSimpleUndo
{
public:
    ScUndoConsolidate(ScDocShell* pNewDocShell,
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
    ScArea const              aDestArea;
    ScDocumentUniquePtr       xUndoDoc;
    ScConsolidateParam const  aParam;
    bool const                bInsRef;
    SCSIZE const              nInsertCount;
    std::unique_ptr<ScOutlineTable> xUndoTab;
    std::unique_ptr<ScDBData> xUndoData;
};

class ScUndoChartData: public ScSimpleUndo
{
public:
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const OUString& rName, const ScRange& rNew,
                                bool bColHdr, bool bRowHdr, bool bAdd );
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const OUString& rName, const ScRangeListRef& rNew,
                                bool bColHdr, bool bRowHdr, bool bAdd );
    virtual             ~ScUndoChartData() override;

    virtual void        Undo() override;
    virtual void        Redo() override;
    virtual void        Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool        CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    OUString const      aChartName;
    ScRangeListRef      aOldRangeListRef;
    bool                bOldColHeaders;
    bool                bOldRowHeaders;
//  ScRange             aNewRange;
    ScRangeListRef      aNewRangeListRef;
    bool const          bNewColHeaders;
    bool const          bNewRowHeaders;
    bool const          bAddRange;

    void                Init();
};

class ScUndoDataForm: public ScBlockUndo
{
public:
    ScUndoDataForm(ScDocShell* pNewDocShell,
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
    ScDocumentUniquePtr const xUndoDoc;
    ScDocumentUniquePtr xRedoDoc;
    std::unique_ptr<ScRefUndoData> xRefUndoData;
    std::unique_ptr<ScRefUndoData> xRefRedoData;
    bool            bRedoFilled;

    void            DoChange( const bool bUndo );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
