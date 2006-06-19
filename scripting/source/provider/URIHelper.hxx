/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: URIHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:22:16 $
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

#ifndef _FRAMEWORK_SCRIPT_PROVIDER_XSCRIPTURIHELPER_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_XSCRIPTURIHELPER_HXX_

#include <com/sun/star/script/provider/XScriptURIHelper.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase3.hxx>

namespace func_provider
{

#define css ::com::sun::star

class ScriptingFrameworkURIHelper :
    public ::cppu::WeakImplHelper3<
        css::script::provider::XScriptURIHelper,
        css::lang::XServiceInfo,
        css::lang::XInitialization >
{
private:

    css::uno::Reference< css::ucb::XSimpleFileAccess > m_xSimpleFileAccess;
    css::uno::Reference<css::uri::XUriReferenceFactory> m_xUriReferenceFactory;

    ::rtl::OUString m_sLanguage;
    ::rtl::OUString m_sLocation;
    ::rtl::OUString m_sBaseURI;

    ::rtl::OUString SCRIPTS_PART;

    bool initBaseURI();
    ::rtl::OUString getLanguagePart(const ::rtl::OUString& rStorageURI);
    ::rtl::OUString getLanguagePath(const ::rtl::OUString& rLanguagePart);

public:

    ScriptingFrameworkURIHelper(
        const css::uno::Reference< css::uno::XComponentContext >& xContext )
            throw( css::uno::RuntimeException );

    ~ScriptingFrameworkURIHelper();

    virtual void SAL_CALL
        initialize( const css::uno::Sequence < css::uno::Any > & args )
            throw ( css::uno::Exception, css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL
        getRootStorageURI()
            throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
        getScriptURI( const ::rtl::OUString& rStorageURI )
            throw( css::lang::IllegalArgumentException,
                   css::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
        getStorageURI( const ::rtl::OUString& rScriptURI )
            throw( css::lang::IllegalArgumentException,
                   css::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
        getImplementationName()
            throw( css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        supportsService( const ::rtl::OUString& ServiceName )
            throw( css::uno::RuntimeException );

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw( css::uno::RuntimeException );
};

} // namespace func_provider
#endif //_FRAMEWORK_SCRIPT_PROVIDER_XSCRIPTURIHELPER_HXX_
