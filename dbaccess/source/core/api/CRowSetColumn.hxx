/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CRowSetColumn.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:33:49 $
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
#ifndef DBACCESS_CORE_API_CROWSETCOLUMN_HXX
#define DBACCESS_CORE_API_CROWSETCOLUMN_HXX

#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#include "RowSetRow.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef DBACCESS_CORE_API_CROWSETDATACOLUMN_HXX
#include "CRowSetDataColumn.hxx"
#endif

namespace dbaccess
{
    class ORowSetColumn;
    typedef ::comphelper::OPropertyArrayUsageHelper<ORowSetColumn> ORowSetColumn_PROP;

    // -------------------------------------------------------------------------
    class ORowSetColumn :   public ORowSetColumn_PROP,
                            public ORowSetDataColumn
    {
    public:
        ORowSetColumn(  const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData >& _xMetaData,
                        const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XRow >& _xRow,
                        sal_Int32 _nPos,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMeta,
                        const ::rtl::OUString& _rDescription,
                        ORowSetCacheIterator& _rColumnValue)
          : ORowSetDataColumn(_xMetaData,_xRow,NULL,_nPos,_rxDBMeta,_rDescription,_rColumnValue)
        {
        }

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

    // com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue,
                                                            ::com::sun::star::uno::Any & rOldValue,
                                                            sal_Int32 nHandle,
                                                            const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue )throw (::com::sun::star::uno::Exception);

    protected:
        using ORowSetDataColumn::getFastPropertyValue;
    };

}
#endif // DBACCESS_CORE_API_CROWSETCOLUMN_HXX

