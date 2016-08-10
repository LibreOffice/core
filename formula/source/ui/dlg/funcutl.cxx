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

#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/fixed.hxx>

#include "formula/funcutl.hxx"
#include "formula/IControlReferenceHandler.hxx"
#include "ControlHelper.hxx"
#include "ModuleHelper.hxx"
#include "ForResId.hrc"
#include "com/sun/star/accessibility/AccessibleRole.hpp"


namespace formula
{

// class ArgEdit
ArgEdit::ArgEdit( vcl::Window* pParent, WinBits nBits )
    :   RefEdit( pParent, nullptr, nBits ),
        pEdPrev ( nullptr ),
        pEdNext ( nullptr ),
        pSlider ( nullptr ),
        nArgs   ( 0 )
{
}

ArgEdit::~ArgEdit()
{
    disposeOnce();
}

void ArgEdit::dispose()
{
    pEdPrev.clear();
    pEdNext.clear();
    pSlider.clear();
    RefEdit::dispose();
}

VCL_BUILDER_FACTORY_ARGS(ArgEdit, WB_BORDER)

void ArgEdit::Init( ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                    ScrollBar& rArgSlider, sal_uInt16 nArgCount )
{
    pEdPrev = pPrevEdit;
    pEdNext = pNextEdit;
    pSlider = &rArgSlider;
    nArgs   = nArgCount;
}

// Cursor control for Edit Fields in Argument Dialog
void ArgEdit::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    bool bUp = (aCode.GetCode() == KEY_UP);
    bool bDown = (aCode.GetCode() == KEY_DOWN);

    if (   pSlider
        && ( !aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
        && ( bUp || bDown ) )
    {
        if ( nArgs > 1 )
        {
            ArgEdit* pEd = nullptr;
            long nThumb = pSlider->GetThumbPos();
            bool bDoScroll = false;
            bool bChangeFocus = false;

            if ( bDown )
            {
                if ( nArgs > 4 )
                {
                    if ( !pEdNext )
                    {
                        nThumb++;
                        bDoScroll = ( nThumb+3 < (long)nArgs );
                    }
                    else
                    {
                        pEd = pEdNext;
                        bChangeFocus = true;
                    }
                }
                else if ( pEdNext )
                {
                    pEd = pEdNext;
                    bChangeFocus = true;
                }
            }
            else // if ( bUp )
            {
                if ( nArgs > 4 )
                {
                    if ( !pEdPrev )
                    {
                        nThumb--;
                        bDoScroll = ( nThumb >= 0 );
                    }
                    else
                    {
                        pEd = pEdPrev;
                        bChangeFocus = true;
                    }
                }
                else if ( pEdPrev )
                {
                    pEd = pEdPrev;
                    bChangeFocus = true;
                }
            }

            if ( bDoScroll )
            {
                pSlider->SetThumbPos( nThumb );
                pSlider->GetEndScrollHdl().Call( pSlider );
            }
            else if ( bChangeFocus )
            {
                pEd->GrabFocus();
            }
        }
    }
    else
        RefEdit::KeyInput( rKEvt );
}

// class ArgInput
ArgInput::ArgInput()
{
    pFtArg=nullptr;
    pBtnFx=nullptr;
    pEdArg=nullptr;
    pRefBtn=nullptr;
}

void ArgInput::InitArgInput( FixedText* pftArg, PushButton* pbtnFx,
                             ArgEdit* pedArg, RefButton* prefBtn)
{
    pFtArg =pftArg;
    pBtnFx =pbtnFx;
    pEdArg =pedArg;
    pRefBtn=prefBtn;

    if(pBtnFx!=nullptr)
    {
        pBtnFx->SetClickHdl   ( LINK( this, ArgInput, FxBtnClickHdl ) );
        pBtnFx->SetGetFocusHdl( LINK( this, ArgInput, FxBtnFocusHdl ) );
    }
    if(pEdArg!=nullptr)
    {
        pEdArg->SetGetFocusHdl ( LINK( this, ArgInput, EdFocusHdl ) );
        pEdArg->SetModifyHdl   ( LINK( this, ArgInput, EdModifyHdl ) );
    }

}

// Sets the Name for the Argument
void ArgInput::SetArgName(const OUString &aArg)
{
    if(pFtArg !=nullptr) pFtArg->SetText(aArg );
}

// Returns the Name for the Argument
OUString ArgInput::GetArgName()
{
    OUString aPrivArgName;
    if(pFtArg !=nullptr)
        aPrivArgName=pFtArg->GetText();

    return aPrivArgName;
}

//Sets the Name for the Argument
void ArgInput::SetArgNameFont   (const vcl::Font &aFont)
{
    if(pFtArg !=nullptr) pFtArg->SetFont(aFont);
}

//Sets up the Selection for the EditBox.
void ArgInput::SetArgSelection  (const Selection& rSel )
{
    if(pEdArg !=nullptr) pEdArg ->SetSelection(rSel );
}

//Sets the Value for the Argument
void ArgInput::SetArgVal(const OUString &rVal)
{
    if(pEdArg != nullptr)
    {
        pEdArg ->SetRefString(rVal);
    }
}

//Returns the Value for the Argument
OUString ArgInput::GetArgVal()
{
    OUString aResult;
    if(pEdArg!=nullptr)
    {
        aResult=pEdArg->GetText();
    }
    return aResult;
}

//Hides the Controls
void ArgInput::Hide()
{
    if ( pFtArg && pBtnFx && pEdArg && pRefBtn)
    {
        pFtArg->Hide();
        pBtnFx->Hide();
        pEdArg->Hide();
        pRefBtn->Hide();
    }
}

//Casts the Controls again.
void ArgInput::Show()
{
    if ( pFtArg && pBtnFx && pEdArg && pRefBtn)
    {
        pFtArg->Show();
        pBtnFx->Show();
        pEdArg->Show();
        pRefBtn->Show();
    }
}

void ArgInput::UpdateAccessibleNames()
{
    OUString aArgName(":");
    aArgName += pFtArg->GetText();

    OUString aName = pBtnFx->GetQuickHelpText();
    aName += aArgName;
    pBtnFx->SetAccessibleName(aName);

    aName = pRefBtn->GetQuickHelpText();
    aName += aArgName;
    pRefBtn->SetAccessibleName(aName);
}

void ArgInput::FxClick()
{
    aFxClickLink.Call(*this);
}

void ArgInput::FxFocus()
{
    aFxFocusLink.Call(*this);
}

void ArgInput::EdFocus()
{
    aEdFocusLink.Call(*this);
}

void ArgInput::EdModify()
{
    aEdModifyLink.Call(*this);
}

IMPL_LINK_TYPED( ArgInput, FxBtnClickHdl, Button*, pBtn, void )
{
    if(pBtn == pBtnFx)
        FxClick();
}

IMPL_LINK_TYPED( ArgInput, FxBtnFocusHdl, Control&, rControl, void )
{
    if(&rControl == pBtnFx)
        FxFocus();
}

IMPL_LINK_TYPED( ArgInput, EdFocusHdl, Control&, rControl, void )
{
    if(&rControl == pEdArg)
        EdFocus();
}

IMPL_LINK_TYPED( ArgInput, EdModifyHdl, Edit&, rEdit, void )
{
    if(&rEdit == pEdArg)
        EdModify();
}

// class EditBox

EditBox::EditBox( vcl::Window* pParent, WinBits nBits )
        :Control(pParent,nBits),
        bMouseFlag(false)
{
    WinBits nStyle=GetStyle();
    SetStyle( nStyle| WB_DIALOGCONTROL);

    pMEdit=VclPtr<MultiLineEdit>::Create(this,WB_LEFT | WB_VSCROLL | (nStyle & WB_TABSTOP) |
                    WB_NOBORDER | WB_NOHIDESELECTION | WB_IGNORETAB);
    pMEdit->Show();
    aOldSel=pMEdit->GetSelection();
    Resize();
    WinBits nWinStyle=GetStyle() | WB_DIALOGCONTROL;
    SetStyle(nWinStyle);

    //  #105582# the HelpId from the resource must be set for the MultiLineEdit,
    //  not for the control that contains it.
    pMEdit->SetHelpId( GetHelpId() );
    SetHelpId( "" );
}

VCL_BUILDER_FACTORY_ARGS(EditBox, WB_BORDER)

EditBox::~EditBox()
{
    disposeOnce();
}

void EditBox::dispose()
{
    pMEdit->Disable();
    pMEdit.disposeAndClear();
    Control::dispose();
}

// When the selection is changed this function will be called
void EditBox::SelectionChanged()
{
    aSelChangedLink.Call(*this);
}

// When the size is changed, MultiLineEdit must be adapted..
void EditBox::Resize()
{
    Size aSize=GetOutputSizePixel();
    if(pMEdit!=nullptr) pMEdit->SetOutputSizePixel(aSize);
}

// When the Control is activated, the Selection is repealed
// and the Cursor set at the end.
void EditBox::GetFocus()
{
    if(pMEdit!=nullptr)
    {
        pMEdit->GrabFocus();
    }
}

//When an Event is cleared, this Routine is
//first called and a PostUserEvent is sent.
bool EditBox::PreNotify( NotifyEvent& rNEvt )
{
    bool bResult = true;

    if(pMEdit==nullptr) return bResult;

    MouseNotifyEvent nSwitch=rNEvt.GetType();
    if(nSwitch==MouseNotifyEvent::KEYINPUT)// || nSwitch==MouseNotifyEvent::KEYUP)
    {
        const vcl::KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey=aKeyCode.GetCode();
        if( (nKey==KEY_RETURN && !aKeyCode.IsShift()) || nKey==KEY_TAB )
        {
            bResult = GetParent()->Notify(rNEvt);
        }
        else
        {
            bResult=Control::PreNotify(rNEvt);
            Application::PostUserEvent( LINK( this, EditBox, ChangedHdl ), nullptr, true );
        }

    }
    else
    {
        bResult=Control::PreNotify(rNEvt);

        if(nSwitch==MouseNotifyEvent::MOUSEBUTTONDOWN || nSwitch==MouseNotifyEvent::MOUSEBUTTONUP)
        {
            bMouseFlag=true;
            Application::PostUserEvent( LINK( this, EditBox, ChangedHdl ), nullptr, true );
        }
    }
    return bResult;
}

//When an Event cleared wurde, this routine is
//first called.
IMPL_LINK_NOARG_TYPED(EditBox, ChangedHdl, void*, void)
{
    if(pMEdit!=nullptr)
    {
        Selection aNewSel=pMEdit->GetSelection();

        if(aNewSel.Min()!=aOldSel.Min() || aNewSel.Max()!=aOldSel.Max())
        {
            SelectionChanged();
            aOldSel=aNewSel;
        }
    }
}

void EditBox::UpdateOldSel()
{
    //  if selection is set for editing a function, store it as aOldSel,
    //  so SelectionChanged isn't called in the next ChangedHdl call

    if (pMEdit)
        aOldSel = pMEdit->GetSelection();
}

// class RefEdit

RefEdit::RefEdit( vcl::Window* _pParent, vcl::Window* pShrinkModeLabel, WinBits nStyle )
    : Edit( _pParent, nStyle )
    , aIdle("formula RefEdit Idle")
    , pAnyRefDlg( nullptr )
    , pLabelWidget(pShrinkModeLabel)
{
    aIdle.SetIdleHdl( LINK( this, RefEdit, UpdateHdl ) );
}

RefEdit::RefEdit( vcl::Window* _pParent,IControlReferenceHandler* pParent,
    vcl::Window* pShrinkModeLabel, const ResId& rResId )
    : Edit( _pParent, rResId )
    , aIdle("formula RefEdit Idle")
    , pAnyRefDlg( pParent )
    , pLabelWidget(pShrinkModeLabel)
{
    aIdle.SetIdleHdl( LINK( this, RefEdit, UpdateHdl ) );
}

VCL_BUILDER_DECL_FACTORY(RefEdit)
{
    (void)rMap;
    rRet = VclPtr<RefEdit>::Create(pParent, nullptr, WB_BORDER);
}

RefEdit::~RefEdit()
{
    disposeOnce();
}

void RefEdit::dispose()
{
    aIdle.SetIdleHdl( Link<Idle *, void>() );
    aIdle.Stop();
    pLabelWidget.clear();
    Edit::dispose();
}

void RefEdit::SetRefString( const OUString& rStr )
{
    // Prevent unwanted side effects by setting only a differing string.
    // See commit message for reasons.
    if (Edit::GetText() != rStr)
        Edit::SetText( rStr );
}

void RefEdit::SetRefValid(bool bValid)
{
    if (bValid)
    {
        SetControlForeground();
        SetControlBackground();
    }
    else
    {
#if 0
        // Setting background color has no effect here so we'd end up with
        // white on white!
        SetControlForeground(COL_WHITE);
        SetControlBackground(0xff6563);
#else
        SetControlForeground( ::Color( RGB_COLORDATA( 0xf0, 0, 0)));
#endif
    }
}

void RefEdit::SetText(const OUString& rStr)
{
    Edit::SetText( rStr );
    UpdateHdl( &aIdle );
}

void RefEdit::StartUpdateData()
{
    aIdle.Start();
}

void RefEdit::SetReferences( IControlReferenceHandler* pDlg, vcl::Window* pLabel )
{
    pAnyRefDlg = pDlg;
    pLabelWidget = pLabel;

    if( pDlg )
    {
        aIdle.SetIdleHdl( LINK( this, RefEdit, UpdateHdl ) );
    }
    else
    {
        aIdle.SetIdleHdl( Link<Idle *, void>() );
        aIdle.Stop();
    }
}

void RefEdit::Modify()
{
    Edit::Modify();
    if( pAnyRefDlg )
        pAnyRefDlg->HideReference();
}

void RefEdit::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if( pAnyRefDlg && !rKeyCode.GetModifier() && (rKeyCode.GetCode() == KEY_F2) )
        pAnyRefDlg->ReleaseFocus( this );
    else
        Edit::KeyInput( rKEvt );
}

void RefEdit::GetFocus()
{
    Edit::GetFocus();
    StartUpdateData();
}

void RefEdit::LoseFocus()
{
    Edit::LoseFocus();
    if( pAnyRefDlg )
        pAnyRefDlg->HideReference();
}

IMPL_LINK_NOARG_TYPED(RefEdit, UpdateHdl, Idle *, void)
{
    if( pAnyRefDlg )
        pAnyRefDlg->ShowReference( GetText() );
}

//class RefButton
RefButton::RefButton( vcl::Window* _pParent, WinBits nStyle ) :
    ImageButton( _pParent, nStyle ),
    aImgRefStart( ModuleRes( RID_BMP_REFBTN1 ) ),
    aImgRefDone( ModuleRes( RID_BMP_REFBTN2 ) ),
    aShrinkQuickHelp( ModuleRes( RID_STR_SHRINK ).toString() ),
    aExpandQuickHelp( ModuleRes( RID_STR_EXPAND ).toString() ),
    pAnyRefDlg( nullptr ),
    pRefEdit( nullptr )
{
    SetStartImage();
}

RefButton::~RefButton()
{
    disposeOnce();
}

void RefButton::dispose()
{
    pRefEdit.clear();
    ImageButton::dispose();
}

VCL_BUILDER_FACTORY_ARGS(RefButton, 0)

void RefButton::SetStartImage()
{
    SetModeImage( aImgRefStart );
    SetQuickHelpText( aShrinkQuickHelp );
}

void RefButton::SetEndImage()
{
    SetModeImage( aImgRefDone );
    SetQuickHelpText( aExpandQuickHelp );
}

void RefButton::SetReferences( IControlReferenceHandler* pDlg, RefEdit* pEdit )
{
    pAnyRefDlg = pDlg;
    pRefEdit = pEdit;
}

void RefButton::Click()
{
    if( pAnyRefDlg )
        pAnyRefDlg->ToggleCollapsed( pRefEdit, this );
}

void RefButton::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if( pAnyRefDlg && !rKeyCode.GetModifier() && (rKeyCode.GetCode() == KEY_F2) )
        pAnyRefDlg->ReleaseFocus( pRefEdit );
    else
        ImageButton::KeyInput( rKEvt );
}

void RefButton::GetFocus()
{
    ImageButton::GetFocus();
    if( pRefEdit )
        pRefEdit->StartUpdateData();
}

void RefButton::LoseFocus()
{
    ImageButton::LoseFocus();
    if( pRefEdit )
        pRefEdit->Modify();
}

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
