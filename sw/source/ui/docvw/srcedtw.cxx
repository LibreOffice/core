/*************************************************************************
 *
 *  $RCSfile: srcedtw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"
#include "cmdid.h"


#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _TEXTVIEW_HXX
#include <svtools/textview.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _TXTATTR_HXX //autogen
#include <svtools/txtattr.hxx>
#endif


#include "swmodule.hxx"
#include "docsh.hxx"
#include "srcview.hxx"
#include "srcedtw.hxx"
#include "srcvcfg.hxx"
#include "helpid.h"


enum SwHtmlTextType {
    TT_SGML     = SRC_SYN_SGML  ,
    TT_COMMENT  = SRC_SYN_COMMENT   ,
    TT_KEYWORD  = SRC_SYN_KEYWRD    ,
    TT_UNKNOWN  = SRC_SYN_UNKNOWN
};


struct SwTextPortion
{
    USHORT nLine;
    USHORT nStart, nEnd;
    SwHtmlTextType eType;
};

#define MAX_SYNTAX_HIGHLIGHT 20

SV_DECL_VARARR(SwTextPortions, SwTextPortion,16,16)

/* -----------------15.01.97 12.07-------------------

--------------------------------------------------*/

SV_IMPL_VARARR(SwTextPortions, SwTextPortion)

/*-----------------15.01.97 12.08-------------------

--------------------------------------------------*/

void lcl_Highlight(const String& rSource, SwTextPortions& aPortionList)
{
    const sal_Unicode cOpenBracket = '<';
    const sal_Unicode cCloseBracket= '>';
    const sal_Unicode cSlash        = '/';
    const sal_Unicode cExclamation = '!';
    const sal_Unicode cQuote        = '"';
    const sal_Unicode cSQuote      = '\'';
    const sal_Unicode cMinus        = '-';
    const sal_Unicode cSpace        = ' ';
    const sal_Unicode cTab          = 0x09;
    const sal_Unicode cLF          = 0x0a;
    const sal_Unicode cCR          = 0x0d;


    const USHORT nStrLen = rSource.Len();
    USHORT nInsert = 0;         // Anzahl der eingefuegten Portions
    USHORT nActPos = 0;         //Position, an der '<' gefunden wurde
    USHORT nOffset = 0;         //Offset von nActPos zur '<'
    USHORT nPortStart = USHRT_MAX;  // fuer die TextPortion
    USHORT nPortEnd  =  0;  //
    SwTextPortion aText;
    while(nActPos < nStrLen)
    {
        SwHtmlTextType eFoundType = TT_UNKNOWN;
        if(rSource.GetChar(nActPos) == cOpenBracket && nActPos < nStrLen - 2 )
        {
            // 'leere' Portion einfuegen
            if(nPortEnd < nActPos - 1 )
            {
                aText.nLine = 0;
                // am Anfang nicht verschieben
                aText.nStart = nPortEnd;
                if(nInsert)
                    aText.nStart += 1;
                aText.nEnd = nActPos - 1;
                aText.eType = TT_UNKNOWN;
                aPortionList.Insert(aText, nInsert++);
            }
            sal_Unicode cFollowFirst = rSource.GetChar((xub_StrLen)(nActPos + 1));
            sal_Unicode cFollowNext = rSource.GetChar((xub_StrLen)(nActPos + 2));
            if(cExclamation == cFollowFirst)
            {
                // "<!" SGML oder Kommentar
                if(cMinus == cFollowNext &&
                    nActPos < nStrLen - 3 && cMinus == rSource.GetChar((xub_StrLen)(nActPos + 3)))
                {
                    eFoundType = TT_COMMENT;
                }
                else
                    eFoundType = TT_SGML;
                nPortStart = nActPos;
                nPortEnd = nActPos + 1;
            }
            else if(cSlash == cFollowFirst)
            {
                // "</" Slash ignorieren
                nPortStart = nActPos;
                nActPos++;
                nOffset++;
            }
            if(TT_UNKNOWN == eFoundType)
            {
                //jetzt koennte hier ein keyword folgen
                USHORT nSrchPos = nActPos;
                while(++nSrchPos < nStrLen - 1)
                {
                    sal_Unicode cNext = rSource.GetChar(nSrchPos);
                    if( cNext == cSpace ||
                        cNext == cTab   ||
                        cNext == cLF    ||
                        cNext == cCR)
                        break;
                    else if(cNext == cCloseBracket)
                    {
                        break;
                    }
                }
                if(nSrchPos > nActPos + 1)
                {
                    //irgend ein String wurde gefunden
                    String sToken = rSource.Copy(nActPos + 1, nSrchPos - nActPos - 1 );
                    sToken.ToUpperAscii();
                    int nToken = ::GetHTMLToken(sToken);
                    if(nToken)
                    {
                        //Token gefunden
                        eFoundType = TT_KEYWORD;
                        nPortEnd = nSrchPos;
                        nPortStart = nActPos;
                    }
                    else
                    {
                        //was war das denn?
#ifdef DEBUG
                        DBG_ERROR("Token nicht erkannt!")
                        DBG_ERROR(ByteString(sToken, gsl_getSystemTextEncoding()).GetBuffer())
#endif
                    }

                }
            }
            // jetzt muss noch '>' gesucht werden
            if(TT_UNKNOWN != eFoundType)
            {
                BOOL bFound = FALSE;
                for(USHORT i = nPortEnd; i < nStrLen; i++)
                    if(cCloseBracket == rSource.GetChar(i))
                    {
                        bFound = TRUE;
                        nPortEnd = i;
                        break;
                    }
                if(!bFound && (eFoundType == TT_COMMENT))
                {
                    // Kommentar ohne Ende in dieser Zeile
                    bFound  = TRUE;
                    nPortEnd = nStrLen - 1;
                }

                if(bFound ||(eFoundType == TT_COMMENT))
                {
                    SwTextPortion aText;
                    aText.nLine = 0;
                    aText.nStart = nPortStart + 1;
                    aText.nEnd = nPortEnd;
                    aText.eType = eFoundType;
                    aPortionList.Insert(aText, nInsert++);
                    eFoundType = TT_UNKNOWN;
                }

            }
        }
        nActPos++;
    }
    if(nInsert && nPortEnd < nActPos - 1)
    {
        aText.nLine = 0;
        aText.nStart = nPortEnd + 1;
        aText.nEnd = nActPos - 1;
        aText.eType = TT_UNKNOWN;
        aPortionList.Insert(aText, nInsert++);
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwSrcEditWindow::SwSrcEditWindow( Window* pParent, SwSrcView* pParentView ) :
    Window( pParent, WB_BORDER|WB_CLIPCHILDREN ),
    pTextEngine(0),
    pOutWin(0),
    pHScrollbar(0),
    pVScrollbar(0),
    pSrcView(pParentView),
    nCurTextWidth(0),
    bDoSyntaxHighlight(TRUE),
    bHighlighting(FALSE),
    pSrcVwConfig(SW_MOD()->GetSourceViewConfig()),
    nStartLine(USHRT_MAX)
{
    SetHelpId(HID_SOURCE_EDITWIN);
    CreateTextEngine();
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


 SwSrcEditWindow::~SwSrcEditWindow()
{
    aSyntaxIdleTimer.Stop();
    if ( pTextEngine )
    {
        EndListening( *pTextEngine );
        pTextEngine->RemoveView( pTextView );

        delete pHScrollbar;
        delete pVScrollbar;

        delete pTextView;
        delete pTextEngine;
    }
    delete pOutWin;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

#if (SUPD>397)
void SwSrcEditWindow::DataChanged( const DataChangedEvent& )
{
    Resize();
}
#endif

void  SwSrcEditWindow::Resize()
{
    // ScrollBars, etc. passiert in Adjust...
    if ( pTextView )
    {
        long nVisY = pTextView->GetStartDocPos().Y();
        pTextView->ShowCursor();
        Size aOutSz( GetOutputSizePixel() );
        long nMaxVisAreaStart = pTextView->GetTextEngine()->GetTextHeight() - aOutSz.Height();
        if ( nMaxVisAreaStart < 0 )
            nMaxVisAreaStart = 0;
        if ( pTextView->GetStartDocPos().Y() > nMaxVisAreaStart )
        {
            Point aStartDocPos( pTextView->GetStartDocPos() );
            aStartDocPos.Y() = nMaxVisAreaStart;
            pTextView->SetStartDocPos( aStartDocPos );
            pTextView->ShowCursor();
        }
        InitScrollBars();
        long nScrollStd = GetSettings().GetStyleSettings().GetScrollBarSize();
        Size aScrollSz(aOutSz.Width() - nScrollStd, nScrollStd );
        Point aScrollPos(0, aOutSz.Height() - nScrollStd);

        pHScrollbar->SetPosSizePixel( aScrollPos, aScrollSz);

        aScrollSz.Width() = aScrollSz.Height();
        aScrollSz.Height() = aOutSz.Height();
        aScrollPos = Point(aOutSz.Width() - nScrollStd, 0);

        pVScrollbar->SetPosSizePixel( aScrollPos, aScrollSz);
        aOutSz.Width()  -= nScrollStd;
        aOutSz.Height()     -= nScrollStd;
        pOutWin->SetOutputSizePixel(aOutSz);
        // Zeile im ersten Resize setzen
        if(USHRT_MAX != nStartLine)
        {
            if(nStartLine < pTextEngine->GetParagraphCount())
            {
                TextSelection aSel(TextPaM( nStartLine, 0 ), TextPaM( nStartLine, 0x0 ));
                pTextView->SetSelection(aSel);
                pTextView->ShowCursor();
            }
            nStartLine = USHRT_MAX;
        }

        if ( nVisY != pTextView->GetStartDocPos().Y() )
            Invalidate();
    }

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void TextViewOutWin::DataChanged( const DataChangedEvent& )
{
    const Color &rCol = GetSettings().GetStyleSettings().GetWindowColor();
    SetBackground( rCol );
    Font aFont( pTextView->GetTextEngine()->GetFont() );
    aFont.SetFillColor( rCol );
    pTextView->GetTextEngine()->SetFont( aFont );
}

void  TextViewOutWin::MouseMove( const MouseEvent &rEvt )
{
    if ( pTextView )
        pTextView->MouseMove( rEvt );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void  TextViewOutWin::MouseButtonUp( const MouseEvent &rEvt )
{
    if ( pTextView )
    {
        pTextView->MouseButtonUp( rEvt );
        ((SwSrcEditWindow*)GetParent())->GetSrcView()->GetViewFrame()->
                        GetBindings().Invalidate( SID_TABLE_CELL );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void  TextViewOutWin::MouseButtonDown( const MouseEvent &rEvt )
{
    GrabFocus();
    if ( pTextView )
        pTextView->MouseButtonDown( rEvt );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void  TextViewOutWin::Command( const CommandEvent& rCEvt )
{
    switch(rCEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
            ((SwSrcEditWindow*)GetParent())->GetSrcView()->GetViewFrame()->
                GetDispatcher()->ExecutePopup();
        break;
        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        {
            const CommandWheelData* pWData = rCEvt.GetWheelData();
            if( !pWData || COMMAND_WHEEL_ZOOM != pWData->GetMode() )
            {
                ((SwSrcEditWindow*)GetParent())->HandleWheelCommand( rCEvt );
            }
        }
        break;

        default:
            if ( pTextView )
            pTextView->Command( rCEvt );
        else
            Window::Command(rCEvt);
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


BOOL  TextViewOutWin::Drop( const DropEvent& rEvt )
{
    BOOL bDone = FALSE;
    if ( pTextView )
    {
        bDone = pTextView->Drop( rEvt );
        if ( bDone )
        {
            SfxBindings& rBindings = ((SwSrcEditWindow*)GetParent())->
                            GetSrcView()->GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_TABLE_CELL );
            rBindings.Invalidate( SID_SAVEDOC );
            rBindings.Invalidate( SID_DOC_MODIFIED );
        }
    }
    return bDone;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


BOOL  TextViewOutWin::QueryDrop( DropEvent& rEvt )
{
    if ( pTextView && !((SwSrcEditWindow*)GetParent())->IsReadonly() )
        return pTextView->QueryDrop( rEvt );
    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void  TextViewOutWin::KeyInput( const KeyEvent& rKEvt )
{
    BOOL bDone = FALSE;
    SwSrcEditWindow* pSrcEditWin = (SwSrcEditWindow*)GetParent();
    BOOL bChange = !pSrcEditWin->IsReadonly() || !TextEngine::DoesKeyChangeText( rKEvt );
    if(bChange)
        bDone = pTextView->KeyInput( rKEvt );

    if ( !bDone )
    {
        if ( !SfxViewShell::Current()->KeyInput( rKEvt ) )
            Window::KeyInput( rKEvt );
    }
    else
    {
        SfxBindings& rBindings = ((SwSrcEditWindow*)GetParent())->GetSrcView()->GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_TABLE_CELL );
        if ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_CURSOR )
            rBindings.Update( SID_BASICIDE_STAT_POS );
        if (pSrcEditWin->GetTextEngine()->IsModified() )
        {
            rBindings.Invalidate( SID_SAVEDOC );
            rBindings.Invalidate( SID_DOC_MODIFIED );
        }
        if( rKEvt.GetKeyCode().GetCode() == KEY_INSERT )
            rBindings.Invalidate( SID_ATTR_INSERT );
    }
    SwDocShell* pDocShell = pSrcEditWin->GetSrcView()->GetDocShell();
    if(pSrcEditWin->GetTextEngine()->IsModified())
    {
        pDocShell->SetModified();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void  TextViewOutWin::Paint( const Rectangle& rRect )
{
    pTextView->Paint( rRect );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwSrcEditWindow::CreateTextEngine()
{
    const Color &rCol = GetSettings().GetStyleSettings().GetWindowColor();
    pOutWin = new TextViewOutWin(this, 0);
    pOutWin->SetBackground(Wallpaper(rCol));
    pOutWin->SetPointer(Pointer(POINTER_TEXT));
    pOutWin->Show();

    //Scrollbars anlegen
    pHScrollbar = new ScrollBar(this, WB_3DLOOK |WB_HSCROLL|WB_DRAG);
    pHScrollbar->SetScrollHdl(LINK(this, SwSrcEditWindow, ScrollHdl));
    pHScrollbar->Show();

    pVScrollbar = new ScrollBar(this, WB_3DLOOK |WB_VSCROLL|WB_DRAG);
    pVScrollbar->SetScrollHdl(LINK(this, SwSrcEditWindow, ScrollHdl));
    pHScrollbar->EnableDrag();
    pVScrollbar->Show();

    pTextEngine = new ExtTextEngine;
    pTextView = new ExtTextView( pTextEngine, pOutWin );
    pTextView->SetAutoIndentMode(TRUE);
    pOutWin->SetTextView(pTextView);

    pTextEngine->SetUpdateMode( FALSE );
    pTextEngine->InsertView( pTextView );

    Font aFont;
    aFont.SetCharSet( gsl_getSystemTextEncoding() );
    aFont.SetSize( Size( 0, 12 ) );
    aFont.SetPitch( PITCH_VARIABLE );
    aFont.SetWeight( WEIGHT_NORMAL );
    aFont.SetName( String::CreateFromAscii("Courier") );
    aFont.SetFamily( FAMILY_MODERN );
    aFont.SetPitch( PITCH_FIXED );
//  Font aFont( System::GetStandardFont( STDFONT_FIXED ) );
    aFont.SetTransparent( FALSE );

    Size aFontSize( 0, 10 );

#ifdef MAC
    aFont.SetName( "Monaco" );
#endif

    aFont.SetFillColor( rCol );
    aFont.SetSize( aFontSize );
    SetPointFont( aFont );
    aFont = GetFont();
    aFont.SetFillColor( rCol );
    pOutWin->SetFont( aFont );

    pTextEngine->SetFont( aFont );


    aSyntaxIdleTimer.SetTimeout( 200 );
    aSyntaxIdleTimer.SetTimeoutHdl( LINK( this, SwSrcEditWindow, SyntaxTimerHdl ) );

    pTextEngine->EnableUndo( TRUE );
    pTextEngine->SetUpdateMode( TRUE );

    pTextView->ShowCursor( TRUE, TRUE );
    InitScrollBars();
    StartListening( *pTextEngine );

    SfxBindings& rBind = GetSrcView()->GetViewFrame()->GetBindings();
    rBind.Invalidate( SID_TABLE_CELL );
    rBind.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwSrcEditWindow::SetScrollBarRanges()
{
    // Extra-Methode, nicht InitScrollBars, da auch fuer TextEngine-Events.

    pHScrollbar->SetRange( Range( 0, nCurTextWidth ) );
    pVScrollbar->SetRange( Range(0, pTextEngine->GetTextHeight()) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwSrcEditWindow::InitScrollBars()
{
    SetScrollBarRanges();

    Size aOutSz( GetOutputSizePixel() );
    pVScrollbar->SetVisibleSize( aOutSz.Height() );
    pVScrollbar->SetPageSize(  aOutSz.Height() * 8 / 10 );
    pVScrollbar->SetLineSize( pOutWin->GetTextHeight() );
    pVScrollbar->SetThumbPos( pTextView->GetStartDocPos().Y() );
    pHScrollbar->SetVisibleSize( aOutSz.Width() );
    pHScrollbar->SetPageSize( aOutSz.Width() * 8 / 10 );
    pHScrollbar->SetLineSize( pOutWin->GetTextWidth( 'x' ) );
    pHScrollbar->SetThumbPos( pTextView->GetStartDocPos().X() );

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


IMPL_LINK(SwSrcEditWindow, ScrollHdl, ScrollBar*, pScroll)
{
    if(pScroll == pVScrollbar)
    {
        long nDiff = pTextView->GetStartDocPos().Y() - pScroll->GetThumbPos();
        GetTextView()->Scroll( 0, nDiff );
        pTextView->ShowCursor( FALSE, TRUE );
        pScroll->SetThumbPos( pTextView->GetStartDocPos().Y() );
    }
    else
    {
        long nDiff = pTextView->GetStartDocPos().X() - pScroll->GetThumbPos();
        GetTextView()->Scroll( nDiff, 0 );
        pTextView->ShowCursor( FALSE, TRUE );
        pScroll->SetThumbPos( pTextView->GetStartDocPos().X() );
    }
    GetSrcView()->GetViewFrame()->GetBindings().Invalidate( SID_TABLE_CELL );
    return 0;
}

/*-----------------15.01.97 09.22-------------------

--------------------------------------------------*/

IMPL_LINK( SwSrcEditWindow, SyntaxTimerHdl, Timer *, pTimer )
{
    DBG_ASSERT( pTextView, "Noch keine View, aber Syntax-Highlight ?!" );
    pTextEngine->SetUpdateMode( FALSE );

    bHighlighting = TRUE;
    USHORT nLine;
    USHORT nCount  = 0;
    // zuerst wird der Bereich um dem Cursor bearbeitet
    TextSelection aSel = pTextView->GetSelection();
    USHORT nCur = aSel.GetStart().GetPara();
    if(nCur > 40)
        nCur -= 40;
    else
        nCur = 0;
    if(aSyntaxLineTable.Count())
        for(USHORT i = 0; i < 80 && nCount < 40; i++, nCur++)
        {
            void * p = aSyntaxLineTable.Get(nCur);
            if(p)
            {
                DoSyntaxHighlight( nCur );
                aSyntaxLineTable.Remove( nCur );
                nCount++;
                if(!aSyntaxLineTable.Count())
                    break;
            }
        }

    // wenn dann noch etwas frei ist, wird von Beginn an weitergearbeitet
    void* p = aSyntaxLineTable.First();
    while ( p && nCount < MAX_SYNTAX_HIGHLIGHT)
    {
        nLine = (USHORT)aSyntaxLineTable.GetCurKey();
        DoSyntaxHighlight( nLine );
        USHORT nCur = (USHORT)aSyntaxLineTable.GetCurKey();
        p = aSyntaxLineTable.Next();
        aSyntaxLineTable.Remove(nCur);
        nCount ++;
    }
    // os: #43050# hier wird ein TextView-Problem umpopelt:
    // waehrend des Highlightings funktionierte das Scrolling nicht
    TextView* pTmp = pTextEngine->GetActiveView();
    pTextEngine->SetActiveView(0);
    pTextEngine->SetUpdateMode( TRUE );
    pTextEngine->SetActiveView(pTmp);
    pTextView->ShowCursor(FALSE, FALSE);

    if(aSyntaxLineTable.Count() && !pTimer->IsActive())
        pTimer->Start();
    // SyntaxTimerHdl wird gerufen, wenn Text-Aenderung
    // => gute Gelegenheit, Textbreite zu ermitteln!
    long nPrevTextWidth = nCurTextWidth;
    nCurTextWidth = pTextEngine->CalcTextWidth() + 25;  // kleine Toleranz
    if ( nCurTextWidth != nPrevTextWidth )
        SetScrollBarRanges();
    bHighlighting = FALSE;

    return 0;
}
/*-----------------15.01.97 10.01-------------------

--------------------------------------------------*/

void SwSrcEditWindow::DoSyntaxHighlight( USHORT nPara )
{
    // Durch das DelayedSyntaxHighlight kann es passieren,
    // dass die Zeile nicht mehr existiert!
    if ( nPara < pTextEngine->GetParagraphCount() )
    {
        BOOL bTempModified = IsModified();
        pTextEngine->RemoveAttribs( nPara );
        String aSource( pTextEngine->GetText( nPara ) );
        pTextEngine->SetUpdateMode( FALSE );
        ImpDoHighlight( aSource, nPara );
        // os: #43050# hier wird ein TextView-Problem umpopelt:
        // waehrend des Highlightings funktionierte das Scrolling nicht
        TextView* pTmp = pTextEngine->GetActiveView();
        pTmp->SetAutoScroll(FALSE);
        pTextEngine->SetActiveView(0);
        pTextEngine->SetUpdateMode( TRUE );
        pTextEngine->SetActiveView(pTmp);
        // Bug 72887 show the cursor
        pTmp->SetAutoScroll(TRUE);
        pTmp->ShowCursor( FALSE/*pTmp->IsAutoScroll()*/ );

        if(!bTempModified)
            ClearModifyFlag();
    }
}

/*-----------------15.01.97 09.49-------------------

--------------------------------------------------*/

void SwSrcEditWindow::DoDelayedSyntaxHighlight( USHORT nPara )
{
    if ( !bHighlighting && bDoSyntaxHighlight )
    {
        aSyntaxLineTable.Insert( nPara, (void*)(USHORT)1 );
        aSyntaxIdleTimer.Start();
    }
}

/*-----------------15.01.97 11.32-------------------

--------------------------------------------------*/

void SwSrcEditWindow::ImpDoHighlight( const String& rSource, USHORT nLineOff )
{
    SwTextPortions aPortionList;
    lcl_Highlight(rSource, aPortionList);

    USHORT nCount = aPortionList.Count();
    if ( !nCount )
        return;

    SwTextPortion& rLast = aPortionList[nCount-1];
    if ( rLast.nStart > rLast.nEnd )    // Nur bis Bug von MD behoeben
    {
        nCount--;
        aPortionList.Remove( nCount);
        if ( !nCount )
            return;
    }

    // Evtl. Optimieren:
    // Wenn haufig gleiche Farbe, dazwischen Blank ohne Farbe,
    // ggf. zusammenfassen, oder zumindest das Blank,
    // damit weniger Attribute
    BOOL bOptimizeHighlight = TRUE; // war in der BasicIDE static
    if ( bOptimizeHighlight )
    {
        // Es muessen nur die Blanks und Tabs mit attributiert werden.
        // Wenn zwei gleiche Attribute hintereinander eingestellt werden,
        // optimiert das die TextEngine.
        USHORT nLastEnd = 0;
        USHORT nLine = aPortionList[0].nLine;
        for ( USHORT i = 0; i < nCount; i++ )
        {
            SwTextPortion& r = aPortionList[i];
            DBG_ASSERT( r.nLine == nLine, "doch mehrere Zeilen ?" );
            if ( r.nStart > r.nEnd )    // Nur bis Bug von MD behoeben
                continue;

            if ( r.nStart > nLastEnd )
            {
                // Kann ich mich drauf verlassen, dass alle ausser
                // Blank und Tab gehighlightet wird ?!
                r.nStart = nLastEnd;
            }
            nLastEnd = r.nEnd+1;
            if ( ( i == (nCount-1) ) && ( r.nEnd < rSource.Len() ) )
                r.nEnd = rSource.Len();
        }
    }

    for ( USHORT i = 0; i < aPortionList.Count(); i++ )
    {
        SwTextPortion& r = aPortionList[i];
        if ( r.nStart > r.nEnd )    // Nur bis Bug von MD behoeben
            continue;
        USHORT nCol = r.eType;
        DBG_ASSERT( nCol < SYNTAX_COLOR_MAX, "Neue Syntax-Farbe?" );
        if ( nCol < SYNTAX_COLOR_MAX )
        {
            Color aColor = pSrcVwConfig->GetSyntaxColor(nCol);
            USHORT nLine = nLineOff+r.nLine; //
            pTextEngine->SetAttrib( TextAttribFontColor( aColor ), nLine, r.nStart, r.nEnd+1 );
        }
    }
}
/*-----------------21.04.97 09:42-------------------

--------------------------------------------------*/
void SwSrcEditWindow::SyntaxColorsChanged()
{
    for(USHORT i = 0; i < pTextEngine->GetParagraphCount(); i++)
        aSyntaxLineTable.Insert( i, (void*)(USHORT)1 );
    if(!aSyntaxIdleTimer.IsActive())
        aSyntaxIdleTimer.Start();

}
/*-----------------30.06.97 09:12-------------------

--------------------------------------------------*/

void SwSrcEditWindow::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( TextHint ) )
    {
        const TextHint& rTextHint = (const TextHint&)rHint;
        if( rTextHint.GetId() == TEXT_HINT_VIEWSCROLLED )
        {
            pHScrollbar->SetThumbPos( pTextView->GetStartDocPos().X() );
            pVScrollbar->SetThumbPos( pTextView->GetStartDocPos().Y() );
        }
        else if( rTextHint.GetId() == TEXT_HINT_TEXTHEIGHTCHANGED )
        {
            if ( (long)pTextEngine->GetTextHeight() < pOutWin->GetOutputSizePixel().Height() )
                pTextView->Scroll( 0, pTextView->GetStartDocPos().Y() );
            pVScrollbar->SetThumbPos( pTextView->GetStartDocPos().Y() );
            SetScrollBarRanges();
        }
        else if( rTextHint.GetId() == TEXT_HINT_FORMATPARA )
        {
            DoDelayedSyntaxHighlight( rTextHint.GetValue() );
        }
    }
}

/*-----------------30.06.97 13:22-------------------

--------------------------------------------------*/

void    SwSrcEditWindow::Invalidate()
{
    pOutWin->Invalidate();
    Window::Invalidate();

}

void SwSrcEditWindow::Command( const CommandEvent& rCEvt )
{
    switch(rCEvt.GetCommand())
    {
        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        {
            const CommandWheelData* pWData = rCEvt.GetWheelData();
            if( !pWData || COMMAND_WHEEL_ZOOM != pWData->GetMode() )
                HandleScrollCommand( rCEvt, pHScrollbar, pVScrollbar );
        }
        break;
        default:
            Window::Command(rCEvt);
    }
}

void SwSrcEditWindow::HandleWheelCommand( const CommandEvent& rCEvt )
{
    pTextView->Command(rCEvt);
    HandleScrollCommand( rCEvt, pHScrollbar, pVScrollbar );
}

void SwSrcEditWindow::GetFocus()
{
//  rView.GotFocus();
    pOutWin->GrabFocus();
//  pOutWin->GetFocus();
//  Window::GetFocus();
}

/*void SwSrcEditWindow::LoseFocus()
{
    Window::LoseFocus();
//  pOutWin->LoseFocus();
//  rView.LostFocus();
} */

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.55  2000/09/18 16:05:24  willem.vandorp
    OpenOffice header added.

    Revision 1.54  2000/09/14 10:36:36  os
    GetBindings()  access corrected

    Revision 1.53  2000/09/07 15:59:22  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.52  2000/06/23 07:49:49  os
    #75591# disable AutoScroll while syntax highlighting

    Revision 1.51  2000/04/18 15:18:17  os
    UNICODE

    Revision 1.50  2000/03/03 15:17:00  os
    StarView remainders removed

    Revision 1.49  2000/02/14 14:48:40  os
    #70473# Unicode

    Revision 1.48  2000/02/10 20:45:06  jp
    Bug #72887#: ShowCursor in HTML code view

    Revision 1.47  1999/11/10 14:25:10  os
    KeyInput: Assertion removed

    Revision 1.46  1999/05/21 13:14:40  OS
    #55722# Scrolling waehrend Syntax-Highlight


      Rev 1.45   21 May 1999 15:14:40   OS
   #55722# Scrolling waehrend Syntax-Highlight

      Rev 1.44   02 Dec 1998 16:30:22   OS
   #66066# Hintergrund- und Font-Farbe initial aus den Systemeinstellungen holen

      Rev 1.43   18 Aug 1998 13:38:14   OS
   Mouse auf Raedern eingebaut #55166#

      Rev 1.42   29 Jun 1998 12:56:52   OS
   SW_MOD statt GetActiveModule

      Rev 1.41   24 Jun 1998 18:43:08   MA
   DataChanged fuer ScrollBar und Retouche, Retouche ganz umgestellt

      Rev 1.40   02 Apr 1998 11:38:08   OS
   neue HelpId

      Rev 1.39   24 Mar 1998 15:59:28   OS
   AutoIndent fuer TextEngine

      Rev 1.38   19 Mar 1998 18:11:28   OS
   ShowCursor nach dem Highlight #48789#

      Rev 1.37   18 Mar 1998 15:05:48   OS
   Fontgroesse berichtigt #47428#

------------------------------------------------------------------------*/

