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

#include "TableManager.hxx"
#include <ooxml/resourceids.hxx>
#include "TagLogger.hxx"
#include "DomainMapperTableHandler.hxx"
#include "DomainMapper_Impl.hxx"
#include "util.hxx"

#include <comphelper/diagnose_ex.hxx>

using namespace com::sun::star;

namespace writerfilter::dmapper
{
void TableManager::clearData() {}

void TableManager::openCell(const css::uno::Reference<css::text::XTextRange>& rHandle,
                            const TablePropertyMapPtr& pProps)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.openCell");
    TagLogger::getInstance().chars(XTextRangeToString(rHandle));
    TagLogger::getInstance().endElement();
#endif

    if (!mTableDataStack.empty())
    {
        TableData::Pointer_t pTableData = mTableDataStack.back();

        pTableData->addCell(rHandle, pProps);
    }
}

bool TableManager::isIgnore() const { return isRowEnd(); }

sal_uInt32 TableManager::getGridBefore(sal_uInt32 nRow)
{
    if (!isInTable())
    {
        SAL_WARN("writerfilter", "TableManager::getGridBefore called while not in table");
        return 0;
    }
    if (nRow >= mTableDataStack.back()->getRowCount())
        return 0;
    return mTableDataStack.back()->getRow(nRow)->getGridBefore();
}

sal_uInt32 TableManager::getCurrentGridBefore()
{
    return mTableDataStack.back()->getCurrentRow()->getGridBefore();
}

void TableManager::setCurrentGridBefore(sal_uInt32 nSkipGrids)
{
    mTableDataStack.back()->getCurrentRow()->setGridBefore(nSkipGrids);
}

sal_uInt32 TableManager::getGridAfter(sal_uInt32 nRow)
{
    if (!isInTable())
    {
        SAL_WARN("writerfilter", "TableManager::getGridAfter called while not in table");
        return 0;
    }
    if (nRow >= mTableDataStack.back()->getRowCount())
        return 0;
    return mTableDataStack.back()->getRow(nRow)->getGridAfter();
}

void TableManager::setCurrentGridAfter(sal_uInt32 nSkipGrids)
{
    assert(isInTable());
    mTableDataStack.back()->getCurrentRow()->setGridAfter(nSkipGrids);
}

std::vector<sal_uInt32> TableManager::getCurrentGridSpans()
{
    return mTableDataStack.back()->getCurrentRow()->getGridSpans();
}

void TableManager::setCurrentGridSpan(sal_uInt32 nGridSpan, bool bFirstCell)
{
    mTableDataStack.back()->getCurrentRow()->setCurrentGridSpan(nGridSpan, bFirstCell);
}

sal_uInt32 TableManager::findColumn(const sal_uInt32 nRow, const sal_uInt32 nCell)
{
    if (nRow >= mTableDataStack.back()->getRowCount())
        return SAL_MAX_UINT32;

    RowData::Pointer_t pRow = mTableDataStack.back()->getRow(nRow);
    if (!pRow || nCell < pRow->getGridBefore()
        || nCell >= pRow->getCellCount() - pRow->getGridAfter())
    {
        return SAL_MAX_UINT32;
    }

    // The gridSpans provide a one-based index, so add up all the spans of the PREVIOUS columns,
    // and that result will provide the first possible zero-based number for the desired column.
    sal_uInt32 nColumn = 0;
    for (sal_uInt32 n = 0; n < nCell; ++n)
        nColumn += pRow->getGridSpan(n);
    return nColumn;
}

sal_uInt32 TableManager::findColumnCell(const sal_uInt32 nRow, const sal_uInt32 nCol)
{
    if (nRow >= mTableDataStack.back()->getRowCount())
        return SAL_MAX_UINT32;

    RowData::Pointer_t pRow = mTableDataStack.back()->getRow(nRow);
    if (!pRow || nCol < pRow->getGridBefore())
        return SAL_MAX_UINT32;

    sal_uInt32 nCell = 0;
    sal_uInt32 nGrids = 0;
    // The gridSpans give us a one-based index, but requested column is zero-based - so keep that in mind.
    const sal_uInt32 nMaxCell = pRow->getCellCount() - pRow->getGridAfter() - 1;
    for (const auto& rSpan : pRow->getGridSpans())
    {
        nGrids += rSpan;
        if (nCol < nGrids)
            return nCell;

        ++nCell;
        if (nCell > nMaxCell)
            break;
    }
    return SAL_MAX_UINT32; // must be in gridAfter or invalid column request
}

void TableManager::endOfRowAction() {}

void TableManager::endOfCellAction() {}

void TableManager::insertTableProps(const TablePropertyMapPtr& pProps)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.insertTableProps");
#endif

    if (getTableProps() && getTableProps() != pProps)
        getTableProps()->InsertProps(pProps.get());
    else
        mState.setTableProps(pProps);

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::insertRowProps(const TablePropertyMapPtr& pProps)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.insertRowProps");
#endif

    if (getRowProps())
        getRowProps()->InsertProps(pProps.get());
    else
        mState.setRowProps(pProps);

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::cellProps(const TablePropertyMapPtr& pProps)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.cellProps");
#endif

    if (getCellProps())
        getCellProps()->InsertProps(pProps.get());
    else
        mState.setCellProps(pProps);

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::tableExceptionProps(const TablePropertyMapPtr& pProps)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.tableExceptionProps");
#endif

    if (getTableExceptionProps())
        getTableExceptionProps()->InsertProps(pProps.get());
    else
        mState.setTableExceptionProps(pProps);

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::utext(const sal_Unicode* const data, std::size_t const len)
{
    // optimization: cell/row end characters are the last characters in a run

    if (len > 0)
    {
        sal_Unicode const nChar = data[len - 1];
        if (nChar == 0x7)
            handle0x7();
    }
}

void TableManager::text(const sal_uInt8* data, std::size_t len)
{
    // optimization: cell/row end characters are the last characters in a run
    if (len > 0 && data[len - 1] == 0x7)
        handle0x7();
}

void TableManager::handle0x7()
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.handle0x7");
#endif

    if (mnTableDepthNew < 1)
        mnTableDepthNew = 1;

    if (isInCell())
        endCell();
    else
        endRow();

#ifdef DBG_UTIL
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
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.closeCell");
    TagLogger::getInstance().chars(XTextRangeToString(rHandle));
    TagLogger::getInstance().endElement();
#endif

    if (!mTableDataStack.empty())
    {
        TableData::Pointer_t pTableData = mTableDataStack.back();

        pTableData->endCell(rHandle);

        if (mpTableDataHandler)
            mpTableDataHandler->getDomainMapperImpl().ClearPreviousParagraph();
    }
}

void TableManager::ensureOpenCell(const TablePropertyMapPtr& pProps)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.ensureOpenCell");
#endif

    if (!mTableDataStack.empty())
    {
        TableData::Pointer_t pTableData = mTableDataStack.back();

        if (pTableData != nullptr)
        {
            if (!pTableData->isCellOpen())
                openCell(getHandle(), pProps);
            else
                pTableData->insertCellProperties(pProps);
        }
    }
#ifdef DBG_UTIL
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

    if (mnTableDepth <= 0)
        return;

    if (isRowEnd())
    {
        endOfRowAction();
        mTableDataStack.back()->endRow(getRowProps());
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

void TableManager::startParagraphGroup()
{
    mState.resetCellSpecifics();
    mnTableDepthNew = 0;
}

void TableManager::resolveCurrentTable()
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.resolveCurrentTable");
#endif

    if (mpTableDataHandler != nullptr)
    {
        try
        {
            TableData::Pointer_t pTableData = mTableDataStack.back();

            unsigned int nRows = pTableData->getRowCount();

            mpTableDataHandler->startTable(getTableProps());

            for (unsigned int nRow = 0; nRow < nRows; ++nRow)
            {
                RowData::Pointer_t pRowData = pTableData->getRow(nRow);

                unsigned int nCells = pRowData->getCellCount();

                mpTableDataHandler->startRow(pRowData->getProperties());

                for (unsigned int nCell = 0; nCell < nCells; ++nCell)
                {
                    mpTableDataHandler->startCell(pRowData->getCellStart(nCell),
                                                  pRowData->getCellProperties(nCell));

                    mpTableDataHandler->endCell(pRowData->getCellEnd(nCell));
                }

                mpTableDataHandler->endRow();
            }

            mpTableDataHandler->endTable(mTableDataStack.size() - 1);
        }
        catch (css::uno::Exception const&)
        {
            TOOLS_WARN_EXCEPTION("writerfilter", "resolving of current table failed");
        }
    }
    mState.resetTableProps();
    clearData();

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::endLevel()
{
    uno::Reference<text::XTextCursor> xCursor;
    if (mpTableDataHandler != nullptr)
    {
        if (mTableDataStack.size() > 1)
        {
            // This is an inner table: create a cursor from the outer cell's start position, in case
            // that would become invalid during the current table resolution.
            TableData::Pointer_t pUpperTableData = mTableDataStack[mTableDataStack.size() - 2];
            RowData::Pointer_t pRow = pUpperTableData->getCurrentRow();
            unsigned int nCells = pRow->getCellCount();
            if (nCells > 0)
            {
                uno::Reference<text::XTextRange> xCellStart = pRow->getCellStart(nCells - 1);
                if (xCellStart.is())
                {
                    try
                    {
                        xCursor = xCellStart->getText()->createTextCursorByRange(
                            xCellStart->getStart());
                        if (xCursor.is())
                        {
                            xCursor->goLeft(1, false);
                        }
                    }
                    catch (const uno::RuntimeException&)
                    {
                        TOOLS_WARN_EXCEPTION(
                            "writerfilter",
                            "TableManager::endLevel: createTextCursorByRange() failed");
                    }
                }
            }
        }
        resolveCurrentTable();
    }

    // Store the unfinished row as it will be used for the next table
    if (mbKeepUnfinishedRow)
        mpUnfinishedRow = mTableDataStack.back()->getCurrentRow();
    mState.endLevel();
    mTableDataStack.pop_back();

    TableData::Pointer_t pTableData;

    if (!mTableDataStack.empty())
        pTableData = mTableDataStack.back();

#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.endLevel");
    TagLogger::getInstance().attribute("level", mTableDataStack.size());
#endif

    if (pTableData != nullptr)
    {
#ifdef DBG_UTIL
        TagLogger::getInstance().attribute("openCell", pTableData->isCellOpen() ? "yes" : "no");
#endif
        if (pTableData->isCellOpen() && !pTableData->IsCellValid() && xCursor.is())
        {
            // The inner table is resolved and we have an outer table, but the currently opened
            // cell's start position is no longer valid. Try to move the cursor back to where it was
            // and update the cell start position accordingly.
            try
            {
                xCursor->goRight(1, false);
                pTableData->SetCellStart(xCursor->getStart());
            }
            catch (const uno::RuntimeException&)
            {
                TOOLS_WARN_EXCEPTION("writerfilter", "TableManager::endLevel: goRight() failed");
            }
        }
    }

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void TableManager::startLevel()
{
#ifdef DBG_UTIL
    TableData::Pointer_t pTableData;

    if (!mTableDataStack.empty())
        pTableData = mTableDataStack.back();

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
            pTableData2->addCell(mpUnfinishedRow->getCellStart(i),
                                 mpUnfinishedRow->getCellProperties(i));
            pTableData2->endCell(mpUnfinishedRow->getCellEnd(i));
            pTableData2->getCurrentRow()->setCurrentGridSpan(mpUnfinishedRow->getGridSpan(i));
        }
        pTableData2->getCurrentRow()->setGridBefore(mpUnfinishedRow->getGridBefore());
        pTableData2->getCurrentRow()->setGridAfter(mpUnfinishedRow->getGridAfter());
        mpUnfinishedRow.clear();
    }

    mTableDataStack.push_back(pTableData2);
    mState.startLevel();
}

bool TableManager::isInTable()
{
    bool bInTable = false;
    if (!mTableDataStack.empty())
        bInTable = mTableDataStack.back()->getDepth() > 0;
    return bInTable;
}

void TableManager::handle(const css::uno::Reference<css::text::XTextRange>& rHandle)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.handle");
    TagLogger::getInstance().chars(XTextRangeToString(rHandle));
    TagLogger::getInstance().endElement();
#endif

    setHandle(rHandle);
}

void TableManager::setHandler(const tools::SvRef<DomainMapperTableHandler>& pTableDataHandler)
{
    mpTableDataHandler = pTableDataHandler;
}

void TableManager::endRow()
{
#ifdef DBG_UTIL
    TagLogger::getInstance().element("tablemanager.endRow");
#endif
    TableData::Pointer_t pTableData = mTableDataStack.back();

    // Add borderless w:gridBefore cell(s) to the row
    sal_uInt32 nGridBefore = getCurrentGridBefore();
    if (pTableData && nGridBefore > 0 && pTableData->getCurrentRow()->getCellCount() > 0)
    {
        const css::uno::Reference<css::text::XTextRange>& xRowStart
            = pTableData->getCurrentRow()->getCellStart(0);
        if (xRowStart.is())
        {
            try
            {
                // valid TextRange for table creation (not a nested table)?
                xRowStart->getText()->createTextCursorByRange(xRowStart);

                for (unsigned int i = 0; i < nGridBefore; ++i)
                {
                    css::table::BorderLine2 aBorderLine;
                    aBorderLine.Color = 0;
                    aBorderLine.InnerLineWidth = 0;
                    aBorderLine.OuterLineWidth = 0;
                    TablePropertyMapPtr pCellProperties(new TablePropertyMap);
                    pCellProperties->Insert(PROP_TOP_BORDER, css::uno::Any(aBorderLine));
                    pCellProperties->Insert(PROP_LEFT_BORDER, css::uno::Any(aBorderLine));
                    pCellProperties->Insert(PROP_BOTTOM_BORDER, css::uno::Any(aBorderLine));
                    pCellProperties->Insert(PROP_RIGHT_BORDER, css::uno::Any(aBorderLine));
                    pTableData->getCurrentRow()->addCell(xRowStart, pCellProperties,
                                                         /*bAddBefore=*/true);
                }
            }
            catch (css::uno::Exception const&)
            {
                // don't add gridBefore cells in not valid TextRange
                setCurrentGridBefore(0);
                setCurrentGridSpan(getCurrentGridSpans().front() + nGridBefore,
                                   /*bFirstCell=*/true);
            }
        }
    }

    setRowEnd(true);
}

void TableManager::endCell()
{
#ifdef DBG_UTIL
    TagLogger::getInstance().element("tablemanager.endCell");
#endif

    setCellEnd(true);
}

void TableManager::inCell()
{
#ifdef DBG_UTIL
    TagLogger::getInstance().element("tablemanager.inCell");
#endif
    setInCell(true);

    if (mnTableDepthNew < 1)
        mnTableDepthNew = 1;
}

void TableManager::cellDepth(sal_uInt32 nDepth)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablemanager.cellDepth");
    TagLogger::getInstance().attribute("depth", nDepth);
    TagLogger::getInstance().endElement();
#endif

    mnTableDepthNew = nDepth;
}

void TableManager::setCellLastParaAfterAutospacing(bool bIsAfterAutospacing)
{
    m_bCellLastParaAfterAutospacing = bIsAfterAutospacing;
}

TableManager::TableManager()
    : mnTableDepthNew(0)
    , mnTableDepth(0)
    , mbKeepUnfinishedRow(false)
{
    setRowEnd(false);
    setInCell(false);
    setCellEnd(false);
    m_bCellLastParaAfterAutospacing = false;
}

TableManager::~TableManager() = default;

bool CellData::IsValid() const
{
    if (!mStart.is())
    {
        return false;
    }

    try
    {
        mStart->getStart();
    }
    catch (const uno::RuntimeException&)
    {
        return false;
    }

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
