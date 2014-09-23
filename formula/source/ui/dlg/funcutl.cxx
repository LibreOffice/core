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
    :   RefEdit( pParent, NULL, nBits ),
        pEdPrev ( NULL ),
        pEdNext ( NULL ),
        pSlider ( NULL ),
        nArgs   ( 0 )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeArgEdit(vcl::Window *pParent, VclBuilder::stringmap &)
{
    return new ArgEdit(pParent, WB_BORDER);
}

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
            ArgEdit* pEd = NULL;
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
                ((Link&)pSlider->GetEndScrollHdl()).Call( pSlider );
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
    pFtArg=NULL;
    pBtnFx=NULL;
    pEdArg=NULL;
    pRefBtn=NULL;
}

void ArgInput::InitArgInput( FixedText* pftArg, PushButton* pbtnFx,
                             ArgEdit* pedArg, RefButton* prefBtn)
{
    pFtArg =pftArg;
    pBtnFx =pbtnFx;
    pEdArg =pedArg;
    pRefBtn=prefBtn;

    if(pBtnFx!=NULL)
    {
        pBtnFx->SetClickHdl   ( LINK( this, ArgInput, FxBtnClickHdl ) );
        pBtnFx->SetGetFocusHdl( LINK( this, ArgInput, FxBtnFocusHdl ) );
    }
    if(pRefBtn!=NULL)
    {
        pRefBtn->SetClickHdl   ( LINK( this, ArgInput, RefBtnClickHdl ) );
        pRefBtn->SetGetFocusHdl( LINK( this, ArgInput, RefBtnFocusHdl ) );
    }
    if(pEdArg!=NULL)
    {
        pEdArg->SetGetFocusHdl ( LINK( this, ArgInput, EdFocusHdl ) );
        pEdArg->SetModifyHdl   ( LINK( this, ArgInput, EdModifyHdl ) );
    }

}

// Sets the Name for the Argument
void ArgInput::SetArgName(const OUString &aArg)
{
    if(pFtArg !=NULL) pFtArg->SetText(aArg );
}

// Returns the Name for the Argument
OUString ArgInput::GetArgName()
{
    OUString aPrivArgName;
    if(pFtArg !=NULL)
        aPrivArgName=pFtArg->GetText();

    return aPrivArgName;
}

//Sets the Name for the Argument
void ArgInput::SetArgNameFont   (const vcl::Font &aFont)
{
    if(pFtArg !=NULL) pFtArg->SetFont(aFont);
}

//Sets up the Selection for the EditBox.
void ArgInput::SetArgSelection  (const Selection& rSel )
{
    if(pEdArg !=NULL) pEdArg ->SetSelection(rSel );
}

//Sets the Value for the Argument
void ArgInput::SetArgVal(const OUString &rVal)
{
    if(pEdArg !=NULL)
    {
        pEdArg ->SetRefString(rVal);
    }
}

//Returns the Value for the Argument
OUString ArgInput::GetArgVal()
{
    OUString aResult;
    if(pEdArg!=NULL)
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
    aFxClickLink.Call(this);
}

void ArgInput::RefClick()
{
    aRefClickLink.Call(this);
}

void ArgInput::FxFocus()
{
    aFxFocusLink.Call(this);
}

void ArgInput::RefFocus()
{
    aRefFocusLink.Call(this);
}

void ArgInput::EdFocus()
{
    aEdFocusLink.Call(this);
}

void ArgInput::EdModify()
{
    aEdModifyLink.Call(this);
}

IMPL_LINK( ArgInput, FxBtnClickHdl, ImageButton*, pBtn )
{
    if(pBtn == pBtnFx)
        FxClick();

    return 0;
}

IMPL_LINK( ArgInput, RefBtnClickHdl,RefButton*, pBtn )
{
    if(pRefBtn == pBtn)
        RefClick();

    return 0;
}

IMPL_LINK( ArgInput, FxBtnFocusHdl, ImageButton*, pBtn )
{
    if(pBtn == pBtnFx)
        FxFocus();

    return 0;
}

IMPL_LINK( ArgInput, RefBtnFocusHdl,RefButton*, pBtn )
{
    if(pRefBtn == pBtn)
        RefFocus();

    return 0;
}

IMPL_LINK( ArgInput, EdFocusHdl, ArgEdit*, pEd )
{
    if(pEd == pEdArg)
        EdFocus();

    return 0;
}

IMPL_LINK( ArgInput, EdModifyHdl,ArgEdit*, pEd )
{
    if(pEd == pEdArg)
        EdModify();

    return 0;
}

// class EditBox

EditBox::EditBox( vcl::Window* pParent, WinBits nBits )
        :Control(pParent,nBits),
        bMouseFlag(false)
{
    WinBits nStyle=GetStyle();
    SetStyle( nStyle| WB_DIALOGCONTROL);

    pMEdit=new MultiLineEdit(this,WB_LEFT | WB_VSCROLL | (nStyle & WB_TABSTOP) |
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

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeEditBox(vcl::Window *pParent, VclBuilder::stringmap &)
{
    return new EditBox(pParent, WB_BORDER);
}

EditBox::~EditBox()
{
    MultiLineEdit* pTheEdit=pMEdit;
    pMEdit->Disable();
    pMEdit=NULL;
    delete pTheEdit;
}

// When the selection is changed this function will be called
void EditBox::SelectionChanged()
{
    aSelChangedLink.Call(this);
}

// When the size is changed, MultiLineEdit must be adapted..
void EditBox::Resize()
{
    Size aSize=GetOutputSizePixel();
    if(pMEdit!=NULL) pMEdit->SetOutputSizePixel(aSize);
}

// When the Control is activated, the Selection is repealed
// and the Cursor set at the end.
void EditBox::GetFocus()
{
    if(pMEdit!=NULL)
    {
        pMEdit->GrabFocus();
    }
}

//When an Event is cleared, this Routine is
//first called and a PostUserEvent is sent.
bool EditBox::PreNotify( NotifyEvent& rNEvt )
{
    bool nResult = true;

    if(pMEdit==NULL) return nResult;

    sal_uInt16 nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_KEYINPUT)// || nSwitch==EVENT_KEYUP)
    {
        const vcl::KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey=aKeyCode.GetCode();
        if( (nKey==KEY_RETURN && !aKeyCode.IsShift()) || nKey==KEY_TAB )
        {
            nResult = GetParent()->Notify(rNEvt);
        }
        else
        {
            nResult=Control::PreNotify(rNEvt);
            Application::PostUserEvent( LINK( this, EditBox, ChangedHdl ) );
        }

    }
    else
    {
        nResult=Control::PreNotify(rNEvt);

        if(nSwitch==EVENT_MOUSEBUTTONDOWN || nSwitch==EVENT_MOUSEBUTTONUP)
        {
            bMouseFlag=true;
            Application::PostUserEvent( LINK( this, EditBox, ChangedHdl ) );
        }
    }
    return nResult;
}

//When an Event cleared wurde, this routine is
//first called.
IMPL_LINK_NOARG(EditBox, ChangedHdl)
{
    if(pMEdit!=NULL)
    {
        Selection aNewSel=pMEdit->GetSelection();

        if(aNewSel.Min()!=aOldSel.Min() || aNewSel.Max()!=aOldSel.Max())
        {
            SelectionChanged();
            aOldSel=aNewSel;
        }
    }
    return 0;
}

void EditBox::UpdateOldSel()
{
    //  if selection is set for editing a function, store it as aOldSel,
    //  so SelectionChanged isn't called in the next ChangedHdl call

    if (pMEdit)
        aOldSel = pMEdit->GetSelection();
}

// class RefEdit

#define SC_ENABLE_TIME 100

RefEdit::RefEdit( vcl::Window* _pParent, vcl::Window* pShrinkModeLabel, WinBits nStyle )
    : Edit( _pParent, nStyle )
    , pAnyRefDlg( NULL )
    , pLabelWidget(pShrinkModeLabel)
{
    aTimer.SetTimeoutHdl( LINK( this, RefEdit, UpdateHdl ) );
    aTimer.SetTimeout( SC_ENABLE_TIME );
}

RefEdit::RefEdit( vcl::Window* _pParent,IControlReferenceHandler* pParent,
    vcl::Window* pShrinkModeLabel, const ResId& rResId )
    : Edit( _pParent, rResId )
    , pAnyRefDlg( pParent )
    , pLabelWidget(pShrinkModeLabel)
{
    aTimer.SetTimeoutHdl( LINK( this, RefEdit, UpdateHdl ) );
    aTimer.SetTimeout( SC_ENABLE_TIME );
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeRefEdit(vcl::Window *pParent, VclBuilder::stringmap &)
{
    return new RefEdit(pParent, NULL, WB_BORDER);
}

RefEdit::~RefEdit()
{
    aTimer.SetTimeoutHdl( Link() );
    aTimer.Stop();
}

void RefEdit::SetRefString( const OUString& rStr )
{
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
        SetControlForeground(COL_WHITE);
        SetControlBackground(0xff6563);
    }
}

void RefEdit::SetText(const OUString& rStr)
{
    Edit::SetText( rStr );
    UpdateHdl( &aTimer );
}

void RefEdit::StartUpdateData()
{
    aTimer.Start();
}

void RefEdit::SetReferences( IControlReferenceHandler* pDlg, vcl::Window* pLabel )
{
    pAnyRefDlg = pDlg;
    pLabelWidget = pLabel;

    if( pDlg )
    {
        aTimer.SetTimeoutHdl( LINK( this, RefEdit, UpdateHdl ) );
        aTimer.SetTimeout( SC_ENABLE_TIME );
    }
    else
    {
        aTimer.SetTimeoutHdl( Link() );
        aTimer.Stop();
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

IMPL_LINK_NOARG(RefEdit, UpdateHdl)
{
    if( pAnyRefDlg )
        pAnyRefDlg->ShowReference( GetText() );
    return 0;
}

//class RefButton
RefButton::RefButton( vcl::Window* _pParent, WinBits nStyle ) :
    ImageButton( _pParent, nStyle ),
    aImgRefStart( ModuleRes( RID_BMP_REFBTN1 ) ),
    aImgRefDone( ModuleRes( RID_BMP_REFBTN2 ) ),
    aShrinkQuickHelp( ModuleRes( RID_STR_SHRINK ).toString() ),
    aExpandQuickHelp( ModuleRes( RID_STR_EXPAND ).toString() ),
    pAnyRefDlg( NULL ),
    pRefEdit( NULL )
{
    SetStartImage();
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeRefButton(vcl::Window *pParent, VclBuilder::stringmap &)
{
    return new RefButton(pParent, 0);
}

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
