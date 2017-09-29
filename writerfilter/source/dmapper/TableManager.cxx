/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <TableManager.hxx>
#include <DomainMapperTableHandler.hxx>
#include <DomainMapper_Impl.hxx>
#include <util.hxx>
#include <cppuhelper/logging.hxx>

namespace writerfilter
{
namespace dmapper
{

void TableManager::clearData()
{
}

void TableManager::openCell(const css::uno::Reference<css::text::XTextRange>& rHandle, const TablePropertyMapPtr& pProps)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.openCell");
    TagLogger::getInstance().chars(XTextRangeToString(rHandle));
    TagLogger::getInstance().endElement();
#endif

    if (!mTableDataStack.empty())
    {
        TableData::Pointer_t pTableData = mTableDataStack.top();

        pTableData->addCell(rHandle, pProps);
    }
}

bool TableManager::isIgnore() const
{
    return isRowEnd();
}

void TableManager::endOfRowAction()
{
}

void TableManager::endOfCellAction()
{
}

void TableManager::insertTableProps(const TablePropertyMapPtr& pProps)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.insertTableProps");
#endif

    if (getTableProps().get() && getTableProps() != pProps)
        getTableProps()->InsertProps(pProps);
    else
        mState.setTableProps(pProps);

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::insertRowProps(const TablePropertyMapPtr& pProps)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.insertRowProps");
#endif

    if (getRowProps().get())
        getRowProps()->InsertProps(pProps);
    else
        mState.setRowProps(pProps);

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::cellPropsByCell(unsigned int i, const TablePropertyMapPtr& pProps)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.cellPropsByCell");
#endif

    mTableDataStack.top()->insertCellProperties(i, pProps);

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::cellProps(const TablePropertyMapPtr& pProps)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.cellProps");
#endif

    if (getCellProps().get())
        getCellProps()->InsertProps(pProps);
    else
        mState.setCellProps(pProps);

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::utext(const sal_uInt8* data, std::size_t len)
{
    // optimization: cell/row end characters are the last characters in a run

    if (len > 0)
    {
        sal_Unicode nChar = data[(len - 1) * 2] + (data[(len - 1) * 2 + 1] << 8);
        if (nChar == 0x7)
            handle0x7();
    }
}

void TableManager::text(const sal_uInt8* data, std::size_t len)
{
    // optimization: cell/row end characters are the last characters in a run
    if (len > 0)
    {
        if (data[len - 1] == 0x7)
            handle0x7();
    }
}

void TableManager::handle0x7()
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.handle0x7");
#endif

    if (mnTableDepthNew < 1)
        mnTableDepthNew = 1;

    if (isInCell())
        endCell();
    else
        endRow();

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

bool TableManager::sprm(Sprm& rSprm)
{
    bool bRet = true;
    switch (rSprm.getId())
    {
    case NS_ooxml::LN_tblDepth:
    {
        Value::Pointer_t pValue = rSprm.getValue();

        cellDepth(pValue->getInt());
    }
    break;
    case NS_ooxml::LN_inTbl:
        inCell();
        break;
    case NS_ooxml::LN_tblCell:
        endCell();
        break;
    case NS_ooxml::LN_tblRow:
        endRow();
        break;
    default:
        bRet = false;
    }
    return bRet;
}

void TableManager::closeCell(const css::uno::Reference<css::text::XTextRange>& rHandle)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.closeCell");
    TagLogger::getInstance().chars(XTextRangeToString(rHandle));
    TagLogger::getInstance().endElement();
#endif

    if (!mTableDataStack.empty())
    {
        TableData::Pointer_t pTableData = mTableDataStack.top();

        pTableData->endCell(rHandle);

        if (mpTableDataHandler)
            mpTableDataHandler->getDomainMapperImpl().ClearPreviousParagraph();
    }
}

void TableManager::ensureOpenCell(const TablePropertyMapPtr& pProps)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.ensureOpenCell");
#endif

    if (!mTableDataStack.empty())
    {
        TableData::Pointer_t pTableData = mTableDataStack.top();

        if (pTableData != nullptr)
        {
            if (!pTableData->isCellOpen())
                openCell(getHandle(), pProps);
            else
                pTableData->insertCellProperties(pProps);
        }
    }
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::endParagraphGroup()
{
    sal_Int32 nTableDepthDifference = mnTableDepthNew - mnTableDepth;

    TablePropertyMapPtr pEmptyProps;

    while (nTableDepthDifference > 0)
    {
        ensureOpenCell(pEmptyProps);
        startLevel();

        --nTableDepthDifference;
    }
    while (nTableDepthDifference < 0)
    {
        endLevel();

        ++nTableDepthDifference;
    }

    mnTableDepth = mnTableDepthNew;

    if (mnTableDepth > 0)
    {
        if (isRowEnd())
        {
            endOfRowAction();
            mTableDataStack.top()->endRow(getRowProps());
            mState.resetRowProps();
        }

        else if (isInCell())
        {
            ensureOpenCell(getCellProps());

            if (mState.isCellEnd())
            {
                endOfCellAction();
                closeCell(getHandle());
            }
        }
        mState.resetCellProps();
    }
}

void TableManager::startParagraphGroup()
{
    mState.resetCellSpecifics();
    mnTableDepthNew = 0;
}

void TableManager::resolveCurrentTable()
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.resolveCurrentTable");
#endif

    if (mpTableDataHandler != nullptr)
    {
        try
        {
            TableData::Pointer_t pTableData = mTableDataStack.top();

            unsigned int nRows = pTableData->getRowCount();

            mpTableDataHandler->startTable(getTableProps());

            for (unsigned int nRow = 0; nRow < nRows; ++nRow)
            {
                RowData::Pointer_t pRowData = pTableData->getRow(nRow);

                unsigned int nCells = pRowData->getCellCount();

                mpTableDataHandler->startRow(pRowData->getProperties());

                for (unsigned int nCell = 0; nCell < nCells; ++nCell)
                {
                    mpTableDataHandler->startCell(pRowData->getCellStart(nCell), pRowData->getCellProperties(nCell));

                    mpTableDataHandler->endCell(pRowData->getCellEnd(nCell));
                }

                mpTableDataHandler->endRow();
            }

            mpTableDataHandler->endTable(mTableDataStack.size() - 1, m_bTableStartsAtCellStart);
        }
        catch (css::uno::Exception const& e)
        {
            SAL_WARN("writerfilter", "resolving of current table failed with: " << e);
        }
    }
    mState.resetTableProps();
    clearData();

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::endLevel()
{
    if (mpTableDataHandler != nullptr)
        resolveCurrentTable();

    // Store the unfinished row as it will be used for the next table
    if (mbKeepUnfinishedRow)
        mpUnfinishedRow = mTableDataStack.top()->getCurrentRow();
    mState.endLevel();
    mTableDataStack.pop();

#ifdef DEBUG_WRITERFILTER
    TableData::Pointer_t pTableData;

    if (!mTableDataStack.empty())
        pTableData = mTableDataStack.top();

    TagLogger::getInstance().startElement("tablemanager.endLevel");
    TagLogger::getInstance().attribute("level", mTableDataStack.size());

    if (pTableData != nullptr)
        TagLogger::getInstance().attribute("openCell", pTableData->isCellOpen() ? "yes" : "no");

    TagLogger::getInstance().endElement();
#endif
}

void TableManager::startLevel()
{
#ifdef DEBUG_WRITERFILTER
    TableData::Pointer_t pTableData;

    if (!mTableDataStack.empty())
        pTableData = mTableDataStack.top();

    TagLogger::getInstance().startElement("tablemanager.startLevel");
    TagLogger::getInstance().attribute("level", mTableDataStack.size());

    if (pTableData != nullptr)
        TagLogger::getInstance().attribute("openCell", pTableData->isCellOpen() ? "yes" : "no");

    TagLogger::getInstance().endElement();
#endif

    TableData::Pointer_t pTableData2(new TableData(mTableDataStack.size()));

    // If we have an unfinished row stored here, then push it to the new TableData
    if (mpUnfinishedRow)
    {
        for (unsigned int i = 0; i < mpUnfinishedRow->getCellCount(); ++i)
        {
            pTableData2->addCell(mpUnfinishedRow->getCellStart(i), mpUnfinishedRow->getCellProperties(i));
            pTableData2->endCell(mpUnfinishedRow->getCellEnd(i));
        }
        mpUnfinishedRow.reset();
    }

    mTableDataStack.push(pTableData2);
    mState.startLevel();
}

bool TableManager::isInTable()
{
    bool bInTable = false;
    if (!mTableDataStack.empty())
        bInTable = mTableDataStack.top()->getDepth() > 0;
    return bInTable;
}

void TableManager::handle(const css::uno::Reference<css::text::XTextRange>& rHandle)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.handle");
    TagLogger::getInstance().chars(XTextRangeToString(rHandle));
    TagLogger::getInstance().endElement();
#endif

    setHandle(rHandle);
}

void TableManager::setHandler(const std::shared_ptr<DomainMapperTableHandler>& pTableDataHandler)
{
    mpTableDataHandler = pTableDataHandler;
}

void TableManager::endRow()
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().element("tablemanager.endRow");
#endif

    setRowEnd(true);
}

void TableManager::endCell()
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().element("tablemanager.endCell");
#endif

    setCellEnd(true);
}

void TableManager::inCell()
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().element("tablemanager.inCell");
#endif
    setInCell(true);

    if (mnTableDepthNew < 1)
        mnTableDepthNew = 1;
}

void TableManager::cellDepth(sal_uInt32 nDepth)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablemanager.cellDepth");
    TagLogger::getInstance().attribute("depth", nDepth);
    TagLogger::getInstance().endElement();
#endif

    mnTableDepthNew = nDepth;
}

void TableManager::setTableStartsAtCellStart(bool bTableStartsAtCellStart)
{
    m_bTableStartsAtCellStart = bTableStartsAtCellStart;
}

TableManager::TableManager()
    : mnTableDepthNew(0), mnTableDepth(0), mbKeepUnfinishedRow(false),
      m_bTableStartsAtCellStart(false)
{
    setRowEnd(false);
    setInCell(false);
    setCellEnd(false);
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
