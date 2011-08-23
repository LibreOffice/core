/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "CNodes.hxx"

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/rdf/XBlankNode.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>


/// anonymous implementation namespace
namespace {

namespace css = ::com::sun::star;

class CBlankNode:
    public ::cppu::WeakImplHelper3<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::rdf::XBlankNode>
{
public:
    explicit CBlankNode(css::uno::Reference< css::uno::XComponentContext > const & context);
    virtual ~CBlankNode() {}

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception);

    // ::com::sun::star::rdf::XNode:
    virtual ::rtl::OUString SAL_CALL getStringValue() throw (css::uno::RuntimeException);

private:
    CBlankNode(const CBlankNode &); // not defined
    CBlankNode& operator=(const CBlankNode &); // not defined

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    ::rtl::OUString m_NodeID;
};

CBlankNode::CBlankNode(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context), m_NodeID()
{}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL CBlankNode::getImplementationName() throw (css::uno::RuntimeException)
{
    return comp_CBlankNode::_getImplementationName();
}

::sal_Bool SAL_CALL CBlankNode::supportsService(::rtl::OUString const & serviceName) throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > serviceNames = comp_CBlankNode::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL CBlankNode::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    return comp_CBlankNode::_getSupportedServiceNames();
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL CBlankNode::initialize(const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception)
{
    if (aArguments.getLength() != 1) {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString::createFromAscii("CBlankNode::initialize: "
                "must give exactly 1 argument"), *this, 1);
    }

    ::rtl::OUString arg;
    if (!(aArguments[0] >>= arg)) {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString::createFromAscii("CBlankNode::initialize: "
                "argument must be string"), *this, 0);
    }

    //FIXME: what is legal?
    if (arg.getLength() > 0) {
        m_NodeID = arg;
    } else {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString::createFromAscii("CBlankNode::initialize: "
                "argument is not valid blank node ID"), *this, 0);
    }
}

// ::com::sun::star::rdf::XNode:
::rtl::OUString SAL_CALL CBlankNode::getStringValue() throw (css::uno::RuntimeException)
{
    return m_NodeID;
}

} // closing anonymous implementation namespace



// component helper namespace
namespace comp_CBlankNode {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "CBlankNode"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.rdf.BlankNode"));
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new CBlankNode(context));
}

} // closing component helper namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
