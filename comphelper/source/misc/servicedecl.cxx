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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include "comphelper/servicedecl.hxx"
#include "osl/diagnose.h"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "cppuhelper/implbase2.hxx"
#include "comphelper/sequence.hxx"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include <vector>

using namespace com::sun::star;

namespace comphelper {
namespace service_decl {

class ServiceDecl::Factory :
        public cppu::WeakImplHelper2<lang::XSingleComponentFactory,
                                     lang::XServiceInfo>,
        private boost::noncopyable
{
public:
    explicit Factory( ServiceDecl const& rServiceDecl )
        : m_rServiceDecl(rServiceDecl) {}

    // XServiceInfo:
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( rtl::OUString const& name )
        throw (uno::RuntimeException);
    virtual uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException);
    // XSingleComponentFactory:
    virtual uno::Reference<uno::XInterface> SAL_CALL createInstanceWithContext(
        uno::Reference<uno::XComponentContext> const& xContext )
        throw (uno::Exception);
    virtual uno::Reference<uno::XInterface> SAL_CALL
    createInstanceWithArgumentsAndContext(
    uno::Sequence<uno::Any> const& args,
    uno::Reference<uno::XComponentContext> const& xContext )
        throw (uno::Exception);

private:
    virtual ~Factory();

    ServiceDecl const& m_rServiceDecl;
};

ServiceDecl::Factory::~Factory()
{
}

// XServiceInfo:
rtl::OUString ServiceDecl::Factory::getImplementationName()
    throw (uno::RuntimeException)
{
    return m_rServiceDecl.getImplementationName();
}

sal_Bool ServiceDecl::Factory::supportsService( rtl::OUString const& name )
    throw (uno::RuntimeException)
{
    return m_rServiceDecl.supportsService(name);
}

uno::Sequence<rtl::OUString> ServiceDecl::Factory::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return m_rServiceDecl.getSupportedServiceNames();
}

// XSingleComponentFactory:
uno::Reference<uno::XInterface> ServiceDecl::Factory::createInstanceWithContext(
    uno::Reference<uno::XComponentContext> const& xContext )
    throw (uno::Exception)
{
    return m_rServiceDecl.m_createFunc(
        m_rServiceDecl, uno::Sequence<uno::Any>(), xContext );
}

uno::Reference<uno::XInterface>
ServiceDecl::Factory::createInstanceWithArgumentsAndContext(
    uno::Sequence<uno::Any > const& args,
    uno::Reference<uno::XComponentContext> const& xContext )
    throw (uno::Exception)
{
    return m_rServiceDecl.m_createFunc(
        m_rServiceDecl, args, xContext );
}

bool ServiceDecl::writeInfo( registry::XRegistryKey * xKey ) const
{
    bool bRet = false;
    if (xKey != 0) {
        rtl::OUStringBuffer buf;
        buf.append( static_cast<sal_Unicode>('/') );
        buf.appendAscii( m_pImplName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/UNO/SERVICES") );
        try {
            uno::Reference<registry::XRegistryKey> const xNewKey(
                xKey->createKey( buf.makeStringAndClear() ) );

            rtl::OString const str(m_pServiceNames);
            sal_Int32 nIndex = 0;
            do {
                rtl::OString const token( str.getToken( 0, m_cDelim, nIndex ) );
                xNewKey->createKey(
                    rtl::OUString( token.getStr(), token.getLength(),
                                   RTL_TEXTENCODING_ASCII_US ) );
            }
            while (nIndex >= 0);

            bRet = true;
        }
        catch (registry::InvalidRegistryException const&) {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return bRet;
}

void * ServiceDecl::getFactory( sal_Char const* pImplName ) const
{
    if (rtl_str_compare(m_pImplName, pImplName) == 0) {
        lang::XSingleComponentFactory * const pFac( new Factory(*this) );
        pFac->acquire();
        return pFac;
    }
    return 0;
}

uno::Sequence<rtl::OUString> ServiceDecl::getSupportedServiceNames() const
{
    std::vector<rtl::OUString> vec;

    rtl::OString const str(m_pServiceNames);
    sal_Int32 nIndex = 0;
    do {
        rtl::OString const token( str.getToken( 0, m_cDelim, nIndex ) );
        vec.push_back( rtl::OUString( token.getStr(), token.getLength(),
                                      RTL_TEXTENCODING_ASCII_US ) );
    }
    while (nIndex >= 0);

    return comphelper::containerToSequence(vec);
}

bool ServiceDecl::supportsService( ::rtl::OUString const& name ) const
{
    rtl::OString const str(m_pServiceNames);
    sal_Int32 nIndex = 0;
    do {
        rtl::OString const token( str.getToken( 0, m_cDelim, nIndex ) );
        if (name.equalsAsciiL( token.getStr(), token.getLength() ))
            return true;
    }
    while (nIndex >= 0);
    return false;
}

rtl::OUString ServiceDecl::getImplementationName() const
{
    return rtl::OUString::createFromAscii(m_pImplName);
}

} // namespace service_decl
} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
