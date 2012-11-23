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


#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"


void SwFrm::Format( const SwBorderAttrs * )
{
    OSL_FAIL( "Format() of the base class called." );
}

void SwFrm::Paint(SwRect const&, SwPrintData const*const) const
{
    OSL_FAIL( "Paint() of the base class called." );
}

sal_Bool SwCntntFrm::WouldFit( SwTwips &, sal_Bool&, sal_Bool )
{
    OSL_FAIL( "WouldFit of CntntFrm called." );
    return sal_False;
}

bool SwFrm::FillSelection( SwSelectionList& , const SwRect& ) const
{
    OSL_FAIL( "Don't call this function at the base class!" );
    return false;
}

sal_Bool SwFrm::GetCrsrOfst( SwPosition *, Point&, SwCrsrMoveState*, bool  ) const
{
    OSL_FAIL( "GetCrsrOfst of the base class, hi!" );
    return sal_False;
}

#ifdef DBG_UTIL

void SwRootFrm::Cut()
{
    OSL_FAIL( "Cut() of RootFrm called." );
}

void SwRootFrm::Paste( SwFrm *, SwFrm * )
{
    OSL_FAIL( "Paste() of RootFrm called." );
}

void SwFlyFrm::Paste( SwFrm *, SwFrm * )
{
    OSL_FAIL( "Paste() of FlyFrm called." );
}

#endif

sal_Bool SwFrm::GetCharRect( SwRect&, const SwPosition&,
                         SwCrsrMoveState* ) const
{
    OSL_FAIL( "GetCharRect() of the base called." );
    return sal_False;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
