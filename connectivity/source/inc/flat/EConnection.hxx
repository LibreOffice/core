/*************************************************************************
 *
 *  $RCSfile: EConnection.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-05 14:46:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_FLAT_ECONNECTION_HXX_
#define _CONNECTIVITY_FLAT_ECONNECTION_HXX_

#ifndef _CONNECTIVITY_FILE_OCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif

namespace connectivity
{
    namespace flat
    {
        class ODriver;
        class OFlatConnection : public file::OConnection
        {
        private:
            sal_Bool    m_bFixedLength;         // row of fixed length
            sal_Bool    m_bHeaderLine;          // column names in first row
            sal_uInt8   m_cFieldDelimiter;      // look at the name
            sal_uInt8   m_cStringDelimiter;     // delimiter for strings m_cStringDelimiter blabla m_cStringDelimiter
            sal_uInt8   m_cDecimalDelimiter;    // Dezimal-delimiter (Dezimalpoint)
            sal_uInt8   m_cThousandDelimiter;   //
        public:
            OFlatConnection(ODriver*    _pDriver);
            virtual ~OFlatConnection();

            virtual void construct(const ::rtl::OUString& _rUrl,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo ) throw( ::com::sun::star::sdbc::SQLException);

            // own methods
            sal_Bool    isFixedLength()         const { return m_bFixedLength;      }
            sal_Bool    isHeaderLine()          const { return m_bHeaderLine;       }
            sal_uInt8   getFieldDelimiter()     const { return m_cFieldDelimiter;   }
            sal_uInt8   getStringDelimiter()    const { return m_cStringDelimiter;  }
            sal_uInt8   getDecimalDelimiter()   const { return m_cDecimalDelimiter; }
            sal_uInt8   getThousandDelimiter()  const { return m_cThousandDelimiter;}
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

