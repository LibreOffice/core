/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AColumn.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:48:30 $
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

#ifndef _CONNECTIVITY_ADO_COLUMN_HXX_
#define _CONNECTIVITY_ADO_COLUMN_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADOX_HXX_
#include "ado/Awrapadox.hxx"
#endif

namespace connectivity
{
    namespace ado
    {
        class OConnection;
        typedef sdbcx::OColumn OColumn_ADO;
        class OAdoColumn :  public OColumn_ADO
        {
            WpADOColumn     m_aColumn;
            OConnection*    m_pConnection;
            ::rtl::OUString m_ReferencedColumn;
            sal_Bool        m_IsAscending;

            void fillPropertyValues();
        protected:
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                     )
                                     throw (::com::sun::star::uno::Exception);
        public:
            OAdoColumn(sal_Bool _bCase,OConnection* _pConnection,_ADOColumn* _pColumn);
            OAdoColumn(sal_Bool _bCase,OConnection* _pConnection);
            // ODescriptor
            virtual void construct();
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            WpADOColumn     getColumnImpl() const;
        };
    }
}
#endif // _CONNECTIVITY_ADO_COLUMN_HXX_

