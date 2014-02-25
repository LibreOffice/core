/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FILTER_SOURCE_HTMLFILTERDETECT_FILTERDETECT_HXX
#define INCLUDED_FILTER_SOURCE_HTMLFILTERDETECT_FILTERDETECT_HXX

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase3.hxx>

class HtmlFilterDetect : public cppu::WeakImplHelper3<
    com::sun::star::document::XExtendedFilterDetection,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo>
{
    com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext> mxCtx;

public:

    HtmlFilterDetect(const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& xCtx) :
        mxCtx(xCtx) {}
    virtual ~HtmlFilterDetect() {}

    // XExtendedFilterDetection

    virtual OUString SAL_CALL detect(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& lDescriptor)
        throw (com::sun::star::uno::RuntimeException, std::exception);

    // XInitialization

    virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArguments)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException, std::exception);

    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw (com::sun::star::uno::RuntimeException, std::exception);

    virtual com::sun::star::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (com::sun::star::uno::RuntimeException, std::exception);
};

OUString HtmlFilterDetect_getImplementationName();

com::sun::star::uno::Sequence<OUString> HtmlFilterDetect_getSupportedServiceNames();

com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
HtmlFilterDetect_createInstance(const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& rCtx);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
