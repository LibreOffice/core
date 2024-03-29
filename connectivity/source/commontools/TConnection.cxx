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

#include <TConnection.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>

using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace ::osl;


OMetaConnection::OMetaConnection()
    : OMetaConnection_BASE(m_aMutex)
    , m_nTextEncoding(RTL_TEXTENCODING_MS_1252)
{
}

void OMetaConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xMetaData = WeakReference< XDatabaseMetaData>();
    for (auto const& statement : m_aStatements)
    {
        try
        {
            Reference< XInterface > xStatement( statement.get() );
            ::comphelper::disposeComponent( xStatement );
        }
        catch (const DisposedException&)
        {
        }
    }
    m_aStatements.clear();
}
//XUnoTunnel
sal_Int64 SAL_CALL OMetaConnection::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
    return comphelper::getSomethingImpl(rId, this);
}

const Sequence< sal_Int8 > & OMetaConnection::getUnoTunnelId()
{
    static const comphelper::UnoIdInit implId;
    return implId.getSeq();
}

::dbtools::OPropertyMap& OMetaConnection::getPropMap()
{
    static ::dbtools::OPropertyMap s_aPropertyNameMap;
    return s_aPropertyNameMap;
}

void OMetaConnection::throwGenericSQLException(TranslateId pErrorResourceId, const Reference< XInterface>& _xContext )
{
    OUString sErrorMessage;
    if (pErrorResourceId)
        sErrorMessage = m_aResources.getResourceString(pErrorResourceId);
    Reference< XInterface> xContext = _xContext;
    if ( !xContext.is() )
        xContext = *this;
    ::dbtools::throwGenericSQLException( sErrorMessage, xContext);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
