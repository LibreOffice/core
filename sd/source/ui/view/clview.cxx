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

#include <sfx2/app.hxx>

#include "ClientView.hxx"
#include "drawview.hxx"
#include "sdpage.hxx"

class DrawDocShell;

namespace sd {

class DrawViewShell;

/**
 * ClientView is used for DrawDocShell::Draw()
 */

ClientView::ClientView(
    DrawDocShell* pDocSh,
    OutputDevice* pOutDev,
    DrawViewShell* pShell)
    : DrawView (pDocSh, pOutDev, pShell)
{
}


ClientView::~ClientView()
{
}

/**
 * If View should not Invalidate() the windows, this method has
 * to be overloaded and properly handled.
 */

void ClientView::InvalidateOneWin(::Window& rWin)
{
    Region aRegion;
    CompleteRedraw(&rWin, aRegion);
}

/**
 * If View should not Invalidate() the windows, this method has
 * to be overloaded and properly handled.
 */

void ClientView::InvalidateOneWin(::Window& rWin, const Rectangle& rRect)
{
    CompleteRedraw(&rWin, Region(rRect));
}

void ClientView::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector )
{
    DrawView::CompleteRedraw(pOutDev, rReg, pRedirector);
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
