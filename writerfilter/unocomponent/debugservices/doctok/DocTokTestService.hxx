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

#ifndef INCLUDED_DOCTOK_SCANNERTESTSERVICE_HXX
#define INCLUDED_DOCTOK_SCANNERTESTSERVICE_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace writerfilter { namespace doctoktest {

class ScannerTestService : public cppu::WeakImplHelper1 < ::com::sun::star::lang::XMain >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext;

public:
    ScannerTestService(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext);

    virtual ::sal_Int32 SAL_CALL run( const ::com::sun::star::uno::Sequence< OUString >& aArguments ) throw (::com::sun::star::uno::RuntimeException);

public:
    const static sal_Char SERVICE_NAME[40];
    const static sal_Char IMPLEMENTATION_NAME[40];

};

OUString ScannerTestService_getImplementationName ();
::com::sun::star::uno::Sequence< OUString > SAL_CALL ScannerTestService_getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ScannerTestService_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext) throw( ::com::sun::star::uno::Exception );

} } /* end namespace writerfilter::rtftok */

#define DOCTOK_SCANNERTESTSERVICE_COMPONENT_ENTRY \
{\
    writerfilter::doctoktest::ScannerTestService_createInstance,\
    writerfilter::doctoktest::ScannerTestService_getImplementationName,\
    writerfilter::doctoktest::ScannerTestService_getSupportedServiceNames,\
   ::cppu::createSingleComponentFactory,\
   0, 0\
}


#endif /* INCLUDED_RTFTOK_SCANNERTESTSERVICE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
