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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"


void SwFrm::Format( const SwBorderAttrs * )
{
    OSL_ENSURE( sal_False, "Format() der Basisklasse gerufen." );
}

void SwFrm::Paint(SwRect const&, SwPrintData const*const) const
{
    OSL_ENSURE( sal_False, "Paint() der Basisklasse gerufen." );
}

sal_Bool SwCntntFrm::WouldFit( SwTwips &, sal_Bool&, sal_Bool )
{
    OSL_ENSURE( sal_False, "WouldFit des CntntFrm gerufen." );
    return sal_False;
}

bool SwFrm::FillSelection( SwSelectionList& , const SwRect& ) const
{
    OSL_ENSURE( false, "Don't call this function at the base class!" );
    return false;
}

sal_Bool SwFrm::GetCrsrOfst( SwPosition *, Point&, SwCrsrMoveState*  ) const
{
    OSL_ENSURE( sal_False, "GetCrsrOfst der Basisklasse, hi!" );
    return sal_False;
}

#if OSL_DEBUG_LEVEL > 1

void SwRootFrm::Cut()
{
    OSL_ENSURE( sal_False, "Cut() des RootFrm gerufen." );
}

void SwRootFrm::Paste( SwFrm *, SwFrm * )
{
    OSL_ENSURE( sal_False, "Paste() des RootFrm gerufen." );
}

void SwFlyFrm::Paste( SwFrm *, SwFrm * )
{
    OSL_ENSURE( sal_False, "Paste() des FlyFrm gerufen." );
}

#endif

sal_Bool SwFrm::GetCharRect( SwRect&, const SwPosition&,
                         SwCrsrMoveState* ) const
{
    OSL_ENSURE( sal_False, "GetCharRect() der Basis gerufen." );
    return sal_False;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
