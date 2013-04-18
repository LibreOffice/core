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

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#define _COMPHELPER_PROCESSFACTORY_HXX_

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include "comphelper/comphelperdllapi.h"

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace comphelper
{

/**
 * This function set the process service factory.
 *
 * @author Juergen Schmidt
 */
COMPHELPER_DLLPUBLIC void setProcessServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMgr);

/**
 * This function gets the process service factory.
 *
 * If no service factory is set the function throws a RuntimeException.
 *
 * @author Juergen Schmidt
 */
COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getProcessServiceFactory();

/** Obtains a component context from a service factory.

    Throws a RuntimeException if no component context can be obtained.

    @param factory may be null
    @return may be null
 */
COMPHELPER_DLLPUBLIC
com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
getComponentContext(
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
        const & factory);

/**
 * This function gets the process service factory's default component context.
 *
 * Throws a RuntimeException if no component context can be obtained.
 */
COMPHELPER_DLLPUBLIC
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
getProcessComponentContext();

}


extern "C" {
/// @internal ATTENTION returns ACQUIRED pointer! release it explicitly!
COMPHELPER_DLLPUBLIC
::com::sun::star::uno::XComponentContext *
comphelper_getProcessComponentContext();
} // extern "C"

#endif // _COMPHELPER_PROCESSFACTORY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
