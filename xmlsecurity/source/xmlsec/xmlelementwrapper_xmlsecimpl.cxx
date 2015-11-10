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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>

namespace cssu = com::sun::star::uno;

#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.XMLElementWrapper_XmlSecImpl"

XMLElementWrapper_XmlSecImpl::XMLElementWrapper_XmlSecImpl(const xmlNodePtr pNode)
    : m_pElement( pNode )
{
}

/* XXMLElementWrapper */


/* XUnoTunnel */
cssu::Sequence< sal_Int8 > XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId()
    throw (cssu::RuntimeException)
{
    static ::cppu::OImplementationId* pId = nullptr;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

sal_Int64 SAL_CALL XMLElementWrapper_XmlSecImpl::getSomething( const cssu::Sequence< sal_Int8 >& aIdentifier )
    throw (cssu::RuntimeException, std::exception)
{
    if (aIdentifier.getLength() == 16 &&
        0 == memcmp(
            getUnoTunnelImplementationId().getConstArray(),
            aIdentifier.getConstArray(),
            16 ))
    {
        return reinterpret_cast < sal_Int64 > ( this );
    }
    else
    {
        return 0;
    }
}


OUString XMLElementWrapper_XmlSecImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{
    return OUString ( IMPLEMENTATION_NAME );
}

cssu::Sequence< OUString > SAL_CALL XMLElementWrapper_XmlSecImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.xml.wrapper.XMLElementWrapper";
    return aRet;
}

cssu::Reference< cssu::XInterface > SAL_CALL
    XMLElementWrapper_XmlSecImpl_createInstance(
        const cssu::Reference< cssu::XComponentContext > &)
    throw( cssu::Exception )
{
    return static_cast<cppu::OWeakObject*>(new XMLElementWrapper_XmlSecImpl(nullptr));
}

/* XServiceInfo */
OUString SAL_CALL XMLElementWrapper_XmlSecImpl::getImplementationName(  )
    throw (cssu::RuntimeException, std::exception)
{
    return XMLElementWrapper_XmlSecImpl_getImplementationName();
}
sal_Bool SAL_CALL XMLElementWrapper_XmlSecImpl::supportsService( const OUString& rServiceName )
    throw (cssu::RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}
cssu::Sequence< OUString > SAL_CALL XMLElementWrapper_XmlSecImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException, std::exception)
{
    return XMLElementWrapper_XmlSecImpl_getSupportedServiceNames();
}


void XMLElementWrapper_XmlSecImpl::setNativeElement(const xmlNodePtr pNode)
/****** XMLElementWrapper_XmlSecImpl/setNativeElement *************************
 *
 *   NAME
 *  setNativeElement -- Configures the libxml2 node wrapped by this object
 *
 *   SYNOPSIS
 *  setNativeElement( pNode );
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pNode - the new libxml2 node to be wrapped by this object
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    m_pElement = pNode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
