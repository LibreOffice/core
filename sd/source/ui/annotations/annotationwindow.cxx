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

#include <editeng/fontitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/frmdir.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editview.hxx>
#include <svx/svdview.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <svx/svxids.hrc>
#include <svtools/langtab.hxx>
#include <svl/slstitm.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/useroptions.hxx>
#include <svl/languageoptions.hxx>
#include <svl/zforlist.hxx>
#include <svtools/svmedit.hxx>

#include <linguistic/lngprops.hxx>

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/mnumgr.hxx>

#include <vcl/vclenum.hxx>
#include <vcl/edit.hxx>
#include <vcl/help.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>
#include <vcl/gradient.hxx>
#include <vcl/cursor.hxx>
#include <vcl/settings.hxx>

#include <tools/helpers.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include "annotations.hrc"
#include "annotationwindow.hxx"
#include "annotationmanagerimpl.hxx"

#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "View.hxx"
#include "textapi.hxx"
#include "sdresid.hxx"

#include <memory>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::text;

#define METABUTTON_WIDTH        16
#define METABUTTON_HEIGHT       18
#define METABUTTON_AREA_WIDTH   30
#define POSTIT_META_HEIGHT  (sal_Int32)     30

namespace sd {

Color ColorFromAlphaColor(sal_uInt8 aTransparency, Color &aFront, Color &aBack )
{
    return Color((sal_uInt8)(aFront.GetRed()    * aTransparency/(double)255 + aBack.GetRed()    * (1-aTransparency/(double)255)),
                 (sal_uInt8)(aFront.GetGreen()  * aTransparency/(double)255 + aBack.GetGreen()  * (1-aTransparency/(double)255)),
                 (sal_uInt8)(aFront.GetBlue()   * aTransparency/(double)255 + aBack.GetBlue()   * (1-aTransparency/(double)255)));
}

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

void AnnotationTextWindow::Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect)
{
    const bool bHighContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
    if ( !bHighContrast )
    {
        DrawGradient(Rectangle(Point(0,0),PixelToLogic(GetSizePixel())),
            Gradient(GradientStyle_LINEAR,mpAnnotationWindow->maColorLight,mpAnnotationWindow->maColor));
     }

    if( mpOutlinerView )
    {
        Color aBackgroundColor( mpAnnotationWindow->maColor );
        if( bHighContrast )
        {
            aBackgroundColor = GetSettings().GetStyleSettings().GetWindowColor();
        }

        mpOutlinerView->SetBackgroundColor( aBackgroundColor );

        mpOutlinerView->Paint( rRect );
    }
}

void AnnotationTextWindow::KeyInput( const KeyEvent& rKeyEvt )
{
    const vcl::KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    sal_uInt16 nKey = rKeyCode.GetCode();

    if ((rKeyCode.IsMod1() && rKeyCode.IsMod2()) && ((nKey == KEY_PAGEUP) || (nKey == KEY_PAGEDOWN)))
    {
        SfxDispatcher* pDispatcher = mpAnnotationWindow->DocShell()->GetViewShell()->GetViewFrame()->GetDispatcher();
        if( pDispatcher )
            pDispatcher->Execute( nKey == KEY_PAGEDOWN ? SID_NEXT_POSTIT : SID_PREVIOUS_POSTIT );
    }
    else if (nKey == KEY_INSERT)
    {
        if (!rKeyCode.IsMod1() && !rKeyCode.IsMod2())
            mpAnnotationWindow->ToggleInsMode();
    }
    else
    {
        long aOldHeight = mpAnnotationWindow->GetPostItTextHeight();
        bool bDone = false;

        /// HACK: need to switch off processing of Undo/Redo in Outliner
        if ( !( (nKey == KEY_Z || nKey == KEY_Y) && rKeyCode.IsMod1()) )
        {
            bool bIsProtected = mpAnnotationWindow->IsProtected();
            if (!bIsProtected || !EditEngine::DoesKeyChangeText(rKeyEvt) )

                bDone = mpOutlinerView->PostKeyEvent( rKeyEvt );
        }
        if (bDone)
        {
            mpAnnotationWindow->ResizeIfNecessary(aOldHeight,mpAnnotationWindow->GetPostItTextHeight());
        }
        else
        {
            Control::KeyInput(rKeyEvt);
        }
    }
}

void AnnotationTextWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( mpOutlinerView )
    {
        mpOutlinerView->MouseMove( rMEvt );
        SetPointer( mpOutlinerView->GetPointer( rMEvt.GetPosPixel() ) );
    }
}

void AnnotationTextWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();
    if ( mpOutlinerView )
        mpOutlinerView->MouseButtonDown( rMEvt );
}

void AnnotationTextWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( mpOutlinerView )
        mpOutlinerView->MouseButtonUp( rMEvt );
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

void AnnotationTextWindow::GetFocus()
{
    Window::GetFocus();
}

void AnnotationTextWindow::LoseFocus()
{
    Window::LoseFocus();
}

OUString AnnotationTextWindow::GetSurroundingText() const
{
    if( mpOutlinerView )
    {
        EditEngine *aEditEngine = mpOutlinerView->GetEditView().GetEditEngine();
        if( mpOutlinerView->HasSelection() )
            return mpOutlinerView->GetSelected();
        else
        {
            ESelection aSelection = mpOutlinerView->GetEditView().GetSelection();
            return aEditEngine->GetText(aSelection.nStartPara);
        }
    }
    return OUString();
}

Selection AnnotationTextWindow::GetSurroundingTextSelection() const
{
    if( mpOutlinerView )
    {
        if( mpOutlinerView->HasSelection() )
            return Selection( 0, mpOutlinerView->GetSelected().getLength() );
        else
        {
            ESelection aSelection = mpOutlinerView->GetEditView().GetSelection();
            return Selection( aSelection.nStartPos, aSelection.nEndPos );
        }
    }
    else
        return Selection( 0, 0 );
}

/************** AnnotationWindow***********************************++*/

AnnotationWindow::AnnotationWindow( AnnotationManagerImpl& rManager, DrawDocShell* pDocShell, vcl::Window* pParent )
: FloatingWindow(pParent, WB_BORDER)
, mrManager( rManager )
, mpDocShell( pDocShell )
, mpDoc( pDocShell->GetDoc() )
, mpOutlinerView(nullptr)
, mpOutliner(nullptr)
, mpVScrollbar(nullptr)
, mbReadonly(pDocShell->IsReadOnly())
, mbProtected(false)
, mbMouseOverButton(false)
, mpTextWindow(nullptr)
, mpMeta(nullptr)
{
    EnableAlwaysOnTop();
}

AnnotationWindow::~AnnotationWindow()
{
    disposeOnce();
}

void AnnotationWindow::dispose()
{
    mpMeta.disposeAndClear();
    delete mpOutlinerView;
    delete mpOutliner;
    mpVScrollbar.disposeAndClear();
    mpTextWindow.disposeAndClear();
    FloatingWindow::dispose();
}

void AnnotationWindow::InitControls()
{
    // actual window which holds the user text
    mpTextWindow = VclPtr<AnnotationTextWindow>::Create(this, WB_NODIALOGCONTROL);
    mpTextWindow->SetPointer(Pointer(PointerStyle::Text));

    // window control for author and date
    mpMeta = VclPtr<MultiLineEdit>::Create(this,0);
    mpMeta->SetReadOnly();
    mpMeta->SetRightToLeft(AllSettings::GetLayoutRTL());
    mpMeta->AlwaysDisableInput(true);
    mpMeta->SetCallHandlersOnInputDisabled(true);

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    AllSettings aSettings = mpMeta->GetSettings();
    StyleSettings aStyleSettings = aSettings.GetStyleSettings();
    vcl::Font aFont = aStyleSettings.GetFieldFont();
    aFont.SetHeight(8);
    aStyleSettings.SetFieldFont(aFont);
    aSettings.SetStyleSettings(aStyleSettings);
    mpMeta->SetSettings(aSettings);

    mpOutliner = new ::Outliner(GetAnnotationPool(),OUTLINERMODE_TEXTOBJECT);
    Doc()->SetCalcFieldValueHdl( mpOutliner );
    mpOutliner->SetUpdateMode( true );
    Rescale();

    OutputDevice* pDev = Doc()->GetRefDevice();
    if( pDev )
    {
        mpOutliner->SetRefDevice( pDev );
    }

    mpTextWindow->EnableRTL( false );
    mpOutlinerView = new OutlinerView ( mpOutliner, mpTextWindow );
    mpOutliner->InsertView(mpOutlinerView );
    mpTextWindow->SetOutlinerView(mpOutlinerView);
    mpOutlinerView->SetOutputArea( PixelToLogic( Rectangle(0,0,1,1) ) );

    //create Scrollbars
    mpVScrollbar = VclPtr<ScrollBar>::Create(this, WB_3DLOOK |WB_VSCROLL|WB_DRAG);
    mpVScrollbar->EnableNativeWidget(false);
    mpVScrollbar->EnableRTL( false );
    mpVScrollbar->SetScrollHdl(LINK(this, AnnotationWindow, ScrollHdl));
    mpVScrollbar->EnableDrag();

    EEControlBits nCntrl = mpOutliner->GetControlWord();
    nCntrl |= EEControlBits::PASTESPECIAL | EEControlBits::AUTOCORRECT | EEControlBits::USECHARATTRIBS | EEControlBits::NOCOLORS;
    mpOutliner->SetControlWord(nCntrl);

    Engine()->SetModifyHdl( Link<LinkParamNone*,void>() );
    Engine()->EnableUndo( false );

    Engine()->ClearModifyFlag();
    Engine()->GetUndoManager().Clear();
    Engine()->EnableUndo( true );

    Invalidate();

    SetLanguage(GetLanguage());

    mpMeta->Show();
    mpVScrollbar->Show();
    mpTextWindow->Show();
}

void AnnotationWindow::StartEdit()
{
    getView()->SetSelection(ESelection(EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT,EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT));
    getView()->ShowCursor();
}

void AnnotationWindow::Rescale()
{
    MapMode aMode(MAP_100TH_MM);
    aMode.SetOrigin( Point() );
    mpOutliner->SetRefMapMode( aMode );
    SetMapMode( aMode );
    mpTextWindow->SetMapMode( aMode );
    if ( mpMeta )
    {
        vcl::Font aFont( mpMeta->GetSettings().GetStyleSettings().GetFieldFont() );
        sal_Int32 nHeight = aFont.GetHeight();
        nHeight = nHeight * aMode.GetScaleY().GetNumerator() / aMode.GetScaleY().GetDenominator();
        aFont.SetHeight( nHeight );
        mpMeta->SetControlFont( aFont );
    }
}

void AnnotationWindow::DoResize()
{
    unsigned long aWidth    =   GetSizePixel().Width();
    long aHeight            =   GetSizePixel().Height() - POSTIT_META_HEIGHT;

    mpOutliner->SetPaperSize( PixelToLogic( Size(aWidth,aHeight) ) ) ;
    long aTextHeight        =   LogicToPixel( mpOutliner->CalcTextSize()).Height();

    if( aTextHeight > aHeight )
    {   // we need vertical scrollbars and have to reduce the width
        aWidth -= GetScrollbarWidth();
        mpVScrollbar->Show();
    }
    else
    {
        mpVScrollbar->Hide();
    }

    mpTextWindow->setPosSizePixel(0,0,aWidth, aHeight);

    if( mbReadonly )
        mpMeta->setPosSizePixel(0,aHeight,GetSizePixel().Width(),POSTIT_META_HEIGHT);
    else
        mpMeta->setPosSizePixel(0,aHeight,GetSizePixel().Width()-METABUTTON_AREA_WIDTH,POSTIT_META_HEIGHT);

    mpOutliner->SetPaperSize( PixelToLogic( Size(aWidth,aHeight) ) ) ;
    mpOutlinerView->SetOutputArea( PixelToLogic( Rectangle(0,0,aWidth,aHeight) ) );
    if (!mpVScrollbar->IsVisible())
    {   // if we do not have a scrollbar anymore, we want to see the complete text
        mpOutlinerView->SetVisArea( PixelToLogic( Rectangle(0,0,aWidth,aHeight) ) );
    }
    mpVScrollbar->setPosSizePixel( 0 + aWidth, 0, GetScrollbarWidth(), aHeight );
    mpVScrollbar->SetVisibleSize( PixelToLogic(Size(0,aHeight)).Height() );
    mpVScrollbar->SetPageSize( PixelToLogic(Size(0,aHeight)).Height() * 8 / 10 );
    mpVScrollbar->SetLineSize( mpOutliner->GetTextHeight() / 10 );
    SetScrollbar();
    mpVScrollbar->SetRange( Range(0, mpOutliner->GetTextHeight()));

    Point aPos( mpMeta->GetPosPixel());
    Point aBase( aPos.X() + aPos.X() + GetSizePixel().Width(), aPos.Y() );
    Point aLeft = PixelToLogic( Point( aBase.X() - (METABUTTON_WIDTH+5), aBase.Y()+17 ) );
    Point aRight = PixelToLogic( Point( aBase.X() - (METABUTTON_WIDTH-1), aBase.Y()+17 ) );
    Point aBottom = PixelToLogic( Point( aBase.X() - (METABUTTON_WIDTH+2), aBase.Y()+20 ) );

    maPopupTriangle.clear();
    maPopupTriangle.append(basegfx::B2DPoint(aLeft.X(),aLeft.Y()));
    maPopupTriangle.append(basegfx::B2DPoint(aRight.X(),aRight.Y()));
    maPopupTriangle.append(basegfx::B2DPoint(aBottom.X(),aBottom.Y()));
    maPopupTriangle.setClosed(true);
    maRectMetaButton = PixelToLogic( Rectangle( Point(
            aPos.X()+GetSizePixel().Width()-(METABUTTON_WIDTH+10),
            aPos.Y()+5 ),
            Size( METABUTTON_WIDTH, METABUTTON_HEIGHT ) ) );

}

void AnnotationWindow::SetSizePixel( const Size& rNewSize )
{
    Window::SetSizePixel(rNewSize);
}

void AnnotationWindow::SetScrollbar()
{
    mpVScrollbar->SetThumbPos(mpOutlinerView->GetVisArea().Top());
}

void AnnotationWindow::ResizeIfNecessary(long aOldHeight, long aNewHeight)
{
    if (aOldHeight != aNewHeight)
    {
        DoResize();
        Invalidate();
    }
    else
    {
        SetScrollbar();
    }
}

void AnnotationWindow::SetLanguage(const SvxLanguageItem &aNewItem)
{
    Engine()->SetModifyHdl( Link<LinkParamNone*,void>() );
    ESelection aOld = getView()->GetSelection();

    ESelection aNewSelection( 0, 0, Engine()->GetParagraphCount()-1, EE_TEXTPOS_ALL );
    getView()->SetSelection( aNewSelection );
    SfxItemSet aEditAttr(getView()->GetAttribs());
    aEditAttr.Put(aNewItem);
    getView()->SetAttribs( aEditAttr );

    getView()->SetSelection(aOld);

    Invalidate();
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

long AnnotationWindow::GetPostItTextHeight()
{
    return mpOutliner ? LogicToPixel(mpOutliner->CalcTextSize()).Height() : 0;
}

IMPL_LINK_TYPED(AnnotationWindow, ScrollHdl, ScrollBar*, pScroll, void)
{
    long nDiff = getView()->GetEditView().GetVisArea().Top() - pScroll->GetThumbPos();
    getView()->Scroll( 0, nDiff );
}

SvxLanguageItem AnnotationWindow::GetLanguage()
{
    return SvxLanguageItem( Doc()->GetLanguage( EE_CHAR_LANGUAGE ), SID_ATTR_LANGUAGE );
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

void AnnotationWindow::setAnnotation( const Reference< XAnnotation >& xAnnotation, bool bGrabFocus )
{
    if( (xAnnotation != mxAnnotation) && xAnnotation.is() )
    {
        mxAnnotation = xAnnotation;

        SetColor();

        SvtUserOptions aUserOptions;
        mbProtected = aUserOptions.GetFullName() != xAnnotation->getAuthor();

        Engine()->Clear();
        TextApiObject* pTextApi = getTextApiObject( mxAnnotation );

        if( pTextApi )
        {
            std::unique_ptr< OutlinerParaObject > pOPO( pTextApi->CreateText() );
            Engine()->SetText( *pOPO.get() );
        }

        Engine()->ClearModifyFlag();
        Engine()->GetUndoManager().Clear();

        Invalidate();

        OUString sMeta( xAnnotation->getAuthor() );
        OUString sDateTime( getAnnotationDateTimeString(xAnnotation) );

        if( !sDateTime.isEmpty() )
        {
            if( !sMeta.isEmpty() )
                sMeta += "\n";

           sMeta += sDateTime;
        }
        mpMeta->SetText(sMeta);

        if( bGrabFocus )
            GrabFocus();
    }
}

void AnnotationWindow::SetColor()
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

    mpOutlinerView->SetBackgroundColor(maColor);
    Engine()->SetBackgroundColor(maColor);

    {
        SvtAccessibilityOptions aOptions;
        Engine()->ForceAutoColor( bHighContrast || aOptions.GetIsAutomaticFontColor() );
    }

    mpMeta->SetControlBackground(maColor);
    AllSettings aSettings = mpMeta->GetSettings();
    StyleSettings aStyleSettings = aSettings.GetStyleSettings();
    aStyleSettings.SetFieldTextColor( bHighContrast ? maColorLight : maColorDark);
    aSettings.SetStyleSettings(aStyleSettings);
    mpMeta->SetSettings(aSettings);

    AllSettings aSettings2 = mpVScrollbar->GetSettings();
    StyleSettings aStyleSettings2 = aSettings2.GetStyleSettings();
    aStyleSettings2.SetButtonTextColor(Color(0,0,0));
    aStyleSettings2.SetCheckedColor(maColorLight); //hintergund
    aStyleSettings2.SetShadowColor(maColorDark);
    aStyleSettings2.SetFaceColor(maColor);
    aSettings2.SetStyleSettings(aStyleSettings2);
    mpVScrollbar->SetSettings(aSettings2);
}

void AnnotationWindow::Deactivate()
{
    Reference< XAnnotation > xAnnotation( mxAnnotation );

    // write changed text back to annotation
    if ( Engine()->IsModified() )
    {
        TextApiObject* pTextApi = getTextApiObject( xAnnotation );

        if( pTextApi )
        {
            OutlinerParaObject* pOPO = Engine()->CreateParaObject();
            if( pOPO )
            {
                if( mpDoc->IsUndoEnabled() )
                    mpDoc->BegUndo( SD_RESSTR( STR_ANNOTATION_UNDO_EDIT ) );

                pTextApi->SetText( *pOPO );
                delete pOPO;

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

void AnnotationWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    FloatingWindow::Paint(rRenderContext, rRect);

    if(mpMeta->IsVisible() && !mbReadonly)
    {
        const bool bHighContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
        //draw left over space
        if ( bHighContrast )
            SetFillColor(COL_BLACK);
        else
            SetFillColor(maColor);
        SetLineColor();
        DrawRect(PixelToLogic(Rectangle(Point(mpMeta->GetPosPixel().X()+mpMeta->GetSizePixel().Width(),mpMeta->GetPosPixel().Y()),Size(METABUTTON_AREA_WIDTH,mpMeta->GetSizePixel().Height()))));

        if ( bHighContrast )
        {
            //draw rect around button
            SetFillColor(COL_BLACK);
            SetLineColor(COL_WHITE);
        }
        else
        {
            //draw button
            Gradient aGradient;
            if (mbMouseOverButton)
                aGradient = Gradient(GradientStyle_LINEAR,ColorFromAlphaColor(80,maColorDark,maColor),ColorFromAlphaColor(15,maColorDark,maColor));
            else
                aGradient = Gradient(GradientStyle_LINEAR,ColorFromAlphaColor(15,maColorDark,maColor),ColorFromAlphaColor(80,maColorDark,maColor));
            DrawGradient(maRectMetaButton,aGradient);
            //draw rect around button
            SetFillColor();
            SetLineColor(ColorFromAlphaColor(90,maColorDark,maColor));
        }
        DrawRect(maRectMetaButton);

        //draw arrow
        if( bHighContrast )
            SetFillColor(COL_WHITE);
        else
            SetFillColor(COL_BLACK);
        SetLineColor();
        DrawPolygon( ::tools::Polygon(maPopupTriangle));
    }
}

void AnnotationWindow::MouseMove( const MouseEvent& rMEvt )
{
    if( !mbReadonly )
    {
        if (maRectMetaButton.IsInside(PixelToLogic(rMEvt.GetPosPixel())))
        {
            if (!mbMouseOverButton)
            {
                Invalidate(maRectMetaButton);
                mbMouseOverButton = true;
            }
        }
        else
        {
            if (mbMouseOverButton)
            {
                Invalidate(maRectMetaButton);
                mbMouseOverButton = false;
            }
        }
    }
}

void AnnotationWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!mbReadonly && maRectMetaButton.IsInside(PixelToLogic(rMEvt.GetPosPixel())) && rMEvt.IsLeft())
    {
        // context menu
        Rectangle aRect(LogicToPixel(maRectMetaButton.BottomLeft()),LogicToPixel(maRectMetaButton.BottomLeft()));
        mrManager.ExecuteAnnotationContextMenu( mxAnnotation, static_cast<vcl::Window*>(this), aRect, true );
    }
}

void AnnotationWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        if( mpMeta->IsVisible() &&(mpMeta->GetPosPixel().Y() < rCEvt.GetMousePosPixel().Y()) )
            return;
        mrManager.ExecuteAnnotationContextMenu( mxAnnotation, this, Rectangle(rCEvt.GetMousePosPixel(),Size(1,1)) );
    }
    else
    {
        FloatingWindow::Command(rCEvt);
    }
}

void AnnotationWindow::GetFocus()
{
    if( mpTextWindow )
        mpTextWindow->GrabFocus();
    else
        FloatingWindow::GetFocus();
}

void AnnotationWindow::ExecuteSlot( sal_uInt16 nSID )
{
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
            FontWeight eFW = static_cast<const SvxWeightItem&>( aEditAttr.Get( EE_CHAR_WEIGHT ) ).GetWeight();
            aNewAttr.Put( SvxWeightItem( eFW == WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        }
        break;
        case SID_ATTR_CHAR_POSTURE:
        {
            FontItalic eFI = static_cast<const SvxPostureItem&>( aEditAttr.Get( EE_CHAR_ITALIC ) ).GetPosture();
            aNewAttr.Put( SvxPostureItem( eFI == ITALIC_NORMAL ? ITALIC_NONE : ITALIC_NORMAL, EE_CHAR_ITALIC ) );
        }
        break;
        case SID_ATTR_CHAR_UNDERLINE:
        {
            FontUnderline eFU = static_cast<const SvxUnderlineItem&>( aEditAttr. Get( EE_CHAR_UNDERLINE ) ).GetLineStyle();
            aNewAttr.Put( SvxUnderlineItem( eFU == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, EE_CHAR_UNDERLINE ) );
        }
        break;
        case SID_ATTR_CHAR_STRIKEOUT:
        {
            FontStrikeout eFSO = static_cast<const SvxCrossedOutItem&>( aEditAttr.Get( EE_CHAR_STRIKEOUT ) ).GetStrikeout();
            aNewAttr.Put( SvxCrossedOutItem( eFSO == STRIKEOUT_SINGLE ? STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT ) );
        }
        break;
        }
        getView()->SetAttribs( aNewAttr );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
