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

#ifndef _FMRWRK_OBJWIN_HXX
#define _FMRWRK_OBJWIN_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#include <vcl/menu.hxx>
#include <vcl/timer.hxx>
#include <soldep/listmacr.hxx>
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
    sal_uIntPtr     mnObjectId;
    PopupMenu*  mpPopup;
    sal_uInt16      mnPopupStaticItems;
    Wallpaper   maObjWallpaper;
    ConnectorList   mConnections;
    sal_uIntPtr     mnMarkMode;
    sal_uIntPtr         mnViewMask;
    sal_Bool        mbVisible;
    sal_Bool        mbMenuExecute;
    static sal_Bool msbHideMode;
    static sal_uIntPtr msnGlobalViewMask;   //will be set by SolDep

public:
    sal_Bool        mbVisited;
//  double x, y;

    sal_uInt16 mnRootDist;
    sal_uInt16 mnHeadDist;
    sal_Bool mbFixed;
    //Depper*       mpDepperDontuseme;
    Timer   aTipTimer;

                    ObjectWin( Window* pParent, WinBits nWinStyle );
                    ~ObjectWin();
        void            SetHideMode(sal_Bool bHide);
        sal_Bool            ToggleHideMode();
        sal_Bool            IsHideMode() { return msbHideMode; };
        sal_Bool            IsNullObject() { return (msBodyText == "null"); };
        void            SetGlobalViewMask(sal_uIntPtr gvm) { msnGlobalViewMask = gvm; };
        void            SetBodyText( const ByteString& rNewText );
        ByteString&         GetBodyText();
        sal_uIntPtr           GetIdByName( const ByteString& rText );
        void            SetTipText( const ByteString& rNewText );
        ByteString&         GetTipText();
        Point           GetFixPoint( const Point& rRefPoint, sal_Bool bUseRealPos = sal_True );

        void            SetCalcPosPixel( const Point& rNewPos ){ maCalcPos = rNewPos; };
        Point           GetCalcPosPixel() const { return maCalcPos; }

        void            AddConnector( Connector* pNewCon );
        void            RemoveConnector( Connector* pOldCon );
        Connector*      GetConnector( sal_uIntPtr nIndex );
        Connector*      GetConnector( sal_uIntPtr nStartId, sal_uIntPtr nEndId );
        virtual void    SetMarkMode( sal_uIntPtr nMarkMode = 0 );
        virtual void    UnsetMarkMode( sal_uIntPtr nMarkMode );
        sal_uIntPtr         GetMarkMode(){ return mnMarkMode; };
        Wallpaper       GetActualWallpaper() { return maObjWallpaper;};
        void            SetActualWallpaper(const Wallpaper& aWp) { maObjWallpaper = aWp;};
        void            MarkNeeded( sal_Bool bReset = sal_False );
        void            MarkDepending( sal_Bool bReset = sal_False );
        void            Paint( const Rectangle& rRecct );
        void            MouseButtonDown( const MouseEvent& rMEvt );
        void            MouseButtonUp( const MouseEvent& rMEvt );
        void            MouseMove( const MouseEvent& rMEvt );
//      void            DoubleClick();
        sal_uInt16          Save( SvFileStream& rOutFile );
        sal_uInt16          Load( SvFileStream& rInFile );
        void            SetId( sal_uIntPtr nId );
        sal_uIntPtr         GetId();
        void            UpdateConnectors();
        void            SetAllConnectorsUnvisible();
        virtual void    Command( const CommandEvent& rEvent);

        void SetViewMask( sal_uIntPtr nMask );
        sal_uIntPtr GetViewMask() { return mnViewMask; }
        using Window::IsVisible;
        sal_Bool IsVisible() { return mbVisible; }

        DECL_LINK( PopupSelected, PopupMenu * );
        DECL_LINK( PopupDeactivated, PopupMenu * );
        DECL_LINK( TipHdl, void * );

        //virtual void GetFocus();
        virtual void LoseFocus();

        sal_Bool            ConnectionExistsInAnyDirection( ObjectWin *pWin );
        void    DrawOutput( OutputDevice* pDevice, const Point& rOffset  );
        sal_Bool    IsTop();
};

DECL_DEST_LIST( TmpObjWinList, ObjWinList, ObjectWin* )

//Extend ObjWinList
class ObjectList : public ObjWinList
{
private:
    ObjectWin* pSelectedObjectWin;
protected:
    sal_Bool       mbHideMode;
public:
               ObjectList();
               ~ObjectList() {};
    void       ResetSelectedObject();
    ObjectWin* GetPtrByName( const ByteString& rText );
    ObjectList* FindTopLevelModules();
};

#endif

