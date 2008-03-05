/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CConnection.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:31:51 $
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

#ifndef _CONNECTIVITY_CALC_CONNECTION_HXX_
#define _CONNECTIVITY_CALC_CONNECTION_HXX_

#ifndef _CONNECTIVITY_FILE_OCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif

namespace com { namespace sun { namespace star { namespace sheet {
    class XSpreadsheetDocument;
} } } }


namespace connectivity
{
    namespace calc
    {
        class ODriver;
        class OCalcConnection : public file::OConnection
        {
            // the spreadsheet document:
            ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument > m_xDoc;
            ::rtl::OUString     m_sPassword;
            String              m_aFileName;
            oslInterlockedCount m_nDocCount;

        public:
            OCalcConnection(ODriver* _pDriver);
            virtual ~OCalcConnection();

            virtual void construct(const ::rtl::OUString& _rUrl,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo )
                throw( ::com::sun::star::sdbc::SQLException);

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // OComponentHelper
            virtual void SAL_CALL disposing(void);

            // XConnection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > createCatalog();
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // no interface methods
            ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument> acquireDoc();
            void releaseDoc();

            class ODocHolder
            {
                OCalcConnection* m_pConnection;
                ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument> m_xDoc;
            public:
                ODocHolder(OCalcConnection* _pConnection) : m_pConnection(_pConnection)
                {
                    m_xDoc = m_pConnection->acquireDoc();
                }
                ~ODocHolder()
                {
                    m_xDoc = NULL;
                    m_pConnection->releaseDoc();
                }
                ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument> getDoc() const { return m_xDoc; }
            };
        };
    }
}

#endif // _CONNECTIVITY_CALC_CONNECTION_HXX_

