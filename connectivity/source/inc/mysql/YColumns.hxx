/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: YColumns.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:37:05 $
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
#ifndef CONNECTIVITY_MYSQL_COLUMN_HXX
#define CONNECTIVITY_MYSQL_COLUMN_HXX
#ifndef CONNECTIVITY_COLUMNSHELPER_HXX
#include "connectivity/TColumnsHelper.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif

namespace connectivity
{
    namespace mysql
    {
        class OMySQLColumns : public OColumnsHelper
        {
        protected:
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
        public:
            OMySQLColumns(  ::cppu::OWeakObject& _rParent
                            ,sal_Bool _bCase
                            ,::osl::Mutex& _rMutex
                            ,const TStringVector &_rVector
                            ,sal_Bool _bUseHardRef = sal_True
                        );
        };

        class OMySQLColumn;
        typedef sdbcx::OColumn OMySQLColumn_BASE;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OMySQLColumn> OMySQLColumn_PROP;

        class OMySQLColumn :    public OMySQLColumn_BASE,
                                public OMySQLColumn_PROP
        {
            ::rtl::OUString m_sAutoIncrement;
        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        public:
            OMySQLColumn(sal_Bool _bCase);
            virtual void construct();

            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // CONNECTIVITY_MYSQL_COLUMN_HXX

