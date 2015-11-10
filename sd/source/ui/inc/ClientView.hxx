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

#ifndef INCLUDED_SD_SOURCE_UI_INC_CLIENTVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_CLIENTVIEW_HXX

#include "drawview.hxx"

namespace sd {

class DrawViewShell;

/**
 * The SdClientView is used for DrawDocShell::Draw()
 */
class ClientView
    : public DrawView
{
public:
    ClientView (
        DrawDocShell* pDocSh,
        OutputDevice* pOutDev,
        DrawViewShell* pShell);
    virtual ~ClientView();

    virtual void CompleteRedraw(OutputDevice* pOutDev, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = nullptr) override;

    /* if the view should not do a Invalidate() on the windows, you have to
       override the following two methods and do something different */
    virtual void InvalidateOneWin(vcl::Window& rWin) override;
    virtual void InvalidateOneWin(vcl::Window& rWin, const Rectangle& rRect) override;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
