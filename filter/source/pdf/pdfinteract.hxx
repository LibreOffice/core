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

#ifndef INCLUDED_FILTER_SOURCE_PDF_PDFINTERACT_HXX
#define INCLUDED_FILTER_SOURCE_PDF_PDFINTERACT_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


class PDFInteractionHandler : public cppu::WeakImplHelper < task::XInteractionHandler2,
                                                             XServiceInfo >
{
protected:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XIniteractionHandler
    virtual void SAL_CALL handle( const Reference< task::XInteractionRequest >& ) override;

    // XIniteractionHandler2
    virtual sal_Bool SAL_CALL handleInteractionRequest( const Reference< task::XInteractionRequest >& ) override;

public:

                PDFInteractionHandler();
    virtual     ~PDFInteractionHandler() override;
};

/// @throws RuntimeException
OUString PDFInteractionHandler_getImplementationName ();

/// @throws RuntimeException
Sequence< OUString > PDFInteractionHandler_getSupportedServiceNames(  );

/// @throws Exception
Reference< XInterface >
PDFInteractionHandler_createInstance( const Reference< XMultiServiceFactory > & rSMgr);

#endif // INCLUDED_FILTER_SOURCE_PDF_PDFINTERACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
