/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#ifndef _CDRIMPORTFILTER_HXX
#define _CDRIMPORTFILTER_HXX

#include "ImportFilter.hxx"

#include "DocumentHandlerForOdg.hxx"

/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */
class CDRImportFilter : public writerperfect::ImportFilter<OdgGenerator>
{
public:
    explicit CDRImportFilter(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext)
        : writerperfect::ImportFilter<OdgGenerator>(rxContext)
    {
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString &ServiceName)
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    virtual bool doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName) override;
    virtual bool doImportDocument(librevenge::RVNGInputStream &rInput, OdgGenerator &rGenerator, utl::MediaDescriptor &) override;
};

OUString CDRImportFilter_getImplementationName()
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Sequence< OUString > SAL_CALL CDRImportFilter_getSupportedServiceNames()
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL CDRImportFilter_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rContext)
throw (::com::sun::star::uno::Exception);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
