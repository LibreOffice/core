#include <iostream>
#include <DomainMapperTableHandler.hxx>

namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

void DomainMapperTableHandler::startTable(unsigned int nRows,
                                          unsigned int /*nDepth*/,
                                          PropertyMapPtr /*pProps*/)
{
    m_pHandleSeq3 = HandleSequence3Pointer_t(new HandleSequence3_t(nRows));
    m_nHandle3Index = 0;
}

void DomainMapperTableHandler::endTable()
{
    PropertyValuesSeq2_t aCellProperties;
    PropertyValuesSeq_t aRowProperties;
    PropertyValues_t aTableProperties;

    sal_Int32 nRows = m_pHandleSeq3->getLength();
    char sBuffer[256];

    snprintf(sBuffer, sizeof(sBuffer), "%ld", nRows);

    clog << "<table rows=\"" << sBuffer << "\">" << endl;

    for (sal_Int32 nRow = 0; nRow < nRows; ++nRow)
    {
        HandleSequence2_t  aHandleSeq2 = (*m_pHandleSeq3)[nRow];

        sal_Int32 nCells = aHandleSeq2.getLength();

        snprintf(sBuffer, sizeof(sBuffer), "%ld", nCells);

        clog << "<table.row cells=\"" << sBuffer << "\">" << endl;

        for (sal_Int32 nCell = 0; nCell < nCells; ++nCell)
        {
            HandleSequence_t aHandleSeq = aHandleSeq2[nCell];

            sal_Int32 nHandles = aHandleSeq.getLength();

            snprintf(sBuffer, sizeof(sBuffer), "%ld", nHandles);

            clog << "<table.cell handles=\"" << sBuffer << "\">" << endl;

            for (sal_Int32 nHandle = 0; nHandle < nHandles; ++nHandle)
            {
                clog << "<table.text>" << endl;

                Handle_t aHandle = aHandleSeq[nHandle];

                rtl::OUString aOUStr = aHandle->getString();
                rtl::OString aOStr(aOUStr.getStr(), RTL_TEXTENCODING_ASCII_US,
                                   aOUStr.getLength());

                clog << aOStr.getStr() << endl;
                clog << "</table.text>" << endl;
            }

            clog << "</table.cell>" << endl;
        }

        clog << "</table.row>" << endl;
    }

    clog << "</table>" << endl;

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
            clog << "failed to import table!" << endl;
        }
    }
}

void DomainMapperTableHandler::startRow(unsigned int nCells,
                                        PropertyMapPtr /*pProps*/)
{
    m_pHandleSeq2 = HandleSequence2Pointer_t(new HandleSequence2_t(nCells));
    m_nHandle2Index = 0;
}

void DomainMapperTableHandler::endRow()
{
    (*m_pHandleSeq3)[m_nHandle3Index] = *m_pHandleSeq2;
    ++m_nHandle3Index;
}

void DomainMapperTableHandler::startCell(const Handle_t & start,
                                         PropertyMapPtr /*pProps*/)
{
    m_pHandleSeq = HandleSequencePointer_t(new HandleSequence_t(2));
    (*m_pHandleSeq)[0] = start->getStart();
}

void DomainMapperTableHandler::endCell(const Handle_t & end)
{
    (*m_pHandleSeq)[1] = end->getEnd();
    (*m_pHandleSeq2)[m_nHandle2Index] = *m_pHandleSeq;
    ++m_nHandle2Index;
}

}
