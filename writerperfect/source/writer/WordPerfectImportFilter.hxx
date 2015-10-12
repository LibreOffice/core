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
    com::sun::star::document::XFilter,
    com::sun::star::document::XImporter,
    com::sun::star::document::XExtendedFilterDetection,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
    >
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mxDoc;
    OUString msFilterName;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > mxHandler;

    bool SAL_CALL importImpl(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > &aDescriptor)
    throw (css::uno::RuntimeException, std::exception);

public:
    explicit WordPerfectImportFilter(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext)
        : mxContext(rxContext) {}
    virtual ~WordPerfectImportFilter() {}

    // XFilter
    virtual sal_Bool SAL_CALL filter(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > &aDescriptor)
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > &xDoc)
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > &Descriptor)
    throw(com::sun::star::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &aArguments)
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString &ServiceName)
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

};

OUString WordPerfectImportFilter_getImplementationName()
throw (::com::sun::star::uno::RuntimeException);

bool SAL_CALL WordPerfectImportFilter_supportsService(const OUString &ServiceName)
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Sequence< OUString > SAL_CALL WordPerfectImportFilter_getSupportedServiceNames()
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL WordPerfectImportFilter_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rContext)
throw (::com::sun::star::uno::Exception);


class WordPerfectImportFilterDialog : public cppu::WeakImplHelper <
    com::sun::star::ui::dialogs::XExecutableDialog,
    com::sun::star::lang::XServiceInfo,
    com::sun::star::beans::XPropertyAccess
    >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;
    OUString msPassword;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > mxInputStream;

    virtual ~WordPerfectImportFilterDialog();

    // XExecutableDialog
    virtual void SAL_CALL setTitle(const OUString &aTitle)
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL execute()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString &ServiceName)
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
    SAL_CALL getPropertyValues() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL    setPropertyValues(const ::com::sun::star::uno::Sequence<
                                               ::com::sun::star::beans::PropertyValue >& aProps)
    throw (::com::sun::star::beans::UnknownPropertyException,
           ::com::sun::star::beans::PropertyVetoException,
           ::com::sun::star::lang::IllegalArgumentException,
           ::com::sun::star::lang::WrappedTargetException,
           ::com::sun::star::uno::RuntimeException, std::exception) override;

public:
    explicit WordPerfectImportFilterDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rContext);

};

OUString WordPerfectImportFilterDialog_getImplementationName()
throw (::com::sun::star::uno::RuntimeException);

bool SAL_CALL WordPerfectImportFilterDialog_supportsService(const OUString &ServiceName)
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Sequence< OUString > SAL_CALL WordPerfectImportFilterDialog_getSupportedServiceNames()
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL WordPerfectImportFilterDialog_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rContext)
throw (::com::sun::star::uno::Exception);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
