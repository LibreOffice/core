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
#ifndef SC_VIEWFUNC_HXX
#define SC_VIEWFUNC_HXX

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

namespace com { namespace sun { namespace star { namespace datatransfer { class XTransferable; } } } }

//==================================================================

class ScViewFunc : public ScTabView
{
private:
    ScAddress   aFormatSource;      // for automatic extension of formatting
    ScRange     aFormatArea;
    sal_Bool        bFormatValid;

public:
                    ScViewFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                    ~ScViewFunc();

    const ScPatternAttr*    GetSelectionPattern ();
    void                    GetSelectionFrame   ( SvxBoxItem&       rLineOuter,
                                                  SvxBoxInfoItem&   rLineInner );

    sal_uInt8           GetSelectionScriptType();

    sal_Bool            GetAutoSumArea(ScRangeList& rRangeList);
    void            EnterAutoSum(const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr);
    bool            AutoSum( const ScRange& rRange, bool bSubTotal, bool bSetCursor, bool bContinue );
    String          GetAutoSumFormula( const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr );

    void            EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString,
                               const EditTextObject* pData = NULL );
    void            EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab,
                               const EditTextObject& rData, bool bTestSimple = false );
    void            EnterValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rValue );

    void            EnterMatrix( const String& rString, ::formula::FormulaGrammar::Grammar eGram );

    /**
     * @param pData The caller must manage the life cycle of the object this
     *              pointer points to.  NULL is allowed.
     */
    void            EnterBlock( const String& rString, const EditTextObject* pData );

    void            EnterDataAtCursor( const String& rString );         //! Not used?

    SC_DLLPUBLIC void           CutToClip( ScDocument* pClipDoc = NULL, sal_Bool bIncludeObjects = false );
    SC_DLLPUBLIC sal_Bool           CopyToClip( ScDocument* pClipDoc = NULL, sal_Bool bCut = false, sal_Bool bApi = false,
                                sal_Bool bIncludeObjects = false, sal_Bool bStopEdit = true );
    SC_DLLPUBLIC sal_Bool           CopyToClip( ScDocument* pClipDoc, const ScRangeList& rRange, sal_Bool bCut = false,
                                sal_Bool bApi = false, sal_Bool bIncludeObjects = false, sal_Bool bStopEdit = true, sal_Bool bUseRangeForVBA = true );
    ScTransferObj*              CopyToTransferable();
    SC_DLLPUBLIC bool           PasteFromClip( sal_uInt16 nFlags, ScDocument* pClipDoc,
                                    sal_uInt16 nFunction = PASTE_NOFUNC, bool bSkipEmpty = false,
                                    bool bTranspose = false, bool bAsLink = false,
                                    InsCellCmd eMoveMode = INS_NONE,
                                    sal_uInt16 nUndoExtraFlags = IDF_NONE,
                                    bool bAllowDialogs = false );

    void            FillTab( sal_uInt16 nFlags, sal_uInt16 nFunction, sal_Bool bSkipEmpty, sal_Bool bAsLink );

    SC_DLLPUBLIC void           PasteFromSystem();
    SC_DLLPUBLIC sal_Bool           PasteFromSystem( sal_uLong nFormatId, sal_Bool bApi = false );
    void                        PasteFromTransferable( const ::com::sun::star::uno::Reference<
                                                       ::com::sun::star::datatransfer::XTransferable >& rxTransferable );

    void            PasteDraw();
    void            PasteDraw( const Point& rLogicPos, SdrModel* pModel,
                                sal_Bool bGroup = false, sal_Bool bSameDocClipboard = false );

    sal_Bool            PasteOnDrawObject( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                        SdrObject* pHitObj, sal_Bool bLink );

    sal_Bool            PasteDataFormat( sal_uLong nFormatId,
                                        const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                        SCCOL nPosX, SCROW nPosY, Point* pLogicPos = NULL,
                                        sal_Bool bLink = false, sal_Bool bAllowDialogs = false );

    sal_Bool            PasteFile( const Point&, const String&, sal_Bool bLink=false );
    sal_Bool            PasteObject( const Point&, const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >&, const Size* = NULL, const Graphic* = NULL, const OUString& = OUString(), sal_Int64 nAspect = ::com::sun::star::embed::Aspects::MSOLE_CONTENT );
    sal_Bool            PasteBitmapEx( const Point&, const BitmapEx& );
    sal_Bool            PasteMetaFile( const Point&, const GDIMetaFile& );
    sal_Bool            PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                    const String& rFile, const String& rFilter );
    sal_Bool            PasteBookmark( sal_uLong nFormatId,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                SCCOL nPosX, SCROW nPosY );
    bool            PasteLink( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::datatransfer::XTransferable >& rxTransferable );

    sal_Bool            ApplyGraphicToObject( SdrObject* pObject, const Graphic& rGraphic );

    void            InsertBookmark( const String& rDescription, const String& rURL,
                                    SCCOL nPosX, SCROW nPosY, const String* pTarget = NULL,
                                    sal_Bool bTryReplace = false );
    bool HasBookmarkAtCursor( SvxHyperlinkItem* pContent );

    long            DropRequestHdl( Exchange* pExchange );
    sal_Bool            MoveBlockTo( const ScRange& rSource, const ScAddress& rDestPos,
                                    sal_Bool bCut, sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );

    sal_Bool            LinkBlock( const ScRange& rSource, const ScAddress& rDestPos, sal_Bool bApi );

    void            CreateNames( sal_uInt16 nFlags );
    sal_uInt16          GetCreateNameFlags();
    void            InsertNameList();
    sal_Bool            InsertName( const String& rName, const String& rSymbol,
                                const OUString& rType );

    void            ApplyAttributes( const SfxItemSet* pDialogSet, const SfxItemSet* pOldSet,
                                        sal_Bool bRecord = sal_True );
    void            ApplyAttr( const SfxPoolItem& rAttrItem );
    void            ApplySelectionPattern( const ScPatternAttr& rAttr,
                                            sal_Bool bRecord = sal_True,
                                            sal_Bool bCursorOnly = false );
    void            ApplyPatternLines( const ScPatternAttr& rAttr,
                                        const SvxBoxItem* pNewOuter,
                                        const SvxBoxInfoItem* pNewInner, sal_Bool bRecord = sal_True );

    void            ApplyUserItemSet( const SfxItemSet& rItemSet );

    const SfxStyleSheet*    GetStyleSheetFromMarked();
    void                    SetStyleSheetToMarked( SfxStyleSheet* pStyleSheet,
                                                        sal_Bool bRecord = sal_True );
    void                    RemoveStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet );
    void                    UpdateStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet );

    void            SetNumberFormat( short nFormatType, sal_uLong nAdd = 0 );
    void            SetNumFmtByStr( const String& rCode );
    void            ChangeNumFmtDecimals( sal_Bool bIncrement );

    void            SetValidation( const ScValidationData& rNew );

    void            ChangeIndent( sal_Bool bIncrement );

    void            ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect );

    void            Protect( SCTAB nTab, const String& rPassword );
    sal_Bool            Unprotect( SCTAB nTab, const String& rPassword );

    void            DeleteCells( DelCellCmd eCmd, sal_Bool bRecord = sal_True );
    sal_Bool            InsertCells( InsCellCmd eCmd, sal_Bool bRecord = sal_True, sal_Bool bPartOfPaste = false );
    void            DeleteMulti( sal_Bool bRows, sal_Bool bRecord = sal_True );

    void            DeleteContents( sal_uInt16 nFlags, sal_Bool bRecord = sal_True );

    void            SetWidthOrHeight( sal_Bool bWidth, SCCOLROW nRangeCnt, SCCOLROW* pRanges,
                                        ScSizeMode eMode, sal_uInt16 nSizeTwips,
                                        sal_Bool bRecord = sal_True, sal_Bool bPaint = sal_True,
                                        ScMarkData* pMarkData = NULL );
    void            SetMarkedWidthOrHeight( sal_Bool bWidth, ScSizeMode eMode, sal_uInt16 nSizeTwips,
                                        sal_Bool bRecord = sal_True, sal_Bool bPaint = sal_True );
    void            ShowMarkedColumns( sal_Bool bShow, sal_Bool bRecord = sal_True );
    void            ShowMarkedRows( sal_Bool bShow, sal_Bool bRecord = sal_True );

    sal_Bool            AdjustBlockHeight( sal_Bool bPaint = sal_True, ScMarkData* pMarkData = NULL );
    sal_Bool            AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, sal_Bool bPaint = sal_True );

    void            ModifyCellSize( ScDirection eDir, sal_Bool bOptimal );

    SC_DLLPUBLIC void           InsertPageBreak( sal_Bool bColumn, sal_Bool bRecord = sal_True,
                                        const ScAddress* pPos = NULL,
                                        sal_Bool bSetModified = sal_True );
    SC_DLLPUBLIC void           DeletePageBreak( sal_Bool bColumn, sal_Bool bRecord = sal_True,
                                        const ScAddress* pPos = NULL,
                                        sal_Bool bSetModified = sal_True );

    void            RemoveManualBreaks();

    void            SetPrintZoom(sal_uInt16 nScale, sal_uInt16 nPages);
    void            AdjustPrintZoom();

    sal_Bool            TestMergeCells();
    sal_Bool            TestRemoveMerge();

    sal_Bool            MergeCells( sal_Bool bApi, sal_Bool& rDoContents, sal_Bool bRecord = true, sal_Bool bCenter = false );
    sal_Bool            RemoveMerge( sal_Bool bRecord = true );

    void            FillSimple( FillDir eDir, bool bRecord = true );
    void            FillSeries( FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                                double fStart, double fStep, double fMax, sal_Bool bRecord = sal_True );
    void            FillAuto( FillDir eDir, SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, sal_uLong nCount, sal_Bool bRecord = sal_True );
    void            FillCrossDblClick();

    void            TransliterateText( sal_Int32 nType );

    ScAutoFormatData* CreateAutoFormatData();
    void            AutoFormat( sal_uInt16 nFormatNo, sal_Bool bRecord = sal_True );

    void            SearchAndReplace( const SvxSearchItem* pSearchItem,
                                        sal_Bool bAddUndo, sal_Bool bIsApi );

    void            Solve( const ScSolveParam& rParam );
    void            TabOp( const ScTabOpParam& rParam,  sal_Bool bRecord = sal_True );

    sal_Bool            InsertTable( const String& rName, SCTAB nTabNr, sal_Bool bRecord = sal_True );
    sal_Bool            InsertTables(std::vector<OUString>& aNames, SCTAB nTab, SCTAB nCount, sal_Bool bRecord = sal_True);


    sal_Bool            AppendTable( const String& rName, sal_Bool bRecord = sal_True );

    sal_Bool            DeleteTable( SCTAB nTabNr, sal_Bool bRecord = true );
    sal_Bool            DeleteTables(const std::vector<SCTAB>& TheTabs, sal_Bool bRecord = true );
    bool                DeleteTables(SCTAB nTab, SCTAB nSheets);

    sal_Bool            RenameTable( const String& rName, SCTAB nTabNr );
    void MoveTable( sal_uInt16 nDestDocNo, SCTAB nDestTab, bool bCopy, const OUString* pNewTabName = NULL );
    void            ImportTables( ScDocShell* pSrcShell,
                                    SCTAB nCount, const SCTAB* pSrcTabs,
                                    sal_Bool bLink,SCTAB nTab);

    bool            SetTabBgColor( const Color& rColor, SCTAB nTabNr );
    bool            SetTabBgColor( ScUndoTabColorInfo::List& rUndoSetTabBgColorInfoList );

    void            InsertTableLink( const String& rFile,
                                        const String& rFilter, const String& rOptions,
                                        const String& rTabName );
    void            InsertAreaLink( const String& rFile,
                                        const String& rFilter, const String& rOptions,
                                        const String& rSource, sal_uLong nRefresh );

    void            ShowTable( const std::vector<String>& rNames );
    void            HideTable( const ScMarkData& rMark );

    void            MakeScenario( const String& rName, const String& rComment,
                                    const Color& rColor, sal_uInt16 nFlags );
    void            ExtendScenario();
    void            UseScenario( const String& rName );

    void            InsertSpecialChar( const String& rStr, const Font& rFont );

    void            InsertDummyObject();
    void            InsertOleObject();

    void            InsertDraw();

    void            SetSelectionFrameLines( const ::editeng::SvxBorderLine* pLine,
                                            sal_Bool bColorOnly );

    void            SetNoteText( const ScAddress& rPos, const String& rNoteText );
    void            ReplaceNote( const ScAddress& rPos, const OUString& rNoteText, const OUString* pAuthor, const OUString* pDate );
    void            DoRefConversion( sal_Bool bRecord = sal_True );

    void            DoHangulHanjaConversion( sal_Bool bRecord = true );
    void            DoThesaurus( sal_Bool bRecord = true );

    /** Generic implementation of sheet conversion functions. */
    void            DoSheetConversion( const ScConversionParam& rParam, sal_Bool bRecord = sal_True );

    void            SetPrintRanges( sal_Bool bEntireSheet,
                                    const String* pPrint,
                                    const String* pRepCol, const String* pRepRow,
                                    sal_Bool bAddPrint );

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
    inline void     HideNote() { ShowNote( false ); }
    void            EditNote();

    void            ForgetFormatArea()      { bFormatValid = false; }
    sal_Bool            SelectionEditable( bool* pOnlyNotBecauseOfMatrix = NULL );

        SC_DLLPUBLIC void                   DataFormPutData( SCROW nCurrentRow ,
                                                             SCROW nStartRow , SCCOL nStartCol ,
                                                             SCROW nEndRow , SCCOL nEndCol ,
                                                             boost::ptr_vector<boost::nullable<Edit> >& aEdits,
                                                             sal_uInt16 aColLength );
    void            UpdateSelectionArea( const ScMarkData& rSel, ScPatternAttr* pAttr = NULL );
                                                // Internal helper functions
protected:
    void            UpdateLineAttrs( ::editeng::SvxBorderLine&        rLine,
                                     const ::editeng::SvxBorderLine* pDestLine,
                                     const ::editeng::SvxBorderLine* pSrcLine,
                                     sal_Bool                 bColor );


private:
    void            PasteRTF( SCCOL nCol, SCROW nStartRow,
                                const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::datatransfer::XTransferable >& rxTransferable );

    bool PasteMultiRangesFromClip( sal_uInt16 nFlags, ScDocument* pClipDoc, sal_uInt16 nFunction,
                                   bool bSkipEmpty, bool bTranspose, bool bAsLink, bool bAllowDialogs,
                                   InsCellCmd eMoveMode, sal_uInt16 nUndoFlags );

    bool PasteFromClipToMultiRanges( sal_uInt16 nFlags, ScDocument* pClipDoc, sal_uInt16 nFunction,
                                     bool bSkipEmpty, bool bTranspose, bool bAsLink, bool bAllowDialogs,
                                     InsCellCmd eMoveMode, sal_uInt16 nUndoFlags );

    void            PostPasteFromClip(const ScRangeList& rPasteRanges, const ScMarkData& rMark);

    sal_uInt16          GetOptimalColWidth( SCCOL nCol, SCTAB nTab, sal_Bool bFormula );

    void            StartFormatArea();
    sal_Bool            TestFormatArea( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bAttrChanged );
    void            DoAutoAttributes( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                        sal_Bool bAttrChanged, sal_Bool bAddUndo );

    void            MarkAndJumpToRanges(const ScRangeList& rRanges);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
