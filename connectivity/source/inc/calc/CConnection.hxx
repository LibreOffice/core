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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_CALC_CCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_CALC_CCONNECTION_HXX

#include "file/FConnection.hxx"
#include <com/sun/star/uno/DeploymentException.hpp>

namespace com { namespace sun { namespace star {
    namespace sheet { class XSpreadsheetDocument; }
} } }

namespace utl { class CloseVeto; }


namespace connectivity
{
    namespace calc
    {
        class ODriver;
        class OCalcConnection : public file::OConnection
        {
            // the spreadsheet document:
            css::uno::Reference< css::sheet::XSpreadsheetDocument > m_xDoc;
            /// close listener that vetoes so nobody disposes m_xDoc
            ::std::unique_ptr< ::utl::CloseVeto> m_pCloseListener;
            OUString m_sPassword;
            OUString m_aFileName;
            oslInterlockedCount m_nDocCount;

        public:
            OCalcConnection(ODriver* _pDriver);
            virtual ~OCalcConnection();

            virtual void construct(const OUString& _rUrl,
                                   const css::uno::Sequence< css::beans::PropertyValue >& _rInfo )
                throw( css::sdbc::SQLException,
                       css::uno::RuntimeException,
                       css::uno::DeploymentException,
                       std::exception) override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XConnection
            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbcx::XTablesSupplier > createCatalog() override;
            virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            // no interface methods
            css::uno::Reference< css::sheet::XSpreadsheetDocument> const & acquireDoc();
            void releaseDoc();

            class ODocHolder
            {
                OCalcConnection* m_pConnection;
                css::uno::Reference< css::sheet::XSpreadsheetDocument> m_xDoc;
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
                const css::uno::Reference< css::sheet::XSpreadsheetDocument>& getDoc() const { return m_xDoc; }
            };
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_CALC_CCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
