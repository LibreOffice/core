/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
            OUString m_sPassword;
            OUString m_aFileName;
            oslInterlockedCount m_nDocCount;

        public:
            OCalcConnection(ODriver* _pDriver);
            virtual ~OCalcConnection();

            virtual void construct(const OUString& _rUrl,
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
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

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
