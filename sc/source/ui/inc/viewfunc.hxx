/*************************************************************************
 *
 *  $RCSfile: viewfunc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-16 13:11:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_VIEWFUNC_HXX
#define SC_VIEWFUNC_HXX

#ifndef SC_TABVIEW_HXX
#include "tabview.hxx"
#endif

#ifndef _SVSTDARR_USHORTS

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#endif

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#endif

#define TABLEID_DOC     0xFFFF

class ScPatternAttr;
class ScAutoFormatData;
class SvxSearchItem;
class SfxItemSet;
class SvxBorderLine;
class SvxBoxItem;
class SvxBoxInfoItem;
class SfxStyleSheet;
class SfxPoolItem;
class SvDataObject;
class EditTextObject;
struct ScSolveParam;
struct ScTabOpParam;
class ScPostIt;
class ScConditionalFormat;
class ScValidationData;
class SdrModel;
class SvInPlaceObject;
class Graphic;
class Exchange;
class ScRangeList;
class SvxHyperlinkItem;

// ---------------------------------------------------------------------------

class ScViewFunc : public ScTabView
{
private:
    ScAddress   aFormatSource;      // fuer automatisches Erweitern von Formatierung
    ScRange     aFormatArea;
    BOOL        bFormatValid;

public:
                    ScViewFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                    ScViewFunc( Window* pParent, const ScViewFunc& rViewFunc, ScTabViewShell* pViewShell );
                    ~ScViewFunc();

    const ScPatternAttr*    GetSelectionPattern ();
    void                    GetSelectionFrame   ( SvxBoxItem&       rLineOuter,
                                                  SvxBoxInfoItem&   rLineInner );

    BOOL            GetAutoSumArea(ScRangeList& rRangeList);
    void            EnterAutoSum(const ScRangeList& rRangeList);

    void            EnterData( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString,
                                BOOL bRecord = TRUE );
    void            EnterData( USHORT nCol, USHORT nRow, USHORT nTab, const double& rValue );
    void            EnterData( USHORT nCol, USHORT nRow, USHORT nTab, const EditTextObject* pData,
                                BOOL bRecord = TRUE, BOOL bTestSimple = FALSE );

    void            EnterMatrix( const String& rString );
    void            EnterBlock( const String& rString, const EditTextObject* pData );

    void            EnterDataAtCursor( const String& rString );         //! nicht benutzt ?

    void            CutToClip( ScDocument* pClipDoc=NULL );
    void            CopyToClip( ScDocument* pClipDoc=NULL, BOOL bCut = FALSE );
    BOOL            PasteFromClip( USHORT nFlags, ScDocument* pClipDoc = NULL,
                                    USHORT nFunction = PASTE_NOFUNC, BOOL bSkipEmpty = FALSE,
                                    BOOL bTranspose = FALSE, BOOL bAsLink = FALSE,
                                    InsCellCmd eMoveMode = INS_NONE,
                                    USHORT nUndoExtraFlags = IDF_NONE );

    void            FillTab( USHORT nFlags, USHORT nFunction, BOOL bSkipEmpty, BOOL bAsLink );

    void            PasteFromSystem();
    BOOL            PasteFromSystem( ULONG nFormatId, BOOL bApi = FALSE );
//  void            ImportString(const String& rString, USHORT nStartCol, USHORT nStartRow);

    BOOL            DropFiles( const DropEvent& );
    BOOL            DropFile( const DropEvent&, const String& );
    BOOL            OpenFile( const String& );

    void            PasteDraw();
    void            PasteDraw( const Point& rLogicPos, SdrModel* pModel,
                                BOOL bGroup = FALSE );

    BOOL            PasteOnDrawObject( SvDataObject* pObject, SdrObject* pHitObj, BOOL bLink );
    BOOL            PasteDataObject( SvDataObject* pObject, USHORT nPosX, USHORT nPosY,
                                        Window* pWin = NULL, Point* pLogicPos = NULL );
    BOOL            LinkDataObject( SvDataObject* pObject, USHORT nPosX, USHORT nPosY,
                                        Window* pWin = NULL, Point* pLogicPos = NULL );
    BOOL            PasteDataFormat( ULONG nFormatId, SvDataObject* pObject,
                                        USHORT nPosX, USHORT nPosY,
                                        Window* pWin = NULL, Point* pLogicPos = NULL );
    BOOL            PasteFile( const Point&, const String&, BOOL bLink=FALSE );
    BOOL            PasteObject( const Point&, SvInPlaceObject*, SvDataObject* = NULL );
    BOOL            PasteBitmap( const Point&, const Bitmap& );
    BOOL            PasteMetaFile( const Point&, const GDIMetaFile& );
    BOOL            PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                    const String& rFile, const String& rFilter );
    BOOL            PasteBookmark( SvDataObject* pObject, USHORT nPosX, USHORT nPosY );
    BOOL            PasteDDE( SvDataObject* pObject );

    BOOL            ApplyGraphicToObject( SdrObject* pObject, const Graphic& rGraphic );

    void            InsertBookmark( const String& rDescription, const String& rURL,
                                    USHORT nPosX, USHORT nPosY, const String* pTarget = NULL );
    BOOL            HasBookmarkAtCursor( SvxHyperlinkItem* pContent );

    long            DropRequestHdl( Exchange* pExchange );
    void            MoveBlockTo( const ScRange& rSource, const ScAddress& rDestPos,
                                    BOOL bCut, BOOL bRecord = TRUE, BOOL bPaint = TRUE );

    void            LinkBlock( const ScRange& rSource, const ScAddress& rDestPos );

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

    void            Protect( USHORT nTab, const String& rPassword );
    BOOL            Unprotect( USHORT nTab, const String& rPassword );

    void            DeleteCells( DelCellCmd eCmd, BOOL bRecord = TRUE );
    BOOL            InsertCells( InsCellCmd eCmd, BOOL bRecord = TRUE );
    void            DeleteMulti( BOOL bRows, BOOL bRecord = TRUE );

    void            DeleteContents( USHORT nFlags, BOOL bRecord = TRUE );

    void            SetWidthOrHeight( BOOL bWidth, USHORT nRangeCnt, USHORT* pRanges,
                                        ScSizeMode eMode, USHORT nSizeTwips,
                                        BOOL bRecord = TRUE, BOOL bPaint = TRUE,
                                        ScMarkData* pMarkData = NULL );
    void            SetMarkedWidthOrHeight( BOOL bWidth, ScSizeMode eMode, USHORT nSizeTwips,
                                        BOOL bRecord = TRUE, BOOL bPaint = TRUE );
    void            ShowMarkedColumns( BOOL bShow, BOOL bRecord = TRUE );
    void            ShowMarkedRows( BOOL bShow, BOOL bRecord = TRUE );

    BOOL            AdjustBlockHeight( BOOL bPaint = TRUE, ScMarkData* pMarkData = NULL );
    BOOL            AdjustRowHeight( USHORT nStartRow, USHORT nEndRow, BOOL bPaint = TRUE );

    void            ModifyCellSize( ScDirection eDir, BOOL bOptimal );

    void            InsertPageBreak( BOOL bColumn, BOOL bRecord = TRUE,
                                        const ScAddress* pPos = NULL,
                                        BOOL bSetModified = TRUE );
    void            DeletePageBreak( BOOL bColumn, BOOL bRecord = TRUE,
                                        const ScAddress* pPos = NULL,
                                        BOOL bSetModified = TRUE );

    void            RemoveManualBreaks();

    void            SetPrintZoom(USHORT nScale, USHORT nPages);
    void            AdjustPrintZoom();

    BOOL            TestMergeCells();
    BOOL            TestRemoveMerge();

    BOOL            MergeCells( BOOL bApi, BOOL& rDoContents, BOOL bRecord = TRUE );
    BOOL            RemoveMerge( BOOL bRecord = TRUE );

    void            FillSimple( FillDir eDir, BOOL bRecord = TRUE );
    void            FillSeries( FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                                double fStart, double fStep, double fMax, BOOL bRecord = TRUE );
    void            FillAuto( FillDir eDir, USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow, USHORT nCount, BOOL bRecord = TRUE );

    ScAutoFormatData* CreateAutoFormatData();
    void            AutoFormat( USHORT nFormatNo, BOOL bRecord = TRUE );

    void            SearchAndReplace( const SvxSearchItem* pSearchItem,
                                        BOOL bAddUndo, BOOL bIsApi );

    void            Solve( const ScSolveParam& rParam );
    void            TabOp( const ScTabOpParam& rParam,  BOOL bRecord = TRUE );

    BOOL            InsertTable( const String& rName, USHORT nTabNr, BOOL bRecord = TRUE );
    BOOL            InsertTables(SvStrings *pNames, USHORT nTab, USHORT nCount, BOOL bRecord = TRUE);


    BOOL            AppendTable( const String& rName, BOOL bRecord = TRUE );

    BOOL            DeleteTable( USHORT nTabNr, BOOL bRecord = TRUE );
    BOOL            DeleteTables(const SvUShorts &TheTabs, BOOL bRecord = TRUE );

    BOOL            RenameTable( const String& rName, USHORT nTabNr );
    void            MoveTable( USHORT nDestDocNo, USHORT nDestTab, BOOL bCopy );
    void            ImportTables( ScDocShell* pSrcShell,
                                    USHORT nCount, const USHORT* pSrcTabs,
                                    BOOL bLink,USHORT nTab);

    void            InsertTableLink( const String& rFile,
                                        const String& rFilter, const String& rOptions,
                                        const String& rTabName );
    void            InsertAreaLink( const String& rFile,
                                        const String& rFilter, const String& rOptions,
                                        const String& rSource );

    void            ShowTable( const String& rName );
    void            HideTable( USHORT nTabNr );

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

    void            SetNote( USHORT nCol, USHORT nRow, USHORT nTab, const ScPostIt& rNote );
    void            DoSpellingChecker( BOOL bRecord = TRUE );
    void            DoThesaurus( BOOL bRecord = TRUE );
    DECL_LINK( SpellError, void * );

    void            SetPrintRanges( const String* pPrint,
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

    void            ShowNote();
    void            EditNote();
    void            HideNote();

    void            ForgetFormatArea()      { bFormatValid = FALSE; }
    BOOL            SelectionEditable( BOOL* pOnlyNotBecauseOfMatrix = NULL );

                                                // interne Hilfsfunktionen
protected:
    void            UpdateLineAttrs( SvxBorderLine&       rLine,
                                     const SvxBorderLine* pDestLine,
                                     const SvxBorderLine* pSrcLine,
                                     BOOL                 bColor );

    void            PaintWidthHeight( BOOL bColumns, USHORT nStart, USHORT nEnd );


private:
    void            PasteRTF( USHORT nCol, USHORT nStartRow, SvDataObject* pObject );
    USHORT          GetOptimalColWidth( USHORT nCol, USHORT nTab, BOOL bFormula );

    void            StartFormatArea();
    BOOL            TestFormatArea( USHORT nCol, USHORT nRow, USHORT nTab, BOOL bAttrChanged );
    void            DoAutoAttributes( USHORT nCol, USHORT nRow, USHORT nTab,
                                        BOOL bAttrChanged, BOOL bAddUndo );
};



#endif

