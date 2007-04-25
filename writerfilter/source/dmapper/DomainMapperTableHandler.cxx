/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableHandler.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2007-04-25 11:30:51 $
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
    m_nTableIndex = 0;

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
    clog << "</table>" << endl;
#endif
    CellPropertyValuesSeq_t     aCellProperties;
    RowPropertyValuesSeq_t      aRowProperties;
    TablePropertyValues_t       aTableProperties;

    if (m_pTableSeq->getLength() > 0)
    {
        try
        {
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
    m_nRowIndex = 0;
}

void DomainMapperTableHandler::endRow()
{
    (*m_pTableSeq)[m_nTableIndex] = *m_pRowSeq;
    ++m_nTableIndex;

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
    (*m_pRowSeq)[m_nRowIndex] = *m_pCellSeq;
    ++m_nRowIndex;
}

}
