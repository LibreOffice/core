/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MABConnection.hxx,v $
 * $Revision: 1.12 $
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

#ifndef _CONNECTIVITY_MAB_CONNECTION_HXX_
#define _CONNECTIVITY_MAB_CONNECTION_HXX_

#include "file/FConnection.hxx"

#ifndef _CONNECTIVITY_MAB_COLUMNALIAS_HXX_
#include "MABColumnAlias.hxx"
#endif

namespace com { namespace sun { namespace star { namespace sheet {
    class XSpreadsheetDocument;
} } } }


namespace connectivity
{
    namespace mozaddressbook
    {
        class OMozabDriver;
        class OMozabConnection : public file::OConnection
        {
            OMozabColumnAlias m_aColumnAlias;
            ::rtl::OUString m_sURL;                 // contains the real url set by construct
            rtl::OUString   m_sMozillaURI;
            sal_Int32       m_nAnonABCount;
            sal_Int32       m_nMaxResultRecords;
            sal_Bool        m_UsesFactory ;
            sal_Bool        m_IsLDAP ;
            sal_Bool        m_bOutlookExpress;

        public:
            OMozabConnection(OMozabDriver* _pDriver);
            virtual ~OMozabConnection();

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
            rtl::OUString getMozURI() const
                { return m_sMozillaURI; }

            ::rtl::OUString getConnectionURL() const { return m_sURL; }

            sal_Bool usesFactory(void) const { return m_UsesFactory ; }
            sal_Bool isLDAP(void) const { return m_IsLDAP ; }

            sal_Bool    isOutlookExpress() const { return m_bOutlookExpress;}
            sal_Int32   getNextAnonymousAB()    { return (++m_nAnonABCount); }
            sal_Int32   getMaxResultRecords() const { return m_nMaxResultRecords; }
            const OMozabColumnAlias & getColumnAlias() const
                          { return (m_aColumnAlias); }

            // static methods to return the names of the uri
            static const sal_Char*    getSDBC_SCHEME_MOZILLA();
            static const sal_Char*    getSDBC_SCHEME_LDAP();
            static const sal_Char*    getSDBC_SCHEME_OUTLOOK_MAPI();
            static const sal_Char*    getSDBC_SCHEME_OUTLOOK_EXPRESS();
        };
    }
}

#endif // _CONNECTIVITY_MAB_CONNECTION_HXX_

