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

#include "CNodes.hxx"

#include <boost/noncopyable.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/rdf/XLiteral.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <rtl/ustrbuf.hxx>


/// anonymous implementation namespace
namespace {

class CLiteral:
    public ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::rdf::XLiteral>,
    private boost::noncopyable
{
public:
    explicit CLiteral(css::uno::Reference< css::uno::XComponentContext > const & context);
    virtual ~CLiteral() {}

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString & ServiceName) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception, std::exception) override;

    // css::rdf::XNode:
    virtual OUString SAL_CALL getStringValue() throw (css::uno::RuntimeException, std::exception) override;

    // css::rdf::XLiteral:
    virtual OUString SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLanguage() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::rdf::XURI > SAL_CALL getDatatype() throw (css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    OUString m_Value;
    OUString m_Language;
    css::uno::Reference< css::rdf::XURI > m_xDatatype;
};

CLiteral::CLiteral(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context), m_Value(), m_Language(), m_xDatatype()
{}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL CLiteral::getImplementationName() throw (css::uno::RuntimeException, std::exception)
{
    return comp_CLiteral::_getImplementationName();
}

sal_Bool SAL_CALL CLiteral::supportsService(OUString const & serviceName) throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > SAL_CALL CLiteral::getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception)
{
    return comp_CLiteral::_getSupportedServiceNames();
}

// css::lang::XInitialization:
void SAL_CALL CLiteral::initialize(const css::uno::Sequence< css::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception, std::exception)
{
    const sal_Int32 len( aArguments.getLength() );
    if (len < 1 || len > 2) {
            throw css::lang::IllegalArgumentException(
                OUString("CLiteral::initialize: "
                    "must give 1 or 2 argument(s)"), *this, 2);
    }

    OUString arg0;
    if (!(aArguments[0] >>= arg0)) {
        throw css::lang::IllegalArgumentException(
            OUString("CLiteral::initialize: "
                "argument must be string"), *this, 0);
    }
    //FIXME: what is legal?
    if (true) {
        m_Value = arg0;
    } else {
        throw css::lang::IllegalArgumentException(
            OUString("CLiteral::initialize: "
                "argument is not valid literal value"), *this, 0);
    }

    if (len > 1) {
        OUString arg1;
        css::uno::Reference< css::rdf::XURI > xURI;
        if ((aArguments[1] >>= arg1)) {
            if (!arg1.isEmpty()) {
                m_Language = arg1;
            } else {
                throw css::lang::IllegalArgumentException(
                    OUString("CLiteral::initialize: "
                        "argument is not valid language"), *this, 1);
            }
        } else if ((aArguments[1] >>= xURI)) {
            if (xURI.is()) {
                m_xDatatype = xURI;
            } else {
                throw css::lang::IllegalArgumentException(
                    OUString("CLiteral::initialize: "
                        "argument is null"), *this, 1);
            }
        } else {
            throw css::lang::IllegalArgumentException(
                OUString("CLiteral::initialize: "
                    "argument must be string or URI"), *this, 1);
        }
    }
}

// css::rdf::XNode:
OUString SAL_CALL CLiteral::getStringValue() throw (css::uno::RuntimeException, std::exception)
{
    if (!m_Language.isEmpty()) {
        OUStringBuffer buf(m_Value);
        buf.append("@");
        buf.append(m_Language);
        return buf.makeStringAndClear();
    } else if (m_xDatatype.is()) {
        OUStringBuffer buf(m_Value);
        buf.append("^^");
        buf.append(m_xDatatype->getStringValue());
        return buf.makeStringAndClear();
    } else {
        return m_Value;
    }
}

// css::rdf::XLiteral:
OUString SAL_CALL CLiteral::getValue() throw (css::uno::RuntimeException, std::exception)
{
    return m_Value;
}

OUString SAL_CALL CLiteral::getLanguage() throw (css::uno::RuntimeException, std::exception)
{
    return m_Language;
}

css::uno::Reference< css::rdf::XURI > SAL_CALL CLiteral::getDatatype() throw (css::uno::RuntimeException, std::exception)
{
    return m_xDatatype;
}

} // closing anonymous implementation namespace



// component helper namespace
namespace comp_CLiteral {

OUString SAL_CALL _getImplementationName() {
    return OUString( "CLiteral");
}

css::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames()
{
    css::uno::Sequence< OUString > s { "com.sun.star.rdf.Literal" };
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
{
    return static_cast< ::cppu::OWeakObject * >(new CLiteral(context));
}

} // closing component helper namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
