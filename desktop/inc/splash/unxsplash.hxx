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

#include <stdio.h>

#include <cppuhelper/implbase.hxx>

namespace desktop {

class UnxSplashScreen : public SplashScreenBase
{
private:
    UnxSplashScreen( const UnxSplashScreen& ) = delete;
    UnxSplashScreen operator =( const UnxSplashScreen& ) = delete;

    virtual ~UnxSplashScreen() override;

    FILE *m_pOutFd;

public:
    explicit UnxSplashScreen();

    // XStatusIndicator
    virtual void SAL_CALL start( const OUString& aText, sal_Int32 nRange ) override;
    virtual void SAL_CALL end() override;
    virtual void SAL_CALL reset() override;
    virtual void SAL_CALL setText( const OUString& aText ) override;
    virtual void SAL_CALL setValue( sal_Int32 nValue ) override;

    // XInitialize
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any>& aArguments ) override;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
