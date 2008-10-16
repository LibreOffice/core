/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: databasecontext.hxx,v $
 * $Revision: 1.17.2.2 $
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

#ifndef _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_
#define _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDATABASEENVIRONMENT_HPP_
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE7_HXX_
#include <cppuhelper/compbase7.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#include <comphelper/componentcontext.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_ELEMENTEXISTEXCEPTION_HPP_
#include <com/sun/star/container/ElementExistException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _DBA_COREDATAACCESS_MODELIMPL_HXX_
#include "ModelImpl.hxx"
#endif
#ifndef BASICMANAGERREPOSITORY_HXX
#include <basic/basicmanagerrepository.hxx>
#endif

#include <boost/shared_ptr.hpp>

// needed for registration
namespace com { namespace sun { namespace star {
    namespace lang
    {
        class XMultiServiceFactory;
        class IllegalArgumentException;
    }
} } }

//........................................................................
namespace dbaccess
{
//........................................................................

//============================================================
//= ODatabaseContext
//============================================================
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    ODatabaseContext_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

typedef ::cppu::WeakComponentImplHelper7    <   ::com::sun::star::lang::XServiceInfo
                                            ,   ::com::sun::star::container::XEnumerationAccess
                                            ,   ::com::sun::star::container::XNameAccess
                                            ,   ::com::sun::star::uno::XNamingService
                                            ,   ::com::sun::star::container::XContainer
                                            ,   ::com::sun::star::lang::XSingleServiceFactory
                                            ,   ::com::sun::star::lang::XUnoTunnel
                                            >   DatabaseAccessContext_Base;

class ODatabaseContext  :public DatabaseAccessContext_Base
                        ,public ::basic::BasicManagerCreationListener
{
private:
    /** loads the given object from the given URL
    @throws WrappedTargetException
        if an error occurs accessing the URL via the UCB

    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > loadObjectFromURL(const ::rtl::OUString& _rName,const ::rtl::OUString& _sURL);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getObject(const ::rtl::OUString& _rName);

    /** retrieves the URL for a given registration name, if any
        @returns <FALSE/> if and only if there exists a registration for the given name
        @throws IllegalArgumentException if the name is empty
    */
    bool    getURLForRegisteredObject( const ::rtl::OUString& _rRegisteredName, ::rtl::OUString& _rURL );

    /** sets all properties which were transient at the data source. e.g. password
        @param  _sURL       The file URL of the data source
        @param  _xObject    The data source itself.
    */
    void setTransientProperties(const ::rtl::OUString& _sURL, ODatabaseModelImpl& _rDataSourceModel );

    /** creates a new data source
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            impl_createNewDataSource();

protected:
    ::osl::Mutex                    m_aMutex;
    ::comphelper::ComponentContext  m_aContext;

    DECLARE_STL_USTRINGACCESS_MAP( ODatabaseModelImpl*, ObjectCache );
    ObjectCache     m_aDatabaseObjects;

    DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >, PropertyCache );
    PropertyCache   m_aDatasourceProperties;
        // as we hold our data sources weak, we have to cache all properties on the data sources which are
        // transient but stored as long as the session lasts. The database context is the session (as it lives
        // as long as the session does), but the data sources may die before the session does, and then be
        // recreated afterwards. So it's our (the context's) responsibility to store the session-persistent
        // properties.

    ::cppu::OInterfaceContainerHelper       m_aContainerListeners;

public:
    ODatabaseContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
    virtual ~ODatabaseContext();


// OComponentHelper
    virtual void SAL_CALL disposing(void);

// ::com::sun::star::lang::XSingleServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance(  ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::uno::XNamingService
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRegisteredObject( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerObject( const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Object ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL revokeObject( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::lang::XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    void registerPrivate(const ::rtl::OUString& _sName
                        ,const ::rtl::Reference<ODatabaseModelImpl>& _pModelImpl);
    void deregisterPrivate(const ::rtl::OUString& _sName);
    void nameChangePrivate(const ::rtl::OUString& _sOldName, const ::rtl::OUString& _sNewName);
    void storeTransientProperties( ODatabaseModelImpl& _rModelImpl);

private:
    // BasicManagerCreationListener
    virtual void onBasicManagerCreated(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxForDocument,
        BasicManager& _rBasicManager
    );
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_


