/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_CALC_CONNECTION_HXX_
#define _CONNECTIVITY_CALC_CONNECTION_HXX_

#include "file/FConnection.hxx"

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
                   m_xDoc.clear();
                    m_pConnection->releaseDoc();
                }
                ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument> getDoc() const { return m_xDoc; }
            };
        };
    }
}

#endif // _CONNECTIVITY_CALC_CONNECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
