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
#ifndef _XMLFILEVIEW_HXX
#define _XMLFILEVIEW_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <svl/lstner.hxx>
#include <vcl/timer.hxx>

#include <tools/table.hxx>
#include <svtools/xtextedt.hxx>
#include <vcl/timer.hxx>
#include <tools/table.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>

class ScrollBar;
class XMLFileWindow;
class TextEngine;
class TextView;
class DataChangedEvent;
class filter_info_impl;

class TextViewOutWin : public Window
{
    TextView*	pTextView;

protected:
    virtual void	Paint( const Rectangle& );
    virtual void	KeyInput( const KeyEvent& rKeyEvt );
    virtual void	MouseMove( const MouseEvent& rMEvt );
    virtual void	MouseButtonDown( const MouseEvent& rMEvt );
    virtual void	MouseButtonUp( const MouseEvent& rMEvt );
    virtual void	Command( const CommandEvent& rCEvt );
    virtual void	DataChanged( const DataChangedEvent& );

public:
        TextViewOutWin(Window* pParent, WinBits nBits) :
            Window(pParent, nBits), pTextView(0){}

    void	SetTextView( TextView* pView ) {pTextView = pView;}

};

//------------------------------------------------------------

class XMLFileWindow : public Window, public SfxListener
{
    using Window::Notify;

private:
    TextView*	pTextView;
    TextEngine*	pTextEngine;

    TextViewOutWin*	pOutWin;
    ScrollBar	   	*pHScrollbar,
                    *pVScrollbar;

    long			nCurTextWidth;
    USHORT			nStartLine;
    rtl_TextEncoding eSourceEncoding;

    Timer			aSyntaxIdleTimer;
    Table			aSyntaxLineTable;

    bool			bHighlighting;
protected:
    DECL_LINK( SyntaxTimerHdl, Timer * );
    void			ImpDoHighlight( const String& rSource, USHORT nLineOff );

    virtual void	Resize();
    virtual void	DataChanged( const DataChangedEvent& );
    virtual void 	GetFocus();

    void 			CreateTextEngine();

    virtual void	Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    DECL_LINK(ScrollHdl, ScrollBar*);

    void			DoDelayedSyntaxHighlight( USHORT nPara );

    void			DoSyntaxHighlight( USHORT nPara );

public:
                    XMLFileWindow( Window* pParent );
                    ~XMLFileWindow();

    void			SetScrollBarRanges();
    void			InitScrollBars();
    ULONG			Read( SvStream& rInput)		{ return pTextEngine->Read(rInput); }

    void			ShowWindow( const rtl::OUString& rFileName );

    TextView*		GetTextView()				{ return pTextView; }
    TextEngine*		GetTextEngine()				{ return pTextEngine; }

    TextViewOutWin*	GetOutWin()					{ return pOutWin; }
    void			InvalidateWindow();

    void			CreateScrollbars();

    void			SetStartLine(USHORT nLine)	{nStartLine = nLine;}

    virtual void	Command( const CommandEvent& rCEvt );
    void 			HandleWheelCommand( const CommandEvent& rCEvt );

    TextView*		getTextView() { return pTextView; }

    void			showLine( sal_Int32 nLine );
};

class XMLSourceFileDialog : public WorkWindow
{

public:
                XMLSourceFileDialog( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF  );
    virtual		~XMLSourceFileDialog();

    void		ShowWindow( const rtl::OUString& rFileName, const filter_info_impl* pFilterInfo );

    virtual void	Resize();

    DECL_LINK(ClickHdl_Impl, PushButton * );
    DECL_LINK(SelectHdl_Impl, ListBox * );

    void		showLine( sal_Int32 nLine );

    sal_Int32	mnOutputHeight;
private:
    void		onValidate();

    const filter_info_impl* mpFilterInfo;
    rtl::OUString	maFileURL;

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
    ResMgr&			mrResMgr;

    XMLFileWindow*	mpTextWindow;
    ListBox			maLBOutput;
//	Control			maCtrlSourceView;
    PushButton		maPBValidate;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
