/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_FILTER_SOURCE_VCLEVENT_VCLEVENTFILTER_HXX
#define INCLUDED_FILTER_SOURCE_VCLEVENT_VCLEVENTFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

class VclEventFilter : public cppu::WeakImplHelper
    < css::document::XFilter,
      css::document::XImporter,
      css::document::XExtendedFilterDetection,
      css::lang::XServiceInfo >
{
private:
    css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;
    css::uno::Reference< css::lang::XComponent > mxDoc;
    css::uno::Reference < css::io::XInputStream > mxInputStream;

    bool SAL_CALL importImpl( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
        throw (css::uno::RuntimeException);

    public:
        VclEventFilter(const css::uno::Reference<css::lang::XMultiServiceFactory > &r );
        virtual ~VclEventFilter();

    // XFilter
        virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL cancel(  )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE {};

    // XImporter
        virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XExtendedTypeDetection
        virtual OUString SAL_CALL detect(
            css::uno::Sequence< css::beans::PropertyValue >& Descriptor )
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
