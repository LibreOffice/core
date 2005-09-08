/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabsplit.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:06:54 $
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

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "tabsplit.hxx"
#include "viewdata.hxx"
#include "dbfunc.hxx"

//==================================================================

ScTabSplitter::ScTabSplitter( Window* pParent, WinBits nWinStyle, ScViewData* pData ) :
    Splitter( pParent, nWinStyle ),
    pViewData(pData)
{
    SetFixed(FALSE);
    EnableRTL( FALSE );
}


ScTabSplitter::~ScTabSplitter()
{
}

void __EXPORT ScTabSplitter::MouseMove( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseMove( rMEvt );
    else
        Splitter::MouseMove( rMEvt );
}

void __EXPORT ScTabSplitter::MouseButtonUp( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseButtonUp( rMEvt );
    else
        Splitter::MouseButtonUp( rMEvt );
}

void __EXPORT ScTabSplitter::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseButtonDown( rMEvt );
    else
        Splitter::MouseButtonDown( rMEvt );
}

void __EXPORT ScTabSplitter::Splitting( Point& rSplitPos )
{
    Window* pParent = GetParent();
    Point aScreenPos = pParent->OutputToNormalizedScreenPixel( rSplitPos );
    pViewData->GetView()->SnapSplitPos( aScreenPos );
    Point aNew = pParent->NormalizedScreenToOutputPixel( aScreenPos );
    if ( IsHorizontal() )
        rSplitPos.X() = aNew.X();
    else
        rSplitPos.Y() = aNew.Y();
}


void ScTabSplitter::SetFixed(BOOL bSet)
{
    bFixed = bSet;
    if (bSet)
        SetPointer(POINTER_ARROW);
    else if (IsHorizontal())
        SetPointer(POINTER_HSPLIT);
    else
        SetPointer(POINTER_VSPLIT);
}



