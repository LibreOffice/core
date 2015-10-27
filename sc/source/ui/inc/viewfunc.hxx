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

#include "tabbgcolor.hxx"

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

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
class Exchange;
class ScRangeList;
class SvxHyperlinkItem;
class ScTransferObj;
class ScTableProtection;

namespace editeng { class SvxBorderLine; }

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

    const ScPatternAttr*    GetSelectionPattern ();
    void                    GetSelectionFrame   ( SvxBoxItem&       rLineOuter,
                                                  SvxBoxInfoItem&   rLineInner );

    SvtScriptType   GetSelectionScriptType();

    bool            GetAutoSumArea(ScRangeList& rRangeList);
    void            EnterAutoSum(const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr);
    bool            AutoSum( const ScRange& rRange, bool bSubTotal, bool bSetCursor, bool bContinue );
    OUString        GetAutoSumFormula( const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr );

    void            EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab, const OUString& rString,
                               const EditTextObject* pData = NULL );
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

    SC_DLLPUBLIC void           CutToClip( ScDocument* pClipDoc = NULL, bool bIncludeObjects = false );
    SC_DLLPUBLIC bool           CopyToClip( ScDocument* pClipDoc = NULL, bool bCut = false, bool bApi = false,
                                            bool bIncludeObjects = false, bool bStopEdit = true );
    SC_DLLPUBLIC bool           CopyToClip( ScDocument* pClipDoc, const ScRangeList& rRange, bool bCut = false,
                                            bool bApi = false, bool bIncludeObjects = false, bool bStopEdit = true, bool bUseRangeForVBA = true );
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
                                        SCCOL nPosX, SCROW nPosY, Point* pLogicPos = NULL,
                                        bool bLink = false, bool bAllowDialogs = false );

    bool            PasteFile( const Point&, const OUString&, bool bLink=false );
    bool            PasteObject( const Point&, const css::uno::Reference < css::embed::XEmbeddedObject >&, const Size* = NULL, const Graphic* = NULL, const OUString& = OUString(), sal_Int64 nAspect = css::embed::Aspects::MSOLE_CONTENT );
    bool            PasteBitmapEx( const Point&, const BitmapEx& );
    bool            PasteMetaFile( const Point&, const GDIMetaFile& );
    bool            PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                    const OUString& rFile, const OUString& rFilter );
    bool            PasteBookmark( SotClipboardFormatId nFormatId,
                                const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable,
                                SCCOL nPosX, SCROW nPosY );
    bool            PasteLink( const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable );

    void            InsertBookmark( const OUString& rDescription, const OUString& rURL,
                                    SCCOL nPosX, SCROW nPosY, const OUString* pTarget = NULL,
                                    bool bTryReplace = false );
    bool            HasBookmarkAtCursor( SvxHyperlinkItem* pContent );

    bool            MoveBlockTo( const ScRange& rSource, const ScAddress& rDestPos,
                                    bool bCut, bool bRecord, bool bPaint, bool bApi );

    bool            LinkBlock( const ScRange& rSource, const ScAddress& rDestPos, bool bApi );

    void            CreateNames( sal_uInt16 nFlags );
    sal_uInt16      GetCreateNameFlags();
    void            InsertNameList();
    bool            InsertName( const OUString& rName, const OUString& rSymbol,
                                const OUString& rType );

    void            ApplyAttributes( const SfxItemSet* pDialogSet, const SfxItemSet* pOldSet,
                                        bool bRecord = true );
    void            ApplyAttr( const SfxPoolItem& rAttrItem );
    void            ApplySelectionPattern( const ScPatternAttr& rAttr,
                                            bool bRecord = true,
                                            bool bCursorOnly = false );
    void            ApplyPatternLines( const ScPatternAttr& rAttr,
                                        const SvxBoxItem* pNewOuter,
                                        const SvxBoxInfoItem* pNewInner, bool bRecord = true );

    void            ApplyUserItemSet( const SfxItemSet& rItemSet );

    const SfxStyleSheet*
                    GetStyleSheetFromMarked();
    void            SetStyleSheetToMarked( SfxStyleSheet* pStyleSheet,
                                                        bool bRecord = true );
    void            RemoveStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet );
    void            UpdateStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet );

    void            SetNumberFormat( short nFormatType, sal_uLong nAdd = 0 );
    void            SetNumFmtByStr( const OUString& rCode );
    void            ChangeNumFmtDecimals( bool bIncrement );

    void            SetValidation( const ScValidationData& rNew );

    void            ChangeIndent( bool bIncrement );

    void            ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect );

    void            Protect( SCTAB nTab, const OUString& rPassword );
    bool            Unprotect( SCTAB nTab, const OUString& rPassword );

    void            DeleteCells( DelCellCmd eCmd, bool bRecord = true );
    bool            InsertCells( InsCellCmd eCmd, bool bRecord = true, bool bPartOfPaste = false );
    void            DeleteMulti( bool bRows, bool bRecord = true );

    void            DeleteContents( InsertDeleteFlags nFlags, bool bRecord = true );

    void SetWidthOrHeight(
        bool bWidth, const std::vector<sc::ColRowSpan>& rRanges, ScSizeMode eMode,
        sal_uInt16 nSizeTwips, bool bRecord = true, bool bPaint = true, ScMarkData* pMarkData = NULL );

    void            SetMarkedWidthOrHeight( bool bWidth, ScSizeMode eMode, sal_uInt16 nSizeTwips,
                                        bool bRecord = true, bool bPaint = true );

    bool            AdjustBlockHeight( bool bPaint = true, ScMarkData* pMarkData = NULL );
    bool            AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, bool bPaint = true );

    void            ModifyCellSize( ScDirection eDir, bool bOptimal );

    SC_DLLPUBLIC void
                    InsertPageBreak( bool bColumn, bool bRecord = true,
                                        const ScAddress* pPos = NULL,
                                        bool bSetModified = true );
    SC_DLLPUBLIC void
                    DeletePageBreak( bool bColumn, bool bRecord = true,
                                        const ScAddress* pPos = NULL,
                                        bool bSetModified = true );

    void            RemoveManualBreaks();

    void            SetPrintZoom(sal_uInt16 nScale, sal_uInt16 nPages);
    void            AdjustPrintZoom();

    bool            TestMergeCells();
    bool            TestRemoveMerge();

    bool            MergeCells( bool bApi, bool& rDoContents, bool bRecord = true, bool bCenter = false );
    bool            RemoveMerge( bool bRecord = true );

    void            FillSimple( FillDir eDir, bool bRecord = true );
    void            FillSeries( FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                                double fStart, double fStep, double fMax, bool bRecord = true );
    void            FillAuto( FillDir eDir, SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, sal_uLong nCount, bool bRecord = true );
    void            FillCrossDblClick();
    void            ConvertFormulaToValue();

    void            TransliterateText( sal_Int32 nType );

    ScAutoFormatData* CreateAutoFormatData();
    void            AutoFormat( sal_uInt16 nFormatNo, bool bRecord = true );

    bool            SearchAndReplace( const SvxSearchItem* pSearchItem,
                                        bool bAddUndo, bool bIsApi );

    void            Solve( const ScSolveParam& rParam );
    void            TabOp( const ScTabOpParam& rParam,  bool bRecord = true );

    bool            InsertTable( const OUString& rName, SCTAB nTabNr, bool bRecord = true );
    bool            InsertTables(std::vector<OUString>& aNames, SCTAB nTab, SCTAB nCount, bool bRecord = true);

    bool            AppendTable( const OUString& rName, bool bRecord = true );

    bool            DeleteTable( SCTAB nTabNr, bool bRecord = true );
    bool            DeleteTables(const std::vector<SCTAB>& TheTabs, bool bRecord = true );
    bool                DeleteTables(SCTAB nTab, SCTAB nSheets);

    bool            RenameTable( const OUString& rName, SCTAB nTabNr );
    void            MoveTable( sal_uInt16 nDestDocNo, SCTAB nDestTab, bool bCopy, const OUString* pNewTabName = NULL );
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
                                        const OUString& rSource, sal_uLong nRefresh );

    void            ShowTable( const std::vector<OUString>& rNames );
    void            HideTable( const ScMarkData& rMark );

    void            MakeScenario( const OUString& rName, const OUString& rComment,
                                    const Color& rColor, sal_uInt16 nFlags );
    void            ExtendScenario();
    void            UseScenario( const OUString& rName );

    void            InsertSpecialChar( const OUString& rStr, const vcl::Font& rFont );

    void            SetSelectionFrameLines( const ::editeng::SvxBorderLine* pLine,
                                            bool bColorOnly );

    void            SetNoteText( const ScAddress& rPos, const OUString& rNoteText );
    void            ReplaceNote( const ScAddress& rPos, const OUString& rNoteText, const OUString* pAuthor, const OUString* pDate );
    void            DoRefConversion( bool bRecord = true );

    void            DoHangulHanjaConversion( bool bRecord = true );
    void            DoThesaurus( bool bRecord = true );

    /** Generic implementation of sheet conversion functions. */
    void            DoSheetConversion( const ScConversionParam& rParam, bool bRecord = true );

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

    void            InsertCurrentTime(short nCellFmt, const OUString& rUndoStr);

    void            ShowNote( bool bShow = true );
    void            EditNote();

    void            ForgetFormatArea()      { bFormatValid = false; }
    bool            SelectionEditable( bool* pOnlyNotBecauseOfMatrix = NULL );

    SC_DLLPUBLIC void
                    DataFormPutData( SCROW nCurrentRow ,
                                     SCROW nStartRow , SCCOL nStartCol ,
                                     SCROW nEndRow , SCCOL nEndCol ,
                                     std::vector<VclPtr<Edit> >& aEdits,
                                     sal_uInt16 aColLength );
    void            UpdateSelectionArea( const ScMarkData& rSel, ScPatternAttr* pAttr = NULL );
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
                                        bool bAttrChanged, bool bAddUndo );

    void            MarkAndJumpToRanges(const ScRangeList& rRanges);
    void            CopyAutoSpellData( FillDir eDir, SCCOL nStartCol, SCROW nStartRow,
                                       SCCOL nEndCol, SCROW nEndRow, sal_uLong nCount );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
