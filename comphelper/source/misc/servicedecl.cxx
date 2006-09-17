/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servicedecl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:14:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
            OSL_ENSURE( false, "### InvalidRegistryException!" );
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

