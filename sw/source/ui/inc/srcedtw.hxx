/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srcedtw.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:09:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SRCEDTW_HXX
#define _SRCEDTW_HXX

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif

#ifndef _XTEXTEDT_HXX //autogen
#include <svtools/xtextedt.hxx>
#endif

class ScrollBar;
class SwSrcView;
class SwSrcEditWindow;
class TextEngine;
class ExtTextView;
class DataChangedEvent;

class TextViewOutWin : public Window
{
    ExtTextView*    pTextView;

protected:
    virtual void    Paint( const Rectangle& );
    virtual void    KeyInput( const KeyEvent& rKeyEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    DataChanged( const DataChangedEvent& );

public:
        TextViewOutWin(Window* pParent, WinBits nBits) :
            Window(pParent, nBits), pTextView(0){}

    void    SetTextView( ExtTextView* pView ) {pTextView = pView;}

};

//------------------------------------------------------------
namespace svt{ class SourceViewConfig;}
class SwSrcEditWindow : public Window, public SfxListener
{
private:
    ExtTextView*    pTextView;
    ExtTextEngine*  pTextEngine;

    TextViewOutWin* pOutWin;
    ScrollBar       *pHScrollbar,
                    *pVScrollbar;

    SwSrcView*      pSrcView;
    svt::SourceViewConfig* pSourceViewConfig;

    long            nCurTextWidth;
    USHORT          nStartLine;
    rtl_TextEncoding eSourceEncoding;
    BOOL            bReadonly;
    BOOL            bDoSyntaxHighlight;
    BOOL            bHighlighting;

    Timer           aSyntaxIdleTimer;
    Table           aSyntaxLineTable;

    void            ImpDoHighlight( const String& rSource, USHORT nLineOff );

    using OutputDevice::SetFont;
    void            SetFont();

    DECL_LINK( SyntaxTimerHdl, Timer * );
    DECL_LINK( TimeoutHdl, Timer * );

protected:

    virtual void    Resize();
    virtual void    DataChanged( const DataChangedEvent& );
    virtual void    GetFocus();
//  virtual void    LoseFocus();

    void            CreateTextEngine();
    void            DoSyntaxHighlight( USHORT nPara );

    using Window::Notify;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    DECL_LINK(ScrollHdl, ScrollBar*);

public:
                    SwSrcEditWindow( Window* pParent, SwSrcView* pParentView );
                    ~SwSrcEditWindow();

    void            SetScrollBarRanges();
    void            InitScrollBars();
    ULONG           Read( SvStream& rInput)
                        {return pTextEngine->Read(rInput);}
    ULONG           Write( SvStream& rOutput)
                        {return pTextEngine->Write(rOutput);}

    ExtTextView*    GetTextView()
                        {return pTextView;}
    TextEngine*     GetTextEngine()
                        {return pTextEngine;}
    SwSrcView*      GetSrcView() {return pSrcView;}

    TextViewOutWin* GetOutWin() {return pOutWin;}

    using Window::Invalidate;
    virtual void    Invalidate( USHORT nFlags = 0 );

    void            ClearModifyFlag()
                        { pTextEngine->SetModified(FALSE); }
    BOOL            IsModified() const
                        { return pTextEngine->IsModified();}
    void            CreateScrollbars();

    void            SetReadonly(BOOL bSet){bReadonly = bSet;}
    BOOL            IsReadonly(){return bReadonly;}

    void            DoDelayedSyntaxHighlight( USHORT nPara );

    void            SetStartLine(USHORT nLine){nStartLine = nLine;}

    virtual void    Command( const CommandEvent& rCEvt );
    void            HandleWheelCommand( const CommandEvent& rCEvt );

    void            SetTextEncoding(rtl_TextEncoding eEncoding);
};

#endif
