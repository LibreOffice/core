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
#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_IMPEDIT_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_IMPEDIT_HXX

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
#include <editeng/editview.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/cursor.hxx>
#include <vcl/vclptr.hxx>
#include <tools/fract.hxx>
#include <vcl/idle.hxx>

#include <vcl/dndhelp.hxx>
#include <svl/ondemand.hxx>
#include <svl/languageoptions.hxx>
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
#include <LibreOfficeKit/LibreOfficeKitTypes.h>

#include <boost/noncopyable.hpp>
#include <memory>
#include <vector>

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

#define LINE_SEP    '\x0A'

class EditView;
class EditEngine;
class OutlinerSearchable;

class SvxSearchItem;
class SvxLRSpaceItem;
class TextRanger;
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
class SvtCTLOptions;
namespace vcl { class Window; }
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
    VclPtr<VirtualDevice> pBackground;
    const SvxFieldItem* pField;
    bool            bVisCursor              : 1;
    bool            bDroppedInMe            : 1;
    bool            bStarterOfDD            : 1;
    bool            bHasValidData           : 1;
    bool            bUndoAction             : 1;
    bool            bOutlinerMode           : 1;
    bool            bDragAccepted           : 1;

    DragAndDropInfo()
    {
            pBackground = nullptr; bVisCursor = false; bDroppedInMe = false; bStarterOfDD = false;
            bHasValidData = false; bUndoAction = false; bOutlinerMode = false;
            nSensibleRange = 0; nCursorWidth = 0; pField = nullptr; nOutlinerDropDest = 0; bDragAccepted = false;
    }
};

struct ImplIMEInfos
{
    OUString    aOldTextAfterStartPos;
    sal_uInt16* pAttribs;
    EditPaM     aPos;
    sal_Int32   nLen;
    bool    bCursor;
    bool    bWasCursorOverwrite;

            ImplIMEInfos( const EditPaM& rPos, const OUString& rOldTextAfterStartPos );
            ~ImplIMEInfos();

    void    CopyAttribs( const sal_uInt16* pA, sal_uInt16 nL );
    void    DestroyAttribs();
};

// #i18881# to be able to identify the positions of changed words
// the positions of each portion need to be saved
typedef std::vector<EditSelection>  SpellContentSelections;

struct SpellInfo
{
    EESpellState    eState;
    EPaM            aSpellStart;
    EPaM            aSpellTo;
    EditPaM         aCurSentenceStart;
    bool        bSpellToEnd;
    bool        bMultipleDoc;
    svx::SpellPortions    aLastSpellPortions;
    SpellContentSelections  aLastSpellContentSelections;
    SpellInfo()
        { bSpellToEnd = true; eState = EE_SPELL_OK; bMultipleDoc = false; }
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

class IdleFormattter : public Idle
{
private:
    EditView*   pView;
    int         nRestarts;

public:
                IdleFormattter();
                virtual ~IdleFormattter();

    void        DoIdleFormat( EditView* pV );
    void        ForceTimeout();
    void        ResetRestarts() { nRestarts = 0; }
    EditView*   GetView()       { return pView; }
};


//  class ImpEditView

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
    vcl::Cursor*        pCursor;
    Color*              pBackgroundColor;
    /// Model callback.
    OutlinerSearchable* mpLibreOfficeKitSearchable;
    /// Per-view callback.
    OutlinerViewCallable* mpLibreOfficeKitViewCallable;
    EditEngine*         pEditEngine;
    VclPtr<vcl::Window> pOutWin;
    Pointer*            pPointer;
    DragAndDropInfo*    pDragAndDropInfo;

    css::uno::Reference< css::datatransfer::dnd::XDragSourceListener > mxDnDListener;


    long                nInvMore;
    EVControlBits       nControl;
    sal_uInt32          nTravelXPos;
    sal_uInt16          nExtraCursorFlags;
    sal_uInt16          nCursorBidiLevel;
    sal_uInt16          nScrollDiffX;
    bool                bReadOnly;
    bool                bClickedInSelection;
    bool                bActiveDragAndDropListener;

    Point               aAnchorPoint;
    Rectangle           aOutArea;
    Point               aVisDocStartPos;
    EESelectionMode     eSelectionMode;
    EditSelection       aEditSelection;
    EVAnchorMode        eAnchorMode;

protected:

    // DragAndDropClient
    void dragGestureRecognized(const css::datatransfer::dnd::DragGestureEvent& dge)
        throw (css::uno::RuntimeException,
               std::exception) override;
    void dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& dsde )
        throw (css::uno::RuntimeException,
               std::exception) override;
    void drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde)
        throw (css::uno::RuntimeException,
               std::exception) override;
    void dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (css::uno::RuntimeException, std::exception) override;
    void dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) throw (css::uno::RuntimeException, std::exception) override;
    void dragOver(const css::datatransfer::dnd::DropTargetDragEvent& dtde)
        throw (css::uno::RuntimeException,
               std::exception) override;

    void ShowDDCursor( const Rectangle& rRect );
    void HideDDCursor();

    void ImplDrawHighlightRect( OutputDevice* _pTarget, const Point& rDocPosTopLeft, const Point& rDocPosBottomRight, tools::PolyPolygon* pPolyPoly );

public:
                    ImpEditView( EditView* pView, EditEngine* pEng, vcl::Window* pWindow );
                    virtual ~ImpEditView();

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

    bool            IsVertical() const;

    bool            PostKeyEvent( const KeyEvent& rKeyEvent, vcl::Window* pFrameWin = nullptr );

    bool            MouseButtonUp( const MouseEvent& rMouseEvent );
    bool            MouseButtonDown( const MouseEvent& rMouseEvent );
    void            ReleaseMouse();
    bool            MouseMove( const MouseEvent& rMouseEvent );
    void            Command( const CommandEvent& rCEvt );

    void            CutCopy( css::uno::Reference< css::datatransfer::clipboard::XClipboard >& rxClipboard, bool bCut );
    void            Paste( css::uno::Reference< css::datatransfer::clipboard::XClipboard >& rxClipboard, bool bUseSpecial = false );

    void            SetVisDocStartPos( const Point& rPos ) { aVisDocStartPos = rPos; }

    long            GetVisDocLeft() const { return aVisDocStartPos.X(); }
    long            GetVisDocTop() const { return aVisDocStartPos.Y(); }
    long            GetVisDocRight() const { return aVisDocStartPos.X() + ( !IsVertical() ? aOutArea.GetWidth() : aOutArea.GetHeight() ); }
    long            GetVisDocBottom() const { return aVisDocStartPos.Y() + ( !IsVertical() ? aOutArea.GetHeight() : aOutArea.GetWidth() ); }
    Rectangle       GetVisDocArea() const;

    EditSelection&  GetEditSelection()          { return aEditSelection; }
    void            SetEditSelection( const EditSelection& rEditSelection );
    bool            HasSelection() const { return aEditSelection.HasRange(); }

    void            DrawSelection() { DrawSelection( aEditSelection ); }
    void            DrawSelection( EditSelection, vcl::Region* pRegion = nullptr, OutputDevice* pTargetDevice = nullptr );
    void GetSelectionRectangles(std::vector<Rectangle>& rLogicRects);

    vcl::Window*    GetWindow() const           { return pOutWin; }

    EESelectionMode GetSelectionMode() const    { return eSelectionMode; }
    void            SetSelectionMode( EESelectionMode eMode );

    inline const Pointer&   GetPointer();

    inline vcl::Cursor*     GetCursor();

    void            AddDragAndDropListeners();
    void            RemoveDragAndDropListeners();

    bool            IsBulletArea( const Point& rPos, sal_Int32* pPara );

//  For the Selection Engine...
    void            CreateAnchor();
    void            DeselectAll();
    bool            SetCursorAtPoint( const Point& rPointPixel );
    bool            IsSelectionAtPoint( const Point& rPosPixel );
    bool            IsInSelection( const EditPaM& rPaM );


    void            SetAnchorMode( EVAnchorMode eMode );
    EVAnchorMode    GetAnchorMode() const           { return eAnchorMode; }
    void            CalcAnchorPoint();
    void            RecalcOutputArea();

    void            ShowCursor( bool bGotoCursor, bool bForceVisCursor );
    Pair            Scroll( long ndX, long ndY, ScrollRangeCheck nRangeCheck = ScrollRangeCheck::NoNegative );

    void        SetInsertMode( bool bInsert );
    bool        IsInsertMode() const            { return !( nControl & EVControlBits::OVERWRITE ); }

    bool        IsPasteEnabled() const          { return bool( nControl & EVControlBits::ENABLEPASTE ); }

    bool        DoSingleLinePaste() const       { return bool( nControl & EVControlBits::SINGLELINEPASTE ); }
    bool        DoAutoScroll() const            { return bool( nControl & EVControlBits::AUTOSCROLL ); }
    bool        DoBigScroll() const             { return bool( nControl & EVControlBits::BIGSCROLL ); }
    bool        DoAutoSize() const              { return bool( nControl & EVControlBits::AUTOSIZE ); }
    bool        DoAutoWidth() const             { return bool( nControl & EVControlBits::AUTOSIZEX); }
    bool        DoAutoHeight() const            { return bool( nControl & EVControlBits::AUTOSIZEY); }
    bool        DoInvalidateMore() const        { return bool( nControl & EVControlBits::INVONEMORE ); }

    void            SetBackgroundColor( const Color& rColor );
    const Color&    GetBackgroundColor() const {
                        return ( pBackgroundColor ? *pBackgroundColor : pOutWin->GetBackground().GetColor() ); }

    /// @see vcl::ITiledRenderable::registerCallback().
    void registerLibreOfficeKitCallback(OutlinerSearchable* pSearchable);
    /// Invokes the registered model callback, if there are any.
    void libreOfficeKitCallback(int nType, const char* pPayload) const;
    /// @see vcl::ITiledRenderable::registerCallback().
    void registerLibreOfficeKitViewCallback(OutlinerViewCallable* pCallable);
    /// Invokes the registered view callback, if there are any.
    void libreOfficeKitViewCallback(int nType, const char* pPayload) const;

    bool            IsWrongSpelledWord( const EditPaM& rPaM, bool bMarkIfWrong );
    OUString        SpellIgnoreWord();

    const SvxFieldItem* GetField( const Point& rPos, sal_Int32* pPara, sal_Int32* pPos ) const;
    void            DeleteSelected();

    //  If possible invalidate more than OutputArea, for the DrawingEngine text frame
    void            SetInvalidateMore( sal_uInt16 nPixel ) { nInvMore = nPixel; }
    sal_uInt16      GetInvalidateMore() const { return (sal_uInt16)nInvMore; }
};


//  ImpEditEngine


class ImpEditEngine : public SfxListener, boost::noncopyable
{
    friend class EditEngine;
    friend class EditDbg;

    typedef EditEngine::ViewsType ViewsType;

private:


    // Data ...


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

    VclPtr< VirtualDevice> pVirtDev;
    VclPtr< OutputDevice > pRefDev;

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

    sal_Int32          nBigTextObjectStart;
    css::uno::Reference< css::linguistic2::XSpellChecker1 > xSpeller;
    css::uno::Reference< css::linguistic2::XHyphenator >    xHyphenator;
    SpellInfo*          pSpellInfo;
    mutable css::uno::Reference < css::i18n::XBreakIterator > xBI;
    mutable css::uno::Reference < css::i18n::XExtendedInputSequenceChecker > xISC;

    ConvInfo *          pConvInfo;

    OUString            aAutoCompleteText;

    InternalEditStatus  aStatus;

    LanguageType        eDefLanguage;

    OnDemandLocaleDataWrapper       xLocaleDataWrapper;
    OnDemandTransliterationWrapper  xTransliterationWrapper;

    // For Formatting / Update ....
    std::vector<std::unique_ptr<DeletedNodeInfo> > aDeletedNodes;
    Rectangle           aInvalidRect;
    sal_uInt32          nCurTextHeight;
    sal_uInt32          nCurTextHeightNTP;  // without trailing empty paragraphs
    sal_uInt16          nOnePixelInRef;

    IdleFormattter      aIdleFormatter;

    Timer               aOnlineSpellTimer;

    // For Chaining
    sal_Int32 mnOverflowingPara = -1;
    sal_Int32 mnOverflowingLine = -1;
    bool mbNeedsChainingHandling = false;

    // If it is detected at one point that the StatusHdl has to be called, but
    // this should not happen immediately (critical section):
    Timer               aStatusTimer;
    Link<EditStatus&,void>  aStatusHdlLink;
    Link<EENotify&,void>    aNotifyHdl;
    Link<ImportInfo&,void>  aImportHdl;
    Link<MoveParagraphsInfo&,void> aBeginMovingParagraphsHdl;
    Link<MoveParagraphsInfo&,void> aEndMovingParagraphsHdl;
    Link<PasteOrDropInfos&,void>   aBeginPasteOrDropHdl;
    Link<PasteOrDropInfos&,void>   aEndPasteOrDropHdl;
    Link<LinkParamNone*,void>      aModifyHdl;
    Link<EditView*,void>           maBeginDropHdl;
    Link<EditView*,void>           maEndDropHdl;

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


    // Methods...


    void                CursorMoved( ContentNode* pPrevNode );
    void                ParaAttribsChanged( ContentNode* pNode );
    void                TextModified();
    void                CalcHeight( ParaPortion* pPortion );

    void                InsertUndo( EditUndo* pUndo, bool bTryMerge = false );
    void                ResetUndoManager();
    bool            HasUndoManager() const  { return pUndoManager != nullptr; }

    EditUndoSetAttribs* CreateAttribUndo( EditSelection aSel, const SfxItemSet& rSet );

    EditTextObject* GetEmptyTextObject();

    EditPaM             GetPaM( Point aDocPos, bool bSmart = true );
    EditPaM             GetPaM( ParaPortion* pPortion, Point aPos, bool bSmart = true );
    long GetXPos(const ParaPortion* pParaPortion, const EditLine* pLine, sal_Int32 nIndex, bool bPreferPortionStart = false) const;
    long GetPortionXOffset(const ParaPortion* pParaPortion, const EditLine* pLine, sal_Int32 nTextPortion) const;
    sal_Int32 GetChar(const ParaPortion* pParaPortion, const EditLine* pLine, long nX, bool bSmart = true);
    Range               GetInvalidYOffsets( ParaPortion* pPortion );
    Range GetLineXPosStartEnd( const ParaPortion* pParaPortion, const EditLine* pLine ) const;

    void                ParaAttribsToCharAttribs( ContentNode* pNode );
    void                GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    EditTextObject*     CreateTextObject(EditSelection aSelection, SfxItemPool*, bool bAllowBigObjects = false, sal_Int32 nBigObjStart = 0);
    EditSelection       InsertTextObject( const EditTextObject&, EditPaM aPaM );
    EditSelection       InsertText( css::uno::Reference< css::datatransfer::XTransferable >& rxDataObj, const OUString& rBaseURL, const EditPaM& rPaM, bool bUseSpecial );

    void                CheckPageOverflow();

    EditPaM             Clear();
    EditPaM             RemoveText();
    bool                CreateLines( sal_Int32 nPara, sal_uInt32 nStartPosY );
    void                CreateAndInsertEmptyLine( ParaPortion* pParaPortion, sal_uInt32 nStartPosY );
    bool                FinishCreateLines( ParaPortion* pParaPortion );
    void                CreateTextPortions( ParaPortion* pParaPortion, sal_Int32& rStartPos /*, sal_Bool bCreateBlockPortions */ );
    void                RecalcTextPortion( ParaPortion* pParaPortion, sal_Int32 nStartPos, sal_Int32 nNewChars );
    sal_Int32           SplitTextPortion( ParaPortion* pParaPortion, sal_Int32 nPos,  EditLine* pCurLine = nullptr );
    void                SeekCursor( ContentNode* pNode, sal_Int32 nPos, SvxFont& rFont, OutputDevice* pOut = nullptr, sal_uInt16 nIgnoreWhich = 0 );
    void                RecalcFormatterFontMetrics( FormatterFontMetric& rCurMetrics, SvxFont& rFont );
    void                CheckAutoPageSize();

    void                ImpBreakLine( ParaPortion* pParaPortion, EditLine* pLine, TextPortion* pPortion, sal_Int32 nPortionStart, long nRemainingWidth, bool bCanHyphenate );
    void                ImpAdjustBlocks( ParaPortion* pParaPortion, EditLine* pLine, long nRemainingSpace );
    EditPaM             ImpConnectParagraphs( ContentNode* pLeft, ContentNode* pRight, bool bBackward = false );
    EditPaM             ImpDeleteSelection(const EditSelection& rCurSel);
    EditPaM             ImpInsertParaBreak( EditPaM& rPaM, bool bKeepEndingAttribs = true );
    EditPaM             ImpInsertParaBreak( const EditSelection& rEditSelection, bool bKeepEndingAttribs = true );
    EditPaM             ImpInsertText(const EditSelection& aCurEditSelection, const OUString& rStr);
    EditPaM             ImpInsertFeature(const EditSelection& rCurSel, const SfxPoolItem& rItem);
    void                ImpRemoveChars( const EditPaM& rPaM, sal_Int32 nChars );
    void                ImpRemoveParagraph( sal_Int32 nPara );
    EditSelection       ImpMoveParagraphs( Range aParagraphs, sal_Int32 nNewPos );

    EditPaM             ImpFastInsertText( EditPaM aPaM, const OUString& rStr );
    EditPaM             ImpFastInsertParagraph( sal_Int32 nPara );

    bool                ImpCheckRefMapMode();

    bool                ImplHasText() const;

    void                ImpFindKashidas( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, std::vector<sal_Int32>& rArray );

    void                InsertContent( ContentNode* pNode, sal_Int32 nPos );
    EditPaM             SplitContent( sal_Int32 nNode, sal_Int32 nSepPos );
    EditPaM             ConnectContents( sal_Int32 nLeftNode, bool bBackward );

    void                ShowParagraph( sal_Int32 nParagraph, bool bShow );

    EditPaM             PageUp( const EditPaM& rPaM, EditView* pView);
    EditPaM             PageDown( const EditPaM& rPaM, EditView* pView);
    EditPaM             CursorUp( const EditPaM& rPaM, EditView* pEditView );
    EditPaM             CursorDown( const EditPaM& rPaM, EditView* pEditView );
    EditPaM             CursorLeft( const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = css::i18n::CharacterIteratorMode::SKIPCELL );
    EditPaM             CursorRight( const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode = css::i18n::CharacterIteratorMode::SKIPCELL );
    EditPaM             CursorStartOfLine( const EditPaM& rPaM );
    EditPaM             CursorEndOfLine( const EditPaM& rPaM );
    static EditPaM      CursorStartOfParagraph( const EditPaM& rPaM );
    static EditPaM      CursorEndOfParagraph( const EditPaM& rPaM );
    EditPaM             CursorStartOfDoc();
    EditPaM             CursorEndOfDoc();
    EditPaM             WordLeft( const EditPaM& rPaM );
    EditPaM             WordRight( const EditPaM& rPaM, sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
    EditPaM             StartOfWord( const EditPaM& rPaM );
    EditPaM             EndOfWord( const EditPaM& rPaM, sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES );
    EditSelection       SelectWord( const EditSelection& rCurSelection, sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES, bool bAcceptStartOfWord = true );
    EditSelection       SelectSentence( const EditSelection& rCurSel ) const;
    EditPaM             CursorVisualLeftRight( EditView* pEditView, const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode, bool bToLeft );
    EditPaM             CursorVisualStartEnd( EditView* pEditView, const EditPaM& rPaM, bool bStart );


    void                InitScriptTypes( sal_Int32 nPara );
    sal_uInt16          GetI18NScriptType( const EditPaM& rPaM, sal_Int32* pEndPos = nullptr ) const;
    SvtScriptType       GetItemScriptType( const EditSelection& rSel ) const;
    bool                IsScriptChange( const EditPaM& rPaM ) const;
    bool                HasScriptType( sal_Int32 nPara, sal_uInt16 nType ) const;

    bool                ImplCalcAsianCompression( ContentNode* pNode, TextPortion* pTextPortion, sal_Int32 nStartPos,
                                                  long* pDXArray, sal_uInt16 n100thPercentFromMax, bool bManipulateDXArray );
    void                ImplExpandCompressedPortions( EditLine* pLine, ParaPortion* pParaPortion, long nRemainingWidth );

    void                ImplInitLayoutMode( OutputDevice* pOutDev, sal_Int32 nPara, sal_Int32 nIndex );
    LanguageType        ImplCalcDigitLang(LanguageType eCurLang) const;
    void                ImplInitDigitMode(OutputDevice* pOutDev, LanguageType eLang);
    static OUString     convertDigits(const OUString &rString, sal_Int32 nStt, sal_Int32 nLen, LanguageType eDigitLang);

    EditPaM             ReadText( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadRTF( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadXML( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadHTML( SvStream& rInput, const OUString& rBaseURL, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs );
    EditPaM             ReadBin( SvStream& rInput, EditSelection aSel );
    sal_uInt32          WriteText( SvStream& rOutput, EditSelection aSel );
    sal_uInt32          WriteRTF( SvStream& rOutput, EditSelection aSel );
    sal_uInt32          WriteXML(SvStream& rOutput, const EditSelection& rSel);
    sal_uInt32          WriteBin(SvStream& rOutput, const EditSelection& rSel, bool bStoreUnicode = false);

    void                WriteItemAsRTF( const SfxPoolItem& rItem, SvStream& rOutput, sal_Int32 nPara, sal_Int32 nPos,
                            std::vector<SvxFontItem*>& rFontTable, SvxColorList& rColorList );
    bool                WriteItemListAsRTF( ItemList& rLst, SvStream& rOutput, sal_Int32 nPara, sal_Int32 nPos,
                            std::vector<SvxFontItem*>& rFontTable, SvxColorList& rColorList );
    sal_Int32           LogicToTwips( sal_Int32 n );

    inline short        GetXValue( short nXValue ) const;
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

    inline VirtualDevice*   GetVirtualDevice( const MapMode& rMapMode, DrawModeFlags nDrawMode );
    inline void             EraseVirtualDevice() { pVirtDev.disposeAndClear(); }

    DECL_LINK_TYPED( StatusTimerHdl, Timer *, void);
    DECL_LINK_TYPED( IdleFormatHdl, Idle *, void);
    DECL_LINK_TYPED( OnlineSpellHdl, Timer *, void);
    DECL_LINK_TYPED( DocModified, LinkParamNone*, void );

    void                CheckIdleFormatter();

    inline const ParaPortion* FindParaPortion( const ContentNode* pNode ) const;
    inline ParaPortion* FindParaPortion( ContentNode* pNode );

    css::uno::Reference< css::datatransfer::XTransferable > CreateTransferable( const EditSelection& rSelection );

    void                SetValidPaperSize( const Size& rSz );

    css::uno::Reference < css::i18n::XBreakIterator > ImplGetBreakIterator() const;
    css::uno::Reference < css::i18n::XExtendedInputSequenceChecker > ImplGetInputSequenceChecker() const;

    void ImplUpdateOverflowingParaNum( sal_uInt32 );
    void ImplUpdateOverflowingLineNum( sal_uInt32, sal_uInt32, sal_uInt32 );

    SpellInfo *     CreateSpellInfo( bool bMultipleDocs );

    ImpEditEngine(EditEngine* pEditEngine, SfxItemPool* pPool);
    void InitDoc(bool bKeepParaAttribs);
    EditDoc&                GetEditDoc()            { return aEditDoc; }
    const EditDoc&          GetEditDoc() const      { return aEditDoc; }

    const ParaPortionList&  GetParaPortions() const { return aParaPortionList; }
    ParaPortionList&        GetParaPortions()       { return aParaPortionList; }

protected:
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

public:
                            virtual ~ImpEditEngine();

    inline EditUndoManager& GetUndoManager();
    inline ::svl::IUndoManager* SetUndoManager(::svl::IUndoManager* pNew);

    void                    SetUpdateMode( bool bUp, EditView* pCurView = nullptr, bool bForceUpdate = false );
    bool                    GetUpdateMode() const   { return bUpdate; }

    ViewsType& GetEditViews() { return aEditViews; }
    const ViewsType& GetEditViews() const { return aEditViews; }

    const Size&             GetPaperSize() const                    { return aPaperSize; }
    void                    SetPaperSize( const Size& rSz )         { aPaperSize = rSz; }

    void                    SetVertical( bool bVertical );
    bool                    IsVertical() const                      { return GetEditDoc().IsVertical(); }

    bool IsPageOverflow( ) const;

    void                    SetFixedCellHeight( bool bUseFixedCellHeight );
    bool                    IsFixedCellHeight() const { return GetEditDoc().IsFixedCellHeight(); }

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir ) { eDefaultHorizontalTextDirection = eHTextDir; }
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const { return eDefaultHorizontalTextDirection; }


    void                    InitWritingDirections( sal_Int32 nPara );
    bool                    IsRightToLeft( sal_Int32 nPara ) const;
    sal_uInt8               GetRightToLeft( sal_Int32 nPara, sal_Int32 nChar, sal_Int32* pStart = nullptr, sal_Int32* pEnd = nullptr );
    bool                    HasDifferentRTLLevels( const ContentNode* pNode );

    void                    SetTextRanger( TextRanger* pRanger );
    TextRanger*             GetTextRanger() const { return pTextRanger; }

    const Size&             GetMinAutoPaperSize() const             { return aMinAutoPaperSize; }
    void                    SetMinAutoPaperSize( const Size& rSz )  { aMinAutoPaperSize = rSz; }

    const Size&             GetMaxAutoPaperSize() const             { return aMaxAutoPaperSize; }
    void                    SetMaxAutoPaperSize( const Size& rSz )  { aMaxAutoPaperSize = rSz; }

    void                    FormatDoc();
    void                    FormatFullDoc();
    void                    UpdateViews( EditView* pCurView = nullptr );
    void                    Paint( ImpEditView* pView, const Rectangle& rRect, OutputDevice* pTargetDevice = nullptr, bool bUseVirtDev = false );
    void                    Paint( OutputDevice* pOutDev, Rectangle aClipRect, Point aStartPos, bool bStripOnly = false, short nOrientation = 0 );

    bool                MouseButtonUp( const MouseEvent& rMouseEvent, EditView* pView );
    bool                MouseButtonDown( const MouseEvent& rMouseEvent, EditView* pView );
    void                ReleaseMouse();
    bool                MouseMove( const MouseEvent& rMouseEvent, EditView* pView );
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
    EditPaM         DeleteSelected(const EditSelection& rEditSelection);
    EditPaM         InsertText( const EditSelection& rCurEditSelection, sal_Unicode c, bool bOverwrite, bool bIsUserInput = false );
    EditPaM         InsertText(const EditSelection& aCurEditSelection, const OUString& rStr);
    EditPaM         AutoCorrect( const EditSelection& rCurEditSelection, sal_Unicode c, bool bOverwrite, vcl::Window* pFrameWin = nullptr );
    EditPaM         DeleteLeftOrRight( const EditSelection& rEditSelection, sal_uInt8 nMode, sal_uInt8 nDelMode = DELMODE_SIMPLE );
    EditPaM         InsertParaBreak(const EditSelection& rEditSelection);
    EditPaM         InsertLineBreak(const EditSelection& aEditSelection);
    EditPaM         InsertTab(const EditSelection& rEditSelection);
    EditPaM         InsertField(const EditSelection& rCurSel, const SvxFieldItem& rFld);
    bool            UpdateFields();

    EditPaM         Read(SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, const EditSelection& rSel, SvKeyValueIterator* pHTTPHeaderAttrs = nullptr);
    void            Write(SvStream& rOutput, EETextFormat eFormat, const EditSelection& rSel);

    EditTextObject* CreateTextObject();
    EditTextObject* CreateTextObject(const EditSelection& rSel);
    void            SetText( const EditTextObject& rTextObject );
    EditSelection   InsertText( const EditTextObject& rTextObject, EditSelection aSel );

    EditSelection   MoveCursor( const KeyEvent& rKeyEvent, EditView* pEditView );

    EditSelection   MoveParagraphs( Range aParagraphs, sal_Int32 nNewPos, EditView* pCurView );

    sal_uInt32      CalcTextHeight( sal_uInt32* pHeightNTP );
    sal_uInt32      GetTextHeight() const;
    sal_uInt32      GetTextHeightNTP() const;
    sal_uInt32      CalcTextWidth( bool bIgnoreExtraSpace );
    sal_uInt32      CalcLineWidth( ParaPortion* pPortion, EditLine* pLine, bool bIgnoreExtraSpace );
    sal_Int32       GetLineCount( sal_Int32 nParagraph ) const;
    sal_Int32       GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const;
    void            GetLineBoundaries( /*out*/sal_Int32& rStart, /*out*/sal_Int32& rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const;
    sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const;
    sal_uInt16      GetLineHeight( sal_Int32 nParagraph, sal_Int32 nLine );
    sal_uInt32      GetParaHeight( sal_Int32 nParagraph );

    SfxItemSet      GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags = GetAttribsFlags::ALL ) const;
    SfxItemSet      GetAttribs( EditSelection aSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs_All  );
    void            SetAttribs( EditSelection aSel, const SfxItemSet& rSet, sal_uInt8 nSpecial = 0 );
    void            RemoveCharAttribs( EditSelection aSel, bool bRemoveParaAttribs, sal_uInt16 nWhich = 0 );
    void            RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich = 0, bool bRemoveFeatures = false );
    void            SetFlatMode( bool bFlat );

    void                SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    const SfxItemSet&   GetParaAttribs( sal_Int32 nPara ) const;

    bool            HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;

    Rectangle       PaMtoEditCursor( EditPaM aPaM, sal_uInt16 nFlags = 0 );
    Rectangle       GetEditCursor( ParaPortion* pPortion, sal_Int32 nIndex, sal_uInt16 nFlags = 0 );

    bool            IsModified() const      { return aEditDoc.IsModified(); }
    void            SetModifyFlag( bool b ) { aEditDoc.SetModified( b ); }
    void            SetModifyHdl( const Link<LinkParamNone*,void>& rLink ) { aModifyHdl = rLink; }
    Link<LinkParamNone*,void> GetModifyHdl() const { return aModifyHdl; }

    bool            IsInSelectionMode() { return bInSelection; }

    void            IndentBlock( EditView* pView, bool bRight );

//  For Undo/Redo
    bool            Undo( EditView* pView );
    bool            Redo( EditView* pView );

//  OV-Special
    void            InvalidateFromParagraph( sal_Int32 nFirstInvPara );
    EditPaM         InsertParagraph( sal_Int32 nPara );
    EditSelection*  SelectParagraph( sal_Int32 nPara );

    void            SetStatusEventHdl( const Link<EditStatus&, void>& rLink ) { aStatusHdlLink = rLink; }
    Link<EditStatus&,void> GetStatusEventHdl() const               { return aStatusHdlLink; }

    void            SetNotifyHdl( const Link<EENotify&,void>& rLink )     { aNotifyHdl = rLink; }
    Link<EENotify&,void>   GetNotifyHdl() const            { return aNotifyHdl; }

    void            FormatAndUpdate( EditView* pCurView = nullptr );
    inline void     IdleFormatAndUpdate( EditView* pCurView = nullptr );

    svtools::ColorConfig& GetColorConfig();
    bool            IsVisualCursorTravelingEnabled();
    bool            DoVisualCursorTraveling( const ContentNode* pNode );

    EditSelection         ConvertSelection( sal_Int32 nStartPara, sal_Int32 nStartPos, sal_Int32 nEndPara, sal_Int32 nEndPos );
    inline EPaM           CreateEPaM( const EditPaM& rPaM );
    inline EditPaM        CreateEditPaM( const EPaM& rEPaM );
    inline ESelection     CreateESel( const EditSelection& rSel );
    inline EditSelection  CreateSel( const ESelection& rSel );

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool() const { return pStylePool; }

    void                SetStyleSheet( EditSelection aSel, SfxStyleSheet* pStyle );
    void                SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle );
    const SfxStyleSheet* GetStyleSheet( sal_Int32 nPara ) const;
    SfxStyleSheet*      GetStyleSheet( sal_Int32 nPara );

    void                UpdateParagraphsWithStyleSheet( SfxStyleSheet* pStyle );
    void                RemoveStyleFromParagraphs( SfxStyleSheet* pStyle );

    OutputDevice*       GetRefDevice() const { return pRefDev.get(); }
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

    css::uno::Reference< css::linguistic2::XSpellChecker1 >
                        GetSpeller();
    void                SetSpeller( css::uno::Reference< css::linguistic2::XSpellChecker1 >  &xSpl )
                            { xSpeller = xSpl; }
    css::uno::Reference< css::linguistic2::XHyphenator >
                        GetHyphenator() const { return xHyphenator; }
    void                SetHyphenator( css::uno::Reference< css::linguistic2::XHyphenator >  &xHyph )
                            { xHyphenator = xHyph; }

    void GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const;
    void SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges );

    SpellInfo*          GetSpellInfo() const { return pSpellInfo; }

    void                SetDefaultLanguage( LanguageType eLang ) { eDefLanguage = eLang; }
    LanguageType        GetDefaultLanguage() const { return eDefLanguage; }

    LanguageType        GetLanguage( const EditPaM& rPaM, sal_Int32* pEndPos = nullptr ) const;
    css::lang::Locale   GetLocale( const EditPaM& rPaM ) const;

    void DoOnlineSpelling( ContentNode* pThisNodeOnly = nullptr, bool bSpellAtCursorPos = false, bool bInteruptable = true );
    EESpellState        Spell( EditView* pEditView, bool bMultipleDoc );
    EESpellState        HasSpellErrors();
    void                ClearSpellErrors();
    EESpellState        StartThesaurus( EditView* pEditView );
    css::uno::Reference< css::linguistic2::XSpellAlternatives >
                        ImpSpell( EditView* pEditView );

    // text conversion functions
    void                Convert( EditView* pEditView, LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont, sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc );
    void                ImpConvert( OUString &rConvTxt, LanguageType &rConvTxtLang, EditView* pEditView, LanguageType nSrcLang, const ESelection &rConvRange,
                                    bool bAllowImplicitChangesForNotConvertibleText, LanguageType nTargetLang, const vcl::Font *pTargetFont );
    ConvInfo *          GetConvInfo() const { return pConvInfo; }
    bool                HasConvertibleTextPortion( LanguageType nLang );
    void                SetLanguageAndFont( const ESelection &rESel,
                                LanguageType nLang, sal_uInt16 nLangWhichId,
                                const vcl::Font *pFont,  sal_uInt16 nFontWhichId );

    // returns true if input sequence checking should be applied
    bool                IsInputSequenceCheckingRequired( sal_Unicode nChar, const EditSelection& rCurSel ) const;

    //find the next error within the given selection - forward only!
    css::uno::Reference< css::linguistic2::XSpellAlternatives >
                        ImpFindNextError(EditSelection& rSelection);
    //spell and return a sentence
    bool                SpellSentence(EditView& rView, svx::SpellPortions& rToFill, bool bIsGrammarChecking );
    //put spelling back to start of current sentence - needed after switch of grammar support
    void                PutSpellingToSentenceStart( EditView& rEditView );
    //applies a changed sentence
    void                ApplyChangedSentence(EditView& rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck );
    //adds one or more portions of text to the SpellPortions depending on language changes
    void                AddPortionIterated(
                          EditView& rEditView,
                          const EditSelection &rSel,
                          const css::uno::Reference< css::linguistic2::XSpellAlternatives >& xAlt,
                          svx::SpellPortions& rToFill);
    //adds one portion to the SpellPortions
    void                AddPortion(
                            const EditSelection &rSel,
                            const css::uno::Reference< css::linguistic2::XSpellAlternatives >& xAlt,
                            svx::SpellPortions& rToFill,
                            bool bIsField );

    bool                    Search( const SvxSearchItem& rSearchItem, EditView* pView );
    bool                    ImpSearch( const SvxSearchItem& rSearchItem, const EditSelection& rSearchSelection, const EditPaM& rStartPos, EditSelection& rFoundSel );
    sal_Int32               StartSearchAndReplace( EditView* pEditView, const SvxSearchItem& rSearchItem );
    bool                    HasText( const SvxSearchItem& rSearchItem );

    void                    SetEditTextObjectPool( SfxItemPool* pP )    { pTextObjectPool = pP; }
    SfxItemPool*            GetEditTextObjectPool() const               { return pTextObjectPool; }

    const SvxNumberFormat * GetNumberFormat( const ContentNode* pNode ) const;
    sal_Int32               GetSpaceBeforeAndMinLabelWidth( const ContentNode *pNode, sal_Int32 *pnSpaceBefore = nullptr, sal_Int32 *pnMinLabelWidth = nullptr ) const;

    const SvxLRSpaceItem&   GetLRSpaceItem( ContentNode* pNode );
    SvxAdjust               GetJustification( sal_Int32 nPara ) const;
    SvxCellJustifyMethod    GetJustifyMethod( sal_Int32 nPara ) const;
    SvxCellVerJustify       GetVerJustification( sal_Int32 nPara ) const;

    void                SetCharStretching( sal_uInt16 nX, sal_uInt16 nY );
    inline void         GetCharStretching( sal_uInt16& rX, sal_uInt16& rY ) const;

    sal_Int32           GetBigTextObjectStart() const                               { return nBigTextObjectStart; }

    inline EditEngine*  GetEditEnginePtr() const    { return pEditEngine; }

    void                StartOnlineSpellTimer()     { aOnlineSpellTimer.Start(); }
    void                StopOnlineSpellTimer()      { aOnlineSpellTimer.Stop(); }

    const OUString&     GetAutoCompleteText() const { return aAutoCompleteText; }
    void                SetAutoCompleteText(const OUString& rStr, bool bUpdateTipWindow);

    EditSelection       TransliterateText( const EditSelection& rSelection, sal_Int32 nTransliterationMode );
    short               ReplaceTextOnly( ContentNode* pNode, sal_Int32 nCurrentStart, sal_Int32 nLen, const OUString& rText, const css::uno::Sequence< sal_Int32 >& rOffsets );

    void                SetAsianCompressionMode( sal_uInt16 n );
    sal_uInt16          GetAsianCompressionMode() const { return nAsianCompressionMode; }

    void                SetKernAsianPunctuation( bool b );
    bool                IsKernAsianPunctuation() const { return bKernAsianPunctuation; }

    sal_Int32 GetOverflowingParaNum() const { return mnOverflowingPara; }
    sal_Int32 GetOverflowingLineNum() const { return mnOverflowingLine; }
    void ClearOverflowingParaNum() { mnOverflowingPara = -1; }


    void                SetAddExtLeading( bool b );
    bool                IsAddExtLeading() const { return bAddExtLeading; }

    rtl::Reference<SvxForbiddenCharactersTable> GetForbiddenCharsTable() const;
    static void         SetForbiddenCharsTable( const rtl::Reference<SvxForbiddenCharactersTable>& xForbiddenChars );

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void                SetBeginDropHdl( const Link<EditView*,void>& rLink ) { maBeginDropHdl = rLink; }
    Link<EditView*,void>  GetBeginDropHdl() const { return maBeginDropHdl; }

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link<EditView*,void>& rLink ) { maEndDropHdl = rLink; }
    Link<EditView*,void>  GetEndDropHdl() const { return maEndDropHdl; }

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

inline VirtualDevice* ImpEditEngine::GetVirtualDevice( const MapMode& rMapMode, DrawModeFlags nDrawMode )
{
    if ( !pVirtDev )
        pVirtDev = VclPtr<VirtualDevice>::Create();

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
        pUndoManager->SetEditEngine(nullptr);
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

inline const Pointer& ImpEditView::GetPointer()
{
    if ( !pPointer )
    {
        pPointer = new Pointer( IsVertical() ? PointerStyle::TextVertical : PointerStyle::Text );
        return *pPointer;
    }

    if(PointerStyle::Text == pPointer->GetStyle() && IsVertical())
    {
        delete pPointer;
        pPointer = new Pointer(PointerStyle::TextVertical);
    }
    else if(PointerStyle::TextVertical == pPointer->GetStyle() && !IsVertical())
    {
        delete pPointer;
        pPointer = new Pointer(PointerStyle::Text);
    }

    return *pPointer;
}

inline vcl::Cursor* ImpEditView::GetCursor()
{
    if ( !pCursor )
        pCursor = new vcl::Cursor;
    return pCursor;
}

void ConvertItem( SfxPoolItem& rPoolItem, MapUnit eSourceUnit, MapUnit eDestUnit );
void ConvertAndPutItems( SfxItemSet& rDest, const SfxItemSet& rSource, const MapUnit* pSourceUnit = nullptr, const MapUnit* pDestUnit = nullptr );
sal_uInt8 GetCharTypeForCompression( sal_Unicode cChar );
Point Rotate( const Point& rPoint, short nOrientation, const Point& rOrigin );

#endif // INCLUDED_EDITENG_SOURCE_EDITENG_IMPEDIT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
