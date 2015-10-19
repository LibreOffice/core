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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <editeng/adjustitem.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include "tphfedit.hxx"
#include "editutil.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "prevwsh.hxx"
#include "AccessibleEditObject.hxx"

#include "scabstdlg.hxx"
#include <memory>

// STATIC DATA -----------------------------------------------------------
static VclPtr<ScEditWindow> pActiveEdWnd = NULL;

ScEditWindow* GetScEditWindow ()
{
    return pActiveEdWnd;
}

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

ScEditWindow::ScEditWindow( vcl::Window* pParent, WinBits nBits, ScEditWindowLocation eLoc )
    :   Control( pParent, nBits ),
    eLocation(eLoc),
    pAcc(NULL)
{
    EnableRTL(false);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    SetMapMode( MAP_TWIP );
    SetPointer( PointerStyle::Text );
    SetBackground( aBgColor );

    Size aSize( GetOutputSize() );
    aSize.Height() *= 4;

    pEdEngine = new ScHeaderEditEngine( EditEngine::CreatePool(), true );
    pEdEngine->SetPaperSize( aSize );
    pEdEngine->SetRefDevice( this );

    ScHeaderFieldData aData;
    lcl_GetFieldData( aData );

        //  Feldbefehle:
    pEdEngine->SetData( aData );
    pEdEngine->SetControlWord( pEdEngine->GetControlWord() | EEControlBits::MARKFIELDS );
    mbRTL = ScGlobal::IsSystemRTL();
    if (mbRTL)
        pEdEngine->SetDefaultHorizontalTextDirection(EE_HTEXTDIR_R2L);

    pEdView = new EditView( pEdEngine, this );
    pEdView->SetOutputArea( Rectangle( Point(0,0), GetOutputSize() ) );

    pEdView->SetBackgroundColor( aBgColor );
    pEdEngine->InsertView( pEdView );
}

void ScEditWindow::Resize()
{
    Size aOutputSize(GetOutputSize());
    Size aSize(aOutputSize);
    aSize.Height() *= 4;
    pEdEngine->SetPaperSize(aSize);
    pEdView->SetOutputArea(Rectangle(Point(0,0), aOutputSize));
    Control::Resize();
}

ScEditWindow::~ScEditWindow()
{
    disposeOnce();
}

void ScEditWindow::dispose()
{
    // delete Accessible object before deleting EditEngine and EditView
    if (pAcc)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xTemp = xAcc;
        if (xTemp.is())
            pAcc->dispose();
    }
    delete pEdEngine;
    delete pEdView;
    Control::dispose();
}

VCL_BUILDER_DECL_FACTORY(ScEditWindow)
{
    (void)rMap;
    rRet = VclPtr<ScEditWindow>::Create(pParent, WB_BORDER|WB_TABSTOP, Left);
}

void ScEditWindow::SetNumType(SvxNumType eNumType)
{
    pEdEngine->SetNumType(eNumType);
    pEdEngine->UpdateFields();
}

EditTextObject* ScEditWindow::CreateTextObject()
{
    //  Absatzattribute zuruecksetzen
    //  (GetAttribs beim Format-Dialog-Aufruf gibt immer gesetzte Items zurueck)

    const SfxItemSet& rEmpty = pEdEngine->GetEmptyItemSet();
    sal_Int32 nParCnt = pEdEngine->GetParagraphCount();
    for (sal_Int32 i=0; i<nParCnt; i++)
        pEdEngine->SetParaAttribs( i, rEmpty );

    return pEdEngine->CreateTextObject();
}

void ScEditWindow::SetFont( const ScPatternAttr& rPattern )
{
    SfxItemSet* pSet = new SfxItemSet( pEdEngine->GetEmptyItemSet() );
    rPattern.FillEditItemSet( pSet );
    //  FillEditItemSet adjusts font height to 1/100th mm,
    //  but for header/footer twips is needed, as in the PatternAttr:
    pSet->Put( rPattern.GetItem(ATTR_FONT_HEIGHT), EE_CHAR_FONTHEIGHT );
    pSet->Put( rPattern.GetItem(ATTR_CJK_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CJK );
    pSet->Put( rPattern.GetItem(ATTR_CTL_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CTL );
    if (mbRTL)
        pSet->Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
    pEdEngine->SetDefaults( pSet );
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
        if(pTabViewSh!=NULL) pTabViewSh->SetInFormatDialog(true);

        SfxItemSet aSet( pEdView->GetAttribs() );

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

        std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateScCharDlg(
            GetParent(),  &aSet, pDocSh));
        OSL_ENSURE(pDlg, "Dialog create fail!");
        pDlg->SetText( ScGlobal::GetRscString( STR_TEXTATTRS ) );
        if ( pDlg->Execute() == RET_OK )
        {
            aSet.ClearItem();
            aSet.Put( *pDlg->GetOutputItemSet() );
            pEdView->SetAttribs( aSet );
        }

        if(pTabViewSh!=NULL) pTabViewSh->SetInFormatDialog(false);
    }
}

void ScEditWindow::Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    pEdView->SetBackgroundColor( aBgColor );

    SetBackground( aBgColor );

    Control::Paint(rRenderContext, rRect);

    pEdView->Paint(rRect);

    if( HasFocus() )
        pEdView->ShowCursor();
}

void ScEditWindow::MouseMove( const MouseEvent& rMEvt )
{
    pEdView->MouseMove( rMEvt );
}

void ScEditWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !HasFocus() )
        GrabFocus();

    pEdView->MouseButtonDown( rMEvt );
}

void ScEditWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    pEdView->MouseButtonUp( rMEvt );
}

void ScEditWindow::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nKey =  rKEvt.GetKeyCode().GetModifier()
                 + rKEvt.GetKeyCode().GetCode();

    if ( nKey == KEY_TAB || nKey == KEY_TAB + KEY_SHIFT )
    {
        Control::KeyInput( rKEvt );
    }
    else if ( !pEdView->PostKeyEvent( rKEvt ) )
    {
        Control::KeyInput( rKEvt );
    }
    else if ( !rKEvt.GetKeyCode().IsMod1() && !rKEvt.GetKeyCode().IsShift() &&
                rKEvt.GetKeyCode().IsMod2() && rKEvt.GetKeyCode().GetCode() == KEY_DOWN )
    {
        aObjectSelectLink.Call(*this);
    }
}

void ScEditWindow::Command( const CommandEvent& rCEvt )
{
    pEdView->Command( rCEvt );
}

void ScEditWindow::GetFocus()
{
    pEdView->ShowCursor();
    pActiveEdWnd = this;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xTemp = xAcc;
    if (xTemp.is() && pAcc)
    {
        pAcc->GotFocus();
    }
    else
        pAcc = NULL;

    Control::GetFocus();
}

void ScEditWindow::LoseFocus()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xTemp = xAcc;
    if (xTemp.is() && pAcc)
    {
        pAcc->LostFocus();
    }
    else
        pAcc = NULL;
    Control::LoseFocus();
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ScEditWindow::CreateAccessible()
{
    OUString sName;
    OUString sDescription(GetHelpText());
    switch (eLocation)
    {
    case Left:
        {
            sName = OUString(ScResId(STR_ACC_LEFTAREA_NAME));
        }
        break;
    case Center:
        {
            sName = OUString(ScResId(STR_ACC_CENTERAREA_NAME));
        }
        break;
    case Right:
        {
            sName = OUString(ScResId(STR_ACC_RIGHTAREA_NAME));
        }
        break;
    }
    pAcc = new ScAccessibleEditObject(GetAccessibleParentWindow()->GetAccessible(), pEdView, this,
        OUString(sName), OUString(sDescription), ScAccessibleEditObject::EditControl);
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccessible = pAcc;
    xAcc = xAccessible;
    return pAcc;
}

ScExtIButton::ScExtIButton(vcl::Window* pParent, WinBits nBits )
    : ImageButton(pParent,nBits), pPopupMenu(NULL)
{
    nSelected=0;
    aIdle.SetPriority(SchedulerPriority::LOWEST);
    SetDropDown(PushButtonDropdownStyle::Toolbox);
}

VCL_BUILDER_FACTORY_ARGS(ScExtIButton, 0 /* WB_BORDER|WB_TABSTOP */)

void ScExtIButton::SetPopupMenu(PopupMenu* pPopUp)
{
    pPopupMenu=pPopUp;
}

void ScExtIButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if(!aIdle.IsActive())
    {
        aIdle.SetIdleHdl(LINK( this, ScExtIButton, TimerHdl));
        aIdle.Start();
    }

    ImageButton::MouseButtonDown(rMEvt );
}

void ScExtIButton::MouseButtonUp( const MouseEvent& rMEvt)
{
    aIdle.Stop();
    aIdle.SetIdleHdl(Link<Idle *, void>());
    ImageButton::MouseButtonUp(rMEvt );
}

void ScExtIButton::Click()
{
    aIdle.Stop();
    aIdle.SetIdleHdl(Link<Idle *, void>());
    ImageButton::Click();
}

void ScExtIButton::StartPopup()
{
    nSelected=0;
    aSelectedIdent.clear();

    if(pPopupMenu!=NULL)
    {
        SetPressed( true );
        EndSelection();
        Point aPoint(0,0);
        aPoint.Y()=GetOutputSizePixel().Height();

        nSelected = pPopupMenu->Execute( this, aPoint );

        if(nSelected)
        {
            aSelectedIdent = pPopupMenu->GetItemIdent(nSelected);
            aMLink.Call(*this);
        }

        SetPressed( false);
    }
}

bool ScExtIButton::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nSwitch=rNEvt.GetType();
    if(nSwitch==MouseNotifyEvent::MOUSEBUTTONUP)
    {
        MouseButtonUp(*rNEvt.GetMouseEvent());
    }

    return ImageButton::PreNotify(rNEvt );
}

IMPL_LINK_NOARG_TYPED(ScExtIButton, TimerHdl, Idle *, void)
{
    StartPopup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
