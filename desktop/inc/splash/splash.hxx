/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#pragma once

#include <sal/config.h>

#include "SplashScreenBase.hxx"
#include "SplashScreenWindow.hxx"

#include <vcl/bitmap.hxx>
#include <vcl/vclevent.hxx>

class SplashScreen : public SplashScreenBase
{
private:
    VclPtr<SplashScreenWindow> m_pWindow;

    DECL_LINK(AppEventListenerHdl, VclSimpleEvent&, void);
    void updateStatus();

    bool m_bProgressEnd;

public:
    SplashScreen(const OUString& rAppName);
    virtual ~SplashScreen() override;

    virtual void end() override;
    virtual void reset() override;
    virtual void setText(const OUString& aText) override;
    virtual void setValue(sal_Int32 nValue) override;
    virtual void start() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
