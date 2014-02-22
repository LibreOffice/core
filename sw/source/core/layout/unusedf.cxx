/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

bool SwCntntFrm::WouldFit( SwTwips &, bool&, bool )
{
    OSL_FAIL( "WouldFit of CntntFrm called." );
    return false;
}

bool SwFrm::FillSelection( SwSelectionList& , const SwRect& ) const
{
    OSL_FAIL( "Don't call this function at the base class!" );
    return false;
}

bool SwFrm::GetCrsrOfst( SwPosition *, Point&, SwCrsrMoveState*, bool  ) const
{
    OSL_FAIL( "GetCrsrOfst of the base class, hi!" );
    return false;
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

bool SwFrm::GetCharRect( SwRect&, const SwPosition&,
                         SwCrsrMoveState* ) const
{
    OSL_FAIL( "GetCharRect() of the base called." );
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
