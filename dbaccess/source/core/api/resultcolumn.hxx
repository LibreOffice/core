/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resultcolumn.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:40:45 $
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

#ifndef _DBACORE_RESULTCOLUMN_HXX_
#define _DBACORE_RESULTCOLUMN_HXX_

#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HDL_
#include <com/sun/star/sdbc/XResultSetMetaData.hdl>
#endif
#ifndef _DBA_COREAPI_COLUMN_HXX_
#include <column.hxx>
#endif
namespace dbaccess
{
    //************************************************************
    //  OResultColumn
    //************************************************************
    class OResultColumn : public OColumn,
                          public ::comphelper::OPropertyArrayUsageHelper < OResultColumn >
    {
    protected:
        ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData > m_xMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >   m_xDBMetaData;
        sal_Int32                   m_nPos;
        ::com::sun::star::uno::Any  m_aIsRowVersion;

        virtual ~OResultColumn();
    public:
        OResultColumn(
            const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData >& _xMetaData,
            sal_Int32 _nPos,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMeta );

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;

    private:
        void    impl_determineIsRowVersion_nothrow();

    protected:
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };
}
#endif // _DBACORE_RESULTCOLUMN_HXX_

