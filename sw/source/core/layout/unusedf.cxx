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

void SwFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs * )
{
    OSL_FAIL( "Format() of the base class called." );
}

void SwFrame::Paint(vcl::RenderContext&, SwRect const&, SwPrintData const*const) const
{
    OSL_FAIL( "Paint() of the base class called." );
}

bool SwContentFrame::WouldFit( SwTwips &, bool&, bool )
{
    OSL_FAIL( "WouldFit of ContentFrame called." );
    return false;
}

bool SwFrame::FillSelection( SwSelectionList& , const SwRect& ) const
{
    OSL_FAIL( "Don't call this function at the base class!" );
    return false;
}

bool SwFrame::GetCursorOfst( SwPosition *, Point&, SwCursorMoveState*, bool  ) const
{
    OSL_FAIL( "GetCursorOfst of the base class, hi!" );
    return false;
}

#ifdef DBG_UTIL

void SwRootFrame::Cut()
{
    OSL_FAIL( "Cut() of RootFrame called." );
}

void SwRootFrame::Paste( SwFrame *, SwFrame * )
{
    OSL_FAIL( "Paste() of RootFrame called." );
}

void SwFlyFrame::Paste( SwFrame *, SwFrame * )
{
    OSL_FAIL( "Paste() of FlyFrame called." );
}

#endif

bool SwFrame::GetCharRect( SwRect&, const SwPosition&,
                         SwCursorMoveState* ) const
{
    OSL_FAIL( "GetCharRect() of the base called." );
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
