/*************************************************************************
 *
 *  $RCSfile: ipwin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:03:02 $
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

#ifndef _IPWIN_HXX
#define _IPWIN_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SVBORDER_HXX
#include <tools/svborder.hxx>
#endif

/********************** SvResizeHelper ***********************************
*************************************************************************/
class SvResizeHelper
{
    Size        aBorder;
    Rectangle   aOuter;
    short       nGrab; // -1 kein Grab,  0 - 7, 8 = Move, siehe FillHandle...
    Point       aSelPos;
    BOOL        bResizeable;
public:
                SvResizeHelper();

    void        SetResizeable( BOOL b ) { bResizeable = b; }
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
    BOOL        SelectBegin( Window *, const Point & rPos );
    void        SelectBegin( Window *, short nGrab );
    short       SelectMove( Window * pWin, const Point & rPos );
    Point       GetTrackPosPixel( const Rectangle & rRect ) const;
    Rectangle   GetTrackRectPixel( const Point & rTrackPos ) const;
    void        ValidateRect( Rectangle & rValidate ) const;
    BOOL        SelectRelease( Window *, const Point & rPos, Rectangle & rOutPosSize );
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

    VCLXHatchWindow* m_pWrapper;
public:
    SvResizeWindow( Window* pParent, VCLXHatchWindow* pWrapper );

    void    SetHatchBorderPixel( const Size & rSize );
    const Size & GetHatchBorderPixel() const;
    void TerminateResizing() ;
    SvBorder GetAllBorderPixel() const;

    void    SelectMouse( const Point & rPos );
    void    MouseButtonUp( const MouseEvent & rEvt );
    void    MouseMove( const MouseEvent & rEvt );
    void    MouseButtonDown( const MouseEvent & rEvt );
    void    KeyInput( const KeyEvent & rEvt );
    void    Resize();
    void    Paint( const Rectangle & );

    void    QueryObjAreaPixel( Rectangle & );
    void    RequestObjAreaPixel( const Rectangle & );
};

#endif // _IPWIN_HXX

