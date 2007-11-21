/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HCatalog.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:05:30 $
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
#ifndef CONNECTIVITY_HSQLDB_CATALOG_HXX
#define CONNECTIVITY_HSQLDB_CATALOG_HXX

#ifndef _CONNECTIVITY_SDBCX_CATALOG_HXX_
#include "connectivity/sdbcx/VCatalog.hxx"
#endif
#ifndef CONNECTIVITY_STDTYPEDEFS_HXX
#include "connectivity/StdTypeDefs.hxx"
#endif

namespace connectivity
{
    namespace hsqldb
    {
        // please don't name the class the same name as in an other namespaces
        // some compilers have problems with this task as I noticed on windows
        class OHCatalog : public connectivity::sdbcx::OCatalog
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

            /** calls XDatabaseMetaData::getTables.
                @param  _sKindOfObject
                    The type of tables to be fetched.
                @param  _rNames
                    The container for the names to be filled.
            */
            void refreshObjects(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _sKindOfObject,TStringVector& _rNames);

        public:
            // implementation of the pure virtual methods
            virtual void refreshTables();
            virtual void refreshViews() ;
            virtual void refreshGroups();
            virtual void refreshUsers() ;

        public:
            OHCatalog(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);

            inline sdbcx::OCollection*      getPrivateTables()  const { return m_pTables;}
            inline sdbcx::OCollection*      getPrivateViews()   const { return m_pViews; }
            inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection() const { return m_xConnection; }

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            // ::cppu::OComponentHelper
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // CONNECTIVITY_HSQLDB_CATALOG_HXX

