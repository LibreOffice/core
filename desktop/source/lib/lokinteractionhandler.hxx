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

#ifndef INCLUDED_DESKTOP_SOURCE_LIB_LOKINTERACTIONHANDLER_HXX
#define INCLUDED_DESKTOP_SOURCE_LIB_LOKINTERACTIONHANDLER_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

/** InteractionHandler is an interface that provides the user with various dialogs / error messages.

We need an own implementation for the LibreOfficeKit so that we can route the
information easily via callbacks.

TODO: the callbacks are not implemented yet, we just approve any interaction
that we get.
*/
class LOKInteractionHandler: public cppu::WeakImplHelper<com::sun::star::lang::XServiceInfo,
                                                         com::sun::star::lang::XInitialization,
                                                         com::sun::star::task::XInteractionHandler2>
{
    LOKInteractionHandler(const LOKInteractionHandler&) SAL_DELETED_FUNCTION;
    LOKInteractionHandler& operator=(const LOKInteractionHandler&) SAL_DELETED_FUNCTION;

public:
    explicit LOKInteractionHandler(com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext> const & rxContext);

    virtual ~LOKInteractionHandler();

    virtual OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL initialize(com::sun::star::uno::Sequence<com::sun::star::uno::Any > const & rArguments)
        throw (com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL handle(com::sun::star::uno::Reference<com::sun::star::task::XInteractionRequest> const & rRequest)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL handleInteractionRequest(const ::com::sun::star::uno::Reference<::com::sun::star::task::XInteractionRequest>& _Request)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
