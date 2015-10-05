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

#ifndef INCLUDED_FILTER_SOURCE_SVG_SVGDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_SVG_SVGDIALOG_HXX

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <svtools/genericunodialog.hxx>
#include <memory>


// - SVGDialog -


class ResMgr;

class SVGDialog : public ::svt::OGenericUnoDialog,
                  public ::comphelper::OPropertyArrayUsageHelper< SVGDialog >,
                  public css::beans::XPropertyAccess,
                  public css::document::XExporter
{
private:

    ::std::unique_ptr< ResMgr >                     mapResMgr;
    css::uno::Sequence< css::beans::PropertyValue > maMediaDescriptor;
    css::uno::Sequence< css::beans::PropertyValue > maFilterData;
    css::uno::Reference< css::lang::XComponent >    mxSrcDoc;

protected:

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE;
    virtual void SAL_CALL release() throw () SAL_OVERRIDE;

    // OGenericUnoDialog
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual VclPtr<Dialog> createDialog( vcl::Window* pParent ) SAL_OVERRIDE;
    virtual void executedDialog( sal_Int16 nExecutionResult ) SAL_OVERRIDE;
    virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

    // XPropertyAccess
    using cppu::OPropertySetHelper::getPropertyValues;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPropertyValues(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    using cppu::OPropertySetHelper::setPropertyValues;
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& aProps ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

       // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public:

    explicit SVGDialog( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual     ~SVGDialog();
};



OUString SVGDialog_getImplementationName ()
    throw ( css::uno::RuntimeException );



bool SAL_CALL SVGDialog_supportsService( const OUString& ServiceName )
    throw ( css::uno::RuntimeException );



css::uno::Sequence< OUString > SAL_CALL SVGDialog_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );



css::uno::Reference< css::uno::XInterface >
    SAL_CALL SVGDialog_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)
        throw ( css::uno::Exception );

#endif // SVGDialog_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
