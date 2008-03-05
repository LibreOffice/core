/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connection.hxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:04:59 $
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
#ifndef _DBA_CORE_CONNECTION_HXX_
#define _DBA_CORE_CONNECTION_HXX_

#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_CORE_QUERYCONTAINER_HXX_
#include "querycontainer.hxx"
#endif
#ifndef _DBA_CORE_TABLECONTAINER_HXX_
#include "tablecontainer.hxx"
#endif
#ifndef _DBA_CORE_VIEWCONTAINER_HXX_
#include "viewcontainer.hxx"
#endif
#ifndef DBA_CORE_REFRESHLISTENER_HXX
#include "RefreshListener.hxx"
#endif
#ifndef DBA_CORE_WARNINGS_HXX
#include "warning.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMMANDPREPARATION_HPP_
#include <com/sun/star/sdb/XCommandPreparation.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XUSERSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XGROUPSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XGroupsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_TOOLS_XCONNECTIONTOOLS_HPP_
#include <com/sun/star/sdb/tools/XConnectionTools.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_APPLICATION_XTABLEUIPROVIDER_HPP_
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#endif
/** === end UNO includes === **/

#if ! defined(INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_14)
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_14
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 14
#include <comphelper/implbase_var.hxx>
#endif

#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#include <comphelper/componentcontext.hxx>
#endif

#ifndef _CONNECTIVITY_CONNECTIONWRAPPER_HXX_
#include <connectivity/ConnectionWrapper.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//==========================================================================
typedef ::comphelper::ImplHelper14  <   ::com::sun::star::container::XChild
                                    ,   ::com::sun::star::sdbcx::XTablesSupplier
                                    ,   ::com::sun::star::sdbcx::XViewsSupplier
                                    ,   ::com::sun::star::sdbc::XConnection
                                    ,   ::com::sun::star::sdbc::XWarningsSupplier
                                    ,   ::com::sun::star::sdb::XQueriesSupplier
                                    ,   ::com::sun::star::sdb::XSQLQueryComposerFactory
                                    ,   ::com::sun::star::sdb::XCommandPreparation
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::lang::XMultiServiceFactory
                                    ,   ::com::sun::star::sdbcx::XUsersSupplier
                                    ,   ::com::sun::star::sdbcx::XGroupsSupplier
                                    ,   ::com::sun::star::sdb::tools::XConnectionTools
                                    ,   ::com::sun::star::sdb::application::XTableUIProvider
                                    >   OConnection_Base;

class ODatabaseSource;
//==========================================================================
//= OConnection
//==========================================================================
class OConnection           :public ::comphelper::OBaseMutex
                            ,public OSubComponent
                            ,public ::connectivity::OConnectionWrapper
                            ,public OConnection_Base
                            ,public IRefreshListener
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier >
                            m_xMasterTables; // just to avoid the recreation of the catalog
    OWeakRefArray           m_aStatements;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                            m_xQueries;
    OWeakRefArray           m_aComposers;

    // the filter as set on the parent data link at construction of the connection
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableFilter;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableTypeFilter;
    ::comphelper::ComponentContext                      m_aContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >                     m_xMasterConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XConnectionTools >          m_xConnectionTools;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XTableUIProvider >    m_xTableUIProvider;


    OTableContainer*            m_pTables;
    OViewContainer*             m_pViews;
    WarningsContainer           m_aWarnings;
    oslInterlockedCount         m_nInAppend;
    sal_Bool                    m_bSupportsViews;       // true when the getTableTypes return "VIEW" as type
    sal_Bool                    m_bSupportsUsers;
    sal_Bool                    m_bSupportsGroups;

protected:
    virtual ~OConnection();
public:
    OConnection(ODatabaseSource& _rDB
                ,::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxMaster
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

// com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

// com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw( );
    virtual void SAL_CALL release() throw( );

// OComponentHelper
    virtual void SAL_CALL disposing(void);

// ::com::sun::star::container::XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbcx::XTablesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);
// ::com::sun::star::sdbcx::XViewsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getViews(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdb::XQueriesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getQueries(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdb::XSQLQueryComposerFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer > SAL_CALL createQueryComposer(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdb::XCommandPreparation
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCommand( const ::rtl::OUString& command, sal_Int32 commandType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XWarningsSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// XConnection
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL nativeSQL( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL commit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isClosed(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCatalog( const ::rtl::OUString& catalog ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCatalog(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XCloseable
    virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XUsersSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getUsers(  ) throw(::com::sun::star::uno::RuntimeException);
    // XGroupsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getGroups(  ) throw(::com::sun::star::uno::RuntimeException);

    // XConnectionTools
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XTableName > SAL_CALL createTableName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XObjectNames > SAL_CALL getObjectNames(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XDataSourceMetaData > SAL_CALL getDataSourceMetaData(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getFieldsByCommandDescriptor( ::sal_Int32 commandType, const ::rtl::OUString& command, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& keepFieldsAlive ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer > SAL_CALL getComposer( ::sal_Int32 commandType, const ::rtl::OUString& command ) throw (::com::sun::star::uno::RuntimeException);

    // XTableUIProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL getTableIcon( const ::rtl::OUString& TableName, ::sal_Int32 ColorMode ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getTableEditor( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& DocumentUI, const ::rtl::OUString& TableName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // IRefreshListener
    virtual void refresh(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rToBeRefreshed);

protected:
    inline  void checkDisposed() throw (::com::sun::star::lang::DisposedException)
    {
        if ( rBHelper.bDisposed || !m_xConnection.is() )
            throw ::com::sun::star::lang::DisposedException();
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > getMasterTables();

private:
    /** checks whether or not there are naming conflicts between tables and queries
    */
    void    impl_checkTableQueryNames_nothrow();

    /** loads the XConnectionTools implementation which we forward the respective functionality to

        @throws ::com::sun::star::uno::RuntimeException
            if the implementation cannot be loaded

        @postcond
            m_xConnectionTools is nol <NULL/>
    */
    void    impl_loadConnectionTools_throw();
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_CORE_CONNECTION_HXX_


