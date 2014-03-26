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

#ifndef INCLUDED_SVTOOLS_SOURCE_HATCHWINDOW_IPWIN_HXX
#define INCLUDED_SVTOOLS_SOURCE_HATCHWINDOW_IPWIN_HXX

#include <tools/gen.hxx>
#include <vcl/window.hxx>

/********************** SvResizeHelper ***********************************
*************************************************************************/
class SvResizeHelper
{
    Size        aBorder;
    Rectangle   aOuter;
    short       nGrab; // -1 no Grab,  0 - 7, 8 = Move, see FillHandle...
    Point       aSelPos;
    sal_Bool        bResizeable;
public:
                SvResizeHelper();

    void        SetResizeable( sal_Bool b ) { bResizeable = b; }
    short       GetGrab() const { return nGrab; }
    void        SetBorderPixel( const Size & rBorderP )
                { aBorder = rBorderP; }
    const Size & GetBorderPixel() const { return aBorder; }
    const Rectangle & GetOuterRectPixel() const
                { return aOuter; }
    void        SetOuterRectPixel( const Rectangle & rRect )
                { aOuter = rRect; }
    Rectangle   GetInnerRectPixel() const
                {
                    Rectangle aRect( aOuter );
                    aRect.Top()    += aBorder.Height();
                    aRect.Left()   += aBorder.Width();
                    aRect.Bottom() -= aBorder.Height();
                    aRect.Right()  -= aBorder.Width();
                    return aRect;
                }
                // Clockwise, start at upper left

    void        FillHandleRectsPixel( Rectangle aRects[ 8 ] ) const;
    void        FillMoveRectsPixel( Rectangle aRects[ 4 ] ) const;
    void        Draw( OutputDevice * );
    void        InvalidateBorder( Window * );
    sal_Bool        SelectBegin( Window *, const Point & rPos );
    short       SelectMove( Window * pWin, const Point & rPos );
    Point       GetTrackPosPixel( const Rectangle & rRect ) const;
    Rectangle   GetTrackRectPixel( const Point & rTrackPos ) const;
    void        ValidateRect( Rectangle & rValidate ) const;
    sal_Bool        SelectRelease( Window *, const Point & rPos, Rectangle & rOutPosSize );
    void        Release( Window * pWin );
};

/********************** SvResizeWindow ***********************************
*************************************************************************/
class VCLXHatchWindow;
class SvResizeWindow : public Window
{
    Pointer         m_aOldPointer;
    short           m_nMoveGrab;  // last pointer type
    SvResizeHelper  m_aResizer;
    sal_Bool        m_bActive;

    VCLXHatchWindow* m_pWrapper;
public:
    SvResizeWindow( Window* pParent, VCLXHatchWindow* pWrapper );

    void    SetHatchBorderPixel( const Size & rSize );

    void    SelectMouse( const Point & rPos );
    virtual void    MouseButtonUp( const MouseEvent & rEvt ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent & rEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent & rEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent & rEvt ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    Paint( const Rectangle & ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    void    QueryObjAreaPixel( Rectangle & );
    void    RequestObjAreaPixel( const Rectangle & );
};

#endif // INCLUDED_SVTOOLS_SOURCE_HATCHWINDOW_IPWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
