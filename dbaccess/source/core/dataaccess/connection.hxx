/*************************************************************************
 *
 *  $RCSfile: connection.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:08:25 $
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
#ifndef _DBA_CORE_CONNECTION_HXX_
#define _DBA_CORE_CONNECTION_HXX_

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
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE9_HXX_
#include <cppuhelper/implbase9.hxx>
#endif
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
#ifndef _CONNECTIVITY_CONNECTIONWRAPPER_HXX_
#include "connectivity/ConnectionWrapper.hxx"
#endif
#ifndef DBA_CORE_REFRESHLISTENER_HXX
#include "RefreshListener.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//==========================================================================
typedef ::cppu::ImplHelper9 <   ::com::sun::star::container::XChild
                            ,   ::com::sun::star::sdbcx::XTablesSupplier
                            ,   ::com::sun::star::sdbcx::XViewsSupplier
                            ,   ::com::sun::star::sdbc::XConnection
                            ,   ::com::sun::star::sdb::XQueriesSupplier
                            ,   ::com::sun::star::sdb::XSQLQueryComposerFactory
                            ,   ::com::sun::star::sdb::XCommandPreparation
                            ,   ::com::sun::star::lang::XServiceInfo
                            ,   ::com::sun::star::lang::XMultiServiceFactory
                            >   OConnection_Base;

class ODatabaseSource;
//==========================================================================
//= OConnection
//==========================================================================
class OConnection           :public ::comphelper::OBaseMutex
                            ,public OSubComponent
                            ,public ::connectivity::OConnectionWrapper
                            ,public OConnection_Base
                            ,public IWarningsContainer
                            ,public IRefreshListener
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier >
                            m_xMasterTables; // just to avoid the recreation of the catalog
    OWeakRefArray           m_aStatements;
    OQueryContainer*        m_pQueries;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        m_xQueries;
    OWeakRefArray           m_aComposers;

    // the filter as set on the parent data link at construction of the connection
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableFilter;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableTypeFilter;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xMasterConnection;

    OTableContainer*            m_pTables;
    OViewContainer*             m_pViews;
    ::com::sun::star::uno::Any  m_aAdditionalWarnings;  // own warnings (appended to the ones got by the master connection)
    sal_Bool                    m_bSupportsViews;       // true when the getTableTypes return "VIEW" as type

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

    // IRefreshListener
    virtual void refresh(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rToBeRefreshed);
protected:
    // IWarningsContainer
    virtual void appendWarning(const ::com::sun::star::sdbc::SQLException& _rWarning);
    virtual void appendWarning(const ::com::sun::star::sdbc::SQLWarning& _rWarning);
    virtual void appendWarning(const ::com::sun::star::sdb::SQLContext& _rContext);

protected:
    static void implConcatWarnings(::com::sun::star::uno::Any& _rChainLeft, const ::com::sun::star::uno::Any& _rChainRight);

    inline  void checkDisposed() throw (::com::sun::star::lang::DisposedException)
    {
        if ( rBHelper.bDisposed || !m_xConnection.is() )
            throw ::com::sun::star::lang::DisposedException();
    }
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_CORE_CONNECTION_HXX_


