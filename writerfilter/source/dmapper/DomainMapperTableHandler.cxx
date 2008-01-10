/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableHandler.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:37:22 $
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
#include <DomainMapperTableHandler.hxx>
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HDL_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifdef DEBUG
#include <iostream>
#endif

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

#define DEF_BORDER_DIST 190  //0,19cm

#ifdef DEBUG
static void lcl_printHandle(const Handle_t rHandle)
{
    if (!rHandle.get())
        return;
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
        PropertyNameSupplier& rPropSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        for( ; aMapIter != aEndIter; ++aMapIter )
        {
            rtl::OUString aOUStr = rPropSupplier.GetName( aMapIter->first.eId );
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

        //contains the default border of the line
        PropertyMapPtr aCellBorders( new PropertyMap );

        sal_Int32 nCell = 0;
        pCellProperties[nRow].realloc( aRowOfCellsIterator->size() );
        beans::PropertyValues* pSingleCellProperties = pCellProperties[nRow].getArray();
        while( aCellIterator != aCellIteratorEnd )
        {
            //TODO: aCellIterator contains HorizontalBorder and VerticalBorder
            // they have to be removed, depending on the position of the cell they
            // have to be moved to BottomBorder/RightBorder respectively
            //aCellIterator points to a PropertyMapPtr;
            if( aCellIterator->get() )
            {
                if( nCell && aCellBorders->size() )
                {
                    //now apply the default border
                    //TODO: This overwrites the existing values!
                    aCellIterator->get()->insert( aCellBorders, false );
                }


                const PropertyMap::iterator aVerticalIter =
                                aCellIterator->get()->find( PropertyDefinition(META_PROP_VERTICAL_BORDER, false) );
                const PropertyMap::iterator aHorizontalIter =
                                aCellIterator->get()->find( PropertyDefinition(META_PROP_HORIZONTAL_BORDER, false) );
                const PropertyMap::const_iterator aRightIter =
                                aCellIterator->get()->find( PropertyDefinition(PROP_RIGHT_BORDER, false) );
                const PropertyMap::const_iterator aBottomIter =
                                aCellIterator->get()->find( PropertyDefinition(PROP_BOTTOM_BORDER, false) );

                if( aVerticalIter != aCellIterator->get()->end())
                {
                    if( !nCell )
                        aCellBorders->insert(*aVerticalIter);
                    aCellIterator->get()->erase( aVerticalIter );
                }
                if( aHorizontalIter != aCellIterator->get()->end())
                {
                    if( !nCell )
                        aCellBorders->insert(*aHorizontalIter);
                    aCellIterator->get()->erase( aHorizontalIter );
                }
                //fill the additional borders into the line default border
                if( !nCell )
                {
                    const PropertyMap::const_iterator aLeftIter =
                                    aCellIterator->get()->find( PropertyDefinition(PROP_RIGHT_BORDER, false) );
                    if(aLeftIter != aCellIterator->get()->end())
                        aCellBorders->insert(*aLeftIter);
                    if(aRightIter != aCellIterator->get()->end())
                        aCellBorders->insert(*aRightIter);
                    const PropertyMap::const_iterator aTopIter =
                                    aCellIterator->get()->find( PropertyDefinition(PROP_TOP_BORDER, false) );
                    if(aTopIter != aCellIterator->get()->end())
                        aCellBorders->insert(*aTopIter);
                    if(aBottomIter != aCellIterator->get()->end())
                        aCellBorders->insert(*aBottomIter);
                }

                //now set the default left+right border distance TODO: there's an sprm containing the default distance!
                const PropertyMap::const_iterator aLeftDistanceIter =
                                aCellIterator->get()->find( PropertyDefinition(PROP_LEFT_BORDER_DISTANCE, false) );
                if( aLeftDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_LEFT_BORDER_DISTANCE, false,
                                                                        uno::makeAny((sal_Int32) DEF_BORDER_DIST ) );
                const PropertyMap::const_iterator aRightDistanceIter =
                                aCellIterator->get()->find( PropertyDefinition(PROP_RIGHT_BORDER_DISTANCE, false) );
                if( aRightDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_RIGHT_BORDER_DISTANCE, false,
                                                        uno::makeAny((sal_Int32) DEF_BORDER_DIST ) );

                pSingleCellProperties[nCell] = aCellIterator->get()->GetPropertyValues();
            }
            ++nCell;
            ++aCellIterator;
        }
#ifdef DEBUG
//-->debug cell properties
        {
            ::rtl::OUString sNames;
            const uno::Sequence< beans::PropertyValues > aDebugCurrentRow = aCellProperties[nRow];
            sal_Int32 nDebugCells = aDebugCurrentRow.getLength();
            (void) nDebugCells;
            for( sal_Int32  nDebugCell = 0; nDebugCell < nDebugCells; ++nDebugCell)
            {
                const uno::Sequence< beans::PropertyValue >& aDebugCellProperties = aDebugCurrentRow[nDebugCell];
                sal_Int32 nDebugCellProperties = aDebugCellProperties.getLength();
                for( sal_Int32  nDebugProperty = 0; nDebugProperty < nDebugCellProperties; ++nDebugProperty)
                {
                    const ::rtl::OUString sName = aDebugCellProperties[nDebugProperty].Name;
                    sNames += sName;
                    sNames += ::rtl::OUString('-');
                }
                sNames += ::rtl::OUString(' ');
            }
            (void)sNames;
        }
//--<
#endif
        ++nRow;
        ++aRowOfCellsIterator;
    }
#ifdef DEBUG
//-->debug cell properties of all rows
    {
        ::rtl::OUString sNames;
        for( sal_Int32  nDebugRow = 0; nDebugRow < aCellProperties.getLength(); ++nDebugRow)
        {
            const uno::Sequence< beans::PropertyValues > aDebugCurrentRow = aCellProperties[nDebugRow];
            sal_Int32 nDebugCells = aDebugCurrentRow.getLength();
            (void) nDebugCells;
            for( sal_Int32  nDebugCell = 0; nDebugCell < nDebugCells; ++nDebugCell)
            {
                const uno::Sequence< beans::PropertyValue >& aDebugCellProperties = aDebugCurrentRow[nDebugCell];
                sal_Int32 nDebugCellProperties = aDebugCellProperties.getLength();
                for( sal_Int32  nDebugProperty = 0; nDebugProperty < nDebugCellProperties; ++nDebugProperty)
                {
                    const ::rtl::OUString sName = aDebugCellProperties[nDebugProperty].Name;
                    sNames += sName;
                    sNames += ::rtl::OUString('-');
                }
                sNames += ::rtl::OUString('+');
            }
            sNames += ::rtl::OUString('|');
        }
        (void)sNames;
    }
//--<
#endif

    RowPropertyValuesSeq_t      aRowProperties( m_aRowProperties.size() );
    PropertyMapVector1::const_iterator aRowIter = m_aRowProperties.begin();
    PropertyMapVector1::const_iterator aRowIterEnd = m_aRowProperties.end();
    nRow = 0;
    while( aRowIter != aRowIterEnd )
    {
        if( aRowIter->get() )
        {
            //set default to 'break across pages"
            if( aRowIter->get()->find( PropertyDefinition( PROP_IS_SPLIT_ALLOWED, false )) == aRowIter->get()->end())
                aRowIter->get()->Insert( PROP_IS_SPLIT_ALLOWED, false, uno::makeAny(sal_True ) );

            aRowProperties[nRow] = aRowIter->get()->GetPropertyValues();
        }
        ++nRow;
        ++aRowIter;
    }

    TablePropertyValues_t       aTableProperties;
    if( m_aTableProperties.get() )
    {
        const PropertyMap::iterator aTableStyleIter =
                                m_aTableProperties->find( PropertyDefinition( META_PROP_TABLE_STYLE_NAME, false ) );
        if(aTableStyleIter != m_aTableProperties->end())
        {
            //TODO: apply table style properties recursively
            m_aTableProperties->erase( aTableStyleIter );
        }
        //fill default value - if not available
        const PropertyMap::const_iterator aRepeatIter =
                                m_aTableProperties->find( PropertyDefinition( PROP_HEADER_ROW_COUNT, false ) );
        if( aRepeatIter == m_aTableProperties->end() )
            m_aTableProperties->Insert( PROP_HEADER_ROW_COUNT, false, uno::makeAny( (sal_Int32)0 ));

        //set default orientation if not already set
        const PropertyMap::const_iterator aOrientIter =
                    m_aTableProperties->find( PropertyDefinition( PROP_HORI_ORIENT, false ));
        if( aOrientIter == m_aTableProperties->end() )
            m_aTableProperties->Insert( PROP_HORI_ORIENT, false, uno::makeAny( text::HoriOrientation::LEFT_AND_WIDTH ) );
        aTableProperties = m_aTableProperties->GetPropertyValues();
    }

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
    m_aTableProperties.reset();
    m_aCellProperties.clear();
    m_aRowProperties.clear();
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
    if (!start.get())
        return;
    (*m_pCellSeq)[0] = start->getStart();
}

void DomainMapperTableHandler::endCell(const Handle_t & end)
{
#ifdef DEBUG
    lcl_printHandle(end);
    clog << "</table.cell>" << endl;
#endif

    if (!end.get())
        return;
    (*m_pCellSeq)[1] = end->getEnd();
    (*m_pRowSeq)[m_nCellIndex] = *m_pCellSeq;
    ++m_nCellIndex;
}

}}
