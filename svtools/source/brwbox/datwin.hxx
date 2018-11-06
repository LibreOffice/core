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



#ifndef _SFXDATWIN_HXX
#define _SFXDATWIN_HXX

#ifndef _BRWBOX_HXX
#include <svtools/brwbox.hxx>
#endif
#include <svtools/brwhead.hxx>
#include <vcl/timer.hxx>
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#include <tools/list.hxx>
#include <svtools/transfer.hxx>

//===================================================================

#define MIN_COLUMNWIDTH  2
#define DRAG_CRITICAL    4

DECLARE_LIST( RectangleList, Rectangle* )

//===================================================================

class ButtonFrame
{
    Rectangle   aRect;
    Rectangle   aInnerRect;
    String      aText;
    sal_Bool        bPressed;
    sal_Bool        bCurs;
    sal_Bool        bAbbr;
    sal_Bool        m_bDrawDisabled;

public:
               ButtonFrame( const Point& rPt, const Size& rSz,
                            const String &rText,
                            sal_Bool bPress = sal_False,
                            sal_Bool bCursor = sal_False,
                            sal_Bool bAbbreviate = sal_True,
                            sal_Bool _bDrawDisabled = sal_False)
                :aRect( rPt, rSz )
                ,aInnerRect( Point( aRect.Left()+1, aRect.Top()+1 ),
                            Size( aRect.GetWidth()-2, aRect.GetHeight()-2 ) )
                ,aText(rText)
                ,bPressed(bPress)
                ,bCurs(bCursor)
                ,bAbbr(bAbbreviate)
                ,m_bDrawDisabled(_bDrawDisabled)
            {
            }

    void    Draw( OutputDevice& rDev );
};

//===================================================================

class BrowserColumn
{
    sal_uInt16              _nId;
    sal_uLong               _nOriginalWidth;
    sal_uLong               _nWidth;
    Image               _aImage;
    String              _aTitle;
    sal_Bool                _bFrozen;
    HeaderBarItemBits   _nFlags;

public:
                        BrowserColumn( sal_uInt16 nItemId, const Image &rImage,
                                        const String& rTitle, sal_uLong nWidthPixel, const Fraction& rCurrentZoom,
                                        HeaderBarItemBits nFlags );
    virtual            ~BrowserColumn();

    sal_uInt16              GetId() const { return _nId; }

    sal_uLong               Width() { return _nWidth; }
    Image&              GetImage() { return _aImage; }
    String&             Title() { return _aTitle; }
    HeaderBarItemBits&  Flags() { return _nFlags; }

    sal_Bool                IsFrozen() const { return _bFrozen; }
    void                Freeze( sal_Bool bFreeze = sal_True ) { _bFrozen = bFreeze; }

    virtual void        Draw( BrowseBox& rBox, OutputDevice& rDev,
                              const Point& rPos, sal_Bool bCurs  );

    void                SetWidth(sal_uLong nNewWidthPixel, const Fraction& rCurrentZoom);
    void                ZoomChanged(const Fraction& rNewZoom);
};

//===================================================================

class BrowserDataWin
            :public Control
            ,public DragSourceHelper
            ,public DropTargetHelper
{
public:
    BrowserHeader*  pHeaderBar;     // only for BROWSER_HEADERBAR_NEW
    Window*         pEventWin;      // Window of forwarded events
    ScrollBarBox*   pCornerWin;     // Window in the corner btw the ScrollBars
    sal_Bool*           pDtorNotify;
    AutoTimer       aMouseTimer;    // recalls MouseMove on dragging out
    MouseEvent      aRepeatEvt;     // a MouseEvent to repeat
    Point           aLastMousePos;  // verhindert pseudo-MouseMoves

    String          aRealRowCount;  // zur Anzeige im VScrollBar

    RectangleList   aInvalidRegion; // invalidated Rectangles during !UpdateMode
    FASTBOOL        bInPaint;       // sal_True while in Paint
    FASTBOOL        bInCommand;     // sal_True while in Command
    FASTBOOL        bNoScrollBack;  // nur vorwaerts scrollen
    FASTBOOL        bNoHScroll;     // kein horizontaler Scrollbar
    FASTBOOL        bNoVScroll;     // no vertical scrollbar
    FASTBOOL        bAutoHScroll;   // autohide horizontaler Scrollbar
    FASTBOOL        bAutoVScroll;   // autohide horizontaler Scrollbar
    FASTBOOL        bUpdateMode;    // nicht SV-UpdateMode wegen Invalidate()
    FASTBOOL        bAutoSizeLastCol;// last column always fills up window
    FASTBOOL        bResizeOnPaint; // outstanding resize-event
    FASTBOOL        bUpdateOnUnlock;    // Update() while locked
    FASTBOOL        bInUpdateScrollbars;    // Rekursionsschutz
    FASTBOOL        bHadRecursion;          // Rekursion war aufgetreten
    FASTBOOL        bOwnDataChangedHdl;     // dont change colors in DataChanged
    FASTBOOL        bCallingDropCallback;   // we're in a callback to AcceptDrop or ExecuteDrop currently
    sal_uInt16          nUpdateLock;    // lock count, dont call Control::Update()!
    short           nCursorHidden;  // new conuter for DoHide/ShowCursor

    long            m_nDragRowDividerLimit;
    long            m_nDragRowDividerOffset;

public:
                    BrowserDataWin( BrowseBox* pParent );
                    ~BrowserDataWin();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    Command( const CommandEvent& rEvt );
    virtual void    MouseButtonDown( const MouseEvent& rEvt );
    virtual void    MouseMove( const MouseEvent& rEvt );
                    DECL_LINK( RepeatedMouseMove, void * );

    virtual void    MouseButtonUp( const MouseEvent& rEvt );
    virtual void    KeyInput( const KeyEvent& rEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );

    // DropTargetHelper overridables
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    // DragSourceHelper overridables
    virtual void    StartDrag( sal_Int8 _nAction, const Point& _rPosPixel );


    BrowseEvent     CreateBrowseEvent( const Point& rPosPixel );
    void            Repaint();
    BrowseBox*      GetParent() const
                         { return (BrowseBox*) Window::GetParent(); }
    const String&   GetRealRowCount() const { return aRealRowCount; }

    void            SetUpdateMode( sal_Bool bMode );
    FASTBOOL        GetUpdateMode() const { return bUpdateMode; }
    void            EnterUpdateLock() { ++nUpdateLock; }
    void            LeaveUpdateLock();
    void            Update();
    void            DoOutstandingInvalidations();
    void            Invalidate( sal_uInt16 nFlags = 0 );
    void            Invalidate( const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    void            Invalidate( const Region& rRegion, sal_uInt16 nFlags = 0 )
                    { Control::Invalidate( rRegion, nFlags ); }

protected:
    void            StartRowDividerDrag( const Point& _rStartPos );
    sal_Bool            ImplRowDividerHitTest( const BrowserMouseEvent& _rEvent );
};

//-------------------------------------------------------------------

inline void BrowserDataWin::Repaint()
{
    if ( GetUpdateMode() )
        Update();
    Paint( Rectangle( Point(), GetOutputSizePixel() ) );
}

//===================================================================

class BrowserScrollBar: public ScrollBar
{
    sal_uLong           _nTip;
    sal_uLong           _nLastPos;
    BrowserDataWin* _pDataWin;

public:
                    BrowserScrollBar( Window* pParent, WinBits nStyle,
                                      BrowserDataWin *pDataWin )
                    :   ScrollBar( pParent, nStyle ),
                        _nTip( 0 ),
                        _nLastPos( ULONG_MAX ),
                        _pDataWin( pDataWin )
                    {}
                    //ScrollBar( Window* pParent, const ResId& rResId );

    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    EndScroll();
};

//===================================================================

void InitSettings_Impl( Window *pWin,
         sal_Bool bFont = sal_True, sal_Bool bForeground = sal_True, sal_Bool bBackground = sal_True );

//===================================================================

#ifdef DBG_MI

void DoLog_Impl( const BrowseBox *pThis, const char *pWhat, const char *pWho );
#define LOG(pThis,what,who) DoLog_Impl(pThis,what,who)

#else

#define LOG(pThis,what,who)

#endif


#endif

