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
#ifndef INCLUDED_SC_SOURCE_UI_INC_VIEWFUNC_HXX
#define INCLUDED_SC_SOURCE_UI_INC_VIEWFUNC_HXX

#include "tabview.hxx"

#include <tabbgcolor.hxx>

#include <com/sun/star/embed/Aspects.hpp>
#include <vector>

class ScPatternAttr;
class ScAutoFormatData;
class SvxSearchItem;
class SfxItemSet;
class SvxBoxItem;
class SvxBoxInfoItem;
class SfxStyleSheetBase;
class SfxStyleSheet;
class SfxPoolItem;
class EditTextObject;
struct ScSolveParam;
struct ScTabOpParam;
class ScValidationData;
class ScConversionParam;
class SdrModel;
class Graphic;
class ScRangeList;
class SvxHyperlinkItem;
class ScTransferObj;
class ScTableProtection;
enum class CreateNameFlags;

namespace editeng { class SvxBorderLine; }
namespace com { namespace sun { namespace star { namespace embed { class XEmbeddedObject; } } } }

namespace sc {

struct ColRowSpan;

}

namespace com { namespace sun { namespace star { namespace datatransfer { class XTransferable; } } } }

class ScViewFunc : public ScTabView
{
private:
    ScAddress   aFormatSource;      // for automatic extension of formatting
    ScRange     aFormatArea;
    bool        bFormatValid;

public:
                    ScViewFunc( vcl::Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                    ~ScViewFunc();

    SC_DLLPUBLIC const ScPatternAttr*    GetSelectionPattern ();
    void                    GetSelectionFrame   ( SvxBoxItem&       rLineOuter,
                                                  SvxBoxInfoItem&   rLineInner );

    SvtScriptType   GetSelectionScriptType();

    bool            GetAutoSumArea(ScRangeList& rRangeList);
    void            EnterAutoSum(const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr);
    bool            AutoSum( const ScRange& rRange, bool bSubTotal, bool bSetCursor, bool bContinue );
    OUString        GetAutoSumFormula( const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr );

    void            EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab, const OUString& rString,
                               const EditTextObject* pData = nullptr );
    void            EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab,
                               const EditTextObject& rData, bool bTestSimple = false );
    void            EnterValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rValue );

    void            EnterMatrix( const OUString& rString, ::formula::FormulaGrammar::Grammar eGram );

    /**
     * @param pData The caller must manage the life cycle of the object this
     *              pointer points to.  NULL is allowed.
     */
    void            EnterBlock( const OUString& rString, const EditTextObject* pData );

    void            EnterDataAtCursor( const OUString& rString );         //! Not used?

    SC_DLLPUBLIC void           CutToClip();
    SC_DLLPUBLIC bool           CopyToClip( ScDocument* pClipDoc, bool bCut, bool bApi = false,
                                            bool bIncludeObjects = false, bool bStopEdit = true );
    SC_DLLPUBLIC bool           CopyToClip( ScDocument* pClipDoc, const ScRangeList& rRange, bool bCut,
                                            bool bApi = false, bool bIncludeObjects = false, bool bStopEdit = true );
    bool                        CopyToClipSingleRange( ScDocument* pClipDoc, const ScRangeList& rRanges, bool bCut,
                                            bool bIncludeObjects );
    bool                        CopyToClipMultiRange( const ScDocument* pClipDoc, const ScRangeList& rRanges, bool bCut,
                                            bool bApi, bool bIncludeObjects );
    ScTransferObj*              CopyToTransferable();
    SC_DLLPUBLIC bool           PasteFromClip( InsertDeleteFlags nFlags, ScDocument* pClipDoc,
                                    ScPasteFunc nFunction = ScPasteFunc::NONE, bool bSkipEmpty = false,
                                    bool bTranspose = false, bool bAsLink = false,
                                    InsCellCmd eMoveMode = INS_NONE,
                                    InsertDeleteFlags nUndoExtraFlags = InsertDeleteFlags::NONE,
                                    bool bAllowDialogs = false );

    void                        FillTab( InsertDeleteFlags nFlags, ScPasteFunc nFunction, bool bSkipEmpty, bool bAsLink );

    SC_DLLPUBLIC void           PasteFromSystem();
    SC_DLLPUBLIC bool           PasteFromSystem( SotClipboardFormatId nFormatId, bool bApi = false );
    void                        PasteFromTransferable( const css::uno::Reference<
                                                       css::datatransfer::XTransferable >& rxTransferable );

    void            PasteDraw();
    void            PasteDraw( const Point& rLogicPos, SdrModel* pModel, bool bGroup,
                        const OUString& rSrcShellID, const OUString& rDestShellID );

    bool            PasteOnDrawObjectLinked(
                        const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable,
                        SdrObject& rHitObj);

    bool            PasteDataFormat( SotClipboardFormatId nFormatId,
                                        const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable,
                                        SCCOL nPosX, SCROW nPosY, const Point* pLogicPos,
                                        bool bLink = false, bool bAllowDialogs = false );

    bool            PasteFile( const Point&, const OUString&, bool bLink );
    bool            PasteObject( const Point&, const css::uno::Reference < css::embed::XEmbeddedObject >&, const Size*, const Graphic* = nullptr, const OUString& = OUString(), sal_Int64 nAspect = css::embed::Aspects::MSOLE_CONTENT );
    bool            PasteBitmapEx( const Point&, const BitmapEx& );
    bool            PasteMetaFile( const Point&, const GDIMetaFile& );
    bool            PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                    const OUString& rFile, const OUString& rFilter );
    bool            PasteBookmark( SotClipboardFormatId nFormatId,
                                const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable,
                                SCCOL nPosX, SCROW nPosY );
    bool            PasteLink( const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable );

    void            InsertBookmark( const OUString& rDescription, const OUString& rURL,
                                    SCCOL nPosX, SCROW nPosY, const OUString* pTarget = nullptr,
                                    bool bTryReplace = false );
    bool            HasBookmarkAtCursor( SvxHyperlinkItem* pContent );

    bool            MoveBlockTo( const ScRange& rSource, const ScAddress& rDestPos,
                                    bool bCut );

    bool            LinkBlock( const ScRange& rSource, const ScAddress& rDestPos );

    void            CreateNames( CreateNameFlags nFlags );
    CreateNameFlags GetCreateNameFlags();
    void            InsertNameList();
    bool            InsertName( const OUString& rName, const OUString& rSymbol,
                                const OUString& rType );

    void            ApplyAttributes( const SfxItemSet* pDialogSet, const SfxItemSet* pOldSet, bool bAdjustBlockHeight = true );
    void            ApplyAttr( const SfxPoolItem& rAttrItem, bool bAdjustBlockHeight = true );

    void            ApplySelectionPattern( const ScPatternAttr& rAttr,
                                            bool bCursorOnly = false);
    void            ApplyPatternLines(const ScPatternAttr& rAttr,
                                      const SvxBoxItem& rNewOuter,
                                      const SvxBoxInfoItem* pNewInner);

    void            ApplyUserItemSet( const SfxItemSet& rItemSet );

    const SfxStyleSheet*
                    GetStyleSheetFromMarked();
    void            SetStyleSheetToMarked( const SfxStyleSheet* pStyleSheet );
    void            RemoveStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet );
    void            UpdateStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet );

    void            SetNumberFormat( SvNumFormatType nFormatType, sal_uLong nAdd = 0 );
    void            SetNumFmtByStr( const OUString& rCode );
    void            ChangeNumFmtDecimals( bool bIncrement );

    void            SetValidation( const ScValidationData& rNew );

    void            ChangeIndent( bool bIncrement );

    void            ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect );

    void            Protect( SCTAB nTab, const OUString& rPassword );
    bool            Unprotect( SCTAB nTab, const OUString& rPassword );

    void            DeleteCells( DelCellCmd eCmd );
    bool            InsertCells( InsCellCmd eCmd, bool bRecord = true, bool bPartOfPaste = false );
    void            DeleteMulti( bool bRows );

    void            DeleteContents( InsertDeleteFlags nFlags );

    void SetWidthOrHeight(
        bool bWidth, const std::vector<sc::ColRowSpan>& rRanges, ScSizeMode eMode,
        sal_uInt16 nSizeTwips, bool bRecord = true, const ScMarkData* pMarkData = nullptr );

    void            SetMarkedWidthOrHeight( bool bWidth, ScSizeMode eMode, sal_uInt16 nSizeTwips );

    bool            AdjustBlockHeight( bool bPaint = true, ScMarkData* pMarkData = nullptr );
    bool            AdjustRowHeight( SCROW nStartRow, SCROW nEndRow );

    void            ModifyCellSize( ScDirection eDir, bool bOptimal );

    SC_DLLPUBLIC void
                    InsertPageBreak( bool bColumn, bool bRecord = true,
                                        const ScAddress* pPos = nullptr,
                                        bool bSetModified = true );
    SC_DLLPUBLIC void
                    DeletePageBreak( bool bColumn, bool bRecord = true,
                                        const ScAddress* pPos = nullptr,
                                        bool bSetModified = true );

    void            RemoveManualBreaks();

    void            SetPrintZoom(sal_uInt16 nScale);
    void            AdjustPrintZoom();

    bool            TestMergeCells();
    bool            TestRemoveMerge();

    bool            MergeCells( bool bApi, bool& rDoContents, bool bCenter );
    bool            RemoveMerge();

    void            FillSimple( FillDir eDir );
    void            FillSeries( FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                                double fStart, double fStep, double fMax );
    void            FillAuto( FillDir eDir, SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, sal_uLong nCount );
    void            FillCrossDblClick();
    void            ConvertFormulaToValue();

    void            TransliterateText( TransliterationFlags nType );

    ScAutoFormatData* CreateAutoFormatData();
    void            AutoFormat( sal_uInt16 nFormatNo );

    bool            SearchAndReplace( const SvxSearchItem* pSearchItem,
                                        bool bAddUndo, bool bIsApi );

    void            Solve( const ScSolveParam& rParam );
    void            TabOp( const ScTabOpParam& rParam,  bool bRecord = true );

    bool            InsertTable( const OUString& rName, SCTAB nTabNr, bool bRecord = true );
    void            InsertTables(std::vector<OUString>& aNames, SCTAB nTab, SCTAB nCount, bool bRecord = true);

    bool            AppendTable( const OUString& rName, bool bRecord = true );

    void            DeleteTable( SCTAB nTabNr, bool bRecord = true );
    bool            DeleteTables(const std::vector<SCTAB>& TheTabs, bool bRecord = true );
    void            DeleteTables(SCTAB nTab, SCTAB nSheets);

    bool            RenameTable( const OUString& rName, SCTAB nTabNr );
    void            MoveTable( sal_uInt16 nDestDocNo, SCTAB nDestTab, bool bCopy, const OUString* pNewTabName = nullptr );
    void            ImportTables( ScDocShell* pSrcShell,
                                    SCTAB nCount, const SCTAB* pSrcTabs,
                                    bool bLink,SCTAB nTab);

    bool            SetTabBgColor( const Color& rColor, SCTAB nTabNr );
    bool            SetTabBgColor( ScUndoTabColorInfo::List& rUndoSetTabBgColorInfoList );

    void            InsertTableLink( const OUString& rFile,
                                        const OUString& rFilter, const OUString& rOptions,
                                        const OUString& rTabName );
    void            InsertAreaLink( const OUString& rFile,
                                        const OUString& rFilter, const OUString& rOptions,
                                        const OUString& rSource );

    void            ShowTable( const std::vector<OUString>& rNames );
    void            HideTable( const ScMarkData& rMark );

    void            MakeScenario(const OUString& rName, const OUString& rComment,
                                 const Color& rColor, ScScenarioFlags nFlags);
    void            ExtendScenario();
    void            UseScenario( const OUString& rName );

    void            InsertSpecialChar( const OUString& rStr, const vcl::Font& rFont );

    void            SetSelectionFrameLines( const ::editeng::SvxBorderLine* pLine,
                                            bool bColorOnly );

    void            SetNoteText( const ScAddress& rPos, const OUString& rNoteText );
    void            ReplaceNote( const ScAddress& rPos, const OUString& rNoteText, const OUString* pAuthor, const OUString* pDate );
    void            DoRefConversion();

    void            DoHangulHanjaConversion();
    void            DoThesaurus();

    /** Generic implementation of sheet conversion functions. */
    void            DoSheetConversion( const ScConversionParam& rParam );

    void            SetPrintRanges( bool bEntireSheet,
                                    const OUString* pPrint,
                                    const OUString* pRepCol, const OUString* pRepRow,
                                    bool bAddPrint );

    void            DetectiveAddPred();
    void            DetectiveDelPred();
    void            DetectiveAddSucc();
    void            DetectiveDelSucc();
    void            DetectiveAddError();
    void            DetectiveMarkInvalid();
    void            DetectiveDelAll();
    void            DetectiveRefresh();
    void            DetectiveMarkPred();
    void            DetectiveMarkSucc();

    void            InsertCurrentTime(SvNumFormatType nCellFmt, const OUString& rUndoStr);

    void            ShowNote( bool bShow );
    void            EditNote();

    bool            SelectionEditable( bool* pOnlyNotBecauseOfMatrix = nullptr );

    SC_DLLPUBLIC void
                    DataFormPutData( SCROW nCurrentRow ,
                                     SCROW nStartRow , SCCOL nStartCol ,
                                     SCROW nEndRow , SCCOL nEndCol ,
                                     std::vector<VclPtr<Edit> >& aEdits,
                                     sal_uInt16 aColLength );
    void            UpdateSelectionArea( const ScMarkData& rSel, ScPatternAttr* pAttr = nullptr );

    void            OnLOKInsertDeleteColumn(SCCOL nStartCol, long nOffset);
    void            OnLOKInsertDeleteRow(SCROW nStartRow, long nOffset);
    void            OnLOKSetWidthOrHeight(SCCOLROW nStart, bool bWidth);

                                                // Internal helper functions
protected:
    static void     UpdateLineAttrs( ::editeng::SvxBorderLine&        rLine,
                                     const ::editeng::SvxBorderLine* pDestLine,
                                     const ::editeng::SvxBorderLine* pSrcLine,
                                     bool                 bColor );

private:
    void            PasteRTF( SCCOL nCol, SCROW nStartRow,
                                const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable );

    bool             PasteMultiRangesFromClip( InsertDeleteFlags nFlags, ScDocument* pClipDoc, ScPasteFunc nFunction,
                                   bool bSkipEmpty, bool bTranspose, bool bAsLink, bool bAllowDialogs,
                                   InsCellCmd eMoveMode, InsertDeleteFlags nUndoFlags );

    bool             PasteFromClipToMultiRanges( InsertDeleteFlags nFlags, ScDocument* pClipDoc, ScPasteFunc nFunction,
                                     bool bSkipEmpty, bool bTranspose, bool bAsLink, bool bAllowDialogs,
                                     InsCellCmd eMoveMode, InsertDeleteFlags nUndoFlags );

    void            PostPasteFromClip(const ScRangeList& rPasteRanges, const ScMarkData& rMark);

    sal_uInt16      GetOptimalColWidth( SCCOL nCol, SCTAB nTab, bool bFormula );

    void            StartFormatArea();
    bool            TestFormatArea( SCCOL nCol, SCROW nRow, SCTAB nTab, bool bAttrChanged );
    void            DoAutoAttributes( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                        bool bAttrChanged );

    void            MarkAndJumpToRanges(const ScRangeList& rRanges);
    void            CopyAutoSpellData( FillDir eDir, SCCOL nStartCol, SCROW nStartRow,
                                       SCCOL nEndCol, SCROW nEndRow, sal_uLong nCount );
};

extern bool bPasteIsDrop;
extern bool bPasteIsMove;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
