/*************************************************************************
 *
 *  $RCSfile: MasterScriptProvider.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:10:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FRAMEWORK_SCRIPT_PROVIDER_XFUNCTIONPROVIDER_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_XFUNCTIONPROVIDER_HXX_

#include <rtl/ustring>

#include <cppuhelper/implbase5.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <com/sun/star/lang/XInitialization.hpp>

#include <drafts/com/sun/star/script/provider/XScriptProvider.hpp>
#include <drafts/com/sun/star/script/browse/XBrowseNode.hpp>

#include "InvocationCtxProperties.hxx"
#include "ProviderCache.hxx"

namespace func_provider
{
// for simplification
#define css ::com::sun::star
#define dcsss ::drafts::com::sun::star::script

 typedef ::cppu::WeakImplHelper5<
     dcsss::provider::XScriptProvider,
     dcsss::browse::XBrowseNode, css::lang::XServiceInfo,
     css::lang::XInitialization,
     css::container::XNameContainer > t_helper;

class MasterScriptProvider :
            public t_helper
{
public:
    MasterScriptProvider(
        const css::uno::Reference< css::uno::XComponentContext >
        & xContext ) throw( css::uno::RuntimeException );
    ~MasterScriptProvider();

    // XServiceInfo implementation
    virtual ::rtl::OUString SAL_CALL getImplementationName( )
        throw( css::uno::RuntimeException );

    // XBrowseNode implementation
    virtual ::rtl::OUString SAL_CALL getName()
        throw ( css::uno::RuntimeException );
    virtual css::uno::Sequence< css::uno::Reference< dcsss::browse::XBrowseNode > > SAL_CALL getChildNodes()
        throw ( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasChildNodes()
        throw ( css::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL getType()
        throw ( css::uno::RuntimeException );
    // XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const css::uno::Any& aElement ) throw ( css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name ) throw ( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const css::uno::Any& aElement ) throw ( css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw ( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw ( css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw ( css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw ( css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames( )
        throw( css::uno::RuntimeException );

    // XScriptProvider implementation
    virtual css::uno::Reference < dcsss::provider::XScript > SAL_CALL
        getScript( const ::rtl::OUString& scriptURI )
        throw( dcsss::provider::ScriptFrameworkErrorException,
               css::uno::RuntimeException );

    /**
     *  XInitialise implementation
     *
     * @param args expected to contain a single ::rtl::OUString
     * containing the URI
     */
    virtual void SAL_CALL initialize( const css::uno::Sequence < css::uno::Any > & args )
        throw ( css::uno::Exception, css::uno::RuntimeException);

    // Public method to return all Language Providers in this MasterScriptProviders
    // context.
    css::uno::Sequence< css::uno::Reference< dcsss::provider::XScriptProvider > > SAL_CALL
        getAllProviders() throw ( css::uno::RuntimeException );

    bool isPkgProvider() { return m_bIsPkgMSP; }
    css::uno::Reference< dcsss::provider::XScriptProvider > getPkgProvider() { return m_xMSPPkg; }
    // returns context string for this provider, eg
    ::rtl::OUString getContextString() { return m_sCtxString; }
    css::uno::Reference< css::frame::XModel > getModel() { return m_xModel; }
private:
    ::rtl::OUString parseLocationName( const ::rtl::OUString& location );
    void  createPkgProvider();
    bool  isValid();
    ::rtl::OUString getURLForModel();
    const css::uno::Sequence< ::rtl::OUString >& getProviderNames();

    ProviderCache* providerCache();
    /* to obtain other services if needed */
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMgr;
    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Sequence< css::uno::Any > m_sAargs;
    ::rtl::OUString m_sNodeName;
    // This component supports XInitialization, it can be created
    // using createInstanceXXX() or createInstanceWithArgumentsXXX using
    // the service Mangager.
    // Need to detect proper initialisation and validity
    // for the object, so m_bIsValid indicates that the object is valid is set in ctor
    // in case of createInstanceWithArgumentsXXX() called m_bIsValid is set to reset
    // and then set to true when initialisation is complete
    bool m_bIsValid;
    // m_bInitialised ensure initialisation only takes place once.
    bool m_bInitialised;
    bool m_bIsPkgMSP;
    css::uno::Reference< dcsss::provider::XScriptProvider > m_xMSPPkg;
    ProviderCache* m_pPCache;
    osl::Mutex m_mutex;
    ::rtl::OUString m_sCtxString;
};
} // namespace func_provider
#endif //_FRAMEWORK_SCRIPT_PROVIDER_XFUNCTIONPROVIDER_HXX_
