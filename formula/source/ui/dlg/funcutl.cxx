/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


ValWnd::ValWnd( Window* pParent, const ResId& rId ) : Window( pParent, rId )
{
    Font aFnt( GetFont() );
    aFnt.SetTransparent( true );
    aFnt.SetWeight( WEIGHT_LIGHT );
    if ( pParent->IsBackground() )
    {
        Wallpaper aBack = pParent->GetBackground();
        SetFillColor( aBack.GetColor() );
        SetBackground( aBack );
        aFnt.SetFillColor( aBack.GetColor() );
    }
    else
    {
        SetFillColor();
        SetBackground();
    }
    SetFont( aFnt );
    SetLineColor();

    Size aSzWnd  = GetOutputSizePixel();
    long nHeight = GetTextHeight();
    long nDiff   = aSzWnd.Height()-nHeight;

    aRectOut = Rectangle( Point( 1, ( nDiff<2 ) ? 1 : nDiff/2),
                          Size ( aSzWnd.Width()-2, nHeight ) );
    SetClipRegion( Region( aRectOut ) );
    SetAccessibleRole( ::com::sun::star::accessibility::AccessibleRole::LABEL );
}

void ValWnd::Paint( const Rectangle& )
{
    DrawText( aRectOut.TopLeft(), aStrValue );
}

void ValWnd::SetValue( const OUString& rStrVal )
{
    if ( aStrValue != rStrVal )
    {
        aStrValue = rStrVal;
        DrawRect( aRectOut );   
        Paint( aRectOut );      
    }
}



ArgEdit::ArgEdit( Window* pParent, const ResId& rResId )
    :   RefEdit( pParent, NULL, NULL, rResId ),
        pEdPrev ( NULL ),
        pEdNext ( NULL ),
        pSlider ( NULL ),
        nArgs   ( 0 )
{
}

void ArgEdit::Init( ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                    ScrollBar& rArgSlider, sal_uInt16 nArgCount )
{
    pEdPrev = pPrevEdit;
    pEdNext = pNextEdit;
    pSlider = &rArgSlider;
    nArgs   = nArgCount;
}


void ArgEdit::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode     aCode   = rKEvt.GetKeyCode();
    sal_Bool        bUp     = (aCode.GetCode() == KEY_UP);
    sal_Bool        bDown   = (aCode.GetCode() == KEY_DOWN);

    if (   pSlider
        && ( !aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
        && ( bUp || bDown ) )
    {
        if ( nArgs > 1 )
        {
            ArgEdit* pEd = NULL;
            long nThumb = pSlider->GetThumbPos();
            sal_Bool bDoScroll = sal_False;
            sal_Bool bChangeFocus = sal_False;

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
                        bChangeFocus = sal_True;
                    }
                }
                else if ( pEdNext )
                {
                    pEd = pEdNext;
                    bChangeFocus = sal_True;
                }
            }
            else 
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
                        bChangeFocus = sal_True;
                    }
                }
                else if ( pEdPrev )
                {
                    pEd = pEdPrev;
                    bChangeFocus = sal_True;
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


ArgInput::ArgInput()
{
    pFtArg=NULL;
    pBtnFx=NULL;
    pEdArg=NULL;
    pRefBtn=NULL;
}

void ArgInput::InitArgInput( FixedText* pftArg, ImageButton* pbtnFx,
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


void ArgInput::SetArgName(const OUString &aArg)
{
    if(pFtArg !=NULL) pFtArg->SetText(aArg );
}


OUString ArgInput::GetArgName()
{
    OUString aPrivArgName;
    if(pFtArg !=NULL)
        aPrivArgName=pFtArg->GetText();

    return aPrivArgName;
}


void ArgInput::SetArgNameFont   (const Font &aFont)
{
    if(pFtArg !=NULL) pFtArg->SetFont(aFont);
}


void ArgInput::SetArgSelection  (const Selection& rSel )
{
    if(pEdArg !=NULL) pEdArg ->SetSelection(rSel );
}


void ArgInput::SetArgVal(const OUString &rVal)
{
    if(pEdArg !=NULL)
    {
        pEdArg ->SetRefString(rVal);
    }
}


OUString ArgInput::GetArgVal()
{
    OUString aResult;
    if(pEdArg!=NULL)
    {
        aResult=pEdArg->GetText();
    }
    return aResult;
}


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


EditBox::EditBox( Window* pParent, const ResId& rResId )
        :Control(pParent,rResId),
        bMouseFlag(sal_False)
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

    
    
    pMEdit->SetHelpId( GetHelpId() );
    SetHelpId( "" );
}

EditBox::~EditBox()
{
    MultiLineEdit* pTheEdit=pMEdit;
    pMEdit->Disable();
    pMEdit=NULL;
    delete pTheEdit;
}


void EditBox::SelectionChanged()
{
    aSelChangedLink.Call(this);
}


void EditBox::Resize()
{
    Size aSize=GetOutputSizePixel();
    if(pMEdit!=NULL) pMEdit->SetOutputSizePixel(aSize);
}



void EditBox::GetFocus()
{
    if(pMEdit!=NULL)
    {
        pMEdit->GrabFocus();
    }
}



bool EditBox::PreNotify( NotifyEvent& rNEvt )
{
    bool nResult = true;

    if(pMEdit==NULL) return nResult;

    sal_uInt16 nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_KEYINPUT)
    {
        const KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
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
            bMouseFlag=sal_True;
            Application::PostUserEvent( LINK( this, EditBox, ChangedHdl ) );
        }
    }
    return nResult;
}



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
    
    

    if (pMEdit)
        aOldSel = pMEdit->GetSelection();
}



#define SC_ENABLE_TIME 100

RefEdit::RefEdit( Window* _pParent,IControlReferenceHandler* pParent,
    Window* pShrinkModeLabel, const ResId& rResId )
    : Edit( _pParent, rResId )
    , pAnyRefDlg( pParent )
    , pLabelWidget(pShrinkModeLabel)
{
    aTimer.SetTimeoutHdl( LINK( this, RefEdit, UpdateHdl ) );
    aTimer.SetTimeout( SC_ENABLE_TIME );
}

RefEdit::RefEdit( Window* _pParent, Window* pShrinkModeLabel, WinBits nStyle )
    : Edit( _pParent, nStyle )
    , pAnyRefDlg( NULL )
    , pLabelWidget(pShrinkModeLabel)
{
    aTimer.SetTimeoutHdl( LINK( this, RefEdit, UpdateHdl ) );
    aTimer.SetTimeout( SC_ENABLE_TIME );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeRefEdit(Window *pParent, VclBuilder::stringmap &)
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

void RefEdit::SetReferences( IControlReferenceHandler* pDlg, Window* pLabel )
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
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
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



RefButton::RefButton( Window* _pParent, const ResId& rResId) :
    ImageButton( _pParent, rResId ),
    aImgRefStart( ModuleRes( RID_BMP_REFBTN1 ) ),
    aImgRefDone( ModuleRes( RID_BMP_REFBTN2 ) ),
    aShrinkQuickHelp( ModuleRes( RID_STR_SHRINK ).toString() ),
    aExpandQuickHelp( ModuleRes( RID_STR_EXPAND ).toString() ),
    pAnyRefDlg( NULL ),
    pRefEdit( NULL )
{
    SetStartImage();
}

RefButton::RefButton( Window* _pParent, WinBits nStyle ) :
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

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeRefButton(Window *pParent, VclBuilder::stringmap &)
{
    return new RefButton(pParent, 0);
}

RefButton::RefButton( Window* _pParent, const ResId& rResId, RefEdit* pEdit, IControlReferenceHandler* _pDlg ) :
    ImageButton( _pParent, rResId ),
    aImgRefStart( ModuleRes( RID_BMP_REFBTN1 ) ),
    aImgRefDone( ModuleRes( RID_BMP_REFBTN2 ) ),
    aShrinkQuickHelp( ModuleRes( RID_STR_SHRINK ).toString() ),
    aExpandQuickHelp( ModuleRes( RID_STR_EXPAND ).toString() ),
    pAnyRefDlg( _pDlg ),
    pRefEdit( pEdit )
{
    SetStartImage();
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
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
