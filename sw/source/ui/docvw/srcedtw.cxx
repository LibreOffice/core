/*************************************************************************
 *
 *  $RCSfile: srcedtw.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2003-11-12 17:13:05 $
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


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
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
#ifndef _SVTOOLS_SOURCEVIEWCONFIG_HXX
#include <svtools/sourceviewconfig.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SRCVIEW_HXX
#include <srcview.hxx>
#endif
#ifndef _SRCEDTW_HXX
#include <srcedtw.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif


enum SwHtmlTextType {
    TT_SGML     = svtools::HTMLSGML   ,
    TT_COMMENT  = svtools::HTMLCOMMENT,
    TT_KEYWORD  = svtools::HTMLKEYWORD,
    TT_UNKNOWN  = svtools::HTMLUNKNOWN
};


struct SwTextPortion
{
    USHORT nLine;
    USHORT nStart, nEnd;
    SwHtmlTextType eType;
};

#define MAX_SYNTAX_HIGHLIGHT 20
#define MAX_HIGHLIGHTTIME 200
#define SYNTAX_HIGHLIGHT_TIMEOUT 200

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
#if OSL_DEBUG_LEVEL > 1
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
    pSourceViewConfig(new svt::SourceViewConfig),
    nCurTextWidth(0),
    bDoSyntaxHighlight(TRUE),
    bHighlighting(FALSE),
    nStartLine(USHRT_MAX),
    eSourceEncoding(gsl_getSystemTextEncoding())
{
    SetHelpId(HID_SOURCE_EDITWIN);
    CreateTextEngine();
    StartListening(*pSourceViewConfig);
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
 SwSrcEditWindow::~SwSrcEditWindow()
{
    EndListening(*pSourceViewConfig);
    delete pSourceViewConfig;
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

void SwSrcEditWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    switch ( rDCEvt.GetType() )
    {
    case DATACHANGED_SETTINGS:
        // ScrollBars neu anordnen bzw. Resize ausloesen, da sich
        // ScrollBar-Groesse geaendert haben kann. Dazu muss dann im
        // Resize-Handler aber auch die Groesse der ScrollBars aus
        // den Settings abgefragt werden.
        if( rDCEvt.GetFlags() & SETTINGS_STYLE )
            Resize();
        break;
    }
}

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
        InitScrollBars();

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

void TextViewOutWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    switch( rDCEvt.GetType() )
    {
    case DATACHANGED_SETTINGS:
        // den Settings abgefragt werden.
        if( rDCEvt.GetFlags() & SETTINGS_STYLE )
        {
            const Color &rCol = GetSettings().GetStyleSettings().GetWindowColor();
            SetBackground( rCol );
            Font aFont( pTextView->GetTextEngine()->GetFont() );
            aFont.SetFillColor( rCol );
            pTextView->GetTextEngine()->SetFont( aFont );
        }
        break;
    }
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
        pHScrollbar->EnableRTL( false ); // #107300# --- RTL --- no mirroring for scrollbars
    pHScrollbar->SetScrollHdl(LINK(this, SwSrcEditWindow, ScrollHdl));
    pHScrollbar->Show();

    pVScrollbar = new ScrollBar(this, WB_3DLOOK |WB_VSCROLL|WB_DRAG);
        pVScrollbar->EnableRTL( false ); // #107300# --- RTL --- no mirroring for scrollbars
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
    aFont.SetTransparent( FALSE );
    aFont.SetFillColor( rCol );
    SetPointFont( aFont );
    aFont = GetFont();
    aFont.SetFillColor( rCol );
    pOutWin->SetFont( aFont );
    pTextEngine->SetFont( aFont );

    aSyntaxIdleTimer.SetTimeout( SYNTAX_HIGHLIGHT_TIMEOUT );
    aSyntaxIdleTimer.SetTimeoutHdl( LINK( this, SwSrcEditWindow, SyntaxTimerHdl ) );

    pTextEngine->EnableUndo( TRUE );
    pTextEngine->SetUpdateMode( TRUE );

    pTextView->ShowCursor( TRUE, TRUE );
    InitScrollBars();
    StartListening( *pTextEngine );

    SfxBindings& rBind = GetSrcView()->GetViewFrame()->GetBindings();
    rBind.Invalidate( SID_TABLE_CELL );
//  rBind.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
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

    pHScrollbar->SetRange( Range( 0, nCurTextWidth-1 ) );
    pVScrollbar->SetRange( Range(0, pTextEngine->GetTextHeight()-1) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwSrcEditWindow::InitScrollBars()
{
    SetScrollBarRanges();

    Size aOutSz( pOutWin->GetOutputSizePixel() );
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
    Time aSyntaxCheckStart;
    DBG_ASSERT( pTextView, "Noch keine View, aber Syntax-Highlight ?!" );
    // pTextEngine->SetUpdateMode( FALSE );

    bHighlighting = TRUE;
    USHORT nLine;
    USHORT nCount  = 0;
    // zuerst wird der Bereich um dem Cursor bearbeitet
    TextSelection aSel = pTextView->GetSelection();
    USHORT nCur = (USHORT)aSel.GetStart().GetPara();
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
                if((Time().GetTime() - aSyntaxCheckStart.GetTime()) > MAX_HIGHLIGHTTIME )
                {
                    pTimer->SetTimeout( 2 * SYNTAX_HIGHLIGHT_TIMEOUT );
                    break;
                }
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
        if(Time().GetTime() - aSyntaxCheckStart.GetTime() > MAX_HIGHLIGHTTIME)
        {
            pTimer->SetTimeout( 2 * SYNTAX_HIGHLIGHT_TIMEOUT );
            break;
        }
    }
    // os: #43050# hier wird ein TextView-Problem umpopelt:
    // waehrend des Highlightings funktionierte das Scrolling nicht
    /* MT: Shouldn't be a oproblem any more, using IdeFormatter in Insert/RemoveAttrib now.

        TextView* pTmp = pTextEngine->GetActiveView();
        pTextEngine->SetActiveView(0);
        // pTextEngine->SetUpdateMode( TRUE );
        pTextEngine->SetActiveView(pTmp);
        pTextView->ShowCursor(FALSE, FALSE);
    */

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
        pTextEngine->RemoveAttribs( nPara, TRUE );
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
        if(r.eType !=  svtools::HTMLSGML    &&
            r.eType != svtools::HTMLCOMMENT &&
            r.eType != svtools::HTMLKEYWORD &&
            r.eType != svtools::HTMLUNKNOWN)
                r.eType = (SwHtmlTextType)svtools::HTMLUNKNOWN;
        Color aColor((ColorData)SW_MOD()->GetColorConfig().GetColorValue((svtools::ColorConfigEntry)r.eType).nColor);
        USHORT nLine = nLineOff+r.nLine; //
        pTextEngine->SetAttrib( TextAttribFontColor( aColor ), nLine, r.nStart, r.nEnd+1, TRUE );
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
        else if( ( rTextHint.GetId() == TEXT_HINT_PARAINSERTED ) ||
                 ( rTextHint.GetId() == TEXT_HINT_PARACONTENTCHANGED ) )
        {
            DoDelayedSyntaxHighlight( (USHORT)rTextHint.GetValue() );
        }
    }
    else if(&rBC == pSourceViewConfig)
        SetFont();
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
    pOutWin->GrabFocus();
}

/*void SwSrcEditWindow::LoseFocus()
{
    Window::LoseFocus();
//  pOutWin->LoseFocus();
//  rView.LostFocus();
} */
/* -----------------------------29.08.2002 13:21------------------------------

 ---------------------------------------------------------------------------*/
BOOL  lcl_GetLanguagesForEncoding(rtl_TextEncoding eEnc, LanguageType aLanguages[])
{
    switch(eEnc)
    {
        case RTL_TEXTENCODING_UTF7             :
        case RTL_TEXTENCODING_UTF8             :
            // don#t fill - all LANGUAGE_SYSTEM means unicode font has to be used
        break;


        case RTL_TEXTENCODING_ISO_8859_3:
        case RTL_TEXTENCODING_ISO_8859_1  :
        case RTL_TEXTENCODING_MS_1252     :
        case RTL_TEXTENCODING_APPLE_ROMAN :
        case RTL_TEXTENCODING_IBM_850     :
        case RTL_TEXTENCODING_ISO_8859_14 :
        case RTL_TEXTENCODING_ISO_8859_15 :
            //fill with western languages
            aLanguages[0] = LANGUAGE_GERMAN;
            aLanguages[1] = LANGUAGE_FRENCH;
            aLanguages[2] = LANGUAGE_ITALIAN;
            aLanguages[3] = LANGUAGE_SPANISH;
        break;

        case RTL_TEXTENCODING_IBM_865     :
            //scandinavian
            aLanguages[0] = LANGUAGE_FINNISH;
            aLanguages[1] = LANGUAGE_NORWEGIAN;
            aLanguages[2] = LANGUAGE_SWEDISH;
            aLanguages[3] = LANGUAGE_DANISH;
        break;

        case RTL_TEXTENCODING_ISO_8859_10      :
        case RTL_TEXTENCODING_ISO_8859_13      :
        case RTL_TEXTENCODING_ISO_8859_2  :
        case RTL_TEXTENCODING_IBM_852     :
        case RTL_TEXTENCODING_MS_1250     :
        case RTL_TEXTENCODING_APPLE_CENTEURO   :
            aLanguages[0] = LANGUAGE_POLISH;
            aLanguages[1] = LANGUAGE_CZECH;
            aLanguages[2] = LANGUAGE_HUNGARIAN;
            aLanguages[3] = LANGUAGE_SLOVAK;
        break;

        case RTL_TEXTENCODING_ISO_8859_4  :
        case RTL_TEXTENCODING_IBM_775     :
        case RTL_TEXTENCODING_MS_1257          :
            aLanguages[0] = LANGUAGE_LATVIAN   ;
            aLanguages[1] = LANGUAGE_LITHUANIAN;
            aLanguages[2] = LANGUAGE_ESTONIAN  ;
        break;

        case RTL_TEXTENCODING_IBM_863       : aLanguages[0] = LANGUAGE_FRENCH_CANADIAN; break;
        case RTL_TEXTENCODING_APPLE_FARSI   : aLanguages[0] = LANGUAGE_FARSI; break;
        case RTL_TEXTENCODING_APPLE_ROMANIAN:aLanguages[0] = LANGUAGE_ROMANIAN; break;

        case RTL_TEXTENCODING_IBM_861     :
        case RTL_TEXTENCODING_APPLE_ICELAND    :
            aLanguages[0] = LANGUAGE_ICELANDIC;
        break;

        case RTL_TEXTENCODING_APPLE_CROATIAN:aLanguages[0] = LANGUAGE_CROATIAN; break;

        case RTL_TEXTENCODING_IBM_437     :
        case RTL_TEXTENCODING_ASCII_US    : aLanguages[0] = LANGUAGE_ENGLISH; break;

        case RTL_TEXTENCODING_IBM_862     :
        case RTL_TEXTENCODING_MS_1255     :
        case RTL_TEXTENCODING_APPLE_HEBREW     :
        case RTL_TEXTENCODING_ISO_8859_8  :
            aLanguages[0] = LANGUAGE_HEBREW;
        break;

        case RTL_TEXTENCODING_IBM_857     :
        case RTL_TEXTENCODING_MS_1254     :
        case RTL_TEXTENCODING_APPLE_TURKISH:
        case RTL_TEXTENCODING_ISO_8859_9  :
            aLanguages[0] = LANGUAGE_TURKISH;
        break;

        case RTL_TEXTENCODING_IBM_860     :
            aLanguages[0] = LANGUAGE_PORTUGUESE;
        break;

        case RTL_TEXTENCODING_IBM_869     :
        case RTL_TEXTENCODING_MS_1253     :
        case RTL_TEXTENCODING_APPLE_GREEK :
        case RTL_TEXTENCODING_ISO_8859_7  :
        case RTL_TEXTENCODING_IBM_737     :
            aLanguages[0] = LANGUAGE_GREEK;
        break;

        case RTL_TEXTENCODING_KOI8_R      :
        case RTL_TEXTENCODING_ISO_8859_5  :
        case RTL_TEXTENCODING_IBM_855     :
        case RTL_TEXTENCODING_MS_1251     :
        case RTL_TEXTENCODING_IBM_866     :
        case RTL_TEXTENCODING_APPLE_CYRILLIC   :
            aLanguages[0] = LANGUAGE_RUSSIAN;
        break;

        case RTL_TEXTENCODING_APPLE_UKRAINIAN:
        case RTL_TEXTENCODING_KOI8_U:
            aLanguages[0] = LANGUAGE_UKRAINIAN;
            break;

        case RTL_TEXTENCODING_IBM_864     :
        case RTL_TEXTENCODING_MS_1256          :
        case RTL_TEXTENCODING_ISO_8859_6  :
        case RTL_TEXTENCODING_APPLE_ARABIC :
            aLanguages[0] = LANGUAGE_ARABIC;
         break;

        case RTL_TEXTENCODING_APPLE_CHINTRAD   :
        case RTL_TEXTENCODING_MS_950           :
        case RTL_TEXTENCODING_GBT_12345        :
        case RTL_TEXTENCODING_BIG5             :
        case RTL_TEXTENCODING_EUC_TW           :
        case RTL_TEXTENCODING_BIG5_HKSCS       :
            aLanguages[0] = LANGUAGE_CHINESE_TRADITIONAL;
        break;

        case RTL_TEXTENCODING_EUC_JP           :
        case RTL_TEXTENCODING_ISO_2022_JP      :
        case RTL_TEXTENCODING_JIS_X_0201       :
        case RTL_TEXTENCODING_JIS_X_0208       :
        case RTL_TEXTENCODING_JIS_X_0212       :
        case RTL_TEXTENCODING_APPLE_JAPANESE   :
        case RTL_TEXTENCODING_MS_932           :
        case RTL_TEXTENCODING_SHIFT_JIS        :
            aLanguages[0] = LANGUAGE_JAPANESE;
        break;

        case RTL_TEXTENCODING_GB_2312          :
        case RTL_TEXTENCODING_MS_936           :
        case RTL_TEXTENCODING_GBK              :
        case RTL_TEXTENCODING_GB_18030         :
        case RTL_TEXTENCODING_APPLE_CHINSIMP   :
        case RTL_TEXTENCODING_EUC_CN           :
        case RTL_TEXTENCODING_ISO_2022_CN      :
            aLanguages[0] = LANGUAGE_CHINESE_SIMPLIFIED;
        break;

        case RTL_TEXTENCODING_APPLE_KOREAN     :
        case RTL_TEXTENCODING_MS_949           :
        case RTL_TEXTENCODING_EUC_KR           :
        case RTL_TEXTENCODING_ISO_2022_KR      :
        case RTL_TEXTENCODING_MS_1361          :
            aLanguages[0] = LANGUAGE_KOREAN;
        break;

        case RTL_TEXTENCODING_APPLE_THAI       :
        case RTL_TEXTENCODING_MS_874      :
        case RTL_TEXTENCODING_TIS_620          :
            aLanguages[0] = LANGUAGE_THAI;
        break;
//        case RTL_TEXTENCODING_SYMBOL      :
//        case RTL_TEXTENCODING_DONTKNOW:        :
        default: aLanguages[0] = Application::GetSettings().GetUILanguage();
    }
    return aLanguages[0] != LANGUAGE_SYSTEM;
}
void SwSrcEditWindow::SetFont()
{
    String sFontName = pSourceViewConfig->GetFontName();
    if(!sFontName.Len())
    {
        LanguageType aLanguages[5] =
        {
            LANGUAGE_SYSTEM, LANGUAGE_SYSTEM, LANGUAGE_SYSTEM, LANGUAGE_SYSTEM, LANGUAGE_SYSTEM
        };
        Font aFont;
        if(lcl_GetLanguagesForEncoding(eSourceEncoding, aLanguages))
        {
            //TODO: check for multiple languages
            aFont = OutputDevice::GetDefaultFont(DEFAULTFONT_FIXED, aLanguages[0], 0, this);
        }
        else
            aFont = OutputDevice::GetDefaultFont(DEFAULTFONT_SANS_UNICODE,
                        Application::GetSettings().GetLanguage(), 0, this);
        sFontName = aFont.GetName();
    }
    const SvxFontListItem* pFontListItem =
        (const SvxFontListItem* )pSrcView->GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST );
    const FontList*  pList = pFontListItem->GetFontList();
    FontInfo aInfo = pList->Get(sFontName,WEIGHT_NORMAL, ITALIC_NONE);

    const Font& rFont = GetTextEngine()->GetFont();
    Font aFont(aInfo);
    Size aSize(rFont.GetSize());
    //font height is stored in point and set in twip
    aSize.Height() = pSourceViewConfig->GetFontHeight() * 20;
    aFont.SetSize(pOutWin->LogicToPixel(aSize, MAP_TWIP));
    GetTextEngine()->SetFont( aFont );
    pOutWin->SetFont(aFont);
}
/* -----------------------------29.08.2002 13:47------------------------------

 ---------------------------------------------------------------------------*/
void SwSrcEditWindow::SetTextEncoding(rtl_TextEncoding eEncoding)
{
    eSourceEncoding = eEncoding;
    SetFont();
}

