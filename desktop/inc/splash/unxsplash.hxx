/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "SplashScreenBase.hxx"
#include "splash_dllapi.h"

#include <stdio.h>

#include <cppuhelper/implbase.hxx>

namespace desktop {

class SPLASH_DLLPUBLIC UnxSplashScreen : public SplashScreenBase
{
private:
    UnxSplashScreen( const UnxSplashScreen& ) = delete;
    UnxSplashScreen operator =( const UnxSplashScreen& ) = delete;


    FILE *m_pOutFd;

public:
    explicit UnxSplashScreen();
    virtual ~UnxSplashScreen() override;

    virtual void start(const OUString& aText, sal_Int32 nRange) override;
    virtual void end() override;
    virtual void reset() override;
    virtual void setText(const OUString& aText) override;
    virtual void setValue(sal_Int32 nValue) override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
