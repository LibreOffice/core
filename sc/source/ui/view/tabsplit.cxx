/*************************************************************************
 *
 *  $RCSfile: tabsplit.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:09 $
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

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#endif

// INCLUDE ---------------------------------------------------------------

#include "tabsplit.hxx"
#include "viewdata.hxx"
#include "dbfunc.hxx"

// STATIC DATA -----------------------------------------------------------

SEG_EOFGLOBALS()


//==================================================================

#pragma SEG_FUNCDEF(tabsplit_01)

ScTabSplitter::ScTabSplitter( Window* pParent, WinBits nWinStyle, ScViewData* pData ) :
    Splitter( pParent, nWinStyle ),
    pViewData(pData)
{
    SetFixed(FALSE);
}

#pragma SEG_FUNCDEF(tabsplit_02)

ScTabSplitter::~ScTabSplitter()
{
}

#pragma SEG_FUNCDEF(tabsplit_03)

void __EXPORT ScTabSplitter::MouseMove( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseMove( rMEvt );
    else
        Splitter::MouseMove( rMEvt );
}

#pragma SEG_FUNCDEF(tabsplit_04)

void __EXPORT ScTabSplitter::MouseButtonUp( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseButtonUp( rMEvt );
    else
        Splitter::MouseButtonUp( rMEvt );
}

#pragma SEG_FUNCDEF(tabsplit_05)

void __EXPORT ScTabSplitter::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (bFixed)
        Window::MouseButtonDown( rMEvt );
    else
        Splitter::MouseButtonDown( rMEvt );
}

#pragma SEG_FUNCDEF(tabsplit_07)

void __EXPORT ScTabSplitter::Splitting( Point& rSplitPos )
{
    Window* pParent = GetParent();
    Point aScreenPos = pParent->OutputToScreenPixel( rSplitPos );
    pViewData->GetView()->SnapSplitPos( aScreenPos );
    Point aNew = pParent->ScreenToOutputPixel( aScreenPos );
    if ( IsHorizontal() )
        rSplitPos.X() = aNew.X();
    else
        rSplitPos.Y() = aNew.Y();
}

#pragma SEG_FUNCDEF(tabsplit_06)

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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.8  2000/09/17 14:09:34  willem.vandorp
    OpenOffice header added.

    Revision 1.7  2000/08/31 16:38:47  willem.vandorp
    Header and footer replaced

    Revision 1.6  1997/08/27 10:56:38  TRI
    VCL Anpassung


      Rev 1.5   27 Aug 1997 12:56:38   TRI
   VCL Anpassung

      Rev 1.4   24 May 1996 10:58:44   NN
   Position einrasten in Splitting

      Rev 1.3   14 Feb 1996 20:32:34   NN
   MouseButtonDown, bFixed auswerten

      Rev 1.2   02 Feb 1996 16:28:18   NN
   #25064# Freeze wieder raus

      Rev 1.1   27 Oct 1995 18:02:58   NN
   Test: Freeze auf rechter Maustaste

      Rev 1.0   25 Oct 1995 16:35:44   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


