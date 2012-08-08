/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <stdtypes.h>
#include <properties.h>
#include <stdtypes.h>
#include <uielement/menubarmanager.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>

namespace framework
{
    class License : public  css::lang::XTypeProvider            ,
                    public  css::lang::XServiceInfo             ,
                    public  css::task::XJob                     ,
                    public  css::util::XCloseable               ,
                    // base classes
                    // Order is neccessary for right initialization!
                    private ThreadHelpBase                      , // Struct for right initalization of mutex member! Must be first of baseclasses.
                    public  ::cppu::OWeakObject                   // => XWeak, XInterface
    {
    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
        sal_Bool m_bTerminate;
    public:
        License( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMGR );
        virtual ~License();

        /** declaration of XInterface, XTypeProvider, XServiceInfo */
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
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
