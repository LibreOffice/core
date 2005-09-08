/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expander.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:25:35 $
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

#ifndef _SV_EXPANDER_HXX
#define _SV_EXPANDER_HXX

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif

#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

enum SvExpanderStateType
{
    EST_MIN=1,
    EST_PLUS=2,
    EST_MIN_DOWN=3,
    EST_PLUS_DOWN=4,
    EST_NONE=5,
    EST_MIN_DIS=6,
    EST_PLUS_DIS=7,
    EST_MIN_DOWN_DIS=8,
    EST_PLUS_DOWN_DIS=9
};

class SvExpander: public Control
{
private:
        Point                   aImagePos;
        Point                   aTextPos;
        Image                   aActiveImage;
        Rectangle               maFocusRect;
        ImageList               maExpanderImages;
        BOOL                    mbIsExpanded;
        BOOL                    mbHasFocusRect;
        BOOL                    mbIsInMouseDown;
        Link                    maToggleHdl;
        SvExpanderStateType eType;

protected:

        virtual long    PreNotify( NotifyEvent& rNEvt );
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
        virtual void    MouseMove( const MouseEvent& rMEvt );
        virtual void    MouseButtonUp( const MouseEvent& rMEvt );
        virtual void    Paint( const Rectangle& rRect );
        virtual void    KeyInput( const KeyEvent& rKEvt );
        virtual void    KeyUp( const KeyEvent& rKEvt );

        virtual void    Click();
        virtual void    Resize();

public:
        SvExpander( Window* pParent, WinBits nStyle = 0 );
        SvExpander( Window* pParent, const ResId& rResId );

        BOOL            IsExpanded() {return mbIsExpanded;}

        void            SetToExpanded(BOOL bFlag=TRUE);

        void            SetExpanderImage( SvExpanderStateType eType);
        Image           GetExpanderImage(SvExpanderStateType eType);
        Size            GetMinSize() const;

        void            SetToggleHdl( const Link& rLink ) { maToggleHdl = rLink; }
        const Link&     GetToggleHdl() const { return maToggleHdl; }
};



#endif
