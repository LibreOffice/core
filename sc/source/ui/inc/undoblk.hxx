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
#include <markdata.hxx>
#include "spellparam.hxx"
#include "cellmergeoption.hxx"
#include <paramisc.hxx>
#include <editeng/boxitem.hxx>

#include <memory>

class ScDocShell;
class ScOutlineTable;
class ScPatternAttr;
class SvxSearchItem;
class SdrUndoAction;
class ScEditDataArray;

class ScUndoInsertCells: public ScMoveUndo
{
public:
                    ScUndoInsertCells( ScDocShell* pNewDocShell,
                                       const ScRange& rRange,
                                       SCTAB nNewCount, std::unique_ptr<SCTAB[]> pNewTabs, std::unique_ptr<SCTAB[]> pNewScenarios,
                                       InsCellCmd eNewCmd, ScDocumentUniquePtr pUndoDocument, std::unique_ptr<ScRefUndoData> pRefData,
                                       bool bNewPartOfPaste );
    virtual         ~ScUndoInsertCells() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat( SfxRepeatTarget& rTarget ) override;
    virtual bool    CanRepeat( SfxRepeatTarget& rTarget ) const override;

    virtual OUString GetComment() const override;

    virtual bool    Merge( SfxUndoAction *pNextAction ) override;

private:
    ScRange         aEffRange;
    SCTAB           nCount;
    std::unique_ptr<SCTAB[]>
                    pTabs;
    std::unique_ptr<SCTAB[]>
                    pScenarios;
    sal_uLong       nEndChangeAction;
    InsCellCmd      eCmd;
    bool            bPartOfPaste;
    std::unique_ptr<SfxUndoAction>
                    pPasteUndo;

    void            DoChange ( const bool bUndo );
    void            SetChangeTrack();
};

class ScUndoDeleteCells: public ScMoveUndo
{
public:
                    ScUndoDeleteCells( ScDocShell* pNewDocShell,
                                       const ScRange& rRange,
                                       SCTAB nNewCount, std::unique_ptr<SCTAB[]> pNewTabs, std::unique_ptr<SCTAB[]> pNewScenarios,
                                       DelCellCmd eNewCmd, ScDocumentUniquePtr pUndoDocument, std::unique_ptr<ScRefUndoData> pRefData );
    virtual         ~ScUndoDeleteCells() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScRange         aEffRange;
    SCTAB           nCount;
    std::unique_ptr<SCTAB[]>
                    pTabs;
    std::unique_ptr<SCTAB[]>
                    pScenarios;
    sal_uLong       nStartChangeAction;
    sal_uLong       nEndChangeAction;
    DelCellCmd      eCmd;

    void            DoChange ( const bool bUndo );
    void            SetChangeTrack();
};

class ScUndoDeleteMulti: public ScMoveUndo
{
public:

    ScUndoDeleteMulti( ScDocShell* pNewDocShell,
                       bool bNewRows, bool bNeedsRefresh, SCTAB nNewTab,
                       std::vector<sc::ColRowSpan>&& rSpans,
                       ScDocumentUniquePtr pUndoDocument, std::unique_ptr<ScRefUndoData> pRefData );

    virtual         ~ScUndoDeleteMulti() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    bool            mbRows:1;
    bool            mbRefresh:1;
    SCTAB           nTab;
    std::vector<sc::ColRowSpan> maSpans;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            DoChange() const;
    void            SetChangeTrack();
};

class SC_DLLPUBLIC ScUndoCut: public ScBlockUndo
{
public:
    ScUndoCut(ScDocShell* pNewDocShell,
              const ScRange& aRange, // adjusted for merged cells
              const ScAddress& aOldEnd, // end position without adjustment
              const ScMarkData& rMark, // selected sheets
              ScDocumentUniquePtr pNewUndoDoc);
    virtual         ~ScUndoCut() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData          aMarkData;
    ScDocumentUniquePtr
                        pUndoDoc;
    ScRange             aExtendedRange;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            DoChange( const bool bUndo );
    void            SetChangeTrack();
};

struct ScUndoPasteOptions
{
    ScPasteFunc  nFunction;
    bool       bSkipEmptyCells;
    bool       bTranspose;
    bool       bAsLink;
    InsCellCmd eMoveMode;

    ScUndoPasteOptions() :
        nFunction( ScPasteFunc::NONE ),
        bSkipEmptyCells( false ),
        bTranspose( false ),
        bAsLink( false ),
        eMoveMode( INS_NONE )
    {}
};

class SC_DLLPUBLIC ScUndoPaste: public ScMultiBlockUndo
{
public:
    ScUndoPaste(ScDocShell* pNewDocShell, const ScRangeList& rRanges,
                const ScMarkData& rMark,
                ScDocumentUniquePtr pNewUndoDoc, ScDocumentUniquePtr pNewRedoDoc,
                InsertDeleteFlags nNewFlags,
                std::unique_ptr<ScRefUndoData> pRefData,
                bool bRedoIsFilled = true,
                const ScUndoPasteOptions* pOptions = nullptr);
    virtual ~ScUndoPaste() override;

    virtual void Undo() override;
    virtual void Redo() override;
    virtual void Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocumentUniquePtr pUndoDoc;
    ScDocumentUniquePtr pRedoDoc;
    InsertDeleteFlags nFlags;
    std::unique_ptr<ScRefUndoData> pRefUndoData;
    std::unique_ptr<ScRefUndoData> pRefRedoData;
    sal_uLong       nStartChangeAction;
    sal_uLong       nEndChangeAction;
    bool            bRedoFilled;
    ScUndoPasteOptions aPasteOptions;

    void DoChange(bool bUndo);
    void SetChangeTrack();
};

class ScUndoDragDrop: public ScMoveUndo
{
public:
                    ScUndoDragDrop( ScDocShell* pNewDocShell,
                                    const ScRange& rRange, const ScAddress& aNewDestPos, bool bNewCut,
                                    ScDocumentUniquePtr pUndoDocument,
                                    bool bScenario );
    virtual         ~ScUndoDragDrop() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    sal_uInt16 mnPaintExtFlags;
    ScRangeList maPaintRanges;

    ScRange         aSrcRange;
    ScRange         aDestRange;
    sal_uLong       nStartChangeAction;
    sal_uLong       nEndChangeAction;
    bool            bCut;
    bool            bKeepScenarioFlags;

    void            PaintArea( ScRange aRange, sal_uInt16 nExtFlags ) const;
    void DoUndo( ScRange aRange );

    void            SetChangeTrack();
};

class ScUndoDeleteContents: public ScSimpleUndo
{
public:
                    ScUndoDeleteContents( ScDocShell* pNewDocShell,
                                          const ScMarkData& rMark,
                                          const ScRange& rRange,
                                          ScDocumentUniquePtr&& pNewUndoDoc, bool bNewMulti,
                                          InsertDeleteFlags nNewFlags, bool bObjects );
    virtual         ~ScUndoDeleteContents() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

    void SetDataSpans( const std::shared_ptr<DataSpansType>& pSpans );

private:
    std::shared_ptr<DataSpansType> mpDataSpans; // Spans of non-empty cells.

    ScRange         aRange;
    ScMarkData      aMarkData;
    ScDocumentUniquePtr pUndoDoc; // Block mark and deleted data
    std::unique_ptr<SdrUndoAction> pDrawUndo;      // Deleted objects
    sal_uLong       nStartChangeAction;
    sal_uLong       nEndChangeAction;
    InsertDeleteFlags nFlags;
    bool            bMulti;         // Multi selection

    void            DoChange( const bool bUndo );
    void            SetChangeTrack();
};

class ScUndoFillTable: public ScSimpleUndo
{
public:
                    ScUndoFillTable( ScDocShell* pNewDocShell,
                                     const ScMarkData& rMark,
                                     SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                     SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                                     ScDocumentUniquePtr pNewUndoDoc, bool bNewMulti, SCTAB nSrc,
                                     InsertDeleteFlags nFlg, ScPasteFunc nFunc, bool bSkip, bool bLink );
    virtual         ~ScUndoFillTable() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScRange         aRange;
    ScMarkData      aMarkData;
    ScDocumentUniquePtr
                    pUndoDoc;       // Block mark and deleted data
    sal_uLong       nStartChangeAction;
    sal_uLong       nEndChangeAction;
    InsertDeleteFlags nFlags;
    ScPasteFunc       nFunction;
    SCTAB           nSrcTab;
    bool            bMulti;         // Multi selection
    bool            bSkipEmpty;
    bool            bAsLink;

    void            DoChange( const bool bUndo );
    void            SetChangeTrack();
};

class ScUndoSelectionAttr: public ScSimpleUndo
{
public:
                    ScUndoSelectionAttr( ScDocShell* pNewDocShell,
                                         const ScMarkData& rMark,
                                         SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                         SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                                         ScDocumentUniquePtr pNewUndoDoc, bool bNewMulti,
                                         const ScPatternAttr* pNewApply,
                                         const SvxBoxItem* pNewOuter = nullptr,
                                         const SvxBoxInfoItem* pNewInner = nullptr,
                                         const ScRange* pRangeCover = nullptr );
    virtual         ~ScUndoSelectionAttr() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

    ScEditDataArray*    GetDataArray();
private:
    ScMarkData      aMarkData;
    ScRange         aRange;
    ScRange         aRangeCover;
    std::unique_ptr<ScEditDataArray> mpDataArray;
    ScDocumentUniquePtr pUndoDoc;
    bool            bMulti;
    ScPatternAttr*  pApplyPattern;
    SvxBoxItem*     pLineOuter;
    SvxBoxInfoItem* pLineInner;

    void            DoChange( const bool bUndo );
    void            ChangeEditData( const bool bUndo );
};

class ScUndoWidthOrHeight: public ScSimpleUndo
{
public:
                    ScUndoWidthOrHeight( ScDocShell* pNewDocShell,
                                         const ScMarkData& rMark,
                                         SCCOLROW nNewStart, SCTAB nNewStartTab,
                                         SCCOLROW nNewEnd, SCTAB nNewEndTab,
                                         ScDocumentUniquePtr pNewUndoDoc,
                                        std::vector<sc::ColRowSpan>&& rRanges,
                                         std::unique_ptr<ScOutlineTable> pNewUndoTab,
                                         ScSizeMode eNewMode, sal_uInt16 nNewSizeTwips,
                                         bool bNewWidth );
    virtual         ~ScUndoWidthOrHeight() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nStartTab;
    SCTAB           nEndTab;
    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScOutlineTable> pUndoTab;
    std::vector<sc::ColRowSpan> maRanges;
    sal_uInt16      nNewSize;
    bool            bWidth;
    ScSizeMode      eMode;
    std::unique_ptr<SdrUndoAction>  pDrawUndo;
};

class ScUndoAutoFill: public ScBlockUndo
{
public:
                    ScUndoAutoFill( ScDocShell* pNewDocShell,
                                    const ScRange& rRange, const ScRange& rSourceArea,
                                    ScDocumentUniquePtr pNewUndoDoc, const ScMarkData& rMark,
                                    FillDir eNewFillDir,
                                    FillCmd eNewFillCmd, FillDateCmd eNewFillDateCmd,
                                    double fNewStartValue, double fNewStepValue, double fNewMaxValue );
    virtual         ~ScUndoAutoFill() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScRange         aSource;
    ScMarkData      aMarkData;
    ScDocumentUniquePtr
                    pUndoDoc;
    FillDir         eFillDir;
    FillCmd         eFillCmd;
    FillDateCmd     eFillDateCmd;
    double          fStartValue;
    double          fStepValue;
    double          fMaxValue;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            SetChangeTrack();
};

class ScUndoMerge: public ScSimpleUndo
{
public:
                    ScUndoMerge( ScDocShell* pNewDocShell, ScCellMergeOption aOption,
                                 bool bMergeContents, ScDocumentUniquePtr pUndoDoc,
                                 std::unique_ptr<SdrUndoAction> pDrawUndo);
    virtual         ~ScUndoMerge() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScCellMergeOption maOption;
    bool            mbMergeContents;        // Merge contents in Redo().
    ScDocumentUniquePtr mxUndoDoc;              // when data is merged
    std::unique_ptr<SdrUndoAction> mpDrawUndo;

    void            DoChange( bool bUndo ) const;
};

class ScUndoAutoFormat: public ScBlockUndo
{
public:
                    ScUndoAutoFormat( ScDocShell* pNewDocShell,
                                      const ScRange& rRange, ScDocumentUniquePtr pNewUndoDoc,
                                      const ScMarkData& rMark,
                                      bool bNewSize, sal_uInt16 nNewFormatNo );
    virtual         ~ScUndoAutoFormat() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocumentUniquePtr
                    pUndoDoc;       // deleted data
    ScMarkData      aMarkData;
    bool            bSize;
    sal_uInt16      nFormatNo;
};

class ScUndoReplace: public ScSimpleUndo
{
public:
                    ScUndoReplace( ScDocShell* pNewDocShell,
                                   const ScMarkData& rMark,
                                   SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                   OUString aNewUndoStr, ScDocumentUniquePtr pNewUndoDoc,
                                   const SvxSearchItem* pItem );
    virtual         ~ScUndoReplace() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress       aCursorPos;
    ScMarkData      aMarkData;
    OUString        aUndoStr;           // Data at single selection
    ScDocumentUniquePtr pUndoDoc;           // Block mark and deleted data
    std::unique_ptr<SvxSearchItem> pSearchItem;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

            void    SetChangeTrack();
};

class ScUndoTabOp: public ScSimpleUndo
{
public:
                    ScUndoTabOp( ScDocShell* pNewDocShell,
                                 SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                 SCCOL nEndX,   SCROW nEndY,   SCTAB nEndZ,
                                 ScDocumentUniquePtr pNewUndoDoc,
                                 const ScRefAddress& rFormulaCell,
                                 const ScRefAddress& rFormulaEnd,
                                 const ScRefAddress& rRowCell,
                                 const ScRefAddress& rColCell,
                                 ScTabOpParam::Mode eMode );
    virtual         ~ScUndoTabOp() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScRange         aRange;
    ScDocumentUniquePtr
                    pUndoDoc;       // Deleted data
    ScRefAddress    theFormulaCell;
    ScRefAddress    theFormulaEnd;
    ScRefAddress    theRowCell;
    ScRefAddress    theColCell;
    ScTabOpParam::Mode meMode;
};

class ScUndoConversion : public ScSimpleUndo
{
public:

                    ScUndoConversion( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                      SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScDocumentUniquePtr pNewUndoDoc,
                                      SCCOL nNewX, SCROW nNewY, SCTAB nNewZ, ScDocumentUniquePtr pNewRedoDoc,
                                      ScConversionParam aConvParam );
    virtual         ~ScUndoConversion() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScAddress       aCursorPos;
    ScDocumentUniquePtr pUndoDoc;           // Block mark and deleted data
    ScAddress       aNewCursorPos;
    ScDocumentUniquePtr pRedoDoc;           // Block mark and new data
    sal_uLong       nStartChangeAction;
    sal_uLong       nEndChangeAction;
    ScConversionParam maConvParam;        /// Conversion type and parameters.

    void            DoChange( ScDocument* pRefDoc, const ScAddress& rCursorPos );
    void            SetChangeTrack();
};

class ScUndoRefConversion: public ScSimpleUndo
{
public:
                    ScUndoRefConversion( ScDocShell* pNewDocShell,
                                         const ScRange& aMarkRange, const ScMarkData& rMark,
                                         ScDocumentUniquePtr pNewUndoDoc, ScDocumentUniquePtr pNewRedoDoc, bool bNewMulti);
    virtual         ~ScUndoRefConversion() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData          aMarkData;
    ScDocumentUniquePtr pUndoDoc;
    ScDocumentUniquePtr pRedoDoc;
    ScRange             aRange;
    bool                bMulti;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void                DoChange( ScDocument* pRefDoc);
    void                SetChangeTrack();
};

class ScUndoListNames: public ScBlockUndo
{
public:
    ScUndoListNames(ScDocShell* pNewDocShell,
                    const ScRange& rRange,
                    ScDocumentUniquePtr pNewUndoDoc, ScDocumentUniquePtr pNewRedoDoc);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocumentUniquePtr xUndoDoc;
    ScDocumentUniquePtr xRedoDoc;

    void            DoChange( ScDocument* pSrcDoc ) const;
};

// This class only uses conditional format lists in the undo/redo documents;
// no other tab data is needed in the documents
class ScUndoConditionalFormat : public ScSimpleUndo
{
public:
    ScUndoConditionalFormat( ScDocShell* pNewDocShell,
            ScDocumentUniquePtr pUndoDoc, ScDocumentUniquePtr pRedoDoc, SCTAB nTab);
    virtual         ~ScUndoConditionalFormat() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    void DoChange(ScDocument* pDoc);
    ScDocumentUniquePtr mpUndoDoc;
    ScDocumentUniquePtr mpRedoDoc;
    SCTAB mnTab;
};

class ScUndoConditionalFormatList : public ScSimpleUndo
{
public:
    ScUndoConditionalFormatList( ScDocShell* pNewDocShell,
            ScDocumentUniquePtr pUndoDoc, ScDocumentUniquePtr pRedoDoc, SCTAB nTab);
    virtual         ~ScUndoConditionalFormatList() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    void DoChange(const ScDocument* pDoc);
    ScDocumentUniquePtr mpUndoDoc;
    ScDocumentUniquePtr mpRedoDoc;
    SCTAB mnTab;
};

class ScUndoUseScenario: public ScSimpleUndo
{
public:
                    ScUndoUseScenario( ScDocShell* pNewDocShell,
                                       const ScMarkData& rMark,
                                       const ScArea& rDestArea, ScDocumentUniquePtr pNewUndoDoc,
                                       OUString aNewName );
    virtual         ~ScUndoUseScenario() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocumentUniquePtr
                    pUndoDoc;
    ScRange         aRange;
    ScMarkData      aMarkData;
    OUString        aName;
};

class ScUndoSelectionStyle: public ScSimpleUndo
{
public:
                    ScUndoSelectionStyle( ScDocShell* pNewDocShell,
                                          const ScMarkData& rMark,
                                          const ScRange& rRange,
                                          OUString aName,
                                          ScDocumentUniquePtr pNewUndoDoc );
    virtual         ~ScUndoSelectionStyle() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocumentUniquePtr
                    pUndoDoc;
    OUString        aStyleName;
    ScRange         aRange;

    void            DoChange( const bool bUndo );
};

class ScUndoRefreshLink: public ScSimpleUndo
{
public:
    ScUndoRefreshLink(ScDocShell* pNewDocShell,
                      ScDocumentUniquePtr pNewUndoDoc);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocumentUniquePtr xUndoDoc;
    ScDocumentUniquePtr xRedoDoc;
};

class ScUndoEnterMatrix: public ScBlockUndo
{
public:
                    ScUndoEnterMatrix( ScDocShell* pNewDocShell,
                                       const ScRange& rArea,
                                       ScDocumentUniquePtr pNewUndoDoc,
                                       OUString aForm );
    virtual         ~ScUndoEnterMatrix() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocumentUniquePtr
                    pUndoDoc;
    OUString        aFormula;
    sal_uLong       nStartChangeAction;
    sal_uLong       nEndChangeAction;

    void            SetChangeTrack();
};

class ScUndoInsertAreaLink : public ScSimpleUndo
{
public:
                    ScUndoInsertAreaLink( ScDocShell* pShell,
                                          OUString aDocName,
                                          OUString aFltName, OUString aOptions,
                                          OUString aAreaName, const ScRange& rDestRange,
                                          sal_uLong nRefreshDelay );
    virtual         ~ScUndoInsertAreaLink() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    OUString        aDocName;
    OUString        aFltName;
    OUString        aOptions;
    OUString        aAreaName;
    ScRange         aRange;
    sal_uLong           nRefreshDelay;
};

class ScUndoRemoveAreaLink : public ScSimpleUndo
{
public:
                    ScUndoRemoveAreaLink( ScDocShell* pShell,
                                          OUString aDocName,
                                          OUString aFltName, OUString aOptions,
                                          OUString aAreaName, const ScRange& rDestRange,
                                          sal_uLong nRefreshDelay );
    virtual         ~ScUndoRemoveAreaLink() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    OUString        aDocName;
    OUString        aFltName;
    OUString        aOptions;
    OUString        aAreaName;
    ScRange         aRange;
    sal_uLong           nRefreshDelay;
};

class ScUndoUpdateAreaLink : public ScSimpleUndo        //! also change BlockUndo?
{
public:
    ScUndoUpdateAreaLink(ScDocShell* pShell,
                         OUString aOldD,
                         OUString aOldF, OUString aOldO,
                         OUString aOldA, const ScRange& rOldR,
                         sal_uLong nOldRD,
                         OUString aNewD,
                         OUString aNewF, OUString aNewO,
                         OUString aNewA, const ScRange& rNewR,
                         sal_uLong nNewRD,
                         ScDocumentUniquePtr pUndo, ScDocumentUniquePtr pRedo,
                         bool bDoInsert);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    OUString        aOldDoc;
    OUString        aOldFlt;
    OUString        aOldOpt;
    OUString        aOldArea;
    ScRange         aOldRange;
    OUString        aNewDoc;
    OUString        aNewFlt;
    OUString        aNewOpt;
    OUString        aNewArea;
    ScRange         aNewRange;
    ScDocumentUniquePtr xUndoDoc;
    ScDocumentUniquePtr xRedoDoc;
    sal_uLong       nOldRefresh;
    sal_uLong       nNewRefresh;
    bool            bWithInsert;

    void            DoChange( const bool bUndo ) const;
};

class ScUndoIndent: public ScBlockUndo
{
public:
                    ScUndoIndent( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                  ScDocumentUniquePtr pNewUndoDoc, bool bIncrement );
    virtual         ~ScUndoIndent() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocumentUniquePtr
                    pUndoDoc;
    bool            bIsIncrement;
};

class ScUndoTransliterate: public ScBlockUndo
{
public:
                    ScUndoTransliterate( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                        ScDocumentUniquePtr pNewUndoDoc, TransliterationFlags nType );
    virtual         ~ScUndoTransliterate() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocumentUniquePtr
                    pUndoDoc;
    TransliterationFlags
                    nTransliterationType;
};

class ScUndoClearItems: public ScBlockUndo
{
public:
                    ScUndoClearItems( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                      ScDocumentUniquePtr pNewUndoDoc, const sal_uInt16* pW );
    virtual         ~ScUndoClearItems() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocumentUniquePtr
                    pUndoDoc;
    std::unique_ptr<sal_uInt16[]>
                    pWhich;
};

class ScUndoRemoveBreaks: public ScSimpleUndo
{
public:
                    ScUndoRemoveBreaks( ScDocShell* pNewDocShell,
                                    SCTAB nNewTab, ScDocumentUniquePtr pNewUndoDoc );
    virtual         ~ScUndoRemoveBreaks() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB           nTab;
    ScDocumentUniquePtr
                    pUndoDoc;
};

class ScUndoRemoveMerge: public ScBlockUndo
{
public:
                    ScUndoRemoveMerge( ScDocShell* pNewDocShell,
                                       const ScCellMergeOption& rOption,
                                       ScDocumentUniquePtr pNewUndoDoc );
                    ScUndoRemoveMerge( ScDocShell* pNewDocShell,
                                       const ScRange& rRange,
                                       ScDocumentUniquePtr pNewUndoDoc );
    virtual         ~ScUndoRemoveMerge() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

    ScDocument*     GetUndoDoc();
    void            AddCellMergeOption( const ScCellMergeOption& rOption );

private:
    void            SetCurTab();

    std::vector<ScCellMergeOption> maOptions;
    ScDocumentUniquePtr    pUndoDoc;
};

class ScUndoBorder: public ScBlockUndo
{
public:
    ScUndoBorder(ScDocShell* pNewDocShell,
                 const ScRangeList& rRangeList,
                 ScDocumentUniquePtr pNewUndoDoc,
                 const SvxBoxItem& rNewOuter,
                 const SvxBoxInfoItem& rNewInner);

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocumentUniquePtr xUndoDoc;
    std::unique_ptr<ScRangeList> xRanges;
    std::unique_ptr<SvxBoxItem> xOuter;
    std::unique_ptr<SvxBoxInfoItem> xInner;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
