/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InternalData.cxx,v $
 * $Revision: 1.0 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "InternalData.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include "macros.hxx"

#include <rtl/math.hxx>

using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

using namespace ::std;

namespace chart
{

// ----------------------------------------
namespace
{
struct lcl_NumberedStringGenerator
{
    lcl_NumberedStringGenerator( const OUString & rStub, const OUString & rWildcard ) :
            m_aStub( rStub ),
            m_nCounter( 0 ),
            m_nStubStartIndex( rStub.indexOf( rWildcard )),
            m_nWildcardLength( rWildcard.getLength())
    {
    }
    OUString operator()() {
        return m_aStub.replaceAt( m_nStubStartIndex, m_nWildcardLength, OUString::valueOf( ++m_nCounter ));
    }
private:
    OUString m_aStub;
    sal_Int32 m_nCounter;
    const sal_Int32 m_nStubStartIndex;
    const sal_Int32 m_nWildcardLength;
};

template< typename T >
    Sequence< T > lcl_ValarrayToSequence( const ::std::valarray< T > & rValarray )
{
    // is there a more elegant way of conversion?
    Sequence< T > aResult( rValarray.size());
    for( size_t i = 0; i < rValarray.size(); ++i )
        aResult[i] = rValarray[i];
    return aResult;
}

} // anonymous namespace
// ----------------------------------------

InternalData::InternalData() :
        m_nColumnCount( 0 ),
        m_nRowCount( 0 )
{}

void InternalData::createDefaultData()
{
    const sal_Int32 nNumRows = 4;
    const sal_Int32 nNumColumns = 3;
    const sal_Int32 nSize = nNumColumns * nNumRows;
    // @todo: localize this!
    const OUString aRowName( ::chart::SchResId::getResString( STR_ROW_LABEL ));
    const OUString aColName( ::chart::SchResId::getResString( STR_COLUMN_LABEL ));

    const double fDefaultData[ nSize ] =
        { 9.10, 3.20, 4.54,
          2.40, 8.80, 9.65,
          3.10, 1.50, 3.70,
          4.30, 9.02, 6.20 };

    m_aData.resize( nSize );
    for( sal_Int32 i=0; i<nSize; ++i )
        m_aData[i] = fDefaultData[i];
    m_nRowCount = nNumRows;
    m_nColumnCount = nNumColumns;

    vector< OUString > aRowLabels;
    aRowLabels.reserve( nNumRows );
    generate_n( back_inserter( aRowLabels ), nNumRows,
                lcl_NumberedStringGenerator( aRowName, C2U("%ROWNUMBER") ));
    setRowLabels( aRowLabels );

    vector< OUString > aColumnLabels;
    aColumnLabels.reserve( nNumColumns );
    generate_n( back_inserter( aColumnLabels ), nNumColumns,
                lcl_NumberedStringGenerator( aColName, C2U("%COLUMNNUMBER") ));
    setColumnLabels( aColumnLabels );
}

void InternalData::setData( const Sequence< Sequence< double > >& rDataInRows )
{
    m_nRowCount = rDataInRows.getLength();
    m_nColumnCount = (m_nRowCount ? rDataInRows[0].getLength() : 0);

    if( m_aRowLabels.size() != static_cast< sal_uInt32 >( m_nRowCount ))
        m_aRowLabels.resize( m_nRowCount );
    if( m_aColumnLabels.size() != static_cast< sal_uInt32 >( m_nColumnCount ))
        m_aColumnLabels.resize( m_nColumnCount );

    m_aData.resize( m_nRowCount * m_nColumnCount );
    double fNan;
    ::rtl::math::setNan( & fNan );
    // set all values to Nan
    m_aData = fNan;

    for( sal_Int32 nRow=0; nRow<m_nRowCount; ++nRow )
    {
        int nDataIdx = nRow*m_nColumnCount;
        const sal_Int32 nMax = ::std::min( rDataInRows[nRow].getLength(), m_nColumnCount );
        for( sal_Int32 nCol=0; nCol < nMax; ++nCol )
        {
            m_aData[nDataIdx] = rDataInRows[nRow][nCol];
            nDataIdx += 1;
        }
    }
}

Sequence< Sequence< double > > InternalData::getData() const
{
    Sequence< Sequence< double > > aResult( m_nRowCount );

    for( sal_Int32 i=0; i<m_nRowCount; ++i )
        aResult[i] = lcl_ValarrayToSequence< tDataType::value_type >(
            m_aData[ ::std::slice( i*m_nColumnCount, m_nColumnCount, 1 ) ] );

    return aResult;
}

Sequence< double > InternalData::getColumnValues( sal_Int32 nColumnIndex ) const
{
    if( nColumnIndex >= 0 && nColumnIndex < m_nColumnCount )
        return lcl_ValarrayToSequence< tDataType::value_type >(
            m_aData[ ::std::slice( nColumnIndex, m_nRowCount, m_nColumnCount ) ] );
    return Sequence< double >();
}
Sequence< double > InternalData::getRowValues( sal_Int32 nRowIndex ) const
{
    if( nRowIndex >= 0 && nRowIndex < m_nRowCount )
        return lcl_ValarrayToSequence< tDataType::value_type >(
            m_aData[ ::std::slice( nRowIndex*m_nColumnCount, m_nColumnCount, 1 ) ] );
    return Sequence< double >();
}

void InternalData::setColumnValues( sal_Int32 nColumnIndex, const ::std::vector< double > & rNewData )
{
    if( nColumnIndex >= 0 && nColumnIndex < m_nColumnCount )
    {
        tDataType aSlice = m_aData[ ::std::slice( nColumnIndex, m_nRowCount, m_nColumnCount ) ];
        for( ::std::vector< double >::size_type i = 0; i < rNewData.size(); ++i )
            aSlice[i] = rNewData[i];
        m_aData[ ::std::slice( nColumnIndex, m_nRowCount, m_nColumnCount ) ] = aSlice;
    }
}

void InternalData::setRowValues( sal_Int32 nRowIndex, const ::std::vector< double > & rNewData )
{
    if( nRowIndex >= 0 && nRowIndex < m_nRowCount )
    {
        tDataType aSlice = m_aData[ ::std::slice( nRowIndex*m_nColumnCount, m_nColumnCount, 1 ) ];
        for( ::std::vector< double >::size_type i = 0; i < rNewData.size(); ++i )
            aSlice[i] = rNewData[i];
        m_aData[ ::std::slice( nRowIndex*m_nColumnCount, m_nColumnCount, 1 ) ]= aSlice;
    }
}

void InternalData::swapRowWithNext( sal_Int32 nRowIndex )
{
    if( nRowIndex < m_nRowCount - 1 )
    {
        const sal_Int32 nMax = m_nColumnCount;
        for( sal_Int32 nColIdx=0; nColIdx<nMax; ++nColIdx )
        {
            size_t nIndex1 = nColIdx + nRowIndex*m_nColumnCount;
            size_t nIndex2 = nIndex1 + m_nColumnCount;
            double fTemp = m_aData[nIndex1];
            m_aData[nIndex1] = m_aData[nIndex2];
            m_aData[nIndex2] = fTemp;
        }
        OUString sTemp( m_aRowLabels[nRowIndex] );
        m_aRowLabels[nRowIndex] = m_aRowLabels[nRowIndex + 1];
        m_aRowLabels[nRowIndex + 1] = sTemp;
    }
}

void InternalData::swapColumnWithNext( sal_Int32 nColumnIndex )
{
    if( nColumnIndex < m_nColumnCount - 1 )
    {
        const sal_Int32 nMax = m_nRowCount;
        for( sal_Int32 nRowIdx=0; nRowIdx<nMax; ++nRowIdx )
        {
            size_t nIndex1 = nColumnIndex + nRowIdx*m_nColumnCount;
            size_t nIndex2 = nIndex1 + 1;
            double fTemp = m_aData[nIndex1];
            m_aData[nIndex1] = m_aData[nIndex2];
            m_aData[nIndex2] = fTemp;
        }
        OUString sTemp( m_aColumnLabels[nColumnIndex] );
        m_aColumnLabels[nColumnIndex] = m_aColumnLabels[nColumnIndex + 1];
        m_aColumnLabels[nColumnIndex + 1] = sTemp;
    }
}

bool InternalData::enlargeData( sal_Int32 nColumnCount, sal_Int32 nRowCount )
{
    sal_Int32 nNewColumnCount( ::std::max<sal_Int32>( m_nColumnCount, nColumnCount ) );
    sal_Int32 nNewRowCount( ::std::max<sal_Int32>( m_nRowCount, nRowCount ) );
    sal_Int32 nNewSize( nNewColumnCount*nNewRowCount );

    bool bGrow = (nNewSize > m_nColumnCount*m_nRowCount);

    if( bGrow )
    {
        double fNan;
        ::rtl::math::setNan( &fNan );
        tDataType aNewData( fNan, nNewSize );
        // copy old data
        for( int nCol=0; nCol<m_nColumnCount; ++nCol )
            static_cast< tDataType >(
                aNewData[ ::std::slice( nCol, m_nRowCount, nNewColumnCount ) ] ) =
                m_aData[ ::std::slice( nCol, m_nRowCount, m_nColumnCount ) ];

        m_aData.resize( nNewSize );
        m_aData = aNewData;
    }
    m_nColumnCount = nNewColumnCount;
    m_nRowCount = nNewRowCount;
    return bGrow;
}

void InternalData::insertColumn( sal_Int32 nAfterIndex )
{
    // note: -1 is allowed, as we insert after the given index
    OSL_ASSERT( nAfterIndex < m_nColumnCount && nAfterIndex >= -1 );
    if( nAfterIndex >= m_nColumnCount || nAfterIndex < -1 )
        return;
    sal_Int32 nNewColumnCount = m_nColumnCount + 1;
    sal_Int32 nNewSize( nNewColumnCount * m_nRowCount );

    double fNan;
    ::rtl::math::setNan( &fNan );
    tDataType aNewData( fNan, nNewSize );

    // copy old data
    int nCol=0;
    for( ; nCol<=nAfterIndex; ++nCol )
        aNewData[ ::std::slice( nCol, m_nRowCount, nNewColumnCount ) ] =
            static_cast< tDataType >(
                m_aData[ ::std::slice( nCol, m_nRowCount, m_nColumnCount ) ] );
    for( ++nCol; nCol<nNewColumnCount; ++nCol )
        aNewData[ ::std::slice( nCol, m_nRowCount, nNewColumnCount ) ] =
            static_cast< tDataType >(
                m_aData[ ::std::slice( nCol - 1, m_nRowCount, m_nColumnCount ) ] );

    m_nColumnCount = nNewColumnCount;
    m_aData.resize( nNewSize );
    m_aData = aNewData;

    // labels
    if( nAfterIndex < static_cast< sal_Int32 >( m_aColumnLabels.size()))
        m_aColumnLabels.insert( m_aColumnLabels.begin() + (nAfterIndex + 1), OUString());

#if OSL_DEBUG_LEVEL > 2
    traceData();
#endif
}

sal_Int32 InternalData::appendColumn()
{
    insertColumn( getColumnCount() - 1 );
    return getColumnCount() - 1;
}

sal_Int32 InternalData::appendRow()
{
    insertRow( getRowCount() - 1 );
    return getRowCount() - 1;
}

void InternalData::insertRow( sal_Int32 nAfterIndex )
{
    // note: -1 is allowed, as we insert after the given index
    OSL_ASSERT( nAfterIndex < m_nRowCount && nAfterIndex >= -1 );
    if( nAfterIndex >= m_nRowCount || nAfterIndex < -1 )
        return;
    sal_Int32 nNewRowCount = m_nRowCount + 1;
    sal_Int32 nNewSize( m_nColumnCount * nNewRowCount );

    double fNan;
    ::rtl::math::setNan( &fNan );
    tDataType aNewData( fNan, nNewSize );

    // copy old data
    sal_Int32 nIndex = nAfterIndex + 1;
    aNewData[ ::std::slice( 0, nIndex * m_nColumnCount, 1 ) ] =
        static_cast< tDataType >(
            m_aData[ ::std::slice( 0, nIndex * m_nColumnCount, 1 ) ] );

    if( nIndex < m_nRowCount )
    {
        sal_Int32 nRemainingCount = m_nColumnCount * (m_nRowCount - nIndex);
        aNewData[ ::std::slice( (nIndex + 1) * m_nColumnCount, nRemainingCount, 1 ) ] =
            static_cast< tDataType >(
                m_aData[ ::std::slice( nIndex * m_nColumnCount, nRemainingCount, 1 ) ] );
    }

    m_nRowCount = nNewRowCount;
    m_aData.resize( nNewSize );
    m_aData = aNewData;

    // labels
    if( nAfterIndex < static_cast< sal_Int32 >( m_aRowLabels.size()))
        m_aRowLabels.insert( m_aRowLabels.begin() + nIndex, OUString());

#if OSL_DEBUG_LEVEL > 2
    traceData();
#endif
}

void InternalData::deleteColumn( sal_Int32 nAtIndex )
{
    OSL_ASSERT( nAtIndex < m_nColumnCount && nAtIndex >= 0 );
    if( nAtIndex >= m_nColumnCount || m_nColumnCount < 1 || nAtIndex < 0 )
        return;
    sal_Int32 nNewColumnCount = m_nColumnCount - 1;
    sal_Int32 nNewSize( nNewColumnCount * m_nRowCount );

    double fNan;
    ::rtl::math::setNan( &fNan );
    tDataType aNewData( fNan, nNewSize );

    // copy old data
    int nCol=0;
    for( ; nCol<nAtIndex; ++nCol )
        aNewData[ ::std::slice( nCol, m_nRowCount, nNewColumnCount ) ] =
            static_cast< tDataType >(
                m_aData[ ::std::slice( nCol, m_nRowCount, m_nColumnCount ) ] );
    for( ; nCol<nNewColumnCount; ++nCol )
        aNewData[ ::std::slice( nCol, m_nRowCount, nNewColumnCount ) ] =
            static_cast< tDataType >(
                m_aData[ ::std::slice( nCol + 1, m_nRowCount, m_nColumnCount ) ] );

    m_nColumnCount = nNewColumnCount;
    m_aData.resize( nNewSize );
    m_aData = aNewData;

    // labels
    if( nAtIndex < static_cast< sal_Int32 >( m_aColumnLabels.size()))
        m_aColumnLabels.erase( m_aColumnLabels.begin() + nAtIndex );

#if OSL_DEBUG_LEVEL > 2
    traceData();
#endif
}

void InternalData::deleteRow( sal_Int32 nAtIndex )
{
    OSL_ASSERT( nAtIndex < m_nRowCount && nAtIndex >= 0 );
    if( nAtIndex >= m_nRowCount || m_nRowCount < 1 || nAtIndex < 0 )
        return;
    sal_Int32 nNewRowCount = m_nRowCount - 1;
    sal_Int32 nNewSize( m_nColumnCount * nNewRowCount );

    double fNan;
    ::rtl::math::setNan( &fNan );
    tDataType aNewData( fNan, nNewSize );

    // copy old data
    sal_Int32 nIndex = nAtIndex;
    if( nIndex )
        aNewData[ ::std::slice( 0, nIndex * m_nColumnCount, 1 ) ] =
            static_cast< tDataType >(
                m_aData[ ::std::slice( 0, nIndex * m_nColumnCount, 1 ) ] );

    if( nIndex < nNewRowCount )
    {
        sal_Int32 nRemainingCount = m_nColumnCount * (nNewRowCount - nIndex);
        aNewData[ ::std::slice( nIndex * m_nColumnCount, nRemainingCount, 1 ) ] =
            static_cast< tDataType >(
                m_aData[ ::std::slice( (nIndex + 1) * m_nColumnCount, nRemainingCount, 1 ) ] );
    }

    m_nRowCount = nNewRowCount;
    m_aData.resize( nNewSize );
    m_aData = aNewData;

    // labels
    if( nAtIndex < static_cast< sal_Int32 >( m_aRowLabels.size()))
        m_aRowLabels.erase( m_aRowLabels.begin() + nAtIndex );

#if OSL_DEBUG_LEVEL > 2
    traceData();
#endif
}

sal_Int32 InternalData::getRowCount() const
{
    return m_nRowCount;
}

sal_Int32 InternalData::getColumnCount() const
{
    return m_nColumnCount;
}

void InternalData::setRowLabels( const ::std::vector< OUString > & rNewRowLabels )
{
    m_aRowLabels = rNewRowLabels;
    if( m_aRowLabels.size() < static_cast< ::std::vector< OUString >::size_type >( m_nRowCount ))
        m_aRowLabels.resize( m_nRowCount );
    else
        enlargeData( 0, static_cast< sal_Int32 >( m_aRowLabels.size() ));
}

::std::vector< OUString > InternalData::getRowLabels() const
{
    return m_aRowLabels;
}

void InternalData::setColumnLabels( const ::std::vector< OUString > & rNewColumnLabels )
{
    m_aColumnLabels = rNewColumnLabels;
    if( m_aColumnLabels.size() < static_cast< ::std::vector< OUString >::size_type >( m_nColumnCount ))
        m_aColumnLabels.resize( m_nColumnCount );
    else
        enlargeData( static_cast< sal_Int32 >( m_aColumnLabels.size()), 0 );
}

::std::vector< OUString > InternalData::getColumnLabels() const
{
    return m_aColumnLabels;
}

#if OSL_DEBUG_LEVEL > 2
void InternalData::traceData() const
{
    OSL_TRACE( "InternalData: Data in rows\n" );

    for( sal_Int32 i=0; i<m_nRowCount; ++i )
    {
        tDataType aSlice( m_aData[ ::std::slice( i*m_nColumnCount, m_nColumnCount, 1 ) ] );
        for( sal_Int32 j=0; j<m_nColumnCount; ++j )
            OSL_TRACE( "%lf ", aSlice[j] );
        OSL_TRACE( "\n" );
    }
    OSL_TRACE( "\n" );
}
#endif

} //  namespace chart
