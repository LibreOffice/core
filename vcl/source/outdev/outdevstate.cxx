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

#include <sal/types.h>

#include <vcl/mapmod.hxx>
#include <vcl/region.hxx>
#include <vcl/font.hxx>
#include <vcl/vclenum.hxx>

#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <tools/fontenum.hxx>

#include <vcl/outdevstate.hxx>
#include "sallayout.hxx"

OutDevState::~OutDevState()
{
    if ( mnFlags & PUSH_LINECOLOR )
    {
        if ( mpLineColor )
            delete mpLineColor;
    }
    if ( mnFlags & PUSH_FILLCOLOR )
    {
        if ( mpFillColor )
            delete mpFillColor;
    }
    if ( mnFlags & PUSH_FONT )
        delete mpFont;
    if ( mnFlags & PUSH_TEXTCOLOR )
        delete mpTextColor;
    if ( mnFlags & PUSH_TEXTFILLCOLOR )
    {
        if ( mpTextFillColor )
            delete mpTextFillColor;
    }
    if ( mnFlags & PUSH_TEXTLINECOLOR )
    {
        if ( mpTextLineColor )
            delete mpTextLineColor;
    }
    if ( mnFlags & PUSH_OVERLINECOLOR )
    {
        if ( mpOverlineColor )
            delete mpOverlineColor;
    }
    if ( mnFlags & PUSH_MAPMODE )
    {
        if ( mpMapMode )
            delete mpMapMode;
    }
    if ( mnFlags & PUSH_CLIPREGION )
    {
        if ( mpClipRegion )
            delete mpClipRegion;
    }
    if ( mnFlags & PUSH_REFPOINT )
    {
        if ( mpRefPoint )
            delete mpRefPoint;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
