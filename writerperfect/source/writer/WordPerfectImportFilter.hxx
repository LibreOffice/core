/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_WORDPERFECTIMPORTFILTER_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_WORDPERFECTIMPORTFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase.hxx>

/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */
class WordPerfectImportFilter : public cppu::WeakImplHelper
    <
    css::document::XFilter,
    css::document::XImporter,
    css::document::XExtendedFilterDetection,
    css::lang::XInitialization,
    css::lang::XServiceInfo
    >
{
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::lang::XComponent > mxDoc;

    /// @throws css::uno::RuntimeException
    bool SAL_CALL importImpl(const css::uno::Sequence< css::beans::PropertyValue > &aDescriptor);

public:
    explicit WordPerfectImportFilter(const css::uno::Reference< css::uno::XComponentContext > &rxContext)
        : mxContext(rxContext) {}

    // XFilter
    virtual sal_Bool SAL_CALL filter(const css::uno::Sequence< css::beans::PropertyValue > &aDescriptor) override;
    virtual void SAL_CALL cancel() override;

    // XImporter
    virtual void SAL_CALL setTargetDocument(const css::uno::Reference< css::lang::XComponent > &xDoc) override;

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect(css::uno::Sequence< css::beans::PropertyValue > &Descriptor) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > &aArguments) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString &ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
