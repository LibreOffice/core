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

#ifndef __FRAMEWORK_DISPATCH_UIEVENTLOGHELPER_HXX_
#define __FRAMEWORK_DISPATCH_UIEVENTLOGHELPER_HXX_

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/URL.hpp>
#include <comphelper/uieventslogger.hxx>
#include <rtl/ustring.hxx>
#include <services.h>

namespace framework
{
    class UiEventLogHelper
    {
        public:
            UiEventLogHelper(const ::rtl::OUString& aWidgetname)
                : m_aWidgetName(aWidgetname)
                , m_hasAppName(false)
            { }

            void log(const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                const ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& rModel,
                const ::com::sun::star::util::URL& rUrl,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs)
            {

                if(!m_hasAppName && rServiceManager.is() && rModel.is())
                {
                    try
                    {
                        static ::rtl::OUString our_aModuleManagerName = SERVICENAME_MODULEMANAGER;
                        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > xModuleManager(
                            rServiceManager->createInstance(our_aModuleManagerName)
                            , ::com::sun::star::uno::UNO_QUERY_THROW);
                        m_aAppName = xModuleManager->identify(rModel);
                        m_hasAppName = true;
                    } catch(::com::sun::star::uno::Exception&) {}
                }
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aArgsWithOrigin(rArgs);
                ::comphelper::UiEventsLogger::appendDispatchOrigin(aArgsWithOrigin, m_aAppName, m_aWidgetName);
                ::comphelper::UiEventsLogger::logDispatch(rUrl, aArgsWithOrigin);
            }

        private:
            const ::rtl::OUString m_aWidgetName;
            bool m_hasAppName;
            ::rtl::OUString m_aAppName;
    };
}

#endif // __FRAMEWORK_DISPATCH_UIEVENTLOGHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
