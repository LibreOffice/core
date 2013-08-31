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
#ifndef _IMPEDIT_HXX
#define _IMPEDIT_HXX

#include <editdoc.hxx>
#include <editsel.hxx>
#include <editundo.hxx>
#include <editobj2.hxx>
#include <editstt2.hxx>
#include <editeng/editdata.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/SpellPortions.hxx>
#include <editeng/eedata.hxx>
#include "editeng/editeng.hxx"
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/cursor.hxx>

#include <vcl/dndhelp.hxx>
#include <svl/ondemand.hxx>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <i18nlangtag/lang.h>
#include <rtl/ref.hxx>

#include <boost/noncopyable.hpp>

DBG_NAMEEX( EditView )
DBG_NAMEEX( EditEngine )

#define PIMPE       pImpEditView->pEditEngine

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

class EditView;
class EditEngine;
class SvxColorList;

class SvxSearchItem;
class SvxLRSpaceItem;
class TextRanger;
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
class SvtCTLOptions;
class Window;
class SvxNumberFormat;


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

namespace editeng {
    struct MisspellRanges;
}

struct DragAndDropInfo
{
    Rectangle           aCurCursor;
    Rectangle           aCurSavedCursor;
    sal_uInt16          nSensibleRange;
    sal_uInt16          nCursorWidth;
    ESelection          aBeginDragSel;
    EditPaM             aDropDest;
    sal_Int32           nOutlinerDropDest;
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

// #i18881# to be able to identify the postions of changed words
// the positions of each portion need to be saved
typedef std::vector<EditSelection>  SpellContentSelections;

struct SpellInfo
{
    EESpellState    eState;
    EPaM            aSpellStart;
    EPaM            aSpellTo;
    EditPaM         aCurSentenceStart;
    sal_Bool        bSpellToEnd;
    sal_Bool        bMultipleDoc;
    ::svx::SpellPortions    aLastSpellPortions;
    SpellContentSelections  aLastSpellContentSelections;
    SpellInfo()
        { bSpellToEnd = sal_True; eState = EE_SPELL_OK; bMultipleDoc = sal_False; }
};

// used for text conversion
struct ConvInfo
{
    EPaM            aConvStart;
    EPaM            aConvTo;
    EPaM            aConvContinue;    // position to start search for next text portion (word) with
    bool            bConvToEnd;
    bool            bMultipleDoc;

    ConvInfo() : bConvToEnd(true), bMultipleDoc(false) {}
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
//  ---------------------------------------------------------------------
class ImpEditView : public vcl::unohelper::DragAndDropClient
{
    friend class EditView;
    friend class EditEngine;
    friend class ImpEditEngine;
    using vcl::unohelper::DragAndDropClient::dragEnter;
    using vcl::unohelper::DragAndDropClient::dragExit;
    using vcl::unohelper::DragAndDropClient::dragOver;

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
    sal_uLong               nControl;
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

    void ImplDrawHighlightRect( OutputDevice* _pTarget, const Point& rDocPosTopLeft, const Point& rDocPosBottomRight, PolyPolygon* pPolyPoly );

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

    void                SetOutputArea( const Rectangle& rRect );
    void                ResetOutputArea( const Rectangle& rRect );
    const Rectangle&    GetOutputArea() const   { return aOutArea; }

    sal_Bool            IsVertical() const;

    sal_Bool            PostKeyEvent( const KeyEvent& rKeyEvent, Window* pFrameWin = NULL );

    sal_Bool            MouseButtonUp( const MouseEvent& rMouseEvent );
    sal_Bool            MouseButtonDown( const MouseEvent& rMouseEvent );
    sal_Bool            MouseMove( const MouseEvent& rMouseEvent );
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
    void            DrawSelection( EditSelection, Region* pRegion = NULL, OutputDevice* pTargetDevice = NULL );

    Window*         GetWindow() const           { return pOutWin; }

    EESelectionMode GetSelectionMode() const    { return eSelectionMode; }
    void            SetSelectionMode( EESelectionMode eMode );

    inline void     SetPointer( const Pointer& rPointer );
    inline const Pointer&   GetPointer();

    inline void     SetCursor( const Cursor& rCursor );
    inline Cursor*  GetCursor();

    void            AddDragAndDropListeners();
    void            RemoveDragAndDropListeners();

    sal_Bool            IsBulletArea( const Point& rPos, sal_Int32* pPara );

//  For the Selection Engine...
    void            CreateAnchor();
    void            DeselectAll();
    sal_Bool        SetCursorAtPoint( const Point& rPointPixel );
    sal_Bool        IsSelectionAtPoint( const Point& rPosPixel );
    sal_Bool        IsInSelection( const EditPaM& rPaM );


    void            SetAnchorMode( EVAnchorMode eMode );
    EVAnchorMode    GetAnchorMode() const           { return eAnchorMode; }
    void            CalcAnchorPoint();
    void            RecalcOutputArea();

    void            ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, sal_Bool test );
    void            ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, sal_uInt16 nShowCursorFlags = 0 );
    Pair            Scroll( long ndX, long ndY, sal_uInt8 nRangeCheck = RGCHK_NEG );

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

    const SvxFieldItem* GetField( const Point& rPos, sal_Int32* pPara, sal_uInt16* pPos ) const;
    void                DeleteSelected();

    //  If possible invalidate more than OutputArea, for the DrawingEngine text frame
    void            SetInvalidateMore( sal_uInt16 nPixel ) { nInvMore = nPixel; }
    sal_uInt16      GetInvalidateMore() const { return (sal_uInt16)nInvMore; }
};

//  ----------------------------------------------------------------------
//  ImpEditEngine
//  ----------------------------------------------------------------------

class ImpEditEngine : public SfxListener, boost::noncopyable
{
    friend class EditEngine;
    friend class EditDbg;

    typedef EditEngine::ViewsType ViewsType;

private:

    // ================================================================
    // Data ...
    // ================================================================

    // Document Specific data ...
    ParaPortionList     aParaPortionList;       // Formatting
    Size                aPaperSize;             // Layout
    Size                aMinAutoPaperSize;      // Layout ?
    Size                aMaxAutoPaperSize;      // Layout ?
    EditDoc             aEditDoc;               // Document content

    // Engine Specific data ...
    EditEngine*         pEditEngine;
    ViewsType           aEditViews;
    EditView*           pActiveView;
    TextRanger*         pTextRanger;

    SfxStyleSheetPool*  pStylePool;
    SfxItemPool*        pTextObjectPool;

    VirtualDevice*      pVirtDev;
    OutputDevice*       pRefDev;

    svtools::ColorConfig*   pColorConfig;
    mutable SvtCTLOptions*  pCTLOptions;

    SfxItemSet*         pEmptyItemSet;
    EditUndoManager*    pUndoManager;
    ESelection*         pUndoMarkSelection;

    ImplIMEInfos*       mpIMEInfos;

    std::vector<EENotify> aNotifyCache;

    OUString            aWordDelimiters;

    EditSelFunctionSet  aSelFuncSet;
    EditSelectionEngine aSelEngine;

    Color               maBackgroundColor;

    sal_uInt32          nBlockNotifications;
    sal_uInt16          nStretchX;
    sal_uInt16          nStretchY;

    sal_uInt16              nAsianCompressionMode;

    EEHorizontalTextDirection eDefaultHorizontalTextDirection;

    sal_uInt16          nBigTextObjectStart;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > xSpeller;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    xHyphenator;
    SpellInfo*          pSpellInfo;
    mutable ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > xBI;
    mutable ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XExtendedInputSequenceChecker > xISC;

    ConvInfo *          pConvInfo;

    OUString            aAutoCompleteText;

    InternalEditStatus  aStatus;

    LanguageType        eDefLanguage;

    OnDemandLocaleDataWrapper       xLocaleDataWrapper;
    OnDemandTransliterationWrapper  xTransliterationWrapper;

    // For Formatting / Update ....
    boost::ptr_vector<DeletedNodeInfo> aDeletedNodes;
    Rectangle           aInvalidRect;
    sal_uInt32          nCurTextHeight;
    sal_uInt32          nCurTextHeightNTP;  // without trailing empty paragraphs
    sal_uInt16          nOnePixelInRef;

    IdleFormattter      aIdleFormatter;

    Timer               aOnlineSpellTimer;

    // If it is detected at one point that the StatusHdl has to be called, but
    // this should not happen immediately (critical section):
    Timer               aStatusTimer;
    Link                aStatusHdlLink;
    Link                aNotifyHdl;
    Link                aImportHdl;
    Link                aBeginMovingParagraphsHdl;
    Link                aEndMovingParagraphsHdl;
    Link                aBeginPasteOrDropHdl;
    Link                aEndPasteOrDropHdl;
    Link                aModifyHdl;
    Link                maBeginDropHdl;
    Link                maEndDropHdl;

    rtl::Reference<SvxForbiddenCharactersTable> xForbiddenCharsTable;

    bool            bKernAsianPunctuation:1;
    bool            bAddExtLeading:1;
    bool            bIsFormatting:1;
    bool            bFormatted:1;
    bool            bInSelection:1;
    bool            bIsInUndo:1;
    bool            bUpdate:1;
    bool            bUndoEnabled:1;
    bool            bOwnerOfRefDev:1;
    bool            bDowning:1;
    bool            bUseAutoColor:1;
    bool            bForceAutoColor:1;
    bool            bCallParaInsertedOrDeleted:1;
    bool            bImpConvertFirstCall:1;   // specifies if ImpConvert is called the very first time after Convert was called
    bool            bFirstWordCapitalization:1;   // specifies if auto-correction should capitalize the first word or not
    bool            mbLastTryMerge:1;

    // ================================================================
    // Methods...
    // ================================================================

    void                CursorMoved( ContentNode* pPrevNode );
    void                ParaAttribsChanged( ContentNode* pNode );
    void                TextModified();
    void                CalcHeight( ParaPortion* pPortion );

    void                InsertUndo( EditUndo* pUndo, bool bTryMerge = false );
    void                ResetUndoManager();
    sal_Bool            HasUndoManager() const  { return pUndoManager ? sal_True : sal_False; }

    EditUndoSetAttribs* CreateAttribUndo( EditSelection aSel, const SfxItemSet& rSet );

    EditPaM             GetPaM( Point aDocPos, sal_Bool bSmart = sal_True );
    EditPaM             GetPaM( ParaPortion* pPortion, Point aPos, sal_Bool bSmart = sal_True );
    long GetXPos(const ParaPortion* pParaPortion, const EditLine* pLine, sal_uInt16 nIndex, bool bPreferPortionStart = false) const;
    long GetPortionXOffset(const ParaPortion* pParaPortion, const EditLine* pLine, sal_uInt16 nTextPortion) const;
    sal_uInt16 GetChar(const ParaPortion* pParaPortion, const EditLine* pLine, long nX, bool bSmart = true);
    Range               GetInvalidYOffsets( ParaPortion* pPortion );
    Range GetLineXPosStartEnd( const ParaPortion* pParaPortion, const EditLine* pLine ) const;

    void                SetParaAttrib( sal_uInt8 nFunc, EditSelection aSel, sal_uInt16 nValue );
    sal_uInt16          GetParaAttrib( sal_uInt8 nFunc, EditSelection aSel );
    void                SetCharAttrib( EditSelection aSel, const SfxPoolItem& rItem );
    void                ParaAttribsToCharAttribs( ContentNode* pNode );
    void                GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    EditTextObject*     CreateTextObject( EditSelection aSelection, SfxItemPool*, sal_Bool bAllowBigObjects = sal_False, sal_uInt16 nBigObjStart = 0 );
    EditSelection       InsertTextObject( const EditTextObject&, EditPaM aPaM );
    EditSelection       InsertText( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rxDataObj, const String& rBaseURL, const EditPaM& rPaM, sal_Bool bUseSpecial );

    EditPaM             Clear();
    EditPaM             RemoveText();
    EditPaM             RemoveText( EditSelection aEditSelection );
    sal_Bool            CreateLines( sal_Int32 nPara, sal_uInt32 nStartPosY );
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
    EditPaM             ImpDeleteSelection(const EditSelection& rCurSel);
    EditPaM             ImpInsertParaBreak( EditPaM& rPaM, bool bKeepEndingAttribs = true );
    EditPaM             ImpInsertParaBreak( const EditSelection& rEditSelection, bool bKeepEndingAttribs = true );
    EditPaM             ImpInsertText(const EditSelection& aCurEditSelection, const String& rStr);
    EditPaM             ImpInsertFeature(const EditSelection& rCurSel, const SfxPoolItem& rItem);
    void                ImpRemoveChars( const EditPaM& rPaM, sal_uInt16 nChars, EditUndoRemoveChars* pCurUndo = 0 );
    void                ImpRemoveParagraph( sal_Int32 nPara );
    EditSelection       ImpMoveParagraphs( Range aParagraphs, sal_Int32 nNewPos );

    EditPaM             ImpFastInsertText( EditPaM aPaM, const OUString& rStr );
    EditPaM             ImpFastInsertParagraph( sal_Int32 nPara );

    sal_Bool            ImpCheckRefMapMode();

    sal_Bool                ImplHasText() const;

    void                ImpFindKashidas( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, std::vector<sal_uInt16>& rArray );

    void                InsertContent( ContentNode* pNode, sal_Int32 nPos );
    EditPaM             SplitContent( sal_Int32 nNode, sal_uInt16 nSepPos );
    EditPaM             ConnectContents( sal_Int32 nLeftNode, sal_Bool bBackward );

    void                ShowParagraph( sal_Int32 nParagraph, bool bShow );

    EditPaM             PageUp( const EditPaM& rPaM, EditView* pView);
    EditPaM             PageDown( const EditPaM& rPaM, EditView* pView);
    EditPaM             CursorUp( const EditPaM& rPaM, EditView* pEditView );
    EditPaM             CursorDown( const EditPaM& rPaM, EditView* pEditView );
    EditPaM             CursorLeft( const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL );
    EditPaM             CursorRight( const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL );
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
    EditSelection       SelectWord( const EditSelection& rCurSelection, sal_Int16 nWordType = ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_Bool bAcceptStartOfWord = sal_True );
    EditSelection       SelectSentence( const EditSelection& rCurSel ) const;
    EditPaM             CursorVisualLeftRight( EditView* pEditView, const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode, sal_Bool bToLeft );
    EditPaM             CursorVisualStartEnd( EditView* pEditView, const EditPaM& rPaM, sal_Bool bStart );


    void                InitScriptTypes( sal_Int32 nPara );
    sal_uInt16              GetScriptType( const EditPaM& rPaM, sal_uInt16* pEndPos = NULL ) const;
    sal_uInt16              GetScriptType( const EditSelection& rSel ) const;
    sal_Bool                IsScriptChange( const EditPaM& rPaM ) const;
    sal_Bool                HasScriptType( sal_Int32 nPara, sal_uInt16 nType ) const;

    sal_Bool                ImplCalcAsianCompression( ContentNode* pNode, TextPortion* pTextPortion, sal_uInt16 nStartPos, sal_Int32* pDXArray, sal_uInt16 n100thPercentFromMax, sal_Bool bManipulateDXArray );
    void                ImplExpandCompressedPortions( EditLine* pLine, ParaPortion* pParaPortion, long nRemainingWidth );

    void                ImplInitLayoutMode( OutputDevice* pOutDev, sal_Int32 nPara, sal_uInt16 nIndex );
    LanguageType        ImplCalcDigitLang(LanguageType eCurLang) const;
    void                ImplInitDigitMode(OutputDevice* pOutDev, LanguageType eLang);
    OUString            convertDigits(const OUString &rString, sal_Int32 nStt, sal_Int32 nLen, LanguageType eDigitLang) const;

    EditPaM             ReadText( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadRTF( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadXML( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadHTML( SvStream& rInput, const String& rBaseURL, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs );
    EditPaM             ReadBin( SvStream& rInput, EditSelection aSel );
    sal_uInt32          WriteText( SvStream& rOutput, EditSelection aSel );
    sal_uInt32          WriteRTF( SvStream& rOutput, EditSelection aSel );
    sal_uInt32          WriteXML( SvStream& rOutput, EditSelection aSel );
    sal_uInt32          WriteHTML( SvStream& rOutput, EditSelection aSel );
    sal_uInt32 WriteBin( SvStream& rOutput, EditSelection aSel, bool bStoreUnicode = false );

    void                WriteItemAsRTF( const SfxPoolItem& rItem, SvStream& rOutput, sal_Int32 nPara, sal_uInt16 nPos,
                        std::vector<SvxFontItem*>& rFontTable, SvxColorList& rColorList );
    sal_Bool            WriteItemListAsRTF( ItemList& rLst, SvStream& rOutput, sal_Int32 nPara, sal_uInt16 nPos,
                        std::vector<SvxFontItem*>& rFontTable, SvxColorList& rColorList );
    sal_Int32               LogicToTwips( sal_Int32 n );

    inline short        GetXValue( short nXValue ) const;
    inline sal_uInt16   GetXValue( sal_uInt16 nXValue ) const;
    inline long         GetXValue( long nXValue ) const;

    inline short        GetYValue( short nYValue ) const;
    inline sal_uInt16   GetYValue( sal_uInt16 nYValue ) const;

    ContentNode*        GetPrevVisNode( ContentNode* pCurNode );
    ContentNode*        GetNextVisNode( ContentNode* pCurNode );

    const ParaPortion*  GetPrevVisPortion( const ParaPortion* pCurPortion ) const;
    const ParaPortion*  GetNextVisPortion( const ParaPortion* pCurPortion ) const;

    void                SetBackgroundColor( const Color& rColor ) { maBackgroundColor = rColor; }
    Color               GetBackgroundColor() const { return maBackgroundColor; }

    long                CalcVertLineSpacing(Point& rStartPos) const;

    Color               GetAutoColor() const;
    void                EnableAutoColor( bool b ) { bUseAutoColor = b; }
    bool                IsAutoColorEnabled() const { return bUseAutoColor; }
    void                ForceAutoColor( bool b ) { bForceAutoColor = b; }
    bool                IsForceAutoColor() const { return bForceAutoColor; }

    inline VirtualDevice*   GetVirtualDevice( const MapMode& rMapMode, sal_uLong nDrawMode );
    inline void             EraseVirtualDevice();

    DECL_LINK(StatusTimerHdl, void *);
    DECL_LINK(IdleFormatHdl, void *);
    DECL_LINK(OnlineSpellHdl, void *);
    DECL_LINK( DocModified, void* );

    void                CheckIdleFormatter();

    inline const ParaPortion* FindParaPortion( const ContentNode* pNode ) const;
    inline ParaPortion* FindParaPortion( ContentNode* pNode );

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > CreateTransferable( const EditSelection& rSelection );

    void                SetValidPaperSize( const Size& rSz );

    ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > ImplGetBreakIterator() const;
    ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XExtendedInputSequenceChecker > ImplGetInputSequenceChecker() const;

    SpellInfo *     CreateSpellInfo( bool bMultipleDocs );

    ImpEditEngine(); // disabled
    ImpEditEngine(EditEngine* pEditEngine, SfxItemPool* pPool);
    void InitDoc(bool bKeepParaAttribs);
    EditDoc&                GetEditDoc()            { return aEditDoc; }
    const EditDoc&          GetEditDoc() const      { return aEditDoc; }

    const ParaPortionList&  GetParaPortions() const { return aParaPortionList; }
    ParaPortionList&        GetParaPortions()       { return aParaPortionList; }

protected:
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

public:
                            ~ImpEditEngine();

    inline EditUndoManager& GetUndoManager();
    inline ::svl::IUndoManager* SetUndoManager(::svl::IUndoManager* pNew);

    void                    SetUpdateMode( bool bUp, EditView* pCurView = 0, sal_Bool bForceUpdate = sal_False );
    bool                    GetUpdateMode() const   { return bUpdate; }

    ViewsType& GetEditViews() { return aEditViews; }
    const ViewsType& GetEditViews() const { return aEditViews; }

    const Size&             GetPaperSize() const                    { return aPaperSize; }
    void                    SetPaperSize( const Size& rSz )         { aPaperSize = rSz; }

    void                    SetVertical( sal_Bool bVertical );
    sal_Bool                    IsVertical() const                      { return GetEditDoc().IsVertical(); }

    void                    SetFixedCellHeight( sal_Bool bUseFixedCellHeight );
    sal_Bool                    IsFixedCellHeight() const { return GetEditDoc().IsFixedCellHeight(); }

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir ) { eDefaultHorizontalTextDirection = eHTextDir; }
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const { return eDefaultHorizontalTextDirection; }


    void                    InitWritingDirections( sal_Int32 nPara );
    sal_Bool                    IsRightToLeft( sal_Int32 nPara ) const;
    sal_uInt8                    GetRightToLeft( sal_Int32 nPara, sal_uInt16 nChar, sal_uInt16* pStart = NULL, sal_uInt16* pEnd = NULL );
    sal_Bool                    HasDifferentRTLLevels( const ContentNode* pNode );

    void                    SetTextRanger( TextRanger* pRanger );
    TextRanger*             GetTextRanger() const { return pTextRanger; }

    const Size&             GetMinAutoPaperSize() const             { return aMinAutoPaperSize; }
    void                    SetMinAutoPaperSize( const Size& rSz )  { aMinAutoPaperSize = rSz; }

    const Size&             GetMaxAutoPaperSize() const             { return aMaxAutoPaperSize; }
    void                    SetMaxAutoPaperSize( const Size& rSz )  { aMaxAutoPaperSize = rSz; }

    void                    FormatDoc();
    void                    FormatFullDoc();
    void                    UpdateViews( EditView* pCurView = 0 );
    void                    Paint( ImpEditView* pView, const Rectangle& rRect, OutputDevice* pTargetDevice = 0, sal_Bool bUseVirtDev = sal_False );
    void                    Paint( OutputDevice* pOutDev, Rectangle aClipRect, Point aStartPos, sal_Bool bStripOnly = sal_False, short nOrientation = 0 );

    sal_Bool                MouseButtonUp( const MouseEvent& rMouseEvent, EditView* pView );
    sal_Bool                MouseButtonDown( const MouseEvent& rMouseEvent, EditView* pView );
    sal_Bool                MouseMove( const MouseEvent& rMouseEvent, EditView* pView );
    void                    Command( const CommandEvent& rCEvt, EditView* pView );

    EditSelectionEngine&    GetSelEngine() { return aSelEngine; }
    OUString                GetSelected( const EditSelection& rSel, const LineEnd eParaSep = LINEEND_LF ) const;

    const SfxItemSet&       GetEmptyItemSet();

    void                    UpdateSelections();

    void                EnableUndo( bool bEnable );
    bool                IsUndoEnabled()         { return bUndoEnabled; }
    void                SetUndoMode( bool b )
    {
        bIsInUndo = b;
        if (bIsInUndo)
            EnterBlockNotifications();
        else
            LeaveBlockNotifications();
    }
    bool                IsInUndo()              { return bIsInUndo; }

    void                SetCallParaInsertedOrDeleted( bool b ) { bCallParaInsertedOrDeleted = b; }
    bool                IsCallParaInsertedOrDeleted() const { return bCallParaInsertedOrDeleted; }

    bool                IsFormatted() const { return bFormatted; }
    bool                IsFormatting() const { return bIsFormatting; }

    void            SetText(const OUString& rText);
    EditPaM         DeleteSelected( EditSelection aEditSelection);
    EditPaM         InsertText( const EditSelection& rCurEditSelection, sal_Unicode c, sal_Bool bOverwrite, sal_Bool bIsUserInput = sal_False );
    EditPaM         InsertText(const EditSelection& aCurEditSelection, const String& rStr);
    EditPaM         AutoCorrect( const EditSelection& rCurEditSelection, sal_Unicode c, sal_Bool bOverwrite, Window* pFrameWin = NULL );
    EditPaM         DeleteLeftOrRight( const EditSelection& rEditSelection, sal_uInt8 nMode, sal_uInt8 nDelMode = DELMODE_SIMPLE );
    EditPaM         InsertParaBreak( EditSelection aEditSelection );
    EditPaM         InsertLineBreak(const EditSelection& aEditSelection);
    EditPaM         InsertTab( EditSelection aEditSelection );
    EditPaM         InsertField(const EditSelection& rCurSel, const SvxFieldItem& rFld);
    sal_Bool        UpdateFields();

    EditPaM         Read( SvStream& rInput, const String& rBaseURL, EETextFormat eFormat, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );
    void            Write( SvStream& rOutput, EETextFormat eFormat, EditSelection aSel );

    EditTextObject* CreateTextObject();
    EditTextObject* CreateTextObject( EditSelection aSel );
    void            SetText( const EditTextObject& rTextObject );
    EditSelection   InsertText( const EditTextObject& rTextObject, EditSelection aSel );

    EditSelection   MoveCursor( const KeyEvent& rKeyEvent, EditView* pEditView );

    EditSelection   MoveParagraphs( Range aParagraphs, sal_Int32 nNewPos, EditView* pCurView );

    sal_uInt32      CalcTextHeight( sal_uInt32* pHeightNTP );
    sal_uInt32      GetTextHeight() const;
    sal_uInt32      GetTextHeightNTP() const;
    sal_uInt32      CalcTextWidth( sal_Bool bIgnoreExtraSpace );
    sal_uInt32      CalcLineWidth( ParaPortion* pPortion, EditLine* pLine, sal_Bool bIgnoreExtraSpace );
    sal_uInt16      GetLineCount( sal_Int32 nParagraph ) const;
    sal_uInt16      GetLineLen( sal_Int32 nParagraph, sal_uInt16 nLine ) const;
    void            GetLineBoundaries( /*out*/sal_uInt16 &rStart, /*out*/sal_uInt16 &rEnd, sal_Int32 nParagraph, sal_uInt16 nLine ) const;
    sal_uInt16          GetLineNumberAtIndex( sal_Int32 nPara, sal_uInt16 nIndex ) const;
    sal_uInt16      GetLineHeight( sal_Int32 nParagraph, sal_uInt16 nLine );
    sal_uInt32      GetParaHeight( sal_Int32 nParagraph );

    SfxItemSet      GetAttribs( sal_Int32 nPara, sal_uInt16 nStart, sal_uInt16 nEnd, sal_uInt8 nFlags = 0xFF ) const;
    SfxItemSet      GetAttribs( EditSelection aSel, sal_Bool bOnlyHardAttrib = sal_False  );
    void            SetAttribs( EditSelection aSel, const SfxItemSet& rSet, sal_uInt8 nSpecial = 0 );
    void            RemoveCharAttribs( EditSelection aSel, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich = 0 );
    void            RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich = 0, sal_Bool bRemoveFeatures = sal_False );
    void            SetFlatMode( sal_Bool bFlat );

    void                SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    const SfxItemSet&   GetParaAttribs( sal_Int32 nPara ) const;

    bool            HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;

    Rectangle       PaMtoEditCursor( EditPaM aPaM, sal_uInt16 nFlags = 0 );
    Rectangle       GetEditCursor( ParaPortion* pPortion, sal_uInt16 nIndex, sal_uInt16 nFlags = 0 );

    sal_Bool        IsModified() const      { return aEditDoc.IsModified(); }
    void            SetModifyFlag( sal_Bool b ) { aEditDoc.SetModified( b ); }
    void            SetModifyHdl( const Link& rLink ) { aModifyHdl = rLink; }
    Link            GetModifyHdl() const { return aModifyHdl; }


    bool        IsInSelectionMode() { return bInSelection; }

    void            IndentBlock( EditView* pView, sal_Bool bRight );

//  For Undo/Redo
    sal_Bool        Undo( EditView* pView );
    sal_Bool        Redo( EditView* pView );

//  OV-Special
    void            InvalidateFromParagraph( sal_Int32 nFirstInvPara );
    EditPaM         InsertParagraph( sal_Int32 nPara );
    EditSelection*  SelectParagraph( sal_Int32 nPara );

    void            SetStatusEventHdl( const Link& rLink )  { aStatusHdlLink = rLink; }
    Link            GetStatusEventHdl() const               { return aStatusHdlLink; }

    void            SetNotifyHdl( const Link& rLink )       { aNotifyHdl = rLink; }
    Link            GetNotifyHdl() const            { return aNotifyHdl; }

    void            FormatAndUpdate( EditView* pCurView = 0 );
    inline void     IdleFormatAndUpdate( EditView* pCurView = 0 );

    svtools::ColorConfig& GetColorConfig();
    sal_Bool            IsVisualCursorTravelingEnabled();
    sal_Bool            DoVisualCursorTraveling( const ContentNode* pNode );

    EditSelection ConvertSelection( sal_Int32 nStartPara, sal_uInt16 nStartPos, sal_Int32 nEndPara, sal_uInt16 nEndPos );
    inline EPaM             CreateEPaM( const EditPaM& rPaM );
    inline EditPaM          CreateEditPaM( const EPaM& rEPaM );
    inline ESelection       CreateESel( const EditSelection& rSel );
    inline EditSelection    CreateSel( const ESelection& rSel );


    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool() const { return pStylePool; }

    void                SetStyleSheet( EditSelection aSel, SfxStyleSheet* pStyle );
    void                SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle );
    const SfxStyleSheet* GetStyleSheet( sal_Int32 nPara ) const;
    SfxStyleSheet* GetStyleSheet( sal_Int32 nPara );

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

    void GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const;
    void SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges );

    SpellInfo*          GetSpellInfo() const { return pSpellInfo; }

    void                SetDefaultLanguage( LanguageType eLang ) { eDefLanguage = eLang; }
    LanguageType        GetDefaultLanguage() const { return eDefLanguage; }


    LanguageType        GetLanguage( const EditSelection &rSelection ) const;
    LanguageType        GetLanguage( const EditPaM& rPaM, sal_uInt16* pEndPos = NULL ) const;
    ::com::sun::star::lang::Locale GetLocale( const EditPaM& rPaM ) const;

    void DoOnlineSpelling( ContentNode* pThisNodeOnly = 0, bool bSpellAtCursorPos = false, bool bInteruptable = true );
    EESpellState        Spell( EditView* pEditView, sal_Bool bMultipleDoc );
    EESpellState        HasSpellErrors();
    EESpellState        StartThesaurus( EditView* pEditView );
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >
                        ImpSpell( EditView* pEditView );

    // text conversion functions
    void                Convert( EditView* pEditView, LanguageType nSrcLang, LanguageType nDestLang, const Font *pDestFont, sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc );
    void                ImpConvert( OUString &rConvTxt, LanguageType &rConvTxtLang, EditView* pEditView, LanguageType nSrcLang, const ESelection &rConvRange,
                                    bool bAllowImplicitChangesForNotConvertibleText, LanguageType nTargetLang, const Font *pTargetFont );
    ConvInfo *          GetConvInfo() const { return pConvInfo; }
    sal_Bool            HasConvertibleTextPortion( LanguageType nLang );
    void                SetLanguageAndFont( const ESelection &rESel,
                                LanguageType nLang, sal_uInt16 nLangWhichId,
                                const Font *pFont,  sal_uInt16 nFontWhichId );

    // returns true if input sequence checking should be applied
    sal_Bool            IsInputSequenceCheckingRequired( sal_Unicode nChar, const EditSelection& rCurSel ) const;

    //find the next error within the given selection - forward only!
    ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellAlternatives >
                    ImpFindNextError(EditSelection& rSelection);
    //initialize sentence spelling
    void            StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc);
    //spell and return a sentence
    bool                SpellSentence(EditView& rView, ::svx::SpellPortions& rToFill, bool bIsGrammarChecking );
    //put spelling back to start of current sentence - needed after switch of grammar support
    void                PutSpellingToSentenceStart( EditView& rEditView );
    //applies a changed sentence
    void                ApplyChangedSentence(EditView& rEditView, const ::svx::SpellPortions& rNewPortions, bool bRecheck );
    //adds one or more portions of text to the SpellPortions depending on language changes
    void            AddPortionIterated(
                        EditView& rEditView,
                        const EditSelection &rSel,
                        ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellAlternatives > xAlt,
                        ::svx::SpellPortions& rToFill);
    //adds one portion to the SpellPortions
    void            AddPortion(
                        const EditSelection &rSel,
                        ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellAlternatives > xAlt,
                        ::svx::SpellPortions& rToFill,
                        bool bIsField );

    sal_Bool            Search( const SvxSearchItem& rSearchItem, EditView* pView );
    sal_Bool            ImpSearch( const SvxSearchItem& rSearchItem, const EditSelection& rSearchSelection, const EditPaM& rStartPos, EditSelection& rFoundSel );
    sal_uInt16          StartSearchAndReplace( EditView* pEditView, const SvxSearchItem& rSearchItem );
    sal_Bool            HasText( const SvxSearchItem& rSearchItem );

    void                SetEditTextObjectPool( SfxItemPool* pP )    { pTextObjectPool = pP; }
    SfxItemPool*        GetEditTextObjectPool() const               { return pTextObjectPool; }

    const SvxNumberFormat * GetNumberFormat( const ContentNode* pNode ) const;
    sal_Int32               GetSpaceBeforeAndMinLabelWidth( const ContentNode *pNode, sal_Int32 *pnSpaceBefore = 0, sal_Int32 *pnMinLabelWidth = 0 ) const;

    const SvxLRSpaceItem&   GetLRSpaceItem( ContentNode* pNode );
    SvxAdjust               GetJustification( sal_Int32 nPara ) const;
    SvxCellJustifyMethod    GetJustifyMethod( sal_Int32 nPara ) const;
    SvxCellVerJustify       GetVerJustification( sal_Int32 nPara ) const;

    void                SetCharStretching( sal_uInt16 nX, sal_uInt16 nY );
    inline void         GetCharStretching( sal_uInt16& rX, sal_uInt16& rY ) const;

    void                SetBigTextObjectStart( sal_uInt16 nStartAtPortionCount )    { nBigTextObjectStart = nStartAtPortionCount; }
    sal_uInt16          GetBigTextObjectStart() const                               { return nBigTextObjectStart; }

    inline EditEngine*  GetEditEnginePtr() const    { return pEditEngine; }

    void                StartOnlineSpellTimer()     { aOnlineSpellTimer.Start(); }
    void                StopOnlineSpellTimer()      { aOnlineSpellTimer.Stop(); }

    const OUString&     GetAutoCompleteText() const { return aAutoCompleteText; }
    void                SetAutoCompleteText(const OUString& rStr, bool bUpdateTipWindow);

    EditSelection       TransliterateText( const EditSelection& rSelection, sal_Int32 nTransliterationMode );
    short               ReplaceTextOnly( ContentNode* pNode, sal_uInt16 nCurrentStart, xub_StrLen nLen, const String& rText, const ::com::sun::star::uno::Sequence< sal_Int32 >& rOffsets );


    void                SetAsianCompressionMode( sal_uInt16 n );
    sal_uInt16              GetAsianCompressionMode() const { return nAsianCompressionMode; }

    void                SetKernAsianPunctuation( bool b );
    bool                IsKernAsianPunctuation() const { return bKernAsianPunctuation; }

    void                SetAddExtLeading( bool b );
    bool                IsAddExtLeading() const { return bAddExtLeading; }

    rtl::Reference<SvxForbiddenCharactersTable> GetForbiddenCharsTable( sal_Bool bGetInternal = sal_True ) const;
    void                SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars );

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void                SetBeginDropHdl( const Link& rLink ) { maBeginDropHdl = rLink; }
    Link                GetBeginDropHdl() const { return maBeginDropHdl; }

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link& rLink ) { maEndDropHdl = rLink; }
    Link            GetEndDropHdl() const { return maEndDropHdl; }

    /// specifies if auto-correction should capitalize the first word or not (default is on)
    void            SetFirstWordCapitalization( bool bCapitalize )  { bFirstWordCapitalization = bCapitalize; }
    bool            IsFirstWordCapitalization() const   { return bFirstWordCapitalization; }
};

inline EPaM ImpEditEngine::CreateEPaM( const EditPaM& rPaM )
{
    const ContentNode* pNode = rPaM.GetNode();
    return EPaM( aEditDoc.GetPos( pNode ), rPaM.GetIndex() );
}

inline EditPaM ImpEditEngine::CreateEditPaM( const EPaM& rEPaM )
{
    DBG_ASSERT( rEPaM.nPara < aEditDoc.Count(), "CreateEditPaM: invalid paragraph" );
    DBG_ASSERT( aEditDoc[ rEPaM.nPara ]->Len() >= rEPaM.nIndex, "CreateEditPaM: invalid Index" );
    return EditPaM( aEditDoc[ rEPaM.nPara], rEPaM.nIndex );
}

inline ESelection ImpEditEngine::CreateESel( const EditSelection& rSel )
{
    const ContentNode* pStartNode = rSel.Min().GetNode();
    const ContentNode* pEndNode = rSel.Max().GetNode();
    ESelection aESel;
    aESel.nStartPara = aEditDoc.GetPos( pStartNode );
    aESel.nStartPos = rSel.Min().GetIndex();
    aESel.nEndPara = aEditDoc.GetPos( pEndNode );
    aESel.nEndPos = rSel.Max().GetIndex();
    return aESel;
}

inline EditSelection ImpEditEngine::CreateSel( const ESelection& rSel )
{
    DBG_ASSERT( rSel.nStartPara < aEditDoc.Count(), "CreateSel: invalid start paragraph" );
    DBG_ASSERT( rSel.nEndPara < aEditDoc.Count(), "CreateSel: invalid end paragraph" );
    EditSelection aSel;
    aSel.Min().SetNode( aEditDoc[ rSel.nStartPara ] );
    aSel.Min().SetIndex( rSel.nStartPos );
    aSel.Max().SetNode( aEditDoc[ rSel.nEndPara ] );
    aSel.Max().SetIndex( rSel.nEndPos );
    DBG_ASSERT( !aSel.DbgIsBuggy( aEditDoc ), "CreateSel: incorrect selection!" );
    return aSel;
}

inline VirtualDevice* ImpEditEngine::GetVirtualDevice( const MapMode& rMapMode, sal_uLong nDrawMode )
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

    pVirtDev->SetDrawMode( nDrawMode );

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

inline EditUndoManager& ImpEditEngine::GetUndoManager()
{
    if ( !pUndoManager )
    {
        pUndoManager = new EditUndoManager();
        pUndoManager->SetEditEngine(pEditEngine);
    }
    return *pUndoManager;
}

inline ::svl::IUndoManager* ImpEditEngine::SetUndoManager(::svl::IUndoManager* pNew)
{
    ::svl::IUndoManager* pRetval = pUndoManager;

    if(pUndoManager)
    {
        pUndoManager->SetEditEngine(0);
    }

    pUndoManager = dynamic_cast< EditUndoManager* >(pNew);

    if(pUndoManager)
    {
        pUndoManager->SetEditEngine(pEditEngine);
    }

    return pRetval;
}

inline const ParaPortion* ImpEditEngine::FindParaPortion( const ContentNode* pNode ) const
{
    sal_Int32 nPos = aEditDoc.GetPos( pNode );
    DBG_ASSERT( nPos < GetParaPortions().Count(), "Portionloser Node?" );
    return GetParaPortions()[ nPos ];
}

inline ParaPortion* ImpEditEngine::FindParaPortion( ContentNode* pNode )
{
    sal_Int32 nPos = aEditDoc.GetPos( pNode );
    DBG_ASSERT( nPos < GetParaPortions().Count(), "Portionloser Node?" );
    return GetParaPortions()[ nPos ];
}

inline void ImpEditEngine::GetCharStretching( sal_uInt16& rX, sal_uInt16& rY ) const
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
sal_uInt8 GetCharTypeForCompression( sal_Unicode cChar );
Point Rotate( const Point& rPoint, short nOrientation, const Point& rOrigin );

#endif // _IMPEDIT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
