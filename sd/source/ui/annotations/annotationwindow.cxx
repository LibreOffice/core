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

#include <editeng/eeitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/editund2.hxx>
#include <officecfg/Office/Common.hxx>
#include <svx/svxids.hrc>
#include <unotools/useroptions.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>

#include <vcl/commandevent.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/svapp.hxx>
#include <vcl/gradient.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>

#include <strings.hrc>
#include "annotationwindow.hxx"
#include "annotationmanagerimpl.hxx"

#include <com/sun/star/office/XAnnotation.hpp>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <svx/annotation/TextAPI.hxx>
#include <sdresid.hxx>

#include <memory>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::text;

#define METABUTTON_WIDTH        16
#define METABUTTON_HEIGHT       18
#define POSTIT_META_HEIGHT  sal_Int32(30)

namespace sd {

void AnnotationTextWindow::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect)
{
    Size aSize = GetOutputSizePixel();

    const bool bHighContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
    if (!bHighContrast)
    {
        rRenderContext.DrawGradient(::tools::Rectangle(Point(0,0), rRenderContext.PixelToLogic(aSize)),
                                    Gradient(css::awt::GradientStyle_LINEAR, mrContents.maColorLight, mrContents.maColor));
    }

    DoPaint(rRenderContext, rRect);
}

void AnnotationTextWindow::EditViewScrollStateChange()
{
    mrContents.SetScrollbar();
}

bool AnnotationTextWindow::KeyInput(const KeyEvent& rKeyEvt)
{
    const vcl::KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    sal_uInt16 nKey = rKeyCode.GetCode();

    bool bDone = false;

    if ((rKeyCode.IsMod1() && rKeyCode.IsMod2()) && ((nKey == KEY_PAGEUP) || (nKey == KEY_PAGEDOWN)))
    {
        SfxDispatcher* pDispatcher = mrContents.DocShell()->GetViewShell()->GetViewFrame()->GetDispatcher();
        if( pDispatcher )
            pDispatcher->Execute( nKey == KEY_PAGEDOWN ? SID_NEXT_POSTIT : SID_PREVIOUS_POSTIT );
        bDone = true;
    }
    else if (nKey == KEY_INSERT)
    {
        if (!rKeyCode.IsMod1() && !rKeyCode.IsMod2())
            mrContents.ToggleInsMode();
        bDone = true;
    }
    else
    {
        ::tools::Long aOldHeight = mrContents.GetPostItTextHeight();

        /// HACK: need to switch off processing of Undo/Redo in Outliner
        if ( !( (nKey == KEY_Z || nKey == KEY_Y) && rKeyCode.IsMod1()) )
        {
            bool bIsProtected = mrContents.IsProtected();
            if (!bIsProtected || !EditEngine::DoesKeyChangeText(rKeyEvt) )
            {
                if (EditView* pEditView = GetEditView())
                {
                    bDone = pEditView->PostKeyEvent(rKeyEvt);
                    if (!bDone && rKeyEvt.GetKeyCode().IsMod1() && !rKeyEvt.GetKeyCode().IsMod2())
                    {
                        if (nKey == KEY_A)
                        {
                            EditEngine* pEditEngine = GetEditEngine();
                            sal_Int32 nPar = pEditEngine->GetParagraphCount();
                            if (nPar)
                            {
                                sal_Int32 nLen = pEditEngine->GetTextLen(nPar - 1);
                                pEditView->SetSelection(ESelection(0, 0, nPar - 1, nLen));
                            }
                            bDone = true;
                        }
                    }
                }
            }
        }
        if (bDone)
        {
            mrContents.ResizeIfNecessary(aOldHeight, mrContents.GetPostItTextHeight());
        }
    }

    return bDone;
}

AnnotationTextWindow::AnnotationTextWindow(AnnotationWindow& rContents)
    : mrContents(rContents)
{
}

EditView* AnnotationTextWindow::GetEditView() const
{
    OutlinerView* pOutlinerView = mrContents.GetOutlinerView();
    if (!pOutlinerView)
        return nullptr;
    return &pOutlinerView->GetEditView();
}

EditEngine* AnnotationTextWindow::GetEditEngine() const
{
    OutlinerView* pOutlinerView = mrContents.GetOutlinerView();
    if (!pOutlinerView)
        return nullptr;
    return &pOutlinerView->GetEditView().getEditEngine();
}

void AnnotationTextWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(0, 0);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

    SetOutputSizePixel(aSize);

    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);

    EnableRTL(false);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    OutputDevice& rDevice = pDrawingArea->get_ref_device();

    rDevice.SetMapMode(MapMode(MapUnit::Map100thMM));
    rDevice.SetBackground(aBgColor);

    Size aOutputSize(rDevice.PixelToLogic(aSize));

    EditView* pEditView = GetEditView();
    pEditView->setEditViewCallbacks(this);

    EditEngine* pEditEngine = GetEditEngine();
    pEditEngine->SetPaperSize(aOutputSize);
    pEditEngine->SetRefDevice(&rDevice);

    pEditView->SetOutputArea(::tools::Rectangle(Point(0, 0), aOutputSize));
    pEditView->SetBackgroundColor(aBgColor);

    pDrawingArea->set_cursor(PointerStyle::Text);

    InitAccessible();
}

// see SwAnnotationWin in sw for something similar
AnnotationWindow::AnnotationWindow(weld::Window* pParent, const ::tools::Rectangle& rRect,
                                   DrawDocShell* pDocShell,
                                   const Reference<XAnnotation>& xAnnotation)
    : mxBuilder(Application::CreateBuilder(pParent, u"modules/simpress/ui/annotation.ui"_ustr))
    , mxPopover(mxBuilder->weld_popover(u"Annotation"_ustr))
    , mxContainer(mxBuilder->weld_widget(u"container"_ustr))
    , mpDocShell(pDocShell)
    , mpDoc(pDocShell->GetDoc())
    , mbReadonly(pDocShell->IsReadOnly())
    , mbProtected(false)
{
    mxContainer->set_size_request(320, 240);
    mxPopover->popup_at_rect(pParent, rRect);

    InitControls();
    setAnnotation(xAnnotation);
    FillMenuButton();

    DoResize();

    mxTextControl->GrabFocus();
}

AnnotationWindow::~AnnotationWindow()
{
}

void AnnotationWindow::InitControls()
{
    // window control for author and date
    mxMeta = mxBuilder->weld_label(u"meta"_ustr);
    mxMeta->set_direction(AllSettings::GetLayoutRTL());

    maLabelFont = Application::GetSettings().GetStyleSettings().GetLabelFont();
    maLabelFont.SetFontHeight(8);

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    mxMeta->set_font(maLabelFont);

    mpOutliner.reset( new ::Outliner(GetAnnotationPool(),OutlinerMode::TextObject) );
    SdDrawDocument::SetCalcFieldValueHdl( mpOutliner.get() );
    mpOutliner->SetUpdateLayout( true );

    if (OutputDevice* pDev = mpDoc->GetRefDevice())
        mpOutliner->SetRefDevice( pDev );

    mpOutlinerView.reset( new OutlinerView ( mpOutliner.get(), nullptr) );
    mpOutliner->InsertView(mpOutlinerView.get() );

    //create Scrollbars
    mxVScrollbar = mxBuilder->weld_scrolled_window(u"scrolledwindow"_ustr, true);

    // actual window which holds the user text
    mxTextControl.reset(new AnnotationTextWindow(*this));
    mxTextControlWin.reset(new weld::CustomWeld(*mxBuilder, u"editview"_ustr, *mxTextControl));
    mxTextControl->SetPointer(PointerStyle::Text);

    Rescale();
    OutputDevice& rDevice = mxTextControl->GetDrawingArea()->get_ref_device();

    mxVScrollbar->set_direction(false);
    mxVScrollbar->connect_vadjustment_changed(LINK(this, AnnotationWindow, ScrollHdl));

    mpOutlinerView->SetBackgroundColor(COL_TRANSPARENT);
    mpOutlinerView->SetOutputArea(rDevice.PixelToLogic(::tools::Rectangle(0, 0, 1, 1)));

    mxMenuButton = mxBuilder->weld_menu_button(u"menubutton"_ustr);
    if (mbReadonly)
        mxMenuButton->hide();
    else
    {
        mxMenuButton->set_size_request(METABUTTON_WIDTH, METABUTTON_HEIGHT);
        mxMenuButton->connect_selected(LINK(this, AnnotationWindow, MenuItemSelectedHdl));
    }

    EEControlBits nCntrl = mpOutliner->GetControlWord();
    nCntrl |= EEControlBits::PASTESPECIAL | EEControlBits::AUTOCORRECT | EEControlBits::USECHARATTRIBS | EEControlBits::NOCOLORS;
    mpOutliner->SetControlWord(nCntrl);

    mpOutliner->SetModifyHdl( Link<LinkParamNone*,void>() );
    mpOutliner->EnableUndo( false );

    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();
    mpOutliner->EnableUndo( true );

    SetLanguage(SvxLanguageItem(mpDoc->GetLanguage(EE_CHAR_LANGUAGE), SID_ATTR_LANGUAGE));

    mxTextControl->GrabFocus();
}

IMPL_LINK(AnnotationWindow, MenuItemSelectedHdl, const OUString&, rIdent, void)
{
    SfxDispatcher* pDispatcher = mpDocShell->GetViewShell()->GetViewFrame()->GetDispatcher();
    if (!pDispatcher)
        return;

    if (rIdent == ".uno:ReplyToAnnotation")
    {
        const SfxUnoAnyItem aItem( SID_REPLYTO_POSTIT, Any( mxAnnotation ) );
        pDispatcher->ExecuteList(SID_REPLYTO_POSTIT,
                SfxCallMode::ASYNCHRON, { &aItem });
    }
    else if (rIdent == ".uno:DeleteAnnotation")
    {
        const SfxUnoAnyItem aItem( SID_DELETE_POSTIT, Any( mxAnnotation ) );
        pDispatcher->ExecuteList(SID_DELETE_POSTIT, SfxCallMode::ASYNCHRON,
                { &aItem });
    }
    else if (rIdent == ".uno:DeleteAllAnnotationByAuthor")
    {
        const SfxStringItem aItem( SID_DELETEALLBYAUTHOR_POSTIT, mxAnnotation->getAuthor() );
        pDispatcher->ExecuteList( SID_DELETEALLBYAUTHOR_POSTIT,
                SfxCallMode::ASYNCHRON, { &aItem });
    }
    else if (rIdent == ".uno:DeleteAllAnnotation")
        pDispatcher->Execute( SID_DELETEALL_POSTIT );
}

void AnnotationWindow::FillMenuButton()
{
    SvtUserOptions aUserOptions;
    OUString sCurrentAuthor( aUserOptions.GetFullName() );
    OUString sAuthor( mxAnnotation->getAuthor() );

    OUString aStr(mxMenuButton->get_item_label(u".uno:DeleteAllAnnotationByAuthor"_ustr));
    OUString aReplace( sAuthor );
    if( aReplace.isEmpty() )
        aReplace = SdResId( STR_ANNOTATION_NOAUTHOR );
    aStr = aStr.replaceFirst("%1", aReplace);
    mxMenuButton->set_item_label(u".uno:DeleteAllAnnotationByAuthor"_ustr, aStr);

    bool bShowReply = sAuthor != sCurrentAuthor && !mbReadonly;
    mxMenuButton->set_item_visible(u".uno:ReplyToAnnotation"_ustr, bShowReply);
    mxMenuButton->set_item_visible(u"separator"_ustr, bShowReply);
    mxMenuButton->set_item_visible(u".uno:DeleteAnnotation"_ustr, mxAnnotation.is() && !mbReadonly);
    mxMenuButton->set_item_visible(u".uno:DeleteAllAnnotationByAuthor"_ustr, !mbReadonly);
    mxMenuButton->set_item_visible(u".uno:DeleteAllAnnotation"_ustr, !mbReadonly);
}

void AnnotationWindow::StartEdit()
{
    GetOutlinerView()->SetSelection(ESelection(EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT,EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT));
    GetOutlinerView()->ShowCursor();
}

void AnnotationWindow::SetMapMode(const MapMode& rNewMapMode)
{
    OutputDevice& rDevice = mxTextControl->GetDrawingArea()->get_ref_device();
    rDevice.SetMapMode(rNewMapMode);
}

void AnnotationWindow::Rescale()
{
    MapMode aMode(MapUnit::Map100thMM);
    aMode.SetOrigin( Point() );
    mpOutliner->SetRefMapMode( aMode );
    SetMapMode( aMode );

    if (mxMeta)
    {
        vcl::Font aFont = maLabelFont;
        sal_Int32 nHeight = ::tools::Long(aFont.GetFontHeight() * aMode.GetScaleY());
        aFont.SetFontHeight( nHeight );
        mxMeta->set_font(aFont);
    }
}

void AnnotationWindow::DoResize()
{
    OutputDevice& rDevice = mxTextControl->GetDrawingArea()->get_ref_device();

    ::tools::Long aHeight = mxContainer->get_preferred_size().Height();
    ::tools::ULong aWidth = mxContainer->get_preferred_size().Width();

    aHeight -= POSTIT_META_HEIGHT;

    mpOutliner->SetPaperSize( rDevice.PixelToLogic( Size(aWidth, aHeight) ) ) ;
    ::tools::Long aTextHeight = rDevice.LogicToPixel(mpOutliner->CalcTextSize()).Height();

    if( aTextHeight > aHeight )
    {
        const int nThickness = mxVScrollbar->get_scroll_thickness();
        if (nThickness)
        {
            // we need vertical scrollbars and have to reduce the width
            aWidth -= nThickness;
            mpOutliner->SetPaperSize(rDevice.PixelToLogic(Size(aWidth, aHeight)));
        }
        mxVScrollbar->set_vpolicy(VclPolicyType::ALWAYS);
    }
    else
    {
        mxVScrollbar->set_vpolicy(VclPolicyType::NEVER);
    }

    ::tools::Rectangle aOutputArea = rDevice.PixelToLogic(::tools::Rectangle(0, 0, aWidth, aHeight));
    if (mxVScrollbar->get_vpolicy() == VclPolicyType::NEVER)
    {
        // if we do not have a scrollbar anymore, we want to see the complete text
        mpOutlinerView->SetVisArea(aOutputArea);
    }
    mpOutlinerView->SetOutputArea(aOutputArea);
    mpOutlinerView->ShowCursor(true, true);

    int nUpper = mpOutliner->GetTextHeight();
    int nCurrentDocPos = mpOutlinerView->GetVisArea().Top();
    int nStepIncrement = mpOutliner->GetTextHeight() / 10;
    int nPageIncrement = rDevice.PixelToLogic(Size(0,aHeight)).Height() * 8 / 10;
    int nPageSize = rDevice.PixelToLogic(Size(0,aHeight)).Height();

    /* limit the page size to below nUpper because gtk's gtk_scrolled_window_start_deceleration has
       effectively...

       lower = gtk_adjustment_get_lower
       upper = gtk_adjustment_get_upper - gtk_adjustment_get_page_size

       and requires that upper > lower or the deceleration animation never ends
    */
    nPageSize = std::min(nPageSize, nUpper);

    mxVScrollbar->vadjustment_configure(nCurrentDocPos, 0, nUpper,
                                        nStepIncrement, nPageIncrement, nPageSize);
}

void AnnotationWindow::SetScrollbar()
{
    mxVScrollbar->vadjustment_set_value(mpOutlinerView->GetVisArea().Top());
}

void AnnotationWindow::ResizeIfNecessary(::tools::Long aOldHeight, ::tools::Long aNewHeight)
{
    if (aOldHeight != aNewHeight)
        DoResize();
    else
        SetScrollbar();
}

void AnnotationWindow::SetLanguage(const SvxLanguageItem &aNewItem)
{
    mpOutliner->SetModifyHdl( Link<LinkParamNone*,void>() );
    ESelection aOld = GetOutlinerView()->GetSelection();

    ESelection aNewSelection( 0, 0, mpOutliner->GetParagraphCount()-1, EE_TEXTPOS_ALL );
    GetOutlinerView()->SetSelection( aNewSelection );
    SfxItemSet aEditAttr(GetOutlinerView()->GetAttribs());
    aEditAttr.Put(aNewItem);
    GetOutlinerView()->SetAttribs( aEditAttr );

    GetOutlinerView()->SetSelection(aOld);

    mxTextControl->Invalidate();
}

void AnnotationWindow::ToggleInsMode()
{
    if( mpOutlinerView )
    {
        SfxBindings &rBnd = mpDocShell->GetViewShell()->GetViewFrame()->GetBindings();
        rBnd.Invalidate(SID_ATTR_INSERT);
        rBnd.Update(SID_ATTR_INSERT);
    }
}

::tools::Long AnnotationWindow::GetPostItTextHeight()
{
    OutputDevice& rDevice = mxTextControl->GetDrawingArea()->get_ref_device();
    return mpOutliner ? rDevice.LogicToPixel(mpOutliner->CalcTextSize()).Height() : 0;
}

IMPL_LINK(AnnotationWindow, ScrollHdl, weld::ScrolledWindow&, rScrolledWindow, void)
{
    ::tools::Long nDiff = GetOutlinerView()->GetEditView().GetVisArea().Top() - rScrolledWindow.vadjustment_get_value();
    GetOutlinerView()->Scroll( 0, nDiff );
}

sdr::annotation::TextApiObject* getTextApiObject( const Reference< XAnnotation >& xAnnotation )
{
    if( xAnnotation.is() )
    {
        Reference< XText > xText( xAnnotation->getTextRange() );
        return sdr::annotation::TextApiObject::getImplementation(xText);
    }
    return nullptr;
}

void AnnotationWindow::setAnnotation( const Reference< XAnnotation >& xAnnotation )
{
    if( (xAnnotation == mxAnnotation) || !xAnnotation.is() )
        return;

    mxAnnotation = xAnnotation;

    SetColor();

    SvtUserOptions aUserOptions;
    mbProtected = aUserOptions.GetFullName() != xAnnotation->getAuthor();

    mpOutliner->Clear();
    auto* pTextApi = getTextApiObject( mxAnnotation );

    if( pTextApi )
    {
        std::optional< OutlinerParaObject > pOPO( pTextApi->CreateText() );
        mpOutliner->SetText(*pOPO);
    }

    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();

//TODO    Invalidate();

    OUString sMeta( xAnnotation->getAuthor() );
    OUString sDateTime( getAnnotationDateTimeString(xAnnotation) );

    if( !sDateTime.isEmpty() )
    {
        if( !sMeta.isEmpty() )
            sMeta += "\n";

        sMeta += sDateTime;
    }
    mxMeta->set_label(sMeta);
}

void AnnotationWindow::SetColor()
{
    sal_uInt16 nAuthorIdx = mpDoc->GetAnnotationAuthorIndex( mxAnnotation->getAuthor() );

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const bool bHighContrast = rStyleSettings.GetHighContrastMode();
    if( bHighContrast )
    {
        maColor = rStyleSettings.GetWindowColor();
        maColorDark = maColor;
        maColorLight = rStyleSettings.GetWindowTextColor();
    }
    else
    {
        maColor = AnnotationManagerImpl::GetColor( nAuthorIdx );
        maColorDark = AnnotationManagerImpl::GetColorDark( nAuthorIdx );
        maColorLight = AnnotationManagerImpl::GetColorLight( nAuthorIdx );
    }

    mpOutliner->ForceAutoColor( bHighContrast || officecfg::Office::Common::Accessibility::IsAutomaticFontColor::get() );

    mxPopover->set_background(maColor);
    mxMenuButton->set_background(maColor);

    mxMeta->set_font_color(bHighContrast ? maColorLight : maColorDark);

    mxVScrollbar->customize_scrollbars(maColorLight,
                                       maColorDark,
                                       maColor);
    mxVScrollbar->set_scroll_thickness(GetPrefScrollbarWidth());
}

void AnnotationWindow::SaveToDocument()
{
    Reference< XAnnotation > xAnnotation( mxAnnotation );

    // write changed text back to annotation
    if (mpOutliner->IsModified())
    {
        auto* pTextApi = getTextApiObject( xAnnotation );

        if( pTextApi )
        {
            std::optional<OutlinerParaObject> pOPO = mpOutliner->CreateParaObject();
            if( pOPO )
            {
                if( mpDoc->IsUndoEnabled() )
                    mpDoc->BegUndo( SdResId( STR_ANNOTATION_UNDO_EDIT ) );

                pTextApi->SetText( *pOPO );
                pOPO.reset();

                // set current time to changed annotation
                xAnnotation->setDateTime( getCurrentDateTime() );

                if( mpDoc->IsUndoEnabled() )
                    mpDoc->EndUndo();

                mpDocShell->SetModified();
            }

        }
    }
    mpOutliner->ClearModifyFlag();

    mpOutliner->GetUndoManager().Clear();
}

bool AnnotationTextWindow::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        const bool bReadOnly = mrContents.DocShell()->IsReadOnly();
        if (bReadOnly)
            return true;

        SfxDispatcher* pDispatcher = mrContents.DocShell()->GetViewShell()->GetViewFrame()->GetDispatcher();
        if( !pDispatcher )
            return true;

        if (IsMouseCaptured())
        {
            // so the menu can capture it and the EditView doesn't get the button release and change its
            // selection on a successful button click
            ReleaseMouse();
        }

        ::tools::Rectangle aRect(rCEvt.GetMousePosPixel(), Size(1, 1));
        weld::Widget* pPopupParent = GetDrawingArea();
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pPopupParent, u"modules/simpress/ui/annotationtagmenu.ui"_ustr));
        std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu(u"menu"_ustr));

        auto xAnnotation = mrContents.getAnnotation();

        SvtUserOptions aUserOptions;
        OUString sCurrentAuthor( aUserOptions.GetFullName() );
        OUString sAuthor( xAnnotation->getAuthor() );

        OUString aStr(xMenu->get_label(u".uno:DeleteAllAnnotationByAuthor"_ustr));
        OUString aReplace( sAuthor );
        if( aReplace.isEmpty() )
            aReplace = SdResId( STR_ANNOTATION_NOAUTHOR );
        aStr = aStr.replaceFirst("%1", aReplace);
        xMenu->set_label(u".uno:DeleteAllAnnotationByAuthor"_ustr, aStr);

        bool bShowReply = sAuthor != sCurrentAuthor && !bReadOnly;
        xMenu->set_visible(u".uno:ReplyToAnnotation"_ustr, bShowReply);
        xMenu->set_visible(u"separator"_ustr, bShowReply);
        xMenu->set_visible(u".uno:DeleteAnnotation"_ustr, xAnnotation.is() && !bReadOnly);
        xMenu->set_visible(u".uno:DeleteAllAnnotationByAuthor"_ustr, !bReadOnly);
        xMenu->set_visible(u".uno:DeleteAllAnnotation"_ustr, !bReadOnly);

        int nInsertPos = 2;

        auto xFrame = mrContents.DocShell()->GetViewShell()->GetViewFrame()->GetFrame().GetFrameInterface();
        OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(xFrame));

        bool bEditable = !mrContents.IsProtected() && !bReadOnly;
        if (bEditable)
        {
            SfxItemSet aSet(mrContents.GetOutlinerView()->GetAttribs());

            xMenu->insert(nInsertPos++, u".uno:Bold"_ustr,
                          vcl::CommandInfoProvider::GetMenuLabelForCommand(
                              vcl::CommandInfoProvider::GetCommandProperties(u".uno:Bold"_ustr, aModuleName)),
                          nullptr, nullptr, vcl::CommandInfoProvider::GetXGraphicForCommand(u".uno:Bold"_ustr, xFrame),
                          TRISTATE_TRUE);

            if ( aSet.GetItemState( EE_CHAR_WEIGHT ) == SfxItemState::SET )
            {
                if( aSet.Get( EE_CHAR_WEIGHT ).GetWeight() == WEIGHT_BOLD )
                    xMenu->set_active(u".uno:Bold"_ustr, true);
            }

            xMenu->insert(nInsertPos++, u".uno:Italic"_ustr,
                          vcl::CommandInfoProvider::GetMenuLabelForCommand(
                              vcl::CommandInfoProvider::GetCommandProperties(u".uno:Italic"_ustr, aModuleName)),
                          nullptr, nullptr, vcl::CommandInfoProvider::GetXGraphicForCommand(u".uno:Italic"_ustr, xFrame),
                          TRISTATE_TRUE);

            if ( aSet.GetItemState( EE_CHAR_ITALIC ) == SfxItemState::SET )
            {
                if( aSet.Get( EE_CHAR_ITALIC ).GetPosture() != ITALIC_NONE )
                    xMenu->set_active(u".uno:Italic"_ustr, true);

            }

            xMenu->insert(nInsertPos++, u".uno:Underline"_ustr,
                          vcl::CommandInfoProvider::GetMenuLabelForCommand(
                              vcl::CommandInfoProvider::GetCommandProperties(u".uno:Underline"_ustr, aModuleName)),
                          nullptr, nullptr, vcl::CommandInfoProvider::GetXGraphicForCommand(u".uno:Underline"_ustr, xFrame),
                          TRISTATE_TRUE);

            if ( aSet.GetItemState( EE_CHAR_UNDERLINE ) == SfxItemState::SET )
            {
                if( aSet.Get( EE_CHAR_UNDERLINE ).GetLineStyle() != LINESTYLE_NONE )
                    xMenu->set_active(u".uno:Underline"_ustr, true);
            }

            xMenu->insert(nInsertPos++, u".uno:Strikeout"_ustr,
                          vcl::CommandInfoProvider::GetMenuLabelForCommand(
                              vcl::CommandInfoProvider::GetCommandProperties(u".uno:Strikeout"_ustr, aModuleName)),
                          nullptr, nullptr, vcl::CommandInfoProvider::GetXGraphicForCommand(u".uno:Strikeout"_ustr, xFrame),
                          TRISTATE_TRUE);

            if ( aSet.GetItemState( EE_CHAR_STRIKEOUT ) == SfxItemState::SET )
            {
                if( aSet.Get( EE_CHAR_STRIKEOUT ).GetStrikeout() != STRIKEOUT_NONE )
                    xMenu->set_active(u".uno:Strikeout"_ustr, true);
            }

            xMenu->insert_separator(nInsertPos++, u"separator2"_ustr);
        }

        xMenu->insert(nInsertPos++, u".uno:Copy"_ustr,
                      vcl::CommandInfoProvider::GetMenuLabelForCommand(
                          vcl::CommandInfoProvider::GetCommandProperties(u".uno:Copy"_ustr, aModuleName)),
                      nullptr, nullptr, vcl::CommandInfoProvider::GetXGraphicForCommand(u".uno:Copy"_ustr, xFrame),
                      TRISTATE_INDET);

        xMenu->insert(nInsertPos++, u".uno:Paste"_ustr,
                      vcl::CommandInfoProvider::GetMenuLabelForCommand(
                          vcl::CommandInfoProvider::GetCommandProperties(u".uno:Paste"_ustr, aModuleName)),
                      nullptr, nullptr, vcl::CommandInfoProvider::GetXGraphicForCommand(u".uno:Paste"_ustr, xFrame),
                      TRISTATE_INDET);

        bool bCanPaste = false;
        if (bEditable)
        {
            TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromClipboard(GetClipboard()));
            bCanPaste = aDataHelper.GetFormatCount() != 0;
        }

        xMenu->insert_separator(nInsertPos++, u"separator3"_ustr);

        xMenu->set_sensitive(u".uno:Copy"_ustr, mrContents.GetOutlinerView()->HasSelection());
        xMenu->set_sensitive(u".uno:Paste"_ustr, bCanPaste);

        auto sId = xMenu->popup_at_rect(pPopupParent, aRect);

        if (sId == ".uno:ReplyToAnnotation")
        {
            const SfxUnoAnyItem aItem( SID_REPLYTO_POSTIT, Any( xAnnotation ) );
            pDispatcher->ExecuteList(SID_REPLYTO_POSTIT,
                    SfxCallMode::ASYNCHRON, { &aItem });
        }
        else if (sId == ".uno:DeleteAnnotation")
        {
            const SfxUnoAnyItem aItem( SID_DELETE_POSTIT, Any( xAnnotation ) );
            pDispatcher->ExecuteList(SID_DELETE_POSTIT, SfxCallMode::ASYNCHRON,
                    { &aItem });
        }
        else if (sId == ".uno:DeleteAllAnnotationByAuthor")
        {
            const SfxStringItem aItem( SID_DELETEALLBYAUTHOR_POSTIT, sAuthor );
            pDispatcher->ExecuteList( SID_DELETEALLBYAUTHOR_POSTIT,
                    SfxCallMode::ASYNCHRON, { &aItem });
        }
        else if (sId == ".uno:DeleteAllAnnotation")
            pDispatcher->Execute( SID_DELETEALL_POSTIT );
        else if (sId == ".uno:Copy")
        {
            mrContents.GetOutlinerView()->Copy();
        }
        else if (sId == ".uno:Paste")
        {
            mrContents.GetOutlinerView()->PasteSpecial();
            mrContents.DoResize();
        }
        else if (!sId.isEmpty())
        {
            SfxItemSet aEditAttr(mrContents.GetOutlinerView()->GetAttribs());
            SfxItemSet aNewAttr(mrContents.GetOutliner()->GetEmptyItemSet());

            if (sId == ".uno:Bold")
            {
                FontWeight eFW = aEditAttr.Get( EE_CHAR_WEIGHT ).GetWeight();
                aNewAttr.Put( SvxWeightItem( eFW == WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
            }
            else if (sId == ".uno:Italic")
            {
                FontItalic eFI = aEditAttr.Get( EE_CHAR_ITALIC ).GetPosture();
                aNewAttr.Put( SvxPostureItem( eFI == ITALIC_NORMAL ? ITALIC_NONE : ITALIC_NORMAL, EE_CHAR_ITALIC ) );
            }
            else if (sId == ".uno:Underline")
            {
                FontLineStyle eFU = aEditAttr. Get( EE_CHAR_UNDERLINE ).GetLineStyle();
                aNewAttr.Put( SvxUnderlineItem( eFU == LINESTYLE_SINGLE ? LINESTYLE_NONE : LINESTYLE_SINGLE, EE_CHAR_UNDERLINE ) );
            }
            else if (sId == ".uno:Strikeout")
            {
                FontStrikeout eFSO = aEditAttr.Get( EE_CHAR_STRIKEOUT ).GetStrikeout();
                aNewAttr.Put( SvxCrossedOutItem( eFSO == STRIKEOUT_SINGLE ? STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT ) );
            }

            mrContents.GetOutlinerView()->SetAttribs( aNewAttr );
        }

        return true;
    }
    return WeldEditView::Command(rCEvt);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
