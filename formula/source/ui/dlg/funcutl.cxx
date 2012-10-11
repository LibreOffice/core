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

#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>

#include "formula/funcutl.hxx"
#include "formula/IControlReferenceHandler.hxx"
#include "ControlHelper.hxx"
#include "ModuleHelper.hxx"
#include "ForResId.hrc"


namespace formula
{
//============================================================================
// class ValWnd
//----------------------------------------------------------------------------

ValWnd::ValWnd( Window* pParent, const ResId& rId ) : Window( pParent, rId )
{
    Font aFnt( GetFont() );
    aFnt.SetTransparent( sal_True );
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
}

//----------------------------------------------------------------------------

void ValWnd::Paint( const Rectangle& )
{
    DrawText( aRectOut.TopLeft(), aStrValue );
}

//----------------------------------------------------------------------------

void ValWnd::SetValue( const String& rStrVal )
{
    if ( aStrValue != rStrVal )
    {
        aStrValue = rStrVal;
        DrawRect( aRectOut );   // alten Text loeschen
        Paint( aRectOut );      // and neu malen
    }
}

//============================================================================
// class ArgEdit
//----------------------------------------------------------------------------

ArgEdit::ArgEdit( Window* pParent, const ResId& rResId )
    :   RefEdit( pParent, NULL, rResId ),
        pEdPrev ( NULL ),
        pEdNext ( NULL ),
        pSlider ( NULL ),
        nArgs   ( 0 )
{
}

//----------------------------------------------------------------------------

void ArgEdit::Init( ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                    ScrollBar& rArgSlider, sal_uInt16 nArgCount )
{
    pEdPrev = pPrevEdit;
    pEdNext = pNextEdit;
    pSlider = &rArgSlider;
    nArgs   = nArgCount;
}

//----------------------------------------------------------------------------

// Cursor control for Edit Fields in Argument Dialog

void ArgEdit::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode     aCode   = rKEvt.GetKeyCode();
    sal_Bool        bUp     = (aCode.GetCode() == KEY_UP);
    sal_Bool        bDown   = (aCode.GetCode() == KEY_DOWN);
    ArgEdit*    pEd     = NULL;

    if (   pSlider
        && ( !aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
        && ( bUp || bDown ) )
    {
        if ( nArgs > 1 )
        {
            long    nThumb       = pSlider->GetThumbPos();
            sal_Bool    bDoScroll    = sal_False;
            sal_Bool    bChangeFocus = sal_False;

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




/*************************************************************************
#*  Member:     ArgInput                                    Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   ArgInput class constructor
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

ArgInput::ArgInput()
{
    pFtArg=NULL;
    pBtnFx=NULL;
    pEdArg=NULL;
    pRefBtn=NULL;
}

/*************************************************************************
#*  Member:     InitArgInput                                Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Initializes the class' Pointer
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ArgInput::InitArgInput(FixedText*      pftArg,
                            ImageButton*    pbtnFx,
                            ArgEdit*        pedArg,
                            RefButton*  prefBtn)
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

/*************************************************************************
#*  Member:     SetArgName                                  Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Sets the Name for the Argument
#*
#*  Input:      String
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::SetArgName(const String &aArg)
{
    if(pFtArg !=NULL) pFtArg->SetText(aArg );
}

/*************************************************************************
#*  Member:     GetArgName                                  Date:06.02.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Returns the Name for the Argument
#*
#*  Input:      String
#*
#*  Output:     ---
#*
#************************************************************************/
String ArgInput::GetArgName()
{
    String aPrivArgName;
    if(pFtArg !=NULL)
        aPrivArgName=pFtArg->GetText();

    return aPrivArgName;
}

/*************************************************************************
#*  Member:     SetArgName                                  Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Sets the Name for the Argument
#*
#*  Input:      String
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::SetArgNameFont   (const Font &aFont)
{
    if(pFtArg !=NULL) pFtArg->SetFont(aFont);
}

/*************************************************************************
#*  Member:     SetArgSelection                             Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Sets up the Selection for the EditBox.
#*
#*  Input:      String
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::SetArgSelection  (const Selection& rSel )
{
    if(pEdArg !=NULL) pEdArg ->SetSelection(rSel );
}


/*************************************************************************
#*  Member:     SetArgVal                                   Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Sets the Value for the Argument
#*
#*  Input:      String
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::SetArgVal(const String &aVal)
{
    if(pEdArg !=NULL)
    {
        pEdArg ->SetRefString(aVal );
    }
}

/*************************************************************************
#*  Member:     SetArgName                                  Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Returns the Value for the Argument
#*
#*  Input:      ---
#*
#*  Output:     String
#*
#************************************************************************/
String ArgInput::GetArgVal()
{
    String aResult;
    if(pEdArg!=NULL)
    {
        aResult=pEdArg->GetText();
    }
    return aResult;
}

/*************************************************************************
#*  Member:     SetArgName                                  Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Hides the Controls
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
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

/*************************************************************************
#*  Member:     SetArgName                                  Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Casts the Controls again.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
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

/*************************************************************************
#*  Member:     FxClick                                     Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Forwards the Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::FxClick()
{
    aFxClickLink.Call(this);
}

/*************************************************************************
#*  Member:     RefClick                                    Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Forwards the Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::RefClick()
{
    aRefClickLink.Call(this);
}

/*************************************************************************
#*  Member:     FxFocus                                     Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Forwards the Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::FxFocus()
{
    aFxFocusLink.Call(this);
}

/*************************************************************************
#*  Member:     RefFocus                                    Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Forwards the Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::RefFocus()
{
    aRefFocusLink.Call(this);
}

/*************************************************************************
#*  Member:     EdFocus                                     Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Forwards the Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::EdFocus()
{
    aEdFocusLink.Call(this);
}

/*************************************************************************
#*  Member:     EdModify                                    Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Forwards the Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ArgInput::EdModify()
{
    aEdModifyLink.Call(this);
}

/*************************************************************************
#*  Handle:     FxBtnHdl                                    Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Handle for Fx-Button Click-Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
IMPL_LINK( ArgInput, FxBtnClickHdl, ImageButton*, pBtn )
{
    if(pBtn==pBtnFx) FxClick();

    return 0;
}

/*************************************************************************
#*  Handle:     RefBtnClickHdl                              Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Handle for Fx-Button Click-Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
IMPL_LINK( ArgInput, RefBtnClickHdl,RefButton*, pBtn )
{
    if(pRefBtn==pBtn) RefClick();

    return 0;
}

/*************************************************************************
#*  Handle:     FxBtnFocusHdl                               Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Handle for Fx-Button Focus-Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
IMPL_LINK( ArgInput, FxBtnFocusHdl, ImageButton*, pBtn )
{
    if(pBtn==pBtnFx) FxFocus();

    return 0;
}

/*************************************************************************
#*  Handle:     RefBtnFocusHdl                              Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Handle for Fx-Button Focus-Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
IMPL_LINK( ArgInput, RefBtnFocusHdl,RefButton*, pBtn )
{
    if(pRefBtn==pBtn) RefFocus();

    return 0;
}

/*************************************************************************
#*  Handle:     EdFocusHdl                                  Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Handle for Fx-Button Focus-Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
IMPL_LINK( ArgInput, EdFocusHdl, ArgEdit*, pEd )
{
    if(pEd==pEdArg) EdFocus();

    return 0;
}

/*************************************************************************
#*  Handle:     RefBtnClickHdl                              Date:13.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      ArgInput
#*
#*  Function:   Handle for Fx-Button Focus-Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
IMPL_LINK( ArgInput, EdModifyHdl,ArgEdit*, pEd )
{
    if(pEd==pEdArg) EdModify();

    return 0;
}

/*************************************************************************
#*  Member:     EditBox                                 Date:20.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      EditBox
#*
#*  Function:   Constructor from Class ArgInput
#*
#*  Input:      Parent, Resource
#*
#*  Output:     ---
#*
#************************************************************************/
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

    //  #105582# the HelpId from the resource must be set for the MultiLineEdit,
    //  not for the control that contains it.
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
/*************************************************************************
#*  Member:     EditBox                                 Date:20.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      EditBox
#*
#*  Function:   When the seleccion is changed this function will be called
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void EditBox::SelectionChanged()
{
    aSelChangedLink.Call(this);
}

/*************************************************************************
#*  Member:     EditBox                                 Date:20.05.98
#*------------------------------------------------------------------------
#*
#*  Class:      EditBox
#*
#*  Function:   When the size is changed, MultiLineEdit must
#*              be adapted..
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void EditBox::Resize()
{
    Size aSize=GetOutputSizePixel();
    if(pMEdit!=NULL) pMEdit->SetOutputSizePixel(aSize);
}

/*************************************************************************
#*  Member:     GetFocus                                    Date:26.05.98
#*------------------------------------------------------------------------
#*
#*  Class:      EditBox
#*
#*  Function:   When the Control is activated,
#*              the Selection is repealed and the Cursor set
#*              at the end.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void EditBox::GetFocus()
{
    if(pMEdit!=NULL)
    {
        pMEdit->GrabFocus();
    }
}



/*************************************************************************
#*  Member:     EditBox                                 Date:20.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      EditBox
#*
#*  Function:   When an Event is cleared, this Routine is
#*              first called and a PostUserEvent is sent.
#*
#*  Input:      Notify-Event
#*
#*  Output:     ---
#*
#************************************************************************/
long EditBox::PreNotify( NotifyEvent& rNEvt )
{
    long nResult=sal_True;

    if(pMEdit==NULL) return nResult;

    sal_uInt16 nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_KEYINPUT)// || nSwitch==EVENT_KEYUP)
    {
        const KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey=aKeyCode.GetCode();
        if( (nKey==KEY_RETURN && !aKeyCode.IsShift()) || nKey==KEY_TAB )
        {
            nResult=GetParent()->Notify(rNEvt);
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

/*************************************************************************
#*  Member:     EditBox                                 Date:21.01.97
#*------------------------------------------------------------------------
#*
#*  Class:      EditBox
#*
#*  Function:   When an Event cleared wurde, this routine is
#*              first called.
#*
#*  Input:      Key-Event
#*
#*  Output:     ---
#*
#************************************************************************/
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
//----------------------------------------------------------------------------

//============================================================================
//  class RefEdit
//----------------------------------------------------------------------------

#define SC_ENABLE_TIME 100

RefEdit::RefEdit( Window* _pParent,IControlReferenceHandler* pParent, const ResId& rResId ) :
    Edit( _pParent, rResId ),
    pAnyRefDlg( pParent )
{
    aTimer.SetTimeoutHdl( LINK( this, RefEdit, UpdateHdl ) );
    aTimer.SetTimeout( SC_ENABLE_TIME );
}

RefEdit::~RefEdit()
{
    aTimer.SetTimeoutHdl( Link() );
    aTimer.Stop();
}

void RefEdit::SetRefString( const XubString& rStr )
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

void RefEdit::SetText( const XubString& rStr )
{
    Edit::SetText( rStr );
    UpdateHdl( &aTimer );
}

void RefEdit::StartUpdateData()
{
    aTimer.Start();
}

void RefEdit::SetRefDialog( IControlReferenceHandler* pDlg )
{
    pAnyRefDlg = pDlg;

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


//============================================================================
//  class RefButton
//----------------------------------------------------------------------------
RefButton::RefButton( Window* _pParent, const ResId& rResId) :
    ImageButton( _pParent, rResId ),
    aImgRefStart( ModuleRes( RID_BMP_REFBTN1 ) ),
    aImgRefDone( ModuleRes( RID_BMP_REFBTN2 ) ),
    pAnyRefDlg( NULL ),
    pRefEdit( NULL )
{
    SetStartImage();
}

RefButton::RefButton( Window* _pParent, const ResId& rResId, RefEdit* pEdit, IControlReferenceHandler* _pDlg ) :
    ImageButton( _pParent, rResId ),
    aImgRefStart( ModuleRes( RID_BMP_REFBTN1 ) ),
    aImgRefDone( ModuleRes( RID_BMP_REFBTN2 ) ),
    pAnyRefDlg( _pDlg ),
    pRefEdit( pEdit )
{
    SetStartImage();
}

void RefButton::SetStartImage()
{
    SetModeImage( aImgRefStart );
}

void RefButton::SetEndImage()
{
    SetModeImage( aImgRefDone );
}

void RefButton::SetReferences( IControlReferenceHandler* pDlg, RefEdit* pEdit )
{
    pAnyRefDlg = pDlg;
    pRefEdit = pEdit;
}

//----------------------------------------------------------------------------

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


} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
