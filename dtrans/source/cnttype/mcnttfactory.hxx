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

#ifndef INCLUDED_DTRANS_SOURCE_CNTTYPE_MCNTTFACTORY_HXX
#define INCLUDED_DTRANS_SOURCE_CNTTYPE_MCNTTFACTORY_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>

class CMimeContentTypeFactory : public
    cppu::WeakImplHelper< css::datatransfer::XMimeContentTypeFactory,
                          css::lang::XServiceInfo >
{

public:
    CMimeContentTypeFactory();

    // XMimeContentTypeFactory

    virtual css::uno::Reference< css::datatransfer::XMimeContentType > SAL_CALL createMimeContentType( const OUString& aContentType )
        throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName(  )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(css::uno::RuntimeException, std::exception) override;

private:
    ::osl::Mutex                                                                           m_aMutex;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
