/*************************************************************************
 *
 *  $RCSfile: xmlfileview.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:32:37 $
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
#ifndef _XMLFILEVIEW_HXX
#define _XMLFILEVIEW_HXX

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

class ScrollBar;
class XMLFileWindow;
class TextEngine;
class TextView;
class DataChangedEvent;
class filter_info_impl;

class TextViewOutWin : public Window
{
    TextView*   pTextView;

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

    void    SetTextView( TextView* pView ) {pTextView = pView;}

};

//------------------------------------------------------------

class XMLFileWindow : public Window, public SfxListener
{
private:
    TextView*   pTextView;
    TextEngine* pTextEngine;

    TextViewOutWin* pOutWin;
    ScrollBar       *pHScrollbar,
                    *pVScrollbar;

    long            nCurTextWidth;
    USHORT          nStartLine;
    rtl_TextEncoding eSourceEncoding;

    Timer           aSyntaxIdleTimer;
    Table           aSyntaxLineTable;

    bool            bHighlighting;
protected:
    DECL_LINK( SyntaxTimerHdl, Timer * );
    void            ImpDoHighlight( const String& rSource, USHORT nLineOff );

    virtual void    Resize();
    virtual void    DataChanged( const DataChangedEvent& );
    virtual void    GetFocus();

    void            CreateTextEngine();

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    DECL_LINK(ScrollHdl, ScrollBar*);

    void            DoDelayedSyntaxHighlight( USHORT nPara );

    void            SyntaxColorsChanged();
    void            DoSyntaxHighlight( USHORT nPara );

public:
                    XMLFileWindow( Window* pParent );
                    ~XMLFileWindow();

    void            SetScrollBarRanges();
    void            InitScrollBars();
    ULONG           Read( SvStream& rInput)     { return pTextEngine->Read(rInput); }

    void            Show( const rtl::OUString& rFileName );

    TextView*       GetTextView()               { return pTextView; }
    TextEngine*     GetTextEngine()             { return pTextEngine; }

    TextViewOutWin* GetOutWin()                 { return pOutWin; }
    void            Invalidate();

    void            CreateScrollbars();

    void            SetStartLine(USHORT nLine)  {nStartLine = nLine;}

    virtual void    Command( const CommandEvent& rCEvt );
    void            HandleWheelCommand( const CommandEvent& rCEvt );

    TextView*       getTextView() { return pTextView; }

    void            showLine( sal_Int32 nLine );
};

class XMLSourceFileDialog : public WorkWindow
{
public:
                XMLSourceFileDialog( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF  );
    virtual     ~XMLSourceFileDialog();

    void        Show( const rtl::OUString& rFileName, const filter_info_impl* pFilterInfo );

    virtual void    Resize();

    DECL_LINK(ClickHdl_Impl, PushButton * );
    DECL_LINK(SelectHdl_Impl, ListBox * );

    void        showLine( sal_Int32 nLine );

    sal_Int32   mnOutputHeight;
private:
    void        onValidate();

    const filter_info_impl* mpFilterInfo;
    rtl::OUString   maFileURL;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
    ResMgr&         mrResMgr;
    XMLFileWindow*  mpTextWindow;
    ListBox         maLBOutput;
//  Control         maCtrlSourceView;
    PushButton      maPBValidate;
};

#endif
