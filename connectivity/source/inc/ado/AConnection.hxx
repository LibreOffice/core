/*************************************************************************
 *
 *  $RCSfile: AConnection.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:11:23 $
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
#ifndef _CONNECTIVITY_ADO_ACONNECTION_HXX_
#define _CONNECTIVITY_ADO_ACONNECTION_HXX_

#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _CONNECTIVITY_OSUBCOMPONENT_HXX_
#include "OSubComponent.hxx"
#endif
#include <map>
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CONNECTIVITY_OTYPEINFO_HXX_
#include "OTypeInfo.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#include "ado/Awrapado.hxx"

namespace connectivity
{
    namespace ado
    {
        struct OExtendedTypeInfo
        {
            ::connectivity::OTypeInfo       aSimpleType;    // the general type info
            DataTypeEnum                    eType;

            inline ::rtl::OUString getDBName() const { return aSimpleType.aTypeName; }
        };

        class WpADOConnection;
        class ODriver;
        class OCatalog;
        typedef ::std::multimap<DataTypeEnum, OExtendedTypeInfo*>       OTypeInfoMap;
        typedef connectivity::OMetaConnection                           OConnection_BASE;


        class OConnection : public OConnection_BASE,
                            public connectivity::OSubComponent<OConnection, OConnection_BASE>
        {
            friend class connectivity::OSubComponent<OConnection, OConnection_BASE>;

        protected:
            //====================================================================
            // Data attributes
            //====================================================================
            OTypeInfoMap                m_aTypeInfo;    //  vector containing an entry
                                                                                //  for each row returned by
                                                                                //  DatabaseMetaData.getTypeInfo.
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData >       m_xMetaData;

            connectivity::OWeakRefArray m_aStatements;  //  vector containing a list
                                                                                //  of all the Statement objects
                                                                                //  for this Connection
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbcx::XTablesSupplier>      m_xCatalog;
            ODriver*                    m_pDriver;
        private:
            WpADOConnection*            m_pAdoConnection;
            OCatalog*                   m_pCatalog;
            sal_Int32                   m_nEngineType;
            sal_Bool                    m_bClosed;
            sal_Bool                    m_bAutocommit;

        protected:
            void buildTypeInfo() throw( ::com::sun::star::sdbc::SQLException);
        public:

            OConnection(const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info,
                    ODriver*        _pDriver) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            //  OConnection(const SQLHANDLE _pConnectionHandle);
            ~OConnection();
            void construct(const ::rtl::OUString& url,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info);

            void closeAllStatements () throw( ::com::sun::star::sdbc::SQLException);

            //XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            // XServiceInfo
            DECLARE_SERVICE_INFO();
            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException);

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
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            //
            WpADOConnection* getConnection() { return m_pAdoConnection; }
            void setCatalog(const ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbcx::XTablesSupplier>& _xCat) { m_xCatalog = _xCat; }
            void setCatalog(OCatalog* _pCatalog) { m_pCatalog = _pCatalog; }

            const OTypeInfoMap* getTypeInfo() const { return &m_aTypeInfo;}
            OCatalog* getAdoCatalog() const
            {
                if ( m_xCatalog.get().is() )
                    return m_pCatalog;
                return NULL;
            }

            sal_Int32 getEngineType()   const { return m_nEngineType; }
            ODriver*  getDriver()       const { return m_pDriver; }

            static const OExtendedTypeInfo* getTypeInfoFromType(const OTypeInfoMap& _rTypeInfo,
                               DataTypeEnum _nType,
                               const ::rtl::OUString& _sTypeName,
                               sal_Int32 _nPrecision,
                               sal_Int32 _nScale,
                               sal_Bool& _brForceToType);
        };
    }
}
#endif // _CONNECTIVITY_ADO_ACONNECTION_HXX_

