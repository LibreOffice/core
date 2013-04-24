/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <osl/mutex.hxx>
#include <rtl/bootstrap.hxx>

namespace desktop {

class  UnxSplashScreen : public ::cppu::WeakImplHelper2< css::task::XStatusIndicator, css::lang::XInitialization >
{
private:
    // don't allow anybody but ourselves to create instances of this class
    UnxSplashScreen( const UnxSplashScreen& );
    UnxSplashScreen( void );
    UnxSplashScreen operator =( const UnxSplashScreen& );

    virtual ~UnxSplashScreen();

    static  UnxSplashScreen *m_pINSTANCE;

    static osl::Mutex m_aMutex;
    css::uno::Reference< css::uno::XComponentContext > m_xCtx;

    FILE *m_pOutFd;

public:
    UnxSplashScreen( const css::uno::Reference< css::uno::XComponentContext >& xCtx );

    // XStatusIndicator
    virtual void SAL_CALL start( const OUString& aText, sal_Int32 nRange ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL end() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL reset() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setText( const OUString& aText ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setValue( sal_Int32 nValue ) throw ( css::uno::RuntimeException );

    // XInitialize
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any>& aArguments ) throw ( css::uno::RuntimeException );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
