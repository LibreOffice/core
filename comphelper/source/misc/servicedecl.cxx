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


#include <comphelper/servicedecl.hxx>
#include <rtl/string.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <cassert>
#include <vector>

using namespace com::sun::star;

namespace comphelper::service_decl {

const char cDelim = ';';

class ServiceDecl::Factory :
        public cppu::WeakImplHelper<lang::XSingleComponentFactory,
                                     lang::XServiceInfo>
{
public:
    explicit Factory( ServiceDecl const& rServiceDecl )
        : m_rServiceDecl(rServiceDecl) {}
    // noncopyable
    Factory(const Factory&) = delete;
    const Factory& operator=(const Factory&) = delete;

    // XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( OUString const& name ) override;
    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
    // XSingleComponentFactory:
    virtual uno::Reference<uno::XInterface> SAL_CALL createInstanceWithContext(
        uno::Reference<uno::XComponentContext> const& xContext ) override;
    virtual uno::Reference<uno::XInterface> SAL_CALL
    createInstanceWithArgumentsAndContext(
    uno::Sequence<uno::Any> const& args,
    uno::Reference<uno::XComponentContext> const& xContext ) override;

private:
    virtual ~Factory() override;

    ServiceDecl const& m_rServiceDecl;
};

ServiceDecl::Factory::~Factory()
{
}

// XServiceInfo:
OUString ServiceDecl::Factory::getImplementationName()
{
    return m_rServiceDecl.getImplementationName();
}

sal_Bool ServiceDecl::Factory::supportsService( OUString const& name )
{
    return m_rServiceDecl.supportsService(name);
}

uno::Sequence<OUString> ServiceDecl::Factory::getSupportedServiceNames()
{
    return m_rServiceDecl.getSupportedServiceNames();
}

// XSingleComponentFactory:
uno::Reference<uno::XInterface> ServiceDecl::Factory::createInstanceWithContext(
    uno::Reference<uno::XComponentContext> const& xContext )
{
    return m_rServiceDecl.m_createFunc(
        m_rServiceDecl, uno::Sequence<uno::Any>(), xContext );
}

uno::Reference<uno::XInterface>
ServiceDecl::Factory::createInstanceWithArgumentsAndContext(
    uno::Sequence<uno::Any > const& args,
    uno::Reference<uno::XComponentContext> const& xContext )
{
    return m_rServiceDecl.m_createFunc(
        m_rServiceDecl, args, xContext );
}

void * ServiceDecl::getFactory( char const* pImplName ) const
{
    if (rtl_str_compare(m_pImplName, pImplName) == 0) {
        lang::XSingleComponentFactory * const pFac( new Factory(*this) );
        pFac->acquire();
        return pFac;
    }
    return nullptr;
}

uno::Sequence<OUString> ServiceDecl::getSupportedServiceNames() const
{
    std::vector<OUString> vec;

    OString const str(m_pServiceNames);
    sal_Int32 nIndex = 0;
    do {
        OString const token( str.getToken( 0, cDelim, nIndex ) );
        vec.emplace_back( token.getStr(), token.getLength(),
                                      RTL_TEXTENCODING_ASCII_US );
    }
    while (nIndex >= 0);

    return comphelper::containerToSequence(vec);
}

bool ServiceDecl::supportsService( OUString const& name ) const
{
    OString const str(m_pServiceNames);
    sal_Int32 nIndex = 0;
    do {
        OString const token( str.getToken( 0, cDelim, nIndex ) );
        if (name.equalsAsciiL( token.getStr(), token.getLength() ))
            return true;
    }
    while (nIndex >= 0);
    return false;
}

OUString ServiceDecl::getImplementationName() const
{
    return OUString::createFromAscii(m_pImplName);
}

void* component_getFactoryHelper( const char* pImplName,
                                  std::initializer_list<ServiceDecl const *> args )
{
    for (ServiceDecl const * i: args) {
        assert(i != nullptr);
        void * fac = i->getFactory(pImplName);
        if (fac != nullptr) {
            return fac;
        }
    }
    return nullptr;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
