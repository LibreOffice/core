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

#ifndef __FRAMEWORK_SERVICES_LICENSE_HXX_
#define __FRAMEWORK_SERVICES_LICENSE_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/resetableguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <properties.h>
#include <stdtypes.h>
#include <uielement/menubarmanager.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/propshlp.hxx>

namespace framework
{
    class License : // base classes
                    // Order is necessary for right initialization!
                    private ThreadHelpBase                      , // Struct for right initalization of mutex member! Must be first of baseclasses.
                    public cppu::WeakImplHelper3<
                               css::lang::XServiceInfo,
                               css::task::XJob,
                               css::util::XCloseable>             // => XWeak, XInterface
    {
    private:
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
        sal_Bool m_bTerminate;
    public:
        License( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~License();

        /** declaration of XInterface, XTypeProvider, XServiceInfo */
        DECLARE_XSERVICEINFO

        virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence<css::beans::NamedValue>& args)
            throw( css::lang::IllegalArgumentException, css::uno::Exception);

        virtual void SAL_CALL close(sal_Bool bDeliverOwnership) throw (css::util::CloseVetoException);

        virtual void SAL_CALL addCloseListener(const css::uno::Reference< css::util::XCloseListener >& aListener) throw (css::uno::RuntimeException);
        virtual void SAL_CALL removeCloseListener(const css::uno::Reference< css::util::XCloseListener >& aListener) throw (css::uno::RuntimeException);

};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
