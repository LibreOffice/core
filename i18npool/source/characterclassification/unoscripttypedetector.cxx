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

#include <unoscripttypedetector.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <i18nutil/scripttypedetector.hxx>

namespace com::sun::star::uno { class XComponentContext; }

sal_Int16 SAL_CALL
UnoScriptTypeDetector::getScriptDirection( const OUString& Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection )
{
    return ScriptTypeDetector::getScriptDirection(Text, nPos, defaultScriptDirection);
}

// return value '-1' means either the direction on nPos is not same as scriptDirection or nPos is out of range.
sal_Int32 SAL_CALL
UnoScriptTypeDetector::beginOfScriptDirection( const OUString& Text, sal_Int32 nPos, sal_Int16 direction )
{
    return ScriptTypeDetector::beginOfScriptDirection(Text, nPos, direction);
}

sal_Int32 SAL_CALL
UnoScriptTypeDetector::endOfScriptDirection( const OUString& Text, sal_Int32 nPos, sal_Int16 direction )
{
    return ScriptTypeDetector::endOfScriptDirection(Text, nPos, direction);
}

sal_Int16 SAL_CALL
UnoScriptTypeDetector::getCTLScriptType( const OUString&, sal_Int32 )
{
    throw css::uno::RuntimeException("not implemented");
}

sal_Int32 SAL_CALL
UnoScriptTypeDetector::beginOfCTLScriptType( const OUString&, sal_Int32 )
{
    throw css::uno::RuntimeException("not implemented");
}

sal_Int32 SAL_CALL
UnoScriptTypeDetector::endOfCTLScriptType( const OUString&, sal_Int32 )
{
    throw css::uno::RuntimeException("not implemented");
}

OUString SAL_CALL
UnoScriptTypeDetector::getImplementationName()
{
    return u"com.sun.star.i18n.ScriptTypeDetector"_ustr;
}

sal_Bool SAL_CALL
UnoScriptTypeDetector::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL
UnoScriptTypeDetector::getSupportedServiceNames()
{
    return { u"com.sun.star.i18n.ScriptTypeDetector"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_i18n_ScriptTypeDetector_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoScriptTypeDetector);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
