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

#ifndef INCLUDED_EXAMPLES_CUSTOMPANEL_CTP_FACTORY_HXX
#define INCLUDED_EXAMPLES_CUSTOMPANEL_CTP_FACTORY_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/basemutex.hxx>


namespace sd { namespace colortoolpanel
{

    typedef ::cppu::WeakImplHelper  <   ::com::sun::star::ui::XUIElementFactory
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    >   ToolPanelFactory_Base;
    class ToolPanelFactory   :public ::cppu::BaseMutex
                            ,public ToolPanelFactory_Base
    {
    public:
        ToolPanelFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext
        );
        ~ToolPanelFactory();

        // XUIElementFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > SAL_CALL createUIElement( const OUString& i_rResourceURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& i_rArgs ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static versions
        static OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext ) throw (::com::sun::star::uno::RuntimeException);

    private:
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                m_xContext;
    };


} } // namespace sd::colortoolpanel


#endif // INCLUDED_EXAMPLES_CUSTOMPANEL_CTP_FACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
