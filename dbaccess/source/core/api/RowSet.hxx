/*************************************************************************
 *
 *  $RCSfile: RowSet.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:38 $
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

#ifndef DBACCESS_CORE_API_ROWSET_HXX
#define DBACCESS_CORE_API_ROWSET_HXX

#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XRESULTSETACCESS_HPP_
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWUPDATE_HPP_
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPARAMETERS_HPP_
#include <com/sun/star/sdbc/XParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XROWSETAPPROVEBROADCASTER_HPP_
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDELETEROWS_HPP_
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE8_HXX_
#include <cppuhelper/compbase8.hxx>
#endif
#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include <connectivity/sqliterator.hxx>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif
#ifndef DBACCESS_CORE_API_ROWSETVALUE_HXX
#include "RowSetValue.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#include "RowSetBase.hxx"

namespace dbaccess
{
    typedef ::cppu::WeakAggComponentImplHelper8<  ::com::sun::star::sdb::XResultSetAccess,
                                                ::com::sun::star::sdb::XRowSetApproveBroadcaster,
                                                ::com::sun::star::sdbcx::XDeleteRows,
                                                ::com::sun::star::sdbc::XParameters,
                                                ::com::sun::star::lang::XEventListener,
                                                ::com::sun::star::sdbc::XResultSetUpdate,
                                                ::com::sun::star::sdbc::XRowUpdate,
                                                ::com::sun::star::util::XCancellable
                                                > ORowSet_BASE1;

    class ORowSet : public ORowSet_BASE1
                    , public ORowSetBase
                    , public ::utl::OPropertyArrayUsageHelper<ORowSet>
    {
        friend class ORowSetClone;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xActiveConnection;
        ::com::sun::star::uno::Any                                                      m_aActiveConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xTypeMap;
        ::com::sun::star::uno::Any                                                      m_aTypeMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement >  m_xStatement;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >    m_xComposer;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xColumns; // the columns from a table or query

        connectivity::OWeakRefArray                 m_aClones;
        connectivity::OSQLParser                    m_aParser;
        connectivity::OSQLParseNode*                m_pParseTree;

        connectivity::ORowVector< ORowSetValue >    m_aParameterRow; // hold all parameters

        rtl::OUString                               m_aCommand;
        rtl::OUString                               m_aDataSourceName;
        rtl::OUString                               m_aURL;
        rtl::OUString                               m_aUser;
        rtl::OUString                               m_aPassword;
        rtl::OUString                               m_aFilter;
        rtl::OUString                               m_aOrder;
        rtl::OUString                               m_aActiveCommand;
        rtl::OUString                               m_aCursorName;
        rtl::OUString                               m_aUpdateTableName; // is set by a query

        sal_Int32                   m_nFetchDirection;
        sal_Int32                   m_nFetchSize;
        sal_Int32                   m_nMaxFieldSize;
        sal_Int32                   m_nMaxRows;
        sal_Int32                   m_nResultSetConcurrency;
        sal_Int32                   m_nResultSetType;
        sal_Int32                   m_nQueryTimeOut;
        sal_Int32                   m_nCommandType;
        sal_Int32                   m_nTransactionIsolation;
        sal_Int32                   m_nPrivileges;
        sal_Int32                   m_nPosition;
        sal_Int32                   m_nAsyncUpdateRowCount;
        sal_Bool                    m_bUseEscapeProcessing ;
        sal_Bool                    m_bApplyFilter ;
        sal_Bool                    m_bIgnoreResult ;
        sal_Bool                    m_bFirst ;
        sal_Bool                    m_bLast ;
        sal_Bool                    m_bCreateStatement ;    // determines we to create a new prepared statement
        sal_Bool                    m_bDeleted ;
        sal_Bool                    m_bInserted ;
        sal_Bool                    m_bUpdated ;
        sal_Bool                    m_bOwnsResultRow ;
        sal_Bool                    m_bRowObsolete ;
        sal_Bool                    m_bModified ;
        sal_Bool                    m_bCanceled ;
        sal_Bool                    m_bRebuildConnOnExecute ;
        sal_Bool                    m_bIsBookmarable ;
        sal_Bool                    m_bNew ;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >  calcConnection() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        rtl::OUString getComposedQuery(const rtl::OUString& rQuery, sal_Bool bEscapeProcessing) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        rtl::OUString getCommand(sal_Bool& bEscapeProcessing);

    protected:
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue,sal_Int32 nHandle) const;

        //  void firePropertyChange(const ORowSetMatrix::iterator& _rOldRow);
    public:
        ORowSet(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::uno::XAggregation
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo - static methods
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    // OComponentHelper
        virtual void SAL_CALL disposing(void);

    // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XCloseable
        virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // utl::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // ::com::sun::star::sdbc::XRowUpdate
        virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deleteRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRowSet
        virtual void SAL_CALL execute(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdb::XRowSetApproveBroadcaster
        virtual void SAL_CALL addRowSetApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener >& listener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRowSetApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener >& listener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdb::XResultSetAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL createResultSet(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XResultSet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XCancellable
        virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // XParameters
        virtual void SAL_CALL setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBytes( sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBinaryStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCharacterStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setObject( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setObjectWithInfo( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setRef( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBlob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setClob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setArray( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearParameters(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    };
    //************************************************************
    //  ORowSetClone
    //************************************************************
    class ORowSetClone : public ORowSetBase
                         ,public OSubComponent
                         ,public ::utl::OPropertyArrayUsageHelper < ORowSetClone >
    {
    protected:
        sal_Int32                   m_nFetchDirection;
        sal_Int32                   m_nFetchSize;
        sal_Int32                   m_nResultSetConcurrency;
        sal_Int32                   m_nResultSetType;
        sal_Int32                   m_nPosition;
        sal_Bool                    m_bIsBookmarable;
        sal_Bool                    m_bFirst : 1;
        sal_Bool                    m_bLast : 1;
        sal_Bool                    m_bRowObsolete : 1;
        sal_Bool                    m_bDeleted : 1;

    protected:
        //  virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);
        //  virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue,sal_Int32 nHandle) const;
        //  void firePropertyChange(const ORowSetMatrix::iterator& _rOldRow);
    public:
        ORowSetClone(ORowSet& rParent);
        virtual ~ORowSetClone();

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
        virtual void SAL_CALL disposing(void);

    // ::com::sun::star::sdbc::XCloseable
        virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
        {
            return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
        }

    // utl::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    };

}
#endif // DBACCESS_CORE_API_ROWSET_HXX
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/09/18 14:52:46  willem.vandorp
    OpenOffice header added.

    Revision 1.1  2000/09/01 15:19:29  oj
    rowset addons

    Revision 1.0 25.07.2000 13:24:59  oj
------------------------------------------------------------------------*/

