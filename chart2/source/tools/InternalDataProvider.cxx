/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InternalDataProvider.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 15:58:12 $
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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <valarray>

#include "InternalDataProvider.hxx"
#include "LabeledDataSequence.hxx"
#include "DataSource.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "XMLRangeHelper.hxx"
#include "ContainerHelper.hxx"
#include "CommonConverters.hxx"
#include "CommonFunctors.hxx"
#include "UncachedDataSequence.hxx"
#include "DataSourceHelper.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "ResId.hxx"
#include "Strings.hrc"

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace chart
{
namespace impl
{


struct OUStringBufferAppend : public unary_function< OUString, void >
{
    OUStringBufferAppend( OUStringBuffer & rBuffer, const OUString & rSeparator ) :
            m_rBuffer( rBuffer ),
            m_aSep( rSeparator )
    {}
    void operator() ( const OUString & rStr )
    {
        m_rBuffer.append( m_aSep );
        m_rBuffer.append( rStr );
    }
private:
    OUStringBuffer m_rBuffer;
    OUString       m_aSep;
};

OUString FlattenStringSequence( const Sequence< OUString > & aSeq )
{
    if( aSeq.getLength() == 0 )
        return OUString();
    OUStringBuffer aBuf( aSeq[0] );
    for_each( aSeq.getConstArray() + 1, aSeq.getConstArray() + aSeq.getLength(),
              OUStringBufferAppend( aBuf, OUString(RTL_CONSTASCII_USTRINGPARAM(" "))));
    return aBuf.makeStringAndClear();
}

class InternalData
{
public:
    InternalData();

    void createDefaultData();

    void setData( const Sequence< Sequence< double > > & rNewData, bool bDataInColumns );
    Sequence< Sequence< double > > getData( bool bDataInColumns ) const;
    Sequence< double > getDataAt( sal_Int32 nIndex, bool bDataInColumns ) const;
    void setDataAt( sal_Int32 nIndex, bool bDataInColumns, const ::std::vector< double > & rNewData );
    void swapAllDataAtIndexWithNext( sal_Int32 nAtIndex, bool bDataInColumns );

    /** resizes the data if at least one of the given dimensions is larger than
        before.  The data is never becoming smaller only larger.

        @return </TRUE>, if the data was enlarged
    */
    bool enlargeData( sal_Int32 nColumnCount, sal_Int32 nRowCount );

    void insertColumn( sal_Int32 nAfterIndex );
    void insertRow( sal_Int32 nAfterIndex );
    void deleteColumn( sal_Int32 nAtIndex );
    void deleteRow( sal_Int32 nAtIndex );

    /// @return the index of the newly appended column
    sal_Int32 appendColumn();
    /// @return the index of the newly appended row
    sal_Int32 appendRow();

    sal_Int32 getRowCount() const;
    sal_Int32 getColumnCount() const;

    void setRowLabels( const ::std::vector< OUString > & rNewRowLabels );
    ::std::vector< OUString > getRowLabels() const;
    void setColumnLabels( const ::std::vector< OUString > & rNewColumnLabels );
    ::std::vector< OUString > getColumnLabels() const;

    /** returns whether the data source was created by putting sequence contents
        into columns (true) or rows (false)
     */
    bool setDataByDataSource(
        const Reference< chart2::data::XDataSource > & xDataSource,
        const Sequence< beans::PropertyValue > & rArgs );

#if OSL_DEBUG_LEVEL > 2
    void traceData() const;
#endif

private:
    sal_Int32                                    m_nColumnCount;
    sal_Int32                                    m_nRowCount;

    typedef ::std::valarray< double > tDataType;
    typedef ::std::vector< OUString > tLabelType;

    tDataType    m_aData;
    tLabelType   m_aRowLabels;
    tLabelType   m_aColumnLabels;
};

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

struct lcl_ValuesOfLabeledSequence :
        public unary_function< Reference< chart2::data::XLabeledDataSequence >, Sequence< double > >
{
    Sequence< double > operator() ( const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
    {
        if( ! xLSeq.is())
            return Sequence< double >();
        return DataSequenceToDoubleSequence( xLSeq->getValues());
    }
};

struct lcl_LabelsOfLabeledSequence :
        public unary_function< Reference< chart2::data::XLabeledDataSequence >, Sequence< OUString > >
{
    Sequence< OUString > operator() ( const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
    {
        if( ! xLSeq.is())
            return Sequence< OUString >();
        return DataSequenceToStringSequence( xLSeq->getLabel());
    }
};

struct lcl_LabelOfLabeledSequence :
        public unary_function< Reference< chart2::data::XLabeledDataSequence >, OUString >
{
    OUString operator() ( const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
    {
        if( ! xLSeq.is())
            return OUString();
        return FlattenStringSequence( DataSequenceToStringSequence( xLSeq->getLabel()));
    }
};

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

void InternalData::setData( const Sequence< Sequence< double > > & rNewData, bool bDataInColumns )
{
    sal_Int32 nOuterSize = rNewData.getLength();
    sal_Int32 nInnerSize = (nOuterSize ? rNewData[0].getLength() : 0);

    m_nRowCount = (bDataInColumns ? nInnerSize : nOuterSize);
    m_nColumnCount = (bDataInColumns ? nOuterSize : nInnerSize);

    if( m_aRowLabels.size() != static_cast< sal_uInt32 >( m_nRowCount ))
        m_aRowLabels.resize( m_nRowCount );
    if( m_aColumnLabels.size() != static_cast< sal_uInt32 >( m_nColumnCount ))
        m_aColumnLabels.resize( m_nColumnCount );

    m_aData.resize( m_nRowCount * m_nColumnCount );
    double fNan;
    ::rtl::math::setNan( & fNan );
    // set all values to Nan
    m_aData = fNan;

    for( sal_Int32 nOuterIdx=0; nOuterIdx<nOuterSize; ++nOuterIdx )
    {
        int nDataIdx = (bDataInColumns ? nOuterIdx : nOuterIdx*nInnerSize);
        const sal_Int32 nMax = ::std::min( rNewData[nOuterIdx].getLength(), nInnerSize );
        sal_Int32 nInnerIdx=0;
        for( ; nInnerIdx < nMax; ++nInnerIdx )
        {
            m_aData[nDataIdx] = rNewData[nOuterIdx][nInnerIdx];
            nDataIdx += (bDataInColumns ? m_nColumnCount : 1);
        }
    }
}

Sequence< Sequence< double > > InternalData::getData( bool bDataInColumns ) const
{
    Sequence< Sequence< double > > aResult( bDataInColumns ? m_nColumnCount : m_nRowCount );

    if( bDataInColumns )
    {
        for( sal_Int32 i=0; i<m_nColumnCount; ++i )
            aResult[i] = lcl_ValarrayToSequence< tDataType::value_type >(
                m_aData[ ::std::slice( i, m_nRowCount, m_nColumnCount ) ] );
    }
    else
    {
        for( sal_Int32 i=0; i<m_nRowCount; ++i )
            aResult[i] = lcl_ValarrayToSequence< tDataType::value_type >(
                m_aData[ ::std::slice( i*m_nColumnCount, m_nColumnCount, 1 ) ] );
    }

    return aResult;
}

Sequence< double > InternalData::getDataAt( sal_Int32 nIndex, bool bDataInColumns ) const
{
    Sequence< double > aResult( bDataInColumns ? m_nRowCount : m_nColumnCount );

    if( bDataInColumns )
    {
        if( nIndex < m_nColumnCount )
            return lcl_ValarrayToSequence< tDataType::value_type >(
                m_aData[ ::std::slice( nIndex, m_nRowCount, m_nColumnCount ) ] );
    }
    else
    {
        if( nIndex < m_nRowCount )
            return lcl_ValarrayToSequence< tDataType::value_type >(
                m_aData[ ::std::slice( nIndex*m_nColumnCount, m_nColumnCount, 1 ) ] );
    }

    return Sequence< double >();
}

void InternalData::setDataAt( sal_Int32 nIndex, bool bDataInColumns, const ::std::vector< double > & rNewData )
{
    if( bDataInColumns )
    {
        if( nIndex < m_nColumnCount )
        {
            tDataType aSlice = m_aData[ ::std::slice( nIndex, m_nRowCount, m_nColumnCount ) ];
            for( ::std::vector< double >::size_type i = 0; i < rNewData.size(); ++i )
                aSlice[i] = rNewData[i];
            m_aData[ ::std::slice( nIndex, m_nRowCount, m_nColumnCount ) ] = aSlice;
        }
    }
    else
    {
        if( nIndex < m_nRowCount )
        {
            tDataType aSlice = m_aData[ ::std::slice( nIndex*m_nColumnCount, m_nColumnCount, 1 ) ];
            for( ::std::vector< double >::size_type i = 0; i < rNewData.size(); ++i )
                aSlice[i] = rNewData[i];
            m_aData[ ::std::slice( nIndex*m_nColumnCount, m_nColumnCount, 1 ) ]= aSlice;
        }
    }
}

void InternalData::swapAllDataAtIndexWithNext( sal_Int32 nAtIndex, bool bDataInColumns )
{
    if( bDataInColumns && nAtIndex < m_nRowCount - 1 )
    {
        const sal_Int32 nMax = m_nColumnCount;
        for( sal_Int32 nColIdx=0; nColIdx<nMax; ++nColIdx )
        {
            size_t nIndex1 = nColIdx + nAtIndex*m_nColumnCount;
            size_t nIndex2 = nIndex1 + m_nColumnCount;
            double fTemp = m_aData[nIndex1];
            m_aData[nIndex1] = m_aData[nIndex2];
            m_aData[nIndex2] = fTemp;
        }
        OUString sTemp( m_aRowLabels[nAtIndex] );
        m_aRowLabels[nAtIndex] = m_aRowLabels[nAtIndex + 1];
        m_aRowLabels[nAtIndex + 1] = sTemp;
    }
    else if( nAtIndex < m_nColumnCount - 1 )
    {
        const sal_Int32 nMax = m_nRowCount;
        for( sal_Int32 nRowIdx=0; nRowIdx<nMax; ++nRowIdx )
        {
            size_t nIndex1 = nAtIndex + nRowIdx*m_nColumnCount;
            size_t nIndex2 = nIndex1 + 1;
            double fTemp = m_aData[nIndex1];
            m_aData[nIndex1] = m_aData[nIndex2];
            m_aData[nIndex2] = fTemp;
        }
        OUString sTemp( m_aColumnLabels[nAtIndex] );
        m_aColumnLabels[nAtIndex] = m_aColumnLabels[nAtIndex + 1];
        m_aColumnLabels[nAtIndex + 1] = sTemp;
    }
}

bool InternalData::enlargeData( sal_Int32 nColumnCount, sal_Int32 nRowCount )
{
    sal_Int32 nNewColumnCount( ::std::max( m_nColumnCount, nColumnCount ));
    sal_Int32 nNewRowCount( ::std::max( m_nRowCount, nRowCount ));
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
        m_nColumnCount = nNewColumnCount;
        m_nRowCount = nNewRowCount;
    }
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

bool InternalData::setDataByDataSource(
    const Reference< chart2::data::XDataSource > & xDataSource,
    const Sequence< beans::PropertyValue > & rArgs )
{
    OUString aRangeRepresentation;
    uno::Sequence< sal_Int32 > aSequenceMapping; //yyyy todo...? InternalData::setDataByDataSource
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;

    DataSourceHelper::readArguments( rArgs, aRangeRepresentation, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories );

    typedef ::std::vector< Reference< chart2::data::XLabeledDataSequence > > tLSeqCntType;
    tLSeqCntType aLSeqVec( ContainerHelper::SequenceToVector( xDataSource->getDataSequences()));
    tLSeqCntType::const_iterator aIt( aLSeqVec.begin());
    const tLSeqCntType::const_iterator aEndIt( aLSeqVec.end());

    if( bHasCategories && aIt != aEndIt )
    {
        if( bUseColumns )
            setRowLabels( ContainerHelper::SequenceToVector(
                              DataSequenceToStringSequence( (*aIt)->getValues() )));
        else
            setColumnLabels( ContainerHelper::SequenceToVector(
                                 DataSequenceToStringSequence( (*aIt)->getValues() )));
        ++aIt;
    }

    ::std::vector< Sequence< double > > aDataVec;
    ::std::vector< OUString > aLabelVec;
    transform( aIt, aEndIt, back_inserter( aDataVec ),  lcl_ValuesOfLabeledSequence());
    transform( aIt, aEndIt, back_inserter( aLabelVec ), lcl_LabelOfLabeledSequence());

    setData( ContainerHelper::ContainerToSequence( aDataVec ), bUseColumns );

    if( bUseColumns )
        setColumnLabels( aLabelVec );
    else
        setRowLabels( aLabelVec );

    return bUseColumns;
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

} // namespace impl

// ================================================================================

namespace
{

// note: in xmloff this name is used to indicate usage of own data
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.InternalDataProvider" ));

static const ::rtl::OUString lcl_aCategoriesRangeName(
    RTL_CONSTASCII_USTRINGPARAM( "categories" ));
static const ::rtl::OUString lcl_aCategoriesRoleName(
    RTL_CONSTASCII_USTRINGPARAM( "categories" ));
static const ::rtl::OUString lcl_aLabelRangePrefix(
    RTL_CONSTASCII_USTRINGPARAM( "label " ));
static const ::rtl::OUString lcl_aCompleteRange(
    RTL_CONSTASCII_USTRINGPARAM( "all" ));


struct lcl_DataProviderRangeCreator : public unary_function< OUString, Reference< chart2::data::XLabeledDataSequence > >
{
    lcl_DataProviderRangeCreator( const Reference< chart2::data::XDataProvider > & xDataProvider ) :
            m_xDataProvider( xDataProvider )
    {}

    Reference< chart2::data::XLabeledDataSequence > operator() ( const OUString & rRange )
    {
        Reference< chart2::data::XLabeledDataSequence > xResult;
        if( m_xDataProvider.is())
            try
            {
                xResult.set( new ::chart::LabeledDataSequence(
                                 m_xDataProvider->createDataSequenceByRangeRepresentation( rRange )));
            }
            catch( const lang::IllegalArgumentException & ex )
            {
                // data provider cannot create single data sequences, but then
                // detectArguments should work also with an empty data source
                (void)(ex);
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        return xResult;
    }

private:
    Reference< chart2::data::XDataProvider > m_xDataProvider;
};

typedef ::std::multimap< OUString, uno::WeakReference< chart2::data::XDataSequence > >
    lcl_tSequenceMap;

struct lcl_modifySeqMapValue : public ::std::unary_function< lcl_tSequenceMap, void >
{
    void operator() ( const lcl_tSequenceMap::value_type & rMapEntry )
    {
        // convert weak reference to reference
        Reference< chart2::data::XDataSequence > xSeq( rMapEntry.second );
        if( xSeq.is())
        {
            Reference< util::XModifiable > xMod( xSeq, uno::UNO_QUERY );
            if( xMod.is())
                xMod->setModified( sal_True );
        }
    }
};

Sequence< Reference< chart2::data::XLabeledDataSequence > >
    lcl_internalizeData(
        const Sequence< Reference< chart2::data::XLabeledDataSequence > > & rDataSeq,
        impl::InternalData & rInternalData,
        InternalDataProvider & rProvider )
{
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aResult( rDataSeq.getLength());
    for( sal_Int32 i=0; i<rDataSeq.getLength(); ++i )
    {
        sal_Int32 nNewIndex( rInternalData.appendColumn());
        OUString aIdentifier( OUString::valueOf( nNewIndex ));
        //@todo: deal also with genericXDataSequence
        Reference< chart2::data::XNumericalDataSequence > xValues( rDataSeq[i]->getValues(), uno::UNO_QUERY );
        Reference< chart2::data::XTextualDataSequence > xLabel( rDataSeq[i]->getLabel(), uno::UNO_QUERY );
        Reference< chart2::data::XDataSequence > xNewValues;

        if( xValues.is())
        {
            ::std::vector< double > aValues( ContainerHelper::SequenceToVector( xValues->getNumericalData()));
            rInternalData.enlargeData( nNewIndex + 1, aValues.size());
            rInternalData.setDataAt( nNewIndex, true, aValues );
            xNewValues.set( rProvider.createDataSequenceByRangeRepresentation( aIdentifier ));
            PropertyHelper::copyProperties(
                Reference< beans::XPropertySet >( xValues, uno::UNO_QUERY ),
                Reference< beans::XPropertySet >( xNewValues, uno::UNO_QUERY ));
        }

        if( xLabel.is())
        {
            ::std::vector< OUString > aLabels( rInternalData.getColumnLabels());
            OSL_ASSERT( static_cast< size_t >( nNewIndex ) < aLabels.size());
            aLabels[nNewIndex] = impl::FlattenStringSequence( xLabel->getTextualData());
            rInternalData.setColumnLabels( aLabels );
            Reference< chart2::data::XDataSequence > xNewLabel(
                rProvider.createDataSequenceByRangeRepresentation( lcl_aLabelRangePrefix + aIdentifier ));
            PropertyHelper::copyProperties(
                Reference< beans::XPropertySet >( xLabel, uno::UNO_QUERY ),
                Reference< beans::XPropertySet >( xNewLabel, uno::UNO_QUERY ));
            aResult[i] =
                Reference< chart2::data::XLabeledDataSequence >(
                    new LabeledDataSequence( xNewValues, xNewLabel ));
        }
        else
        {
            aResult[i] =
                Reference< chart2::data::XLabeledDataSequence >(
                    new LabeledDataSequence( xNewValues ));
        }
    }
    return aResult;
}

struct lcl_internalizeSeries : public ::std::unary_function< Reference< chart2::XDataSeries >, void >
{
    lcl_internalizeSeries( impl::InternalData & rInternalData,
                           InternalDataProvider & rProvider ) :
            m_rInternalData( rInternalData ),
            m_rProvider( rProvider )
    {}
    void operator() ( const Reference< chart2::XDataSeries > & xSeries )
    {
        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        Reference< chart2::data::XDataSink >   xSink(   xSeries, uno::UNO_QUERY );
        if( xSource.is() && xSink.is())
            xSink->setData( lcl_internalizeData( xSource->getDataSequences(), m_rInternalData, m_rProvider ));
     }

private:
    impl::InternalData &    m_rInternalData;
    InternalDataProvider &  m_rProvider;
};

} // anonymous namespace

// ================================================================================

InternalDataProvider::InternalDataProvider() :
        m_bDataInColumns( true )
{}

InternalDataProvider::InternalDataProvider(
    const Reference< ::com::sun::star::chart::XChartDataArray > & xDataToCopy ) :
        m_bDataInColumns( true )
{
    if( xDataToCopy.is())
    {
        setData( xDataToCopy->getData() );
        setColumnDescriptions( xDataToCopy->getColumnDescriptions() );
        setRowDescriptions( xDataToCopy->getRowDescriptions() );
    }
}

InternalDataProvider::InternalDataProvider(
    const Reference< chart2::XChartDocument > & xChartDoc ) :
        m_bDataInColumns( true )
{
    try
    {
        Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartDoc ) );
        if( xDiagram.is())
        {
            impl::InternalData & rData( getInternalData());
            // categories
            Reference< chart2::data::XLabeledDataSequence > xCategories( DiagramHelper::getCategoriesFromDiagram( xDiagram ));
            if( xCategories.is())
            {
                // @todo: be able to deal with XDataSequence, too
                Reference< chart2::data::XTextualDataSequence > xSeq( xCategories->getValues(), uno::UNO_QUERY );
                if( xSeq.is())
                    rData.setRowLabels( ContainerHelper::SequenceToVector( xSeq->getTextualData()));
                DiagramHelper::setCategoriesToDiagram(
                    new LabeledDataSequence(
                        createDataSequenceByRangeRepresentation( lcl_aCategoriesRangeName )),
                    xDiagram );
            }

            // data series
            ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector( ChartModelHelper::getDataSeries( xChartDoc ));
            ::std::for_each( aSeriesVector.begin(), aSeriesVector.end(),
                             lcl_internalizeSeries( rData, *this ));

            // unused data
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aUnusedData( xDiagram->getUnusedData());
            aUnusedData = lcl_internalizeData( aUnusedData, rData, *this );
            xDiagram->setUnusedData( aUnusedData );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// copy-CTOR
InternalDataProvider::InternalDataProvider( const InternalDataProvider & rOther ) :
        impl::InternalDataProvider_Base(),
        m_aSequenceMap( rOther.m_aSequenceMap ),
        m_apData( new impl::InternalData( rOther.getInternalData())),
        m_bDataInColumns( rOther.m_bDataInColumns )
{}

InternalDataProvider::~InternalDataProvider()
{}

void InternalDataProvider::addDataSequenceToMap(
    const OUString & rRangeRepresentation,
    const Reference< chart2::data::XDataSequence > & xSequence )
{
    m_aSequenceMap.insert(
        tSequenceMap::value_type(
            rRangeRepresentation,
            uno::WeakReference< chart2::data::XDataSequence >( xSequence )));
}

void InternalDataProvider::deleteMapReferences( const OUString & rRangeRepresentation )
{
    // set sequence to deleted by setting its range to an empty string
    tSequenceMapRange aRange( m_aSequenceMap.equal_range( rRangeRepresentation ));
    for( tSequenceMap::iterator aIt( aRange.first ); aIt != aRange.second; ++aIt )
    {
        Reference< chart2::data::XDataSequence > xSeq( aIt->second );
        if( xSeq.is())
        {
            Reference< container::XNamed > xNamed( xSeq, uno::UNO_QUERY );
            if( xNamed.is())
                xNamed->setName( OUString());
        }
    }
    // remove from map
    m_aSequenceMap.erase( aRange.first, aRange.second );
}

void InternalDataProvider::adaptMapReferences(
    const OUString & rOldRangeRepresentation,
    const OUString & rNewRangeRepresentation )
{
    tSequenceMapRange aRange( m_aSequenceMap.equal_range( rOldRangeRepresentation ));
    tSequenceMap aNewElements;
    for( tSequenceMap::iterator aIt( aRange.first ); aIt != aRange.second; ++aIt )
    {
        Reference< chart2::data::XDataSequence > xSeq( aIt->second );
        if( xSeq.is())
        {
            Reference< container::XNamed > xNamed( xSeq, uno::UNO_QUERY );
            if( xNamed.is())
                xNamed->setName( rNewRangeRepresentation );
        }
        aNewElements.insert( tSequenceMap::value_type( rNewRangeRepresentation, aIt->second ));
    }
    // erase map values for old index
    m_aSequenceMap.erase( aRange.first, aRange.second );
    // add new entries for values with new index
    ::std::copy( aNewElements.begin(), aNewElements.end(),
                 ::std::inserter( m_aSequenceMap,
                                  m_aSequenceMap.upper_bound( rNewRangeRepresentation )));
}

void InternalDataProvider::increaseMapReferences(
    sal_Int32 nBegin, sal_Int32 nEnd )
{
    for( sal_Int32 nIndex = nEnd - 1; nIndex >= nBegin; --nIndex )
    {
        adaptMapReferences( OUString::valueOf( nIndex ),
                            OUString::valueOf( nIndex + 1 ));
        adaptMapReferences( lcl_aLabelRangePrefix + OUString::valueOf( nIndex ),
                            lcl_aLabelRangePrefix + OUString::valueOf( nIndex + 1 ));
    }
}

void InternalDataProvider::decreaseMapReferences(
    sal_Int32 nBegin, sal_Int32 nEnd )
{
    for( sal_Int32 nIndex = nBegin; nIndex < nEnd; ++nIndex )
    {
        adaptMapReferences( OUString::valueOf( nIndex ),
                            OUString::valueOf( nIndex - 1 ));
        adaptMapReferences( lcl_aLabelRangePrefix + OUString::valueOf( nIndex ),
                            lcl_aLabelRangePrefix + OUString::valueOf( nIndex - 1 ));
    }
}

Reference< chart2::data::XDataSequence > InternalDataProvider::createDataSequenceAndAddToMap(
    const OUString & rRangeRepresentation )
{
    Reference< chart2::data::XDataSequence > xSeq(
        new UncachedDataSequence( this, rRangeRepresentation ));
    addDataSequenceToMap( rRangeRepresentation, xSeq );
    return xSeq;
}

Reference< chart2::data::XDataSequence > InternalDataProvider::createDataSequenceAndAddToMap(
    const OUString & rRangeRepresentation,
    const OUString & rRole )
{
    Reference< chart2::data::XDataSequence > xSeq(
        new UncachedDataSequence( this, rRangeRepresentation, rRole ));
    addDataSequenceToMap( rRangeRepresentation, xSeq );
    return xSeq;
}

const impl::InternalData & InternalDataProvider::getInternalData() const
{
    if( m_apData.get())
        return *(m_apData.get());

    m_apData.reset( new impl::InternalData());
    return *(m_apData.get());
}

impl::InternalData & InternalDataProvider::getInternalData()
{
    if( m_apData.get())
        return *(m_apData.get());

    m_apData.reset( new impl::InternalData());
    return *(m_apData.get());
}

void InternalDataProvider::createDefaultData()
{
    getInternalData().createDefaultData();
}

// ____ XDataProvider ____
::sal_Bool SAL_CALL InternalDataProvider::createDataSourcePossible( const Sequence< beans::PropertyValue >& /* aArguments */ )
    throw (uno::RuntimeException)
{
    return true;
}

Reference< chart2::data::XDataSource > SAL_CALL InternalDataProvider::createDataSource(
    const Sequence< beans::PropertyValue >& aArguments )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    OUString aRangeRepresentation;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;
    DataSourceHelper::readArguments( aArguments, aRangeRepresentation, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories );

    OSL_ASSERT( aRangeRepresentation.equals( lcl_aCompleteRange ));

    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aResultLSeqVec;
    impl::InternalData & rData( getInternalData());

    // categories
    aResultLSeqVec.push_back(
        new LabeledDataSequence( createDataSequenceAndAddToMap( lcl_aCategoriesRangeName, lcl_aCategoriesRoleName )));

    // data with labels
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aDataVec;
    const sal_Int32 nCount = (bUseColumns ? rData.getColumnCount() : rData.getRowCount());
                           for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
    {
        aDataVec.push_back(
            new LabeledDataSequence(
                createDataSequenceAndAddToMap( OUString::valueOf( nIdx )),
                createDataSequenceAndAddToMap( lcl_aLabelRangePrefix + OUString::valueOf( nIdx ))));
    }

    // attention: this data provider has the limitation that it stores
    // internally if data comes from columns or rows. It is intended for
    // creating only one used data source.
    // @todo: add this information in the range representation strings
    m_bDataInColumns = bUseColumns;

    //reorder labeled sequences according to aSequenceMapping; ignore categories
    for( sal_Int32 nNewIndex = 0; nNewIndex < aSequenceMapping.getLength(); nNewIndex++ )
    {
        std::vector< LabeledDataSequence* >::size_type nOldIndex = aSequenceMapping[nNewIndex];
        if( nOldIndex < aDataVec.size() )
        {
            if( aDataVec[nOldIndex].is() )
            {
                aResultLSeqVec.push_back( aDataVec[nOldIndex] );
                aDataVec[nOldIndex] = 0;
            }
        }
    }

    //add left over data sequences to result
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::iterator aIt(aDataVec.begin());
    const ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::const_iterator aEndIt(aDataVec.end());
    for( ;aIt!=aEndIt; ++aIt)
    {
        if( aIt->is() )
            aResultLSeqVec.push_back( *aIt );
    }

    return new DataSource( ContainerHelper::ContainerToSequence(aResultLSeqVec) );
}

Sequence< beans::PropertyValue > SAL_CALL InternalDataProvider::detectArguments(
    const Reference< chart2::data::XDataSource >& /* xDataSource */ )
    throw (uno::RuntimeException)
{
    Sequence< beans::PropertyValue > aArguments( 4 );
    aArguments[0] = beans::PropertyValue(
        C2U("CellRangeRepresentation"), -1, uno::makeAny( lcl_aCompleteRange ),
        beans::PropertyState_DIRECT_VALUE );
    aArguments[1] = beans::PropertyValue(
        C2U("DataRowSource"), -1, uno::makeAny(
            m_bDataInColumns
            ? ::com::sun::star::chart::ChartDataRowSource_COLUMNS
            : ::com::sun::star::chart::ChartDataRowSource_ROWS ),
        beans::PropertyState_DIRECT_VALUE );
    // internal data always contains labels and categories
    aArguments[2] = beans::PropertyValue(
        C2U("FirstCellAsLabel"), -1, uno::makeAny( true ), beans::PropertyState_DIRECT_VALUE );
    aArguments[3] = beans::PropertyValue(
        C2U("HasCategories"), -1, uno::makeAny( true ), beans::PropertyState_DIRECT_VALUE );

    // #i85913# Sequence Mapping is not needed for internal data, as it is
    // applied to the data when the data source is created.

    return aArguments;
}

::sal_Bool SAL_CALL InternalDataProvider::createDataSequenceByRangeRepresentationPossible( const OUString& /* aRangeRepresentation */ )
    throw (uno::RuntimeException)
{
    return true;
}

Reference< chart2::data::XDataSequence > SAL_CALL InternalDataProvider::createDataSequenceByRangeRepresentation(
    const OUString& aRangeRepresentation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( aRangeRepresentation.equals( lcl_aCategoriesRangeName ))
    {
        // categories
        return createDataSequenceAndAddToMap( lcl_aCategoriesRangeName, lcl_aCategoriesRoleName );
    }
    else if( aRangeRepresentation.match( lcl_aLabelRangePrefix ))
    {
        // label
        sal_Int32 nIndex = aRangeRepresentation.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        return createDataSequenceAndAddToMap( lcl_aLabelRangePrefix + OUString::valueOf( nIndex ));
    }
    else if( aRangeRepresentation.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "last" )))
    {
        sal_Int32 nIndex = (m_bDataInColumns
                            ? getInternalData().getColumnCount()
                            : getInternalData().getRowCount()) - 1;
        return createDataSequenceAndAddToMap( OUString::valueOf( nIndex ));
    }
    else if( aRangeRepresentation.getLength())
    {
        // data
        sal_Int32 nIndex = aRangeRepresentation.toInt32();
        return createDataSequenceAndAddToMap( OUString::valueOf( nIndex ));
    }

    return Reference< chart2::data::XDataSequence >();
}

Reference< sheet::XRangeSelection > SAL_CALL InternalDataProvider::getRangeSelection()
    throw (uno::RuntimeException)
{
    // there is no range selection component
    return Reference< sheet::XRangeSelection >();
}

// ____ XInternalDataProvider ____
::sal_Bool SAL_CALL InternalDataProvider::hasDataByRangeRepresentation( const OUString& aRange )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = false;
    const impl::InternalData & rData( getInternalData());

    if( aRange.equals( lcl_aCategoriesRangeName ))
    {
        bResult = true;
    }
    else if( aRange.match( lcl_aLabelRangePrefix ))
    {
        sal_Int32 nIndex = aRange.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        bResult = (nIndex < (m_bDataInColumns ? rData.getColumnCount(): rData.getRowCount()));
    }
    else
    {
        sal_Int32 nIndex = aRange.toInt32();
        bResult = (nIndex < (m_bDataInColumns ? rData.getColumnCount(): rData.getRowCount()));
    }

    return bResult;
}

Sequence< uno::Any > SAL_CALL InternalDataProvider::getDataByRangeRepresentation( const OUString& aRange )
    throw (uno::RuntimeException)
{
    Sequence< uno::Any > aResult;
    const impl::InternalData & rData( getInternalData());

    if( aRange.equals( lcl_aCategoriesRangeName ))
    {
        vector< OUString > aCategories( m_bDataInColumns ? rData.getRowLabels() : rData.getColumnLabels());
        aResult.realloc( aCategories.size());
        transform( aCategories.begin(), aCategories.end(),
                   aResult.getArray(), CommonFunctors::makeAny< OUString >());
    }
    else if( aRange.match( lcl_aLabelRangePrefix ))
    {
        sal_Int32 nIndex = aRange.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        vector< OUString > aLabels( m_bDataInColumns ? rData.getColumnLabels() : rData.getRowLabels());
        if( nIndex < static_cast< sal_Int32 >( aLabels.size()))
        {
            aResult.realloc( 1 );
            aResult[0] = uno::makeAny( aLabels[ nIndex ] );
        }
    }
    else
    {
        sal_Int32 nIndex = aRange.toInt32();
        if( nIndex < (m_bDataInColumns ? rData.getColumnCount() : rData.getRowCount()))
        {
            Sequence< double > aData( rData.getDataAt( nIndex, m_bDataInColumns ));
            aResult.realloc( aData.getLength());
            transform( aData.getConstArray(), aData.getConstArray() + aData.getLength(),
                       aResult.getArray(), CommonFunctors::makeAny< double >());
        }
    }

    return aResult;
}

void SAL_CALL InternalDataProvider::setDataByRangeRepresentation(
    const OUString& aRange, const Sequence< uno::Any >& aNewData )
    throw (uno::RuntimeException)
{
    impl::InternalData & rData( getInternalData());

    if( aRange.equals( lcl_aCategoriesRangeName ))
    {
        vector< OUString > aCategories;
        transform( aNewData.getConstArray(), aNewData.getConstArray() + aNewData.getLength(),
                   back_inserter( aCategories ), CommonFunctors::AnyToString());

        if( m_bDataInColumns )
            rData.setRowLabels( aCategories );
        else
            rData.setColumnLabels( aCategories );
    }
    else if( aRange.match( lcl_aLabelRangePrefix ))
    {
        sal_Int32 nIndex = aRange.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        OUString aNewLabel;
        if( aNewData.getLength() &&
            (aNewData[0] >>= aNewLabel))
        {
            if( m_bDataInColumns )
            {
                vector< OUString > aLabels( rData.getColumnLabels());
                aLabels[ nIndex ] = aNewLabel;
                rData.setColumnLabels( aLabels );
            }
            else
            {
                vector< OUString > aLabels( rData.getRowLabels());
                aLabels[ nIndex ] = aNewLabel;
                rData.setRowLabels( aLabels );
            }
        }
    }
    else
    {
        sal_Int32 nIndex = aRange.toInt32();
        // ensure that the data is large enough
        if( m_bDataInColumns )
            rData.enlargeData( nIndex, 0 );
        else
            rData.enlargeData( 0, nIndex );

        if( nIndex < (m_bDataInColumns ? rData.getColumnCount() : rData.getRowCount()))
        {
            vector< double > aNewDataVec;
            transform( aNewData.getConstArray(), aNewData.getConstArray() + aNewData.getLength(),
                       back_inserter( aNewDataVec ), CommonFunctors::AnyToDouble());
            rData.setDataAt( nIndex, m_bDataInColumns, aNewDataVec );
        }
    }
}

void SAL_CALL InternalDataProvider::insertSequence( ::sal_Int32 nAfterIndex )
    throw (uno::RuntimeException)
{
    if( m_bDataInColumns )
    {
        increaseMapReferences( nAfterIndex + 1, getInternalData().getColumnCount());
        getInternalData().insertColumn( nAfterIndex );
    }
    else
    {
        increaseMapReferences( nAfterIndex + 1, getInternalData().getRowCount());
        getInternalData().insertRow( nAfterIndex );
    }
}

void SAL_CALL InternalDataProvider::deleteSequence( ::sal_Int32 nAtIndex )
    throw (uno::RuntimeException)
{
    deleteMapReferences( OUString::valueOf( nAtIndex ));
    deleteMapReferences( lcl_aLabelRangePrefix + OUString::valueOf( nAtIndex ));
    if( m_bDataInColumns )
    {
        decreaseMapReferences( nAtIndex + 1, getInternalData().getColumnCount());
        getInternalData().deleteColumn( nAtIndex );
    }
    else
    {
        decreaseMapReferences( nAtIndex + 1, getInternalData().getRowCount());
        getInternalData().deleteRow( nAtIndex );
    }
}

void SAL_CALL InternalDataProvider::appendSequence()
    throw (uno::RuntimeException)
{
    if( m_bDataInColumns )
        getInternalData().appendColumn();
    else
        getInternalData().appendRow();
}

void SAL_CALL InternalDataProvider::insertDataPointForAllSequences( ::sal_Int32 nAfterIndex )
    throw (uno::RuntimeException)
{
    sal_Int32 nMaxRep = 0;
    if( m_bDataInColumns )
    {
        getInternalData().insertRow( nAfterIndex );
        nMaxRep = getInternalData().getColumnCount();
    }
    else
    {
        getInternalData().insertColumn( nAfterIndex );
        nMaxRep = getInternalData().getRowCount();
    }

    // notify change to all affected ranges
    tSequenceMap::const_iterator aBegin( m_aSequenceMap.lower_bound( C2U("0")));
    tSequenceMap::const_iterator aEnd( m_aSequenceMap.upper_bound( OUString::valueOf( nMaxRep )));
    ::std::for_each( aBegin, aEnd, lcl_modifySeqMapValue());

    tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
    ::std::for_each( aRange.first, aRange.second, lcl_modifySeqMapValue());
}

void SAL_CALL InternalDataProvider::deleteDataPointForAllSequences( ::sal_Int32 nAtIndex )
    throw (uno::RuntimeException)
{
    sal_Int32 nMaxRep = 0;
    if( m_bDataInColumns )
    {
        getInternalData().deleteRow( nAtIndex );
        nMaxRep = getInternalData().getColumnCount();
    }
    else
    {
        getInternalData().deleteColumn( nAtIndex );
        nMaxRep = getInternalData().getRowCount();
    }

    // notify change to all affected ranges
    tSequenceMap::const_iterator aBegin( m_aSequenceMap.lower_bound( C2U("0")));
    tSequenceMap::const_iterator aEnd( m_aSequenceMap.upper_bound( OUString::valueOf( nMaxRep )));
    ::std::for_each( aBegin, aEnd, lcl_modifySeqMapValue());

    tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
    ::std::for_each( aRange.first, aRange.second, lcl_modifySeqMapValue());
}

void SAL_CALL InternalDataProvider::swapDataPointWithNextOneForAllSequences( ::sal_Int32 nAtIndex )
    throw (uno::RuntimeException)
{
    getInternalData().swapAllDataAtIndexWithNext( nAtIndex, m_bDataInColumns );
    sal_Int32 nMaxRep = (m_bDataInColumns
                         ? getInternalData().getColumnCount()
                         : getInternalData().getRowCount());

    // notify change to all affected ranges
    tSequenceMap::const_iterator aBegin( m_aSequenceMap.lower_bound( C2U("0")));
    tSequenceMap::const_iterator aEnd( m_aSequenceMap.upper_bound( OUString::valueOf( nMaxRep )));
    ::std::for_each( aBegin, aEnd, lcl_modifySeqMapValue());

    tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
    ::std::for_each( aRange.first, aRange.second, lcl_modifySeqMapValue());
}

void SAL_CALL InternalDataProvider::registerDataSequenceForChanges( const Reference< chart2::data::XDataSequence >& xSeq )
    throw (uno::RuntimeException)
{
    if( xSeq.is())
        addDataSequenceToMap( xSeq->getSourceRangeRepresentation(), xSeq );
}


// ____ XRangeXMLConversion ____
OUString SAL_CALL InternalDataProvider::convertRangeToXML( const OUString& aRangeRepresentation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    XMLRangeHelper::CellRange aRange;
    aRange.aTableName = OUString(RTL_CONSTASCII_USTRINGPARAM("local-table"));
    impl::InternalData & rData( getInternalData());

    // attention: this data provider has the limitation that it stores
    // internally if data comes from columns or rows. It is intended for
    // creating only one used data source.
    // @todo: add this information in the range representation strings
    if( aRangeRepresentation.equals( lcl_aCategoriesRangeName ))
    {
        aRange.aUpperLeft.bIsEmpty = false;
        if( m_bDataInColumns )
        {
            aRange.aUpperLeft.nColumn = 0;
            aRange.aUpperLeft.nRow = 1;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nRow = rData.getRowCount();
        }
        else
        {
            aRange.aUpperLeft.nColumn = 1;
            aRange.aUpperLeft.nRow = 0;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nColumn = rData.getColumnCount();
        }
    }
    else if( aRangeRepresentation.match( lcl_aLabelRangePrefix ))
    {
        sal_Int32 nIndex = aRangeRepresentation.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        aRange.aUpperLeft.bIsEmpty = false;
        aRange.aLowerRight.bIsEmpty = true;
        if( m_bDataInColumns )
        {
            aRange.aUpperLeft.nColumn = nIndex + 1;
            aRange.aUpperLeft.nRow = 0;
        }
        else
        {
            aRange.aUpperLeft.nColumn = 0;
            aRange.aUpperLeft.nRow = nIndex + 1;
        }
    }
    else if( aRangeRepresentation.equals( lcl_aCompleteRange ))
    {
        aRange.aUpperLeft.bIsEmpty = false;
        aRange.aLowerRight.bIsEmpty = false;
        aRange.aUpperLeft.nColumn = 0;
        aRange.aUpperLeft.nRow = 0;
        aRange.aLowerRight.nColumn = rData.getColumnCount();
        aRange.aLowerRight.nRow = rData.getRowCount();
    }
    else
    {
        sal_Int32 nIndex = aRangeRepresentation.toInt32();
        aRange.aUpperLeft.bIsEmpty = false;
        if( m_bDataInColumns )
        {
            aRange.aUpperLeft.nColumn = nIndex + 1;
            aRange.aUpperLeft.nRow = 1;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nRow = rData.getRowCount();
        }
        else
        {
            aRange.aUpperLeft.nColumn = 1;
            aRange.aUpperLeft.nRow = nIndex + 1;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nColumn = rData.getColumnCount();
        }
    }

    return XMLRangeHelper::getXMLStringFromCellRange( aRange );
}

OUString SAL_CALL InternalDataProvider::convertRangeFromXML( const OUString& aXMLRange )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    XMLRangeHelper::CellRange aRange( XMLRangeHelper::getCellRangeFromXMLString( aXMLRange ));
    if( aRange.aUpperLeft.bIsEmpty )
    {
        OSL_ENSURE( aRange.aLowerRight.bIsEmpty, "Weird Range" );
        return OUString();
    }

    // "all"
    if( !aRange.aLowerRight.bIsEmpty &&
        ( aRange.aUpperLeft.nColumn != aRange.aLowerRight.nColumn ) &&
        ( aRange.aUpperLeft.nRow != aRange.aLowerRight.nRow ) )
        return lcl_aCompleteRange;

    // attention: this data provider has the limitation that it stores
    // internally if data comes from columns or rows. It is intended for
    // creating only one used data source.
    // @todo: add this information in the range representation strings

    // data in columns
    if( m_bDataInColumns )
    {
        if( aRange.aUpperLeft.nColumn == 0 )
            return lcl_aCategoriesRangeName;
        if( aRange.aUpperLeft.nRow == 0 )
            return lcl_aLabelRangePrefix + OUString::valueOf( aRange.aUpperLeft.nColumn - 1 );

        return OUString::valueOf( aRange.aUpperLeft.nColumn - 1 );
    }

    // data in rows
    if( aRange.aUpperLeft.nRow == 0 )
        return lcl_aCategoriesRangeName;
    if( aRange.aUpperLeft.nColumn == 0 )
        return lcl_aLabelRangePrefix + OUString::valueOf( aRange.aUpperLeft.nRow - 1 );

    return OUString::valueOf( aRange.aUpperLeft.nRow - 1 );
}

// ____ XChartDataArray ____
// note: do not use m_bDataInColumns for all XChartDataArray-specific code
// the chart-API assumes data is always in rows
Sequence< Sequence< double > > SAL_CALL InternalDataProvider::getData()
    throw (uno::RuntimeException)
{
    return getInternalData().getData( false );
}

void SAL_CALL InternalDataProvider::setData( const Sequence< Sequence< double > >& aData )
    throw (uno::RuntimeException)
{
    return getInternalData().setData( aData, false );
}

Sequence< OUString > SAL_CALL InternalDataProvider::getRowDescriptions()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( getInternalData().getRowLabels());
}

void SAL_CALL InternalDataProvider::setRowDescriptions( const Sequence< OUString >& aRowDescriptions )
    throw (uno::RuntimeException)
{
    getInternalData().setRowLabels( ContainerHelper::SequenceToVector( aRowDescriptions ));
}

Sequence< OUString > SAL_CALL InternalDataProvider::getColumnDescriptions()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( getInternalData().getColumnLabels());
}

void SAL_CALL InternalDataProvider::setColumnDescriptions( const Sequence< OUString >& aColumnDescriptions )
    throw (uno::RuntimeException)
{
    getInternalData().setColumnLabels( ContainerHelper::SequenceToVector( aColumnDescriptions ));
}


// ____ XChartData (base of XChartDataArray) ____
void SAL_CALL InternalDataProvider::addChartDataChangeEventListener(
    const Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL InternalDataProvider::removeChartDataChangeEventListener(
    const Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& )
    throw (uno::RuntimeException)
{
}

double SAL_CALL InternalDataProvider::getNotANumber()
    throw (uno::RuntimeException)
{
    double fNan;
    ::rtl::math::setNan( & fNan );
    return fNan;
}

::sal_Bool SAL_CALL InternalDataProvider::isNotANumber( double nNumber )
    throw (uno::RuntimeException)
{
    return ::rtl::math::isNan( nNumber )
        || ::rtl::math::isInf( nNumber );
}

// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL InternalDataProvider::createClone()
    throw (uno::RuntimeException)
{
    return Reference< util::XCloneable >( new InternalDataProvider( *this ));
}


// ================================================================================

Sequence< OUString > InternalDataProvider::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.data.DataProvider" ));
    return aServices;
}

// ================================================================================

APPHELPER_XSERVICEINFO_IMPL( InternalDataProvider, lcl_aServiceName );

} //  namespace chart
