/*************************************************************************
 *
 *  $RCSfile: VCatalog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-09 12:09:06 $
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
#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_TABLE_HXX_
#include "connectivity/sdbcx/VTable.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_VIEW_HXX_
#include "connectivity/sdbcx/VView.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_GROUP_HXX_
#include "connectivity/sdbcx/VGroup.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_USER_HXX_
#include "connectivity/sdbcx/VUser.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_KEY_HXX_
#include "connectivity/sdbcx/VKey.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_INDEX_HXX_
#include "connectivity/sdbcx/VIndex.hxx"
#endif
#ifndef _CONNECTIVITY_OSUBCOMPONENT_HXX_
#include "OSubComponent.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_
#include "connectivity/sdbcx/IRefreshable.hxx"
#endif

namespace connectivity
{
    namespace sdbcx
    {

        // OCatalog is a general catalog class
        // other drivers can be derived their catalog from this class when they want to support sdbcx
        // it holds already tables, views, groups and users

        typedef ::cppu::WeakComponentImplHelper5< ::com::sun::star::sdbcx::XTablesSupplier,
                                                  ::com::sun::star::sdbcx::XViewsSupplier,
                                                  ::com::sun::star::sdbcx::XUsersSupplier,
                                                  ::com::sun::star::sdbcx::XGroupsSupplier,
                                                  ::com::sun::star::lang::XServiceInfo> OCatalog_BASE;


        class OCatalog :    public OCatalog_BASE,
                            public IRefreshableGroups,
                            public IRefreshableUsers,
                            public connectivity::OSubComponent<OCatalog>
        {
            friend class connectivity::OSubComponent<OCatalog>;
        protected:

            ::osl::Mutex        m_aMutex;

            // this members are deleted when the dtor is called
            // they are hold weak
            OCollection*        m_pTables;
            OCollection*        m_pViews;
            OCollection*        m_pGroups;
            OCollection*        m_pUsers;

        public:
            DECLARE_CTY_ACQUIRE( OCatalog_BASE);

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
            void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException);
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

