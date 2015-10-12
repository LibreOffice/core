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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SRCEDTW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SRCEDTW_HXX

#include <vcl/window.hxx>
#include <svl/lstner.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>

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

class TextViewOutWin : public vcl::Window
{
    ExtTextView*    pTextView;

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& ) override;
    virtual void    KeyInput( const KeyEvent& rKeyEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& ) override;

public:
        TextViewOutWin(vcl::Window* pParent, WinBits nBits) :
            Window(pParent, nBits), pTextView(0){}

    void    SetTextView( ExtTextView* pView ) {pTextView = pView;}

};

typedef std::set<sal_uInt16> SyntaxLineSet;

class SwSrcEditWindow : public vcl::Window, public SfxListener
{
private:
    class ChangesListener;
    friend class ChangesListener;
    ExtTextView*    pTextView;
    ExtTextEngine*  pTextEngine;

    VclPtr<TextViewOutWin> pOutWin;
    VclPtr<ScrollBar>      pHScrollbar,
                           pVScrollbar;

    SwSrcView*      pSrcView;

    rtl::Reference< ChangesListener > listener_;
    osl::Mutex mutex_;
    com::sun::star::uno::Reference< com::sun::star::beans::XMultiPropertySet >
        notifier_;

    long            nCurTextWidth;
    sal_uInt16          nStartLine;
    rtl_TextEncoding eSourceEncoding;
    bool            bReadonly;
    bool            bDoSyntaxHighlight;
    bool            bHighlighting;

    Idle            aSyntaxIdle;
    SyntaxLineSet   aSyntaxLineTable;

    void            ImpDoHighlight( const OUString& rSource, sal_uInt16 nLineOff );

    using OutputDevice::SetFont;
    void            SetFont();

    DECL_LINK_TYPED( SyntaxTimerHdl, Idle *, void );

    using Window::Notify;
    using Window::Invalidate;

protected:

    virtual void    Resize() override;
    virtual void    DataChanged( const DataChangedEvent& ) override;
    virtual void    GetFocus() override;
//  virtual void    LoseFocus();

    void            CreateTextEngine();
    void            DoSyntaxHighlight( sal_uInt16 nPara );

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    DECL_LINK_TYPED(ScrollHdl, ScrollBar*, void);

public:
                    SwSrcEditWindow( vcl::Window* pParent, SwSrcView* pParentView );
                    virtual ~SwSrcEditWindow();
    virtual void    dispose() override;

    void            SetScrollBarRanges();
    void            InitScrollBars();
    void            Read(SvStream& rInput) { pTextEngine->Read(rInput); }
    void            Write(SvStream& rOutput) { pTextEngine->Write(rOutput); }

    ExtTextView*    GetTextView()
                        {return pTextView;}
    TextEngine*     GetTextEngine()
                        {return pTextEngine;}
    SwSrcView*      GetSrcView() {return pSrcView;}

    TextViewOutWin* GetOutWin() {return pOutWin;}

    virtual void    Invalidate( InvalidateFlags nFlags = InvalidateFlags::NONE ) override;

    void            ClearModifyFlag()
                        { pTextEngine->SetModified(false); }
    bool            IsModified() const
                        { return pTextEngine->IsModified();}

    void            SetReadonly(bool bSet){bReadonly = bSet;}
    bool            IsReadonly(){return bReadonly;}

    void            DoDelayedSyntaxHighlight( sal_uInt16 nPara );

    void            SetStartLine(sal_uInt16 nLine){nStartLine = nLine;}

    virtual void    Command( const CommandEvent& rCEvt ) override;
    void            HandleWheelCommand( const CommandEvent& rCEvt );

    void            SetTextEncoding(rtl_TextEncoding eEncoding);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
