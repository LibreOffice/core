/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableHandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2006-11-02 12:37:24 $
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
#endif

void DomainMapperTableHandler::startTable(unsigned int nRows,
                                          unsigned int /*nDepth*/,
                                          PropertyMapPtr /*pProps*/)
{
    m_pHandleSeq3 = HandleSequence3Pointer_t(new HandleSequence3_t(nRows));
    m_nHandle3Index = 0;

#ifdef DEBUG
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%d", nRows);
    clog << "<table rows=\"" << sBuffer << "\">" << endl;
#endif
}

void DomainMapperTableHandler::endTable()
{
#ifdef DEBUG
    clog << "</table>" << endl;
#endif

    PropertyValuesSeq2_t aCellProperties;
    PropertyValuesSeq_t aRowProperties;
    PropertyValues_t aTableProperties;

    if (m_pHandleSeq3->getLength() > 0)
    {
        try
        {
            m_xText->convertToTable(*m_pHandleSeq3, aCellProperties,
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
                                        PropertyMapPtr /*pProps*/)
{
    char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%d", nCells);

#ifdef DEBUG
    clog << "<table.row cells=\"" << sBuffer << "\">" << endl;
#endif

    m_pHandleSeq2 = HandleSequence2Pointer_t(new HandleSequence2_t(nCells));
    m_nHandle2Index = 0;
}

void DomainMapperTableHandler::endRow()
{
    (*m_pHandleSeq3)[m_nHandle3Index] = *m_pHandleSeq2;
    ++m_nHandle3Index;

#ifdef DEBUG
    clog << "</table.row>" << endl;
#endif
}

void DomainMapperTableHandler::startCell(const Handle_t & start,
                                         PropertyMapPtr /*pProps*/)
{
#ifdef DEBUG
    clog << "<table.cell>";
    lcl_printHandle(start);
    clog << ",";
#endif

    m_pHandleSeq = HandleSequencePointer_t(new HandleSequence_t(2));
    (*m_pHandleSeq)[0] = start->getStart();
}

void DomainMapperTableHandler::endCell(const Handle_t & end)
{
#ifdef DEBUG
    lcl_printHandle(end);
    clog << "</table.cell>" << endl;
#endif

    (*m_pHandleSeq)[1] = end->getEnd();
    (*m_pHandleSeq2)[m_nHandle2Index] = *m_pHandleSeq;
    ++m_nHandle2Index;
}

}
