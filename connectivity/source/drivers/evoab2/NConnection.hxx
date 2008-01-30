 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NConnection.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:51:22 $
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

#ifndef _CONNECTIVITY_EVOAB_CONNECTION_HXX_
#define _CONNECTIVITY_EVOAB_CONNECTION_HXX_

#ifndef _CONNECTIVITY_EVOAB_DRIVER_HXX_
#include "NDriver.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _CONNECTIVITY_OSUBCOMPONENT_HXX_
#include "OSubComponent.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _CONNECTIVITY_EVOAB_EVOLUTION_API_HXX_
#include "EApi.h"
#endif
namespace connectivity
{
    namespace evoab
    {

        namespace SDBCAddress {
            typedef enum {
                Unknown     = 0,
                EVO_LOCAL       = 1,
                EVO_LDAP    = 2,
                EVO_GWISE   = 3
            } sdbc_address_type;
        }

        typedef connectivity::OMetaConnection               OConnection_BASE; // implements basics and text encoding

        class OEvoabConnection : public OConnection_BASE,
                     public connectivity::OSubComponent<OEvoabConnection, OConnection_BASE>
        {

            friend class connectivity::OSubComponent<OEvoabConnection, OConnection_BASE>;

        private:
            OEvoabDriver   *m_pDriver;
            ::rtl::OUString m_pCurrentTableName;
            SDBCAddress::sdbc_address_type  m_eSDBCAddressType;

        protected:

            ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier>       m_xCatalog;
            rtl::OString  m_aPassword;


        public:
            OEvoabConnection(OEvoabDriver*  _pDriver);
            virtual ~OEvoabConnection();

            virtual void construct(const ::rtl::OUString& _rUrl,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo ) throw( ::com::sun::star::sdbc::SQLException);

            inline rtl::OString getPassword() { return m_aPassword; }
            inline void         setPassword( rtl::OString aStr ) { m_aPassword = aStr; }
            inline rtl::OUString& getCurrentTableName() {return m_pCurrentTableName;}
            inline void setCurrentTableName(::rtl::OUString _name) {m_pCurrentTableName=_name;}
            // own methods
            inline const OEvoabDriver*  getDriver()             const { return static_cast< const OEvoabDriver* >( m_pDriver );         }

            SDBCAddress::sdbc_address_type getSDBCAddressType() const { return m_eSDBCAddressType;}
            void setSDBCAddressType(SDBCAddress::sdbc_address_type _eSDBCAddressType) {m_eSDBCAddressType = _eSDBCAddressType;}

            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual void SAL_CALL release() throw();

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XConnection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > createCatalog();
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


        };
    }
}
#endif // _CONNECTIVITY_EVOAB_CONNECTION_HXX_
