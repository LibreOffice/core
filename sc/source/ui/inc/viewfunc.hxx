/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef SC_VIEWFUNC_HXX
#define SC_VIEWFUNC_HXX

#include "tabview.hxx"

#include "tabbgcolor.hxx"

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>

#endif
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <vector>

class ScPatternAttr;
class ScAutoFormatData;
class SvxSearchItem;
class SfxItemSet;
class SvxBorderLine;
class SvxBoxItem;
class SvxBoxInfoItem;
class SfxStyleSheet;
class SfxPoolItem;
class EditTextObject;
struct ScSolveParam;
struct ScTabOpParam;
class ScPostIt;
class ScConditionalFormat;
class ScValidationData;
class ScConversionParam;
class SdrModel;
class Graphic;
class Exchange;
class ScRangeList;
class SvxHyperlinkItem;
class ScTransferObj;
class ScTableProtection;

namespace com { namespace sun { namespace star { namespace datatransfer { class XTransferable; } } } }

//==================================================================

class ScViewFunc : public ScTabView
{
private:
    ScAddress   aFormatSource;      // fuer automatisches Erweitern von Formatierung
    ScRange     aFormatArea;
    BOOL        bFormatValid;

public:
                    ScViewFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                    ~ScViewFunc();

    const ScPatternAttr*    GetSelectionPattern ();
    void                    GetSelectionFrame   ( SvxBoxItem&       rLineOuter,
                                                  SvxBoxInfoItem&   rLineInner );

    BYTE            GetSelectionScriptType();

    BOOL            GetAutoSumArea(ScRangeList& rRangeList);
    void            EnterAutoSum(const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr);
    bool            AutoSum( const ScRange& rRange, bool bSubTotal, bool bSetCursor, bool bContinue );
    String          GetAutoSumFormula( const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr );

    void            EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString,
                               BOOL bRecord = TRUE, const EditTextObject* pData = NULL );
    void            EnterValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rValue );
    void            EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab, const EditTextObject* pData,
                                BOOL bRecord = TRUE, BOOL bTestSimple = FALSE );

    void            EnterMatrix( const String& rString );
    void            EnterBlock( const String& rString, const EditTextObject* pData );

    void            EnterDataAtCursor( const String& rString );         //! nicht benutzt ?

    SC_DLLPUBLIC void           CutToClip( ScDocument* pClipDoc = NULL, BOOL bIncludeObjects = FALSE );
    SC_DLLPUBLIC BOOL           CopyToClip( ScDocument* pClipDoc = NULL, BOOL bCut = FALSE, BOOL bApi = FALSE,
                                BOOL bIncludeObjects = FALSE, BOOL bStopEdit = TRUE );
    SC_DLLPUBLIC BOOL           CopyToClip( ScDocument* pClipDoc, const ScRange& rRange, BOOL bCut = FALSE,
                                BOOL bApi = FALSE, BOOL bIncludeObjects = FALSE, BOOL bStopEdit = TRUE );
    ScTransferObj*              CopyToTransferable();
    SC_DLLPUBLIC BOOL           PasteFromClip( USHORT nFlags, ScDocument* pClipDoc,
                                    USHORT nFunction = PASTE_NOFUNC, BOOL bSkipEmpty = FALSE,
                                    BOOL bTranspose = FALSE, BOOL bAsLink = FALSE,
                                    InsCellCmd eMoveMode = INS_NONE,
                                    USHORT nUndoExtraFlags = IDF_NONE,
                                    BOOL bAllowDialogs = FALSE );

    void            FillTab( USHORT nFlags, USHORT nFunction, BOOL bSkipEmpty, BOOL bAsLink );

    SC_DLLPUBLIC void           PasteFromSystem();
    SC_DLLPUBLIC BOOL           PasteFromSystem( ULONG nFormatId, BOOL bApi = FALSE );
    void                        PasteFromTransferable( const ::com::sun::star::uno::Reference<
                                                       ::com::sun::star::datatransfer::XTransferable >& rxTransferable );

    void            PasteDraw();
    void            PasteDraw( const Point& rLogicPos, SdrModel* pModel,
                                BOOL bGroup = FALSE, BOOL bSameDocClipboard = FALSE );

    BOOL            PasteOnDrawObject( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                        SdrObject* pHitObj, BOOL bLink );

    BOOL            PasteDataFormat( ULONG nFormatId,
                                        const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                        SCCOL nPosX, SCROW nPosY, Point* pLogicPos = NULL,
                                        BOOL bLink = FALSE, BOOL bAllowDialogs = FALSE );

    BOOL            PasteFile( const Point&, const String&, BOOL bLink=FALSE );
    BOOL            PasteObject( const Point&, const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >&, const Size* = NULL, const Graphic* = NULL, const ::rtl::OUString& = ::rtl::OUString(), sal_Int64 nAspect = ::com::sun::star::embed::Aspects::MSOLE_CONTENT );
    BOOL            PasteBitmap( const Point&, const Bitmap& );
    BOOL            PasteMetaFile( const Point&, const GDIMetaFile& );
    BOOL            PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                    const String& rFile, const String& rFilter );
    BOOL            PasteBookmark( ULONG nFormatId,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                SCCOL nPosX, SCROW nPosY );
    BOOL            PasteDDE( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::datatransfer::XTransferable >& rxTransferable );

    BOOL            ApplyGraphicToObject( SdrObject* pObject, const Graphic& rGraphic );

    void            InsertBookmark( const String& rDescription, const String& rURL,
                                    SCCOL nPosX, SCROW nPosY, const String* pTarget = NULL,
                                    BOOL bTryReplace = FALSE );
    BOOL            HasBookmarkAtCursor( SvxHyperlinkItem* pContent );

    long            DropRequestHdl( Exchange* pExchange );
    BOOL            MoveBlockTo( const ScRange& rSource, const ScAddress& rDestPos,
                                    BOOL bCut, BOOL bRecord, BOOL bPaint, BOOL bApi );

    BOOL            LinkBlock( const ScRange& rSource, const ScAddress& rDestPos, BOOL bApi );

    void            CreateNames( USHORT nFlags );
    USHORT          GetCreateNameFlags();
    void            InsertNameList();
    BOOL            InsertName( const String& rName, const String& rSymbol,
                                const String& rType );

    void            ApplyAttributes( const SfxItemSet* pDialogSet, const SfxItemSet* pOldSet,
                                        BOOL bRecord = TRUE );
    void            ApplyAttr( const SfxPoolItem& rAttrItem );
    void            ApplySelectionPattern( const ScPatternAttr& rAttr,
                                            BOOL bRecord = TRUE,
                                            BOOL bCursorOnly = FALSE );
    void            ApplyPatternLines( const ScPatternAttr& rAttr,
                                        const SvxBoxItem* pNewOuter,
                                        const SvxBoxInfoItem* pNewInner, BOOL bRecord = TRUE );

    void            ApplyUserItemSet( const SfxItemSet& rItemSet );

    const SfxStyleSheet*    GetStyleSheetFromMarked();
    void                    SetStyleSheetToMarked( SfxStyleSheet* pStyleSheet,
                                                        BOOL bRecord = TRUE );
    void                    RemoveStyleSheetInUse( SfxStyleSheet* pStyleSheet );
    void                    UpdateStyleSheetInUse( SfxStyleSheet* pStyleSheet );

    void            SetNumberFormat( short nFormatType, ULONG nAdd = 0 );
    void            SetNumFmtByStr( const String& rCode );
    void            ChangeNumFmtDecimals( BOOL bIncrement );

    void            SetConditionalFormat( const ScConditionalFormat& rNew );
    void            SetValidation( const ScValidationData& rNew );

    void            ChangeIndent( BOOL bIncrement );

    void            ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect );

    void            Protect( SCTAB nTab, const String& rPassword );
    BOOL            Unprotect( SCTAB nTab, const String& rPassword );

    void            DeleteCells( DelCellCmd eCmd, BOOL bRecord = TRUE );
    BOOL            InsertCells( InsCellCmd eCmd, BOOL bRecord = TRUE, BOOL bPartOfPaste = FALSE );
    void            DeleteMulti( BOOL bRows, BOOL bRecord = TRUE );

    void            DeleteContents( USHORT nFlags, BOOL bRecord = TRUE );

    void            SetWidthOrHeight( BOOL bWidth, SCCOLROW nRangeCnt, SCCOLROW* pRanges,
                                        ScSizeMode eMode, USHORT nSizeTwips,
                                        BOOL bRecord = TRUE, BOOL bPaint = TRUE,
                                        ScMarkData* pMarkData = NULL );
    void            SetMarkedWidthOrHeight( BOOL bWidth, ScSizeMode eMode, USHORT nSizeTwips,
                                        BOOL bRecord = TRUE, BOOL bPaint = TRUE );
    void            ShowMarkedColumns( BOOL bShow, BOOL bRecord = TRUE );
    void            ShowMarkedRows( BOOL bShow, BOOL bRecord = TRUE );

    BOOL            AdjustBlockHeight( BOOL bPaint = TRUE, ScMarkData* pMarkData = NULL );
    BOOL            AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, BOOL bPaint = TRUE );

    void            ModifyCellSize( ScDirection eDir, BOOL bOptimal );

    SC_DLLPUBLIC void           InsertPageBreak( BOOL bColumn, BOOL bRecord = TRUE,
                                        const ScAddress* pPos = NULL,
                                        BOOL bSetModified = TRUE );
    SC_DLLPUBLIC void           DeletePageBreak( BOOL bColumn, BOOL bRecord = TRUE,
                                        const ScAddress* pPos = NULL,
                                        BOOL bSetModified = TRUE );

    void            RemoveManualBreaks();

    void            SetPrintZoom(USHORT nScale, USHORT nPages);
    void            AdjustPrintZoom();

    BOOL            TestMergeCells();
    BOOL            TestRemoveMerge();

    BOOL            MergeCells( BOOL bApi, BOOL& rDoContents, BOOL bRecord = TRUE, BOOL bCenter = FALSE );
    BOOL            RemoveMerge( BOOL bRecord = TRUE );

    void            FillSimple( FillDir eDir, BOOL bRecord = TRUE );
    void            FillSeries( FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                                double fStart, double fStep, double fMax, BOOL bRecord = TRUE );
    void            FillAuto( FillDir eDir, SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, ULONG nCount, BOOL bRecord = TRUE );
    void            FillCrossDblClick();

    void            TransliterateText( sal_Int32 nType );

    ScAutoFormatData* CreateAutoFormatData();
    void            AutoFormat( USHORT nFormatNo, BOOL bRecord = TRUE );

    void            SearchAndReplace( const SvxSearchItem* pSearchItem,
                                        BOOL bAddUndo, BOOL bIsApi );

    void            Solve( const ScSolveParam& rParam );
    void            TabOp( const ScTabOpParam& rParam,  BOOL bRecord = TRUE );

    BOOL            InsertTable( const String& rName, SCTAB nTabNr, BOOL bRecord = TRUE );
    BOOL            InsertTables(SvStrings *pNames, SCTAB nTab, SCTAB nCount, BOOL bRecord = TRUE);


    BOOL            AppendTable( const String& rName, BOOL bRecord = TRUE );

    BOOL            DeleteTable( SCTAB nTabNr, BOOL bRecord = TRUE );
    BOOL            DeleteTables(const std::vector<SCTAB>& TheTabs, BOOL bRecord = TRUE );

    BOOL            RenameTable( const String& rName, SCTAB nTabNr );
    void            MoveTable( USHORT nDestDocNo, SCTAB nDestTab, BOOL bCopy, const String* pNewTabName = NULL );
    void            ImportTables( ScDocShell* pSrcShell,
                                    SCTAB nCount, const SCTAB* pSrcTabs,
                                    BOOL bLink,SCTAB nTab);

    bool            SetTabBgColor( const Color& rColor, SCTAB nTabNr );
    bool            SetTabBgColor( ScUndoTabColorInfo::List& rUndoSetTabBgColorInfoList );

    void            InsertTableLink( const String& rFile,
                                        const String& rFilter, const String& rOptions,
                                        const String& rTabName );
    void            InsertAreaLink( const String& rFile,
                                        const String& rFilter, const String& rOptions,
                                        const String& rSource, ULONG nRefresh );

    void            ShowTable( const String& rName );
    void            HideTable( SCTAB nTabNr );

    void            MakeScenario( const String& rName, const String& rComment,
                                    const Color& rColor, USHORT nFlags );
    void            ExtendScenario();
    void            UseScenario( const String& rName );

    void            InsertSpecialChar( const String& rStr, const Font& rFont );

    void            InsertDummyObject();
    void            InsertOleObject();

    void            InsertDraw();

    void            SetSelectionFrameLines( const SvxBorderLine* pLine,
                                            BOOL bColorOnly );

    void            SetNoteText( const ScAddress& rPos, const String& rNoteText );
    void            ReplaceNote( const ScAddress& rPos, const String& rNoteText, const String* pAuthor, const String* pDate );
    void            DoRefConversion( BOOL bRecord = TRUE );

    void            DoHangulHanjaConversion( BOOL bRecord = TRUE );
    void            DoThesaurus( BOOL bRecord = TRUE );

    /** Generic implementation of sheet conversion functions. */
    void            DoSheetConversion( const ScConversionParam& rParam, BOOL bRecord = TRUE );

    void            SetPrintRanges( BOOL bEntireSheet,
                                    const String* pPrint,
                                    const String* pRepCol, const String* pRepRow,
                                    BOOL bAddPrint );

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

    void            InsertCurrentTime(short nCellFmt, const ::rtl::OUString& rUndoStr);

    void            ShowNote( bool bShow = true );
    inline void     HideNote() { ShowNote( false ); }
    void            EditNote();

    void            ForgetFormatArea()      { bFormatValid = FALSE; }
    BOOL            SelectionEditable( BOOL* pOnlyNotBecauseOfMatrix = NULL );

        // Amelia Wang
        SC_DLLPUBLIC void                   DataFormPutData( SCROW nCurrentRow ,
                                                             SCROW nStartRow , SCCOL nStartCol ,
                                                             SCROW nEndRow , SCCOL nEndCol ,
                                                             Edit** pEdits ,
                                                             sal_uInt16 aColLength );

                                                // interne Hilfsfunktionen
protected:
    void            UpdateLineAttrs( SvxBorderLine&       rLine,
                                     const SvxBorderLine* pDestLine,
                                     const SvxBorderLine* pSrcLine,
                                     BOOL                 bColor );


private:
    void            PasteRTF( SCCOL nCol, SCROW nStartRow,
                                const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::datatransfer::XTransferable >& rxTransferable );
    bool            PasteMultiRangesFromClip( sal_uInt16 nFlags, ScDocument* pClipDoc, sal_uInt16 nFunction,
                                              bool bSkipEmpty, bool bTranspos, bool bAsLink, bool bAllowDialogs,
                                              InsCellCmd eMoveMode, sal_uInt16 nCondFlags, sal_uInt16 nUndoFlags );
    void            PostPasteFromClip(const ScRange& rPasteRange, const ScMarkData& rMark);

    USHORT          GetOptimalColWidth( SCCOL nCol, SCTAB nTab, BOOL bFormula );

    void            StartFormatArea();
    BOOL            TestFormatArea( SCCOL nCol, SCROW nRow, SCTAB nTab, BOOL bAttrChanged );
    void            DoAutoAttributes( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                        BOOL bAttrChanged, BOOL bAddUndo );

    void            MarkAndJumpToRanges(const ScRangeList& rRanges);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
