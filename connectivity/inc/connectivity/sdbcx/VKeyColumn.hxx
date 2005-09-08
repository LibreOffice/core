/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VKeyColumn.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:05:21 $
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

#ifndef _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_
#define _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif

namespace connectivity
{
    namespace sdbcx
    {
        class OKeyColumn;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OKeyColumn> OKeyColumn_PROP;

        class OKeyColumn :  public OColumn,
                            public OKeyColumn_PROP
        {
        protected:
            ::rtl::OUString m_ReferencedColumn;
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        public:
            OKeyColumn(sal_Bool _bCase);
            OKeyColumn( const ::rtl::OUString& _ReferencedColumn,
                            const ::rtl::OUString& _Name,
                            const ::rtl::OUString& _TypeName,
                            const ::rtl::OUString& _DefaultValue,
                            sal_Int32       _IsNullable,
                            sal_Int32       _Precision,
                            sal_Int32       _Scale,
                            sal_Int32       _Type,
                            sal_Bool        _IsAutoIncrement,
                            sal_Bool        _IsRowVersion,
                            sal_Bool        _IsCurrency,
                            sal_Bool        _bCase);
            // just to make it not inline
            virtual ~OKeyColumn();

            virtual void construct();
            DECLARE_SERVICE_INFO();
        };
    }
}
#endif // _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_

