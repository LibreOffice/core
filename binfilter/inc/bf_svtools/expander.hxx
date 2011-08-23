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

#ifndef _SV_EXPANDER_HXX
#define _SV_EXPANDER_HXX

#include <vcl/ctrl.hxx>

#include <vcl/image.hxx>

namespace binfilter
{

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
        Point					aImagePos;
        Point					aTextPos;
        Image					aActiveImage;
        Rectangle				maFocusRect;
        ImageList				maExpanderImages;
        BOOL					mbIsExpanded;
        BOOL					mbHasFocusRect;
        BOOL					mbIsInMouseDown;
        Link					maToggleHdl;
        SvExpanderStateType	eType;

protected:
        
        virtual long	PreNotify( NotifyEvent& rNEvt );
        virtual void	MouseButtonDown( const MouseEvent& rMEvt );
        virtual void	MouseMove( const MouseEvent& rMEvt );
        virtual void	MouseButtonUp( const MouseEvent& rMEvt );
        virtual void	Paint( const Rectangle& rRect );
        virtual void	KeyInput( const KeyEvent& rKEvt );
        virtual void	KeyUp( const KeyEvent& rKEvt );

        virtual void	Click();
        virtual void	Resize();

public:
        SvExpander( Window* pParent, WinBits nStyle = 0 ); 
        SvExpander( Window* pParent, const ResId& rResId );

        BOOL			IsExpanded() {return mbIsExpanded;}

        void			SetToExpanded(BOOL bFlag=TRUE);

        void			SetExpanderImage( SvExpanderStateType eType);
        Image			GetExpanderImage(SvExpanderStateType eType);
        Size			GetMinSize() const;

        void			SetToggleHdl( const Link& rLink ) { maToggleHdl = rLink; }
        const Link& 	GetToggleHdl() const { return maToggleHdl; }
};

}

#endif	

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
