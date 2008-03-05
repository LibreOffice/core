/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DatabaseDataProvider.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:06:05 $
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

#include "DatabaseDataProvider.hxx"
#include "dbastrings.hrc"
#include "cppuhelper/implbase1.hxx"
#include <comphelper/types.hxx>
#include <connectivity/FValue.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>

#include <vector>
#include <list>

namespace dbaccess
{
using namespace ::com::sun::star;
//--------------------------------------------------------------------------
DatabaseDataProvider::DatabaseDataProvider(uno::Reference< uno::XComponentContext > const & context) :
    TDatabaseDataProvider(m_aMutex),
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >(
        context, static_cast< Implements >(
            IMPLEMENTS_PROPERTY_SET), uno::Sequence< ::rtl::OUString >()),
    m_xContext(context),
    m_pDataSource(NULL),
    m_CommandType(0),
    m_RowLimit(0),
    m_EscapeProcessing(sal_False)
{
}
void SAL_CALL DatabaseDataProvider::disposing()
{
    ::comphelper::disposeComponent(m_xRowSet);
    m_xActiveConnection.clear();
}
uno::Any DatabaseDataProvider::queryInterface(uno::Type const & type) throw (uno::RuntimeException)
{
    return TDatabaseDataProvider::queryInterface(type);
}

//------------------------------------------------------------------------------
rtl::OUString DatabaseDataProvider::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.chart2.data.DatabaseDataProvider");
}
// -------------------------------------------------------------------------
// XServiceInfo
::rtl::OUString SAL_CALL DatabaseDataProvider::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL DatabaseDataProvider::supportsService( const ::rtl::OUString& _rServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}
//------------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > DatabaseDataProvider::getSupportedServiceNames_Static(  ) throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2.data.DatabaseDataProvider"));
    return aSNS;
}
// -------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL DatabaseDataProvider::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > DatabaseDataProvider::Create(uno::Reference< uno::XComponentContext > const & context)
{
    return *(new DatabaseDataProvider(context)) ;
}
// lang::XInitialization:
void SAL_CALL DatabaseDataProvider::initialize(const uno::Sequence< uno::Any > & aArguments) throw (uno::RuntimeException, uno::Exception)
{
    osl::MutexGuard g(m_aMutex);
    const uno::Any* pIter = aArguments.getConstArray();
    const uno::Any* pEnd      = pIter + aArguments.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if ( !m_xActiveConnection.is() && ((*pIter) >>= m_xActiveConnection) )
            break;
    }
}

// chart2::data::XDataProvider:
::sal_Bool SAL_CALL DatabaseDataProvider::createDataSourcePossible(const uno::Sequence< beans::PropertyValue > & /*aArguments*/) throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    sal_Bool bRet = sal_False;
    if ( m_Command.getLength() != 0 && m_xActiveConnection.is() )
    {
        try
        {
            ::comphelper::disposeComponent(m_xRowSet);
            m_xRowSet.set( m_xContext->getServiceManager()->createInstanceWithContext(SERVICE_SDB_ROWSET,m_xContext ), uno::UNO_QUERY );
            uno::Reference< beans::XPropertySet> xRowSetProp( m_xRowSet, uno::UNO_QUERY_THROW );

            xRowSetProp->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, uno::makeAny( m_xActiveConnection ) );
            xRowSetProp->setPropertyValue( PROPERTY_APPLYFILTER, uno::makeAny( sal_True ) );
            xRowSetProp->setPropertyValue( PROPERTY_COMMAND_TYPE, uno::makeAny( m_CommandType ) );
            xRowSetProp->setPropertyValue( PROPERTY_COMMAND,     uno::makeAny( m_Command ) );

            uno::Reference<sdb::XCompletedExecution> xExecute( m_xRowSet, uno::UNO_QUERY );
            if ( xExecute.is() )
            {
                uno::Reference<task::XInteractionHandler> xHandler(
                                m_xContext->getServiceManager()->createInstanceWithContext(
                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.InteractionHandler"))
                                            ,m_xContext),
                                uno::UNO_QUERY);
                xExecute->executeWithCompletion(xHandler);
            }
            else
                m_xRowSet->execute();

            bRet = sal_True;
        }
        catch(const uno::Exception& e)
        {
            (void)e;
            OSL_ENSURE(0,"Exception caught!");
        }
    }
    else // no command set, use Internal data handler
    {
    }

    return bRet;
}
class DatabaseDataSource :    public ::cppu::WeakImplHelper1< chart2::data::XDataSource>
{
public:
    explicit DatabaseDataSource(uno::Reference< uno::XComponentContext > const & context,const uno::Reference< sdbc::XRowSet >& _xRowSet,sal_Int32 _nRowLimit);

    // chart2::data::XDataSource:
    virtual uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences() throw (uno::RuntimeException);

    inline ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence> getDataSequence(sal_Int32 _nPos)
        { impl_fill(); return (static_cast<sal_uInt32>(_nPos) < m_aDataValues.size() ? m_aDataValues[_nPos] : ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence>()); }

    inline ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence> getLabelSequence(sal_Int32 _nPos)
        { impl_fill(); return (static_cast<sal_uInt32>(_nPos) < m_aLabels.size() ? m_aLabels[_nPos] : ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence>()); }

private:
    DatabaseDataSource(DatabaseDataSource &); // not defined
    void operator =(DatabaseDataSource &); // not defined

    virtual ~DatabaseDataSource() {}
    void impl_fill();

    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence> >
                                                                                    m_aLabels;
    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence> >
                                                                                    m_aDataValues;
    ::std::vector< sal_Int32 >                                                      m_aColumnTypes;


    uno::Reference< uno::XComponentContext >    m_xContext;
    uno::Reference< sdbc::XRowSet >             m_xRowSet;
    sal_Int32                                   m_nRowLimit;
};

DatabaseDataSource::DatabaseDataSource(uno::Reference< uno::XComponentContext > const & context,const uno::Reference< sdbc::XRowSet >& _xRowSet,sal_Int32 _nRowLimit) :
    m_xContext(context)
    ,m_xRowSet(_xRowSet)
    ,m_nRowLimit(_nRowLimit)
{}
void DatabaseDataSource::impl_fill()
{
    if ( m_aLabels.empty() && m_xRowSet.is() )
    {
        uno::Reference< sdbc::XResultSet> xRes(m_xRowSet,uno::UNO_QUERY);
        uno::Reference< sdbc::XRow> xRow(m_xRowSet,uno::UNO_QUERY);
        uno::Reference< sdbc::XResultSetMetaDataSupplier> xResMDSup(m_xRowSet,uno::UNO_QUERY);
        uno::Reference< sdbc::XResultSetMetaData> xResultSetMetaData = xResMDSup->getMetaData();


        uno::Sequence< uno::Any > aLabelArgs(1);
        uno::Sequence< ::rtl::OUString > aColumnLabelArgs(1);
        beans::NamedValue aParam;
        aParam.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataSequence"));
        const sal_Int32 nCount = xResultSetMetaData->getColumnCount();
        for (sal_Int32 i = 1; i <= nCount; ++i)
        {
            sal_Int32 nColumnType = xResultSetMetaData->getColumnType(i);
            m_aColumnTypes.push_back(nColumnType);
            aColumnLabelArgs[0] = xResultSetMetaData->getColumnName(i);
            aParam.Value <<= aColumnLabelArgs;
            aLabelArgs[0] <<= aParam;
            m_aLabels.push_back( uno::Reference< chart2::data::XDataSequence>(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.chart.CachedDataSequence")),aLabelArgs,m_xContext ), uno::UNO_QUERY ));
        }

        ::std::vector< ::std::vector< ::rtl::OUString > > aDataValues(nCount);
        sal_Int32 nRowCount = 0;
        while( xRes->next() && (!m_nRowLimit || nRowCount < m_nRowLimit) )
        {
            ++nRowCount;
            ::std::vector< sal_Int32 >::iterator aColumnTypeIter = m_aColumnTypes.begin();
            for (sal_Int32 j = 1; j <= nCount; ++j,++aColumnTypeIter)
                aDataValues[j-1].push_back(xRow->getString(j));
        }
        ::std::vector< ::std::vector< ::rtl::OUString> >::iterator aDataValuesIter = aDataValues.begin();
        const ::std::vector< ::std::vector< ::rtl::OUString > >::iterator aDataValuesEnd = aDataValues.end();
        for (;aDataValuesIter != aDataValuesEnd ; ++aDataValuesIter)
        {
            aParam.Value <<= uno::Sequence< ::rtl::OUString >(&(*aDataValuesIter->begin()),aDataValuesIter->size());
            aLabelArgs[0] <<= aParam;
            m_aDataValues.push_back(
                uno::Reference< chart2::data::XDataSequence>(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.chart.CachedDataSequence")),aLabelArgs,m_xContext ), uno::UNO_QUERY )
                );
        }
    }
}
// chart2::data::XDataSource:
uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > SAL_CALL DatabaseDataSource::getDataSequences() throw (uno::RuntimeException)
{
    ::std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aSeq;

    impl_fill();

    ::std::vector< uno::Reference< chart2::data::XDataSequence> >::iterator aDataValuesIter = m_aDataValues.begin();
    ::std::vector< uno::Reference< chart2::data::XDataSequence> >::iterator aIter = m_aLabels.begin();
    const ::std::vector< uno::Reference< chart2::data::XDataSequence> >::iterator aEnd  = m_aLabels.end();
    for (;aIter != aEnd ; ++aIter,++aDataValuesIter)
    {
        uno::Reference< chart2::data::XLabeledDataSequence > xLabeled(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.chart2.LabeledDataSequence")),m_xContext ), uno::UNO_QUERY );
        xLabeled->setLabel(*aIter);
        xLabeled->setValues(*aDataValuesIter);
        aSeq.push_back(xLabeled);
    }


    return aSeq.empty()
        ? uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > ()
        : uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > (&(*aSeq.begin()),aSeq.size());
}
// -----------------------------------------------------------------------------
uno::Reference< chart2::data::XDataSource > SAL_CALL DatabaseDataProvider::createDataSource(const uno::Sequence< beans::PropertyValue > & aArguments) throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    osl::MutexGuard g(m_aMutex);
    if ( !m_pDataSource && createDataSourcePossible(aArguments))
    {
        m_pDataSource = new DatabaseDataSource(m_xContext,m_xRowSet,m_RowLimit);
        m_xDataSource = m_pDataSource;
    }
    return m_xDataSource;
}

uno::Sequence< beans::PropertyValue > SAL_CALL DatabaseDataProvider::detectArguments(const uno::Reference< chart2::data::XDataSource > & /*xDataSource*/) throw (uno::RuntimeException)
{
    uno::Sequence< beans::PropertyValue > aArguments( 4 );
    aArguments[0] = beans::PropertyValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CellRangeRepresentation")), -1, uno::Any(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("all")) ),
        beans::PropertyState_DIRECT_VALUE );
    aArguments[1] = beans::PropertyValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataRowSource")), -1, uno::makeAny( chart::ChartDataRowSource_COLUMNS ),
        beans::PropertyState_DIRECT_VALUE );
    // internal data always contains labels and categories
    aArguments[2] = beans::PropertyValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FirstCellAsLabel")), -1, uno::makeAny( false ), beans::PropertyState_DIRECT_VALUE );
    aArguments[3] = beans::PropertyValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HasCategories")), -1, uno::makeAny( true ), beans::PropertyState_DIRECT_VALUE );
    return aArguments;
}

::sal_Bool SAL_CALL DatabaseDataProvider::createDataSequenceByRangeRepresentationPossible(const ::rtl::OUString & /*aRangeRepresentation*/) throw (uno::RuntimeException)
{
    return sal_True;
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL DatabaseDataProvider::createDataSequenceByRangeRepresentation(const ::rtl::OUString & _sRangeRepresentation) throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    osl::MutexGuard g(m_aMutex);
    uno::Reference< chart2::data::XDataSequence > xRet;
    createDataSource(uno::Sequence< beans::PropertyValue >());
    if ( m_pDataSource )
    {
        if ( _sRangeRepresentation.equalsAscii("categories"))
        {
            xRet = m_pDataSource->getDataSequence(0);
        }
        else if ( _sRangeRepresentation.equalsAscii("label") )
        {
            sal_Int32 nIndex = _sRangeRepresentation.copy( 6 ).toInt32();
            xRet = m_pDataSource->getLabelSequence(nIndex);
        }
        else
            xRet = m_pDataSource->getDataSequence(1);
    }
    else
    {
        xRet.set(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.chart.CachedDataSequence")),m_xContext ), uno::UNO_QUERY );
    }
    return xRet;
}

uno::Reference< sheet::XRangeSelection > SAL_CALL DatabaseDataProvider::getRangeSelection() throw (uno::RuntimeException)
{
    // TODO: Exchange the default return implementation for "getRangeSelection" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return uno::Reference< sheet::XRangeSelection >();
}

// chart2::data::XRangeXMLConversion:
::rtl::OUString SAL_CALL DatabaseDataProvider::convertRangeToXML(const ::rtl::OUString & _sRangeRepresentation) throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    ::rtl::OUStringBuffer sBuffer;
    sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("local-table."));
    if ( _sRangeRepresentation.equalsAscii("categories") )
    {
        sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("$A2:$A100"));
    }
    else if ( _sRangeRepresentation.equalsAscii("label") )
        sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("$A1:$F1"));
    else if ( _sRangeRepresentation.equalsAscii("all") )
        sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("$A1:$F100"));
    else
    {
        sal_Int32 nCol = _sRangeRepresentation.toInt32();

        sBuffer.append( (sal_Unicode)'$' );
        // get A, B, C, ..., AA, AB, ... representation of column number
        if( nCol < 26 )
            sBuffer.append( (sal_Unicode)('A' + nCol) );
        else if( nCol < 702 )
        {
            sBuffer.append( (sal_Unicode)('A' + nCol / 26 - 1 ));
            sBuffer.append( (sal_Unicode)('A' + nCol % 26) );
        }
        else    // works for nCol <= 18,278
        {
            sBuffer.append( (sal_Unicode)('A' + nCol / 702 - 1 ));
            sBuffer.append( (sal_Unicode)('A' + (nCol % 702) / 26 ));
            sBuffer.append( (sal_Unicode)('A' + nCol % 26) );
        }
        sBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("2:$F100") );
    }
    return sBuffer.makeStringAndClear();
}

::rtl::OUString SAL_CALL DatabaseDataProvider::convertRangeFromXML(const ::rtl::OUString & _sXMLRange) throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    ::rtl::OUStringBuffer sBuffer;
    if ( _sXMLRange.equalsAscii("local-table.$A2:$A100") )
        sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("categories"));
    else if ( _sXMLRange.equalsAscii("local-table.$A1:$F1") )
        sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("label"));
    else if ( _sXMLRange.equalsAscii("local-table.$A1:$F100") )
        sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("all"));
    else
        sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("data"));
    return sBuffer.makeStringAndClear();
}

// com.sun.star.beans.XPropertySet:
uno::Reference< beans::XPropertySetInfo > SAL_CALL DatabaseDataProvider::getPropertySetInfo() throw (uno::RuntimeException)
{
    return ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::getPropertySetInfo();
}

void SAL_CALL DatabaseDataProvider::setPropertyValue(const ::rtl::OUString & aPropertyName, const uno::Any & aValue) throw (uno::RuntimeException, beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::setPropertyValue(aPropertyName, aValue);
}

uno::Any SAL_CALL DatabaseDataProvider::getPropertyValue(const ::rtl::OUString & aPropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException)
{
    return ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::getPropertyValue(aPropertyName);
}

void SAL_CALL DatabaseDataProvider::addPropertyChangeListener(const ::rtl::OUString & aPropertyName, const uno::Reference< beans::XPropertyChangeListener > & xListener) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::addPropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL DatabaseDataProvider::removePropertyChangeListener(const ::rtl::OUString & aPropertyName, const uno::Reference< beans::XPropertyChangeListener > & xListener) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::removePropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL DatabaseDataProvider::addVetoableChangeListener(const ::rtl::OUString & aPropertyName, const uno::Reference< beans::XVetoableChangeListener > & xListener) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::addVetoableChangeListener(aPropertyName, xListener);
}

void SAL_CALL DatabaseDataProvider::removeVetoableChangeListener(const ::rtl::OUString & aPropertyName, const uno::Reference< beans::XVetoableChangeListener > & xListener) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::removeVetoableChangeListener(aPropertyName, xListener);
}

// chart2::data::XDatabaseDataProvider:
uno::Sequence< ::rtl::OUString > SAL_CALL DatabaseDataProvider::getMasterFields() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_MasterFields;
}

void SAL_CALL DatabaseDataProvider::setMasterFields(const uno::Sequence< ::rtl::OUString > & the_value) throw (uno::RuntimeException)
{
    set(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MasterFields")),the_value,m_MasterFields);
}

uno::Sequence< ::rtl::OUString > SAL_CALL DatabaseDataProvider::getDetailFields() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_DetailFields;
}

void SAL_CALL DatabaseDataProvider::setDetailFields(const uno::Sequence< ::rtl::OUString > & the_value) throw (uno::RuntimeException)
{
    set(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DetailFields")),the_value,m_DetailFields);
}
::rtl::OUString SAL_CALL DatabaseDataProvider::getCommand() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_Command;
}

void SAL_CALL DatabaseDataProvider::setCommand(const ::rtl::OUString & the_value) throw (uno::RuntimeException)
{
    if ( the_value != m_Command )
    {
        osl::MutexGuard g(m_aMutex);
        m_pDataSource = NULL;
        ::comphelper::disposeComponent(m_xDataSource);
    }
    set(PROPERTY_COMMAND,the_value,m_Command);

}

::sal_Int32 SAL_CALL DatabaseDataProvider::getCommandType() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_CommandType;
}

void SAL_CALL DatabaseDataProvider::setCommandType(::sal_Int32 the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_COMMAND_TYPE,the_value,m_CommandType);
}

::rtl::OUString SAL_CALL DatabaseDataProvider::getFilter() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_Filter;
}

void SAL_CALL DatabaseDataProvider::setFilter(const ::rtl::OUString & the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_FILTER,the_value,m_Filter);
}

::sal_Bool SAL_CALL DatabaseDataProvider::getEscapeProcessing() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_EscapeProcessing;
}

void SAL_CALL DatabaseDataProvider::setEscapeProcessing(::sal_Bool the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_ESCAPE_PROCESSING,the_value,m_EscapeProcessing);
}
::sal_Int32 SAL_CALL DatabaseDataProvider::getRowLimit() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_RowLimit;
}

void SAL_CALL DatabaseDataProvider::setRowLimit(::sal_Int32 the_value) throw (uno::RuntimeException)
{
    set(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RowLimit")),the_value,m_RowLimit);
}
uno::Reference< sdbc::XConnection > SAL_CALL DatabaseDataProvider::getActiveConnection() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_xActiveConnection;
}

void SAL_CALL DatabaseDataProvider::setActiveConnection(const uno::Reference< sdbc::XConnection > & the_value) throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    if ( !the_value.is() )
        throw lang::IllegalArgumentException();
    set(PROPERTY_ACTIVE_CONNECTION,the_value,m_xActiveConnection);
}
::rtl::OUString SAL_CALL DatabaseDataProvider::getDataSourceName() throw (uno::RuntimeException)
{
    osl::MutexGuard g(m_aMutex);
    return m_DataSourceName;
}

void SAL_CALL DatabaseDataProvider::setDataSourceName(const ::rtl::OUString& the_value) throw (uno::RuntimeException)
{
    set(PROPERTY_DATASOURCENAME,the_value,m_DataSourceName);
}


} // namespace dbaccess
