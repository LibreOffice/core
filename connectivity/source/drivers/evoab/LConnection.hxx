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

#ifndef _CONNECTIVITY_EVOAB_LCONNECTION_HXX_
#define _CONNECTIVITY_EVOAB_LCONNECTION_HXX_

#include "LDriver.hxx"
#include "file/FConnection.hxx"
#include "LColumnAlias.hxx"

namespace connectivity
{
    namespace evoab
    {
        class OEvoabConnection : public file::OConnection
        {
        private:
            OColumnAlias    m_aColumnAlias;
            sal_Bool        m_bHeaderLine;          // column names in first row
            sal_Unicode     m_cFieldDelimiter;      // look at the name
            sal_Unicode     m_cStringDelimiter;     // delimiter for strings m_cStringDelimiter blabla m_cStringDelimiter
            sal_Unicode     m_cDecimalDelimiter;    // Dezimal-delimiter (Dezimalpoint)
            sal_Unicode     m_cThousandDelimiter;   //
            rtl::OUString   m_aEvoFlatURI;

        public:
            OEvoabConnection(OEvoabDriver*  _pDriver);
            virtual ~OEvoabConnection();

            virtual void construct(const ::rtl::OUString& _rUrl,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo ) throw( ::com::sun::star::sdbc::SQLException);

            // own methods
            inline const OEvoabDriver*  getDriver()             const { return static_cast< const OEvoabDriver* >( m_pDriver );         }
            inline sal_Bool         isHeaderLine()              const { return m_bHeaderLine;       }
            inline sal_Unicode      getFieldDelimiter()         const { return m_cFieldDelimiter;   }
            inline sal_Unicode      getStringDelimiter()            const { return m_cStringDelimiter;  }
            inline sal_Unicode      getDecimalDelimiter()           const { return m_cDecimalDelimiter; }
            inline sal_Unicode      getThousandDelimiter()          const { return m_cThousandDelimiter;}
            const OColumnAlias&     getColumnAlias()                const { return m_aColumnAlias; }

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
#endif // _CONNECTIVITY_EVOAB_LCONNECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
