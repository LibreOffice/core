/*************************************************************************
 *
 *  $RCSfile: impedit.hxx,v $
 *
 *  $Revision: 1.63 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:01:11 $
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

#ifndef _IMPEDIT_HXX
#define _IMPEDIT_HXX

#include <editdoc.hxx>
#include <editsel.hxx>
#include <editundo.hxx>
#include <editobj2.hxx>
#include <editstt2.hxx>
#include <editdata.hxx>
#include <svxacorr.hxx>

#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_CURSOR_HXX //autogen
#include <vcl/cursor.hxx>
#endif

#include <vcl/dndhelp.hxx>

#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLALTERNATIVES_HPP_
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_SPELLFAILURE_HPP_
#include <com/sun/star/linguistic2/SpellFailure.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLCHECKER_HPP_
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATOR_HPP_
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_CHARACTERITERATORMODE_HPP_
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#include <vos/ref.hxx>

DBG_NAMEEX( EditView );
DBG_NAMEEX( EditEngine );

#define PIMPEE      pImpEditView->pEditEngine->pImpEditEngine

#define DEL_LEFT    1
#define DEL_RIGHT   2
#define TRAVEL_X_DONTKNOW   0xFFFFFFFF
#define CURSOR_BIDILEVEL_DONTKNOW   0xFFFF
#define MAXCHARSINPARA      0x3FFF-CHARPOSGROW  // Max 16K, because WYSIWYG array

#define ATTRSPECIAL_WHOLEWORD   1
#define ATTRSPECIAL_EDGE        2

#define GETCRSR_TXTONLY             0x0001
#define GETCRSR_STARTOFLINE         0x0002
#define GETCRSR_ENDOFLINE           0x0004
#define GETCRSR_PREFERPORTIONSTART  0x0008

#define LINE_SEP    0x0A

typedef EENotify* EENotifyPtr;
SV_DECL_PTRARR_DEL( NotifyList, EENotifyPtr, 1, 1 );    // IMPL is in outliner.cxx, move to EE later and share declaration, or use BlockNotifications from EE directly


class EditView;
class EditEngine;
class SvxFontTable;
class SvxColorList;

class SvxSearchItem;
class SvxLRSpaceItem;
class TextRanger;
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
class SvtCTLOptions;


class SvUShorts;

namespace com {
namespace sun {
namespace star {
namespace datatransfer {
namespace clipboard {
    class XClipboard;
}}}}}

namespace svtools {
    class ColorConfig;
}

struct DragAndDropInfo
{
    Rectangle           aCurCursor;
    Rectangle           aCurSavedCursor;
    sal_uInt16          nSensibleRange;
    sal_uInt16          nCursorWidth;
    ESelection          aBeginDragSel;
    EditPaM             aDropDest;
    USHORT              nOutlinerDropDest;
    ESelection          aDropSel;
    VirtualDevice*      pBackground;
    const SvxFieldItem* pField;
    sal_Bool            bVisCursor              : 1;
    sal_Bool            bDroppedInMe            : 1;
    sal_Bool            bStarterOfDD            : 1;
    sal_Bool            bHasValidData           : 1;
    sal_Bool            bUndoAction             : 1;
    sal_Bool            bOutlinerMode           : 1;
    sal_Bool            bDragAccepted           : 1;

    DragAndDropInfo()
    {
            pBackground = NULL; bVisCursor = sal_False; bDroppedInMe = sal_False; bStarterOfDD = sal_False;
            bHasValidData = sal_False; bUndoAction = sal_False; bOutlinerMode = sal_False;
            nSensibleRange = 0; nCursorWidth = 0; pField = 0; nOutlinerDropDest = 0; bDragAccepted = sal_False;
    }
};

struct ImplIMEInfos
{
    String      aOldTextAfterStartPos;
    sal_uInt16* pAttribs;
    EditPaM     aPos;
    sal_uInt16  nLen;
    sal_Bool    bCursor;
    sal_Bool    bWasCursorOverwrite;

            ImplIMEInfos( const EditPaM& rPos, const String& rOldTextAfterStartPos );
            ~ImplIMEInfos();

    void    CopyAttribs( const sal_uInt16* pA, sal_uInt16 nL );
    void    DestroyAttribs();
};

struct SpellInfo
{
    EESpellState    eState;
    EPaM            aSpellStart;
    EPaM            aSpellTo;
    sal_Bool        bSpellToEnd;
    sal_Bool        bMultipleDoc;

    SpellInfo()
        { bSpellToEnd = sal_True; eState = EE_SPELL_OK; bMultipleDoc = sal_False; }
};

struct FormatterFontMetric
{
    sal_uInt16 nMaxAscent;
    sal_uInt16 nMaxDescent;

    FormatterFontMetric()               { nMaxAscent = 0; nMaxDescent = 0; /* nMinLeading = 0xFFFF; */ }
    sal_uInt16  GetHeight() const       { return nMaxAscent+nMaxDescent; }
};

class IdleFormattter : public Timer
{
private:
    EditView*   pView;
    int         nRestarts;

public:
                IdleFormattter();
                ~IdleFormattter();

    void        DoIdleFormat( EditView* pV );
    void        ForceTimeout();
    void        ResetRestarts() { nRestarts = 0; }
    EditView*   GetView()       { return pView; }
};

// ----------------------------------------------------------------------
//  class ImpEditView
//  ----------------------------------------------------------------------
class ImpEditView : public vcl::unohelper::DragAndDropClient
{
    friend class EditView;
    friend class EditEngine;
    friend class ImpEditEngine;

private:
    EditView*           pEditView;
    Cursor*             pCursor;
    Color*              pBackgroundColor;
    EditEngine*         pEditEngine;
    Window*             pOutWin;
    Pointer*            pPointer;
    DragAndDropInfo*    pDragAndDropInfo;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener > mxDnDListener;


    long                nInvMore;
    sal_uInt32          nControl;
    sal_uInt32          nTravelXPos;
    sal_uInt16          nExtraCursorFlags;
    sal_uInt16          nCursorBidiLevel;
    sal_uInt16          nScrollDiffX;
    sal_Bool            bReadOnly;
    sal_Bool            bClickedInSelection;
    sal_Bool            bActiveDragAndDropListener;

    Point               aAnchorPoint;
    Rectangle           aOutArea;
    Point               aVisDocStartPos;
    EESelectionMode     eSelectionMode;
    EditSelection       aEditSelection;
    EVAnchorMode        eAnchorMode;

protected:

    // DragAndDropClient
    void dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge ) throw (::com::sun::star::uno::RuntimeException);
    void dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde ) throw (::com::sun::star::uno::RuntimeException);
    void drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);
    void dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (::com::sun::star::uno::RuntimeException);
    void dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException);
    void dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException);

    void ShowDDCursor( const Rectangle& rRect );
    void HideDDCursor();

    void ImplDrawHighlightRect( Window* pOutWin, const Point& rDocPosTopLeft, const Point& rDocPosBottomRight, PolyPolygon* pPolyPoly );

public:
                    ImpEditView( EditView* pView, EditEngine* pEng, Window* pWindow );
                    ~ImpEditView();

    EditView*       GetEditViewPtr() { return pEditView; }

    sal_uInt16      GetScrollDiffX() const          { return nScrollDiffX; }
    void            SetScrollDiffX( sal_uInt16 n )  { nScrollDiffX = n; }

    sal_uInt16      GetCursorBidiLevel() const      { return nCursorBidiLevel; }
    void            SetCursorBidiLevel( sal_uInt16 n ) { nCursorBidiLevel = n; }

    Point           GetDocPos( const Point& rWindowPos ) const;
    Point           GetWindowPos( const Point& rDocPos ) const;
    Rectangle       GetWindowPos( const Rectangle& rDocPos ) const;

    void                SetOutputArea( const Rectangle& rRec );
    void                ResetOutputArea( const Rectangle& rRec );
    const Rectangle&    GetOutputArea() const   { return aOutArea; }

    BOOL            IsVertical() const;

    BOOL            PostKeyEvent( const KeyEvent& rKeyEvent );

    BOOL            MouseButtonUp( const MouseEvent& rMouseEvent );
    BOOL            MouseButtonDown( const MouseEvent& rMouseEvent );
    BOOL            MouseMove( const MouseEvent& rMouseEvent );
    void            Command( const CommandEvent& rCEvt );

    void            CutCopy( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard, sal_Bool bCut );
    void            Paste( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard, sal_Bool bUseSpecial = sal_False );

    void            SetVisDocStartPos( const Point& rPos ) { aVisDocStartPos = rPos; }
    const Point&    GetVisDocStartPos() const { return aVisDocStartPos; }

    long            GetVisDocLeft() const { return aVisDocStartPos.X(); }
    long            GetVisDocTop() const { return aVisDocStartPos.Y(); }
    long            GetVisDocRight() const { return aVisDocStartPos.X() + ( !IsVertical() ? aOutArea.GetWidth() : aOutArea.GetHeight() ); }
    long            GetVisDocBottom() const { return aVisDocStartPos.Y() + ( !IsVertical() ? aOutArea.GetHeight() : aOutArea.GetWidth() ); }
    Rectangle       GetVisDocArea() const;

    EditSelection&  GetEditSelection()          { return aEditSelection; }
    void            SetEditSelection( const EditSelection& rEditSelection );
    sal_Bool        HasSelection() const { return aEditSelection.HasRange(); }

    void            DrawSelection() { DrawSelection( aEditSelection ); }
    void            DrawSelection( EditSelection, Region* pRegion = NULL );
    Region*         CalcSelectedRegion();

    Window*         GetWindow() const           { return pOutWin; }

    EESelectionMode GetSelectionMode() const    { return eSelectionMode; }
    void            SetSelectionMode( EESelectionMode eMode );

    inline void     SetPointer( const Pointer& rPointer );
    inline const Pointer&   GetPointer();

    inline void     SetCursor( const Cursor& rCursor );
    inline Cursor*  GetCursor();

    void            AddDragAndDropListeners();
    void            RemoveDragAndDropListeners();

    BOOL            IsBulletArea( const Point& rPos, sal_uInt16* pPara );

//  Fuer die SelectionEngine...
    void            CreateAnchor();
    void            DeselectAll();
    sal_Bool        SetCursorAtPoint( const Point& rPointPixel );
    sal_Bool        IsSelectionAtPoint( const Point& rPosPixel );
    sal_Bool        IsInSelection( const EditPaM& rPaM );


    void            SetAnchorMode( EVAnchorMode eMode );
    EVAnchorMode    GetAnchorMode() const           { return eAnchorMode; }
    void            CalcAnchorPoint();
    void            RecalcOutputArea();

    void            ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, BOOL test );
    void            ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, USHORT nShowCursorFlags = 0 );
    Pair            Scroll( long ndX, long ndY, BYTE nRangeCheck = RGCHK_NEG );

    void            SetInsertMode( sal_Bool bInsert );
    sal_Bool            IsInsertMode() const            { return ( ( nControl & EV_CNTRL_OVERWRITE ) == 0 ); }

    void                EnablePaste( sal_Bool bEnable )     { SetFlags( nControl, EV_CNTRL_ENABLEPASTE, bEnable ); }
    sal_Bool            IsPasteEnabled() const          { return ( ( nControl & EV_CNTRL_ENABLEPASTE ) != 0 ); }

    sal_Bool            DoSingleLinePaste() const       { return ( ( nControl & EV_CNTRL_SINGLELINEPASTE ) != 0 ); }
    sal_Bool            DoAutoScroll() const            { return ( ( nControl & EV_CNTRL_AUTOSCROLL ) != 0 ); }
    sal_Bool            DoBigScroll() const             { return ( ( nControl & EV_CNTRL_BIGSCROLL ) != 0 ); }
    sal_Bool            DoAutoSize() const              { return ( ( nControl & EV_CNTRL_AUTOSIZE ) != 0 ); }
    sal_Bool            DoAutoWidth() const             { return ( ( nControl & EV_CNTRL_AUTOSIZEX) != 0 ); }
    sal_Bool            DoAutoHeight() const            { return ( ( nControl & EV_CNTRL_AUTOSIZEY) != 0 ); }
    sal_Bool            DoInvalidateMore() const        { return ( ( nControl & EV_CNTRL_INVONEMORE ) != 0 ); }

    void            SetBackgroundColor( const Color& rColor );
    const Color&    GetBackgroundColor() const {
                        return ( pBackgroundColor ? *pBackgroundColor : pOutWin->GetBackground().GetColor() ); }

    sal_Bool            IsWrongSpelledWord( const EditPaM& rPaM, sal_Bool bMarkIfWrong );
    String          SpellIgnoreOrAddWord( sal_Bool bAdd );

    const SvxFieldItem* GetField( const Point& rPos, sal_uInt16* pPara, sal_uInt16* pPos ) const;
    void                DeleteSelected();

    // Ggf. mehr als OutputArea invalidieren, fuer den DrawingEngine-Textrahmen...
    void            SetInvalidateMore( sal_uInt16 nPixel ) { nInvMore = nPixel; }
    sal_uInt16      GetInvalidateMore() const { return (sal_uInt16)nInvMore; }
};

// ----------------------------------------------------------------------
//  ImpEditEngine
//  ----------------------------------------------------------------------

typedef EditView* EditViewPtr;
SV_DECL_PTRARR( EditViews, EditViewPtr, 0, 1 );

class ImpEditEngine : public SfxListener
{
    // Die Undos muessen direkt manipulieren ( private-Methoden ),
    // damit keine neues Undos eingefuegt werden!
    friend class EditUndoInsertChars;
    friend class EditUndoRemoveChars;
    friend class EditUndoDelContent;
    friend class EditUndoConnectParas;
    friend class EditUndoSplitPara;
    friend class EditUndoInsertFeature;
    friend class EditUndoMoveParagraphs;

    friend class EditView;
    friend class ImpEditView;
    friend class EditEngine;        // Fuer Zugriff auf Imp-Methoden
    friend class EditRTFParser;     // Fuer Zugriff auf Imp-Methoden
    friend class EditHTMLParser;    // Fuer Zugriff auf Imp-Methoden
    friend class EdtAutoCorrDoc;    // Fuer Zugriff auf Imp-Methoden
    friend class EditDbg;           // DebugRoutinen

private:

    // ================================================================
    // Daten...
    // ================================================================

    // Dokument-Spezifische Daten...
    ParaPortionList     aParaPortionList;       // Formatierung
    Size                aPaperSize;             // Layout
    Size                aMinAutoPaperSize;      // Layout ?
    Size                aMaxAutoPaperSize;      // Layout ?
    EditDoc             aEditDoc;               // Dokumenteninhalt

    // Engine-Spezifische Daten....
    EditEngine*         pEditEngine;
    EditViews           aEditViews;
    EditView*           pActiveView;
    TextRanger*         pTextRanger;

    SfxStyleSheetPool*  pStylePool;
    SfxItemPool*        pTextObjectPool;

    VirtualDevice*      pVirtDev;
    OutputDevice*       pRefDev;

    svtools::ColorConfig*   pColorConfig;
    SvtCTLOptions*      pCTLOptions;

    SfxItemSet*         pEmptyItemSet;
    EditUndoManager*    pUndoManager;
    ESelection*         pUndoMarkSelection;

    ImplIMEInfos*       mpIMEInfos;

    NotifyList          aNotifyCache;

    XubString           aWordDelimiters;
    XubString           aGroupChars;

    EditSelFunctionSet  aSelFuncSet;
    EditSelectionEngine aSelEngine;

    Color               maBackgroundColor;

    sal_uInt32          nBlockNotifications;
    sal_uInt16          nStretchX;
    sal_uInt16          nStretchY;

    USHORT              nAsianCompressionMode;
    BOOL                bKernAsianPunctuation;

    EEHorizontalTextDirection eDefaultHorizontalTextDirection;

    sal_uInt16          nBigTextObjectStart;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > xSpeller;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    xHyphenator;
    SpellInfo*          pSpellInfo;
    ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > xBI;

    XubString           aAutoCompleteText;

    InternalEditStatus  aStatus;

    LanguageType        eDefLanguage;

    sal_Bool            bIsFormatting;
    sal_Bool            bFormatted;
    sal_Bool            bInSelection;
    sal_Bool            bIsInUndo;
    sal_Bool            bUpdate;
    sal_Bool            bUndoEnabled;
    sal_Bool            bOwnerOfRefDev;
    sal_Bool            bDowning;
    sal_Bool            bUseAutoColor;
    sal_Bool            bForceAutoColor;
    sal_Bool            bCallParaInsertedOrDeleted;

    // Fuer Formatierung / Update....
    DeletedNodesList    aDeletedNodes;
    Rectangle           aInvalidRec;
    sal_uInt32          nCurTextHeight;
    sal_uInt16          nOnePixelInRef;

    IdleFormattter      aIdleFormatter;

    Timer               aOnlineSpellTimer;

    // Wenn an einer Stelle erkannt wird, dass der StatusHdl gerufen werden
    // muss, dies aber nicht sofort geschehen darf (kritischer Abschnitt):
    Timer               aStatusTimer;
    Link                aStatusHdlLink;
    Link                aNotifyHdl;
    Link                aImportHdl;
    Link                aBeginMovingParagraphsHdl;
    Link                aEndMovingParagraphsHdl;
    Link                aBeginPasteOrDropHdl;
    Link                aEndPasteOrDropHdl;
    Link                aModifyHdl;

    vos::ORef<SvxForbiddenCharactersTable>  xForbiddenCharsTable;


    // ================================================================
    // Methoden...
    // ================================================================

    void                CursorMoved( ContentNode* pPrevNode );
    void                ParaAttribsChanged( ContentNode* pNode );
    void                TextModified();
    void                CalcHeight( ParaPortion* pPortion );

    // ggf. lieber inline, aber so einiges...
    void                InsertUndo( EditUndo* pUndo, sal_Bool bTryMerge = sal_False );
    void                ResetUndoManager();
    sal_Bool            HasUndoManager() const  { return pUndoManager ? sal_True : sal_False; }

    EditUndoSetAttribs* CreateAttribUndo( EditSelection aSel, const SfxItemSet& rSet );

    EditPaM             GetPaM( Point aDocPos, sal_Bool bSmart = sal_True );
    EditPaM             GetPaM( ParaPortion* pPortion, Point aPos, sal_Bool bSmart = sal_True );
    long                GetXPos( ParaPortion* pParaPortion, EditLine* pLine, USHORT nIndex, BOOL bPreferPortionStart = FALSE );
    long                GetPortionXOffset( ParaPortion* pParaPortion, EditLine* pLine, USHORT nTextPortion );
    USHORT              GetChar( ParaPortion* pParaPortion, EditLine* pLine, long nX, BOOL bSmart = TRUE );
    Range               GetInvalidYOffsets( ParaPortion* pPortion );
    Range               GetLineXPosStartEnd( ParaPortion* pParaPortion, EditLine* pLine );

    void                SetParaAttrib( BYTE nFunc, EditSelection aSel, sal_uInt16 nValue );
    sal_uInt16          GetParaAttrib( BYTE nFunc, EditSelection aSel );
    void                SetCharAttrib( EditSelection aSel, const SfxPoolItem& rItem );
    void                ParaAttribsToCharAttribs( ContentNode* pNode );
    void                GetCharAttribs( sal_uInt16 nPara, EECharAttribArray& rLst ) const;

    EditTextObject*     CreateBinTextObject( EditSelection aSelection, SfxItemPool*, sal_Bool bAllowBigObjects = sal_False, sal_uInt16 nBigObjStart = 0 ) const;
    void                StoreBinTextObject( SvStream& rOStream, BinTextObject& rTextObject );
    EditSelection       InsertBinTextObject( BinTextObject&, EditPaM aPaM );
    EditSelection       InsertText( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rxDataObj, const EditPaM& rPaM, BOOL bUseSpecial );

    EditPaM             Clear();
    EditPaM             RemoveText();
    EditPaM             RemoveText( EditSelection aEditSelection );
    sal_Bool            CreateLines( USHORT nPara, sal_uInt32 nStartPosY );
    void                CreateAndInsertEmptyLine( ParaPortion* pParaPortion, sal_uInt32 nStartPosY );
    sal_Bool            FinishCreateLines( ParaPortion* pParaPortion );
    void                CalcCharPositions( ParaPortion* pParaPortion );
    void                CreateTextPortions( ParaPortion* pParaPortion, sal_uInt16& rStartPos /*, sal_Bool bCreateBlockPortions */ );
    void                RecalcTextPortion( ParaPortion* pParaPortion, sal_uInt16 nStartPos, short nNewChars );
    sal_uInt16          SplitTextPortion( ParaPortion* pParaPortion, sal_uInt16 nPos,  EditLine* pCurLine = 0 );
    void                SeekCursor( ContentNode* pNode, sal_uInt16 nPos, SvxFont& rFont, OutputDevice* pOut = NULL, sal_uInt16 nIgnoreWhich = 0 );
    void                RecalcFormatterFontMetrics( FormatterFontMetric& rCurMetrics, SvxFont& rFont );
    void                CheckAutoPageSize();

    void                ImpBreakLine( ParaPortion* pParaPortion, EditLine* pLine, TextPortion* pPortion, sal_uInt16 nPortionStart, long nRemainingWidth, sal_Bool bCanHyphenate );
    void                ImpAdjustBlocks( ParaPortion* pParaPortion, EditLine* pLine, long nRemainingSpace );
    EditPaM             ImpConnectParagraphs( ContentNode* pLeft, ContentNode* pRight, sal_Bool bBackward = sal_False );
    EditPaM             ImpDeleteSelection( EditSelection aEditSelection);
    EditPaM             ImpInsertParaBreak( const EditPaM& rPaM, sal_Bool bKeepEndingAttribs = sal_True );
    EditPaM             ImpInsertParaBreak( const EditSelection& rEditSelection, sal_Bool bKeepEndingAttribs = sal_True );
    EditPaM             ImpInsertText( EditSelection aCurEditSelection, const String& rStr );
    EditPaM             ImpInsertFeature( EditSelection aEditSelection, const SfxPoolItem& rItem );
    void                ImpRemoveChars( const EditPaM& rPaM, sal_uInt16 nChars, EditUndoRemoveChars* pCurUndo = 0 );
    void                ImpRemoveParagraph( sal_uInt16 nPara );
    EditSelection       ImpMoveParagraphs( Range aParagraphs, sal_uInt16 nNewPos );

    EditPaM             ImpFastInsertText( EditPaM aPaM, const String& rStr );
    EditPaM             ImpFastInsertParagraph( sal_uInt16 nPara );

    sal_Bool            ImpCheckRefMapMode();

    BOOL                ImplHasText() const;

    void                ImpFindKashidas( ContentNode* pNode, USHORT nStart, USHORT nEnd, SvUShorts& rArray );

    void                InsertContent( ContentNode* pNode, sal_uInt16 nPos );
    EditPaM             SplitContent( sal_uInt16 nNode, sal_uInt16 nSepPos );
    EditPaM             ConnectContents( sal_uInt16 nLeftNode, sal_Bool bBackward );

    void                ShowParagraph( sal_uInt16 nParagraph, sal_Bool bShow );
    sal_Bool            IsParagraphVisible( sal_uInt16 nParagraph );

    EditPaM             PageUp( const EditPaM& rPaM, EditView* pView);
    EditPaM             PageDown( const EditPaM& rPaM, EditView* pView);
    EditPaM             CursorUp( const EditPaM& rPaM, EditView* pEditView );
    EditPaM             CursorDown( const EditPaM& rPaM, EditView* pEditView );
    EditPaM             CursorLeft( const EditPaM& rPaM, USHORT nCharacterIteratorMode = ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL );
    EditPaM             CursorRight( const EditPaM& rPaM, USHORT nCharacterIteratorMode = ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL );
    EditPaM             CursorStartOfLine( const EditPaM& rPaM );
    EditPaM             CursorEndOfLine( const EditPaM& rPaM );
    EditPaM             CursorStartOfParagraph( const EditPaM& rPaM );
    EditPaM             CursorEndOfParagraph( const EditPaM& rPaM );
    EditPaM             CursorStartOfDoc();
    EditPaM             CursorEndOfDoc();
    EditPaM             WordLeft( const EditPaM& rPaM, sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
    EditPaM             WordRight( const EditPaM& rPaM, sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
    EditPaM             StartOfWord( const EditPaM& rPaM, sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
    EditPaM             EndOfWord( const EditPaM& rPaM, sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
    EditSelection       SelectWord( const EditSelection& rCurSelection, sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, BOOL bAcceptStartOfWord = TRUE );
    EditPaM             CursorVisualLeftRight( EditView* pEditView, const EditPaM& rPaM, USHORT nCharacterIteratorMode, BOOL bToLeft );
    EditPaM             CursorVisualStartEnd( EditView* pEditView, const EditPaM& rPaM, BOOL bStart );


    void                InitScriptTypes( USHORT nPara );
    USHORT              GetScriptType( const EditPaM& rPaM, USHORT* pEndPos = NULL ) const;
    USHORT              GetScriptType( const EditSelection& rSel ) const;
    BOOL                IsScriptChange( const EditPaM& rPaM ) const;
    BOOL                HasScriptType( USHORT nPara, USHORT nType ) const;

    BOOL                ImplCalcAsianCompression( ContentNode* pNode, TextPortion* pTextPortion, USHORT nStartPos, long* pDXArray, USHORT n100thPercentFromMax, BOOL bManipulateDXArray );
    void                ImplExpandCompressedPortions( EditLine* pLine, ParaPortion* pParaPortion, long nRemainingWidth );

    void                ImplInitLayoutMode( OutputDevice* pOutDev, USHORT nPara, USHORT nIndex );

    EditPaM             ReadText( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadRTF( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadXML( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadHTML( SvStream& rInput, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs );
    EditPaM             ReadBin( SvStream& rInput, EditSelection aSel );
    sal_uInt32          WriteText( SvStream& rOutput, EditSelection aSel );
    sal_uInt32          WriteRTF( SvStream& rOutput, EditSelection aSel );
    sal_uInt32          WriteXML( SvStream& rOutput, EditSelection aSel );
    sal_uInt32          WriteHTML( SvStream& rOutput, EditSelection aSel );
    sal_uInt32          WriteBin( SvStream& rOutput, EditSelection aSel, BOOL bStoreUnicode = FALSE ) const;

    void                WriteItemAsRTF( const SfxPoolItem& rItem, SvStream& rOutput, sal_uInt16 nPara, sal_uInt16 nPos,
                        SvxFontTable& rFontTable, SvxColorList& rColorList );
    sal_Bool            WriteItemListAsRTF( ItemList& rLst, SvStream& rOutput, sal_uInt16 nPara, sal_uInt16 nPos,
                        SvxFontTable& rFontTable, SvxColorList& rColorList );
    long                LogicToTwips( long n );

    inline short        GetXValue( short nXValue ) const;
    inline sal_uInt16   GetXValue( sal_uInt16 nXValue ) const;
    inline long         GetXValue( long nXValue ) const;

    inline short        GetYValue( short nYValue ) const;
    inline sal_uInt16   GetYValue( sal_uInt16 nYValue ) const;

    ContentNode*        GetPrevVisNode( ContentNode* pCurNode );
    ContentNode*        GetNextVisNode( ContentNode* pCurNode );

    ParaPortion*        GetPrevVisPortion( ParaPortion* pCurPortion );
    ParaPortion*        GetNextVisPortion( ParaPortion* pCurPortion );

    void                SetBackgroundColor( const Color& rColor ) { maBackgroundColor = rColor; }
    Color               GetBackgroundColor() const { return maBackgroundColor; }

    Color               GetAutoColor() const;
    void                EnableAutoColor( BOOL b ) { bUseAutoColor = b; }
    BOOL                IsAutoColorEnabled() const { return bUseAutoColor; }
    void                ForceAutoColor( BOOL b ) { bForceAutoColor = b; }
    BOOL                IsForceAutoColor() const { return bForceAutoColor; }

    inline VirtualDevice*   GetVirtualDevice( const MapMode& rMapMode );
    inline void             EraseVirtualDevice();

    DECL_LINK( StatusTimerHdl, Timer * );
    DECL_LINK( IdleFormatHdl, Timer * );
    DECL_LINK( OnlineSpellHdl, Timer * );
    DECL_LINK( DocModified, void* );

    void                CheckIdleFormatter();

    inline ParaPortion* FindParaPortion( ContentNode* pNode ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > CreateTransferable( const EditSelection& rSelection ) const;

    void                SetValidPaperSize( const Size& rSz );

    ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > ImplGetBreakIterator();



protected:
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

public:
                            ImpEditEngine( EditEngine* pEditEngine, SfxItemPool* pPool );
                            ~ImpEditEngine();

    void                    InitDoc( BOOL bKeepParaAttribs );
    EditDoc&                GetEditDoc()            { return aEditDoc; }
    const EditDoc&          GetEditDoc() const      { return aEditDoc; }

    inline EditUndoManager& GetUndoManager();

    void                    SetUpdateMode( sal_Bool bUp, EditView* pCurView = 0, sal_Bool bForceUpdate = sal_False );
    sal_Bool                GetUpdateMode() const   { return bUpdate; }

    const ParaPortionList&  GetParaPortions() const { return aParaPortionList; }
    ParaPortionList&        GetParaPortions()       { return aParaPortionList; }
    EditViews&              GetEditViews()          { return aEditViews; }

    const Size&             GetPaperSize() const                    { return aPaperSize; }
    void                    SetPaperSize( const Size& rSz )         { aPaperSize = rSz; }

    void                    SetVertical( BOOL bVertical );
    BOOL                    IsVertical() const                      { return GetEditDoc().IsVertical(); }

    void                    SetFixedCellHeight( BOOL bUseFixedCellHeight );
    BOOL                    IsFixedCellHeight() const { return GetEditDoc().IsFixedCellHeight(); }

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir ) { eDefaultHorizontalTextDirection = eHTextDir; }
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const { return eDefaultHorizontalTextDirection; }


    void                    InitWritingDirections( USHORT nPara );
    BOOL                    IsRightToLeft( USHORT nPara ) const;
    BYTE                    GetRightToLeft( USHORT nPara, USHORT nChar, USHORT* pStart = NULL, USHORT* pEnd = NULL );
    BOOL                    HasDifferentRTLLevels( const ContentNode* pNode );

    void                    SetTextRanger( TextRanger* pRanger );
    TextRanger*             GetTextRanger() const { return pTextRanger; }

    const Size&             GetMinAutoPaperSize() const             { return aMinAutoPaperSize; }
    void                    SetMinAutoPaperSize( const Size& rSz )  { aMinAutoPaperSize = rSz; }

    const Size&             GetMaxAutoPaperSize() const             { return aMaxAutoPaperSize; }
    void                    SetMaxAutoPaperSize( const Size& rSz )  { aMaxAutoPaperSize = rSz; }

    void                    FormatDoc();
    void                    FormatFullDoc();
    void                    UpdateViews( EditView* pCurView = 0 );
    void                    Paint( ImpEditView* pView, const Rectangle& rRect, sal_Bool bUseVirtDev = sal_False );
    void                    Paint( OutputDevice* pOutDev, Rectangle aClipRec, Point aStartPos, sal_Bool bStripOnly = sal_False, short nOrientation = 0 );

    sal_Bool                MouseButtonUp( const MouseEvent& rMouseEvent, EditView* pView );
    sal_Bool                MouseButtonDown( const MouseEvent& rMouseEvent, EditView* pView );
    sal_Bool                MouseMove( const MouseEvent& rMouseEvent, EditView* pView );
    void                    Command( const CommandEvent& rCEvt, EditView* pView );

    EditSelectionEngine&    GetSelEngine() { return aSelEngine; }
    XubString               GetSelected( const EditSelection& rSel, const LineEnd eParaSep = LINEEND_LF ) const;

    const SfxItemSet&       GetEmptyItemSet();

    void                    UpdateSelections();

    void                    EnableUndo( sal_Bool bEnable );
    sal_Bool                IsUndoEnabled()         { return bUndoEnabled; }
    void                    SetUndoMode( sal_Bool b )   { bIsInUndo = b; }
    sal_Bool                IsInUndo()              { return bIsInUndo; }

    void                    SetCallParaInsertedOrDeleted( sal_Bool b ) { bCallParaInsertedOrDeleted = b; }
    sal_Bool                IsCallParaInsertedOrDeleted() const { return bCallParaInsertedOrDeleted; }

    sal_Bool                IsFormatted() const { return bFormatted; }
    sal_Bool                IsFormatting() const { return bIsFormatting; }

    void            SetText( const String& rText );
    EditPaM         DeleteSelected( EditSelection aEditSelection);
    EditPaM         InsertText( const EditSelection& rCurEditSelection, sal_Unicode c, sal_Bool bOverwrite );
    EditPaM         InsertText( EditSelection aCurEditSelection, const String& rStr );
    EditPaM         AutoCorrect( const EditSelection& rCurEditSelection, sal_Unicode c, sal_Bool bOverwrite );
    EditPaM         DeleteLeftOrRight( const EditSelection& rEditSelection, BYTE nMode, BYTE nDelMode = DELMODE_SIMPLE );
    EditPaM         InsertParaBreak( EditSelection aEditSelection );
    EditPaM         InsertLineBreak( EditSelection aEditSelection );
    EditPaM         InsertTab( EditSelection aEditSelection );
    EditPaM         InsertField( EditSelection aEditSelection, const SvxFieldItem& rFld );
    sal_Bool        UpdateFields();

    EditPaM         Read( SvStream& rInput, EETextFormat eFormat, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );
    void            Write( SvStream& rOutput, EETextFormat eFormat, EditSelection aSel );

    EditTextObject* CreateTextObject();
    EditTextObject* CreateTextObject( EditSelection aSel );
    void            SetText( const EditTextObject& rTextObject );
    EditSelection   InsertText( const EditTextObject& rTextObject, EditSelection aSel );

    EditSelection   MoveCursor( const KeyEvent& rKeyEvent, EditView* pEditView );

    EditSelection   MoveParagraphs( Range aParagraphs, sal_uInt16 nNewPos, EditView* pCurView );

    sal_uInt32      CalcTextHeight();
    sal_uInt32      GetTextHeight() const;
    sal_uInt32      CalcTextWidth( BOOL bIgnoreExtraSpace );
    sal_uInt32      CalcLineWidth( ParaPortion* pPortion, EditLine* pLine, BOOL bIgnoreExtraSpace );
    sal_uInt16      GetLineCount( sal_uInt16 nParagraph ) const;
    sal_uInt16      GetLineLen( sal_uInt16 nParagraph, sal_uInt16 nLine ) const;
    sal_uInt16      GetLineHeight( sal_uInt16 nParagraph, sal_uInt16 nLine );
    sal_uInt32      GetParaHeight( sal_uInt16 nParagraph );

    SfxItemSet      GetAttribs( USHORT nPara, USHORT nStart, USHORT nEnd, sal_uInt8 nFlags = 0xFF ) const;
    SfxItemSet      GetAttribs( EditSelection aSel, BOOL bOnlyHardAttrib = FALSE  );
    void            SetAttribs( EditSelection aSel, const SfxItemSet& rSet, BYTE nSpecial = 0 );
    void            RemoveCharAttribs( EditSelection aSel, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich = 0 );
    void            RemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich = 0, sal_Bool bRemoveFeatures = sal_False );
    void            SetFlatMode( sal_Bool bFlat );

    void                SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet );
    const SfxItemSet&   GetParaAttribs( sal_uInt16 nPara ) const;

    sal_Bool            HasParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich ) const;

    Rectangle       PaMtoEditCursor( EditPaM aPaM, sal_uInt16 nFlags = 0 );
    Rectangle       GetEditCursor( ParaPortion* pPortion, sal_uInt16 nIndex, sal_uInt16 nFlags = 0 );

    sal_Bool        IsModified() const      { return aEditDoc.IsModified(); }
    void            SetModifyFlag( sal_Bool b ) { aEditDoc.SetModified( b ); }
    void            SetModifyHdl( const Link& rLink ) { aModifyHdl = rLink; }
    Link            GetModifyHdl() const { return aModifyHdl; }


    sal_Bool        IsInSelectionMode() { return bInSelection; }
    void            StopSelectionMode();

    void            IndentBlock( EditView* pView, sal_Bool bRight );

//  Fuer Undo/Redo
    sal_Bool        Undo( EditView* pView );
    sal_Bool        Redo( EditView* pView );
    sal_Bool        Repeat( EditView* pView );

//  OV-Special
    void            InvalidateFromParagraph( sal_uInt16 nFirstInvPara );
    EditPaM         InsertParagraph( sal_uInt16 nPara );
    EditSelection*  SelectParagraph( sal_uInt16 nPara );

    void            SetStatusEventHdl( const Link& rLink )  { aStatusHdlLink = rLink; }
    Link            GetStatusEventHdl() const               { return aStatusHdlLink; }

    void            SetNotifyHdl( const Link& rLink )       { aNotifyHdl = rLink; }
    Link            GetNotifyHdl() const            { return aNotifyHdl; }

    void            FormatAndUpdate( EditView* pCurView = 0 );
    inline void     IdleFormatAndUpdate( EditView* pCurView = 0 );

    svtools::ColorConfig& GetColorConfig();
    BOOL            IsVisualCursorTravelingEnabled();
    BOOL            DoVisualCursorTraveling( const ContentNode* pNode );

    EditSelection           ConvertSelection( sal_uInt16 nStartPara, sal_uInt16 nStartPos, sal_uInt16 nEndPara, sal_uInt16 nEndPos ) const;
    inline EPaM             CreateEPaM( const EditPaM& rPaM );
    inline EditPaM          CreateEditPaM( const EPaM& rEPaM );
    inline ESelection       CreateESel( const EditSelection& rSel );
    inline EditSelection    CreateSel( const ESelection& rSel );


    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool() const { return pStylePool; }

    void                SetStyleSheet( EditSelection aSel, SfxStyleSheet* pStyle );
    void                SetStyleSheet( sal_uInt16 nPara, SfxStyleSheet* pStyle );
    SfxStyleSheet*      GetStyleSheet( sal_uInt16 nPara ) const;

    void                UpdateParagraphsWithStyleSheet( SfxStyleSheet* pStyle );
    void                RemoveStyleFromParagraphs( SfxStyleSheet* pStyle );

    OutputDevice*       GetRefDevice() const { return pRefDev; }
    void                SetRefDevice( OutputDevice* pRefDef );

    const MapMode&      GetRefMapMode() { return pRefDev->GetMapMode(); }
    void                SetRefMapMode( const MapMode& rMapMode );

    InternalEditStatus& GetStatus() { return aStatus; }
    void                CallStatusHdl();
    void                DelayedCallStatusHdl()  { aStatusTimer.Start(); }

    void                CallNotify( EENotify& rNotify );
    void                EnterBlockNotifications();
    void                LeaveBlockNotifications();


    EditSelection       MatchGroup( const EditSelection& rSel );

    void                UndoActionStart( sal_uInt16 nId );
    void                UndoActionStart( sal_uInt16 nId, const ESelection& rSel );
    void                UndoActionEnd( sal_uInt16 nId );

    EditView*           GetActiveView() const   { return pActiveView; }
    void                SetActiveView( EditView* pView );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >
                        GetSpeller();
    void                SetSpeller( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XSpellChecker1 >  &xSpl )
                            { xSpeller = xSpl; }
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >
                        GetHyphenator() const { return xHyphenator; }
    void                SetHyphenator( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XHyphenator >  &xHyph )
                            { xHyphenator = xHyph; }
    SpellInfo*          GetSpellInfo() const { return pSpellInfo; }

    void                SetDefaultLanguage( LanguageType eLang ) { eDefLanguage = eLang; }
    LanguageType        GetDefaultLanguage() const { return eDefLanguage; }


    LanguageType        GetLanguage( const EditSelection rSelection ) const;
    LanguageType        GetLanguage( const EditPaM& rPaM, USHORT* pEndPos = NULL ) const;
    ::com::sun::star::lang::Locale GetLocale( const EditPaM& rPaM ) const;

    void                DoOnlineSpelling( ContentNode* pThisNodeOnly = 0, sal_Bool bSpellAtCursorPos = sal_False, sal_Bool bInteruptable = sal_True );
    EESpellState        Spell( EditView* pEditView, sal_Bool bMultipleDoc );
    EESpellState        HasSpellErrors();
    EESpellState        StartThesaurus( EditView* pEditView );
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >
                        ImpSpell( EditView* pEditView );

    sal_Bool            Search( const SvxSearchItem& rSearchItem, EditView* pView );
    sal_Bool            ImpSearch( const SvxSearchItem& rSearchItem, const EditSelection& rSearchSelection, const EditPaM& rStartPos, EditSelection& rFoundSel );
    sal_uInt16          StartSearchAndReplace( EditView* pEditView, const SvxSearchItem& rSearchItem );
    sal_Bool            HasText( const SvxSearchItem& rSearchItem );

    void                SetEditTextObjectPool( SfxItemPool* pP )    { pTextObjectPool = pP; }
    SfxItemPool*        GetEditTextObjectPool() const               { return pTextObjectPool; }

    const SvxLRSpaceItem&   GetLRSpaceItem( ContentNode* pNode );
    SvxAdjust               GetJustification( USHORT nPara ) const;

    void                SetCharStretching( sal_uInt16 nX, sal_uInt16 nY );
    inline void         GetCharStretching( sal_uInt16& rX, sal_uInt16& rY );
    void                DoStretchChars( sal_uInt16 nX, sal_uInt16 nY );

    void                SetBigTextObjectStart( sal_uInt16 nStartAtPortionCount )    { nBigTextObjectStart = nStartAtPortionCount; }
    sal_uInt16          GetBigTextObjectStart() const                               { return nBigTextObjectStart; }

    inline EditEngine*  GetEditEnginePtr() const    { return pEditEngine; }

    void                StartOnlineSpellTimer()     { aOnlineSpellTimer.Start(); }
    void                StopOnlineSpellTimer()      { aOnlineSpellTimer.Stop(); }

    const XubString&    GetAutoCompleteText() const { return aAutoCompleteText; }
    void                SetAutoCompleteText( const String& rStr, sal_Bool bUpdateTipWindow );

    EditSelection       TransliterateText( const EditSelection& rSelection, sal_Int32 nTransliterationMode );

    void                SetAsianCompressionMode( USHORT n );
    USHORT              GetAsianCompressionMode() const { return nAsianCompressionMode; }

    void                SetKernAsianPunctuation( BOOL b );
    BOOL                IsKernAsianPunctuation() const { return bKernAsianPunctuation; }

    vos::ORef<SvxForbiddenCharactersTable>  GetForbiddenCharsTable( BOOL bGetInternal = TRUE ) const;
    void                SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars );
};

inline EPaM ImpEditEngine::CreateEPaM( const EditPaM& rPaM )
{
    ContentNode* pNode = rPaM.GetNode();
    return EPaM( aEditDoc.GetPos( pNode ), rPaM.GetIndex() );
}

inline EditPaM ImpEditEngine::CreateEditPaM( const EPaM& rEPaM )
{
    DBG_ASSERT( rEPaM.nPara < aEditDoc.Count(), "CreateEditPaM: Ungueltiger Absatz" );
    DBG_ASSERT( aEditDoc[ rEPaM.nPara ]->Len() >= rEPaM.nIndex, "CreateEditPaM: Ungueltiger Index" );
    return EditPaM( aEditDoc[ rEPaM.nPara], rEPaM.nIndex );
}

inline ESelection ImpEditEngine::CreateESel( const EditSelection& rSel )
{
    ContentNode* pStartNode = rSel.Min().GetNode();
    ContentNode* pEndNode = rSel.Max().GetNode();
    ESelection aESel;
    aESel.nStartPara = aEditDoc.GetPos( pStartNode );
    aESel.nStartPos = rSel.Min().GetIndex();
    aESel.nEndPara = aEditDoc.GetPos( pEndNode );
    aESel.nEndPos = rSel.Max().GetIndex();
    return aESel;
}

inline EditSelection ImpEditEngine::CreateSel( const ESelection& rSel )
{
    DBG_ASSERT( rSel.nStartPara < aEditDoc.Count(), "CreateSel: Ungueltiger Start-Absatz" );
    DBG_ASSERT( rSel.nEndPara < aEditDoc.Count(), "CreateSel: Ungueltiger End-Absatz" );
    EditSelection aSel;
    aSel.Min().SetNode( aEditDoc[ rSel.nStartPara ] );
    aSel.Min().SetIndex( rSel.nStartPos );
    aSel.Max().SetNode( aEditDoc[ rSel.nEndPara ] );
    aSel.Max().SetIndex( rSel.nEndPos );
    DBG_ASSERT( !aSel.DbgIsBuggy( aEditDoc ), "CreateSel: Fehlerhafte Selektion!" );
    return aSel;
}

inline VirtualDevice* ImpEditEngine::GetVirtualDevice( const MapMode& rMapMode )
{
    if ( !pVirtDev )
        pVirtDev = new VirtualDevice;

    if ( ( pVirtDev->GetMapMode().GetMapUnit() != rMapMode.GetMapUnit() ) ||
         ( pVirtDev->GetMapMode().GetScaleX() != rMapMode.GetScaleX() ) ||
         ( pVirtDev->GetMapMode().GetScaleY() != rMapMode.GetScaleY() ) )
    {
        MapMode aMapMode( rMapMode );
        aMapMode.SetOrigin( Point( 0, 0 ) );
        pVirtDev->SetMapMode( aMapMode );
    }

    return pVirtDev;
}

inline void ImpEditEngine::EraseVirtualDevice()
{
    delete pVirtDev;
    pVirtDev = 0;
}

inline void ImpEditEngine::IdleFormatAndUpdate( EditView* pCurView )
{
    aIdleFormatter.DoIdleFormat( pCurView );
}

#ifndef SVX_LIGHT
inline EditUndoManager& ImpEditEngine::GetUndoManager()
{
    if ( !pUndoManager )
        pUndoManager = new EditUndoManager( this );
    return *pUndoManager;
}
#endif

inline ParaPortion* ImpEditEngine::FindParaPortion( ContentNode* pNode ) const
{
    sal_uInt16 nPos = aEditDoc.GetPos( pNode );
    DBG_ASSERT( nPos < GetParaPortions().Count(), "Portionloser Node?" );
    return GetParaPortions()[ nPos ];
}

inline void ImpEditEngine::GetCharStretching( sal_uInt16& rX, sal_uInt16& rY )
{
    rX = nStretchX;
    rY = nStretchY;
}

inline short ImpEditEngine::GetXValue( short nXValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchX == 100 ) )
        return nXValue;

    return (short) ((long)nXValue*nStretchX/100);
}

inline sal_uInt16 ImpEditEngine::GetXValue( sal_uInt16 nXValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchX == 100 ) )
        return nXValue;

    return (sal_uInt16) ((long)nXValue*nStretchX/100);
}

inline long ImpEditEngine::GetXValue( long nXValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchX == 100 ) )
        return nXValue;

    return nXValue*nStretchX/100;
}

inline short ImpEditEngine::GetYValue( short nYValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchY == 100 ) )
        return nYValue;

    return (short) ((long)nYValue*nStretchY/100);
}

inline sal_uInt16 ImpEditEngine::GetYValue( sal_uInt16 nYValue ) const
{
    if ( !aStatus.DoStretch() || ( nStretchY == 100 ) )
        return nYValue;

    return (sal_uInt16) ((long)nYValue*nStretchY/100);
}

inline void ImpEditView::SetPointer( const Pointer& rPointer )
{
    delete pPointer;
    pPointer = new Pointer( rPointer );
}

inline const Pointer& ImpEditView::GetPointer()
{
    if ( !pPointer )
    {
        pPointer = new Pointer( IsVertical() ? POINTER_TEXT_VERTICAL : POINTER_TEXT );
        return *pPointer;
    }

    if(POINTER_TEXT == pPointer->GetStyle() && IsVertical())
    {
        delete pPointer;
        pPointer = new Pointer(POINTER_TEXT_VERTICAL);
    }
    else if(POINTER_TEXT_VERTICAL == pPointer->GetStyle() && !IsVertical())
    {
        delete pPointer;
        pPointer = new Pointer(POINTER_TEXT);
    }

    return *pPointer;
}

inline void ImpEditView::SetCursor( const Cursor& rCursor )
{
    delete pCursor;
    pCursor = new Cursor( rCursor );
}

inline Cursor* ImpEditView::GetCursor()
{
    if ( !pCursor )
        pCursor = new Cursor;
    return pCursor;
}

void ConvertItem( SfxPoolItem& rPoolItem, MapUnit eSourceUnit, MapUnit eDestUnit );
void ConvertAndPutItems( SfxItemSet& rDest, const SfxItemSet& rSource, const MapUnit* pSourceUnit = NULL, const MapUnit* pDestUnit = NULL );
BYTE GetCharTypeForCompression( xub_Unicode cChar );
Point Rotate( const Point& rPoint, short nOrientation, const Point& rOrigin );

#endif // _IMPEDIT_HXX


