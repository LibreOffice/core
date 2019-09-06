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

#include <string.h>

#include "xmlelementwrapper_xmlsecimpl.hxx"
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace com::sun::star;

XMLElementWrapper_XmlSecImpl::XMLElementWrapper_XmlSecImpl(const xmlNodePtr pNode)
    : m_pElement( pNode )
{
}

/* XUnoTunnel */
uno::Sequence< sal_Int8 > XMLElementWrapper_XmlSecImpl::getUnoTunnelId()
{
    static ::cppu::OImplementationId implId;

    return implId.getImplementationId();
}

sal_Int64 SAL_CALL XMLElementWrapper_XmlSecImpl::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
{
    if (isUnoTunnelId<XMLElementWrapper_XmlSecImpl>(aIdentifier))
    {
        return reinterpret_cast < sal_Int64 > ( this );
    }

    return 0;
}

/* XServiceInfo */
OUString SAL_CALL XMLElementWrapper_XmlSecImpl::getImplementationName(  )
{
    return "com.sun.star.xml.wrapper.XMLElementWrapper";
}

sal_Bool SAL_CALL XMLElementWrapper_XmlSecImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

uno::Sequence< OUString > SAL_CALL XMLElementWrapper_XmlSecImpl::getSupportedServiceNames(  )
{
    uno::Sequence<OUString> aRet { "com.sun.star.xml.wrapper.XMLElementWrapper" };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_wrapper_XMLElementWrapper_get_implementation(
    uno::XComponentContext* /*pCtx*/, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new XMLElementWrapper_XmlSecImpl(nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
