/*************************************************************************
 *
 *  $RCSfile: datwin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-27 11:47:48 $
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

#ifndef _SFXDATWIN_HXX
#define _SFXDATWIN_HXX

#ifndef _BRWBOX_HXX
#include <brwbox.hxx>
#endif

#ifndef _BRWHEAD_HXX
#include <brwhead.hxx>
#endif

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

//===================================================================

#define MIN_COLUMNWIDTH  2
#define DRAG_CRITICAL    4

DECLARE_LIST( RectangleList, Rectangle* );

//===================================================================

class ButtonFrame
{
    Rectangle   aRect;
    Rectangle   aInnerRect;
    String      aText;
    BOOL        bPressed;
    BOOL        bCurs;
    BOOL        bAbbr;
    BOOL        m_bDrawDisabled;

public:
               ButtonFrame( const Point& rPt, const Size& rSz,
                            const String &rText,
                            BOOL bPress = FALSE,
                            BOOL bCursor = FALSE,
                            BOOL bAbbreviate = TRUE,
                            BOOL _bDrawDisabled = FALSE)
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
    USHORT              _nId;
    ULONG               _nOriginalWidth;
    ULONG               _nWidth;
    Image               _aImage;
    String              _aTitle;
    BOOL                _bFrozen;
    HeaderBarItemBits   _nFlags;

public:
                        BrowserColumn( USHORT nItemId, const Image &rImage,
                                        const String& rTitle, ULONG nWidthPixel, const Fraction& rCurrentZoom,
                                        HeaderBarItemBits nFlags );

    USHORT              GetId() const { return _nId; }

    ULONG               Width() { return _nWidth; }
    Image&              GetImage() { return _aImage; }
    String&             Title() { return _aTitle; }
    HeaderBarItemBits&  Flags() { return _nFlags; }

    BOOL                IsFrozen() const { return _bFrozen; }
    void                Freeze( BOOL bFreeze = TRUE ) { _bFrozen = bFreeze; }

    virtual void        Draw( BrowseBox& rBox, OutputDevice& rDev,
                              const Point& rPos, BOOL bCurs  );

    void                SetWidth(ULONG nNewWidthPixel, const Fraction& rCurrentZoom);
    void                ZoomChanged(const Fraction& rNewZoom);
};

//===================================================================

class BrowserDataWin
            :public Control
            ,public DragSourceHelper
{
friend class BrowseBox;
    BrowserHeader*  pHeaderBar;     // only for BROWSER_HEADERBAR_NEW
    Window*         pEventWin;      // Window of forwarded events
    ScrollBarBox*   pCornerWin;     // Window in the corner btw the ScrollBars
    BOOL*           pDtorNotify;
    AutoTimer       aMouseTimer;    // recalls MouseMove on dragging out
    MouseEvent      aRepeatEvt;     // a MouseEvent to repeat
    Point           aLastMousePos;  // verhindert pseudo-MouseMoves

    String          aRealRowCount;  // zur Anzeige im VScrollBar

    RectangleList   aInvalidRegion; // invalidated Rectangles during !UpdateMode
    FASTBOOL        bInPaint;       // TRUE while in Paint
    FASTBOOL        bInCommand;     // TRUE while in Command
    FASTBOOL        bNoScrollBack;  // nur vorwaerts scrollen
    FASTBOOL        bNoHScroll;     // kein horizontaler Scrollbar
    FASTBOOL        bAutoHScroll;   // autohide horizontaler Scrollbar
    FASTBOOL        bAutoVScroll;   // autohide horizontaler Scrollbar
    FASTBOOL        bUpdateMode;    // nicht SV-UpdateMode wegen Invalidate()
    FASTBOOL        bAutoSizeLastCol;// last column always fills up window
    FASTBOOL        bHighlightAuto; // new auto-highlight by SetFont() etc.
    FASTBOOL        bResizeOnPaint; // outstanding resize-event
    FASTBOOL        bUpdateOnUnlock;    // Update() while locked
    FASTBOOL        bInUpdateScrollbars;    // Rekursionsschutz
    FASTBOOL        bHadRecursion;          // Rekursion war aufgetreten
    FASTBOOL        bOwnDataChangedHdl; // dont change colors in DataChanged
    USHORT          nUpdateLock;    // lock count, dont call Control::Update()!
    short           nCursorHidden;  // new conuter for DoHide/ShowCursor

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

    virtual BOOL    QueryDrop( DropEvent& rEvt );
    virtual BOOL    Drop( const DropEvent& rEvt );

    // DragSourceHelper overridables
    virtual void    StartDrag( sal_Int8 _nAction, const Point& _rPosPixel );


    BrowseEvent     CreateBrowseEvent( const Point& rPosPixel );
    void            Repaint();
    BrowseBox*      GetParent() const
                         { return (BrowseBox*) Window::GetParent(); }
    const String&   GetRealRowCount() const { return aRealRowCount; }

    void            SetUpdateMode( BOOL bMode );
    FASTBOOL        GetUpdateMode() const { return bUpdateMode; }
    void            EnterUpdateLock() { ++nUpdateLock; }
    void            LeaveUpdateLock();
    void            Update();
    void            DoOutstandingInvalidations();
    void            Invalidate();
    void            Invalidate( const Rectangle& rRect );
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
    ULONG           _nTip;
    ULONG           _nLastPos;
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
         BOOL bFont = TRUE, BOOL bForeground = TRUE, BOOL bBackground = TRUE );

//===================================================================

#ifdef DBG_MI

void DoLog_Impl( const BrowseBox *pThis, const char *pWhat, const char *pWho );
#define LOG(pThis,what,who) DoLog_Impl(pThis,what,who)

#else

#define LOG(pThis,what,who)

#endif


#endif

