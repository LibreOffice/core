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
#include <mutex>
#include <set>

namespace com::sun::star::beans { class XMultiPropertySet; }
namespace weld { class Scrollbar; }
class ScrollAdaptor;
class SwSrcView;
class TextEngine;
class TextView;

class TextViewOutWin final : public vcl::Window
{
    TextView*    m_pTextView;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;
    virtual void    KeyInput( const KeyEvent& rKeyEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& ) override;

public:
        TextViewOutWin(vcl::Window* pParent, WinBits nBits) :
            Window(pParent, nBits), m_pTextView(nullptr){}

    void    SetTextView( TextView* pView ) {m_pTextView = pView;}

};

class SwSrcEditWindow final : public vcl::Window, public SfxListener
{
private:
    class ChangesListener;
    friend class ChangesListener;
    std::unique_ptr<TextView>       m_pTextView;
    std::unique_ptr<ExtTextEngine>  m_pTextEngine;

    VclPtr<TextViewOutWin> m_pOutWin;
    VclPtr<ScrollAdaptor>  m_pHScrollbar,
                           m_pVScrollbar;

    SwSrcView*      m_pSrcView;

    rtl::Reference< ChangesListener > m_xListener;
    std::mutex mutex_;
    css::uno::Reference< css::beans::XMultiPropertySet >
        m_xNotifier;

    tools::Long            m_nCurTextWidth;
    sal_uInt16          m_nStartLine;
    rtl_TextEncoding m_eSourceEncoding;
    bool            m_bReadonly;
    bool            m_bHighlighting;

    Idle            m_aSyntaxIdle;
    std::set<sal_uInt16>   m_aSyntaxLineTable;

    void            ImpDoHighlight( std::u16string_view aSource, sal_uInt16 nLineOff );

    void            SetFont();

    DECL_LINK( SyntaxTimerHdl, Timer *, void );

    virtual void    Resize() override;
    virtual void    DataChanged( const DataChangedEvent& ) override;
    virtual void    GetFocus() override;

    void            CreateTextEngine();
    void            DoSyntaxHighlight( sal_uInt16 nPara );

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    DECL_LINK(HorzScrollHdl, weld::Scrollbar&, void);
    DECL_LINK(VertScrollHdl, weld::Scrollbar&, void);

public:
                    SwSrcEditWindow( vcl::Window* pParent, SwSrcView* pParentView );
                    virtual ~SwSrcEditWindow() override;
    virtual void    dispose() override;

    void            SetScrollBarRanges();
    void            InitScrollBars();
    void            Read(SvStream& rInput) { m_pTextEngine->Read(rInput); }
    void            Write(SvStream& rOutput) { m_pTextEngine->Write(rOutput); }

    TextView*       GetTextView()
                        {return m_pTextView.get();}
    TextEngine*     GetTextEngine()
                        {return m_pTextEngine.get();}
    SwSrcView*      GetSrcView() {return m_pSrcView;}

    TextViewOutWin* GetOutWin() {return m_pOutWin;}

    virtual void    ImplInvalidate( const vcl::Region* pRegion, InvalidateFlags nFlags ) override;

    void            ClearModifyFlag()
                        { m_pTextEngine->SetModified(false); }
    bool            IsModified() const
                        { return m_pTextEngine->IsModified();}

    void            SetReadonly(bool bSet){m_bReadonly = bSet;}
    bool            IsReadonly() const {return m_bReadonly;}

    void            SetStartLine(sal_uInt16 nLine){m_nStartLine = nLine;}

    virtual void    Command( const CommandEvent& rCEvt ) override;
    void            HandleWheelCommand( const CommandEvent& rCEvt );

    void            SetTextEncoding(rtl_TextEncoding eEncoding);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
