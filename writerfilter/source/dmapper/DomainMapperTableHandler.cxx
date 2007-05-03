/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableHandler.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2007-05-03 06:25:37 $
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
#include <iostream>
#include <DomainMapperTableHandler.hxx>

namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

#ifdef DEBUG
static void lcl_printHandle(const Handle_t rHandle)
{
    rtl::OUString aOUStr = rHandle->getString();
    rtl::OString aOStr(aOUStr.getStr(), aOUStr.getLength(),  RTL_TEXTENCODING_ASCII_US );

    clog << aOStr.getStr() << endl;
}
static void  lcl_printProperties( PropertyMapPtr pProps )
{
    if( pProps.get() )
    {
        clog << "<properties>";
        PropertyMap::const_iterator aMapIter = pProps->begin();
        PropertyMap::const_iterator aEndIter = pProps->end();
        for( ; aMapIter != aEndIter; ++aMapIter )
        {
            rtl::OUString aOUStr = aMapIter->first;
            rtl::OString aOStr(aOUStr.getStr(), aOUStr.getLength(),  RTL_TEXTENCODING_ASCII_US );
            clog << aOStr.getStr() << '-';
        }
        clog << endl;
    }
}
#endif

void DomainMapperTableHandler::startTable(unsigned int nRows,
                                          unsigned int /*nDepth*/,
                                          PropertyMapPtr pProps)
{
    m_aTableProperties = pProps;
    m_pTableSeq = TableSequencePointer_t(new TableSequence_t(nRows));
    m_nRowIndex = 0;

#ifdef DEBUG
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%d", nRows);
    clog << "<table rows=\"" << sBuffer << "\">" << endl;
    lcl_printProperties( pProps );
#endif
}

void DomainMapperTableHandler::endTable()
{
#ifdef DEBUG
{
    clog << "</table>" << endl;
    sal_uInt32 nCells = 0;
    sal_uInt32 nRows = m_aRowProperties.size();
    if( nRows == m_aCellProperties.size() )
    {
        for( sal_uInt32 nRow = 0; nRow < nRows; ++nRow )
            nCells += m_aCellProperties[nRow].size();
    }
    sal_uInt32 nTblPropSize = m_aTableProperties.get() ? m_aTableProperties->size() : 0;
    (void)nTblPropSize;
}
#endif


    //  expands to uno::Sequence< Sequence< beans::PropertyValues > >
    CellPropertyValuesSeq_t     aCellProperties( m_aCellProperties.size() );

    // std::vector< std::vector<PropertyMapPtr> > m_aCellProperties
    PropertyMapVector2::const_iterator aRowOfCellsIterator = m_aCellProperties.begin();
    PropertyMapVector2::const_iterator aRowOfCellsIteratorEnd = m_aCellProperties.end();
    sal_Int32 nRow = 0;
    //it's a uno::Sequence< beans::PropertyValues >*
    RowPropertyValuesSeq_t* pCellProperties = aCellProperties.getArray();
    while( aRowOfCellsIterator != aRowOfCellsIteratorEnd )
    {
        //aRowOfCellsIterator points to a vector of PropertyMapPtr
        PropertyMapVector1::const_iterator aCellIterator = aRowOfCellsIterator->begin();
        PropertyMapVector1::const_iterator aCellIteratorEnd = aRowOfCellsIterator->end();

        sal_Int32 nCell = 0;
        pCellProperties[nRow].realloc( aRowOfCellsIterator->size() );
        beans::PropertyValues* pSingleCellProperties = pCellProperties->getArray();
        while( aCellIterator != aCellIteratorEnd )
        {
            //TODO: aCellIterator contains HorizontalBorder and VerticalBorder
            // they have to be removed, depending on the position of the cell they
            // have to be moved to BottomBorder/RightBorder respectively
            //aCellIterator points to a PropertyMapPtr;
            if( aCellIterator->get() )
            {

                PropertyNameSupplier& rPropSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
                const PropertyMap::iterator aVerticalIter =
                                aCellIterator->get()->find( rPropSupplier.GetName(META_PROP_VERTICAL_BORDER) );
                const PropertyMap::iterator aHorizontalIter =
                                aCellIterator->get()->find( rPropSupplier.GetName(META_PROP_HORIZONTAL_BORDER) );
                const PropertyMap::const_iterator aRightIter =
                                aCellIterator->get()->find( rPropSupplier.GetName(PROP_RIGHT_BORDER) );
                const PropertyMap::const_iterator aBottomIter =
                                aCellIterator->get()->find( rPropSupplier.GetName(PROP_BOTTOM_BORDER) );
                aCellIterator->get()->erase( aVerticalIter );
                aCellIterator->get()->erase( aHorizontalIter );

                pSingleCellProperties[nCell] = aCellIterator->get()->GetPropertyValues();
            }
            ++nCell;
            ++aCellIterator;
        }
        ++nRow;
        ++aRowOfCellsIterator;
    }

    RowPropertyValuesSeq_t      aRowProperties( m_aRowProperties.size() );
    PropertyMapVector1::const_iterator aRowIter = m_aRowProperties.begin();
    PropertyMapVector1::const_iterator aRowIterEnd = m_aRowProperties.end();
    nRow = 0;
    while( aRowIter != aRowIterEnd )
    {
        if( aRowIter->get() )
            aRowProperties[nRow] = aRowIter->get()->GetPropertyValues();
        ++nRow;
        ++aRowIter;
    }

    TablePropertyValues_t       aTableProperties;
    if( m_aTableProperties.get() )
        aTableProperties = m_aTableProperties->GetPropertyValues();

    if (m_pTableSeq->getLength() > 0)
    {
        try
        {
#ifdef DEBUG
    {
        sal_Int32 nCellPropertiesRows = aCellProperties.getLength();
        sal_Int32 nCellPropertiesCells = aCellProperties[0].getLength();
        sal_Int32 nCellPropertiesProperties = aCellProperties[0][0].getLength();
        (void) nCellPropertiesRows;
        (void) nCellPropertiesCells;
        (void) nCellPropertiesProperties;
        ++nCellPropertiesProperties;
    }
#endif
            m_xText->convertToTable(*m_pTableSeq,
                                    aCellProperties,
                                    aRowProperties,
                                    aTableProperties);
        }
        catch (lang::IllegalArgumentException e)
        {
#ifdef DEBUG
            clog << "failed to import table!" << endl;
#endif
        }
    }
}

void DomainMapperTableHandler::startRow(unsigned int nCells,
                                        PropertyMapPtr pProps)
{
    m_aRowProperties.push_back( pProps );
    m_aCellProperties.push_back( PropertyMapVector1() );

#ifdef DEBUG
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%d", nCells);

    clog << "<table.row cells=\"" << sBuffer << "\">" << endl;
    lcl_printProperties( pProps );
#endif

    m_pRowSeq = RowSequencePointer_t(new RowSequence_t(nCells));
    m_nCellIndex = 0;
}

void DomainMapperTableHandler::endRow()
{
    (*m_pTableSeq)[m_nRowIndex] = *m_pRowSeq;
    ++m_nRowIndex;
    m_nCellIndex = 0;
#ifdef DEBUG
    clog << "</table.row>" << endl;
#endif
}

void DomainMapperTableHandler::startCell(const Handle_t & start,
                                         PropertyMapPtr pProps )
{
    sal_uInt32 nRow = m_aRowProperties.size();
    m_aCellProperties[nRow - 1].push_back( pProps );

#ifdef DEBUG
    clog << "<table.cell>";
    lcl_printHandle(start);
    lcl_printProperties( pProps );
    clog << ",";
#endif

    //add a new 'row' of properties
//    if( m_pCellProperties.size() <= sal::static_int_cast< sal_uInt32, sal_Int32>(m_nRowIndex) )
//        m_pCellProperties.push_back( RowProperties_t() );
//    m_pCellProperties[m_nRowIndex].push_back( pProps );
    m_pCellSeq = CellSequencePointer_t(new CellSequence_t(2));
    (*m_pCellSeq)[0] = start->getStart();
}

void DomainMapperTableHandler::endCell(const Handle_t & end)
{
#ifdef DEBUG
    lcl_printHandle(end);
    clog << "</table.cell>" << endl;
#endif

    (*m_pCellSeq)[1] = end->getEnd();
    (*m_pRowSeq)[m_nCellIndex] = *m_pCellSeq;
    ++m_nCellIndex;
}

}
