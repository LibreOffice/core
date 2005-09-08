/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LConnection.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:43:43 $
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

#ifndef _CONNECTIVITY_EVOAB_LCONNECTION_HXX_
#define _CONNECTIVITY_EVOAB_LCONNECTION_HXX_

#ifndef _CONNECTIVITY_EVOAB_LDRIVER_HXX_
#include "LDriver.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_OCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_LCOLUMNALIAS_HXX_
#include "LColumnAlias.hxx"
#endif

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
