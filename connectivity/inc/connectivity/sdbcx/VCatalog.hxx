/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCatalog.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:03:20 $
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

#ifndef _CONNECTIVITY_SDBCX_CATALOG_HXX_
#define _CONNECTIVITY_SDBCX_CATALOG_HXX_

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
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
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CONNECTIVITY_OSUBCOMPONENT_HXX_
#include "OSubComponent.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_
#include "connectivity/sdbcx/IRefreshable.hxx"
#endif
#ifndef CONNECTIVITY_STDTYPEDEFS_HXX
#include "connectivity/StdTypeDefs.hxx"
#endif

namespace connectivity
{
    namespace sdbcx
    {

        class OCollection;
        // OCatalog is a general catalog class
        // other drivers can be derived their catalog from this class when they want to support sdbcx
        // it holds already tables, views, groups and users

        typedef ::cppu::WeakComponentImplHelper5< ::com::sun::star::sdbcx::XTablesSupplier,
                                                  ::com::sun::star::sdbcx::XViewsSupplier,
                                                  ::com::sun::star::sdbcx::XUsersSupplier,
                                                  ::com::sun::star::sdbcx::XGroupsSupplier,
                                                  ::com::sun::star::lang::XServiceInfo> OCatalog_BASE;


        class SAL_NO_VTABLE OCatalog :  public OCatalog_BASE,
                            public IRefreshableGroups,
                            public IRefreshableUsers,
                            public connectivity::OSubComponent<OCatalog, OCatalog_BASE>
        {
            friend class connectivity::OSubComponent<OCatalog, OCatalog_BASE>;
        protected:

            ::osl::Mutex        m_aMutex;

            // this members are deleted when the dtor is called
            // they are hold weak
            OCollection*        m_pTables;
            OCollection*        m_pViews;
            OCollection*        m_pGroups;
            OCollection*        m_pUsers;

            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData; // just to make things easier

            /** builds the name which should be used to access the object later on in the collection.
                Will only be called in fillNames.
                @param  _xRow
                    The current row from the resultset given to fillNames.
            */
            virtual ::rtl::OUString buildName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >& _xRow);

            /** fills a vector with the nescessary names which can be used in combination with the collections.
                For each row buildName will be called.
                @param  _xResult
                    The resultset which should be used to fill the names. Will be disposed after return and set to NULL.
                @param  _rNames
                    The vector who will be filled.
            */
            void fillNames(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& _xResult,TStringVector& _rNames);

        public:
            OCatalog(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> &_xConnection);
            virtual ~OCatalog();

            DECLARE_SERVICE_INFO();

            // refreshTables is called when the method getTables had been called
            // the member m_pTables has to be created
            virtual void refreshTables()    = 0;
            // refreshViews is called when the method getViews had been called
            virtual void refreshViews()     = 0;

            // the other refresh methods come from base classes IRefreshableGroups and IRefreshableUsers

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            void SAL_CALL acquire() throw();
            void SAL_CALL release() throw();
            // XTablesSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);
            // XViewsSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getViews(  ) throw(::com::sun::star::uno::RuntimeException);
            // XUsersSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getUsers(  ) throw(::com::sun::star::uno::RuntimeException);
            // XGroupsSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getGroups(  ) throw(::com::sun::star::uno::RuntimeException);

        };
    }
}

#endif // _CONNECTIVITY_SDBCX_CATALOG_HXX_

