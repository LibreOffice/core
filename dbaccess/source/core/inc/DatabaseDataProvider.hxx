/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DatabaseDataProvider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:05:53 $
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
#ifndef DBA_DATABASEDATAPROVIDER_HXX_INCLUDED
#define DBA_DATABASEDATAPROVIDER_HXX_INCLUDED

#include "sal/config.h"

#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/chart2/data/XDatabaseDataProvider.hpp"
#include <com/sun/star/sdbc/XRowSet.hpp>

#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/basemutex.hxx"
#include "cppuhelper/propertysetmixin.hxx"
#include <cppuhelper/implementationentry.hxx>
#include <comphelper/sequence.hxx>

namespace dbaccess
{

class DatabaseDataSource;
typedef ::cppu::WeakComponentImplHelper2<    ::com::sun::star::chart2::data::XDatabaseDataProvider
                                          ,  ::com::sun::star::lang::XServiceInfo > TDatabaseDataProvider;

class DatabaseDataProvider: private ::cppu::BaseMutex,
                            public TDatabaseDataProvider,
                            public ::cppu::PropertySetMixin< ::com::sun::star::chart2::data::XDatabaseDataProvider >
{
public:
    explicit DatabaseDataProvider(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

private:
    // ::com::sun::star::uno::XInterface:
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(::com::sun::star::uno::Type const & type) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ()
        { TDatabaseDataProvider::acquire(); }
    virtual void SAL_CALL release() throw ()
        { TDatabaseDataProvider::release(); }

    // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::chart2::data::XDataProvider:
    virtual ::sal_Bool SAL_CALL createDataSourcePossible(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aArguments) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > SAL_CALL createDataSource(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aArguments) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL detectArguments(const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > & xDataSource) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(const ::rtl::OUString & aRangeRepresentation) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(const ::rtl::OUString & aRangeRepresentation) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XRangeSelection > SAL_CALL getRangeSelection() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::chart2::data::XRangeXMLConversion:
    virtual ::rtl::OUString SAL_CALL convertRangeToXML(const ::rtl::OUString & aRangeRepresentation) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException);
    virtual ::rtl::OUString SAL_CALL convertRangeFromXML(const ::rtl::OUString & aXMLRange) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException);

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception);

    // ::com::sun::star::beans::XPropertySet:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue(const ::rtl::OUString & aPropertyName, const ::com::sun::star::uno::Any & aValue) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const ::rtl::OUString & PropertyName) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL addPropertyChangeListener(const ::rtl::OUString & aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & xListener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL removePropertyChangeListener(const ::rtl::OUString & aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL addVetoableChangeListener(const ::rtl::OUString & PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL removeVetoableChangeListener(const ::rtl::OUString & PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException);

    // ::com::sun::star::chart2::data::XDatabaseDataProvider:
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getMasterFields() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMasterFields(const ::com::sun::star::uno::Sequence< ::rtl::OUString > & the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getDetailFields() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDetailFields(const ::com::sun::star::uno::Sequence< ::rtl::OUString > & the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCommand() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCommand(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCommandType() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCommandType(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFilter() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFilter(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getEscapeProcessing() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEscapeProcessing(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getRowLimit() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowLimit( ::sal_Int32 _rowlimit ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getActiveConnection() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActiveConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException);
    virtual ::rtl::OUString SAL_CALL getDataSourceName() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDataSourceName( const ::rtl::OUString& _datasourcename ) throw (::com::sun::star::uno::RuntimeException);
private:
    DatabaseDataProvider(DatabaseDataProvider &); // not defined
    void operator =(DatabaseDataProvider &); // not defined

    virtual ~DatabaseDataProvider() {}

    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void SAL_CALL disposing();

    template <typename T> void set(  const ::rtl::OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
    {
        BoundListeners l;
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
            _member = _Value;
        }
        l.notify();
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xActiveConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >             m_xRowSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > m_xDataSource;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >                              m_MasterFields;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >                              m_DetailFields;

    DatabaseDataSource* m_pDataSource;
    ::rtl::OUString     m_Command;
    ::rtl::OUString     m_DataSourceName;
    ::sal_Int32         m_CommandType;
    sal_Int32           m_RowLimit;
    ::rtl::OUString     m_Filter;
    ::sal_Bool          m_EscapeProcessing;
};

} // namespace dbaccess

#endif // DBA_DATABASEDATAPROVIDER_HXX_INCLUDED
