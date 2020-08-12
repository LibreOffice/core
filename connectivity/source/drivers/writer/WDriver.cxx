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

#include <writer/WDriver.hxx>
#include <writer/WConnection.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbexception.hxx>
#include <resource/sharedresources.hxx>
#include <strings.hrc>

using namespace connectivity::file;
using namespace ::com::sun::star;

namespace connectivity::writer
{
OUString SAL_CALL ODriver::getImplementationName()
{
    return "com.sun.star.comp.sdbc.writer.ODriver";
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_writer_ODriver(css::uno::XComponentContext* context,
                            css::uno::Sequence<css::uno::Any> const& /*rArguments*/)
{
    rtl::Reference<ODriver> ret;
    try
    {
        ret = new ODriver(context);
    }
    catch (...)
    {
    }
    if (ret)
        ret->acquire();
    return static_cast<cppu::OWeakObject*>(ret.get());
}

uno::Reference<sdbc::XConnection>
    SAL_CALL ODriver::connect(const OUString& url, const uno::Sequence<beans::PropertyValue>& info)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (ODriver_BASE::rBHelper.bDisposed)
        throw lang::DisposedException();

    if (!acceptsURL(url))
        return nullptr;

    auto pCon = new OWriterConnection(this);
    pCon->construct(url, info);
    uno::Reference<sdbc::XConnection> xCon = pCon;
    m_xConnections.push_back(uno::WeakReferenceHelper(*pCon));

    return xCon;
}

sal_Bool SAL_CALL ODriver::acceptsURL(const OUString& url)
{
    return url.startsWith("sdbc:writer:");
}

uno::Sequence<sdbc::DriverPropertyInfo> SAL_CALL
ODriver::getPropertyInfo(const OUString& url, const uno::Sequence<beans::PropertyValue>& /*info*/)
{
    if (!acceptsURL(url))
    {
        SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage, *this);
    }
    return uno::Sequence<sdbc::DriverPropertyInfo>();
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
