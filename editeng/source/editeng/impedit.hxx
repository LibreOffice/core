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
#pragma once

#include <eerdll2.hxx>
#include <editdoc.hxx>
#include "editsel.hxx"
#include "editundo.hxx"
#include "editstt2.hxx"
#include <editeng/editdata.hxx>
#include <editeng/SpellPortions.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/outliner.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cursor.hxx>
#include <vcl/vclptr.hxx>
#include <tools/fract.hxx>
#include <vcl/idle.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/ptrstyle.hxx>

#include <vcl/dndhelp.hxx>
#include <svl/ondemand.hxx>
#include <svl/languageoptions.hxx>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <i18nlangtag/lang.h>
#include <o3tl/deleter.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <functional>
#include <optional>
#include <memory>
#include <tuple>
#include <string_view>
#include <vector>

class EditView;
class EditEngine;
class OutlinerSearchable;

class SvxSearchItem;
class SvxLRSpaceItem;
class TextRanger;
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
namespace vcl { class Window; }
class SvxNumberFormat;
namespace com::sun::star::datatransfer::clipboard {
    class XClipboard;
}

namespace editeng {
    struct MisspellRanges;
}

#define DEL_LEFT    1
#define DEL_RIGHT   2
#define TRAVEL_X_DONTKNOW           0xFFFFFFFF
#define CURSOR_BIDILEVEL_DONTKNOW   0xFFFF
#define MAXCHARSINPARA              0x3FFF-CHARPOSGROW  // Max 16K, because WYSIWYG array
#define LINE_SEP    '\x0A'

#define ATTRSPECIAL_WHOLEWORD   1
#define ATTRSPECIAL_EDGE        2

struct CursorFlags
{
    bool bTextOnly : 1 = false;
    bool bStartOfLine : 1 = false;
    bool bEndOfLine : 1 = false;
    bool bPreferPortionStart : 1 = false;
};

struct DragAndDropInfo
{
    tools::Rectangle aCurCursor;
    tools::Rectangle aCurSavedCursor;
    sal_uInt16 nSensibleRange = 0;
    sal_uInt16 nCursorWidth = 0;
    ESelection aBeginDragSel;
    EditPaM aDropDest;
    sal_Int32 nOutlinerDropDest = 0;
    ESelection aDropSel;
    VclPtr<VirtualDevice> pBackground = nullptr;
    const SvxFieldItem* pField = nullptr;
    bool bVisCursor : 1 = false;
    bool bDroppedInMe : 1 = false;
    bool bStarterOfDD : 1 = false;
    bool bHasValidData : 1 = false;
    bool bUndoAction : 1 = false;
    bool bOutlinerMode : 1 = false;
    bool bDragAccepted : 1 = false;

    ~DragAndDropInfo()
    {
        pBackground.disposeAndClear();
    }
};

struct ImplIMEInfos
{
    OUString aOldTextAfterStartPos;
    std::unique_ptr<ExtTextInputAttr[]> pAttribs;
    EditPaM aPos;
    sal_Int32 nLen = 0;
    bool bWasCursorOverwrite = false;

    ImplIMEInfos(const EditPaM& rPos, OUString _aOldTextAfterStartPos)
        : aOldTextAfterStartPos(std::move(_aOldTextAfterStartPos))
        , aPos(rPos)
    {}

    void CopyAttribs(const ExtTextInputAttr* pInputAttributes, sal_uInt16 nInputLength)
    {
        nLen = nInputLength;
        pAttribs.reset(new ExtTextInputAttr[nInputLength]);
        memcpy(pAttribs.get(), pInputAttributes, nInputLength * sizeof(ExtTextInputAttr));
    }

    void DestroyAttribs()
    {
        pAttribs.reset();
        nLen = 0;
    }
};

// #i18881# to be able to identify the positions of changed words
// the positions of each portion need to be saved
typedef std::vector<EditSelection>  SpellContentSelections;

struct SpellInfo
{
    EditPaM aCurSentenceStart;
    svx::SpellPortions aLastSpellPortions;
    SpellContentSelections aLastSpellContentSelections;
    EESpellState eState = EESpellState::Ok;
    EPaM aSpellStart;
    EPaM aSpellTo;
    bool bSpellToEnd : 1 = true;
    bool bMultipleDoc : 1 = false;
};

// used for text conversion
struct ConvInfo
{
    EPaM aConvStart;
    EPaM aConvTo;
    EPaM aConvContinue;    // position to start search for next text portion (word) with
    bool bConvToEnd : 1 = true;
    bool bMultipleDoc : 1 = false;
};

struct FormatterFontMetric
{
    sal_uInt16 nMaxAscent = 0;
    sal_uInt16 nMaxDescent = 0;

    sal_uInt16 GetHeight() const
    {
        return nMaxAscent + nMaxDescent;
    }
};

class IdleFormattter : public Idle
{
private:
    EditView* mpView = nullptr;
    int mnRestarts = 0;

public:
    IdleFormattter();
    virtual ~IdleFormattter() override;

    void DoIdleFormat(EditView* pView);
    void ForceTimeout();
    void ResetRestarts() { mnRestarts = 0; }
    EditView* GetView() { return mpView; }
};

class ImpEditView;

/// This is meant just for Calc, where all positions in logical units (twips for LOK) are computed by
/// doing independent pixel-alignment for each cell's size. LOKSpecialPositioning stores
/// both 'output-area' and 'visible-doc-position' in pure logical unit (twips for LOK).
/// This allows the cursor/selection messages to be in regular(print) twips unit like in Writer.
class LOKSpecialPositioning
{
public:
    LOKSpecialPositioning(const ImpEditView& rImpEditView, MapUnit eUnit, const tools::Rectangle& rOutputArea,
                          const Point& rVisDocStartPos);

    void ReInit(MapUnit eUnit, const tools::Rectangle& rOutputArea, const Point& rVisDocStartPos);

    void SetOutputArea(const tools::Rectangle& rOutputArea);
    const tools::Rectangle& GetOutputArea() const;
    void SetVisDocStartPos(const Point& rVisDocStartPos);

    bool IsVertical() const;
    bool IsTopToBottom() const;

    tools::Long GetVisDocLeft() const { return maVisDocStartPos.X(); }
    tools::Long GetVisDocTop() const  { return maVisDocStartPos.Y(); }
    tools::Long GetVisDocRight() const  { return maVisDocStartPos.X() + (!IsVertical() ? maOutArea.GetWidth() : maOutArea.GetHeight()); }
    tools::Long GetVisDocBottom() const { return maVisDocStartPos.Y() + (!IsVertical() ? maOutArea.GetHeight() : maOutArea.GetWidth()); }
    tools::Rectangle GetVisDocArea() const;

    Point            GetWindowPos(const Point& rDocPos, MapUnit eDocPosUnit) const;
    tools::Rectangle GetWindowPos(const tools::Rectangle& rDocRect, MapUnit eDocRectUnit) const;

    void SetFlags(LOKSpecialFlags eFlags) { meFlags = eFlags; }
    bool IsLayoutRTL() { return bool(meFlags & LOKSpecialFlags::LayoutRTL); }

    Point GetRefPoint() const;

private:
    Point convertUnit(const Point& rPos, MapUnit ePosUnit) const;
    tools::Rectangle convertUnit(const tools::Rectangle& rRect, MapUnit eRectUnit) const;

    const ImpEditView& mrImpEditView;
    tools::Rectangle maOutArea;
    Point maVisDocStartPos;
    MapUnit meUnit;
    LOKSpecialFlags meFlags;
};



class ImpEditView : public vcl::unohelper::DragAndDropClient
{
    friend class EditView;
    friend class EditEngine;
    friend class ImpEditEngine;
    using vcl::unohelper::DragAndDropClient::dragEnter;
    using vcl::unohelper::DragAndDropClient::dragExit;
    using vcl::unohelper::DragAndDropClient::dragOver;

private:
    EditView* mpEditView;
    std::unique_ptr<vcl::Cursor, o3tl::default_delete<vcl::Cursor>> mpCursor;
    std::optional<Color> mxBackgroundColor;
    /// Containing view shell, if any.
    OutlinerViewShell* mpViewShell;
    /// Another shell, just listening to our state, if any.
    OutlinerViewShell* mpOtherShell;
    EditEngine* mpEditEngine;
    VclPtr<vcl::Window> mpOutputWindow;
    EditView::OutWindowSet maOutWindowSet;
    std::optional<PointerStyle> mxPointer;
    std::unique_ptr<DragAndDropInfo> mpDragAndDropInfo;

    css::uno::Reference<css::datatransfer::dnd::XDragSourceListener> mxDnDListener;

    tools::Long mnInvalidateMore;
    EVControlBits mnControl;
    sal_uInt32 mnTravelXPos;
    CursorFlags maExtraCursorFlags;
    sal_uInt16 mnCursorBidiLevel;
    sal_uInt16 mnScrollDiffX;
    bool mbReadOnly;
    bool mbClickedInSelection;
    bool mbActiveDragAndDropListener;

    Point maAnchorPoint;
    tools::Rectangle maOutputArea;
    Point maVisDocStartPos;
    EESelectionMode meSelectionMode;
    EditSelection maEditSelection;
    EEAnchorMode meAnchorMode;

    /// mechanism to change from the classic refresh mode that simply
    // invalidates the area where text was changed. When set, the invalidate
    // and the direct repaint of the Window-plugged EditView will be suppressed.
    // Instead, a consumer that has registered using an EditViewCallbacks
    // incarnation has to handle that. Used e.g. to represent the edited text
    // in Draw/Impress in an OverlayObject which avoids evtl. expensive full
    // repaints of the EditView(s)
    EditViewCallbacks* mpEditViewCallbacks;
    std::unique_ptr<LOKSpecialPositioning> mpLOKSpecialPositioning;
    bool mbBroadcastLOKViewCursor:1;
    bool mbSuppressLOKMessages:1;
    bool mbNegativeX:1;

    EditViewCallbacks* getEditViewCallbacks() const
    {
        return mpEditViewCallbacks;
    }

    void lokSelectionCallback(const std::optional<tools::PolyPolygon> &pPolyPoly, bool bStartHandleVisible, bool bEndHandleVisible);

    void setEditViewCallbacks(EditViewCallbacks* pEditViewCallbacks)
    {
        mpEditViewCallbacks = pEditViewCallbacks;
    }

    void InvalidateAtWindow(const tools::Rectangle& rRect);

    css::uno::Reference<css::datatransfer::clipboard::XClipboard> GetClipboard() const;

    void SetBroadcastLOKViewCursor(bool bSet)
    {
        mbBroadcastLOKViewCursor = bSet;
    }

protected:

    // DragAndDropClient
    void dragGestureRecognized(const css::datatransfer::dnd::DragGestureEvent& dge) override;
    void dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& dsde ) override;
    void drop(const css::datatransfer::dnd::DropTargetDropEvent& dtde) override;
    void dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) override;
    void dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) override;
    void dragOver(const css::datatransfer::dnd::DropTargetDragEvent& dtde) override;

    void ShowDDCursor( const tools::Rectangle& rRect );
    void HideDDCursor();

    void ImplDrawHighlightRect(OutputDevice& rTarget, const Point& rDocPosTopLeft, const Point& rDocPosBottomRight, tools::PolyPolygon* pPolyPoly, bool bLOKCalcRTL);
    tools::Rectangle ImplGetEditCursor(EditPaM& aPaM, CursorFlags aShowCursorFlags, sal_Int32& nTextPortionStart, ParaPortion const& rParaPortion) const;

public:
    ImpEditView(EditView* pView, EditEngine* pEditEngine, vcl::Window* pWindow);
    virtual ~ImpEditView() override;

    EditView* GetEditViewPtr() { return mpEditView; }

    EditEngine& getEditEngine() const { return *mpEditEngine; }
    ImpEditEngine& getImpEditEngine() const { return getEditEngine().getImpl(); }


    sal_uInt16 GetScrollDiffX() const { return mnScrollDiffX; }
    void SetScrollDiffX(sal_uInt16 n) { mnScrollDiffX = n; }

    sal_uInt16 GetCursorBidiLevel() const { return mnCursorBidiLevel; }
    void SetCursorBidiLevel(sal_uInt16 n) { mnCursorBidiLevel = n; }

    Point           GetDocPos( const Point& rWindowPos ) const;
    Point           GetWindowPos( const Point& rDocPos ) const;
    tools::Rectangle       GetWindowPos( const tools::Rectangle& rDocPos ) const;

    void                SetOutputArea( const tools::Rectangle& rRect );
    void                ResetOutputArea( const tools::Rectangle& rRect );
    const tools::Rectangle& GetOutputArea() const { return maOutputArea; }

    bool            IsVertical() const;
    bool            IsTopToBottom() const;

    bool            PostKeyEvent( const KeyEvent& rKeyEvent, vcl::Window const * pFrameWin );

    bool            MouseButtonUp( const MouseEvent& rMouseEvent );
    bool            MouseButtonDown( const MouseEvent& rMouseEvent );
    void            ReleaseMouse();
    bool            MouseMove( const MouseEvent& rMouseEvent );
    bool            Command(const CommandEvent& rCEvt);

    void            CutCopy( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard, bool bCut );
    void            Paste( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard, bool bUseSpecial = false, SotClipboardFormatId format = SotClipboardFormatId::NONE);

    void SetVisDocStartPos(const Point& rPos) { maVisDocStartPos = rPos; }

    tools::Long GetVisDocLeft() const { return maVisDocStartPos.X(); }
    tools::Long GetVisDocTop() const { return maVisDocStartPos.Y(); }
    tools::Long GetVisDocRight() const
    {
        return maVisDocStartPos.X() + ( !IsVertical() ? maOutputArea.GetWidth() : maOutputArea.GetHeight() );
    }
    tools::Long GetVisDocBottom() const
    {
        return maVisDocStartPos.Y() + ( !IsVertical() ? maOutputArea.GetHeight() : maOutputArea.GetWidth() );
    }
    tools::Rectangle       GetVisDocArea() const;

    const EditSelection&  GetEditSelection() const { return maEditSelection; }
    void SetEditSelection(const EditSelection& rEditSelection);
    bool HasSelection() const { return maEditSelection.HasRange(); }

    void SelectionChanged();
    void DrawSelectionXOR()
    {
        DrawSelectionXOR(maEditSelection);
    }
    void            DrawSelectionXOR( EditSelection, vcl::Region* pRegion = nullptr, OutputDevice* pTargetDevice = nullptr );
    void GetSelectionRectangles(EditSelection aTmpSel, std::vector<tools::Rectangle>& rLogicRects);

    void ScrollStateChange();

    OutputDevice&   GetOutputDevice() const;
    weld::Widget*   GetPopupParent(tools::Rectangle& rRect) const;
    vcl::Window* GetWindow() const { return mpOutputWindow; }

    void            SetSelectionMode( EESelectionMode eMode );

    PointerStyle GetPointer()
    {
        if ( !mxPointer )
        {
            mxPointer = IsVertical() ? PointerStyle::TextVertical : PointerStyle::Text;
            return *mxPointer;
        }

        if(PointerStyle::Text == *mxPointer && IsVertical())
        {
            mxPointer = PointerStyle::TextVertical;
        }
        else if(PointerStyle::TextVertical == *mxPointer && !IsVertical())
        {
            mxPointer = PointerStyle::Text;
        }

        return *mxPointer;
    }

    vcl::Cursor* GetCursor()
    {
        if (!mpCursor)
            mpCursor.reset(new vcl::Cursor);
        return mpCursor.get();
    }

    void            AddDragAndDropListeners();
    void            RemoveDragAndDropListeners();

    bool            IsBulletArea( const Point& rPos, sal_Int32* pPara );

//  For the Selection Engine...
    void            CreateAnchor();
    void            DeselectAll();
    bool            SetCursorAtPoint( const Point& rPointPixel );
    bool            IsSelectionAtPoint( const Point& rPosPixel );
    bool            IsInSelection( const EditPaM& rPaM );

    bool            IsSelectionFullPara() const;
    bool            IsSelectionInSinglePara() const;

    void            SetAnchorMode( EEAnchorMode eMode );
    EEAnchorMode GetAnchorMode() const { return meAnchorMode; }
    void            CalcAnchorPoint();
    void            RecalcOutputArea();

    tools::Rectangle GetEditCursor() const;

    void            ShowCursor( bool bGotoCursor, bool bForceVisCursor );
    Pair            Scroll( tools::Long ndX, tools::Long ndY, ScrollRangeCheck nRangeCheck = ScrollRangeCheck::NoNegative );

    void SetInsertMode( bool bInsert );
    bool IsInsertMode() const { return !(mnControl & EVControlBits::OVERWRITE); }

    bool IsPasteEnabled() const { return bool(mnControl & EVControlBits::ENABLEPASTE); }

    bool DoSingleLinePaste() const { return bool(mnControl & EVControlBits::SINGLELINEPASTE); }
    bool DoAutoScroll() const { return bool(mnControl & EVControlBits::AUTOSCROLL); }
    bool DoAutoSize() const { return bool(mnControl & EVControlBits::AUTOSIZE); }
    bool DoAutoWidth() const { return bool(mnControl & EVControlBits::AUTOSIZEX); }
    bool DoAutoHeight() const { return bool(mnControl & EVControlBits::AUTOSIZEY); }
    bool DoInvalidateMore() const { return bool(mnControl & EVControlBits::INVONEMORE ); }

    void        SetBackgroundColor( const Color& rColor );
    const Color& GetBackgroundColor() const;

    /// Informs this edit view about which view shell contains it.
    void RegisterViewShell(OutlinerViewShell* pViewShell);
    const OutlinerViewShell* GetViewShell() const;
    /// Informs this edit view about which other shell listens to it.
    void RegisterOtherShell(OutlinerViewShell* pViewShell);

    bool            IsWrongSpelledWord( const EditPaM& rPaM, bool bMarkIfWrong );
    OUString        SpellIgnoreWord();

    const SvxFieldItem* GetField( const Point& rPos, sal_Int32* pPara, sal_Int32* pPos ) const;
    void            DeleteSelected();

    //  If possible invalidate more than OutputArea, for the DrawingEngine text frame
    void SetInvalidateMore(sal_uInt16 nPixel)
    {
        mnInvalidateMore = nPixel;
    }
    sal_uInt16 GetInvalidateMore() const
    {
        return sal_uInt16(mnInvalidateMore);
    }

    void InitLOKSpecialPositioning(MapUnit eUnit, const tools::Rectangle& rOutputArea,
                                   const Point& rVisDocStartPos);
    void SetLOKSpecialOutputArea(const tools::Rectangle& rOutputArea);
    const tools::Rectangle & GetLOKSpecialOutputArea() const;
    void SetLOKSpecialVisArea(const tools::Rectangle& rVisArea);
    tools::Rectangle GetLOKSpecialVisArea() const;
    bool HasLOKSpecialPositioning() const;

    void SetLOKSpecialFlags(LOKSpecialFlags eFlags);

    void SuppressLOKMessages(bool bSet) { mbSuppressLOKMessages = bSet; }
    bool IsSuppressLOKMessages() const { return mbSuppressLOKMessages; }

    void SetNegativeX(bool bSet) { mbNegativeX = bSet; }
    bool IsNegativeX() const { return mbNegativeX; }
};


//  ImpEditEngine


class ImpEditEngine : public SfxListener, public svl::StyleSheetUser
{
    friend class EditEngine;

    typedef EditEngine::ViewsType ViewsType;

private:
    std::shared_ptr<editeng::SharedVclResources> pSharedVCL;

    // Document Specific data ...
    ParaPortionList maParaPortionList; // Formatting
    Size maPaperSize; // Layout
    Size maMinAutoPaperSize; // Layout ?
    Size maMaxAutoPaperSize; // Layout ?
    tools::Long mnMinColumnWrapHeight = 0; // Corresponds to graphic object height
    EditDoc maEditDoc; // Document content

    // Engine Specific data ...
    EditEngine* mpEditEngine;
    ViewsType maEditViews;
    EditView*  mpActiveView;
    std::unique_ptr<TextRanger> mpTextRanger;

    SfxStyleSheetPool* mpStylePool;
    SfxItemPool* mpTextObjectPool;

    VclPtr<VirtualDevice> mpVirtDev;
    VclPtr<OutputDevice> mpRefDev;
    VclPtr<VirtualDevice> mpOwnDev;

    svtools::ColorConfig maColorConfig;

    mutable std::unique_ptr<SfxItemSet> pEmptyItemSet;
    EditUndoManager* mpUndoManager;
    std::optional<ESelection> moUndoMarkSelection;

    std::unique_ptr<ImplIMEInfos> mpIMEInfos;

    OUString maWordDelimiters;

    EditSelFunctionSet  maSelFuncSet;
    EditSelectionEngine maSelEngine;

    Color               maBackgroundColor;

    ScalingParameters maCustomScalingParameters;
    ScalingParameters maScalingParameters;
    bool mbRoundToNearestPt;

    CharCompressType mnAsianCompressionMode;

    EEHorizontalTextDirection meDefaultHorizontalTextDirection;

    sal_Int32 mnBigTextObjectStart;
    css::uno::Reference<css::linguistic2::XSpellChecker1> mxSpeller;
    css::uno::Reference<css::linguistic2::XHyphenator>    mxHyphenator;
    std::unique_ptr<SpellInfo> mpSpellInfo;
    mutable css::uno::Reference <css::i18n::XBreakIterator> mxBI;
    mutable css::uno::Reference <css::i18n::XExtendedInputSequenceChecker> mxISC;

    std::unique_ptr<ConvInfo> mpConvInfo;

    OUString maAutoCompleteText;

    InternalEditStatus maStatus;

    LanguageType meDefLanguage;

    OnDemandLocaleDataWrapper       mxLocaleDataWrapper;
    OnDemandTransliterationWrapper  mxTransliterationWrapper;

    // For Formatting / Update...
    std::vector<std::unique_ptr<DeletedNodeInfo>> maDeletedNodes;
    tools::Rectangle maInvalidRect;
    tools::Long mnCurTextHeight;
    tools::Long mnCurTextHeightNTP;  // without trailing empty paragraphs
    sal_uInt16 mnOnePixelInRef;

    IdleFormattter maIdleFormatter;
    Timer maOnlineSpellTimer;

    // For Chaining
    sal_Int32 mnOverflowingPara = -1;
    sal_Int32 mnOverflowingLine = -1;
    bool mbNeedsChainingHandling = false;

    sal_Int16 mnColumns = 1;
    sal_Int32 mnColumnSpacing = 0;

    // If it is detected at one point that the StatusHdl has to be called, but
    // this should not happen immediately (critical section):
    Timer maStatusTimer;
    Size maLOKSpecialPaperSize;

    Link<EditStatus&,void>         maStatusHdlLink;
    Link<EENotify&,void>           maNotifyHdl;
    Link<HtmlImportInfo&,void>     maHtmlImportHdl;
    Link<RtfImportInfo&,void>      maRtfImportHdl;
    Link<MoveParagraphsInfo&,void> maBeginMovingParagraphsHdl;
    Link<MoveParagraphsInfo&,void> maEndMovingParagraphsHdl;
    Link<PasteOrDropInfos&,void>   maBeginPasteOrDropHdl;
    Link<PasteOrDropInfos&,void>   maEndPasteOrDropHdl;
    Link<LinkParamNone*,void>      maModifyHdl;
    Link<EditView*,void>           maBeginDropHdl;
    Link<EditView*,void>           maEndDropHdl;

    bool mbKernAsianPunctuation : 1;
    bool mbAddExtLeading : 1;
    bool mbIsFormatting : 1;
    bool mbFormatted : 1;
    bool mbInSelection : 1;
    bool mbIsInUndo : 1;
    bool mbUpdateLayout : 1;
    bool mbUndoEnabled : 1;
    bool mbDowning : 1;
    bool mbUseAutoColor : 1;
    bool mbForceAutoColor : 1;
    bool mbCallParaInsertedOrDeleted : 1;
    bool mbFirstWordCapitalization : 1;   // specifies if auto-correction should capitalize the first word or not
    bool mbLastTryMerge : 1;
    bool mbReplaceLeadingSingleQuotationMark : 1;
    bool mbSkipOutsideFormat : 1;
    bool mbFuzzing : 1;

    bool mbNbspRunNext;  // can't be a bitfield as it is passed as bool&

    // Methods...


    void                ParaAttribsChanged( ContentNode const * pNode, bool bIgnoreUndoCheck = false );
    void                TextModified();
    void                CalcHeight(ParaPortion& rParaPortion);
    bool isInEmptyClusterAtTheEnd(ParaPortion& rParaPortion);

    void                InsertUndo( std::unique_ptr<EditUndo> pUndo, bool bTryMerge = false );
    void                ResetUndoManager();
    bool            HasUndoManager() const  { return mpUndoManager != nullptr; }

    std::unique_ptr<EditUndoSetAttribs> CreateAttribUndo( EditSelection aSel, const SfxItemSet& rSet );

    std::unique_ptr<EditTextObject> GetEmptyTextObject();

    std::tuple<const ParaPortion*, const EditLine*, tools::Long> GetPortionAndLine(Point aDocPos);
    EditPaM             GetPaM( Point aDocPos, bool bSmart = true );
    bool IsTextPos(const Point& rDocPos, sal_uInt16 nBorder);
    tools::Long GetXPos(ParaPortion const& rParaPortion, EditLine const& rLine, sal_Int32 nIndex, bool bPreferPortionStart = false) const;
    tools::Long GetPortionXOffset(ParaPortion const& rParaPortion, EditLine const& rLine, sal_Int32 nTextPortion) const;
    sal_Int32 GetChar(ParaPortion const& rParaPortion, EditLine const& rLine, tools::Long nX, bool bSmart = true);
    Range GetLineXPosStartEnd(ParaPortion const& rParaPortion, EditLine const& rLine) const;

    void                ParaAttribsToCharAttribs( ContentNode* pNode );
    void                GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    std::unique_ptr<EditTextObject>
                        CreateTextObject(EditSelection aSelection, SfxItemPool*, bool bAllowBigObjects = false, sal_Int32 nBigObjStart = 0);
    EditSelection       InsertTextObject( const EditTextObject&, EditPaM aPaM );
    EditSelection       PasteText( css::uno::Reference< css::datatransfer::XTransferable > const & rxDataObj, const OUString& rBaseURL, const EditPaM& rPaM, bool bUseSpecial, SotClipboardFormatId format = SotClipboardFormatId::NONE);

    void                CheckPageOverflow();

    void                Clear();
    EditPaM             RemoveText();

    bool createLinesForEmptyParagraph(ParaPortion& rParaPortion);
    tools::Long calculateMaxLineWidth(tools::Long nStartX, SvxLRSpaceItem const& rLRItem);
    bool CreateLines(sal_Int32 nPara, sal_uInt32 nStartPosY);

    void                CreateAndInsertEmptyLine(ParaPortion& rParaPortion);
    bool                FinishCreateLines(ParaPortion& rParaPortion);
    void                CreateTextPortions(ParaPortion& rParaPortion, sal_Int32& rStartPos);
    void                RecalcTextPortion(ParaPortion& rParaPortion, sal_Int32 nStartPos, sal_Int32 nNewChars);
    sal_Int32           SplitTextPortion(ParaPortion& rParaPortion, sal_Int32 nPos,  EditLine* pCurLine = nullptr);
    void                SeekCursor( ContentNode* pNode, sal_Int32 nPos, SvxFont& rFont, OutputDevice* pOut = nullptr );
    void                RecalcFormatterFontMetrics( FormatterFontMetric& rCurMetrics, SvxFont& rFont );
    void                CheckAutoPageSize();

    void                ImpBreakLine(ParaPortion& rParaPortion, EditLine& rLine, TextPortion const * pPortion, sal_Int32 nPortionStart, tools::Long nRemainingWidth, bool bCanHyphenate);
    void                ImpAdjustBlocks(ParaPortion& rParaPortion, EditLine& rLine, tools::Long nRemainingSpace );
    EditPaM             ImpConnectParagraphs( ContentNode* pLeft, ContentNode* pRight, bool bBackward = false );
    EditPaM             ImpDeleteSelection(const EditSelection& rCurSel);
    EditPaM             ImpInsertParaBreak( EditPaM& rPaM, bool bKeepEndingAttribs = true );
    EditPaM             ImpInsertParaBreak( const EditSelection& rEditSelection );
    EditPaM             ImpInsertText(const EditSelection& aCurEditSelection, const OUString& rStr);
    EditPaM             ImpInsertFeature(const EditSelection& rCurSel, const SfxPoolItem& rItem);
    void                ImpRemoveChars( const EditPaM& rPaM, sal_Int32 nChars );
    void                ImpRemoveParagraph( sal_Int32 nPara );
    EditSelection       ImpMoveParagraphs( Range aParagraphs, sal_Int32 nNewPos );

    EditPaM             ImpFastInsertText( EditPaM aPaM, const OUString& rStr );
    EditPaM             ImpFastInsertParagraph( sal_Int32 nPara );

    bool                ImplHasText() const;

    void                ImpFindKashidas( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, std::vector<sal_Int32>& rArray );

    void                InsertContent(std::unique_ptr<ContentNode> pNode, sal_Int32 nPos);
    EditPaM             SplitContent( sal_Int32 nNode, sal_Int32 nSepPos );
    EditPaM             ConnectContents( sal_Int32 nLeftNode, bool bBackward );

    void                ShowParagraph( sal_Int32 nParagraph, bool bShow );

    EditPaM             PageUp( const EditPaM& rPaM, EditView const * pView);
    EditPaM             PageDown( const EditPaM& rPaM, EditView const * pView);
    EditPaM             CursorUp( const EditPaM& rPaM, EditView const * pEditView );
    EditPaM             CursorDown( const EditPaM& rPaM, EditView const * pEditView );
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
    EditPaM             EndOfWord( const EditPaM& rPaM );
    EditSelection       SelectWord( const EditSelection& rCurSelection, sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES, bool bAcceptStartOfWord = true, bool bAcceptEndOfWord = false );
    EditSelection       SelectSentence( const EditSelection& rCurSel ) const;
    EditPaM             CursorVisualLeftRight( EditView const * pEditView, const EditPaM& rPaM, sal_uInt16 nCharacterIteratorMode, bool bToLeft );
    EditPaM             CursorVisualStartEnd( EditView const * pEditView, const EditPaM& rPaM, bool bStart );


    void                InitScriptTypes( sal_Int32 nPara );
    sal_uInt16          GetI18NScriptType( const EditPaM& rPaM, sal_Int32* pEndPos = nullptr ) const;
    SvtScriptType       GetItemScriptType( const EditSelection& rSel ) const;
    bool                IsScriptChange( const EditPaM& rPaM ) const;
    bool                HasScriptType( sal_Int32 nPara, sal_uInt16 nType ) const;

    bool                ImplCalcAsianCompression( ContentNode* pNode, TextPortion* pTextPortion, sal_Int32 nStartPos,
                                                sal_Int32* pDXArray, sal_uInt16 n100thPercentFromMax, bool bManipulateDXArray );
    void                ImplExpandCompressedPortions(EditLine& rLine, ParaPortion& rParaPortion, tools::Long nRemainingWidth);

    void                ImplInitLayoutMode(OutputDevice& rOutDev, sal_Int32 nPara, sal_Int32 nIndex);
    static LanguageType ImplCalcDigitLang(LanguageType eCurLang);
    static void         ImplInitDigitMode(OutputDevice& rOutDev, LanguageType eLang);
    static OUString     convertDigits(std::u16string_view rString, sal_Int32 nStt, sal_Int32 nLen, LanguageType eDigitLang);

    EditPaM             ReadText( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadRTF( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadXML( SvStream& rInput, EditSelection aSel );
    EditPaM             ReadHTML( SvStream& rInput, const OUString& rBaseURL, EditSelection aSel, SvKeyValueIterator* pHTTPHeaderAttrs );
    ErrCode             WriteText( SvStream& rOutput, EditSelection aSel );
    ErrCode             WriteRTF( SvStream& rOutput, EditSelection aSel );
    void                WriteXML(SvStream& rOutput, const EditSelection& rSel);

    void                WriteItemAsRTF( const SfxPoolItem& rItem, SvStream& rOutput, sal_Int32 nPara, sal_Int32 nPos,
                            std::vector<std::unique_ptr<SvxFontItem>>& rFontTable, SvxColorList& rColorList );
    bool                WriteItemListAsRTF( ItemList& rLst, SvStream& rOutput, sal_Int32 nPara, sal_Int32 nPos,
                            std::vector<std::unique_ptr<SvxFontItem>>& rFontTable, SvxColorList& rColorList );
    sal_Int32           LogicToTwips( sal_Int32 n );

    double scaleXSpacingValue(tools::Long nXValue) const
    {
        if (!maStatus.DoStretch() || maScalingParameters.fSpacingX == 1.0)
            return nXValue;

        return double(nXValue) * maScalingParameters.fSpacingX;
    }

    double scaleYSpacingValue(sal_uInt16 nYValue) const
    {
        if (!maStatus.DoStretch() || maScalingParameters.fSpacingY == 1.0)
            return nYValue;

        return double(nYValue) * maScalingParameters.fSpacingY;
    }

    double scaleXFontValue(tools::Long nXValue) const
    {
        if (!maStatus.DoStretch() || (maScalingParameters.fFontX == 1.0))
            return nXValue;

        return double(nXValue) * maScalingParameters.fFontX;
    }

    double scaleYFontValue(sal_uInt16 nYValue) const
    {
        if (!maStatus.DoStretch() || (maScalingParameters.fFontY == 1.0))
            return nYValue;

        return double(nYValue) * maScalingParameters.fFontY;
    }

    void setRoundToNearestPt(bool bRound) { mbRoundToNearestPt = bRound; }
    double roundToNearestPt(double fInput) const;

    ContentNode*        GetPrevVisNode( ContentNode const * pCurNode );
    ContentNode*        GetNextVisNode( ContentNode const * pCurNode );

    const ParaPortion*  GetPrevVisPortion( const ParaPortion* pCurPortion ) const;
    const ParaPortion*  GetNextVisPortion( const ParaPortion* pCurPortion ) const;

    void                SetBackgroundColor( const Color& rColor ) { maBackgroundColor = rColor; }
    const Color&        GetBackgroundColor() const { return maBackgroundColor; }

    tools::Long                CalcVertLineSpacing(Point& rStartPos) const;

    Color               GetAutoColor() const;
    void EnableAutoColor( bool b ) { mbUseAutoColor = b; }
    bool IsAutoColorEnabled() const { return mbUseAutoColor; }
    void ForceAutoColor( bool b ) { mbForceAutoColor = b; }
    bool IsForceAutoColor() const { return mbForceAutoColor; }

    VirtualDevice* GetVirtualDevice(const MapMode& rMapMode, DrawModeFlags nDrawMode)
    {
        if (!mpVirtDev)
            mpVirtDev = VclPtr<VirtualDevice>::Create();

        if ((mpVirtDev->GetMapMode().GetMapUnit() != rMapMode.GetMapUnit()) ||
            (mpVirtDev->GetMapMode().GetScaleX() != rMapMode.GetScaleX()) ||
            (mpVirtDev->GetMapMode().GetScaleY() != rMapMode.GetScaleY()) )
        {
            MapMode aMapMode(rMapMode);
            aMapMode.SetOrigin(Point(0, 0));
            mpVirtDev->SetMapMode(aMapMode);
        }

        mpVirtDev->SetDrawMode(nDrawMode);

        return mpVirtDev;
    }

    void EraseVirtualDevice() { mpVirtDev.disposeAndClear(); }

    DECL_LINK( StatusTimerHdl, Timer *, void);
    DECL_LINK( IdleFormatHdl, Timer *, void);
    DECL_LINK( OnlineSpellHdl, Timer *, void);
    DECL_LINK( DocModified, LinkParamNone*, void );

    void                CheckIdleFormatter();

    const ParaPortion* FindParaPortion(const ContentNode* pNode) const
    {
        sal_Int32 nPos = maEditDoc.GetPos( pNode );
        DBG_ASSERT( nPos < GetParaPortions().Count(), "Portionloser Node?" );
        return GetParaPortions().SafeGetObject(nPos);
    }

    ParaPortion* FindParaPortion(ContentNode const * pNode)
    {
        sal_Int32 nPos = maEditDoc.GetPos( pNode );
        DBG_ASSERT( nPos < GetParaPortions().Count(), "Portionloser Node?" );
        return GetParaPortions().SafeGetObject(nPos);
    }

    css::uno::Reference< css::datatransfer::XTransferable > CreateTransferable( const EditSelection& rSelection );

    void                SetValidPaperSize( const Size& rSz );

    css::uno::Reference < css::i18n::XBreakIterator > const & ImplGetBreakIterator() const;
    css::uno::Reference < css::i18n::XExtendedInputSequenceChecker > const & ImplGetInputSequenceChecker() const;

    void ImplUpdateOverflowingParaNum(tools::Long);
    void ImplUpdateOverflowingLineNum(tools::Long, sal_uInt32, tools::Long);

    void CreateSpellInfo( bool bMultipleDocs );
    /// Obtains a view shell ID from the active EditView.
    ViewShellId CreateViewShellId();

    ImpEditEngine(EditEngine* pEditEngine, SfxItemPool* pPool);
    void InitDoc(bool bKeepParaAttribs);
    EditDoc&                GetEditDoc()            { return maEditDoc; }
    const EditDoc&          GetEditDoc() const      { return maEditDoc; }

    const ParaPortionList&  GetParaPortions() const { return maParaPortionList; }
    ParaPortionList&        GetParaPortions()       { return maParaPortionList; }

    tools::Long Calc1ColumnTextHeight(tools::Long* pHeightNTP);

    void IdleFormatAndLayout(EditView* pCurView) { maIdleFormatter.DoIdleFormat(pCurView); }

protected:
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

public:
                            virtual ~ImpEditEngine() override;
                            ImpEditEngine(const ImpEditEngine&) = delete;
    ImpEditEngine&          operator=(const ImpEditEngine&) = delete;

    EditUndoManager& GetUndoManager()
    {
        if (!mpUndoManager)
        {
            mpUndoManager = new EditUndoManager();
            mpUndoManager->SetEditEngine(mpEditEngine);
        }
        return *mpUndoManager;
    }

    EditUndoManager* SetUndoManager(EditUndoManager* pNew)
    {
        EditUndoManager* pRetval = mpUndoManager;

        if (mpUndoManager)
        {
            mpUndoManager->SetEditEngine(nullptr);
        }

        mpUndoManager = pNew;

        if (mpUndoManager)
        {
            mpUndoManager->SetEditEngine(mpEditEngine);
        }

        return pRetval;
    }

    // @return the previous bUpdateLayout state
    bool                    SetUpdateLayout( bool bUpdate, EditView* pCurView = nullptr, bool bForceUpdate = false );
    bool IsUpdateLayout() const   { return mbUpdateLayout; }

    ViewsType& GetEditViews() { return maEditViews; }
    const ViewsType& GetEditViews() const { return maEditViews; }

    const Size& GetPaperSize() const { return maPaperSize; }
    void SetPaperSize(const Size& rSize) { maPaperSize = rSize; }

    void                    SetVertical( bool bVertical);
    bool                    IsEffectivelyVertical() const                      { return GetEditDoc().IsEffectivelyVertical(); }
    bool                    IsTopToBottom() const                   { return GetEditDoc().IsTopToBottom(); }
    bool                    GetVertical() const               { return GetEditDoc().GetVertical(); }
    void                    SetRotation( TextRotation nRotation);
    TextRotation            GetRotation() const                     { return GetEditDoc().GetRotation(); }

    void SetTextColumns(sal_Int16 nColumns, sal_Int32 nSpacing);

    bool IsPageOverflow( ) const;

    void                    SetFixedCellHeight( bool bUseFixedCellHeight );
    bool                    IsFixedCellHeight() const { return GetEditDoc().IsFixedCellHeight(); }

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir ) { meDefaultHorizontalTextDirection = eHTextDir; }
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const { return meDefaultHorizontalTextDirection; }


    void                    InitWritingDirections( sal_Int32 nPara );
    bool                    IsRightToLeft( sal_Int32 nPara ) const;
    sal_uInt8               GetRightToLeft( sal_Int32 nPara, sal_Int32 nChar, sal_Int32* pStart = nullptr, sal_Int32* pEnd = nullptr );
    bool                    HasDifferentRTLLevels( const ContentNode* pNode );

    void                    SetTextRanger( std::unique_ptr<TextRanger> pRanger );
    TextRanger*             GetTextRanger() const { return mpTextRanger.get(); }

    const Size& GetMinAutoPaperSize() const { return maMinAutoPaperSize; }
    void SetMinAutoPaperSize(const Size& rSize) { maMinAutoPaperSize = rSize; }

    const Size& GetMaxAutoPaperSize() const { return maMaxAutoPaperSize; }
    void SetMaxAutoPaperSize(const Size& rSize) { maMaxAutoPaperSize = rSize; }

    void SetMinColumnWrapHeight(tools::Long nVal) { mnMinColumnWrapHeight = nVal; }

    tools::Long FormatParagraphs(o3tl::sorted_vector<sal_Int32>& rRepaintParagraphs);
    void ScaleContentToFitWindow(o3tl::sorted_vector<sal_Int32>& rRepaintParagraphs);
    void FormatDoc();
    void FormatFullDoc();

    void                    UpdateViews( EditView* pCurView = nullptr );
    void                    Paint( ImpEditView* pView, const tools::Rectangle& rRect, OutputDevice* pTargetDevice );
    void                    Paint(OutputDevice& rOutDev, tools::Rectangle aClipRect, Point aStartPos, bool bStripOnly = false, Degree10 nOrientation = 0_deg10);

    bool                MouseButtonUp( const MouseEvent& rMouseEvent, EditView* pView );
    bool                MouseButtonDown( const MouseEvent& rMouseEvent, EditView* pView );
    void                ReleaseMouse();
    bool                MouseMove( const MouseEvent& rMouseEvent, EditView* pView );
    bool                    Command(const CommandEvent& rCEvt, EditView* pView);

    EditSelectionEngine&    GetSelEngine() { return maSelEngine; }
    OUString                GetSelected( const EditSelection& rSel ) const;

    const SfxItemSet& GetEmptyItemSet() const;

    void                    UpdateSelections();

    void                EnableUndo( bool bEnable );
    bool IsUndoEnabled() const { return mbUndoEnabled; }
    void SetUndoMode( bool b ) { mbIsInUndo = b; }
    bool IsInUndo() const { return mbIsInUndo; }

    void SetCallParaInsertedOrDeleted( bool b ) { mbCallParaInsertedOrDeleted = b; }
    bool IsCallParaInsertedOrDeleted() const { return mbCallParaInsertedOrDeleted; }

    bool IsFormatted() const { return mbFormatted; }
    bool IsFormatting() const { return mbIsFormatting; }

    void            SetText(const OUString& rText);
    EditPaM         DeleteSelected(const EditSelection& rEditSelection);
    EditPaM         InsertTextUserInput( const EditSelection& rCurEditSelection, sal_Unicode c, bool bOverwrite );
    EditPaM         InsertText(const EditSelection& aCurEditSelection, const OUString& rStr);
    EditPaM         AutoCorrect( const EditSelection& rCurEditSelection, sal_Unicode c, bool bOverwrite, vcl::Window const * pFrameWin = nullptr );
    EditPaM         DeleteLeftOrRight( const EditSelection& rEditSelection, sal_uInt8 nMode, DeleteMode nDelMode );
    EditPaM         InsertParaBreak(const EditSelection& rEditSelection);
    EditPaM         InsertLineBreak(const EditSelection& aEditSelection);
    EditPaM         InsertTab(const EditSelection& rEditSelection);
    EditPaM         InsertField(const EditSelection& rCurSel, const SvxFieldItem& rFld);
    bool            UpdateFields();

    EditPaM         Read(SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, const EditSelection& rSel, SvKeyValueIterator* pHTTPHeaderAttrs = nullptr);
    void            Write(SvStream& rOutput, EETextFormat eFormat, const EditSelection& rSel);

    std::unique_ptr<EditTextObject> CreateTextObject();
    std::unique_ptr<EditTextObject> CreateTextObject(const EditSelection& rSel);
    void            SetText( const EditTextObject& rTextObject );
    EditSelection   InsertText( const EditTextObject& rTextObject, EditSelection aSel );

    EditSelection const & MoveCursor( const KeyEvent& rKeyEvent, EditView* pEditView );

    EditSelection   MoveParagraphs( Range aParagraphs, sal_Int32 nNewPos, EditView* pCurView );

    tools::Long     CalcTextHeight( tools::Long* pHeightNTP );
    sal_uInt32      GetTextHeight() const;
    sal_uInt32      GetTextHeightNTP() const;
    sal_uInt32      CalcTextWidth( bool bIgnoreExtraSpace);
    sal_uInt32      CalcParaWidth( sal_Int32 nParagraph, bool bIgnoreExtraSpace );
    sal_uInt32      CalcLineWidth(ParaPortion const& rPortion, EditLine const& rLine, bool bIgnoreExtraSpace);
    sal_Int32       GetLineCount( sal_Int32 nParagraph ) const;
    sal_Int32       GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const;
    void            GetLineBoundaries( /*out*/sal_Int32& rStart, /*out*/sal_Int32& rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const;
    sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const;
    sal_uInt16      GetLineHeight( sal_Int32 nParagraph, sal_Int32 nLine );
    sal_uInt32 GetParaHeight(sal_Int32 nParagraph) const;

    SfxItemSet      GetAttribs( sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, GetAttribsFlags nFlags = GetAttribsFlags::ALL ) const;
    SfxItemSet      GetAttribs( EditSelection aSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs::All  );
    void            SetAttribs( EditSelection aSel, const SfxItemSet& rSet, SetAttribsMode nSpecial = SetAttribsMode::NONE, bool bSetSelection = true );
    void            RemoveCharAttribs( EditSelection aSel, EERemoveParaAttribsMode eMode, sal_uInt16 nWhich );
    void            RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich = 0, bool bRemoveFeatures = false );
    void            SetFlatMode( bool bFlat );

    void                SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    const SfxItemSet&   GetParaAttribs( sal_Int32 nPara ) const;

    bool            HasParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    const SfxPoolItem&  GetParaAttrib( sal_Int32 nPara, sal_uInt16 nWhich ) const;
    template<class T>
    const T&            GetParaAttrib( sal_Int32 nPara, TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T&>(GetParaAttrib(nPara, sal_uInt16(nWhich)));
    }

    tools::Rectangle PaMtoEditCursor(EditPaM aPaM, CursorFlags aFlags = CursorFlags());
    tools::Rectangle GetEditCursor(ParaPortion const& rPortion, EditLine const& rLine, sal_Int32 nIndex, CursorFlags aFlags);

    bool            IsModified() const { return maEditDoc.IsModified(); }
    void            SetModifyFlag(bool b) { maEditDoc.SetModified( b ); }
    void            SetModifyHdl( const Link<LinkParamNone*,void>& rLink ) { maModifyHdl = rLink; }

    bool IsInSelectionMode() const { return mbInSelection; }

//  For Undo/Redo
    void            Undo( EditView* pView );
    void            Redo( EditView* pView );

//  OV-Special
    void            InvalidateFromParagraph( sal_Int32 nFirstInvPara );
    EditPaM         InsertParagraph( sal_Int32 nPara );
    std::optional<EditSelection> SelectParagraph( sal_Int32 nPara );

    void            SetStatusEventHdl( const Link<EditStatus&, void>& rLink ) { maStatusHdlLink = rLink; }
    const Link<EditStatus&,void>& GetStatusEventHdl() const               { return maStatusHdlLink; }

    void            SetNotifyHdl( const Link<EENotify&,void>& rLink )     { maNotifyHdl = rLink; }
    const Link<EENotify&,void>&   GetNotifyHdl() const            { return maNotifyHdl; }

    void            FormatAndLayout( EditView* pCurView = nullptr, bool bCalledFromUndo = false );

    const svtools::ColorConfig& GetColorConfig() const { return maColorConfig; }
    static bool     IsVisualCursorTravelingEnabled();
    static bool     DoVisualCursorTraveling();

    EditSelection         ConvertSelection( sal_Int32 nStartPara, sal_Int32 nStartPos, sal_Int32 nEndPara, sal_Int32 nEndPos );

    EPaM CreateEPaM( const EditPaM& rPaM ) const
    {
        const ContentNode* pNode = rPaM.GetNode();
        return EPaM(maEditDoc.GetPos(pNode), rPaM.GetIndex());
    }

    EditPaM CreateEditPaM( const EPaM& rEPaM )
    {
        DBG_ASSERT( rEPaM.nPara < maEditDoc.Count(), "CreateEditPaM: invalid paragraph" );
        DBG_ASSERT(maEditDoc.GetObject(rEPaM.nPara)->Len() >= rEPaM.nIndex, "CreateEditPaM: invalid Index");
        return EditPaM(maEditDoc.GetObject(rEPaM.nPara), rEPaM.nIndex);
    }

    ESelection CreateESel(const EditSelection& rSel) const
    {
        const ContentNode* pStartNode = rSel.Min().GetNode();
        const ContentNode* pEndNode = rSel.Max().GetNode();
        ESelection aESel;
        aESel.nStartPara = maEditDoc.GetPos( pStartNode );
        aESel.nStartPos = rSel.Min().GetIndex();
        aESel.nEndPara = maEditDoc.GetPos( pEndNode );
        aESel.nEndPos = rSel.Max().GetIndex();
        return aESel;
    }

    EditSelection CreateSel(const ESelection& rSel)
    {
        DBG_ASSERT( rSel.nStartPara < maEditDoc.Count(), "CreateSel: invalid start paragraph" );
        DBG_ASSERT( rSel.nEndPara < maEditDoc.Count(), "CreateSel: invalid end paragraph" );
        EditSelection aSel;
        aSel.Min().SetNode(maEditDoc.GetObject(rSel.nStartPara));
        aSel.Min().SetIndex( rSel.nStartPos );
        aSel.Max().SetNode(maEditDoc.GetObject(rSel.nEndPara));
        aSel.Max().SetIndex( rSel.nEndPos );
        DBG_ASSERT( !aSel.DbgIsBuggy( maEditDoc ), "CreateSel: incorrect selection!" );
        return aSel;

    }

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool() const { return mpStylePool; }

    void                SetStyleSheet( EditSelection aSel, SfxStyleSheet* pStyle );
    void                SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle );
    const SfxStyleSheet* GetStyleSheet( sal_Int32 nPara ) const;
    SfxStyleSheet*      GetStyleSheet( sal_Int32 nPara );

    void                UpdateParagraphsWithStyleSheet( SfxStyleSheet* pStyle );
    void                RemoveStyleFromParagraphs( SfxStyleSheet const * pStyle );

    bool isUsedByModel() const override { return true; }

    OutputDevice*       GetRefDevice() const { return mpRefDev.get(); }
    void                SetRefDevice(OutputDevice* mpRefDef);

    const MapMode&      GetRefMapMode() const { return mpRefDev->GetMapMode(); }
    void                SetRefMapMode(const MapMode& rMapMode);

    InternalEditStatus& GetStatus() { return maStatus; }
    InternalEditStatus const& GetStatus() const{ return maStatus; }

    void                CallStatusHdl();
    void                DelayedCallStatusHdl()  { maStatusTimer.Start(); }

    void                UndoActionStart( sal_uInt16 nId );
    void                UndoActionStart( sal_uInt16 nId, const ESelection& rSel );
    void                UndoActionEnd();

    EditView*           GetActiveView() const   { return mpActiveView; }
    void                SetActiveView( EditView* pView );

    css::uno::Reference< css::linguistic2::XSpellChecker1 > const &
                        GetSpeller();
    void                SetSpeller( css::uno::Reference< css::linguistic2::XSpellChecker1 > const &xSpl )
                            { mxSpeller = xSpl; }
    const css::uno::Reference< css::linguistic2::XHyphenator >&
                        GetHyphenator() const { return mxHyphenator; }
    void                SetHyphenator( css::uno::Reference< css::linguistic2::XHyphenator > const &xHyph )
                            { mxHyphenator = xHyph; }

    void GetAllMisspellRanges( std::vector<editeng::MisspellRanges>& rRanges ) const;
    void SetAllMisspellRanges( const std::vector<editeng::MisspellRanges>& rRanges );

    SpellInfo*          GetSpellInfo() const { return mpSpellInfo.get(); }

    void SetDefaultLanguage(LanguageType eLang) { meDefLanguage = eLang; }
    LanguageType GetDefaultLanguage() const { return meDefLanguage; }

    editeng::LanguageSpan GetLanguage( const EditPaM& rPaM, sal_Int32* pEndPos = nullptr ) const;
    css::lang::Locale   GetLocale( const EditPaM& rPaM ) const;

    void DoOnlineSpelling( ContentNode* pThisNodeOnly = nullptr, bool bSpellAtCursorPos = false, bool bInterruptible = true );
    EESpellState        Spell(EditView* pEditView, weld::Widget* pDialogParent, bool bMultipleDoc);
    EESpellState        HasSpellErrors();
    void                ClearSpellErrors();
    EESpellState        StartThesaurus(EditView* pEditView, weld::Widget* pDialogParent);
    css::uno::Reference< css::linguistic2::XSpellAlternatives >
                        ImpSpell( EditView* pEditView );

    // text conversion functions
    void                Convert(EditView* pEditView, weld::Widget* pDialogParent, LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont, sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc);
    void                ImpConvert( OUString &rConvTxt, LanguageType &rConvTxtLang, EditView* pEditView, LanguageType nSrcLang, const ESelection &rConvRange,
                                    bool bAllowImplicitChangesForNotConvertibleText, LanguageType nTargetLang, const vcl::Font *pTargetFont );
    ConvInfo *          GetConvInfo() const { return mpConvInfo.get(); }
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
    bool                SpellSentence(EditView const & rView, svx::SpellPortions& rToFill );
    //put spelling back to start of current sentence - needed after switch of grammar support
    void                PutSpellingToSentenceStart( EditView const & rEditView );
    //applies a changed sentence
    void                ApplyChangedSentence(EditView const & rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck );
    //adds one or more portions of text to the SpellPortions depending on language changes
    void                AddPortionIterated(
                          EditView const & rEditView,
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

    void                    SetEditTextObjectPool( SfxItemPool* pP )    { mpTextObjectPool = pP; }
    SfxItemPool*            GetEditTextObjectPool() const               { return mpTextObjectPool; }

    const SvxNumberFormat * GetNumberFormat( const ContentNode* pNode ) const;
    sal_Int32               GetSpaceBeforeAndMinLabelWidth( const ContentNode *pNode, sal_Int32 *pnSpaceBefore = nullptr, sal_Int32 *pnMinLabelWidth = nullptr ) const;

    const SvxLRSpaceItem&   GetLRSpaceItem( ContentNode* pNode );
    SvxAdjust               GetJustification( sal_Int32 nPara ) const;
    SvxCellJustifyMethod    GetJustifyMethod( sal_Int32 nPara ) const;
    SvxCellVerJustify       GetVerJustification( sal_Int32 nPara ) const;

    void setScalingParameters(ScalingParameters const& rScalingParameters);

    void resetScalingParameters()
    {
        setScalingParameters(ScalingParameters());
    }

    ScalingParameters getScalingParameters()
    {
        return maScalingParameters;
    }

    sal_Int32 GetBigTextObjectStart() const { return mnBigTextObjectStart; }

    EditEngine*  GetEditEnginePtr() const    { return mpEditEngine; }

    void                StartOnlineSpellTimer()     { maOnlineSpellTimer.Start(); }
    void                StopOnlineSpellTimer()      { maOnlineSpellTimer.Stop(); }

    const OUString& GetAutoCompleteText() const { return maAutoCompleteText; }
    void                SetAutoCompleteText(const OUString& rStr, bool bUpdateTipWindow);

    EditSelection       TransliterateText( const EditSelection& rSelection, TransliterationFlags nTransliterationMode );
    short               ReplaceTextOnly( ContentNode* pNode, sal_Int32 nCurrentStart, std::u16string_view rText, const css::uno::Sequence< sal_Int32 >& rOffsets );

    void                SetAsianCompressionMode( CharCompressType n );
    CharCompressType    GetAsianCompressionMode() const { return mnAsianCompressionMode; }

    void                SetKernAsianPunctuation( bool b );
    bool                IsKernAsianPunctuation() const { return mbKernAsianPunctuation; }

    sal_Int32 GetOverflowingParaNum() const { return mnOverflowingPara; }
    sal_Int32 GetOverflowingLineNum() const { return mnOverflowingLine; }
    void ClearOverflowingParaNum() { mnOverflowingPara = -1; }


    void                SetAddExtLeading( bool b );
    bool IsAddExtLeading() const { return mbAddExtLeading; }

    static std::shared_ptr<SvxForbiddenCharactersTable> const & GetForbiddenCharsTable();
    static void         SetForbiddenCharsTable( const std::shared_ptr<SvxForbiddenCharactersTable>& xForbiddenChars );

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void                SetBeginDropHdl( const Link<EditView*,void>& rLink ) { maBeginDropHdl = rLink; }
    const Link<EditView*,void>&  GetBeginDropHdl() const { return maBeginDropHdl; }

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link<EditView*,void>& rLink ) { maEndDropHdl = rLink; }
    const Link<EditView*,void>&  GetEndDropHdl() const { return maEndDropHdl; }

    /// specifies if auto-correction should capitalize the first word or not (default is on)
    void SetFirstWordCapitalization( bool bCapitalize ) { mbFirstWordCapitalization = bCapitalize; }
    bool IsFirstWordCapitalization() const { return mbFirstWordCapitalization; }

    /** specifies if auto-correction should replace a leading single quotation
        mark (apostrophe) or not (default is on) */
    void            SetReplaceLeadingSingleQuotationMark( bool bReplace ) { mbReplaceLeadingSingleQuotationMark = bReplace; }
    bool            IsReplaceLeadingSingleQuotationMark() const { return mbReplaceLeadingSingleQuotationMark; }

    /** Whether last AutoCorrect inserted a NO-BREAK SPACE that may need to be removed again. */
    bool            IsNbspRunNext() const { return mbNbspRunNext; }

    void EnableSkipOutsideFormat(bool set) { mbSkipOutsideFormat = set; }

    void Dispose();
    void SetLOKSpecialPaperSize(const Size& rSize) { maLOKSpecialPaperSize = rSize; }
    const Size& GetLOKSpecialPaperSize() const { return maLOKSpecialPaperSize; }

    enum class CallbackResult
    {
        Continue,
        SkipThisPortion, // Do not call callback until next portion
        Stop, // Stop iteration
    };
    struct LineAreaInfo
    {
        ParaPortion& rPortion; // Current ParaPortion
        EditLine* pLine; // Current line, or nullptr for paragraph start
        tools::Long nHeightNeededToNotWrap;
        tools::Rectangle aArea; // The area for the line (or for rPortion's first line offset)
                                // Bottom coordinate *does not* belong to the area
        sal_Int32 nPortion;
        sal_Int32 nLine;
        sal_Int16 nColumn; // Column number; when overflowing, equal to total number of columns
    };
    using IterateLinesAreasFunc = std::function<CallbackResult(const LineAreaInfo&)>;
    enum IterFlag // bitmask
    {
        none = 0,
        inclILS = 1, // rArea includes interline space
    };

    void IterateLineAreas(const IterateLinesAreasFunc& f, IterFlag eOptions);

    tools::Long GetColumnWidth(const Size& rPaperSize) const;
    Point MoveToNextLine(Point& rMovePos, tools::Long nLineHeight, sal_Int16& nColumn,
                         Point aOrigin, tools::Long* pnHeightNeededToNotWrap = nullptr) const;

    tools::Long getWidthDirectionAware(const Size& sz) const;
    tools::Long getHeightDirectionAware(const Size& sz) const;
    void adjustXDirectionAware(Point& pt, tools::Long x) const;
    void adjustYDirectionAware(Point& pt, tools::Long y) const;
    void setXDirectionAwareFrom(Point& ptDest, const Point& ptSrc) const;
    void setYDirectionAwareFrom(Point& ptDest, const Point& ptSrc) const;
    tools::Long getYOverflowDirectionAware(const Point& pt, const tools::Rectangle& rectMax) const;
    bool isXOverflowDirectionAware(const Point& pt, const tools::Rectangle& rectMax) const;
    // Offset of the rectangle's direction-aware corners in document coordinates
    tools::Long getBottomDocOffset(const tools::Rectangle& rect) const;
    Size getTopLeftDocOffset(const tools::Rectangle& rect) const;
};

void ConvertItem( std::unique_ptr<SfxPoolItem>& rPoolItem, MapUnit eSourceUnit, MapUnit eDestUnit );
void ConvertAndPutItems( SfxItemSet& rDest, const SfxItemSet& rSource, const MapUnit* pSourceUnit = nullptr, const MapUnit* pDestUnit = nullptr );
AsianCompressionFlags GetCharTypeForCompression( sal_Unicode cChar );


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
