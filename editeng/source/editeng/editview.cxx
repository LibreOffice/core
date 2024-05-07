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


#include <memory>
#include <vcl/image.hxx>

#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>

#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/langtab.hxx>
#include <tools/stream.hxx>

#include <svl/srchitem.hxx>

#include "impedit.hxx"
#include <comphelper/propertyvalue.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/eerdll.hxx>
#include <eerdll2.hxx>
#include <editeng/editrids.hrc>
#include <editeng.hxx>
#include <i18nlangtag/lang.h>
#include <vcl/window.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/lingucfg.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <linguistic/lngprops.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/lokhelper.hxx>
#include <osl/diagnose.h>
#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


// static
LanguageType EditView::CheckLanguage(
        const OUString &rText,
        const Reference< linguistic2::XSpellChecker1 >& xSpell,
        const Reference< linguistic2::XLanguageGuessing >& xLangGuess,
        bool bIsParaText )
{
    LanguageType nLang = LANGUAGE_NONE;
    if (bIsParaText)    // check longer texts with language-guessing...
    {
        if (!xLangGuess.is())
            return nLang;

        LanguageTag aGuessTag( xLangGuess->guessPrimaryLanguage( rText, 0, rText.getLength()) );

        // If the result from language guessing does not provide a 'Country'
        // part, try to get it by looking up the locale setting of the office,
        // "Tools/Options - Languages and Locales - General: Locale setting", if
        // the language matches.
        if ( aGuessTag.getCountry().isEmpty() )
        {
            const LanguageTag& rAppLocaleTag = Application::GetSettings().GetLanguageTag();
            if (rAppLocaleTag.getLanguage() == aGuessTag.getLanguage())
                nLang = rAppLocaleTag.getLanguageType();
        }
        if (nLang == LANGUAGE_NONE) // language not found by looking up the system language...
            nLang = aGuessTag.makeFallback().getLanguageType();     // best known locale match
        if (nLang == LANGUAGE_SYSTEM)
            nLang = Application::GetSettings().GetLanguageTag().getLanguageType();
        if (nLang == LANGUAGE_DONTKNOW)
            nLang = LANGUAGE_NONE;
    }
    else    // check single word
    {
        if (!xSpell.is())
            return nLang;


        // build list of languages to check

        LanguageType aLangList[4];
        const AllSettings& rSettings  = Application::GetSettings();
        SvtLinguOptions aLinguOpt;
        SvtLinguConfig().GetOptions( aLinguOpt );
        // The default document language from "Tools/Options - Languages and Locales - General: Western"
        aLangList[0] = MsLangId::resolveSystemLanguageByScriptType( aLinguOpt.nDefaultLanguage,
                css::i18n::ScriptType::LATIN);
        // The one from "Tools/Options - Languages and Locales - General: User interface"
        aLangList[1] = rSettings.GetUILanguageTag().getLanguageType();
        // The one from "Tools/Options - Languages and Locales - General: Locale setting"
        aLangList[2] = rSettings.GetLanguageTag().getLanguageType();
        // en-US
        aLangList[3] = LANGUAGE_ENGLISH_US;
#if OSL_DEBUG_LEVEL >= 2
        lang::Locale a0( LanguageTag::convertToLocale( aLangList[0] ) );
        lang::Locale a1( LanguageTag::convertToLocale( aLangList[1] ) );
        lang::Locale a2( LanguageTag::convertToLocale( aLangList[2] ) );
        lang::Locale a3( LanguageTag::convertToLocale( aLangList[3] ) );
#endif

        for (const LanguageType& nTmpLang : aLangList)
        {
            if (nTmpLang != LANGUAGE_NONE  &&  nTmpLang != LANGUAGE_DONTKNOW)
            {
                if (xSpell->hasLanguage( static_cast<sal_uInt16>(nTmpLang) ) &&
                    xSpell->isValid( rText, static_cast<sal_uInt16>(nTmpLang), Sequence< PropertyValue >() ))
                {
                    nLang = nTmpLang;
                    break;
                }
            }
        }
    }

    return nLang;
}

EditViewCallbacks::~EditViewCallbacks()
{
}

EditView::EditView(EditEngine* pEditEngine, vcl::Window* pWindow)
    : mpImpEditView(new ImpEditView(this, pEditEngine, pWindow))
{
    assert(pEditEngine);
    getImpl().mbReadOnly = getImpl().mbReadOnly || SfxViewShell::IsCurrentLokViewReadOnly();
}

EditView::~EditView()
{
}

void EditView::setEditViewCallbacks(EditViewCallbacks* pEditViewCallbacks)
{
    getImpl().setEditViewCallbacks(pEditViewCallbacks);
}

EditViewCallbacks* EditView::getEditViewCallbacks() const
{
    return getImpl().getEditViewCallbacks();
}

ImpEditEngine& EditView::getImpEditEngine() const
{
    return getImpl().getImpEditEngine();
}

EditEngine& EditView::getEditEngine() const
{
    return getImpl().getEditEngine();
}

tools::Rectangle EditView::GetInvalidateRect() const
{
    if (!getImpl().DoInvalidateMore())
        return getImpl().maOutputArea;
    else
    {
        tools::Rectangle aRect(getImpl().maOutputArea);
        tools::Long nMore = getImpl().GetOutputDevice().PixelToLogic( Size( getImpl().GetInvalidateMore(), 0 ) ).Width();
        aRect.AdjustLeft( -nMore );
        aRect.AdjustRight(nMore );
        aRect.AdjustTop( -nMore );
        aRect.AdjustBottom(nMore );
        return aRect;
    }
}

namespace {

tools::Rectangle lcl_negateRectX(const tools::Rectangle& rRect)
{
    return tools::Rectangle(-rRect.Right(), rRect.Top(), -rRect.Left(), rRect.Bottom());
}

}

void EditView::InvalidateWindow(const tools::Rectangle& rClipRect)
{
    LOKEditViewHistory::Update();
    bool bNegativeX = IsNegativeX();
    if (EditViewCallbacks* pEditViewCallbacks = getImpl().getEditViewCallbacks())
    {
        // do not invalidate and trigger a global repaint, but forward
        // the need for change to the applied EditViewCallback, can e.g.
        // be used to visualize the active edit text in an OverlayObject
        pEditViewCallbacks->EditViewInvalidate(bNegativeX ? lcl_negateRectX(rClipRect) : rClipRect);
    }
    else
    {
        // classic mode: invalidate and trigger full repaint
        // of the changed area
        GetWindow()->Invalidate(bNegativeX ? lcl_negateRectX(rClipRect) : rClipRect);
    }
}

void EditView::InvalidateOtherViewWindows( const tools::Rectangle& rInvRect )
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        bool bNegativeX = IsNegativeX();
        for (auto& pWin : getImpl().maOutWindowSet)
        {
            if (pWin)
            {
                if (!pWin->InvalidateByForeignEditView(this))
                    pWin->Invalidate( bNegativeX ? lcl_negateRectX(rInvRect) : rInvRect );
            }
        }
    }
}

void EditView::Invalidate()
{
    const tools::Rectangle& rInvRect = GetInvalidateRect();

    LOKEditViewHistory::Update();
    getImpl().InvalidateAtWindow(rInvRect);
    InvalidateOtherViewWindows(rInvRect);
}

void EditView::SetReadOnly( bool bReadOnly )
{
    getImpl().mbReadOnly = bReadOnly || SfxViewShell::IsCurrentLokViewReadOnly();
}

bool EditView::IsReadOnly() const
{
    return getImpl().mbReadOnly;
}

void EditView::SetSelection( const ESelection& rESel )
{
    // If someone has just left an empty attribute, and then the outliner manipulates the
    // selection, call the CursorMoved method so that empty attributes get cleaned up.
    if ( !HasSelection() )
    {
        // tdf#113591 Get node from EditDoc, as the selection might have a pointer to an
        // already deleted node.
        const ContentNode* pNode(getEditEngine().GetEditDoc().GetEndPaM().GetNode());
        if (nullptr != pNode)
            pNode->checkAndDeleteEmptyAttribs();
    }
    EditSelection aNewSelection(getImpEditEngine().ConvertSelection(rESel.nStartPara, rESel.nStartPos, rESel.nEndPara, rESel.nEndPos));

    // If the selection is manipulated after a KeyInput:
    getEditEngine().CheckIdleFormatter();

    // Selection may not start/end at an invisible paragraph:
    const ParaPortion* pPortion = getEditEngine().FindParaPortion( aNewSelection.Min().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = getEditEngine().GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : getEditEngine().GetEditDoc().GetObject( 0 );
        aNewSelection.Min() = EditPaM( pNode, pNode->Len() );
    }
    pPortion = getEditEngine().FindParaPortion( aNewSelection.Max().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = getEditEngine().GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : getEditEngine().GetEditDoc().GetObject( 0 );
        aNewSelection.Max() = EditPaM( pNode, pNode->Len() );
    }

    getImpl().DrawSelectionXOR();
    getImpl().SetEditSelection( aNewSelection );
    getImpl().DrawSelectionXOR();
    bool bGotoCursor = getImpl().DoAutoScroll();

    // comments section in Writer:
    // don't scroll to the selection if it is
    // out of visible area of comment canvas.
    if (HasSelection())
        ShowCursor( bGotoCursor );
}

ESelection EditView::GetSelection() const
{
    ESelection aSelection;

    aSelection.nStartPara = getEditEngine().GetEditDoc().GetPos( getImpl().GetEditSelection().Min().GetNode() );
    aSelection.nEndPara = getEditEngine().GetEditDoc().GetPos( getImpl().GetEditSelection().Max().GetNode() );

    aSelection.nStartPos = getImpl().GetEditSelection().Min().GetIndex();
    aSelection.nEndPos = getImpl().GetEditSelection().Max().GetIndex();

    return aSelection;
}

bool EditView::HasSelection() const
{
    return getImpl().HasSelection();
}

bool EditView::IsSelectionFullPara() const
{
    return getImpl().IsSelectionFullPara();
}

bool EditView::IsSelectionWithinSinglePara() const
{
    return getImpl().IsSelectionInSinglePara();
}

bool EditView::IsSelectionAtPoint(const Point& rPointPixel)
{
    return getImpl().IsSelectionAtPoint(rPointPixel);
}

void EditView::DeleteSelected()
{
    getImpl().DeleteSelected();
}

SvtScriptType EditView::GetSelectedScriptType() const
{
    return getEditEngine().GetScriptType( getImpl().GetEditSelection() );
}

void EditView::GetSelectionRectangles(std::vector<tools::Rectangle>& rLogicRects) const
{
    return getImpl().GetSelectionRectangles(getImpl().GetEditSelection(), rLogicRects);
}

void EditView::Paint( const tools::Rectangle& rRect, OutputDevice* pTargetDevice )
{
    getImpEditEngine().Paint(&getImpl(), rRect, pTargetDevice);
}

void EditView::setEditEngine(EditEngine* pEditEngine)
{
    assert(pEditEngine);

    getImpl().mpEditEngine = pEditEngine;
    EditSelection aStartSel = getEditEngine().GetEditDoc().GetStartPaM();
    getImpl().SetEditSelection( aStartSel );
}

void EditView::SetWindow(vcl::Window* pWindow)
{
    getImpl().mpOutputWindow = pWindow;
    getImpEditEngine().GetSelEngine().Reset();
}

vcl::Window* EditView::GetWindow() const
{
    return getImpl().mpOutputWindow;
}

OutputDevice& EditView::GetOutputDevice() const
{
    return getImpl().GetOutputDevice();
}

LanguageType EditView::GetInputLanguage() const
{
    // it might make sense to add this to getEditViewCallbacks
    if (const vcl::Window* pWindow = GetWindow())
        return pWindow->GetInputLanguage();
    return LANGUAGE_DONTKNOW;
}

bool EditView::HasOtherViewWindow( vcl::Window* pWin )
{
    OutWindowSet& rOutWindowSet = getImpl().maOutWindowSet;
    auto found = std::find(rOutWindowSet.begin(), rOutWindowSet.end(), pWin);
    return (found != rOutWindowSet.end());
}

bool EditView::AddOtherViewWindow( vcl::Window* pWin )
{
    if (HasOtherViewWindow(pWin))
        return false;
    getImpl().maOutWindowSet.emplace_back(pWin);
    return true;
}

bool EditView::RemoveOtherViewWindow( vcl::Window* pWin )
{
    OutWindowSet& rOutWindowSet = getImpl().maOutWindowSet;
    auto found = std::find(rOutWindowSet.begin(), rOutWindowSet.end(), pWin);
    if (found == rOutWindowSet.end())
        return false;
    rOutWindowSet.erase(found);
    return true;
}

void EditView::SetVisArea( const tools::Rectangle& rRect )
{
    getImpl().SetVisDocStartPos( rRect.TopLeft() );
}

tools::Rectangle EditView::GetVisArea() const
{
    return getImpl().GetVisDocArea();
}

void EditView::SetOutputArea( const tools::Rectangle& rRect )
{
    getImpl().SetOutputArea( rRect );

    // the rest here only if it is an API call:
    getImpl().CalcAnchorPoint();
    if (getImpEditEngine().GetStatus().AutoPageSize() )
        getImpl().RecalcOutputArea();
    getImpl().ShowCursor( false, false );
}

const tools::Rectangle& EditView::GetOutputArea() const
{
    return getImpl().GetOutputArea();
}

PointerStyle EditView::GetPointer() const
{
    return getImpl().GetPointer();
}

vcl::Cursor* EditView::GetCursor() const
{
    return getImpl().GetCursor();
}

void EditView::InsertText( const OUString& rStr, bool bSelect, bool bLOKShowSelect )
{

    EditEngine& rEditEngine = getEditEngine();

    if (bLOKShowSelect)
        getImpl().DrawSelectionXOR();

    EditPaM aPaM1;
    if ( bSelect )
    {
        EditSelection aTmpSel( getImpl().GetEditSelection() );
        aTmpSel.Adjust(rEditEngine.GetEditDoc());
        aPaM1 = aTmpSel.Min();
    }

    rEditEngine.UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM2(rEditEngine.InsertText( getImpl().GetEditSelection(), rStr ) );
    rEditEngine.UndoActionEnd();

    if ( bSelect )
    {
        DBG_ASSERT( !aPaM1.DbgIsBuggy(rEditEngine.GetEditDoc()), "Insert: PaM broken" );
        getImpl().SetEditSelection( EditSelection( aPaM1, aPaM2 ) );
    }
    else
        getImpl().SetEditSelection( EditSelection( aPaM2, aPaM2 ) );

    if (bLOKShowSelect)
        rEditEngine.FormatAndLayout( this );
}

bool EditView::PostKeyEvent( const KeyEvent& rKeyEvent, vcl::Window const * pFrameWin )
{
    return getImpl().PostKeyEvent( rKeyEvent, pFrameWin );
}

bool EditView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    return getImpl().MouseButtonUp( rMouseEvent );
}

void EditView::ReleaseMouse()
{
    return getImpl().ReleaseMouse();
}

bool EditView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    return getImpl().MouseButtonDown( rMouseEvent );
}

bool EditView::MouseMove( const MouseEvent& rMouseEvent )
{
    return getImpl().MouseMove( rMouseEvent );
}

bool EditView::Command(const CommandEvent& rCEvt)
{
    return getImpl().Command(rCEvt);
}

void EditView::SetBroadcastLOKViewCursor(bool bSet)
{
    getImpl().SetBroadcastLOKViewCursor(bSet);
}

tools::Rectangle EditView::GetEditCursor() const
{
    return getImpl().GetEditCursor();
}

void EditView::ShowCursor( bool bGotoCursor, bool bForceVisCursor, bool bActivate )
{
    if (!getEditEngine().HasView(this))
        return;

    // The control word is more important:
    if ( !getImpl().DoAutoScroll() )
        bGotoCursor = false;
    getImpl().ShowCursor( bGotoCursor, bForceVisCursor );

    if (getImpl().mpViewShell && !bActivate)
    {
        if (!getImpl().mpOutputWindow)
            return;
        VclPtr<vcl::Window> pParent = getImpl().mpOutputWindow->GetParentWithLOKNotifier();
        if (pParent && pParent->GetLOKWindowId() != 0)
            return;

        static const OString aPayload = OString::boolean(true);
        getImpl().mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CURSOR_VISIBLE, aPayload);
        getImpl().mpViewShell->NotifyOtherViews(LOK_CALLBACK_VIEW_CURSOR_VISIBLE, "visible"_ostr, aPayload);
    }
}

void EditView::HideCursor(bool bDeactivate)
{
    getImpl().GetCursor()->Hide();

    if (getImpl().mpViewShell && !bDeactivate)
    {
        if (!getImpl().mpOutputWindow)
            return;
        VclPtr<vcl::Window> pParent = getImpl().mpOutputWindow->GetParentWithLOKNotifier();
        if (pParent && pParent->GetLOKWindowId() != 0)
            return;

        OString aPayload = OString::boolean(false);
        getImpl().mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CURSOR_VISIBLE, aPayload);
        getImpl().mpViewShell->NotifyOtherViews(LOK_CALLBACK_VIEW_CURSOR_VISIBLE, "visible"_ostr, aPayload);
    }
}

Pair EditView::Scroll( tools::Long ndX, tools::Long ndY, ScrollRangeCheck nRangeCheck )
{
    return getImpl().Scroll( ndX, ndY, nRangeCheck );
}

const SfxItemSet& EditView::GetEmptyItemSet() const
{
    return getEditEngine().GetEmptyItemSet();
}

void EditView::SetAttribs( const SfxItemSet& rSet )
{
    DBG_ASSERT(!getImpl().maEditSelection.IsInvalid(), "Blind Selection in...");

    getImpl().DrawSelectionXOR();
    getEditEngine().SetAttribs( getImpl().GetEditSelection(), rSet, SetAttribsMode::WholeWord );
    if (getEditEngine().IsUpdateLayout())
        getEditEngine().FormatAndLayout( this );
}

void EditView::RemoveAttribsKeepLanguages( bool bRemoveParaAttribs )
{

    getImpl().DrawSelectionXOR();
    getEditEngine().UndoActionStart( EDITUNDO_RESETATTRIBS );
    EditSelection aSelection( getImpl().GetEditSelection() );

    for (sal_uInt16 nWID = EE_ITEMS_START; nWID <= EE_ITEMS_END; ++nWID)
    {
        bool bIsLang =  EE_CHAR_LANGUAGE     == nWID ||
                        EE_CHAR_LANGUAGE_CJK == nWID ||
                        EE_CHAR_LANGUAGE_CTL == nWID;
        if (!bIsLang)
            getEditEngine().RemoveCharAttribs( aSelection, bRemoveParaAttribs, nWID );
    }

    getEditEngine().UndoActionEnd();
    if (getEditEngine().IsUpdateLayout())
        getEditEngine().FormatAndLayout( this );
}

void EditView::RemoveAttribs( bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    RemoveAttribs(bRemoveParaAttribs ? EERemoveParaAttribsMode::RemoveAll
            : EERemoveParaAttribsMode::RemoveCharItems, nWhich);
}

void EditView::RemoveAttribs( EERemoveParaAttribsMode eMode, sal_uInt16 nWhich )
{
    getImpl().DrawSelectionXOR();
    getEditEngine().UndoActionStart( EDITUNDO_RESETATTRIBS );
    getEditEngine().RemoveCharAttribs( getImpl().GetEditSelection(), eMode, nWhich  );
    getEditEngine().UndoActionEnd();
    if (getEditEngine().IsUpdateLayout())
        getEditEngine().FormatAndLayout( this );
}

void EditView::RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich )
{
    getEditEngine().UndoActionStart( EDITUNDO_RESETATTRIBS );
    getEditEngine().RemoveCharAttribs( nPara, nWhich );
    getEditEngine().UndoActionEnd();
    if (getEditEngine().IsUpdateLayout())
        getEditEngine().FormatAndLayout( this );
}

SfxItemSet EditView::GetAttribs()
{
    DBG_ASSERT(!getImpl().maEditSelection.IsInvalid(), "Blind Selection in...");
    return getImpEditEngine().GetAttribs( getImpl().GetEditSelection() );
}

void EditView::Undo()
{
    getEditEngine().Undo( this );
}

void EditView::Redo()
{
    getEditEngine().Redo( this );
}

ErrCode EditView::Read( SvStream& rInput, EETextFormat eFormat, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    EditSelection aOldSel( getImpl().GetEditSelection() );
    getImpl().DrawSelectionXOR();
    getImpEditEngine().UndoActionStart( EDITUNDO_READ );
    EditPaM aEndPaM = getImpEditEngine().Read( rInput, u""_ustr, eFormat, aOldSel, pHTTPHeaderAttrs );
    getImpEditEngine().UndoActionEnd();
    EditSelection aNewSel( aEndPaM, aEndPaM );

    getImpl().SetEditSelection( aNewSel );
    bool bGotoCursor = getImpl().DoAutoScroll();
    ShowCursor( bGotoCursor );

    return rInput.GetError();
}

void EditView::Cut()
{
    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
    getImpl().CutCopy( aClipBoard, true );
}

Reference<css::datatransfer::clipboard::XClipboard> EditView::GetClipboard() const
{
    return getImpl().GetClipboard();
}

css::uno::Reference< css::datatransfer::XTransferable > EditView::GetTransferable() const
{
    uno::Reference< datatransfer::XTransferable > xData = getEditEngine().CreateTransferable( getImpl().GetEditSelection() );
    return xData;
}

void EditView::Copy()
{
    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
    getImpl().CutCopy( aClipBoard, false );
}

void EditView::Paste()
{
    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
    getImpl().Paste( aClipBoard );
}

void EditView::PasteSpecial(SotClipboardFormatId format)
{
    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
    getImpl().Paste(aClipBoard, true, format );
}

Point EditView::GetWindowPosTopLeft( sal_Int32 nParagraph )
{
    Point aDocPos(getEditEngine().GetDocPosTopLeft(nParagraph));
    return getImpl().GetWindowPos( aDocPos );
}

void EditView::SetSelectionMode( EESelectionMode eMode )
{
    getImpl().SetSelectionMode( eMode );
}

OUString EditView::GetSelected() const
{
    return getImpEditEngine().GetSelected( getImpl().GetEditSelection() );
}

void EditView::MoveParagraphs( Range aParagraphs, sal_Int32 nNewPos )
{
    getImpEditEngine().UndoActionStart( EDITUNDO_MOVEPARAS );
    getImpEditEngine().MoveParagraphs( aParagraphs, nNewPos, this );
    getImpEditEngine().UndoActionEnd();
}

void EditView::MoveParagraphs( tools::Long nDiff )
{
    ESelection aSel = GetSelection();
    Range aRange( aSel.nStartPara, aSel.nEndPara );
    aRange.Normalize();
    tools::Long nDest = ( nDiff > 0  ? aRange.Max() : aRange.Min() ) + nDiff;
    if ( nDiff > 0 )
        nDest++;
    DBG_ASSERT( ( nDest >= 0 ) && ( nDest <= getEditEngine().GetParagraphCount() ), "MoveParagraphs - wrong Parameters!" );
    MoveParagraphs( aRange, sal::static_int_cast< sal_Int32 >( nDest ) );
}

void EditView::SetBackgroundColor( const Color& rColor )
{
    getImpl().SetBackgroundColor( rColor );
    getEditEngine().SetBackgroundColor( rColor );
}

Color const & EditView::GetBackgroundColor() const
{
    return getImpl().GetBackgroundColor();
}

void EditView::RegisterViewShell(OutlinerViewShell* pViewShell)
{
    getImpl().RegisterViewShell(pViewShell);
}

void EditView::RegisterOtherShell(OutlinerViewShell* pOtherShell)
{
    getImpl().RegisterOtherShell(pOtherShell);
}

void EditView::SetControlWord( EVControlBits nWord )
{
    getImpl().mnControl = nWord;
}

EVControlBits EditView::GetControlWord() const
{
    return getImpl().mnControl;
}

std::unique_ptr<EditTextObject> EditView::CreateTextObject()
{
    return getImpEditEngine().CreateTextObject( getImpl().GetEditSelection() );
}

void EditView::InsertText( const EditTextObject& rTextObject )
{
    getImpl().DrawSelectionXOR();

    getEditEngine().UndoActionStart( EDITUNDO_INSERT );
    EditSelection aTextSel(getEditEngine().InsertText(rTextObject, getImpl().GetEditSelection()));
    getEditEngine().UndoActionEnd();

    aTextSel.Min() = aTextSel.Max();    // Selection not retained.
    getImpl().SetEditSelection( aTextSel );
    if (getEditEngine().IsUpdateLayout())
        getEditEngine().FormatAndLayout( this );
}

void EditView::InsertText( css::uno::Reference< css::datatransfer::XTransferable > const & xDataObj, const OUString& rBaseURL, bool bUseSpecial )
{
    getEditEngine().UndoActionStart( EDITUNDO_INSERT );
    getImpl().DeleteSelected();
    EditSelection aTextSel =
        getEditEngine().InsertText(xDataObj, rBaseURL, getImpl().GetEditSelection().Max(), bUseSpecial);
    getEditEngine().UndoActionEnd();

    aTextSel.Min() = aTextSel.Max();    // Selection not retained.
    getImpl().SetEditSelection( aTextSel );
    if (getEditEngine().IsUpdateLayout())
        getEditEngine().FormatAndLayout( this );
}

bool EditView::SetEditEngineUpdateLayout( bool bUpdate )
{
    return getImpEditEngine().SetUpdateLayout( bUpdate, this );
}

void EditView::ForceLayoutCalculation()
{
    getImpEditEngine().SetUpdateLayout( true, this, true );
}

SfxStyleSheet* EditView::GetStyleSheet()
{
    EditSelection aSel( getImpl().GetEditSelection() );
    aSel.Adjust(getEditEngine().GetEditDoc());
    sal_Int32 nStartPara = getEditEngine().GetEditDoc().GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndPara = getEditEngine().GetEditDoc().GetPos( aSel.Max().GetNode() );

    SfxStyleSheet* pStyle = nullptr;
    for ( sal_Int32 n = nStartPara; n <= nEndPara; n++ )
    {
        SfxStyleSheet* pTmpStyle = getEditEngine().GetStyleSheet( n );
        if ( ( n != nStartPara ) && ( pStyle != pTmpStyle ) )
            return nullptr;    // Not unique.
        pStyle = pTmpStyle;
    }
    return pStyle;
}

const SfxStyleSheet* EditView::GetStyleSheet() const
{
    return const_cast< EditView* >( this )->GetStyleSheet();
}

bool EditView::IsInsertMode() const
{
    return getImpl().IsInsertMode();
}

void EditView::SetInsertMode( bool bInsert )
{
    getImpl().SetInsertMode( bInsert );
}

void EditView::SetAnchorMode( EEAnchorMode eMode )
{
    getImpl().SetAnchorMode( eMode );
}

EEAnchorMode EditView::GetAnchorMode() const
{
    return getImpl().GetAnchorMode();
}

void EditView::TransliterateText( TransliterationFlags nTransliterationMode )
{
    EditSelection aOldSel( getImpl().GetEditSelection() );
    EditSelection aNewSel = getEditEngine().TransliterateText( getImpl().GetEditSelection(), nTransliterationMode );
    if ( aNewSel != aOldSel )
    {
        getImpl().DrawSelectionXOR();
        getImpl().SetEditSelection( aNewSel );
        getImpl().DrawSelectionXOR();
    }
}

void EditView::CompleteAutoCorrect( vcl::Window const * pFrameWin )
{
    if ( !HasSelection() && getImpEditEngine().GetStatus().DoAutoCorrect() )
    {
        getImpl().DrawSelectionXOR();
        EditSelection aSel = getImpl().GetEditSelection();
        aSel = getEditEngine().EndOfWord( aSel.Max() );
        aSel = getImpEditEngine().AutoCorrect( aSel, 0, !IsInsertMode(), pFrameWin );
        getImpl().SetEditSelection( aSel );
        if (getEditEngine().IsModified())
            getEditEngine().FormatAndLayout( this );
    }
}

EESpellState EditView::StartSpeller(weld::Widget* pDialogParent, bool bMultipleDoc)
{
    if (!getImpEditEngine().GetSpeller().is())
        return EESpellState::NoSpeller;

    return getImpEditEngine().Spell(this, pDialogParent, bMultipleDoc);
}

EESpellState EditView::StartThesaurus(weld::Widget* pDialogParent)
{
    if (!getImpEditEngine().GetSpeller().is())
        return EESpellState::NoSpeller;

    return getImpEditEngine().StartThesaurus(this, pDialogParent);
}

void EditView::StartTextConversion(weld::Widget* pDialogParent,
        LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont,
        sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc )
{
    getImpEditEngine().Convert(this, pDialogParent, nSrcLang, nDestLang, pDestFont, nOptions, bIsInteractive, bMultipleDoc);
}

sal_Int32 EditView::StartSearchAndReplace( const SvxSearchItem& rSearchItem )
{
    return getImpEditEngine().StartSearchAndReplace( this, rSearchItem );
}

bool EditView::IsCursorAtWrongSpelledWord()
{
    bool bIsWrong = false;
    if ( !HasSelection() )
    {
        EditPaM aPaM = getImpl().GetEditSelection().Max();
        bIsWrong = getImpl().IsWrongSpelledWord( aPaM, false/*bMarkIfWrong*/ );
    }
    return bIsWrong;
}

bool EditView::IsWrongSpelledWordAtPos( const Point& rPosPixel, bool bMarkIfWrong )
{
    Point aPos(getImpl().GetOutputDevice().PixelToLogic(rPosPixel));
    aPos = getImpl().GetDocPos( aPos );
    EditPaM aPaM = getEditEngine().GetPaM(aPos, false);
    return getImpl().IsWrongSpelledWord( aPaM , bMarkIfWrong );
}

static void LOKSendSpellPopupMenu(const weld::Menu& rMenu, LanguageType nGuessLangWord,
                                  LanguageType nGuessLangPara, sal_uInt16 nSuggestions)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    // Generate the menu structure and send it to the client code.
    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (!pViewShell)
        return;

    boost::property_tree::ptree aMenu;

    boost::property_tree::ptree aItemTree;
    if (nSuggestions)
    {
        for(int i = 0; i < nSuggestions; ++i)
        {
            OUString sItemId = OUString::number(MN_ALTSTART + i);
            OUString sText = rMenu.get_label(sItemId);
            aItemTree.put("text", sText.toUtf8().getStr());
            aItemTree.put("type", "command");
            OUString sCommandString = ".uno:SpellCheckApplySuggestion?ApplyRule:string=Spelling_" + sText;
            aItemTree.put("command", sCommandString.toUtf8().getStr());
            aItemTree.put("enabled", rMenu.get_sensitive(sItemId));
            aMenu.push_back(std::make_pair("", aItemTree));
            aItemTree.clear();
        }

        aItemTree.put("type", "separator");
        aMenu.push_back(std::make_pair("", aItemTree));
        aItemTree.clear();
    }

    // First we need to set item commands for the context menu.
    OUString aTmpWord( SvtLanguageTable::GetLanguageString( nGuessLangWord ) );
    OUString aTmpPara( SvtLanguageTable::GetLanguageString( nGuessLangPara ) );

    aItemTree.put("text", rMenu.get_label(u"ignore"_ustr).toUtf8().getStr());
    aItemTree.put("type", "command");
    aItemTree.put("command", ".uno:SpellCheckIgnoreAll?Type:string=Spelling");
    aItemTree.put("enabled", rMenu.get_sensitive(u"ignore"_ustr));
    aMenu.push_back(std::make_pair("", aItemTree));
    aItemTree.clear();

    aItemTree.put("type", "separator");
    aMenu.push_back(std::make_pair("", aItemTree));
    aItemTree.clear();

    aItemTree.put("text", rMenu.get_label(u"wordlanguage"_ustr).toUtf8().getStr());
    aItemTree.put("type", "command");
    OUString sCommandString = ".uno:LanguageStatus?Language:string=Current_" + aTmpWord;
    aItemTree.put("command", sCommandString.toUtf8().getStr());
    aItemTree.put("enabled", rMenu.get_sensitive(u"wordlanguage"_ustr));
    aMenu.push_back(std::make_pair("", aItemTree));
    aItemTree.clear();

    aItemTree.put("text", rMenu.get_label(u"paralanguage"_ustr).toUtf8().getStr());
    aItemTree.put("type", "command");
    sCommandString = ".uno:LanguageStatus?Language:string=Paragraph_" + aTmpPara;
    aItemTree.put("command", sCommandString.toUtf8().getStr());
    aItemTree.put("enabled", rMenu.get_sensitive(u"paralanguage"_ustr));
    aMenu.push_back(std::make_pair("", aItemTree));
    aItemTree.clear();

    boost::property_tree::ptree aRoot;
    aRoot.add_child("menu", aMenu);

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aRoot, true);
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CONTEXT_MENU, OString(aStream.str()));
}

bool EditView::ExecuteSpellPopup(const Point& rPosPixel, const Link<SpellCallbackInfo&,void> &rCallBack)
{
    OutputDevice& rDevice = getImpl().GetOutputDevice();
    Point aPos(rDevice.PixelToLogic(rPosPixel));
    aPos = getImpl().GetDocPos( aPos );
    EditPaM aPaM = getEditEngine().GetPaM(aPos, false);
    Reference< linguistic2::XSpellChecker1 >  xSpeller(getImpEditEngine().GetSpeller());
    ESelection aOldSel = GetSelection();
    if ( !(xSpeller.is() && getImpl().IsWrongSpelledWord( aPaM, true )) )
        return false;

    // PaMtoEditCursor returns Logical units
    tools::Rectangle aTempRect = getImpEditEngine().PaMtoEditCursor(aPaM, CursorFlags{ .bTextOnly = true });
    // GetWindowPos works in Logical units
    aTempRect = getImpl().GetWindowPos(aTempRect);
    // Convert to pixels
    aTempRect = rDevice.LogicToPixel(aTempRect);

    weld::Widget* pPopupParent = getImpl().GetPopupParent(aTempRect);
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pPopupParent, u"editeng/ui/spellmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xPopupMenu(xBuilder->weld_menu(u"editviewspellmenu"_ustr));
    std::unique_ptr<weld::Menu> xInsertMenu(xBuilder->weld_menu(u"insertmenu"_ustr)); // add word to user-dictionaries
    std::unique_ptr<weld::Menu> xAutoMenu(xBuilder->weld_menu(u"automenu"_ustr));

    EditPaM aPaM2( aPaM );
    aPaM2.SetIndex( aPaM2.GetIndex()+1 );

    // Are there any replace suggestions?
    OUString aSelected( GetSelected() );

    // restrict the maximal number of suggestions displayed
    // in the context menu.
    // Note: That could of course be done by clipping the
    // resulting sequence but the current third party
    // implementations result differs greatly if the number of
    // suggestions to be returned gets changed. Statistically
    // it gets much better if told to return e.g. only 7 strings
    // than returning e.g. 16 suggestions and using only the
    // first 7. Thus we hand down the value to use to that
    // implementation here by providing an additional parameter.
    Sequence< PropertyValue > aPropVals { comphelper::makePropertyValue(UPN_MAX_NUMBER_OF_SUGGESTIONS, sal_Int16(7)) };

    // Are there any replace suggestions?
    Reference< linguistic2::XSpellAlternatives >  xSpellAlt =
            xSpeller->spell( aSelected, static_cast<sal_uInt16>(getImpEditEngine().GetLanguage( aPaM2 ).nLang), aPropVals );

    Reference< linguistic2::XLanguageGuessing >  xLangGuesser( EditDLL::Get().GetGlobalData()->GetLanguageGuesser() );

    // check if text might belong to a different language...
    LanguageType nGuessLangWord = LANGUAGE_NONE;
    LanguageType nGuessLangPara = LANGUAGE_NONE;
    if (xSpellAlt.is() && xLangGuesser.is())
    {
        OUString aParaText;
        ContentNode *pNode = aPaM.GetNode();
        if (pNode)
        {
            aParaText = pNode->GetString();
        }
        else
        {
            OSL_FAIL( "content node is NULL" );
        }

        nGuessLangWord = CheckLanguage( xSpellAlt->getWord(), xSpeller, xLangGuesser, false );
        nGuessLangPara = CheckLanguage( aParaText, xSpeller, xLangGuesser, true );
    }
    if (nGuessLangWord != LANGUAGE_NONE || nGuessLangPara != LANGUAGE_NONE)
    {
        // make sure LANGUAGE_NONE gets not used as menu entry
        if (nGuessLangWord == LANGUAGE_NONE)
            nGuessLangWord = nGuessLangPara;
        if (nGuessLangPara == LANGUAGE_NONE)
            nGuessLangPara = nGuessLangWord;

        xPopupMenu->append_separator(u"separator1"_ustr);
        OUString aTmpWord( SvtLanguageTable::GetLanguageString( nGuessLangWord ) );
        OUString aTmpPara( SvtLanguageTable::GetLanguageString( nGuessLangPara ) );
        OUString aWordStr( EditResId( RID_STR_WORD ) );
        aWordStr = aWordStr.replaceFirst( "%x", aTmpWord );
        OUString aParaStr( EditResId( RID_STR_PARAGRAPH ) );
        aParaStr = aParaStr.replaceFirst( "%x", aTmpPara );
        xPopupMenu->append(u"wordlanguage"_ustr, aWordStr);
        xPopupMenu->append(u"paralanguage"_ustr, aParaStr);
    }

    // Replace suggestions...
    Sequence< OUString > aAlt;
    if (xSpellAlt.is())
        aAlt = xSpellAlt->getAlternatives();
    const OUString *pAlt = aAlt.getConstArray();
    sal_uInt16 nWords = static_cast<sal_uInt16>(aAlt.getLength());
    if ( nWords )
    {
        for ( sal_uInt16 nW = 0; nW < nWords; nW++ )
        {
            OUString aAlternate( pAlt[nW] );
            OUString sId(OUString::number(MN_ALTSTART + nW));
            xPopupMenu->insert(nW, sId, aAlternate, nullptr, nullptr, nullptr, TRISTATE_INDET);
            xAutoMenu->append(sId, aAlternate);
        }
        xPopupMenu->insert_separator(nWords, u"separator2"_ustr);
    }
    else
    {
        xAutoMenu.reset();
        xPopupMenu->remove(u"autocorrect"_ustr);
    }

    SvtLinguConfig aCfg;

    Reference< linguistic2::XSearchableDictionaryList >  xDicList( LinguMgr::GetDictionaryList() );
    Sequence< Reference< linguistic2::XDictionary >  > aDics;
    if (xDicList.is())
    {
        const Reference< linguistic2::XDictionary >  *pDic = nullptr;
        // add the default positive dictionary to dic-list (if not already done).
        // This is to ensure that there is at least one dictionary to which
        // words could be added.
        uno::Reference< linguistic2::XDictionary >  xDic( LinguMgr::GetStandardDic() );
        if (xDic.is())
            xDic->setActive( true );

        aDics = xDicList->getDictionaries();
        pDic  = aDics.getConstArray();
        LanguageType nCheckedLanguage = getImpEditEngine().GetLanguage( aPaM2 ).nLang;
        sal_uInt16 nDicCount = static_cast<sal_uInt16>(aDics.getLength());
        for (sal_uInt16 i = 0; i < nDicCount; i++)
        {
            uno::Reference< linguistic2::XDictionary >  xDicTmp = pDic[i];
            if (!xDicTmp.is() || LinguMgr::GetIgnoreAllList() == xDicTmp)
                continue;

            uno::Reference< frame::XStorable > xStor( xDicTmp, uno::UNO_QUERY );
            LanguageType nActLanguage = LanguageTag( xDicTmp->getLocale() ).getLanguageType();
            if( xDicTmp->isActive()
                &&  xDicTmp->getDictionaryType() != linguistic2::DictionaryType_NEGATIVE
                && (nCheckedLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage )
                && (!xStor.is() || !xStor->isReadonly()) )
            {
                OUString sImage;

                uno::Reference< lang::XServiceInfo > xSvcInfo( xDicTmp, uno::UNO_QUERY );
                if (xSvcInfo.is())
                {
                    OUString aDictionaryImageUrl( aCfg.GetSpellAndGrammarContextDictionaryImage(
                            xSvcInfo->getImplementationName()) );
                    if (!aDictionaryImageUrl.isEmpty() )
                        sImage = aDictionaryImageUrl;
                }

                if (sImage.isEmpty())
                {
                    xInsertMenu->append(OUString::number(MN_DICTSTART + i), xDicTmp->getName());
                }
                else
                {
                    Image aImage(sImage);
                    ScopedVclPtr<VirtualDevice> xVirDev(pPopupParent->create_virtual_device());
                    Size aSize(aImage.GetSizePixel());
                    xVirDev->SetOutputSizePixel(aSize);
                    xVirDev->DrawImage(Point(0, 0), aImage);
                    xInsertMenu->append(OUString::number(MN_DICTSTART + i), xDicTmp->getName(), *xVirDev);
                }
                aDicNameSingle = xDicTmp->getName();
            }
        }
    }

    if (xInsertMenu->n_children() != 1)
        xPopupMenu->remove(u"add"_ustr);
    if (xInsertMenu->n_children() < 2)
    {
        xInsertMenu.reset();
        xPopupMenu->remove(u"insert"_ustr);
    }

    //tdf#106123 store and restore the EditPaM around the menu Execute
    //because the loss of focus in the current editeng causes writer
    //annotations to save their contents, making the pContent of the
    //current EditPams invalid
    EPaM aP = getImpEditEngine().CreateEPaM(aPaM);
    EPaM aP2 = getImpEditEngine().CreateEPaM(aPaM2);

    if (comphelper::LibreOfficeKit::isActive())
    {
        xPopupMenu->remove(u"autocorrect"_ustr);
        xPopupMenu->remove(u"autocorrectdlg"_ustr);

        LOKSendSpellPopupMenu(*xPopupMenu, nGuessLangWord, nGuessLangPara, nWords);
        return true;
    }

    OUString sId = xPopupMenu->popup_at_rect(pPopupParent, aTempRect);

    aPaM2 = getImpEditEngine().CreateEditPaM(aP2);
    aPaM = getImpEditEngine().CreateEditPaM(aP);

    if (sId == "ignore")
    {
        OUString aWord = getImpl().SpellIgnoreWord();
        SpellCallbackInfo aInf( SpellCallbackCommand::IGNOREWORD, aWord );
        rCallBack.Call(aInf);
        SetSelection( aOldSel );
    }
    else if (sId == "wordlanguage" || sId == "paralanguage")
    {
        LanguageType nLangToUse = (sId == "wordlanguage") ? nGuessLangWord : nGuessLangPara;
        SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nLangToUse );

        SfxItemSet aAttrs = getEditEngine().GetEmptyItemSet();
        if (nScriptType == SvtScriptType::LATIN)
            aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE ) );
        if (nScriptType == SvtScriptType::COMPLEX)
            aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CTL ) );
        if (nScriptType == SvtScriptType::ASIAN)
            aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CJK ) );
        if (sId == "paralanguage")
        {
            ESelection aSel = GetSelection();
            aSel.nStartPos = 0;
            aSel.nEndPos = EE_TEXTPOS_ALL;
            SetSelection( aSel );
        }
        SetAttribs( aAttrs );
        getImpEditEngine().StartOnlineSpellTimer();

        SpellCallbackInfo aInf((sId == "wordlanguage") ? SpellCallbackCommand::WORDLANGUAGE : SpellCallbackCommand::PARALANGUAGE);
        rCallBack.Call(aInf);
        SetSelection( aOldSel );
    }
    else if (sId == "check")
    {
        SpellCallbackInfo aInf( SpellCallbackCommand::STARTSPELLDLG, OUString() );
        rCallBack.Call(aInf);
    }
    else if (sId == "autocorrectdlg")
    {
        SpellCallbackInfo aInf( SpellCallbackCommand::AUTOCORRECT_OPTIONS, OUString() );
        rCallBack.Call(aInf);
    }
    else if ( sId.toInt32() >= MN_DICTSTART || sId == "add")
    {
        OUString aDicName;
        if (sId.toInt32() >= MN_DICTSTART)
        {
            assert(xInsertMenu && "this case only occurs when xInsertMenu exists");
            // strip_mnemonic is necessary to retrieve the correct dictionary name
            aDicName = pPopupParent->strip_mnemonic(xInsertMenu->get_label(sId));
        }
        else
            aDicName = aDicNameSingle;

        uno::Reference< linguistic2::XDictionary >      xDic;
        if (xDicList.is())
            xDic = xDicList->getDictionaryByName( aDicName );

        if (xDic.is())
            xDic->add( aSelected, false, OUString() );
        // save modified user-dictionary if it is persistent
        Reference< frame::XStorable >  xSavDic( xDic, UNO_QUERY );
        if (xSavDic.is())
            xSavDic->store();

        aPaM.GetNode()->GetWrongList()->ResetInvalidRange(0, aPaM.GetNode()->Len());
        getImpEditEngine().StartOnlineSpellTimer();

        SpellCallbackInfo aInf( SpellCallbackCommand::ADDTODICTIONARY, aSelected );
        rCallBack.Call(aInf);
        SetSelection( aOldSel );
    }
    else if ( sId.toInt32() >= MN_AUTOSTART )
    {
        DBG_ASSERT(sId.toInt32() - MN_AUTOSTART < aAlt.getLength(), "index out of range");
        OUString aWord = pAlt[sId.toInt32() - MN_AUTOSTART];
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        if ( pAutoCorrect )
            pAutoCorrect->PutText( aSelected, aWord, getImpEditEngine().GetLanguage( aPaM2 ).nLang );
        InsertText( aWord );
    }
    else if ( sId.toInt32() >= MN_ALTSTART )  // Replace
    {
        DBG_ASSERT(sId.toInt32() - MN_ALTSTART < aAlt.getLength(), "index out of range");
        OUString aWord = pAlt[sId.toInt32() - MN_ALTSTART];
        InsertText( aWord );
    }
    else
    {
        SetSelection( aOldSel );
    }
    return true;
}

OUString EditView::SpellIgnoreWord()
{
    return getImpl().SpellIgnoreWord();
}

void EditView::SelectCurrentWord( sal_Int16 nWordType )
{
    EditSelection aCurSel( getImpl().GetEditSelection() );
    getImpl().DrawSelectionXOR();
    aCurSel = getEditEngine().SelectWord(aCurSel.Max(), nWordType);
    getImpl().SetEditSelection( aCurSel );
    getImpl().DrawSelectionXOR();
    ShowCursor( true, false );
}

void EditView::InsertParaBreak()
{
    getEditEngine().UndoActionStart(EDITUNDO_INSERT);
    getImpl().DeleteSelected();
    EditPaM aPaM(getEditEngine().InsertParaBreak(getImpl().GetEditSelection()));
    getEditEngine().UndoActionEnd();
    getImpl().SetEditSelection(EditSelection(aPaM, aPaM));
    if (getEditEngine().IsUpdateLayout())
        getEditEngine().FormatAndLayout(this);
}

void EditView::InsertField( const SvxFieldItem& rFld )
{
    EditEngine& rEditEngine = getImpl().getEditEngine();
    getImpl().DrawSelectionXOR();
    rEditEngine.UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM(rEditEngine.InsertField(getImpl().GetEditSelection(), rFld));
    rEditEngine.UndoActionEnd();
    getImpl().SetEditSelection( EditSelection( aPaM, aPaM ) );
    rEditEngine.UpdateFields();
    if (rEditEngine.IsUpdateLayout())
        rEditEngine.FormatAndLayout( this );
}

const SvxFieldItem* EditView::GetFieldUnderMousePointer() const
{
    sal_Int32 nPara;
    sal_Int32 nPos;
    return GetFieldUnderMousePointer( nPara, nPos );
}

const SvxFieldItem* EditView::GetField( const Point& rPos, sal_Int32* pPara, sal_Int32* pPos ) const
{
    return getImpl().GetField( rPos, pPara, pPos );
}

const SvxFieldItem* EditView::GetFieldUnderMousePointer( sal_Int32& nPara, sal_Int32& nPos ) const
{
    Point aPos;
    if (EditViewCallbacks* pEditViewCallbacks = getImpl().getEditViewCallbacks())
        aPos = pEditViewCallbacks->EditViewPointerPosPixel();
    else
        aPos = getImpl().GetWindow()->GetPointerPosPixel();
    OutputDevice& rDevice = getImpl().GetOutputDevice();
    aPos = rDevice.PixelToLogic(aPos);
    return GetField( aPos, &nPara, &nPos );
}

const SvxFieldItem* EditView::GetFieldAtSelection(bool bAlsoCheckBeforeCursor) const
{
    bool* pIsBeforeCursor = bAlsoCheckBeforeCursor ? &bAlsoCheckBeforeCursor : nullptr;
    return GetFieldAtSelection(pIsBeforeCursor);
}

// If pIsBeforeCursor != nullptr, the position before the cursor will also be checked for a field
// and pIsBeforeCursor will return true if that fallback field is returned.
// If no field is returned, the value in pIsBeforeCursor is meaningless.
const SvxFieldItem* EditView::GetFieldAtSelection(bool* pIsBeforeCursor) const
{
    // a field is a dummy character - so it cannot span nodes or be a selection larger than 1
    EditSelection aSel( getImpl().GetEditSelection() );
    if (aSel.Min().GetNode() != aSel.Max().GetNode())
        return nullptr;

    // normalize: min < max
    aSel.Adjust(getEditEngine().GetEditDoc());

    const sal_Int32 nMinIndex =  aSel.Min().GetIndex();
    const sal_Int32 nMaxIndex =  aSel.Max().GetIndex();
    if (nMaxIndex > nMinIndex + 1)
        return nullptr;

    // Only when cursor is in font of field, no selection,
    // or only selecting field
    bool bAlsoCheckBeforeCursor = false;
    if (pIsBeforeCursor)
    {
        *pIsBeforeCursor = false;
        bAlsoCheckBeforeCursor = nMaxIndex == nMinIndex;
    }
    const SvxFieldItem* pFoundBeforeCursor = nullptr;
    const CharAttribList::AttribsType& rAttrs = aSel.Min().GetNode()->GetCharAttribs().GetAttribs();
    for (const auto& rAttr: rAttrs)
    {
        if (rAttr->Which() == EE_FEATURE_FIELD)
        {
            DBG_ASSERT(dynamic_cast<const SvxFieldItem*>(rAttr->GetItem()), "No FieldItem...");
            if (rAttr->GetStart() == nMinIndex)
                return static_cast<const SvxFieldItem*>(rAttr->GetItem());

            // perhaps the cursor is behind the field?
            if (nMinIndex && rAttr->GetStart() == nMinIndex - 1)
                pFoundBeforeCursor = static_cast<const SvxFieldItem*>(rAttr->GetItem());
        }
    }
    if (bAlsoCheckBeforeCursor)
    {
        *pIsBeforeCursor = /*(bool)*/pFoundBeforeCursor;
        return pFoundBeforeCursor;
    }
    return nullptr;
}

void EditView::SelectFieldAtCursor()
{
    bool bIsBeforeCursor = false;
    const SvxFieldItem* pFieldItem = GetFieldAtSelection(&bIsBeforeCursor);
    if (!pFieldItem)
        return;

    // Make sure the whole field is selected
    // A field is represented by a dummy character - so it cannot be a selection larger than 1
    ESelection aSel = GetSelection();
    if (aSel.nStartPos == aSel.nEndPos) // not yet selected
    {
        if (bIsBeforeCursor)
        {
            assert (aSel.nStartPos);
            --aSel.nStartPos;
        }
        else
            aSel.nEndPos++;
        SetSelection(aSel);
    }
    else
        assert(std::abs(aSel.nStartPos - aSel.nEndPos) == 1);
}

const SvxFieldData* EditView::GetFieldUnderMouseOrInSelectionOrAtCursor(bool bAlsoCheckBeforeCursor) const
{
    const SvxFieldItem* pFieldItem = GetFieldUnderMousePointer();
    if (!pFieldItem)
        pFieldItem = GetFieldAtSelection(bAlsoCheckBeforeCursor);

    return pFieldItem ? pFieldItem->GetField() : nullptr;
}

sal_Int32 EditView::countFieldsOffsetSum(sal_Int32 nPara, sal_Int32 nPos, bool bCanOverflow) const
{
    int nOffset = 0;

    for (int nCurrentPara = 0; nCurrentPara <= nPara; nCurrentPara++)
    {
        int nFields = getEditEngine().GetFieldCount( nCurrentPara );
        for (int nField = 0; nField < nFields; nField++)
        {
            EFieldInfo aFieldInfo = getEditEngine().GetFieldInfo( nCurrentPara, nField );

            bool bLastPara = nCurrentPara == nPara;
            sal_Int32 nFieldPos = aFieldInfo.aPosition.nIndex;

            if (bLastPara && nFieldPos >= nPos)
                break;

            sal_Int32 nFieldLen = aFieldInfo.aCurrentText.getLength();

            // position in the middle of a field
            if (!bCanOverflow && bLastPara && nFieldPos + nFieldLen > nPos)
                nFieldLen = nPos - nFieldPos;

            nOffset += nFieldLen - 1;
        }
    }

    return nOffset;
}

sal_Int32 EditView::GetPosNoField(sal_Int32 nPara, sal_Int32 nPos) const
{
    sal_Int32 nOffset = countFieldsOffsetSum(nPara, nPos, false);
    assert(nPos >= nOffset);
    return nPos - nOffset;
}

sal_Int32 EditView::GetPosWithField(sal_Int32 nPara, sal_Int32 nPos) const
{
    sal_Int32 nOffset = countFieldsOffsetSum(nPara, nPos, true);
    return nPos + nOffset;
}

void EditView::SetInvalidateMore( sal_uInt16 nPixel )
{
    getImpl().SetInvalidateMore( nPixel );
}

sal_uInt16 EditView::GetInvalidateMore() const
{
    return getImpl().GetInvalidateMore();
}

static void ChangeFontSizeImpl( EditView* pEditView, bool bGrow, const ESelection& rSel, const FontList* pFontList )
{
    pEditView->SetSelection( rSel );

    SfxItemSet aSet( pEditView->GetAttribs() );
    if( EditView::ChangeFontSize( bGrow, aSet, pFontList ) )
    {
        SfxItemSet aNewSet( pEditView->GetEmptyItemSet() );
        aNewSet.Put( aSet.Get( EE_CHAR_FONTHEIGHT ) );
        aNewSet.Put( aSet.Get( EE_CHAR_FONTHEIGHT_CJK ) );
        aNewSet.Put( aSet.Get( EE_CHAR_FONTHEIGHT_CTL ) );
        pEditView->SetAttribs( aNewSet );
    }
}

void EditView::ChangeFontSize( bool bGrow, const FontList* pFontList )
{

    EditEngine& rEditEngine = getEditEngine();

    ESelection aSel( GetSelection() );
    ESelection aOldSelection( aSel );
    aSel.Adjust();

    if( !aSel.HasRange() )
    {
        aSel = rEditEngine.GetWord( aSel, css::i18n::WordType::DICTIONARY_WORD );
    }

    if( aSel.HasRange() )
    {
        for( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
        {
            std::vector<sal_Int32> aPortions;
            rEditEngine.GetPortions( nPara, aPortions );

            if( aPortions.empty() )
                aPortions.push_back( rEditEngine.GetTextLen(nPara) );

            const sal_Int32 nBeginPos = (nPara == aSel.nStartPara) ? aSel.nStartPos : 0;
            const sal_Int32 nEndPos = (nPara == aSel.nEndPara) ? aSel.nEndPos : EE_TEXTPOS_ALL;

            for ( size_t nPos = 0; nPos < aPortions.size(); ++nPos )
            {
                sal_Int32 nPortionEnd   = aPortions[ nPos ];
                sal_Int32 nPortionStart = nPos > 0 ? aPortions[ nPos - 1 ] : 0;

                if( (nPortionEnd < nBeginPos) || (nPortionStart > nEndPos) )
                    continue;

                if( nPortionStart < nBeginPos )
                    nPortionStart = nBeginPos;
                if( nPortionEnd > nEndPos )
                    nPortionEnd = nEndPos;

                if( nPortionStart == nPortionEnd )
                    continue;

                ESelection aPortionSel( nPara, nPortionStart, nPara, nPortionEnd );
                ChangeFontSizeImpl( this, bGrow, aPortionSel, pFontList );
            }
        }
    }
    else
    {
        ChangeFontSizeImpl( this, bGrow, aSel, pFontList );
    }

    SetSelection( aOldSelection );
}

bool EditView::ChangeFontSize( bool bGrow, SfxItemSet& rSet, const FontList* pFontList )
{
    if (!pFontList)
        return false;

    static const sal_uInt16 gFontSizeWichMap[] = { EE_CHAR_FONTHEIGHT, EE_CHAR_FONTHEIGHT_CJK, EE_CHAR_FONTHEIGHT_CTL, 0 };
    bool bRet = false;

    const sal_uInt16* pWhich = gFontSizeWichMap;
    while( *pWhich )
    {
        SvxFontHeightItem aFontHeightItem( static_cast<const SvxFontHeightItem&>(rSet.Get( *pWhich )) );
        tools::Long nHeight = aFontHeightItem.GetHeight();
        const MapUnit eUnit = rSet.GetPool()->GetMetric( *pWhich );
        nHeight = OutputDevice::LogicToLogic(nHeight * 10, eUnit, MapUnit::MapPoint);

        const int* pAry = FontList::GetStdSizeAry();

        if( bGrow )
        {
            while( *pAry )
            {
                if( *pAry > nHeight )
                {
                    nHeight = *pAry;
                    break;
                }
                pAry++;
            }

            if( *pAry == 0 )
            {
                nHeight += (nHeight + 5) / 10;
                if( nHeight > 9999 )
                    nHeight = 9999;
            }

        }
        else if( *pAry )
        {
            bool bFound = false;
            if( *pAry < nHeight )
            {
                pAry++;
                while( *pAry )
                {
                    if( *pAry >= nHeight )
                    {
                        nHeight = pAry[-1];
                        bFound = true;
                        break;
                    }
                    pAry++;
                }
            }

            if( !bFound )
            {
                nHeight -= (nHeight + 5) / 10;
                if( nHeight < 2 )
                    nHeight = 2;
            }
        }

        if( (nHeight >= 2) && (nHeight <= 9999 ) )
        {
            nHeight = OutputDevice::LogicToLogic( nHeight, MapUnit::MapPoint, eUnit  ) / 10;

            if( nHeight != static_cast<tools::Long>(aFontHeightItem.GetHeight()) )
            {
                aFontHeightItem.SetHeight( nHeight );
                rSet.Put( aFontHeightItem.CloneSetWhich(*pWhich) );
                bRet = true;
            }
        }
        pWhich++;
    }
    return bRet;
}

OUString EditView::GetSurroundingText() const
{
    EditSelection aSel( getImpl().GetEditSelection() );
    aSel.Adjust(getEditEngine().GetEditDoc());

    if( HasSelection() )
    {
        OUString aStr = getEditEngine().GetSelected(aSel);

        // Stop reconversion if the selected text includes a line break.
        if ( aStr.indexOf( 0x0A ) == -1 )
          return aStr;
        else
          return OUString();
    }
    else
    {
        aSel.Min().SetIndex( 0 );
        aSel.Max().SetIndex( aSel.Max().GetNode()->Len() );
        return getEditEngine().GetSelected(aSel);
    }
}

Selection EditView::GetSurroundingTextSelection() const
{
    ESelection aSelection( GetSelection() );
    aSelection.Adjust();

    if( HasSelection() )
    {
        EditSelection aSel( getImpl().GetEditSelection() );
        aSel.Adjust(getEditEngine().GetEditDoc());
        OUString aStr = getEditEngine().GetSelected(aSel);

        // Stop reconversion if the selected text includes a line break.
        if ( aStr.indexOf( 0x0A ) == -1 )
            return Selection( 0, aSelection.nEndPos - aSelection.nStartPos );
        else
            return Selection( 0, 0 );
    }
    else
    {
        return Selection( aSelection.nStartPos, aSelection.nEndPos );
    }
}

bool EditView::DeleteSurroundingText(const Selection& rRange)
{
    ESelection aSel(GetSelection());
    aSel.nEndPara = aSel.nStartPara;
    aSel.nStartPos = rRange.Min();
    aSel.nEndPos = rRange.Max();
    SetSelection(aSel);
    DeleteSelected();
    return true;
}

void EditView::SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark)
{
    Point aDocPos(getImpl().GetDocPos(rPosition));
    EditPaM aPaM = getEditEngine().GetPaM(aDocPos);
    EditSelection aSelection(getImpl().GetEditSelection());

    // Explicitly create or delete the selection.
    if (bClearMark)
    {
        getImpl().DeselectAll();
        aSelection = getImpl().GetEditSelection();
    }
    else
        getImpl().CreateAnchor();

    if (bPoint)
        aSelection.Max() = aPaM;
    else
        aSelection.Min() = aPaM;

    if (getImpl().GetEditSelection().Min() != aSelection.Min())
    {
        const ContentNode* pNode(getImpl().GetEditSelection().Min().GetNode());
        if (nullptr != pNode)
            pNode->checkAndDeleteEmptyAttribs();
    }

    getImpl().DrawSelectionXOR(aSelection);
    if (getImpl().GetEditSelection() != aSelection)
        getImpl().SetEditSelection(aSelection);
    ShowCursor(/*bGotoCursor=*/false);
}

void EditView::DrawSelectionXOR(OutlinerViewShell* pOtherShell)
{
    getImpl().RegisterOtherShell(pOtherShell);
    getImpl().DrawSelectionXOR();
    getImpl().RegisterOtherShell(nullptr);
}

void EditView::InitLOKSpecialPositioning(MapUnit eUnit,
                                         const tools::Rectangle& rOutputArea,
                                         const Point& rVisDocStartPos)
{
    getImpl().InitLOKSpecialPositioning(eUnit, rOutputArea, rVisDocStartPos);
}

void EditView::SetLOKSpecialOutputArea(const tools::Rectangle& rOutputArea)
{
    getImpl().SetLOKSpecialOutputArea(rOutputArea);
}

const tools::Rectangle & EditView::GetLOKSpecialOutputArea() const
{
    return getImpl().GetLOKSpecialOutputArea();
}

void EditView::SetLOKSpecialVisArea(const tools::Rectangle& rVisArea)
{
    getImpl().SetLOKSpecialVisArea(rVisArea);
}

tools::Rectangle EditView::GetLOKSpecialVisArea() const
{
    return getImpl().GetLOKSpecialVisArea();
}

bool EditView::HasLOKSpecialPositioning() const
{
    return getImpl().HasLOKSpecialPositioning();
}

void EditView::SetLOKSpecialFlags(LOKSpecialFlags eFlags)
{
    getImpl().SetLOKSpecialFlags(eFlags);
}

void EditView::SuppressLOKMessages(bool bSet)
{
    getImpl().SuppressLOKMessages(bSet);
}

bool EditView::IsSuppressLOKMessages() const
{
    return getImpl().IsSuppressLOKMessages();
}

void EditView::SetNegativeX(bool bSet)
{
    getImpl().SetNegativeX(bSet);
}

bool EditView::IsNegativeX() const
{
    return getImpl().IsNegativeX();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
