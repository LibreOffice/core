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
#include <svx/svxids.hrc>
#include <unotools/useroptions.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include <vcl/commandevent.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/scrbar.hxx>
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
#include <textapi.hxx>
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

/************ AnnotationTextWindow **********************************/

AnnotationTextWindow::AnnotationTextWindow( AnnotationWindow* pParent, WinBits nBits )
: Control(pParent, nBits)
, mpOutlinerView(nullptr)
, mpAnnotationWindow( pParent )
{
}

AnnotationTextWindow::~AnnotationTextWindow()
{
    disposeOnce();
}

void AnnotationTextWindow::dispose()
{
    mpAnnotationWindow.clear();
    Control::dispose();
}

void AnnotationTextControl::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect)
{
    Size aSize = GetOutputSizePixel();

    const bool bHighContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
    if (!bHighContrast)
    {
        rRenderContext.DrawGradient(::tools::Rectangle(Point(0,0), rRenderContext.PixelToLogic(aSize)),
                                    Gradient(GradientStyle::Linear, mrContents.maColorLight, mrContents.maColor));
    }

    DoPaint(rRenderContext, rRect);
}

void AnnotationTextControl::EditViewScrollStateChange()
{
    mrContents.SetScrollbar();
}

bool AnnotationTextControl::KeyInput(const KeyEvent& rKeyEvt)
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
                EditView* pEditView = GetEditView();
                bDone = pEditView && pEditView->PostKeyEvent(rKeyEvt);
            }
        }
        if (bDone)
        {
            mrContents.ResizeIfNecessary(aOldHeight, mrContents.GetPostItTextHeight());
        }
    }

    return bDone;
}

void AnnotationTextWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
           mpAnnotationWindow->Command(rCEvt);
    }
    else
    {
        if ( mpOutlinerView )
            mpOutlinerView->Command( rCEvt );
        else
            Window::Command(rCEvt);
    }
}

/************** AnnotationWindow***********************************++*/
AnnotationWindow::AnnotationWindow(AnnotationManagerImpl& rManager, DrawDocShell* pDocShell, vcl::Window* pParent)
    : FloatingWindow(pParent, WB_BORDER | WB_SYSTEMWINDOW)
    , mrManager( rManager )
    , mpDocShell( pDocShell )
    , mpDoc( pDocShell->GetDoc() )
    , mxContents(VclPtr<AnnotationContents>::Create(this, pDocShell))
{
    EnableAlwaysOnTop();
}

AnnotationWindow::~AnnotationWindow()
{
    disposeOnce();
}

AnnotationTextControl::AnnotationTextControl(AnnotationContents& rContents)
    : mrContents(rContents)
{
}

EditView* AnnotationTextControl::GetEditView() const
{
    OutlinerView* pOutlinerView = mrContents.GetOutlinerView();
    if (!pOutlinerView)
        return nullptr;
    return &pOutlinerView->GetEditView();
}

EditEngine* AnnotationTextControl::GetEditEngine() const
{
    OutlinerView* pOutlinerView = mrContents.GetOutlinerView();
    if (!pOutlinerView)
        return nullptr;
    return pOutlinerView->GetEditView().GetEditEngine();
}

void AnnotationTextControl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
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
    aSize = aOutputSize;
    aSize.setHeight(aSize.Height());

    EditView* pEditView = GetEditView();
    pEditView->setEditViewCallbacks(this);

    EditEngine* pEditEngine = GetEditEngine();
    pEditEngine->SetPaperSize(aSize);
    pEditEngine->SetRefDevice(&rDevice);

    pEditView->SetOutputArea(::tools::Rectangle(Point(0, 0), aOutputSize));
    pEditView->SetBackgroundColor(aBgColor);

    pDrawingArea->set_cursor(PointerStyle::Text);

    InitAccessible();
}

// see SwAnnotationWin in sw for something similar
AnnotationContents::AnnotationContents(vcl::Window* pParent, DrawDocShell* pDocShell)
    : InterimItemWindow(pParent, "modules/simpress/ui/annotation.ui", "Annotation")
    , mpDocShell(pDocShell)
    , mpDoc(pDocShell->GetDoc())
    , mbReadonly(pDocShell->IsReadOnly())
    , mbProtected(false)
{
}

void AnnotationContents::dispose()
{
    mxTextControlWin.reset();
    mxTextControl.reset();

    mxMeta.reset();
    mxVScrollbar.reset();

    mxMenuButton.reset();

    mpOutliner.reset();
    mpOutlinerView.reset();

    InterimItemWindow::dispose();
}

void AnnotationWindow::dispose()
{
    mxContents.disposeAndClear();
    FloatingWindow::dispose();
}

void AnnotationContents::InitControls()
{
    // window control for author and date
    mxMeta = m_xBuilder->weld_label("meta");
    mxMeta->set_direction(AllSettings::GetLayoutRTL());

    maLabelFont = Application::GetSettings().GetStyleSettings().GetLabelFont();
    maLabelFont.SetFontHeight(8);

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    mxMeta->set_font(maLabelFont);

    mpOutliner.reset( new ::Outliner(GetAnnotationPool(),OutlinerMode::TextObject) );
    SdDrawDocument::SetCalcFieldValueHdl( mpOutliner.get() );
    mpOutliner->SetUpdateMode( true );
    Rescale();

    if (OutputDevice* pDev = mpDoc->GetRefDevice())
        mpOutliner->SetRefDevice( pDev );

    mpOutlinerView.reset( new OutlinerView ( mpOutliner.get(), nullptr) );
    mpOutliner->InsertView(mpOutlinerView.get() );

    //create Scrollbars
    mxVScrollbar = m_xBuilder->weld_scrolled_window("scrolledwindow", true);

    // actual window which holds the user text
    mxTextControl.reset(new AnnotationTextControl(*this));
    mxTextControlWin.reset(new weld::CustomWeld(*m_xBuilder, "editview", *mxTextControl));
    mxTextControl->SetPointer(PointerStyle::Text);

    mxVScrollbar->set_direction(false);
    mxVScrollbar->connect_vadjustment_changed(LINK(this, AnnotationContents, ScrollHdl));

    mpOutlinerView->SetBackgroundColor(COL_TRANSPARENT);
    mpOutlinerView->SetOutputArea( PixelToLogic( ::tools::Rectangle(0,0,1,1) ) );

    mxMenuButton = m_xBuilder->weld_menu_button("menubutton");
    mxMenuButton->set_size_request(METABUTTON_WIDTH, METABUTTON_HEIGHT);

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

void AnnotationContents::StartEdit()
{
    GetOutlinerView()->SetSelection(ESelection(EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT,EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT));
    GetOutlinerView()->ShowCursor();
}

void AnnotationContents::Rescale()
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
    mxContents->SetSizePixel(GetSizePixel());
    mxContents->Show();

    mxContents->DoResize();
}

void AnnotationContents::DoResize()
{
    ::tools::Long aTextHeight = LogicToPixel( mpOutliner->CalcTextSize()).Height();
    ::tools::Long aHeight = GetSizePixel().Height();
    ::tools::ULong aWidth = GetSizePixel().Width();

    aHeight -= POSTIT_META_HEIGHT;

    if( aTextHeight > aHeight )
    {
        // we need vertical scrollbars and have to reduce the width
        aWidth -= mxVScrollbar->get_vscroll_width();
        mxVScrollbar->set_vpolicy(VclPolicyType::ALWAYS);
    }
    else
    {
        mxVScrollbar->set_vpolicy(VclPolicyType::NEVER);
    }

    mpOutliner->SetPaperSize( PixelToLogic( Size(aWidth,aHeight) ) ) ;

    ::tools::Rectangle aOutputArea = PixelToLogic(::tools::Rectangle(0, 0, aWidth, aHeight));
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
    int nPageIncrement = PixelToLogic(Size(0,aHeight)).Height() * 8 / 10;
    int nPageSize = PixelToLogic(Size(0,aHeight)).Height();

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

void AnnotationContents::SetScrollbar()
{
    mxVScrollbar->vadjustment_set_value(mpOutlinerView->GetVisArea().Top());
}

void AnnotationContents::ResizeIfNecessary(::tools::Long aOldHeight, ::tools::Long aNewHeight)
{
    if (aOldHeight != aNewHeight)
        DoResize();
    else
        SetScrollbar();
}

void AnnotationContents::SetLanguage(const SvxLanguageItem &aNewItem)
{
    mpOutliner->SetModifyHdl( Link<LinkParamNone*,void>() );
    ESelection aOld = GetOutlinerView()->GetSelection();

    ESelection aNewSelection( 0, 0, mpOutliner->GetParagraphCount()-1, EE_TEXTPOS_ALL );
    GetOutlinerView()->SetSelection( aNewSelection );
    SfxItemSet aEditAttr(GetOutlinerView()->GetAttribs());
    aEditAttr.Put(aNewItem);
    GetOutlinerView()->SetAttribs( aEditAttr );

    GetOutlinerView()->SetSelection(aOld);

    Invalidate();
}

void AnnotationContents::ToggleInsMode()
{
    if( mpOutlinerView )
    {
        SfxBindings &rBnd = mpDocShell->GetViewShell()->GetViewFrame()->GetBindings();
        rBnd.Invalidate(SID_ATTR_INSERT);
        rBnd.Update(SID_ATTR_INSERT);
    }
}

::tools::Long AnnotationContents::GetPostItTextHeight()
{
    return mpOutliner ? LogicToPixel(mpOutliner->CalcTextSize()).Height() : 0;
}

IMPL_LINK(AnnotationContents, ScrollHdl, weld::ScrolledWindow&, rScrolledWindow, void)
{
    ::tools::Long nDiff = GetOutlinerView()->GetEditView().GetVisArea().Top() - rScrolledWindow.vadjustment_get_value();
    GetOutlinerView()->Scroll( 0, nDiff );
}

TextApiObject* getTextApiObject( const Reference< XAnnotation >& xAnnotation )
{
    if( xAnnotation.is() )
    {
        Reference< XText > xText( xAnnotation->getTextRange() );
        return TextApiObject::getImplementation( xText );
    }
    return nullptr;
}

void AnnotationContents::setAnnotation( const Reference< XAnnotation >& xAnnotation )
{
    if( (xAnnotation == mxAnnotation) || !xAnnotation.is() )
        return;

    mxAnnotation = xAnnotation;

    SetColor();

    SvtUserOptions aUserOptions;
    mbProtected = aUserOptions.GetFullName() != xAnnotation->getAuthor();

    mpOutliner->Clear();
    TextApiObject* pTextApi = getTextApiObject( mxAnnotation );

    if( pTextApi )
    {
        std::unique_ptr< OutlinerParaObject > pOPO( pTextApi->CreateText() );
        mpOutliner->SetText(*pOPO);
    }

    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();

    Invalidate();

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

void AnnotationContents::SetColor()
{
    sal_uInt16 nAuthorIdx = mpDoc->GetAnnotationAuthorIndex( mxAnnotation->getAuthor() );

    const bool bHighContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
    if( bHighContrast )
    {
        StyleSettings aStyleSettings = GetSettings().GetStyleSettings();

        maColor = aStyleSettings.GetWindowColor();
        maColorDark = maColor;
        maColorLight = aStyleSettings.GetWindowTextColor();
    }
    else
    {
        maColor = AnnotationManagerImpl::GetColor( nAuthorIdx );
        maColorDark = AnnotationManagerImpl::GetColorDark( nAuthorIdx );
        maColorLight = AnnotationManagerImpl::GetColorLight( nAuthorIdx );
    }

    {
        SvtAccessibilityOptions aOptions;
        mpOutliner->ForceAutoColor( bHighContrast || aOptions.GetIsAutomaticFontColor() );
    }

    m_xContainer->set_background(maColor);
    mxMenuButton->set_background(maColor);

    mxMeta->set_font_color(bHighContrast ? maColorLight : maColorDark);

    mxVScrollbar->customize_scrollbars(maColorLight,
                                       maColorDark,
                                       maColor,
                                       GetPrefScrollbarWidth());
}

void AnnotationContents::GetFocus()
{
    if (mxTextControl)
        mxTextControl->GrabFocus();
}

#if 0
void AnnotationWindow::Deactivate()
{
    //tdf#99388 and tdf#99712, don't deactivate if we lose focus because of our
    //own popup
    if (mrManager.getPopupMenuActive())
        return;

    if (!mpOutliner) //in dispose
        return;

    Reference< XAnnotation > xAnnotation( mxAnnotation );

    // write changed text back to annotation
    if ( Engine()->IsModified() )
    {
        TextApiObject* pTextApi = getTextApiObject( xAnnotation );

        if( pTextApi )
        {
            std::unique_ptr<OutlinerParaObject> pOPO = Engine()->CreateParaObject();
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
    Engine()->ClearModifyFlag();

    Engine()->GetUndoManager().Clear();
}
#endif

#if 0

void AnnotationWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        if( mpMeta->IsVisible() &&(mpMeta->GetPosPixel().Y() < rCEvt.GetMousePosPixel().Y()) )
            return;
        mrManager.ExecuteAnnotationContextMenu( mxAnnotation, this, ::tools::Rectangle(rCEvt.GetMousePosPixel(),Size(1,1)) );
    }
    else
    {
        FloatingWindow::Command(rCEvt);
    }
}
#endif

void AnnotationWindow::GetFocus()
{
    if (mxContents)
        mxContents->GrabFocus();
    else
        FloatingWindow::GetFocus();
}

void AnnotationWindow::ExecuteSlot( sal_uInt16 /*nSID*/ )
{
#if 0
    if( nSID == SID_COPY )
    {
        getView()->Copy();
    }
    else if( nSID == SID_PASTE )
    {
        getView()->PasteSpecial();
        DoResize();
    }
    else
    {
        SfxItemSet aEditAttr(getView()->GetAttribs());
        SfxItemSet aNewAttr(mpOutliner->GetEmptyItemSet());

        switch( nSID )
        {
        case SID_ATTR_CHAR_WEIGHT:
        {
            FontWeight eFW = aEditAttr.Get( EE_CHAR_WEIGHT ).GetWeight();
            aNewAttr.Put( SvxWeightItem( eFW == WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        }
        break;
        case SID_ATTR_CHAR_POSTURE:
        {
            FontItalic eFI = aEditAttr.Get( EE_CHAR_ITALIC ).GetPosture();
            aNewAttr.Put( SvxPostureItem( eFI == ITALIC_NORMAL ? ITALIC_NONE : ITALIC_NORMAL, EE_CHAR_ITALIC ) );
        }
        break;
        case SID_ATTR_CHAR_UNDERLINE:
        {
            FontLineStyle eFU = aEditAttr. Get( EE_CHAR_UNDERLINE ).GetLineStyle();
            aNewAttr.Put( SvxUnderlineItem( eFU == LINESTYLE_SINGLE ? LINESTYLE_NONE : LINESTYLE_SINGLE, EE_CHAR_UNDERLINE ) );
        }
        break;
        case SID_ATTR_CHAR_STRIKEOUT:
        {
            FontStrikeout eFSO = aEditAttr.Get( EE_CHAR_STRIKEOUT ).GetStrikeout();
            aNewAttr.Put( SvxCrossedOutItem( eFSO == STRIKEOUT_SINGLE ? STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT ) );
        }
        break;
        }
        getView()->SetAttribs( aNewAttr );
    }
#endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
