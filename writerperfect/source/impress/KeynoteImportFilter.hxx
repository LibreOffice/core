/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_IMPRESS_KEYNOTEIMPORTFILTER_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_IMPRESS_KEYNOTEIMPORTFILTER_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "writerperfect/ImportFilter.hxx"
#include "DocumentHandlerForOdp.hxx"

#include <stdio.h>

/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */
class KeynoteImportFilter : public writerperfect::ImportFilter<OdpGenerator>
{
public:
    KeynoteImportFilter(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext)
        : writerperfect::ImportFilter<OdpGenerator>(rxContext) {}
    virtual ~KeynoteImportFilter() {}

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect(com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > &Descriptor)
    throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString &ServiceName)
    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    virtual bool doDetectFormat(librevenge::RVNGInputStream &rInput, OUString &rTypeName) SAL_OVERRIDE;
    virtual bool doImportDocument(librevenge::RVNGInputStream &rInput, OdpGenerator &rGenerator, utl::MediaDescriptor &) SAL_OVERRIDE;
};

OUString KeynoteImportFilter_getImplementationName()
throw (::com::sun::star::uno::RuntimeException);

bool SAL_CALL KeynoteImportFilter_supportsService(const OUString &ServiceName)
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Sequence< OUString > SAL_CALL KeynoteImportFilter_getSupportedServiceNames()
throw (::com::sun::star::uno::RuntimeException);

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL KeynoteImportFilter_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rContext)
throw (::com::sun::star::uno::Exception);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
