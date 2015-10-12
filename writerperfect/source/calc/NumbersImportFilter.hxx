/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_NUMBERSIMPORTFILTER_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_NUMBERSIMPORTFILTER_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "DocumentHandlerForOds.hxx"
#include "ImportFilter.hxx"

/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */
class NumbersImportFilter : public writerperfect::ImportFilter<OdsGenerator>
{
public:
    explicit NumbersImportFilter(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext)
        : writerperfect::ImportFilter<OdsGenerator>(rxContext) {}

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString &ServiceName)
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    virtual bool doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName) override;
    virtual bool doImportDocument(librevenge::RVNGInputStream &rInput, OdsGenerator &rGenerator, utl::MediaDescriptor &) override;
    virtual void doRegisterHandlers(OdsGenerator &rGenerator) override;
};

OUString NumbersImportFilter_getImplementationName()
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Sequence< OUString > SAL_CALL NumbersImportFilter_getSupportedServiceNames()
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL NumbersImportFilter_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rContext)
throw (::com::sun::star::uno::Exception);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
