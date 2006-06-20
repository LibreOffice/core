/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CRowSetDataColumn.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:34:16 $
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
#ifndef DBACCESS_CORE_API_CROWSETDATACOLUMN_HXX
#define DBACCESS_CORE_API_CROWSETDATACOLUMN_HXX

#ifndef _DBACORE_DATACOLUMN_HXX_
#include "datacolumn.hxx"
#endif
#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#include "RowSetRow.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef DBACCESS_ROWSETCACHEITERATOR_HXX
#include "RowSetCacheIterator.hxx"
#endif

namespace dbaccess
{
    class ORowSetDataColumn;
    typedef ::comphelper::OPropertyArrayUsageHelper<ORowSetDataColumn> ORowSetDataColumn_PROP;

    class ORowSetDataColumn :   public ODataColumn,
                                public OColumnSettings,
                                public ORowSetDataColumn_PROP
    {
    protected:
        ORowSetCacheIterator        m_aColumnValue;
        ::com::sun::star::uno::Any  m_aOldValue;

        ::rtl::OUString             m_aDescription;     // description
        ORowSetBase*                m_pRowSet;

        virtual ~ORowSetDataColumn();
    public:
        ORowSetDataColumn(const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData >& _xMetaData,
                          const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XRow >& _xRow,
                          const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XRowUpdate >& _xRowUpdate,
                          sal_Int32 _nPos,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMeta,
                          const ::rtl::OUString& _rDescription,
                          const ORowSetCacheIterator& _rColumnValue);


        // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);
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

        virtual void fireValueChange(const ::connectivity::ORowSetValue& _rOldValue);

    protected:
        using ODataColumn::getFastPropertyValue;
    };
    // -------------------------------------------------------------------------
//  typedef connectivity::ORefVector< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> >
//          ORowSetDataColumns_COLLECTION;

    typedef connectivity::sdbcx::OCollection ORowSetDataColumns_BASE;
    class ORowSetDataColumns : public ORowSetDataColumns_BASE
    {
        ::vos::ORef< ::connectivity::OSQLColumns> m_aColumns;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
    public:
        ORowSetDataColumns(
                        sal_Bool _bCase,
                        const ::vos::ORef< ::connectivity::OSQLColumns>& _rColumns,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector
                        );
        virtual ~ORowSetDataColumns();
        // only the name is identical to ::cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);
        void assign(const ::vos::ORef< ::connectivity::OSQLColumns>& _rColumns,const ::std::vector< ::rtl::OUString> &_rVector);
    };
}

#endif

