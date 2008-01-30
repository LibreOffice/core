/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BDatabaseMetaData.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:00:02 $
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

#ifndef _CONNECTIVITY_ODBC_ODATABASEMETADATA_HXX_
#include "odbc/ODatabaseMetaData.hxx"
#endif

namespace connectivity
{
    namespace odbc
    {
        class OConnection;
    }
    namespace adabas
    {
        typedef odbc::ODatabaseMetaData OAdabasDatabaseMetaData_BASE;

        class OAdabasDatabaseMetaData : public OAdabasDatabaseMetaData_BASE
        {
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > impl_getTypeInfo_throw();
        public:
            OAdabasDatabaseMetaData(const SQLHANDLE _pHandle,odbc::OConnection* _pCon)
                : ODatabaseMetaData(_pHandle,_pCon)
            {
            }

            // just to return our url
            virtual ::rtl::OUString SAL_CALL getURL(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsIntegrityEnhancementFacility(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}

