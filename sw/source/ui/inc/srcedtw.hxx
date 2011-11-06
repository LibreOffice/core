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


#ifndef _SRCEDTW_HXX
#define _SRCEDTW_HXX

#include <vcl/window.hxx>
#include <svl/lstner.hxx>
#include <unotools/options.hxx>
#include <vcl/timer.hxx>

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
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
    sal_uInt16          nStartLine;
    rtl_TextEncoding eSourceEncoding;
    sal_Bool            bReadonly;
    sal_Bool            bDoSyntaxHighlight;
    sal_Bool            bHighlighting;

    Timer           aSyntaxIdleTimer;
    Table           aSyntaxLineTable;

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
    virtual void    ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

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
