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

#ifndef _SVSTDARR_SHORTS
#define _SVSTDARR_SHORTS
#include <svl/svstdarr.hxx>

#endif

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>

#endif
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>

class ScPatternAttr;
class ScAutoFormatData;
class SvxSearchItem;
class SfxItemSet;
class SvxBorderLine;
class SvxBoxItem;
class SvxBoxInfoItem;
class SfxStyleSheetBase;
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
    sal_Bool        bFormatValid;

public:
                    ScViewFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
//UNUSED2008-05     ScViewFunc( Window* pParent, const ScViewFunc& rViewFunc, ScTabViewShell* pViewShell );
                    ~ScViewFunc();

    const ScPatternAttr*    GetSelectionPattern ();
    void                    GetSelectionFrame   ( SvxBoxItem&       rLineOuter,
                                                  SvxBoxInfoItem&   rLineInner );

    sal_uInt8           GetSelectionScriptType();

    sal_Bool            GetAutoSumArea(ScRangeList& rRangeList);
    void            EnterAutoSum(const ScRangeList& rRangeList, sal_Bool bSubTotal);
    bool            AutoSum( const ScRange& rRange, bool bSubTotal, bool bSetCursor, bool bContinue );
    String          GetAutoSumFormula( const ScRangeList& rRangeList, bool bSubTotal );

    void            EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString,
                               sal_Bool bRecord = sal_True, const EditTextObject* pData = NULL );
    void            EnterValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rValue );
    void            EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab, const EditTextObject* pData,
                                sal_Bool bRecord = sal_True, sal_Bool bTestSimple = sal_False );

    void            EnterMatrix( const String& rString );
    void            EnterBlock( const String& rString, const EditTextObject* pData );

    void            EnterDataAtCursor( const String& rString );         //! nicht benutzt ?

    SC_DLLPUBLIC void           CutToClip( ScDocument* pClipDoc = NULL, sal_Bool bIncludeObjects = sal_False );
    SC_DLLPUBLIC sal_Bool           CopyToClip( ScDocument* pClipDoc = NULL, sal_Bool bCut = sal_False, sal_Bool bApi = sal_False,
                                sal_Bool bIncludeObjects = sal_False, sal_Bool bStopEdit = sal_True );
    ScTransferObj*              CopyToTransferable();
    SC_DLLPUBLIC sal_Bool           PasteFromClip( sal_uInt16 nFlags, ScDocument* pClipDoc,
                                    sal_uInt16 nFunction = PASTE_NOFUNC, sal_Bool bSkipEmpty = sal_False,
                                    sal_Bool bTranspose = sal_False, sal_Bool bAsLink = sal_False,
                                    InsCellCmd eMoveMode = INS_NONE,
                                    sal_uInt16 nUndoExtraFlags = IDF_NONE,
                                    sal_Bool bAllowDialogs = sal_False );

    void            FillTab( sal_uInt16 nFlags, sal_uInt16 nFunction, sal_Bool bSkipEmpty, sal_Bool bAsLink );

    SC_DLLPUBLIC void           PasteFromSystem();
    SC_DLLPUBLIC sal_Bool           PasteFromSystem( sal_uLong nFormatId, sal_Bool bApi = sal_False );
    void                        PasteFromTransferable( const ::com::sun::star::uno::Reference<
                                                       ::com::sun::star::datatransfer::XTransferable >& rxTransferable );

    void            PasteDraw();
    void            PasteDraw( const Point& rLogicPos, SdrModel* pModel,
                                sal_Bool bGroup = sal_False, sal_Bool bSameDocClipboard = sal_False );

    sal_Bool            PasteOnDrawObject( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                        SdrObject* pHitObj, sal_Bool bLink );

    sal_Bool            PasteDataFormat( sal_uLong nFormatId,
                                        const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                        SCCOL nPosX, SCROW nPosY, Point* pLogicPos = NULL,
                                        sal_Bool bLink = sal_False, sal_Bool bAllowDialogs = sal_False );

    sal_Bool            PasteFile( const Point&, const String&, sal_Bool bLink=sal_False );
    sal_Bool            PasteObject( const Point&, const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >&, const Size* = NULL, const Graphic* = NULL, const ::rtl::OUString& = ::rtl::OUString(), sal_Int64 nAspect = ::com::sun::star::embed::Aspects::MSOLE_CONTENT );
    sal_Bool            PasteBitmap( const Point&, const Bitmap& );
    sal_Bool            PasteMetaFile( const Point&, const GDIMetaFile& );
    sal_Bool            PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                    const String& rFile, const String& rFilter );
    sal_Bool            PasteBookmark( sal_uLong nFormatId,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                SCCOL nPosX, SCROW nPosY );
    sal_Bool            PasteDDE( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::datatransfer::XTransferable >& rxTransferable );

    sal_Bool            ApplyGraphicToObject( SdrObject* pObject, const Graphic& rGraphic );

    void            InsertBookmark( const String& rDescription, const String& rURL,
                                    SCCOL nPosX, SCROW nPosY, const String* pTarget = NULL,
                                    sal_Bool bTryReplace = sal_False );
    sal_Bool            HasBookmarkAtCursor( SvxHyperlinkItem* pContent );

    long            DropRequestHdl( Exchange* pExchange );
    sal_Bool            MoveBlockTo( const ScRange& rSource, const ScAddress& rDestPos,
                                    sal_Bool bCut, sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );

    sal_Bool            LinkBlock( const ScRange& rSource, const ScAddress& rDestPos, sal_Bool bApi );

    void            CreateNames( sal_uInt16 nFlags );
    sal_uInt16          GetCreateNameFlags();
    void            InsertNameList();
    sal_Bool            InsertName( const String& rName, const String& rSymbol,
                                const String& rType );

    void            ApplyAttributes( const SfxItemSet* pDialogSet, const SfxItemSet* pOldSet,
                                        sal_Bool bRecord = sal_True );
    void            ApplyAttr( const SfxPoolItem& rAttrItem );
    void            ApplySelectionPattern( const ScPatternAttr& rAttr,
                                            sal_Bool bRecord = sal_True,
                                            sal_Bool bCursorOnly = sal_False );
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

    void            SetConditionalFormat( const ScConditionalFormat& rNew );
    void            SetValidation( const ScValidationData& rNew );

    void            ChangeIndent( sal_Bool bIncrement );

    void            ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect );

    void            Protect( SCTAB nTab, const String& rPassword );
    sal_Bool            Unprotect( SCTAB nTab, const String& rPassword );

    void            DeleteCells( DelCellCmd eCmd, sal_Bool bRecord = sal_True );
    sal_Bool            InsertCells( InsCellCmd eCmd, sal_Bool bRecord = sal_True, sal_Bool bPartOfPaste = sal_False );
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

    sal_Bool            MergeCells( sal_Bool bApi, sal_Bool& rDoContents, sal_Bool bRecord = sal_True );
    sal_Bool            RemoveMerge( sal_Bool bRecord = sal_True );

    void            FillSimple( FillDir eDir, sal_Bool bRecord = sal_True );
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
    sal_Bool            InsertTables(SvStrings *pNames, SCTAB nTab, SCTAB nCount, sal_Bool bRecord = sal_True);


    sal_Bool            AppendTable( const String& rName, sal_Bool bRecord = sal_True );

    sal_Bool            DeleteTable( SCTAB nTabNr, sal_Bool bRecord = sal_True );
    sal_Bool            DeleteTables(const SvShorts &TheTabs, sal_Bool bRecord = sal_True );

    sal_Bool            RenameTable( const String& rName, SCTAB nTabNr );
    void            MoveTable( sal_uInt16 nDestDocNo, SCTAB nDestTab, sal_Bool bCopy );
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

    void            ShowTable( const String& rName );
    void            HideTable( SCTAB nTabNr );

    void            MakeScenario( const String& rName, const String& rComment,
                                    const Color& rColor, sal_uInt16 nFlags );
    void            ExtendScenario();
    void            UseScenario( const String& rName );

    void            InsertSpecialChar( const String& rStr, const Font& rFont );

    void            InsertDummyObject();
    void            InsertOleObject();

    void            InsertDraw();

    void            SetSelectionFrameLines( const SvxBorderLine* pLine,
                                            sal_Bool bColorOnly );

    void            SetNoteText( const ScAddress& rPos, const String& rNoteText );
    void            ReplaceNote( const ScAddress& rPos, const String& rNoteText, const String* pAuthor, const String* pDate );
    void            DoRefConversion( sal_Bool bRecord = sal_True );

//UNUSED2008-05  void            DoSpellingChecker( sal_Bool bRecord = sal_True );
    void            DoHangulHanjaConversion( sal_Bool bRecord = sal_True );
    void            DoThesaurus( sal_Bool bRecord = sal_True );
//UNUSED2008-05  DECL_LINK( SpellError, void * );

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

    void            ShowNote( bool bShow = true );
    inline void     HideNote() { ShowNote( false ); }
    void            EditNote();

    void            ForgetFormatArea()      { bFormatValid = sal_False; }
    sal_Bool            SelectionEditable( sal_Bool* pOnlyNotBecauseOfMatrix = NULL );

                                                // interne Hilfsfunktionen
protected:
    void            UpdateLineAttrs( SvxBorderLine&       rLine,
                                     const SvxBorderLine* pDestLine,
                                     const SvxBorderLine* pSrcLine,
                                     sal_Bool                 bColor );

//UNUSED2008-05  void            PaintWidthHeight( sal_Bool bColumns, SCCOLROW nStart, SCCOLROW nEnd );


private:
    void            PasteRTF( SCCOL nCol, SCROW nStartRow,
                                const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::datatransfer::XTransferable >& rxTransferable );
    bool            PasteMultiRangesFromClip( sal_uInt16 nFlags, ScDocument* pClipDoc, sal_uInt16 nFunction,
                                              bool bSkipEmpty, bool bTranspos, bool bAsLink, bool bAllowDialogs,
                                              InsCellCmd eMoveMode, sal_uInt16 nCondFlags, sal_uInt16 nUndoFlags );
    void            PostPasteFromClip(const ScRange& rPasteRange, const ScMarkData& rMark);

    sal_uInt16          GetOptimalColWidth( SCCOL nCol, SCTAB nTab, sal_Bool bFormula );

    void            StartFormatArea();
    sal_Bool            TestFormatArea( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bAttrChanged );
    void            DoAutoAttributes( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                        sal_Bool bAttrChanged, sal_Bool bAddUndo );
};



#endif

