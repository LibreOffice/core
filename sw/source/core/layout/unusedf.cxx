/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unusedf.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:38:05 $
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
#include "precompiled_sw.hxx"



#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"
#include "errhdl.hxx"


void SwFrm::Format( const SwBorderAttrs * )
{
    ASSERT( FALSE, "Format() der Basisklasse gerufen." );
}

void SwFrm::Paint(const SwRect &) const
{
    ASSERT( FALSE, "Paint() der Basisklasse gerufen." );
}

sal_Bool SwCntntFrm::WouldFit( SwTwips &, sal_Bool&, sal_Bool )
{
    ASSERT( FALSE, "WouldFit des CntntFrm gerufen." );
    return FALSE;
}

bool SwFrm::FillSelection( SwSelectionList& , const SwRect& ) const
{
    ASSERT( false, "Don't call this function at the base class!" );
    return false;
}

BOOL SwFrm::GetCrsrOfst( SwPosition *, Point&, SwCrsrMoveState*  ) const
{
    ASSERT( FALSE, "GetCrsrOfst der Basisklasse, hi!" );
    return FALSE;
}

#ifndef PRODUCT

void SwRootFrm::Cut()
{
    ASSERT( FALSE, "Cut() des RootFrm gerufen." );
}

void SwRootFrm::Paste( SwFrm *, SwFrm * )
{
    ASSERT( FALSE, "Paste() des RootFrm gerufen." );
}

void SwFlyFrm::Paste( SwFrm *, SwFrm * )
{
    ASSERT( FALSE, "Paste() des FlyFrm gerufen." );
}

#endif

BOOL SwFrm::GetCharRect( SwRect&, const SwPosition&,
                         SwCrsrMoveState* ) const
{
    ASSERT( FALSE, "GetCharRect() der Basis gerufen." );
    return FALSE;
}


