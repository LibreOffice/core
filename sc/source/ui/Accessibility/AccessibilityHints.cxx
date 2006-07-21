/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibilityHints.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:59:51 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "AccessibilityHints.hxx"

using namespace ::com::sun::star;

// -----------------------------------------------------------------------

TYPEINIT1(ScAccWinFocusLostHint, SfxHint);

// -----------------------------------------------------------------------
//      ScAccWinFocusLostHint - the current window lost its focus (to another application, view or document)
// -----------------------------------------------------------------------

ScAccWinFocusLostHint::ScAccWinFocusLostHint(
        const uno::Reference< uno::XInterface >& xOld )
    :
    xOldAccessible(xOld)
{
}

ScAccWinFocusLostHint::~ScAccWinFocusLostHint()
{
}

// -----------------------------------------------------------------------

TYPEINIT1(ScAccWinFocusGotHint, SfxHint);

// -----------------------------------------------------------------------
//      ScAccWinFocusGotHint - the window got the focus (from another application, view or document)
// -----------------------------------------------------------------------

ScAccWinFocusGotHint::ScAccWinFocusGotHint(
        const uno::Reference< uno::XInterface >& xNew )
    :
    xNewAccessible(xNew)
{
}

ScAccWinFocusGotHint::~ScAccWinFocusGotHint()
{
}
// -----------------------------------------------------------------------

TYPEINIT1(ScAccGridWinFocusLostHint, SfxHint);

// -----------------------------------------------------------------------
//      ScAccGridWinFocusLostHint - the current grid window lost its focus (to another application, view or document)
// -----------------------------------------------------------------------

ScAccGridWinFocusLostHint::ScAccGridWinFocusLostHint(ScSplitPos eOld,
        const uno::Reference< uno::XInterface >& xOld )
    :
    ScAccWinFocusLostHint(xOld),
    eOldGridWin(eOld)
{
}

ScAccGridWinFocusLostHint::~ScAccGridWinFocusLostHint()
{
}

// -----------------------------------------------------------------------

TYPEINIT1(ScAccGridWinFocusGotHint, SfxHint);

// -----------------------------------------------------------------------
//      ScAccGridWinFocusGotHint - the grid window got the focus (from another application, view or document)
// -----------------------------------------------------------------------

ScAccGridWinFocusGotHint::ScAccGridWinFocusGotHint(ScSplitPos eNew,
        const uno::Reference< uno::XInterface >& xNew )
    :
    ScAccWinFocusGotHint(xNew),
    eNewGridWin(eNew)
{
}

ScAccGridWinFocusGotHint::~ScAccGridWinFocusGotHint()
{
}
