/*************************************************************************
 *
 *  $RCSfile: basprov.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: npower $ $Date: 2003-10-15 08:35:32 $
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

#ifndef SCRIPTING_BASPROV_HXX
#define SCRIPTING_BASPROV_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_BROWSE_XBROWSENODE_HPP_
#include <drafts/com/sun/star/script/framework/browse/XBrowseNode.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_PROVIDER_XSCRIPTPROVIDER_HPP_
#include <drafts/com/sun/star/script/framework/provider/XScriptProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

class BasicManager;


//.........................................................................
namespace basprov
{
//.........................................................................

    //  ----------------------------------------------------
    //  class BasicProviderImpl
    //  ----------------------------------------------------

    typedef ::cppu::WeakImplHelper4<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization,
        ::drafts::com::sun::star::script::framework::provider::XScriptProvider,
        ::drafts::com::sun::star::script::framework::browse::XBrowseNode > BasicProviderImpl_BASE;


    class BasicProviderImpl : public BasicProviderImpl_BASE
    {
    private:
        BasicManager*   m_pAppBasicManager;
        BasicManager*   m_pDocBasicManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer > m_xLibContainerApp;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer > m_xLibContainerDoc;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xScriptingContext;
        bool m_bIsAppScriptCtx;
        bool m_bIsUserCtx;
        ::rtl::OUString m_sCtxLocation;

        ::rtl::OUString getLocationFromURI( const ::rtl::OUString& scriptURI );

    public:
        BasicProviderImpl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
        virtual ~BasicProviderImpl();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XScriptProvider
        virtual ::com::sun::star::uno::Reference < ::drafts::com::sun::star::script::framework::provider::XScript > SAL_CALL getScript(
            const ::rtl::OUString& scriptURI )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XBrowseNode
        virtual ::rtl::OUString SAL_CALL getName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::framework::browse::XBrowseNode > > SAL_CALL getChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getType(  )
            throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace basprov
//.........................................................................

#endif // SCRIPTING_BASPROV_HXX
