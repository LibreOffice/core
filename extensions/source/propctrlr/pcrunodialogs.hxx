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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRUNODIALOGS_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRUNODIALOGS_HXX

#include <svtools/genericunodialog.hxx>
#include "modulepcr.hxx"
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>


namespace pcr
{



    //= OTabOrderDialog

    class OTabOrderDialog;
    typedef ::svt::OGenericUnoDialog                                    OTabOrderDialog_DBase;
    typedef ::comphelper::OPropertyArrayUsageHelper< OTabOrderDialog >  OTabOrderDialog_PBase;

    class OTabOrderDialog
                :public OTabOrderDialog_DBase
                ,public OTabOrderDialog_PBase
                ,public PcrClient
    {
    protected:
        // <properties>
        css::uno::Reference< css::awt::XTabControllerModel >
                m_xTabbingModel;
        css::uno::Reference< css::awt::XControlContainer >
                m_xControlContext;
        // </properties>

    public:
        OTabOrderDialog( const css::uno::Reference< css::uno::XComponentContext >& _rxContext );
        virtual ~OTabOrderDialog();

        // XTypeProvider
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo - static methods
        static css::uno::Sequence< OUString > getSupportedServiceNames_static() throw( css::uno::RuntimeException );
        static OUString getImplementationName_static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface >
                SAL_CALL Create(const css::uno::Reference< css::uno::XComponentContext >&);

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

    protected:
    // OGenericUnoDialog overridables
        virtual VclPtr<Dialog> createDialog(vcl::Window* _pParent) SAL_OVERRIDE;
    };


} // namespacepcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRUNODIALOGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
