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

#include <services/uriabbreviation.hxx>

#include <sal/config.h>

#include <tools/urlobj.hxx>
#include <cppuhelper/supportsservice.hxx>

// framework namespace
namespace framework
{

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL UriAbbreviation::getImplementationName()
{
    return u"com.sun.star.comp.framework.UriAbbreviation"_ustr;
}

sal_Bool SAL_CALL UriAbbreviation::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL UriAbbreviation::getSupportedServiceNames()
{
    return { u"com.sun.star.util.UriAbbreviation"_ustr };
}

UriAbbreviation::UriAbbreviation(css::uno::Reference< css::uno::XComponentContext > const & )
{
}

// css::util::XStringAbbreviation:
OUString SAL_CALL UriAbbreviation::abbreviateString(const css::uno::Reference< css::util::XStringWidth > & xStringWidth, ::sal_Int32 nWidth, const OUString & aString)
{
    OUString aResult( aString );
    if ( xStringWidth.is() )
    {
        // Use INetURLObject to abbreviate URLs
        INetURLObject aURL( aString );
        aResult = aURL.getAbbreviated( xStringWidth, nWidth, INetURLObject::DecodeMechanism::Unambiguous );
    }

    return aResult;
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_UriAbbreviation_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::UriAbbreviation(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
