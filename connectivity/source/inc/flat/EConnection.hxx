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

#pragma once
#if 1

#include "file/FConnection.hxx"

namespace connectivity
{
    namespace flat
    {
        class ODriver;
        class OFlatConnection : public file::OConnection
        {
        private:
            sal_Int32   m_nMaxRowsToScan;
            sal_Bool    m_bHeaderLine;          // column names in first row
            sal_Unicode m_cFieldDelimiter;      // look at the name
            sal_Unicode m_cStringDelimiter;
            sal_Unicode m_cDecimalDelimiter;
            sal_Unicode m_cThousandDelimiter;
        public:
            OFlatConnection(ODriver*    _pDriver);
            virtual ~OFlatConnection();

            virtual void construct(const ::rtl::OUString& _rUrl,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo ) throw( ::com::sun::star::sdbc::SQLException);

            // own methods
            inline sal_Bool     isHeaderLine()          const { return m_bHeaderLine;       }
            inline sal_Unicode  getFieldDelimiter()     const { return m_cFieldDelimiter;   }
            inline sal_Unicode  getStringDelimiter()    const { return m_cStringDelimiter;  }
            inline sal_Unicode  getDecimalDelimiter()   const { return m_cDecimalDelimiter; }
            inline sal_Unicode  getThousandDelimiter()  const { return m_cThousandDelimiter;}
            inline sal_Int32    getMaxRowsToScan()      const { return m_nMaxRowsToScan;}
            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XConnection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > createCatalog();
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // _CONNECTIVITY_FLAT_DCONNECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
