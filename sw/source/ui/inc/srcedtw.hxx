/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SRCEDTW_HXX
#define _SRCEDTW_HXX

#include <vcl/window.hxx>
#include <svl/lstner.hxx>
#include <unotools/options.hxx>
#include <vcl/timer.hxx>

#include <tools/table.hxx>
#include <svtools/xtextedt.hxx>

class ScrollBar;
class SwSrcView;
class SwSrcEditWindow;
class TextEngine;
class ExtTextView;
class DataChangedEvent;

namespace utl
{
    class SourceViewConfig;
}

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
class SwSrcEditWindow : public Window, public SfxListener, public utl::ConfigurationListener
{
private:
    ExtTextView*    pTextView;
    ExtTextEngine*  pTextEngine;

    TextViewOutWin* pOutWin;
    ScrollBar       *pHScrollbar,
                    *pVScrollbar;

    SwSrcView*      pSrcView;
    utl::SourceViewConfig* pSourceViewConfig;

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

    using Window::Notify;
    using Window::Invalidate;

protected:

    virtual void    Resize();
    virtual void    DataChanged( const DataChangedEvent& );
    virtual void    GetFocus();
//  virtual void    LoseFocus();

    void            CreateTextEngine();
    void            DoSyntaxHighlight( USHORT nPara );

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void    ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
