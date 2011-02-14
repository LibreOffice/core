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

#ifndef _IPWIN_HXX
#define _IPWIN_HXX

#include <tools/gen.hxx>
#include <vcl/window.hxx>
#include <tools/svborder.hxx>

/********************** SvResizeHelper ***********************************
*************************************************************************/
class SvResizeHelper
{
    Size        aBorder;
    Rectangle   aOuter;
    short       nGrab; // -1 kein Grab,  0 - 7, 8 = Move, siehe FillHandle...
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
                // Im Uhrzeigersinn, beginnend bei Linksoben
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
    short           m_nMoveGrab;  // Letzer Pointertyp
    SvResizeHelper  m_aResizer;
    sal_Bool        m_bActive;

    VCLXHatchWindow* m_pWrapper;
public:
    SvResizeWindow( Window* pParent, VCLXHatchWindow* pWrapper );

    void    SetHatchBorderPixel( const Size & rSize );

    void    SelectMouse( const Point & rPos );
    virtual void    MouseButtonUp( const MouseEvent & rEvt );
    virtual void    MouseMove( const MouseEvent & rEvt );
    virtual void    MouseButtonDown( const MouseEvent & rEvt );
    virtual void    KeyInput( const KeyEvent & rEvt );
    virtual void    Resize();
    virtual void    Paint( const Rectangle & );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

    void    QueryObjAreaPixel( Rectangle & );
    void    RequestObjAreaPixel( const Rectangle & );
};

#endif // _IPWIN_HXX

