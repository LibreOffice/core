/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#ifndef _WORDPERFECTIMPORTFILTER_HXX
#define _WORDPERFECTIMPORTFILTER_HXX

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
protected:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::lang::XComponent > mxDoc;
    OUString msFilterName;
    css::uno::Reference< css::xml::sax::XDocumentHandler > mxHandler;

    bool SAL_CALL importImpl(const css::uno::Sequence< css::beans::PropertyValue > &aDescriptor)
    throw (css::uno::RuntimeException, std::exception);

public:
    explicit WordPerfectImportFilter(const css::uno::Reference< css::uno::XComponentContext > &rxContext)
        : mxContext(rxContext) {}
    virtual ~WordPerfectImportFilter() {}

    // XFilter
    virtual sal_Bool SAL_CALL filter(const css::uno::Sequence< css::beans::PropertyValue > &aDescriptor)
    throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel()
    throw (css::uno::RuntimeException, std::exception) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument(const css::uno::Reference< css::lang::XComponent > &xDoc)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect(css::uno::Sequence< css::beans::PropertyValue > &Descriptor)
    throw(css::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > &aArguments)
    throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
    throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString &ServiceName)
    throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception) override;

};

OUString WordPerfectImportFilter_getImplementationName()
throw (css::uno::RuntimeException);

bool SAL_CALL WordPerfectImportFilter_supportsService(const OUString &ServiceName)
throw (css::uno::RuntimeException);

css::uno::Sequence< OUString > SAL_CALL WordPerfectImportFilter_getSupportedServiceNames()
throw (css::uno::RuntimeException);

css::uno::Reference< css::uno::XInterface >
SAL_CALL WordPerfectImportFilter_createInstance(const css::uno::Reference< css::uno::XComponentContext > &rContext)
throw (css::uno::Exception);


class WordPerfectImportFilterDialog : public cppu::WeakImplHelper <
    css::ui::dialogs::XExecutableDialog,
    css::lang::XServiceInfo,
    css::beans::XPropertyAccess
    >
{
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    OUString msPassword;
    css::uno::Reference< css::io::XInputStream > mxInputStream;

    virtual ~WordPerfectImportFilterDialog();

    // XExecutableDialog
    virtual void SAL_CALL setTitle(const OUString &aTitle)
    throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL execute()
    throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
    throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString &ServiceName)
    throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception) override;

    // XPropertyAccess
    virtual css::uno::Sequence< css::beans::PropertyValue >
    SAL_CALL getPropertyValues() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL    setPropertyValues(const css::uno::Sequence<
                                               css::beans::PropertyValue >& aProps)
    throw (css::beans::UnknownPropertyException,
           css::beans::PropertyVetoException,
           css::lang::IllegalArgumentException,
           css::lang::WrappedTargetException,
           css::uno::RuntimeException, std::exception) override;

public:
    explicit WordPerfectImportFilterDialog(const css::uno::Reference< css::uno::XComponentContext > &rContext);

};

OUString WordPerfectImportFilterDialog_getImplementationName()
throw (css::uno::RuntimeException);

bool SAL_CALL WordPerfectImportFilterDialog_supportsService(const OUString &ServiceName)
throw (css::uno::RuntimeException);

css::uno::Sequence< OUString > SAL_CALL WordPerfectImportFilterDialog_getSupportedServiceNames()
throw (css::uno::RuntimeException);

css::uno::Reference< css::uno::XInterface >
SAL_CALL WordPerfectImportFilterDialog_createInstance(const css::uno::Reference< css::uno::XComponentContext > &rContext)
throw (css::uno::Exception);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
