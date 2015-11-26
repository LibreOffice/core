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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_XMLELEMENTWRAPPER_XMLSECIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_XMLELEMENTWRAPPER_XMLSECIMPL_HXX

#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include <libxml/tree.h>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

class XMLElementWrapper_XmlSecImpl : public cppu::WeakImplHelper
<
    css::xml::wrapper::XXMLElementWrapper,
    css::lang::XUnoTunnel,
    css::lang::XServiceInfo
>
/****** XMLElementWrapper_XmlSecImpl.hxx/CLASS XMLElementWrapper_XmlSecImpl ***
 *
 *   NAME
 *  XMLElementWrapper_XmlSecImpl -- Class to wrap a libxml2 node
 *
 *   FUNCTION
 *  Used as a wrapper class to transfer a libxml2 node structure
 *  between different UNO components.
 ******************************************************************************/
{
private:
    /* the libxml2 node wrapped by this object */
    xmlNodePtr m_pElement;

public:
    explicit XMLElementWrapper_XmlSecImpl(const xmlNodePtr pNode);
    virtual ~XMLElementWrapper_XmlSecImpl() {};

    /* XXMLElementWrapper */

    /* css::lang::XUnoTunnel */
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
        throw (css::uno::RuntimeException, std::exception) override;
    static css::uno::Sequence < sal_Int8 > getUnoTunnelImplementationId()
        throw(css::uno::RuntimeException);

    /* css::lang::XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;

public:
    /*
    *   NAME
    *  getNativeElement -- Retrieves the libxml2 node wrapped by this object
    *
    *   SYNOPSIS
    *  pNode = getNativeElement();
    *
    *   RESULT
    *  pNode - the libxml2 node wrapped by this object
    */
    xmlNodePtr getNativeElement( ) const { return m_pElement;}
    void setNativeElement(const xmlNodePtr pNode);
};

OUString XMLElementWrapper_XmlSecImpl_getImplementationName()
    throw ( css::uno::RuntimeException );

css::uno::Sequence< OUString > SAL_CALL XMLElementWrapper_XmlSecImpl_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL XMLElementWrapper_XmlSecImpl_createInstance(
    const css::uno::Reference< css::uno::XComponentContext > &)
    throw ( css::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
