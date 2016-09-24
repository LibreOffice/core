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

#include <com/sun/star/uno/XComponentContext.hpp>
#include "unoscripttypedetector.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <i18nutil/scripttypedetector.hxx>

//      class UnoScriptTypeDetector
sal_Int16 SAL_CALL
UnoScriptTypeDetector::getScriptDirection( const OUString& Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection ) throw (css::uno::RuntimeException, std::exception)
{
    return ScriptTypeDetector::getScriptDirection(Text, nPos, defaultScriptDirection);
}

// return value '-1' means either the direction on nPos is not same as scriptDirection or nPos is out of range.
sal_Int32 SAL_CALL
UnoScriptTypeDetector::beginOfScriptDirection( const OUString& Text, sal_Int32 nPos, sal_Int16 direction ) throw (css::uno::RuntimeException, std::exception)
{
    return ScriptTypeDetector::beginOfScriptDirection(Text, nPos, direction);
}

sal_Int32 SAL_CALL
UnoScriptTypeDetector::endOfScriptDirection( const OUString& Text, sal_Int32 nPos, sal_Int16 direction ) throw (css::uno::RuntimeException, std::exception)
{
    return ScriptTypeDetector::endOfScriptDirection(Text, nPos, direction);
}

sal_Int16 SAL_CALL
UnoScriptTypeDetector::getCTLScriptType( const OUString& Text, sal_Int32 nPos ) throw (css::uno::RuntimeException, std::exception)
{
    return ScriptTypeDetector::getCTLScriptType(Text, nPos);
}

// Begin of Script Type is inclusive.
sal_Int32 SAL_CALL
UnoScriptTypeDetector::beginOfCTLScriptType( const OUString& Text, sal_Int32 nPos ) throw (css::uno::RuntimeException, std::exception)
{
    return ScriptTypeDetector::beginOfCTLScriptType(Text, nPos);
}

// End of the Script Type is exclusive, the return value pointing to the begin of next script type
sal_Int32 SAL_CALL
UnoScriptTypeDetector::endOfCTLScriptType( const OUString& Text, sal_Int32 nPos ) throw (css::uno::RuntimeException, std::exception)
{
    return ScriptTypeDetector::endOfCTLScriptType(Text, nPos);
}

OUString SAL_CALL
UnoScriptTypeDetector::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.i18n.ScriptTypeDetector");
}

sal_Bool SAL_CALL
UnoScriptTypeDetector::supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL
UnoScriptTypeDetector::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence< OUString > aRet { "com.sun.star.i18n.ScriptTypeDetector" };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_i18n_ScriptTypeDetector_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoScriptTypeDetector());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
