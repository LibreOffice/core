#include <iostream>
#include <DomainMapperTableHandler.hxx>

namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

static void lcl_printHandle(const Handle_t rHandle)
{
    rtl::OUString aOUStr = rHandle->getString();
    rtl::OString aOStr(aOUStr.getStr(), RTL_TEXTENCODING_ASCII_US,
                       aOUStr.getLength());

    clog << aOStr.getStr() << endl;
}

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
