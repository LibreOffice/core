/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objwin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:38:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _FMRWRK_OBJWIN_HXX
#define _FMRWRK_OBJWIN_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#include <vcl/menu.hxx>
#include <vcl/timer.hxx>
#include <bootstrp/listmacr.hxx>
#include <tools/stream.hxx>

#include <soldep/connctr.hxx>

#define OBJWIN_EDIT_TEXT        1
#define OBJWIN_ADD_CONNECTOR    2
#define OBJWIN_REMOVE_WIN       3
#define OBJWIN_VIEW_CONTENT     4

#define MARKMODE_DEFAULT        0
#define MARKMODE_DEPENDING      1
#define MARKMODE_NEEDED         2
#define MARKMODE_ACTIVATED      4
#define MARKMODE_SELECTED       8

class Depper;
class Connector;
class ObjectWin;

class ObjectWin : public Window
{
    Point       maMouseOffset;
    Point       maCalcPos;
    ByteString      msBodyText;
    ByteString      msTipText;
    ULONG       mnObjectId;
    PopupMenu*  mpPopup;
    USHORT      mnPopupStaticItems;
    Wallpaper   maObjWallpaper;
    ConnectorList   mConnections;
    ULONG       mnMarkMode;
    ULONG       mnViewMask;
    BOOL        mbVisible;
    BOOL        mbMenuExecute;
    static BOOL msbHideMode;
    static ULONG msnGlobalViewMask;   //will be set by SolDep

public:
    BOOL        mbVisited;
//  double x, y;

    USHORT mnRootDist;
    USHORT mnHeadDist;
    BOOL mbFixed;
    //Depper*       mpDepperDontuseme;
    Timer   aTipTimer;

                    ObjectWin( Window* pParent, WinBits nWinStyle );
                    ~ObjectWin();
        void            SetHideMode(BOOL bHide);
        BOOL            ToggleHideMode();
        BOOL            IsHideMode() { return msbHideMode; };
        BOOL            IsNullObject() { return (msBodyText == "null"); };
        void            SetGlobalViewMask(ULONG gvm) { msnGlobalViewMask = gvm; };
        void            SetBodyText( const ByteString& rNewText );
        ByteString&         GetBodyText();
        ULONG           GetIdByName( const ByteString& rText );
        void            SetTipText( const ByteString& rNewText );
        ByteString&         GetTipText();
        Point           GetFixPoint( const Point& rRefPoint, BOOL bUseRealPos = TRUE );

        void            SetCalcPosPixel( const Point& rNewPos ){ maCalcPos = rNewPos; };
        Point           GetCalcPosPixel() const { return maCalcPos; }

        void            AddConnector( Connector* pNewCon );
        void            RemoveConnector( Connector* pOldCon );
        Connector*      GetConnector( ULONG nIndex );
        Connector*      GetConnector( ULONG nStartId, ULONG nEndId );
        virtual void    SetMarkMode( ULONG nMarkMode = 0 );
        virtual void    UnsetMarkMode( ULONG nMarkMode );
        ULONG           GetMarkMode(){ return mnMarkMode; };
        Wallpaper       GetActualWallpaper() { return maObjWallpaper;};
        void            SetActualWallpaper(const Wallpaper& aWp) { maObjWallpaper = aWp;};
        void            MarkNeeded( BOOL bReset = FALSE );
        void            MarkDepending( BOOL bReset = FALSE );
        void            Paint( const Rectangle& rRecct );
        void            MouseButtonDown( const MouseEvent& rMEvt );
        void            MouseButtonUp( const MouseEvent& rMEvt );
        void            MouseMove( const MouseEvent& rMEvt );
//      void            DoubleClick();
        USHORT          Save( SvFileStream& rOutFile );
        USHORT          Load( SvFileStream& rInFile );
        void            SetId( ULONG nId );
        ULONG           GetId();
        void            UpdateConnectors();
        void            SetAllConnectorsUnvisible();
        virtual void    Command( const CommandEvent& rEvent);

        void SetViewMask( ULONG nMask );
        ULONG GetViewMask() { return mnViewMask; }
        BOOL IsVisible() { return mbVisible; }

        DECL_LINK( PopupSelected, PopupMenu * );
        DECL_LINK( PopupDeactivated, PopupMenu * );
        DECL_LINK( TipHdl, void * );

        //virtual void GetFocus();
        virtual void LoseFocus();

        BOOL            ConnectionExistsInAnyDirection( ObjectWin *pWin );
};

DECL_DEST_LIST( TmpObjWinList, ObjWinList, ObjectWin* )

//Extend ObjWinList
class ObjectList : public ObjWinList
{
private:
    ObjectWin* pSelectedObjectWin;
protected:
    BOOL       mbHideMode;
public:
               ObjectList();
               ~ObjectList() {};
    void       ResetSelectedObject();
    ObjectWin* GetPtrByName( const ByteString& rText );
};

#endif

