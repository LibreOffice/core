/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabTables.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:55:37 $
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

#ifndef _CONNECTIVITY_MACAB_TABLES_HXX_
#define _CONNECTIVITY_MACAB_TABLES_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif

namespace connectivity
{
    namespace macab
    {
        class MacabTables : public sdbcx::OCollection
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >       m_xMetaData;

        protected:
            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);

        public:
            MacabTables(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rMetaData,
                ::cppu::OWeakObject& _rParent,
                ::osl::Mutex& _rMutex,
                const TStringVector &_rVector)
                : sdbcx::OCollection(_rParent,sal_True,_rMutex,_rVector),
                  m_xMetaData(_rMetaData)
                { }

            virtual void SAL_CALL disposing(void);
        };
    }
}

#endif // _CONNECTIVITY_MACAB_TABLES_HXX_
