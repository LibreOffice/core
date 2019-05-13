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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxdlg.hxx>
#include <vcl/cursor.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <tphfedit.hxx>
#include <editutil.hxx>
#include <global.hxx>
#include <patattr.hxx>
#include <scresid.hxx>
#include <globstr.hrc>
#include <strings.hrc>
#include <tabvwsh.hxx>
#include <prevwsh.hxx>
#include <AccessibleEditObject.hxx>

#include <scabstdlg.hxx>
#include <memory>


static void lcl_GetFieldData( ScHeaderFieldData& rData )
{
    SfxViewShell* pShell = SfxViewShell::Current();
    if (pShell)
    {
        if (dynamic_cast<const ScTabViewShell*>( pShell) !=  nullptr)
            static_cast<ScTabViewShell*>(pShell)->FillFieldData(rData);
        else if (dynamic_cast<const ScPreviewShell*>( pShell) !=  nullptr)
            static_cast<ScPreviewShell*>(pShell)->FillFieldData(rData);
    }
}

// class ScEditWindow

ScEditWindow::ScEditWindow(ScEditWindowLocation eLoc, weld::Window* pDialog)
    : eLocation(eLoc)
    , mbRTL(ScGlobal::IsSystemRTL())
    , mpDialog(pDialog)
    , pAcc(nullptr)
{
}

void ScEditWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    OutputDevice& rDevice = pDrawingArea->get_ref_device();

    Size aSize = rDevice.LogicToPixel(Size(80, 120), MapMode(MapUnit::MapAppFont));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    SetOutputSizePixel(aSize);

    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);

    EnableRTL(false);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    rDevice.SetMapMode(MapMode(MapUnit::MapTwip));
    rDevice.SetBackground(aBgColor);

    Size aOutputSize(rDevice.PixelToLogic(aSize));
    aSize = aOutputSize;
    aSize.setHeight( aSize.Height() * 4 );

    pEdEngine.reset( new ScHeaderEditEngine( EditEngine::CreatePool() ) );
    pEdEngine->SetPaperSize( aSize );
    pEdEngine->SetRefDevice( &rDevice );

    ScHeaderFieldData aData;
    lcl_GetFieldData( aData );

    // fields
    pEdEngine->SetData( aData );
    pEdEngine->SetControlWord( pEdEngine->GetControlWord() | EEControlBits::MARKFIELDS );
    if (mbRTL)
        pEdEngine->SetDefaultHorizontalTextDirection(EEHorizontalTextDirection::R2L);

    pEdView.reset(new EditView(pEdEngine.get(), nullptr));
    pEdView->setEditViewCallbacks(this);
    pEdView->SetOutputArea(tools::Rectangle(Point(0,0), aOutputSize));

    pEdView->SetBackgroundColor( aBgColor );
    pEdEngine->InsertView( pEdView.get() );

    pDrawingArea->set_cursor(PointerStyle::Text);

    if (pAcc)
    {
        OUString sName;
        switch (eLocation)
        {
            case Left:
                sName = ScResId(STR_ACC_LEFTAREA_NAME);
                break;
            case Center:
                sName = ScResId(STR_ACC_CENTERAREA_NAME);
                break;
            case Right:
                sName = ScResId(STR_ACC_RIGHTAREA_NAME);
                break;
        }

        pAcc->InitAcc(nullptr, pEdView.get(), nullptr,
                      sName, pDrawingArea->get_tooltip_text());
    }
}

void ScEditWindow::Resize()
{
    OutputDevice& rDevice = GetDrawingArea()->get_ref_device();
    Size aOutputSize(rDevice.PixelToLogic(GetOutputSizePixel()));
    Size aSize(aOutputSize);
    aSize.setHeight( aSize.Height() * 4 );
    pEdEngine->SetPaperSize(aSize);
    pEdView->SetOutputArea(tools::Rectangle(Point(0,0), aOutputSize));
    weld::CustomWidgetController::Resize();
}

ScEditWindow::~ScEditWindow()
{
    // delete Accessible object before deleting EditEngine and EditView
    if (pAcc)
    {
        css::uno::Reference< css::accessibility::XAccessible > xTemp = xAcc;
        if (xTemp.is())
            pAcc->dispose();
    }
    pEdEngine.reset();
    pEdView.reset();
}

void ScEditWindow::SetNumType(SvxNumType eNumType)
{
    pEdEngine->SetNumType(eNumType);
    pEdEngine->UpdateFields();
}

std::unique_ptr<EditTextObject> ScEditWindow::CreateTextObject()
{
    //  reset paragraph attributes
    //  (GetAttribs at creation of format dialog always returns the set items)

    const SfxItemSet& rEmpty = pEdEngine->GetEmptyItemSet();
    sal_Int32 nParCnt = pEdEngine->GetParagraphCount();
    for (sal_Int32 i=0; i<nParCnt; i++)
        pEdEngine->SetParaAttribs( i, rEmpty );

    return pEdEngine->CreateTextObject();
}

void ScEditWindow::SetFont( const ScPatternAttr& rPattern )
{
    auto pSet = std::make_unique<SfxItemSet>( pEdEngine->GetEmptyItemSet() );
    rPattern.FillEditItemSet( pSet.get() );
    //  FillEditItemSet adjusts font height to 1/100th mm,
    //  but for header/footer twips is needed, as in the PatternAttr:
    pSet->Put( rPattern.GetItem(ATTR_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT) );
    pSet->Put( rPattern.GetItem(ATTR_CJK_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CJK) );
    pSet->Put( rPattern.GetItem(ATTR_CTL_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CTL) );
    if (mbRTL)
        pSet->Put( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );
    pEdEngine->SetDefaults( std::move(pSet) );
}

void ScEditWindow::SetText( const EditTextObject& rTextObject )
{
    pEdEngine->SetText( rTextObject );
}

void ScEditWindow::InsertField( const SvxFieldItem& rFld )
{
    pEdView->InsertField( rFld );
}

void ScEditWindow::SetCharAttributes()
{
    SfxObjectShell* pDocSh  = SfxObjectShell::Current();

    SfxViewShell*       pViewSh = SfxViewShell::Current();

    ScTabViewShell* pTabViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() );

    OSL_ENSURE( pDocSh,  "Current DocShell not found" );
    OSL_ENSURE( pViewSh, "Current ViewShell not found" );

    if ( pDocSh && pViewSh )
    {
        if(pTabViewSh!=nullptr) pTabViewSh->SetInFormatDialog(true);

        SfxItemSet aSet( pEdView->GetAttribs() );

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

        ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScCharDlg(
            mpDialog,  &aSet, pDocSh));
        pDlg->SetText( ScResId( STR_TEXTATTRS ) );
        if ( pDlg->Execute() == RET_OK )
        {
            aSet.ClearItem();
            aSet.Put( *pDlg->GetOutputItemSet() );
            pEdView->SetAttribs( aSet );
        }

        if(pTabViewSh!=nullptr) pTabViewSh->SetInFormatDialog(false);
    }
}

void ScEditWindow::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect )
{
    //note: ClassificationEditView::Paint is similar

    rRenderContext.Push(PushFlags::ALL);
    rRenderContext.SetClipRegion();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    pEdView->SetBackgroundColor( aBgColor );

    rRenderContext.SetBackground( aBgColor );

    tools::Rectangle aLogicRect(rRenderContext.PixelToLogic(rRect));
    pEdView->Paint(aLogicRect, &rRenderContext);

    if (HasFocus())
    {
        pEdView->ShowCursor();
        vcl::Cursor* pCursor = pEdView->GetCursor();
        pCursor->DrawToDevice(rRenderContext);
    }

    std::vector<tools::Rectangle> aLogicRects;

    // get logic selection
    pEdView->GetSelectionRectangles(aLogicRects);

    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(COL_BLACK);
    rRenderContext.SetRasterOp(RasterOp::Invert);

    for (const auto &rSelectionRect : aLogicRects)
        rRenderContext.DrawRect(rSelectionRect);

    rRenderContext.Pop();
}

bool ScEditWindow::MouseMove( const MouseEvent& rMEvt )
{
    return pEdView->MouseMove( rMEvt );
}

bool ScEditWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !HasFocus() )
        GrabFocus();

    return pEdView->MouseButtonDown( rMEvt );
}

bool ScEditWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    return pEdView->MouseButtonUp( rMEvt );
}

bool ScEditWindow::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nKey =  rKEvt.GetKeyCode().GetModifier()
                 + rKEvt.GetKeyCode().GetCode();

    if ( nKey == KEY_TAB || nKey == KEY_TAB + KEY_SHIFT )
    {
        return false;
    }
    else if ( !pEdView->PostKeyEvent( rKEvt ) )
    {
        return false;
    }
    else if ( !rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsShift() &&
                rKEvt.GetKeyCode().IsMod2() && rKEvt.GetKeyCode().GetCode() == KEY_DOWN )
    {
        aObjectSelectLink.Call(*this);
        return true;
    }
    return false;
}

void ScEditWindow::GetFocus()
{
    pEdView->ShowCursor();

    assert(m_GetFocusLink);
    m_GetFocusLink(*this);

    css::uno::Reference< css::accessibility::XAccessible > xTemp = xAcc;
    if (xTemp.is() && pAcc)
    {
        pAcc->GotFocus();
    }
    else
        pAcc = nullptr;

    weld::CustomWidgetController::GetFocus();
}

void ScEditWindow::LoseFocus()
{
    css::uno::Reference< css::accessibility::XAccessible > xTemp = xAcc;
    if (xTemp.is() && pAcc)
    {
        pAcc->LostFocus();
    }
    else
        pAcc = nullptr;
    weld::CustomWidgetController::LoseFocus();
    Invalidate(); // redraw without cursor
}

css::uno::Reference< css::accessibility::XAccessible > ScEditWindow::CreateAccessible()
{
    pAcc = new ScAccessibleEditControlObject(this);
    css::uno::Reference< css::accessibility::XAccessible > xAccessible = pAcc;
    xAcc = xAccessible;
    return pAcc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
