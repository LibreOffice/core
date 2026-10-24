/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <com/sun/star/awt/grid/GridDataEvent.hpp>
#include <com/sun/star/awt/grid/XMutableGridDataModel.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <test/unoapi_test.hxx>
#include "GridDataListener.hxx"

using namespace cpo::uno;
using namespace css::awt::grid;

static std::ostream& operator<<(std::ostream& s, const std::vector<std::vector<Any>>&) { return s; }
static std::ostream& operator<<(std::ostream& s, const std::vector<Any>&) { return s; }

/** test for the <code>css.awt.grid.XMutableGridData</code> interface
 *
 */
class TMutableGridDataModel
{
public:
    TMutableGridDataModel(const Reference<XMutableGridDataModel>& i_dataModel)
    {
        m_dataModel = i_dataModel;

        m_listener = new GridDataListener;
        m_dataModel->addGridDataListener(m_listener);
    }

    /*
     * tests the XMutableGridDataModel->addRow method
     */
    void testAddRow()
    {
        m_dataModel->addRow(m_rowHeadings[0], m_rowValues[0]);
        GridDataEvent event = m_listener->assertSingleRowInsertionEvent();
        m_listener->reset();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row insertion: wrong FirstRow (1)", sal_Int32(0), event.FirstRow);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row insertion: wrong LastRow (1)", sal_Int32(0), event.LastRow);
        impl_assertRowData(0);

        m_dataModel->addRow(m_rowHeadings[1], m_rowValues[1]);
        event = m_listener->assertSingleRowInsertionEvent();
        m_listener->reset();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row insertion: wrong FirstRow (2)", sal_Int32(1), event.FirstRow);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row insertion: wrong LastRow (2)", sal_Int32(1), event.LastRow);
        impl_assertRowData(1);
    }

    /**
     * tests the XMutableGridDataModel->addRows method
     */
    void testAddRows()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "precondition not met: call this directly after testAddRow, please!", sal_Int32(2),
            m_dataModel->getRowCount());

        m_dataModel->addRows(
            Sequence<Any>{ m_rowHeadings[2], m_rowHeadings[3], m_rowHeadings[4] },
            Sequence<Sequence<Any>>{ m_rowValues[2], m_rowValues[3], m_rowValues[4] });
        GridDataEvent event = m_listener->assertSingleRowInsertionEvent();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row insertion: wrong FirstRow (1)", sal_Int32(2), event.FirstRow);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row insertion: wrong LastRow (1)", sal_Int32(4), event.LastRow);
        m_listener->reset();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("data model's row count is not adjusted when adding rows",
                                     sal_Int32(m_rowValues.size()), m_dataModel->getRowCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("data model's column count is not adjusted when adding rows",
                                     sal_Int32(m_rowValues[0].size()),
                                     m_dataModel->getColumnCount());
        for (size_t row = 0; row < m_rowValues.size(); ++row)
        {
            for (size_t col = 0; col < m_rowValues[row].size(); ++col)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("added row values are not preserved",
                                             m_rowValues[row][col],
                                             m_dataModel->getCellData(col, row));
            }
        }

        CPPUNIT_ASSERT_THROW_MESSAGE(
            "addRows is expected to throw when invoked with different-sized arrays",
            m_dataModel->addRows(Sequence<Any>{}, Sequence<Sequence<Any>>{ { Any(), Any() } }),
            IllegalArgumentException);
    }

    /**
     * tests the XMutableGridDataModel.insertRow method
     */
    void testInsertRow()
    {
        sal_Int32 expectedRowCount = m_rowValues.size();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "precondition not met: call this directly after testAddRows, please!", expectedRowCount,
            m_dataModel->getRowCount());

        // inserting some row somewhere between the other rows
        const Any heading(u"inbetweenRow"_ustr);
        const Sequence<Any> inbetweenRow{ Any(u"foo"_ustr), Any(u"bar"_ustr), Any(sal_Int32(3)), Any(sal_Int32(4)),
                                          Any(sal_Int32(5)) };
        const sal_Int32 insertionPos = 2;
        m_dataModel->insertRow(insertionPos, heading, inbetweenRow);
        ++expectedRowCount;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("inserting a row is expected to increment the row count",
                                     expectedRowCount, m_dataModel->getRowCount());

        const GridDataEvent event = m_listener->assertSingleRowInsertionEvent();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("inserting a row results in wrong FirstRow being notified",
                                     insertionPos, event.FirstRow);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("inserting a row results in wrong LastRow being notified",
                                     insertionPos, event.LastRow);
        m_listener->reset();

        for (sal_Int32 row = 0; row < expectedRowCount; ++row)
        {
            const Sequence<Any> actualRowData = m_dataModel->getRowData(row);
            const Sequence<Any> expectedRowData = (row < insertionPos)    ? m_rowValues[row]
                                                  : (row == insertionPos) ? inbetweenRow
                                                                          : m_rowValues[row - 1];
            CPPUNIT_ASSERT_EQUAL_MESSAGE("row number " + std::to_string(row)
                                             + " has wrong content after inserting a row",
                                         expectedRowData, actualRowData);

            const Any actualHeading = m_dataModel->getRowHeading(row);
            const Any expectedHeading = (row < insertionPos)    ? m_rowHeadings[row]
                                        : (row == insertionPos) ? heading
                                                                : m_rowHeadings[row - 1];
            CPPUNIT_ASSERT_EQUAL_MESSAGE("row " + std::to_string(row)
                                             + " has a wrong heading after invoking insertRow",
                                         expectedHeading, actualHeading);
        }

        // exceptions
        CPPUNIT_ASSERT_THROW_MESSAGE(
            "inserting a row at a position > rowCount is expected to throw",
            m_dataModel->insertRow(expectedRowCount + 1, Any(u""_ustr),
                                   Sequence<Any>{ Any(u"1"_ustr), Any(sal_Int32(2)), Any(sal_Int32(3)) }),
            IndexOutOfBoundsException);
        CPPUNIT_ASSERT_THROW_MESSAGE(
            "inserting a row at a position < 0 is expected to throw",
            m_dataModel->insertRow(-1, Any(u""_ustr),
                                   Sequence<Any>{ Any(u"1"_ustr), Any(sal_Int32(2)), Any(sal_Int32(3)) }),
            IndexOutOfBoundsException);

        // remove the row, to create the situation expected by the next test
        m_dataModel->removeRow(insertionPos);
        m_listener->reset();
    }

    /**
     * tests the XMutableGridDataModel.insertRows method
     */
    void testInsertRows()
    {
        sal_Int32 expectedRowCount = m_rowValues.size();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "precondition not met: call this directly after testInsertRow, please!",
            expectedRowCount, m_dataModel->getRowCount());

        // inserting some rows somewhere between the other rows
        const sal_Int32 insertionPos = 3;
        const Sequence<Any> rowHeadings{ Any(u"A"_ustr), Any(u"B"_ustr), Any(u"C"_ustr) };
        const Sequence<Sequence<Any>> rowData{
            { Any(u"A"_ustr), Any(u"B"_ustr), Any(u"C"_ustr), Any(u"D"_ustr), Any(u"E"_ustr) },
            { Any(u"J"_ustr), Any(u"I"_ustr), Any(u"H"_ustr), Any(u"G"_ustr), Any(u"F"_ustr) },
            { Any(u"K"_ustr), Any(u"L"_ustr), Any(u"M"_ustr), Any(u"N"_ustr), Any(u"O"_ustr) }
        };
        const sal_Int32 insertedRowCount = rowData.size();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("invalid test data", sal_Int32(rowHeadings.size()),
                                     insertedRowCount);

        m_dataModel->insertRows(insertionPos, rowHeadings, rowData);
        expectedRowCount += insertedRowCount;

        const GridDataEvent event = m_listener->assertSingleRowInsertionEvent();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "inserting multiple rows results in wrong FirstRow being notified", insertionPos,
            event.FirstRow);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "inserting multiple rows results in wrong LastRow being notified",
            insertionPos + insertedRowCount - 1, event.LastRow);
        m_listener->reset();

        for (sal_Int32 row = 0; row < expectedRowCount; ++row)
        {
            const Sequence<Any> actualRowData = m_dataModel->getRowData(row);
            const Sequence<Any> expectedRowData
                = (row < insertionPos) ? m_rowValues[row]
                  : (row >= insertionPos) && (row < insertionPos + insertedRowCount)
                      ? rowData[row - insertionPos]
                      : m_rowValues[row - insertedRowCount];
            CPPUNIT_ASSERT_EQUAL_MESSAGE("row number " + std::to_string(row)
                                             + " has wrong content after inserting multiple rows",
                                         expectedRowData, actualRowData);

            const Any actualHeading = m_dataModel->getRowHeading(row);
            const Any expectedHeading
                = (row < insertionPos) ? m_rowHeadings[row]
                  : (row >= insertionPos) && (row < insertionPos + insertedRowCount)
                      ? rowHeadings[row - insertionPos]
                      : m_rowHeadings[row - insertedRowCount];
            CPPUNIT_ASSERT_EQUAL_MESSAGE("row " + std::to_string(row)
                                             + " has a wrong heading after invoking insertRows",
                                         expectedHeading, actualHeading);
        }

        // exceptions
        CPPUNIT_ASSERT_THROW_MESSAGE("inserting multiple rows at a position > rowCount is expected "
                                     "to throw an IndexOutOfBoundsException",
                                     m_dataModel->insertRows(expectedRowCount + 1, Sequence<Any>{},
                                                             Sequence<Sequence<Any>>{}),
                                     IndexOutOfBoundsException);
        CPPUNIT_ASSERT_THROW_MESSAGE(
            "inserting multiple rows at a position < 0 is expected to throw an "
            "IndexOutOfBoundsException",
            m_dataModel->insertRows(-1, Sequence<Any>{}, Sequence<Sequence<Any>>{}),
            IndexOutOfBoundsException);
        CPPUNIT_ASSERT_THROW_MESSAGE(
            "inserting multiple rows with inconsistent array lengths is expected to throw an "
            "IllegalArgumentException",
            m_dataModel->insertRows(0, Sequence<Any>{}, Sequence<Sequence<Any>>{ {} }),
            IllegalArgumentException);

        // remove the row, to create the situation expected by the next test
        for (sal_Int32 i = 0; i < insertedRowCount; ++i)
        {
            m_dataModel->removeRow(insertionPos);
            m_listener->reset();
        }
    }

    /**
     * tests the XMutableGridDataModel.removeRow method
     */
    void testRemoveRow()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "precondition not met: call this directly after testAddRows, please!",
            sal_Int32(m_rowValues.size()), m_dataModel->getRowCount());

        const sal_Int32 rowToRemove = 2;
        m_dataModel->removeRow(rowToRemove);
        GridDataEvent event = m_listener->assertSingleRowRemovalEvent();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect notification of row removal (FirstRow)",
                                     rowToRemove, event.FirstRow);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("incorrect notification of row removal (LastRow)", rowToRemove,
                                     event.LastRow);
        m_listener->reset();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("data model's row count does not reflect the removed row",
                                     sal_Int32(m_rowValues.size() - 1), m_dataModel->getRowCount());
        for (size_t row = rowToRemove; row < m_rowValues.size() - 1; ++row)
        {
            for (size_t col = 0; col < m_rowValues[row].size(); ++col)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(
                    "unexpected row values after removing a row (col: " + std::to_string(col)
                        + ", row: " + std::to_string(row) + ")",
                    m_rowValues[row + 1][col], m_dataModel->getCellData(col, row));
            }
        }

        CPPUNIT_ASSERT_THROW_MESSAGE("removeRow silently ignores an invalid index (1)",
                                     m_dataModel->removeRow(-1), IndexOutOfBoundsException);
        CPPUNIT_ASSERT_THROW_MESSAGE("removeRow silently ignores an invalid index (2)",
                                     m_dataModel->removeRow(m_dataModel->getRowCount()),
                                     IndexOutOfBoundsException);
    }

    /**
     * tests the XMutableGridDataModel.removeAllRows method
     */
    void testRemoveAllRows()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "precondition not met: call this directly after testRemoveRow, please!",
            sal_Int32(m_rowValues.size() - 1), m_dataModel->getRowCount());

        m_dataModel->removeAllRows();
        const GridDataEvent event = m_listener->assertSingleRowRemovalEvent();
        if (event.FirstRow != -1)
        { // notifying "-1" is allowed, this means "all rows affected", by definition
            CPPUNIT_ASSERT_EQUAL_MESSAGE("removeAllRows is not notifying properly (1)", sal_Int32(0),
                                         event.FirstRow);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("removeAllRows is not notifying properly (2)",
                                         sal_Int32(m_rowValues.size() - 1), event.LastRow);
        }
        m_listener->reset();
    }

    /**
     * tests the XMutableGridDataModel.updateCellData method
     */
    void testUpdateCellData()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "precondition not met: call this directly after testRemoveAllRows, please!", sal_Int32(0),
            m_dataModel->getRowCount());

        m_dataModel->addRows(m_rowHeadings, m_rowValues);
        m_listener->assertSingleRowInsertionEvent();
        m_listener->reset();

        const Sequence<Sequence<Any>> modifyValues{ { Any(sal_Int32(2)), Any(sal_Int32(1)), Any(u"text"_ustr) },
                                                    { Any(sal_Int32(3)), Any(sal_Int32(0)), Any() },
                                                    { Any(sal_Int32(0)), Any(sal_Int32(4)), Any(33.0) } };
        for (size_t i = 0; i < modifyValues.size(); ++i)
        {
            const sal_Int32 row = modifyValues[i][0].get<sal_Int32>();
            const sal_Int32 col = modifyValues[i][1].get<sal_Int32>();
            const Any value = modifyValues[i][2];
            m_dataModel->updateCellData(col, row, value);

            const GridDataEvent event = m_listener->assertSingleDataChangeEvent();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("data change notification: FirstRow is invalid", row,
                                         event.FirstRow);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("data change notification: LastRow is invalid", row,
                                         event.LastRow);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("data change notification: FirstColumn is invalid", col,
                                         event.FirstColumn);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("data change notification: LastColumn is invalid", col,
                                         event.LastColumn);
            m_listener->reset();

            CPPUNIT_ASSERT_EQUAL_MESSAGE("data change at (" + std::to_string(col) + ", "
                                             + std::to_string(row) + ") not successful",
                                         value, m_dataModel->getCellData(col, row));
        }

        CPPUNIT_ASSERT_THROW_MESSAGE("updateCellData silently ignores an invalid index (1)",
                                     m_dataModel->updateCellData(-1, -1, Any(u"text"_ustr)),
                                     IndexOutOfBoundsException);
        CPPUNIT_ASSERT_THROW_MESSAGE(
            "updateCellData silently ignores an invalid index (2)",
            m_dataModel->updateCellData(0, m_dataModel->getRowCount(), Any(u"text"_ustr)),
            IndexOutOfBoundsException);
        CPPUNIT_ASSERT_THROW_MESSAGE(
            "updateCellData silently ignores an invalid index (3)",
            m_dataModel->updateCellData(m_dataModel->getColumnCount(), 0, Any(u"text"_ustr)),
            IndexOutOfBoundsException);
    }

    /**
     * tests the XMutableGridDataModel.updateRowData method
     */
    void testUpdateRowData()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "precondition not met: call this directly after testRemoveAllRows, please!",
            sal_Int32(m_rowValues.size()), m_dataModel->getRowCount());

        // get data from before the update
        std::vector<std::vector<Any>> preUpdateValues = impl_getCurrentData();

        // do the update
        const Sequence<sal_Int32> colIndexes{ 0, 3, 4 };
        const Sequence<Any> values = { Any(sal_Int32(13)), Any(), Any(42.0) };
        const sal_Int32 rowToUpdate = 2;
        m_dataModel->updateRowData(colIndexes, rowToUpdate, values);
        const GridDataEvent event = m_listener->assertSingleDataChangeEvent();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row update notification: FirstRow is invalid", rowToUpdate,
                                     event.FirstRow);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row update notification: LastRow is invalid", rowToUpdate,
                                     event.LastRow);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row update notification: FirstColumn is invalid", sal_Int32(0),
                                     event.FirstColumn);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row update notification: LastColumn is invalid", sal_Int32(4),
                                     event.LastColumn);
        m_listener->reset();

        // reflect the changes made in the pre-update data
        for (size_t i = 0; i < colIndexes.size(); ++i)
        {
            preUpdateValues[rowToUpdate][colIndexes[i]] = values[i];
        }

        // get data from after the update
        const std::vector<std::vector<Any>> postUpdateValues = impl_getCurrentData();

        // ensure both the manually updated pre-update data and the post-update data are identical
        CPPUNIT_ASSERT_EQUAL(preUpdateValues, postUpdateValues);

        CPPUNIT_ASSERT_THROW_MESSAGE("updateRowData silently ignores an invalid index (1)",
                                     m_dataModel->updateRowData(Sequence<sal_Int32>{ -1 }, 0,
                                                                Sequence<Any>{ Any(u"text"_ustr) }),
                                     IndexOutOfBoundsException);
        CPPUNIT_ASSERT_THROW_MESSAGE("updateRowData silently ignores an invalid index (2)",
                                     m_dataModel->updateRowData(Sequence<sal_Int32>{ 0 }, -1,
                                                                Sequence<Any>{ Any(u""_ustr) }),
                                     IndexOutOfBoundsException);
        CPPUNIT_ASSERT_THROW_MESSAGE("updateRowData silently ignores different-sized arrays",
                                     m_dataModel->updateRowData(Sequence<sal_Int32>{ 0, 0 }, 0,
                                                                Sequence<Any>{ Any(u""_ustr) }),
                                     IllegalArgumentException);
    }

    /**
     * tests the XMutableGridDataModel.updateRowHeading method
     */
    void testUpdateRowHeading()
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "precondition not met: call this directly after testUpdateRowData, please!",
            sal_Int32(m_rowValues.size()), m_dataModel->getRowCount());

        std::vector<Any> preUpdateHeadings = impl_getCurrentRowHeadings();

        const sal_Int32 rowToUpdate = 2;
        const Any valueToUpdate(u"some text"_ustr);
        m_dataModel->updateRowHeading(rowToUpdate, valueToUpdate);
        const GridDataEvent event = m_listener->assertSingleRowHeadingChangeEvent();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row heading update notification: FirstRow is invalid",
                                     rowToUpdate, event.FirstRow);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("row heading update notification: FirstRow is invalid",
                                     rowToUpdate, event.LastRow);
        m_listener->reset();

        preUpdateHeadings[rowToUpdate] = valueToUpdate;

        const std::vector<Any> postUpdateHeadings = impl_getCurrentRowHeadings();
        CPPUNIT_ASSERT_EQUAL(preUpdateHeadings, postUpdateHeadings);

        CPPUNIT_ASSERT_THROW_MESSAGE("updateRowHeading silently ignores an invalid index",
                                     m_dataModel->updateRowHeading(-1, Any(u""_ustr)),
                                     IndexOutOfBoundsException);
    }

    void cleanup() { m_dataModel->removeGridDataListener(m_listener); }

private:
    std::vector<std::vector<Any>> impl_getCurrentData()
    {
        const sal_Int32 rowCount = m_dataModel->getRowCount();
        const sal_Int32 colCount = m_dataModel->getColumnCount();
        std::vector<std::vector<Any>> data(rowCount, std::vector<Any>(colCount));
        for (sal_Int32 row = 0; row < rowCount; ++row)
        {
            for (sal_Int32 col = 0; col < colCount; ++col)
            {
                data[row][col] = m_dataModel->getCellData(col, row);
            }
        }
        return data;
    }

    std::vector<Any> impl_getCurrentRowHeadings()
    {
        const sal_Int32 rowCount = m_dataModel->getRowCount();
        std::vector<Any> headings(rowCount);
        for (sal_Int32 row = 0; row < rowCount; ++row)
            headings[row] = m_dataModel->getRowHeading(row);
        return headings;
    }

    void impl_assertRowData(const int i_rowIndex)
    {
        for (size_t i = 0; i < m_rowValues[i_rowIndex].size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(m_rowValues[i_rowIndex][i],
                                 m_dataModel->getCellData(i, i_rowIndex));
        }
    }

    Reference<XMutableGridDataModel> m_dataModel;
    rtl::Reference<GridDataListener> m_listener;

    const Sequence<Sequence<Any>> m_rowValues{
        { Any(sal_Int32(1)), Any(sal_Int32(2)), Any(u"3"_ustr), Any(sal_Int32(4)), Any(sal_Int32(5)) },
        { Any(sal_Int32(2)), Any(sal_Int32(3)), Any(sal_Int32(4)), Any(u"5"_ustr), Any(sal_Int32(6)) },
        { Any(u"3"_ustr), Any(sal_Int32(4)), Any(sal_Int32(5)), Any(sal_Int32(6)), Any(sal_Int32(7)) },
        { Any(sal_Int32(4)), Any(sal_Int32(5)), Any(sal_Int32(6)), Any(sal_Int32(7)), Any(u"8"_ustr) },
        { Any(sal_Int32(5)), Any(u"6"_ustr), Any(sal_Int32(7)), Any(sal_Int32(8)), Any(sal_Int32(9)) },
    };

    const Sequence<Any> m_rowHeadings{ Any(u"1"_ustr), Any(sal_Int32(2)), Any(3.0), Any(u"4"_ustr),
                                       Any(float(5.0)) };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
