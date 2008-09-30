/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: funcutl.cxx,v $
 * $Revision: 1.9.32.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//----------------------------------------------------------------------------

#include <vcl/sound.hxx>
#include <vcl/svapp.hxx>

#include "funcutl.hxx"


//============================================================================
// class ValWnd
//----------------------------------------------------------------------------

ValWnd::ValWnd( Window* pParent, const ResId& rId ) : Window( pParent, rId )
{
    Font aFnt( GetFont() );
    aFnt.SetTransparent( TRUE );
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

void __EXPORT ValWnd::Paint( const Rectangle& )
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
        Paint( aRectOut );      // und neu malen
    }
}

//============================================================================
// class ArgEdit
//----------------------------------------------------------------------------

ArgEdit::ArgEdit( Window* pParent, const ResId& rResId )
    :   ScRefEdit( pParent, rResId ),
        pEdPrev ( NULL ),
        pEdNext ( NULL ),
        pSlider ( NULL ),
        nArgs   ( 0 )
{
}

//----------------------------------------------------------------------------

void ArgEdit::Init( ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                    ScrollBar& rArgSlider, USHORT nArgCount )
{
    pEdPrev = pPrevEdit;
    pEdNext = pNextEdit;
    pSlider = &rArgSlider;
    nArgs   = nArgCount;
}

//----------------------------------------------------------------------------

// Cursorsteuerung fuer EditFelder im Argument-Dialog

void __EXPORT ArgEdit::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode     aCode   = rKEvt.GetKeyCode();
    BOOL        bUp     = (aCode.GetCode() == KEY_UP);
    BOOL        bDown   = (aCode.GetCode() == KEY_DOWN);
    ArgEdit*    pEd     = NULL;

    if (   pSlider
        && ( !aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
        && ( bUp || bDown ) )
    {
        if ( nArgs > 1 )
        {
            long    nThumb       = pSlider->GetThumbPos();
            BOOL    bDoScroll    = FALSE;
            BOOL    bChangeFocus = FALSE;

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
                        bChangeFocus = TRUE;
                    }
                }
                else if ( pEdNext )
                {
                    pEd = pEdNext;
                    bChangeFocus = TRUE;
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
                        bChangeFocus = TRUE;
                    }
                }
                else if ( pEdPrev )
                {
                    pEd = pEdPrev;
                    bChangeFocus = TRUE;
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
            else
                Sound::Beep();
        }
        else
            Sound::Beep();
    }
    else
        ScRefEdit::KeyInput( rKEvt );
}




/*************************************************************************
#*  Member:     ArgInput                                    Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Konstruktor der Klasse ArgInput
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
#*  Member:     InitArgInput                                Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Initialisiert die Pointer der Klasse
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ArgInput::InitArgInput(FixedText*      pftArg,
                            ImageButton*    pbtnFx,
                            ArgEdit*        pedArg,
                            ScRefButton*    prefBtn)
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
#*  Member:     SetArgName                                  Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Setzt den Namen fuer das Argument
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
#*  Member:     GetArgName                                  Datum:06.02.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Liefert den Namen fuer das Argument zurueck
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
#*  Member:     SetArgName                                  Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Setzt den Namen fuer das Argument
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
#*  Member:     SetArgSelection                             Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Stellt die Selection fuer die EditBox ein.
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
#*  Member:     SetArgVal                                   Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Setzt den Wert fuer das Argument
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
#*  Member:     SetArgName                                  Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Liefert den Wert fuer das Argument
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
#*  Member:     SetArgName                                  Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Versteckt die Controls
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
#*  Member:     SetArgName                                  Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Zaubert die Controls wieder hervor.
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
#*  Member:     FxClick                                     Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Gibt den Event weiter.
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
#*  Member:     RefClick                                    Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Gibt den Event weiter.
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
#*  Member:     FxFocus                                     Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Gibt den Event weiter.
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
#*  Member:     RefFocus                                    Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Gibt den Event weiter.
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
#*  Member:     EdFocus                                     Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Gibt den Event weiter.
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
#*  Member:     EdModify                                    Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Gibt den Event weiter.
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
#*  Handle:     FxBtnHdl                                    Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Handle fuer Fx-Button Click-Event.
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
#*  Handle:     RefBtnClickHdl                              Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Handle fuer Fx-Button Click-Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
IMPL_LINK( ArgInput, RefBtnClickHdl,ScRefButton*, pBtn )
{
    if(pRefBtn==pBtn) RefClick();

    return 0;
}

/*************************************************************************
#*  Handle:     FxBtnFocusHdl                               Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Handle fuer Fx-Button Focus-Event.
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
#*  Handle:     RefBtnFocusHdl                              Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Handle fuer Fx-Button Focus-Event.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
IMPL_LINK( ArgInput, RefBtnFocusHdl,ScRefButton*, pBtn )
{
    if(pRefBtn==pBtn) RefFocus();

    return 0;
}

/*************************************************************************
#*  Handle:     EdFocusHdl                                  Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Handle fuer Fx-Button Focus-Event.
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
#*  Handle:     RefBtnClickHdl                              Datum:13.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ArgInput
#*
#*  Funktion:   Handle fuer Fx-Button Focus-Event.
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
#*  Member:     ScEditBox                                   Datum:20.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScEditBox
#*
#*  Funktion:   Konstruktor der Klasse ArgInput
#*
#*  Input:      Parent, Resource
#*
#*  Output:     ---
#*
#************************************************************************/
ScEditBox::ScEditBox( Window* pParent, const ResId& rResId )
        :Control(pParent,rResId),
        bMouseFlag(FALSE)
{
    WinBits nStyle=GetStyle();
    SetStyle( nStyle| WB_DIALOGCONTROL);

    pMEdit=new MultiLineEdit(this,WB_LEFT | WB_VSCROLL | nStyle & WB_TABSTOP|
                    WB_NOBORDER | WB_NOHIDESELECTION | WB_IGNORETAB);
    pMEdit->Show();
    aOldSel=pMEdit->GetSelection();
    Resize();
    WinBits nWinStyle=GetStyle() | WB_DIALOGCONTROL;
    SetStyle(nWinStyle);

    //  #105582# the HelpId from the resource must be set for the MultiLineEdit,
    //  not for the control that contains it.
    pMEdit->SetHelpId( GetHelpId() );
    SetHelpId( 0 );
}

ScEditBox::~ScEditBox()
{
    MultiLineEdit* pTheEdit=pMEdit;
    pMEdit->Disable();
    pMEdit=NULL;
    delete pTheEdit;
}
/*************************************************************************
#*  Member:     ScEditBox                                   Datum:20.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScEditBox
#*
#*  Funktion:   Wenn sich die Selektion geaendert hat, so wird
#*              diese Funktion aufgerufen.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ScEditBox::SelectionChanged()
{
    aSelChangedLink.Call(this);
}

/*************************************************************************
#*  Member:     ScEditBox                                   Datum:20.05.98
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScEditBox
#*
#*  Funktion:   Wenn sich die Groesse geaendert hat, so muss
#*              auch der MultiLineEdit angepasst werden..
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ScEditBox::Resize()
{
    Size aSize=GetOutputSizePixel();
    if(pMEdit!=NULL) pMEdit->SetOutputSizePixel(aSize);
}

/*************************************************************************
#*  Member:     GetFocus                                    Datum:26.05.98
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScEditBox
#*
#*  Funktion:   Wenn der Control aktiviert wird, so wird
#*              die Selection aufgehoben und der Cursor ans
#*              Ende gesetzt.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/
void ScEditBox::GetFocus()
{
    if(pMEdit!=NULL)
    {
        pMEdit->GrabFocus();
    }
}



/*************************************************************************
#*  Member:     ScEditBox                                   Datum:20.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScEditBox
#*
#*  Funktion:   Wenn ein Event ausgeloest wird, so wird diese Routine
#*              zuerst aufgerufen und ein PostUserEvent verschickt.
#*
#*  Input:      Notify-Event
#*
#*  Output:     ---
#*
#************************************************************************/
long ScEditBox::PreNotify( NotifyEvent& rNEvt )
{
    long nResult=TRUE;

    if(pMEdit==NULL) return nResult;

    USHORT nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_KEYINPUT)// || nSwitch==EVENT_KEYUP)
    {
        const KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        USHORT nKey=aKeyCode.GetCode();
        if(nKey==KEY_RETURN && !aKeyCode.IsShift() || nKey==KEY_TAB)
        {
            nResult=GetParent()->Notify(rNEvt);
        }
        else
        {
            nResult=Control::PreNotify(rNEvt);
            Application::PostUserEvent( LINK( this, ScEditBox, ChangedHdl ) );
        }

    }
    else
    {
        nResult=Control::PreNotify(rNEvt);

        if(nSwitch==EVENT_MOUSEBUTTONDOWN || nSwitch==EVENT_MOUSEBUTTONUP)
        {
            bMouseFlag=TRUE;
            Application::PostUserEvent( LINK( this, ScEditBox, ChangedHdl ) );
        }
    }
    return nResult;
}

/*************************************************************************
#*  Member:     ScEditBox                                   Datum:21.01.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScEditBox
#*
#*  Funktion:   Wenn ein Event ausgeloest wurde, so wird diese Routine
#*              zuerst aufgerufen.
#*
#*  Input:      Key-Event
#*
#*  Output:     ---
#*
#************************************************************************/
IMPL_LINK( ScEditBox, ChangedHdl, ScEditBox*, EMPTYARG )
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

void ScEditBox::UpdateOldSel()
{
    //  if selection is set for editing a function, store it as aOldSel,
    //  so SelectionChanged isn't called in the next ChangedHdl call

    if (pMEdit)
        aOldSel = pMEdit->GetSelection();
}


