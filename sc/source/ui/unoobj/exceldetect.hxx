/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_EXCELDETECT_HXX__
#define __SC_EXCELDETECT_HXX__

#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
}}}}

class ScExcelBiffDetect : public cppu::WeakImplHelper2<com::sun::star::document::XExtendedFilterDetection, com::sun::star::lang::XServiceInfo>
{
public:
    ScExcelBiffDetect( const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& xContext );
    virtual ~ScExcelBiffDetect();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& aName ) throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

    // XExtendedFilterDetection
    virtual OUString SAL_CALL detect( com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& lDescriptor )
        throw (com::sun::star::uno::RuntimeException);

    static com::sun::star::uno::Sequence<OUString> impl_getStaticSupportedServiceNames();
    static OUString impl_getStaticImplementationName();
    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
        impl_createInstance( const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& xContext )
            throw (com::sun::star::uno::Exception);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
