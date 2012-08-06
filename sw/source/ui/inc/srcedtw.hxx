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
#include <vcl/timer.hxx>

#include <vcl/xtextedt.hxx>
#include <set>

namespace com { namespace sun { namespace star { namespace beans {
    class XMultiPropertySet;
} } } }

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
typedef std::set<sal_uInt16> SyntaxLineSet;

class SwSrcEditWindow : public Window, public SfxListener
{
private:
    class ChangesListener;
    friend class ChangesListener;
    ExtTextView*    pTextView;
    ExtTextEngine*  pTextEngine;

    TextViewOutWin* pOutWin;
    ScrollBar       *pHScrollbar,
                    *pVScrollbar;

    SwSrcView*      pSrcView;

    rtl::Reference< ChangesListener > listener_;
    osl::Mutex mutex_;
    com::sun::star::uno::Reference< com::sun::star::beans::XMultiPropertySet >
        notifier_;

    long            nCurTextWidth;
    sal_uInt16          nStartLine;
    rtl_TextEncoding eSourceEncoding;
    sal_Bool            bReadonly;
    sal_Bool            bDoSyntaxHighlight;
    sal_Bool            bHighlighting;

    Timer           aSyntaxIdleTimer;
    SyntaxLineSet   aSyntaxLineTable;

    void            ImpDoHighlight( const String& rSource, sal_uInt16 nLineOff );

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
    void            DoSyntaxHighlight( sal_uInt16 nPara );

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    DECL_LINK(ScrollHdl, ScrollBar*);

public:
                    SwSrcEditWindow( Window* pParent, SwSrcView* pParentView );
                    ~SwSrcEditWindow();

    void            SetScrollBarRanges();
    void            InitScrollBars();
    sal_uLong           Read( SvStream& rInput)
                        {return pTextEngine->Read(rInput);}
    sal_uLong           Write( SvStream& rOutput)
                        {return pTextEngine->Write(rOutput);}

    ExtTextView*    GetTextView()
                        {return pTextView;}
    TextEngine*     GetTextEngine()
                        {return pTextEngine;}
    SwSrcView*      GetSrcView() {return pSrcView;}

    TextViewOutWin* GetOutWin() {return pOutWin;}

    virtual void    Invalidate( sal_uInt16 nFlags = 0 );

    void            ClearModifyFlag()
                        { pTextEngine->SetModified(sal_False); }
    sal_Bool            IsModified() const
                        { return pTextEngine->IsModified();}
    void            CreateScrollbars();

    void            SetReadonly(sal_Bool bSet){bReadonly = bSet;}
    sal_Bool            IsReadonly(){return bReadonly;}

    void            DoDelayedSyntaxHighlight( sal_uInt16 nPara );

    void            SetStartLine(sal_uInt16 nLine){nStartLine = nLine;}

    virtual void    Command( const CommandEvent& rCEvt );
    void            HandleWheelCommand( const CommandEvent& rCEvt );

    void            SetTextEncoding(rtl_TextEncoding eEncoding);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
