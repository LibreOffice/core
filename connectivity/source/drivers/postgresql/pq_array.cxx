#include <rtl/ustrbuf.hxx>

#include <com/sun/star/sdbc/XArray.hpp>
#include <com/sun/star/sdbc/DataType.hpp>


#include "pq_array.hxx"
#include "pq_statics.hxx"
#include "pq_sequenceresultset.hxx"

using rtl::OUString;

using com::sun::star::sdbc::SQLException;
using com::sun::star::uno::Any;

using com::sun::star::uno::Sequence;
namespace pq_sdbc_driver
{


::rtl::OUString Array::getBaseTypeName(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "varchar" ) );
}

sal_Int32 Array::getBaseType(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return  com::sun::star::sdbc::DataType::VARCHAR;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > Array::getArray(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return m_data;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > Array::getArrayAtIndex(
    sal_Int32 index,
    sal_Int32 count,
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    checkRange( index, count );
    return Sequence< Any > ( &m_data[index-1], count );
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > Array::getResultSet(
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return getResultSetAtIndex( 0 , m_data.getLength() , typeMap );
}

::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > Array::getResultSetAtIndex(
    sal_Int32 index,
    sal_Int32 count,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    checkRange( index, count );
    Sequence< Sequence< Any > > ret( count );

    for( int i = 0 ; i < count ; i ++ )
    {
        Sequence< Any > row( 2 );
        row[0] <<= (sal_Int32) ( i + index );
        row[1] = m_data[i+index-1];
        ret[i] = row;
    }

    return new SequenceResultSet(
        m_refMutex, m_owner, getStatics().resultSetArrayColumnNames, ret, m_tc );
}


void Array::checkRange( sal_Int32 index, sal_Int32 count )
{
    if( index >= 1 && index -1 + count <= m_data.getLength() )
        return;
    rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "Array::getArrayAtIndex(): allowed range for index + count " ) );
    buf.append( m_data.getLength() );
    buf.appendAscii( ", got " );
    buf.append( index );
    buf.appendAscii( " + " );
    buf.append( count );

    throw SQLException( buf.makeStringAndClear() , *this, rtl::OUString(), 1, Any());

}

}
