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
#ifndef SC_UNDOBLK_HXX
#define SC_UNDOBLK_HXX

#include "undobase.hxx"
#include "markdata.hxx"
#include "viewutil.hxx"
#include "spellparam.hxx"
#include "cellmergeoption.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

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

//----------------------------------------------------------------------------

class ScUndoInsertCells: public ScMoveUndo
{
public:
                    TYPEINFO();
                    ScUndoInsertCells( ScDocShell* pNewDocShell,
                                       const ScRange& rRange, SCTAB nNewCount, SCTAB* pNewTabs, SCTAB* pNewScenarios,
                                       InsCellCmd eNewCmd, ScDocument* pUndoDocument, ScRefUndoData* pRefData,
                                       sal_Bool bNewPartOfPaste );
    virtual         ~ScUndoInsertCells();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat( SfxRepeatTarget& rTarget );
    virtual bool    CanRepeat( SfxRepeatTarget& rTarget ) const;

    virtual OUString GetComment() const;

    virtual bool    Merge( SfxUndoAction *pNextAction );

private:
    ScRange         aEffRange;
    SCTAB           nCount;
    SCTAB*          pTabs;
    SCTAB*          pScenarios;
    sal_uLong           nEndChangeAction;
    InsCellCmd      eCmd;
    sal_Bool            bPartOfPaste;
    SfxUndoAction*  pPasteUndo;

    void            DoChange ( const sal_Bool bUndo );
    void            SetChangeTrack();
};


class ScUndoDeleteCells: public ScMoveUndo
{
public:
                    TYPEINFO();
                    ScUndoDeleteCells( ScDocShell* pNewDocShell,
                                       const ScRange& rRange, SCTAB nNewCount, SCTAB* pNewTabs, SCTAB* pNewScenarios,
                                       DelCellCmd eNewCmd, ScDocument* pUndoDocument, ScRefUndoData* pRefData );
    virtual         ~ScUndoDeleteCells();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScRange         aEffRange;
    SCTAB           nCount;
    SCTAB*          pTabs;
    SCTAB*          pScenarios;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;
    DelCellCmd      eCmd;

    void            DoChange ( const sal_Bool bUndo );
    void            SetChangeTrack();
};


class ScUndoDeleteMulti: public ScMoveUndo
{
public:
                    TYPEINFO();
                    ScUndoDeleteMulti( ScDocShell* pNewDocShell,
                                       sal_Bool bNewRows, sal_Bool bNeedsRefresh, SCTAB nNewTab,
                                       const SCCOLROW* pRng, SCCOLROW nRngCnt,
                                       ScDocument* pUndoDocument, ScRefUndoData* pRefData );
    virtual         ~ScUndoDeleteMulti();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    sal_Bool            bRows;
    sal_Bool            bRefresh;
    SCTAB           nTab;
    SCCOLROW*       pRanges;
    SCCOLROW        nRangeCnt;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            DoChange() const;
    void            SetChangeTrack();
};


class ScUndoCut: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoCut( ScDocShell* pNewDocShell,
                               ScRange aRange,              // adjusted for merged cells
                               ScAddress aOldEnd,           // end position without adjustment
                               const ScMarkData& rMark,     // selected sheets
                               ScDocument* pNewUndoDoc );
    virtual         ~ScUndoCut();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    ScRange         aExtendedRange;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            DoChange( const sal_Bool bUndo );
    void            SetChangeTrack();
};


struct ScUndoPasteOptions
{
    sal_uInt16 nFunction;
    sal_Bool bSkipEmpty;
    sal_Bool bTranspose;
    sal_Bool bAsLink;
    InsCellCmd eMoveMode;

    ScUndoPasteOptions() :
        nFunction( PASTE_NOFUNC ),
        bSkipEmpty( false ),
        bTranspose( false ),
        bAsLink( false ),
        eMoveMode( INS_NONE )
    {}
};

class ScUndoPaste: public ScMultiBlockUndo
{
public:
    TYPEINFO();
    ScUndoPaste(ScDocShell* pNewDocShell, const ScRangeList& rRanges,
                const ScMarkData& rMark,
                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                sal_uInt16 nNewFlags,
                ScRefUndoData* pRefData,
                bool bRedoIsFilled = true,
                const ScUndoPasteOptions* pOptions = NULL);
    virtual ~ScUndoPaste();

    virtual void Undo();
    virtual void Redo();
    virtual void Repeat(SfxRepeatTarget& rTarget);
    virtual bool CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
    sal_uInt16          nFlags;
    ScRefUndoData*  pRefUndoData;
    ScRefUndoData*  pRefRedoData;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;
    sal_Bool            bRedoFilled;
    ScUndoPasteOptions aPasteOptions;

    void DoChange(bool bUndo);
    void SetChangeTrack();
};


class ScUndoDragDrop: public ScMoveUndo
{
public:
                    TYPEINFO();
                    ScUndoDragDrop( ScDocShell* pNewDocShell,
                                    const ScRange& rRange, ScAddress aNewDestPos, sal_Bool bNewCut,
                                    ScDocument* pUndoDocument, ScRefUndoData* pRefData,
                                    sal_Bool bScenario );
    virtual         ~ScUndoDragDrop();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScRange         aSrcRange;
    ScRange         aDestRange;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;
    sal_Bool            bCut;
    sal_Bool            bKeepScenarioFlags;

    void            PaintArea( ScRange aRange, sal_uInt16 nExtFlags ) const;
    void            DoUndo( ScRange aRange ) const;

    void            SetChangeTrack();
};


class ScUndoDeleteContents: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoDeleteContents( ScDocShell* pNewDocShell,
                                          const ScMarkData& rMark,
                                          const ScRange& rRange,
                                          ScDocument* pNewUndoDoc, sal_Bool bNewMulti,
                                          sal_uInt16 nNewFlags, sal_Bool bObjects );
    virtual         ~ScUndoDeleteContents();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScRange         aRange;
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;       // Block mark and deleted data
    SdrUndoAction*  pDrawUndo;      // Deleted objects
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;
    sal_uInt16          nFlags;
    sal_Bool            bMulti;         // Multi selection

    void            DoChange( const sal_Bool bUndo );
    void            SetChangeTrack();
};


class ScUndoFillTable: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoFillTable( ScDocShell* pNewDocShell,
                                     const ScMarkData& rMark,
                                     SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                     SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                                     ScDocument* pNewUndoDoc, sal_Bool bNewMulti, SCTAB nSrc,
                                     sal_uInt16 nFlg, sal_uInt16 nFunc, sal_Bool bSkip, sal_Bool bLink );
    virtual         ~ScUndoFillTable();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScRange         aRange;
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;       // Block mark and deleted data
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;
    sal_uInt16          nFlags;
    sal_uInt16          nFunction;
    SCTAB           nSrcTab;
    sal_Bool            bMulti;         // Multi selection
    sal_Bool            bSkipEmpty;
    sal_Bool            bAsLink;

    void            DoChange( const sal_Bool bUndo );
    void            SetChangeTrack();
};


class ScUndoSelectionAttr: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoSelectionAttr( ScDocShell* pNewDocShell,
                                         const ScMarkData& rMark,
                                         SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                         SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                                         ScDocument* pNewUndoDoc, sal_Bool bNewMulti,
                                         const ScPatternAttr* pNewApply,
                                         const SvxBoxItem* pNewOuter = NULL,
                                         const SvxBoxInfoItem* pNewInner = NULL );
    virtual         ~ScUndoSelectionAttr();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

    ScEditDataArray*    GetDataArray();
private:
    ScMarkData      aMarkData;
    ScRange         aRange;
    boost::scoped_ptr<ScEditDataArray> mpDataArray;
    ScDocument*     pUndoDoc;
    sal_Bool            bMulti;
    ScPatternAttr*  pApplyPattern;
    SvxBoxItem*     pLineOuter;
    SvxBoxInfoItem* pLineInner;

    void            DoChange( const sal_Bool bUndo );
    void            ChangeEditData( const bool bUndo );
};


class ScUndoWidthOrHeight: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoWidthOrHeight( ScDocShell* pNewDocShell,
                                         const ScMarkData& rMark,
                                         SCCOLROW nNewStart, SCTAB nNewStartTab,
                                         SCCOLROW nNewEnd, SCTAB nNewEndTab,
                                         ScDocument* pNewUndoDoc,
                                         SCCOLROW nNewCnt, SCCOLROW* pNewRanges,
                                         ScOutlineTable* pNewUndoTab,
                                         ScSizeMode eNewMode, sal_uInt16 nNewSizeTwips,
                                         sal_Bool bNewWidth );
    virtual         ~ScUndoWidthOrHeight();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScMarkData      aMarkData;
    SCCOLROW        nStart;
    SCCOLROW        nEnd;
    SCTAB           nStartTab;
    SCTAB           nEndTab;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTab;
    SCCOLROW        nRangeCnt;
    SCCOLROW*       pRanges;
    sal_uInt16          nNewSize;
    sal_Bool            bWidth;
    ScSizeMode      eMode;
    SdrUndoAction*  pDrawUndo;
};


class ScUndoAutoFill: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoAutoFill( ScDocShell* pNewDocShell,
                                    const ScRange& rRange, const ScRange& rSourceArea,
                                    ScDocument* pNewUndoDoc, const ScMarkData& rMark,
                                    FillDir eNewFillDir,
                                    FillCmd eNewFillCmd, FillDateCmd eNewFillDateCmd,
                                    double fNewStartValue, double fNewStepValue, double fNewMaxValue );
    virtual         ~ScUndoAutoFill();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

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
                    TYPEINFO();
                    ScUndoMerge( ScDocShell* pNewDocShell, const ScCellMergeOption& rOption,
                                 bool bMergeContents, ScDocument* pUndoDoc, SdrUndoAction* pDrawUndo);
    virtual         ~ScUndoMerge();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

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
                    TYPEINFO();
                    ScUndoAutoFormat( ScDocShell* pNewDocShell,
                                      const ScRange& rRange, ScDocument* pNewUndoDoc,
                                      const ScMarkData& rMark,
                                      sal_Bool bNewSize, sal_uInt16 nNewFormatNo );
    virtual         ~ScUndoAutoFormat();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScDocument*     pUndoDoc;       // deleted data
    ScMarkData      aMarkData;
    sal_Bool            bSize;
    sal_uInt16          nFormatNo;
};


class ScUndoReplace: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoReplace( ScDocShell* pNewDocShell,
                                   const ScMarkData& rMark,
                                   SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                   const String& rNewUndoStr, ScDocument* pNewUndoDoc,
                                   const SvxSearchItem* pItem );
    virtual         ~ScUndoReplace();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScAddress       aCursorPos;
    ScMarkData      aMarkData;
    String          aUndoStr;           // Data at single selection
    ScDocument*     pUndoDoc;           // Block mark and deleted data
    SvxSearchItem*  pSearchItem;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

            void    SetChangeTrack();
};


class ScUndoTabOp: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoTabOp( ScDocShell* pNewDocShell,
                                 SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                 SCCOL nEndX,   SCROW nEndY,   SCTAB nEndZ,
                                 ScDocument* pNewUndoDoc,
                                 const ScRefAddress& rFormulaCell,
                                 const ScRefAddress& rFormulaEnd,
                                 const ScRefAddress& rRowCell,
                                 const ScRefAddress& rColCell,
                                 sal_uInt8 nMode );
    virtual         ~ScUndoTabOp();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScRange         aRange;
    ScDocument*     pUndoDoc;       // Deleted data
    ScRefAddress    theFormulaCell;
    ScRefAddress    theFormulaEnd;
    ScRefAddress    theRowCell;
    ScRefAddress    theColCell;
    sal_uInt8           nMode;
};


class ScUndoConversion : public ScSimpleUndo
{
public:
                    TYPEINFO();

                    ScUndoConversion( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                      SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScDocument* pNewUndoDoc,
                                      SCCOL nNewX, SCROW nNewY, SCTAB nNewZ, ScDocument* pNewRedoDoc,
                                      const ScConversionParam& rConvParam );
    virtual         ~ScUndoConversion();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

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
                    TYPEINFO();
                    ScUndoRefConversion( ScDocShell* pNewDocShell,
                                         const ScRange& aMarkRange, const ScMarkData& rMark,
                                         ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc, sal_Bool bNewMulti, sal_uInt16 nNewFlag);
    virtual         ~ScUndoRefConversion();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScMarkData          aMarkData;
    ScDocument*         pUndoDoc;
    ScDocument*         pRedoDoc;
    ScRange             aRange;
    sal_Bool                bMulti;
    sal_uInt16              nFlags;
    sal_uLong               nStartChangeAction;
    sal_uLong               nEndChangeAction;

    void                DoChange( ScDocument* pRefDoc);
    void                SetChangeTrack();
};

class ScUndoListNames: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoListNames( ScDocShell* pNewDocShell,
                                     const ScRange& rRange,
                                     ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc );
    virtual         ~ScUndoListNames();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;

    void            DoChange( ScDocument* pSrcDoc ) const;
};


class ScUndoUseScenario: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoUseScenario( ScDocShell* pNewDocShell,
                                       const ScMarkData& rMark,
                                       const ScArea& rDestArea, ScDocument* pNewUndoDoc,
                                       const String& rNewName );
    virtual         ~ScUndoUseScenario();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScDocument*     pUndoDoc;
    ScRange         aRange;
    ScMarkData      aMarkData;
    String          aName;
};


class ScUndoSelectionStyle: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoSelectionStyle( ScDocShell* pNewDocShell,
                                          const ScMarkData& rMark,
                                          const ScRange& rRange,
                                          const String& rName,
                                          ScDocument* pNewUndoDoc );
    virtual         ~ScUndoSelectionStyle();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;
    virtual sal_uInt16  GetId() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    String          aStyleName;
    ScRange         aRange;

    void            DoChange( const sal_Bool bUndo );
};


class ScUndoRefreshLink: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRefreshLink( ScDocShell* pNewDocShell,
                                       ScDocument* pNewUndoDoc );
    virtual         ~ScUndoRefreshLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
};


class ScUndoEnterMatrix: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoEnterMatrix( ScDocShell* pNewDocShell,
                                       const ScRange& rArea,
                                       ScDocument* pNewUndoDoc,
                                       const String& rForm );
    virtual         ~ScUndoEnterMatrix();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScDocument*     pUndoDoc;
    String          aFormula;
    sal_uLong           nStartChangeAction;
    sal_uLong           nEndChangeAction;

    void            SetChangeTrack();
};


class ScUndoInsertAreaLink : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoInsertAreaLink( ScDocShell* pShell,
                                          const OUString& rDocName,
                                          const OUString& rFltName, const String& rOptions,
                                          const OUString& rAreaName, const ScRange& rDestRange,
                                          sal_uLong nRefreshDelay );
    virtual         ~ScUndoInsertAreaLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    OUString        aDocName;
    OUString        aFltName;
    String          aOptions;
    OUString        aAreaName;
    ScRange         aRange;
    sal_uLong           nRefreshDelay;
};


class ScUndoRemoveAreaLink : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRemoveAreaLink( ScDocShell* pShell,
                                          const OUString& rDocName,
                                          const OUString& rFltName, const String& rOptions,
                                          const OUString& rAreaName, const ScRange& rDestRange,
                                          sal_uLong nRefreshDelay );
    virtual         ~ScUndoRemoveAreaLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    OUString        aDocName;
    OUString        aFltName;
    String          aOptions;
    OUString        aAreaName;
    ScRange         aRange;
    sal_uLong           nRefreshDelay;
};


class ScUndoUpdateAreaLink : public ScSimpleUndo        //! also change BlockUndo?
{
public:
                    TYPEINFO();
                    ScUndoUpdateAreaLink( ScDocShell* pShell,
                                          const String& rOldD,
                                          const String& rOldF, const String& rOldO,
                                          const String& rOldA, const ScRange& rOldR,
                                          sal_uLong nOldRD,
                                          const String& rNewD,
                                          const String& rNewF, const String& rNewO,
                                          const String& rNewA, const ScRange& rNewR,
                                          sal_uLong nNewRD,
                                          ScDocument* pUndo, ScDocument* pRedo,
                                          sal_Bool bDoInsert );
    virtual         ~ScUndoUpdateAreaLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    String          aOldDoc;
    String          aOldFlt;
    String          aOldOpt;
    String          aOldArea;
    ScRange         aOldRange;
    String          aNewDoc;
    String          aNewFlt;
    String          aNewOpt;
    String          aNewArea;
    ScRange         aNewRange;
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
    sal_uLong           nOldRefresh;
    sal_uLong           nNewRefresh;
    sal_Bool            bWithInsert;

    void            DoChange( const sal_Bool bUndo ) const;
};


class ScUndoIndent: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoIndent( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                    ScDocument* pNewUndoDoc, sal_Bool bIncrement );
    virtual         ~ScUndoIndent();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    sal_Bool            bIsIncrement;
};


class ScUndoTransliterate: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoTransliterate( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                        ScDocument* pNewUndoDoc, sal_Int32 nType );
    virtual         ~ScUndoTransliterate();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    sal_Int32       nTransliterationType;
};


class ScUndoClearItems: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoClearItems( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                        ScDocument* pNewUndoDoc, const sal_uInt16* pW );
    virtual         ~ScUndoClearItems();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    sal_uInt16*         pWhich;
};


class ScUndoRemoveBreaks: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRemoveBreaks( ScDocShell* pNewDocShell,
                                    SCTAB nNewTab, ScDocument* pNewUndoDoc );
    virtual         ~ScUndoRemoveBreaks();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    SCTAB           nTab;
    ScDocument*     pUndoDoc;
};


class ScUndoRemoveMerge: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoRemoveMerge( ScDocShell* pNewDocShell,
                                       const ScCellMergeOption& rOption,
                                       ScDocument* pNewUndoDoc );
    virtual         ~ScUndoRemoveMerge();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    void            SetCurTab();

    ScCellMergeOption maOption;
    ScDocument*     pUndoDoc;
};


class ScUndoBorder: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoBorder( ScDocShell* pNewDocShell,
                                    const ScRangeList& rRangeList,
                                    ScDocument* pNewUndoDoc,
                                    const SvxBoxItem& rNewOuter,
                                    const SvxBoxInfoItem& rNewInner );
    virtual         ~ScUndoBorder();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual OUString GetComment() const;

private:
    ScDocument*     pUndoDoc;
    ScRangeList*    pRanges;
    SvxBoxItem*     pOuter;
    SvxBoxInfoItem* pInner;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
