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

#ifndef _SV_FLOATWIN_HXX
#define _SV_FLOATWIN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>

class ToolBox;

// ------------------------
// - FloatingWindow-Types -
// ------------------------

#define FLOATWIN_POPUPMODE_ALLOWTEAROFF         ((sal_uLong)0x00000001)
#define FLOATWIN_POPUPMODE_ANIMATIONSLIDE       ((sal_uLong)0x00000002)
#define FLOATWIN_POPUPMODE_NOAUTOARRANGE        ((sal_uLong)0x00000004)
#define FLOATWIN_POPUPMODE_NOANIMATION          ((sal_uLong)0x00000008)
#define FLOATWIN_POPUPMODE_DOWN                 ((sal_uLong)0x00000010)
#define FLOATWIN_POPUPMODE_UP                   ((sal_uLong)0x00000020)
#define FLOATWIN_POPUPMODE_LEFT                 ((sal_uLong)0x00000040)
#define FLOATWIN_POPUPMODE_RIGHT                ((sal_uLong)0x00000080)
#define FLOATWIN_POPUPMODE_NOFOCUSCLOSE         ((sal_uLong)0x00000100)
#define FLOATWIN_POPUPMODE_NOKEYCLOSE           ((sal_uLong)0x00000200)
#define FLOATWIN_POPUPMODE_NOMOUSECLOSE         ((sal_uLong)0x00000400)
#define FLOATWIN_POPUPMODE_NOMOUSERECTCLOSE     ((sal_uLong)0x00000800)
#define FLOATWIN_POPUPMODE_ALLMOUSEBUTTONCLOSE  ((sal_uLong)0x00001000)
#define FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE      ((sal_uLong)0x00002000)
#define FLOATWIN_POPUPMODE_PATHMOUSECANCELCLICK ((sal_uLong)0x00004000)
#define FLOATWIN_POPUPMODE_NEWLEVEL             ((sal_uLong)0x00008000)
#define FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE       ((sal_uLong)0x00010000)
#define FLOATWIN_POPUPMODE_GRABFOCUS            ((sal_uLong)0x00020000)

#define FLOATWIN_POPUPMODEEND_CANCEL            ((sal_uInt16)0x0001)
#define FLOATWIN_POPUPMODEEND_TEAROFF           ((sal_uInt16)0x0002)
#define FLOATWIN_POPUPMODEEND_DONTCALLHDL       ((sal_uInt16)0x0004)
#define FLOATWIN_POPUPMODEEND_CLOSEALL          ((sal_uInt16)0x0008)

#define FLOATWIN_TITLE_NORMAL                   ((sal_uInt16)0x0001)
#define FLOATWIN_TITLE_TEAROFF                  ((sal_uInt16)0x0002)
#define FLOATWIN_TITLE_POPUP                    ((sal_uInt16)0x0004)
#define FLOATWIN_TITLE_NONE                     ((sal_uInt16)0x0008)

// ------------------
// - FloatingWindow -
// ------------------

class VCL_DLLPUBLIC FloatingWindow : public SystemWindow
{
    class   ImplData;
private:
    FloatingWindow* mpNextFloat;
    Window*         mpFirstPopupModeWin;
    ImplData*       mpImplData;
    Rectangle       maFloatRect;
    sal_uLong           mnPostId;
    sal_uLong           mnPopupModeFlags;
    sal_uInt16          mnTitle;
    sal_uInt16          mnOldTitle;
    sal_Bool            mbInPopupMode;
    sal_Bool            mbPopupMode;
    sal_Bool            mbPopupModeCanceled;
    sal_Bool            mbPopupModeTearOff;
    sal_Bool            mbMouseDown;
    sal_Bool            mbOldSaveBackMode;
    sal_Bool            mbGrabFocus;    // act as key input window, although focus is not set
    sal_Bool            mbInCleanUp;
    Link            maPopupModeEndHdl;

    SAL_DLLPRIVATE void    ImplCallPopupModeEnd();
    DECL_DLLPRIVATE_LINK(  ImplEndPopupModeHdl, void* );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         FloatingWindow (const FloatingWindow &);
    SAL_DLLPRIVATE         FloatingWindow & operator= (const FloatingWindow &);

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();
    SAL_DLLPRIVATE void    ImplLoadRes( const ResId& rResId );

public:
#define IMPL_FLOATWIN_HITTEST_OUTSIDE       ((sal_uInt16)0x0001)
#define IMPL_FLOATWIN_HITTEST_WINDOW        ((sal_uInt16)0x0002)
#define IMPL_FLOATWIN_HITTEST_RECT          ((sal_uInt16)0x0004)
    SAL_DLLPRIVATE FloatingWindow*  ImplFloatHitTest( Window* pReference, const Point& rPos, sal_uInt16& rHitTest );
    SAL_DLLPRIVATE FloatingWindow*  ImplFindLastLevelFloat();
    SAL_DLLPRIVATE sal_Bool             ImplIsFloatPopupModeWindow( const Window* pWindow );
    SAL_DLLPRIVATE void             ImplSetMouseDown() { mbMouseDown = sal_True; }
    SAL_DLLPRIVATE sal_Bool             ImplIsMouseDown() const  { return mbMouseDown; }
    SAL_DLLPRIVATE static Point     ImplCalcPos( Window* pWindow,
                                                 const Rectangle& rRect, sal_uLong nFlags,
                                                 sal_uInt16& rArrangeIndex );
    SAL_DLLPRIVATE void             ImplEndPopupMode( sal_uInt16 nFlags = 0, sal_uLong nFocusId = 0 );
    SAL_DLLPRIVATE Rectangle&       ImplGetItemEdgeClipRect();
    SAL_DLLPRIVATE sal_Bool             ImplIsInPrivatePopupMode() const { return mbInPopupMode; }

public:
                    FloatingWindow( Window* pParent, WinBits nStyle = WB_STDFLOATWIN );
                    FloatingWindow( Window* pParent, const ResId& rResId );
                    ~FloatingWindow();

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    PopupModeEnd();

    void            SetTitleType( sal_uInt16 nTitle );
    sal_uInt16          GetTitleType() const { return mnTitle; }

    void            StartPopupMode( const Rectangle& rRect, sal_uLong nFlags = 0 );
    void            StartPopupMode( ToolBox* pBox, sal_uLong nFlags = 0  );
    void            EndPopupMode( sal_uInt16 nFlags = 0 );
    void            AddPopupModeWindow( Window* pWindow );
    sal_uLong           GetPopupModeFlags() const { return mnPopupModeFlags; }
    void            SetPopupModeFlags( sal_uLong nFlags ) { mnPopupModeFlags = nFlags; }
    sal_Bool            IsInPopupMode() const { return mbPopupMode; }
    sal_Bool            IsInCleanUp() const { return mbInCleanUp; }
    sal_Bool            IsPopupModeCanceled() const { return mbPopupModeCanceled; }
    sal_Bool            IsPopupModeTearOff() const { return mbPopupModeTearOff; }

    void            SetPopupModeEndHdl( const Link& rLink ) { maPopupModeEndHdl = rLink; }
    const Link&     GetPopupModeEndHdl() const  { return maPopupModeEndHdl; }

    sal_Bool            GrabsFocus() const { return mbGrabFocus; }

    static Point    CalcFloatingPosition( Window* pWindow, const Rectangle& rRect, sal_uLong nFlags, sal_uInt16& rArrangeIndex );
};

#endif // _SV_FLOATWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
