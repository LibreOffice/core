/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *               Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Jan Holesovsky <kendy@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <stdio.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <osl/mutex.hxx>
#include <rtl/bootstrap.hxx>

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;

namespace desktop {

class  UnxSplashScreen : public ::cppu::WeakImplHelper2< XStatusIndicator, XInitialization >
{
private:
    // don't allow anybody but ourselves to create instances of this class
    UnxSplashScreen( const UnxSplashScreen& );
    UnxSplashScreen( void );
    UnxSplashScreen operator =( const UnxSplashScreen& );

    UnxSplashScreen( const Reference< XMultiServiceFactory >& xFactory );

    virtual ~UnxSplashScreen();

    static  UnxSplashScreen *m_pINSTANCE;

    static osl::Mutex m_aMutex;
    Reference< XMultiServiceFactory > m_rFactory;

    FILE *m_pOutFd;

public:
    static const char* interfaces[];
    static const sal_Char *serviceName;
    static const sal_Char *implementationName;
    static const sal_Char *supportedServiceNames[];

    static Reference< XInterface > getInstance( const Reference < XMultiServiceFactory >& xFactory );

    // XStatusIndicator
    virtual void SAL_CALL start( const OUString& aText, sal_Int32 nRange ) throw ( RuntimeException );
    virtual void SAL_CALL end() throw ( RuntimeException );
    virtual void SAL_CALL reset() throw ( RuntimeException );
    virtual void SAL_CALL setText( const OUString& aText ) throw ( RuntimeException );
    virtual void SAL_CALL setValue( sal_Int32 nValue ) throw ( RuntimeException );

    // XInitialize
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArguments ) throw ( RuntimeException );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
