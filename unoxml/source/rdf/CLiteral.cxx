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
#include <com/sun/star/rdf/XLiteral.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <rtl/ustrbuf.hxx>


/// anonymous implementation namespace
namespace {

namespace css = ::com::sun::star;

class CLiteral:
    public ::cppu::WeakImplHelper3<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::rdf::XLiteral>
{
public:
    explicit CLiteral(css::uno::Reference< css::uno::XComponentContext > const & context);
    virtual ~CLiteral() {}

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception);

    // ::com::sun::star::rdf::XNode:
    virtual ::rtl::OUString SAL_CALL getStringValue() throw (css::uno::RuntimeException);

    // ::com::sun::star::rdf::XLiteral:
    virtual ::rtl::OUString SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLanguage() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::rdf::XURI > SAL_CALL getDatatype() throw (css::uno::RuntimeException);

private:
    CLiteral(const CLiteral &); // not defined
    CLiteral& operator=(const CLiteral &); // not defined

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    ::rtl::OUString m_Value;
    ::rtl::OUString m_Language;
    css::uno::Reference< css::rdf::XURI > m_xDatatype;
};

CLiteral::CLiteral(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context), m_Value(), m_Language(), m_xDatatype()
{}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL CLiteral::getImplementationName() throw (css::uno::RuntimeException)
{
    return comp_CLiteral::_getImplementationName();
}

::sal_Bool SAL_CALL CLiteral::supportsService(::rtl::OUString const & serviceName) throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > serviceNames = comp_CLiteral::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL CLiteral::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    return comp_CLiteral::_getSupportedServiceNames();
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL CLiteral::initialize(const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception)
{
    const sal_Int32 len( aArguments.getLength() );
    if (len < 1 || len > 2) {
            throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("CLiteral::initialize: "
                    "must give 1 or 2 argument(s)"), *this, 2);
    }

    ::rtl::OUString arg0;
    if (!(aArguments[0] >>= arg0)) {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString::createFromAscii("CLiteral::initialize: "
                "argument must be string"), *this, 0);
    }
    //FIXME: what is legal?
    if (true) {
        m_Value = arg0;
    } else {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString::createFromAscii("CLiteral::initialize: "
                "argument is not valid literal value"), *this, 0);
    }

    if (len > 1) {
        ::rtl::OUString arg1;
        css::uno::Reference< css::rdf::XURI > xURI;
        if ((aArguments[1] >>= arg1)) {
            if (arg1.getLength() > 0) {
                m_Language = arg1;
            } else {
                throw css::lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii("CLiteral::initialize: "
                        "argument is not valid language"), *this, 1);
            }
        } else if ((aArguments[1] >>= xURI)) {
            if (xURI.is()) {
                m_xDatatype = xURI;
            } else {
                throw css::lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii("CLiteral::initialize: "
                        "argument is null"), *this, 1);
            }
        } else {
            throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("CLiteral::initialize: "
                    "argument must be string or URI"), *this, 1);
        }
    }
}

// ::com::sun::star::rdf::XNode:
::rtl::OUString SAL_CALL CLiteral::getStringValue() throw (css::uno::RuntimeException)
{
    if (!m_Language.equalsAscii("")) {
        ::rtl::OUStringBuffer buf(m_Value);
        buf.appendAscii("@");
        buf.append(m_Language);
        return buf.makeStringAndClear();
    } else if (m_xDatatype.is()) {
        ::rtl::OUStringBuffer buf(m_Value);
        buf.appendAscii("^^");
        buf.append(m_xDatatype->getStringValue());
        return buf.makeStringAndClear();
    } else {
        return m_Value;
    }
}

// ::com::sun::star::rdf::XLiteral:
::rtl::OUString SAL_CALL CLiteral::getValue() throw (css::uno::RuntimeException)
{
    return m_Value;
}

::rtl::OUString SAL_CALL CLiteral::getLanguage() throw (css::uno::RuntimeException)
{
    return m_Language;
}

css::uno::Reference< css::rdf::XURI > SAL_CALL CLiteral::getDatatype() throw (css::uno::RuntimeException)
{
    return m_xDatatype;
}

} // closing anonymous implementation namespace



// component helper namespace
namespace comp_CLiteral {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "CLiteral"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.rdf.Literal"));
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new CLiteral(context));
}

} // closing component helper namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
