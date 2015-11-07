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
#ifndef INCLUDED_SC_SOURCE_UI_INC_UNDOBLK_HXX
#define INCLUDED_SC_SOURCE_UI_INC_UNDOBLK_HXX

#include "undobase.hxx"
#include "markdata.hxx"
#include "viewutil.hxx"
#include "spellparam.hxx"
#include "cellmergeoption.hxx"
#include "paramisc.hxx"

#include <memory>

class ScDocShell;
class ScDocument;
class ScOutlineTable;
class ScRangeList;
class ScPatternAttr;
class SvxBoxItem;
class SvxBoxInfoItem;
class SvxSearchItem;
class SdrUndoAction;
class ScEditDataArray;

class ScUndoInsertCells: public ScMoveUndo
{
public:
                    ScUndoInsertCells( ScDocShell* pNewDocShell,
                                       const ScRange& rRange, SCTAB nNewCount, SCTAB* pNewTabs, SCTAB* pNewScenarios,
                                       InsCellCmd eNewCmd, ScDocument* pUndoDocument, ScRefUndoData* pRefData,
                                       bool bNewPartOfPaste );
    virtual         ~ScUndoInsertCells();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat( SfxRepeatTarget& rTarget ) override;
    virtual bool    CanRepeat( SfxRepeatTarget& rTarget ) const override;

    virtual OUString GetComment() const override;

    virtual bool    Merge( SfxUndoAction *pNextAction ) override;

private:
    ScRange         aEffRange;
    SCTAB           nCount;
    SCTAB*          pTabs;
    SCTAB*          pScenarios;
    sal_uLong       nEndChangeAction;
    InsCellCmd      eCmd;
    bool            bPartOfPaste;
    SfxUndoAction*  pPasteUndo;

    void            DoChange ( const bool bUndo );
    void            SetChangeTrack();
};

class ScUndoDeleteCells: public ScMoveUndo
{
public:
                    ScUndoDeleteCells( ScDocShell* pNewDocShell,
                                       const ScRange& rRange, SCTAB nNewCount, SCTAB* pNewTabs, SCTAB* pNewScenarios,
                                       DelCellCmd eNewCmd, ScDocument* pUndoDocument, ScRefUndoData* pRefData );
    virtual         ~ScUndoDeleteCells();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScRange         aEffRange;
    SCTAB           nCount;
    SCTAB*          pTabs;
    SCTAB*          pScenarios;
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
                       const std::vector<sc::ColRowSpan>& rSpans,
                       ScDocument* pUndoDocument, ScRefUndoData* pRefData );

    virtual         ~ScUndoDeleteMulti();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    bool mbRows:1;
    bool mbRefresh:1;
    SCTAB           nTab;
    std::vector<sc::ColRowSpan> maSpans;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            DoChange() const;
    void            SetChangeTrack();
};

class ScUndoCut: public ScBlockUndo
{
public:
                    ScUndoCut( ScDocShell* pNewDocShell,
                               ScRange aRange,              // adjusted for merged cells
                               ScAddress aOldEnd,           // end position without adjustment
                               const ScMarkData& rMark,     // selected sheets
                               ScDocument* pNewUndoDoc );
    virtual         ~ScUndoCut();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    ScRange         aExtendedRange;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            DoChange( const bool bUndo );
    void            SetChangeTrack();
};

struct ScUndoPasteOptions
{
    ScPasteFunc  nFunction;
    bool       bSkipEmpty;
    bool       bTranspose;
    bool       bAsLink;
    InsCellCmd eMoveMode;

    ScUndoPasteOptions() :
        nFunction( ScPasteFunc::NONE ),
        bSkipEmpty( false ),
        bTranspose( false ),
        bAsLink( false ),
        eMoveMode( INS_NONE )
    {}
};

class ScUndoPaste: public ScMultiBlockUndo
{
public:
    ScUndoPaste(ScDocShell* pNewDocShell, const ScRangeList& rRanges,
                const ScMarkData& rMark,
                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                InsertDeleteFlags nNewFlags,
                ScRefUndoData* pRefData,
                bool bRedoIsFilled = true,
                const ScUndoPasteOptions* pOptions = nullptr);
    virtual ~ScUndoPaste();

    virtual void Undo() override;
    virtual void Redo() override;
    virtual void Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
    InsertDeleteFlags nFlags;
    ScRefUndoData*  pRefUndoData;
    ScRefUndoData*  pRefRedoData;
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
                                    const ScRange& rRange, ScAddress aNewDestPos, bool bNewCut,
                                    ScDocument* pUndoDocument, ScRefUndoData* pRefData,
                                    bool bScenario );
    virtual         ~ScUndoDragDrop();

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
                                          std::unique_ptr<ScDocument>&& pNewUndoDoc, bool bNewMulti,
                                          InsertDeleteFlags nNewFlags, bool bObjects );
    virtual         ~ScUndoDeleteContents();

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
    std::unique_ptr<ScDocument> pUndoDoc; // Block mark and deleted data
    SdrUndoAction*  pDrawUndo;      // Deleted objects
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
                                     ScDocument* pNewUndoDoc, bool bNewMulti, SCTAB nSrc,
                                     InsertDeleteFlags nFlg, ScPasteFunc nFunc, bool bSkip, bool bLink );
    virtual         ~ScUndoFillTable();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScRange         aRange;
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;       // Block mark and deleted data
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
                                         ScDocument* pNewUndoDoc, bool bNewMulti,
                                         const ScPatternAttr* pNewApply,
                                         const SvxBoxItem* pNewOuter = nullptr,
                                         const SvxBoxInfoItem* pNewInner = nullptr );
    virtual         ~ScUndoSelectionAttr();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

    ScEditDataArray*    GetDataArray();
private:
    ScMarkData      aMarkData;
    ScRange         aRange;
    std::unique_ptr<ScEditDataArray> mpDataArray;
    ScDocument*     pUndoDoc;
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
                                         ScDocument* pNewUndoDoc,
                                         const std::vector<sc::ColRowSpan>& rRanges,
                                         ScOutlineTable* pNewUndoTab,
                                         ScSizeMode eNewMode, sal_uInt16 nNewSizeTwips,
                                         bool bNewWidth );
    virtual         ~ScUndoWidthOrHeight();

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
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTab;
    std::vector<sc::ColRowSpan> maRanges;
    sal_uInt16      nNewSize;
    bool            bWidth;
    ScSizeMode      eMode;
    SdrUndoAction*  pDrawUndo;
};

class ScUndoAutoFill: public ScBlockUndo
{
public:
                    ScUndoAutoFill( ScDocShell* pNewDocShell,
                                    const ScRange& rRange, const ScRange& rSourceArea,
                                    ScDocument* pNewUndoDoc, const ScMarkData& rMark,
                                    FillDir eNewFillDir,
                                    FillCmd eNewFillCmd, FillDateCmd eNewFillDateCmd,
                                    double fNewStartValue, double fNewStepValue, double fNewMaxValue );
    virtual         ~ScUndoAutoFill();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScRange         aSource;
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
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
                    ScUndoMerge( ScDocShell* pNewDocShell, const ScCellMergeOption& rOption,
                                 bool bMergeContents, ScDocument* pUndoDoc, SdrUndoAction* pDrawUndo);
    virtual         ~ScUndoMerge();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScCellMergeOption maOption;
    bool            mbMergeContents;        // Merge contents in Redo().
    ScDocument*     mpUndoDoc;              // when data is merged
    SdrUndoAction*  mpDrawUndo;

    void            DoChange( bool bUndo ) const;
};

class ScUndoAutoFormat: public ScBlockUndo
{
public:
                    ScUndoAutoFormat( ScDocShell* pNewDocShell,
                                      const ScRange& rRange, ScDocument* pNewUndoDoc,
                                      const ScMarkData& rMark,
                                      bool bNewSize, sal_uInt16 nNewFormatNo );
    virtual         ~ScUndoAutoFormat();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocument*     pUndoDoc;       // deleted data
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
                                   const OUString& rNewUndoStr, ScDocument* pNewUndoDoc,
                                   const SvxSearchItem* pItem );
    virtual         ~ScUndoReplace();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress       aCursorPos;
    ScMarkData      aMarkData;
    OUString        aUndoStr;           // Data at single selection
    ScDocument*     pUndoDoc;           // Block mark and deleted data
    SvxSearchItem*  pSearchItem;
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
                                 ScDocument* pNewUndoDoc,
                                 const ScRefAddress& rFormulaCell,
                                 const ScRefAddress& rFormulaEnd,
                                 const ScRefAddress& rRowCell,
                                 const ScRefAddress& rColCell,
                                 ScTabOpParam::Mode eMode );
    virtual         ~ScUndoTabOp();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScRange         aRange;
    ScDocument*     pUndoDoc;       // Deleted data
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
                                      SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScDocument* pNewUndoDoc,
                                      SCCOL nNewX, SCROW nNewY, SCTAB nNewZ, ScDocument* pNewRedoDoc,
                                      const ScConversionParam& rConvParam );
    virtual         ~ScUndoConversion();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScAddress       aCursorPos;
    ScDocument*     pUndoDoc;           // Block mark and deleted data
    ScAddress       aNewCursorPos;
    ScDocument*     pRedoDoc;           // Block mark and new data
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
                                         ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc, bool bNewMulti, InsertDeleteFlags nNewFlag);
    virtual         ~ScUndoRefConversion();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData          aMarkData;
    ScDocument*         pUndoDoc;
    ScDocument*         pRedoDoc;
    ScRange             aRange;
    bool                bMulti;
    InsertDeleteFlags   nFlags;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void                DoChange( ScDocument* pRefDoc);
    void                SetChangeTrack();
};

class ScUndoListNames: public ScBlockUndo
{
public:
                    ScUndoListNames( ScDocShell* pNewDocShell,
                                     const ScRange& rRange,
                                     ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc );
    virtual         ~ScUndoListNames();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;

    void            DoChange( ScDocument* pSrcDoc ) const;
};

class ScUndoConditionalFormat : public ScSimpleUndo
{
public:
    ScUndoConditionalFormat( ScDocShell* pNewDocShell,
            ScDocument* pUndoDoc, ScDocument* pRedoDoc, const ScRange& rRange);
    virtual         ~ScUndoConditionalFormat();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    void DoChange(ScDocument* pDoc);
    std::unique_ptr<ScDocument> mpUndoDoc;
    std::unique_ptr<ScDocument> mpRedoDoc;
    ScRange maRange;
};

class ScUndoUseScenario: public ScSimpleUndo
{
public:
                    ScUndoUseScenario( ScDocShell* pNewDocShell,
                                       const ScMarkData& rMark,
                                       const ScArea& rDestArea, ScDocument* pNewUndoDoc,
                                       const OUString& rNewName );
    virtual         ~ScUndoUseScenario();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocument*     pUndoDoc;
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
                                          const OUString& rName,
                                          ScDocument* pNewUndoDoc );
    virtual         ~ScUndoSelectionStyle();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;
    virtual sal_uInt16  GetId() const override;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    OUString        aStyleName;
    ScRange         aRange;

    void            DoChange( const bool bUndo );
};

class ScUndoRefreshLink: public ScSimpleUndo
{
public:
                    ScUndoRefreshLink( ScDocShell* pNewDocShell,
                                       ScDocument* pNewUndoDoc );
    virtual         ~ScUndoRefreshLink();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
};

class ScUndoEnterMatrix: public ScBlockUndo
{
public:
                    ScUndoEnterMatrix( ScDocShell* pNewDocShell,
                                       const ScRange& rArea,
                                       ScDocument* pNewUndoDoc,
                                       const OUString& rForm );
    virtual         ~ScUndoEnterMatrix();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocument*     pUndoDoc;
    OUString        aFormula;
    sal_uLong       nStartChangeAction;
    sal_uLong       nEndChangeAction;

    void            SetChangeTrack();
};

class ScUndoInsertAreaLink : public ScSimpleUndo
{
public:
                    ScUndoInsertAreaLink( ScDocShell* pShell,
                                          const OUString& rDocName,
                                          const OUString& rFltName, const OUString& rOptions,
                                          const OUString& rAreaName, const ScRange& rDestRange,
                                          sal_uLong nRefreshDelay );
    virtual         ~ScUndoInsertAreaLink();

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
                                          const OUString& rDocName,
                                          const OUString& rFltName, const OUString& rOptions,
                                          const OUString& rAreaName, const ScRange& rDestRange,
                                          sal_uLong nRefreshDelay );
    virtual         ~ScUndoRemoveAreaLink();

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
                    ScUndoUpdateAreaLink( ScDocShell* pShell,
                                          const OUString& rOldD,
                                          const OUString& rOldF, const OUString& rOldO,
                                          const OUString& rOldA, const ScRange& rOldR,
                                          sal_uLong nOldRD,
                                          const OUString& rNewD,
                                          const OUString& rNewF, const OUString& rNewO,
                                          const OUString& rNewA, const ScRange& rNewR,
                                          sal_uLong nNewRD,
                                          ScDocument* pUndo, ScDocument* pRedo,
                                          bool bDoInsert );
    virtual         ~ScUndoUpdateAreaLink();

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
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
    sal_uLong       nOldRefresh;
    sal_uLong       nNewRefresh;
    bool            bWithInsert;

    void            DoChange( const bool bUndo ) const;
};

class ScUndoIndent: public ScBlockUndo
{
public:
                    ScUndoIndent( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                    ScDocument* pNewUndoDoc, bool bIncrement );
    virtual         ~ScUndoIndent();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    bool            bIsIncrement;
};

class ScUndoTransliterate: public ScBlockUndo
{
public:
                    ScUndoTransliterate( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                        ScDocument* pNewUndoDoc, sal_Int32 nType );
    virtual         ~ScUndoTransliterate();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    sal_Int32       nTransliterationType;
};

class ScUndoClearItems: public ScBlockUndo
{
public:
                    ScUndoClearItems( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                        ScDocument* pNewUndoDoc, const sal_uInt16* pW );
    virtual         ~ScUndoClearItems();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    sal_uInt16*         pWhich;
};

class ScUndoRemoveBreaks: public ScSimpleUndo
{
public:
                    ScUndoRemoveBreaks( ScDocShell* pNewDocShell,
                                    SCTAB nNewTab, ScDocument* pNewUndoDoc );
    virtual         ~ScUndoRemoveBreaks();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB           nTab;
    ScDocument*     pUndoDoc;
};

class ScUndoRemoveMerge: public ScBlockUndo
{
public:
                    ScUndoRemoveMerge( ScDocShell* pNewDocShell,
                                       const ScCellMergeOption& rOption,
                                       ScDocument* pNewUndoDoc );
    virtual         ~ScUndoRemoveMerge();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    void            SetCurTab();

    ScCellMergeOption maOption;
    ScDocument*     pUndoDoc;
};

class ScUndoBorder: public ScBlockUndo
{
public:
                    ScUndoBorder( ScDocShell* pNewDocShell,
                                    const ScRangeList& rRangeList,
                                    ScDocument* pNewUndoDoc,
                                    const SvxBoxItem& rNewOuter,
                                    const SvxBoxInfoItem& rNewInner );
    virtual         ~ScUndoBorder();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScDocument*     pUndoDoc;
    ScRangeList*    pRanges;
    SvxBoxItem*     pOuter;
    SvxBoxInfoItem* pInner;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
