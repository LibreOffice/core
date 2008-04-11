/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: floatwin.hxx,v $
 * $Revision: 1.3 $
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

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>

class ToolBox;
class PopupModeEvent;

// ------------------------
// - FloatingWindow-Types -
// ------------------------

#define FLOATWIN_POPUPMODE_ALLOWTEAROFF         ((ULONG)0x00000001)
#define FLOATWIN_POPUPMODE_ANIMATIONSLIDE       ((ULONG)0x00000002)
#define FLOATWIN_POPUPMODE_NOAUTOARRANGE        ((ULONG)0x00000004)
#define FLOATWIN_POPUPMODE_NOANIMATION          ((ULONG)0x00000008)
#define FLOATWIN_POPUPMODE_DOWN                 ((ULONG)0x00000010)
#define FLOATWIN_POPUPMODE_UP                   ((ULONG)0x00000020)
#define FLOATWIN_POPUPMODE_LEFT                 ((ULONG)0x00000040)
#define FLOATWIN_POPUPMODE_RIGHT                ((ULONG)0x00000080)
#define FLOATWIN_POPUPMODE_NOFOCUSCLOSE         ((ULONG)0x00000100)
#define FLOATWIN_POPUPMODE_NOKEYCLOSE           ((ULONG)0x00000200)
#define FLOATWIN_POPUPMODE_NOMOUSECLOSE         ((ULONG)0x00000400)
#define FLOATWIN_POPUPMODE_NOMOUSERECTCLOSE     ((ULONG)0x00000800)
#define FLOATWIN_POPUPMODE_ALLMOUSEBUTTONCLOSE  ((ULONG)0x00001000)
#define FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE      ((ULONG)0x00002000)
#define FLOATWIN_POPUPMODE_PATHMOUSECANCELCLICK ((ULONG)0x00004000)
#define FLOATWIN_POPUPMODE_NEWLEVEL             ((ULONG)0x00008000)
#define FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE       ((ULONG)0x00010000)
#define FLOATWIN_POPUPMODE_GRABFOCUS            ((ULONG)0x00020000)

#define FLOATWIN_POPUPMODEEND_CANCEL            ((USHORT)0x0001)
#define FLOATWIN_POPUPMODEEND_TEAROFF           ((USHORT)0x0002)
#define FLOATWIN_POPUPMODEEND_DONTCALLHDL       ((USHORT)0x0004)
#define FLOATWIN_POPUPMODEEND_CLOSEALL          ((USHORT)0x0008)

#define FLOATWIN_TITLE_NORMAL                   ((USHORT)0x0001)
#define FLOATWIN_TITLE_TEAROFF                  ((USHORT)0x0002)
#define FLOATWIN_TITLE_NONE                     ((USHORT)0x0004)

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
    ULONG           mnPostId;
    ULONG           mnPopupModeFlags;
    USHORT          mnTitle;
    USHORT          mnOldTitle;
    BOOL            mbInPopupMode;
    BOOL            mbPopupMode;
    BOOL            mbPopupModeCanceled;
    BOOL            mbPopupModeTearOff;
    BOOL            mbMouseDown;
    BOOL            mbOldSaveBackMode;
    BOOL            mbGrabFocus;    // act as key input window, although focus is not set
    BOOL            mbInCleanUp;
    Link            maPopupModeEndHdl;

//#if 0 // _SOLAR__PRIVATE
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
#define IMPL_FLOATWIN_HITTEST_OUTSIDE       ((USHORT)0x0001)
#define IMPL_FLOATWIN_HITTEST_WINDOW        ((USHORT)0x0002)
#define IMPL_FLOATWIN_HITTEST_RECT          ((USHORT)0x0004)
    SAL_DLLPRIVATE FloatingWindow*  ImplFloatHitTest( Window* pReference, const Point& rPos, USHORT& rHitTest );
    SAL_DLLPRIVATE FloatingWindow*  ImplFindLastLevelFloat();
    SAL_DLLPRIVATE BOOL             ImplIsFloatPopupModeWindow( const Window* pWindow );
    SAL_DLLPRIVATE void             ImplSetMouseDown() { mbMouseDown = TRUE; }
    SAL_DLLPRIVATE BOOL             ImplIsMouseDown() const  { return mbMouseDown; }
    SAL_DLLPRIVATE static Point     ImplCalcPos( Window* pWindow,
                                                 const Rectangle& rRect, ULONG nFlags,
                                                 USHORT& rArrangeIndex );
    SAL_DLLPRIVATE void             ImplEndPopupMode( USHORT nFlags = 0, ULONG nFocusId = 0 );
    SAL_DLLPRIVATE Rectangle&       ImplGetItemEdgeClipRect();
    SAL_DLLPRIVATE BOOL             ImplIsInPrivatePopupMode() const { return mbInPopupMode; }
//#endif

public:
                    FloatingWindow( Window* pParent, WinBits nStyle = WB_STDFLOATWIN );
                    FloatingWindow( Window* pParent, const ResId& rResId );
                    ~FloatingWindow();

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    PopupModeEnd();

    void            SetTitleType( USHORT nTitle );
    USHORT          GetTitleType() const { return mnTitle; }

    void            StartPopupMode( const Rectangle& rRect, ULONG nFlags = 0 );
    void            StartPopupMode( ToolBox* pBox, ULONG nFlags = 0  );
    void            EndPopupMode( USHORT nFlags = 0 );
    void            AddPopupModeWindow( Window* pWindow );
    void            RemovePopupModeWindow( Window* pWindow );
    ULONG           GetPopupModeFlags() const { return mnPopupModeFlags; }
    void            SetPopupModeFlags( ULONG nFlags ) { mnPopupModeFlags = nFlags; }
    BOOL            IsInPopupMode() const { return mbPopupMode; }
    BOOL            IsInCleanUp() const { return mbInCleanUp; }
    BOOL            IsPopupModeCanceled() const { return mbPopupModeCanceled; }
    BOOL            IsPopupModeTearOff() const { return mbPopupModeTearOff; }

    void            SetPopupModeEndHdl( const Link& rLink ) { maPopupModeEndHdl = rLink; }
    const Link&     GetPopupModeEndHdl() const  { return maPopupModeEndHdl; }

    BOOL            GrabsFocus() const { return mbGrabFocus; }
};

#endif // _SV_FLOATWIN_HXX
