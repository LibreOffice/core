/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _XMLFILEVIEW_HXX
#define _XMLFILEVIEW_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <svl/lstner.hxx>
#include <vcl/timer.hxx>

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#include <svtools/xtextedt.hxx>
#include <vcl/timer.hxx>
#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#include <vcl/wrkwin.hxx>
#include <vcl/ctrl.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>

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
    using Window::Notify;

private:
    TextView*   pTextView;
    TextEngine* pTextEngine;

    TextViewOutWin* pOutWin;
    ScrollBar       *pHScrollbar,
                    *pVScrollbar;

    long            nCurTextWidth;
    sal_uInt16          nStartLine;
    rtl_TextEncoding eSourceEncoding;

    Timer           aSyntaxIdleTimer;
    Table           aSyntaxLineTable;

    bool            bHighlighting;
protected:
    DECL_LINK( SyntaxTimerHdl, Timer * );
    void            ImpDoHighlight( const String& rSource, sal_uInt16 nLineOff );

    virtual void    Resize();
    virtual void    DataChanged( const DataChangedEvent& );
    virtual void    GetFocus();

    void            CreateTextEngine();

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    DECL_LINK(ScrollHdl, ScrollBar*);

    void            DoDelayedSyntaxHighlight( sal_uInt16 nPara );

    void            DoSyntaxHighlight( sal_uInt16 nPara );

public:
                    XMLFileWindow( Window* pParent );
                    ~XMLFileWindow();

    void            SetScrollBarRanges();
    void            InitScrollBars();
    sal_uLong           Read( SvStream& rInput)     { return pTextEngine->Read(rInput); }

    void            ShowWindow( const rtl::OUString& rFileName );

    TextView*       GetTextView()               { return pTextView; }
    TextEngine*     GetTextEngine()             { return pTextEngine; }

    TextViewOutWin* GetOutWin()                 { return pOutWin; }
    void            InvalidateWindow();

    void            CreateScrollbars();

    void            SetStartLine(sal_uInt16 nLine)  {nStartLine = nLine;}

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

    void        ShowWindow( const rtl::OUString& rFileName, const filter_info_impl* pFilterInfo );

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
