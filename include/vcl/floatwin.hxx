/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_FLOATWIN_HXX
#define INCLUDED_VCL_FLOATWIN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>

class ToolBox;


// - FloatingWindow-Types -


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
#define FLOATWIN_POPUPMODE_NEWLEVEL             ((sal_uLong)0x00004000)
#define FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE       ((sal_uLong)0x00008000)
#define FLOATWIN_POPUPMODE_GRABFOCUS            ((sal_uLong)0x00010000)
#define FLOATWIN_POPUPMODE_NOHORZPLACEMENT      ((sal_uLong)0x00020000)

#define FLOATWIN_POPUPMODEEND_CANCEL            ((sal_uInt16)0x0001)
#define FLOATWIN_POPUPMODEEND_TEAROFF           ((sal_uInt16)0x0002)
#define FLOATWIN_POPUPMODEEND_DONTCALLHDL       ((sal_uInt16)0x0004)
#define FLOATWIN_POPUPMODEEND_CLOSEALL          ((sal_uInt16)0x0008)

#define FLOATWIN_TITLE_NORMAL                   ((sal_uInt16)0x0001)
#define FLOATWIN_TITLE_TEAROFF                  ((sal_uInt16)0x0002)
#define FLOATWIN_TITLE_POPUP                    ((sal_uInt16)0x0004)
#define FLOATWIN_TITLE_NONE                     ((sal_uInt16)0x0008)


enum HitTest
{
    HITTEST_OUTSIDE,
    HITTEST_WINDOW,
    HITTEST_RECT
};

// - FloatingWindow -

class VCL_DLLPUBLIC FloatingWindow : public SystemWindow
{
    class   ImplData;
private:
    FloatingWindow* mpNextFloat;
    vcl::Window*         mpFirstPopupModeWin;
    ImplData*       mpImplData;
    Rectangle       maFloatRect;
    ImplSVEvent *   mnPostId;
    sal_uLong           mnPopupModeFlags;
    sal_uInt16          mnTitle;
    sal_uInt16          mnOldTitle;
    bool            mbInPopupMode;
    bool            mbPopupMode;
    bool            mbPopupModeCanceled;
    bool            mbPopupModeTearOff;
    bool            mbMouseDown;
    bool            mbOldSaveBackMode;
    bool            mbGrabFocus;    // act as key input window, although focus is not set
    bool            mbInCleanUp;
    Link            maPopupModeEndHdl;

    SAL_DLLPRIVATE void    ImplCallPopupModeEnd();
    DECL_DLLPRIVATE_LINK(  ImplEndPopupModeHdl, void* );
    virtual void setPosSizeOnContainee(Size aSize, VclContainer &rBox) SAL_OVERRIDE;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         FloatingWindow (const FloatingWindow &);
    SAL_DLLPRIVATE         FloatingWindow & operator= (const FloatingWindow &);

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();

public:
    SAL_DLLPRIVATE FloatingWindow*  ImplFloatHitTest( vcl::Window* pReference, const Point& rPos, HitTest& rHitTest );
    SAL_DLLPRIVATE FloatingWindow*  ImplFindLastLevelFloat();
    SAL_DLLPRIVATE bool             ImplIsFloatPopupModeWindow( const vcl::Window* pWindow );
    SAL_DLLPRIVATE void             ImplSetMouseDown() { mbMouseDown = true; }
    SAL_DLLPRIVATE bool             ImplIsMouseDown() const  { return mbMouseDown; }
    SAL_DLLPRIVATE static Point     ImplCalcPos( vcl::Window* pWindow,
                                                 const Rectangle& rRect, sal_uLong nFlags,
                                                 sal_uInt16& rArrangeIndex );
    SAL_DLLPRIVATE void             ImplEndPopupMode( sal_uInt16 nFlags = 0, sal_uLong nFocusId = 0 );
    SAL_DLLPRIVATE Rectangle&       ImplGetItemEdgeClipRect();
    SAL_DLLPRIVATE bool             ImplIsInPrivatePopupMode() const { return mbInPopupMode; }
    virtual        void             doDeferredInit(WinBits nBits) SAL_OVERRIDE;

public:
    explicit        FloatingWindow(vcl::Window* pParent, WinBits nStyle = WB_STDFLOATWIN);
    explicit        FloatingWindow(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
                                   const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
    virtual         ~FloatingWindow();

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void    PopupModeEnd();

    void            SetTitleType( sal_uInt16 nTitle );
    sal_uInt16      GetTitleType() const { return mnTitle; }

    void            StartPopupMode( const Rectangle& rRect, sal_uLong nFlags = 0 );
    void            StartPopupMode( ToolBox* pBox, sal_uLong nFlags = 0  );
    void            EndPopupMode( sal_uInt16 nFlags = 0 );
    void            AddPopupModeWindow( vcl::Window* pWindow );
    sal_uLong           GetPopupModeFlags() const { return mnPopupModeFlags; }
    void            SetPopupModeFlags( sal_uLong nFlags ) { mnPopupModeFlags = nFlags; }
    bool            IsInPopupMode() const { return mbPopupMode; }
    bool            IsInCleanUp() const { return mbInCleanUp; }
    bool            IsPopupModeCanceled() const { return mbPopupModeCanceled; }
    bool            IsPopupModeTearOff() const { return mbPopupModeTearOff; }

    void            SetPopupModeEndHdl( const Link& rLink ) { maPopupModeEndHdl = rLink; }
    const Link&     GetPopupModeEndHdl() const  { return maPopupModeEndHdl; }

    bool            GrabsFocus() const { return mbGrabFocus; }

    static Point    CalcFloatingPosition( vcl::Window* pWindow, const Rectangle& rRect, sal_uLong nFlags, sal_uInt16& rArrangeIndex );
};

#endif // INCLUDED_VCL_FLOATWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
