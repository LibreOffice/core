/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/grid/DefaultGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/grid/XSortableMutableGridDataModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/random.hxx>
#include <test/unoapi_test.hxx>
#include "TMutableGridDataModel.hxx"
#include "DummyColumn.hxx"

using namespace ::cpo::uno;
using namespace ::css::awt;
using namespace ::css::awt::grid;
using namespace ::css::beans;
using namespace ::css::container;
using namespace ::css::lang;
using namespace ::css::util;

namespace
{

class DisposeListener : public cppu::WeakImplHelper<XEventListener>
{
public:
    DisposeListener(const Reference<XInterface>& i_component)
    {
        m_component.set(i_component, UNO_QUERY);
        CPPUNIT_ASSERT(m_component);
        m_component->addEventListener(this);
    }

    virtual void disposing(const EventObject& i_event) override
    {
        CPPUNIT_ASSERT(i_event.Source == m_component);
        m_isDisposed = true;
    }

    bool isDisposed() { return m_isDisposed; }

private:
    Reference<XComponent> m_component;
    bool m_isDisposed { false };
};

class ColumnModelListener : public cppu::WeakImplHelper<XContainerListener>
{
public:
    virtual void elementInserted(const ContainerEvent& i_event) override
    {
        m_insertionEvents.push_back(i_event);
    }

    virtual void elementRemoved(const ContainerEvent& i_event) override
    {
        m_removalEvents.push_back(i_event);
    }

    virtual void elementReplaced(const ContainerEvent& i_event) override
    {
        m_replacementEvents.push_back(i_event);
    }

    virtual void disposing(const EventObject&) override {}

    std::vector<ContainerEvent> assertExclusiveInsertionEvents()
    {
        CPPUNIT_ASSERT(!m_insertionEvents.empty());
        CPPUNIT_ASSERT(m_removalEvents.empty());
        CPPUNIT_ASSERT(m_replacementEvents.empty());
        return m_insertionEvents;
    }

    std::vector<ContainerEvent> assertExclusiveRemovalEvents()
    {
        CPPUNIT_ASSERT(m_insertionEvents.empty());
        CPPUNIT_ASSERT(!m_removalEvents.empty());
        CPPUNIT_ASSERT(m_replacementEvents.empty());
        return m_removalEvents;
    }

    void reset()
    {
        m_insertionEvents.clear();
        m_removalEvents.clear();
        m_replacementEvents.clear();
    }

    const std::vector<ContainerEvent>& getInsertionEvents() const { return m_insertionEvents; }
    const std::vector<ContainerEvent>& getRemovalEvents() const { return m_removalEvents; }

private:
    std::vector<ContainerEvent> m_insertionEvents;
    std::vector<ContainerEvent> m_removalEvents;
    std::vector<ContainerEvent> m_replacementEvents;
};

class GridControlTest : public UnoApiTest
{
    Reference<XPropertySet> m_gridControlModel;
    Reference<XGridColumnModel> m_columnModel;
    Reference<XSortableMutableGridDataModel> m_dataModel;
    std::vector<Reference<XInterface>> m_disposables;

public:
    GridControlTest()
        : UnoApiTest(u"/toolkit/qa/cppunit/data"_ustr)
    {
    }

    virtual void setUp() override;
    virtual void tearDown() override;

    void testGridControlCloning();
    void testDisposal();
    void testMutableGridDataModel();
    void testGridColumnModel();
    void testDataModel();
    void testSortableDataModel();
    void testView();

    void impl_recreateGridModel();
    Reference<XInterface> createInstance(const OUString& i_serviceName);
    template <class T> Reference<T> createInstance(const OUString& i_serviceIndentifier)
    {
        return Reference<T>(createInstance(i_serviceIndentifier), UNO_QUERY);
    }
    void impl_assertEquality(const Reference<XGridDataModel>& i_reference,
                             const Reference<XGridDataModel>& i_compare);
    void impl_assertEquality(const Reference<XGridColumnModel>& i_reference,
                             const Reference<XGridColumnModel>& i_compare);
    void impl_assertEquality(const Reference<XGridColumn>& i_reference,
                             const Reference<XGridColumn>& i_compare);
    bool impl_areSameInterface(const Reference<XInterface>& i_lhs,
                               const Reference<XInterface>& i_rhs);
    bool impl_areSameInterface(const Any& i_lhs, const Any& i_rhs);
    void impl_assertColumnModelConsistency();
    int impl_assertInteger(const Any& i_object);
    Reference<XControl> impl_createDialogWithGridControl();

    CPPUNIT_TEST_SUITE(GridControlTest);
    CPPUNIT_TEST(testGridControlCloning);
    CPPUNIT_TEST(testDisposal);
    CPPUNIT_TEST(testMutableGridDataModel);
    CPPUNIT_TEST(testGridColumnModel);
    CPPUNIT_TEST(testDataModel);
    CPPUNIT_TEST(testSortableDataModel);
    CPPUNIT_TEST(testView);
    CPPUNIT_TEST_SUITE_END();
};

void impl_dispose(const Reference<XInterface>& i_component)
{
    if (i_component)
    {
        Reference<XComponent> component(i_component, UNO_QUERY);
        component->dispose();
    }
}

void GridControlTest::impl_recreateGridModel()
{
    impl_dispose(m_gridControlModel);
    impl_dispose(m_columnModel);
    impl_dispose(m_dataModel);

    // create a grid control model, and ensure it has a proper data and column model already
    m_gridControlModel.set(createInstance(u"com.sun.star.awt.grid.UnoControlGridModel"_ustr), UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("grid control model does not provide XPropertySet interface",
                           m_gridControlModel != nullptr);

    // ensure that the model has default column/data models
    m_columnModel.set(m_gridControlModel->getPropertyValue(u"ColumnModel"_ustr), UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("the control model is expected to have an initial column model",
                           m_columnModel != nullptr);
    Reference<XGridDataModel> dataModel(m_gridControlModel->getPropertyValue(u"GridDataModel"_ustr),
                                        UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("the control model is expected to have an initial data model",
                           dataModel != nullptr);
    m_dataModel.set(dataModel, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("the out-of-the-box data model should be mutable and sortable",
                           m_dataModel != nullptr);
}

void GridControlTest::testGridControlCloning()
{
    impl_recreateGridModel();

    // give the test something to compare, actually
    Reference<XGridColumnModel> columnModel(m_gridControlModel->getPropertyValue(u"ColumnModel"_ustr),
                                            UNO_QUERY);
    columnModel->setDefaultColumns(10);

    // clone the grid model
    Reference<XCloneable> cloneable(m_gridControlModel, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("all UnoControlModel's are expected to be clonable",
                           cloneable != nullptr);

    Reference<XInterface> clone = cloneable->createClone();
    Reference<XPropertySet> clonedProps(clone, UNO_QUERY);

    // TODO: check all those generic properties for equality

    // the data model and the column model should have been cloned, too
    // in particular, the clone should not share the sub models with the original
    Reference<XMutableGridDataModel> originalDataModel(
        m_gridControlModel->getPropertyValue(u"GridDataModel"_ustr), UNO_QUERY);
    Reference<XMutableGridDataModel> clonedDataModel(clonedProps->getPropertyValue(u"GridDataModel"_ustr),
                                                     UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("data model should not be shared after cloning",
                           originalDataModel != clonedDataModel);
    impl_assertEquality(originalDataModel, clonedDataModel);

    Reference<XGridColumnModel> originalColumnModel = columnModel;
    Reference<XGridColumnModel> clonedColumnModel(clonedProps->getPropertyValue(u"ColumnModel"_ustr),
                                                  UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("column model should not be shared after cloning",
                           originalColumnModel != clonedColumnModel);
    impl_assertEquality(originalColumnModel, clonedColumnModel);
}

void GridControlTest::testDisposal()
{
    impl_recreateGridModel();

    const sal_Int32 columnCount = 3;
    m_columnModel->setDefaultColumns(columnCount);

    // add disposal listeners to all columns so far
    Sequence<Reference<XGridColumn>> columns = m_columnModel->getColumns();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("creating default columns resulted in unexpected column count",
                                 columnCount, columns.getLength());
    std::vector<Reference<DisposeListener>> columnListeners(columnCount);
    for (int i = 0; i < columnCount; ++i)
        columnListeners[i] = new DisposeListener(columns[i]);

    // add another column, and check that upon removal, it is disposed
    const sal_Int32 newColumnIndex = m_columnModel->addColumn(m_columnModel->createColumn());
    const rtl::Reference<DisposeListener> columnListener
        = new DisposeListener(m_columnModel->getColumn(newColumnIndex));
    m_columnModel->removeColumn(newColumnIndex);
    CPPUNIT_ASSERT_MESSAGE("explicit column removal is expected to dispose the column",
                           columnListener->isDisposed());

    // by definition, the grid control model is the owner of both the column and the data model. So, setting
    // a new column/data model should implicitly dispose the old models
    const rtl::Reference<DisposeListener> oldDataModelListener = new DisposeListener(m_dataModel);
    const rtl::Reference<DisposeListener> oldColumnModelListener = new DisposeListener(m_columnModel);

    const Reference<XInterface> newDataModel
        = createInstance(u"com.sun.star.awt.grid.DefaultGridDataModel"_ustr);
    const Reference<XInterface> newColumnModel
        = createInstance(u"com.sun.star.awt.grid.DefaultGridColumnModel"_ustr);
    const rtl::Reference<DisposeListener> newDataModelListener = new DisposeListener(newDataModel);
    const rtl::Reference<DisposeListener> newColumnModelListener = new DisposeListener(newColumnModel);

    m_gridControlModel->setPropertyValue(u"GridDataModel"_ustr, Any(newDataModel));
    CPPUNIT_ASSERT_MESSAGE(
        "setting a new data model failed",
        impl_areSameInterface(Any(newDataModel),
                              m_gridControlModel->getPropertyValue(u"GridDataModel"_ustr)));
    m_gridControlModel->setPropertyValue(u"ColumnModel"_ustr, Any(newColumnModel));
    CPPUNIT_ASSERT_MESSAGE(
        "setting a new column model failed",
        impl_areSameInterface(Any(newColumnModel),
                              m_gridControlModel->getPropertyValue(u"ColumnModel"_ustr)));

    CPPUNIT_ASSERT_MESSAGE("old data model has not been disposed",
                           oldDataModelListener->isDisposed());
    CPPUNIT_ASSERT_MESSAGE("old column model has not been disposed",
                           oldColumnModelListener->isDisposed());
    for (int i = 0; i < columnCount; ++i)
        CPPUNIT_ASSERT_MESSAGE("column no. " + std::to_string(i) + " has not been disposed",
                               columnListeners[i]->isDisposed());

    // the same holds if the grid control model itself is disposed - it should dispose the depending models, too
    CPPUNIT_ASSERT_MESSAGE("new data model is already disposed - this is unexpected",
                           !newDataModelListener->isDisposed());
    CPPUNIT_ASSERT_MESSAGE("new column model is already disposed - this is unexpected",
                           !newColumnModelListener->isDisposed());
    impl_dispose(m_gridControlModel);
    CPPUNIT_ASSERT_MESSAGE("new data model is not disposed after disposing the grid column model",
                           newDataModelListener->isDisposed());
    CPPUNIT_ASSERT_MESSAGE("new column model is not disposed after disposing the grid column model",
                           newColumnModelListener->isDisposed());
}

/**
 * tests various aspects of the <code>XMutableGridDataModel</code> interface
 */
void GridControlTest::testMutableGridDataModel()
{
    impl_recreateGridModel();

    TMutableGridDataModel test(m_dataModel);
    test.testAddRow();
    test.testAddRows();
    test.testInsertRow();
    test.testInsertRows();
    test.testRemoveRow();
    test.testRemoveAllRows();
    test.testUpdateCellData();
    test.testUpdateRowData();
    test.testUpdateRowHeading();
    test.cleanup();

    // a somewhat less straight-forward test: the data model is expected to implicitly increase its column count
    // when you add a row which has more columns than currently known
    const Reference<XMutableGridDataModel> dataModel = DefaultGridDataModel::create(m_xContext);
    dataModel->addRow(Any(sal_Int32(0)), Sequence<Any>{ Any(sal_Int32(1)) });
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected column count after adding the most simple row", sal_Int32(1),
                                 dataModel->getColumnCount());
    dataModel->addRow(Any(sal_Int32(1)), Sequence<Any>{ Any(sal_Int32(1)), Any(sal_Int32(2)) });
    CPPUNIT_ASSERT_EQUAL_MESSAGE("implicit extension of the column count doesn't work", sal_Int32(2),
                                 dataModel->getColumnCount());
}

void GridControlTest::testGridColumnModel()
{
    impl_recreateGridModel();

    rtl::Reference<ColumnModelListener> listener = new ColumnModelListener;
    m_columnModel->addContainerListener(listener);

    // insert default columns into the previously empty model, ensure we get the right notifications
    const sal_Int32 defaultColumnsCount = 3;
    m_columnModel->setDefaultColumns(defaultColumnsCount);
    impl_assertColumnModelConsistency();
    std::vector<ContainerEvent> events = listener->assertExclusiveInsertionEvents();
    listener->reset();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong number of events fired by setDefaultColumns",
                                 defaultColumnsCount, sal_Int32(events.size()));
    for (sal_Int32 i = 0; i < defaultColumnsCount; ++i)
    {
        const ContainerEvent& event = events[i];
        const sal_Int32 index = impl_assertInteger(event.Accessor);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected Accessor value in insert notification", i, index);
        CPPUNIT_ASSERT_MESSAGE(
            "wrong column object notified in insert notification",
            impl_areSameInterface(event.Element, Any(m_columnModel->getColumn(i))));
    }

    // insert some more default columns, ensure that all previously existing columns are removed
    const sal_Int32 moreDefaultColumnsCount = 5;
    m_columnModel->setDefaultColumns(moreDefaultColumnsCount);
    impl_assertColumnModelConsistency();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "setting default columns is expected to remove all previously existing columns",
        moreDefaultColumnsCount, m_columnModel->getColumnCount());

    // in this situation, both removal and insertion events have been notified
    const std::vector<ContainerEvent> removalEvents = listener->getRemovalEvents();
    const std::vector<ContainerEvent> insertionEvents = listener->getInsertionEvents();
    listener->reset();

    // for the removal events, check the indexes
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "wrong number of columns removed (or notified) upon setting default columns",
        defaultColumnsCount, sal_Int32(removalEvents.size()));
    for (size_t i = 0; i < removalEvents.size(); ++i)
    {
        const ContainerEvent event = removalEvents[i];
        const sal_Int32 removedIndex = impl_assertInteger(event.Accessor);

        // The implementation is allowed to remove the columns from the beginning, in which case the
        // index of the removed column must always be 0, since e.g. the second column has index 0
        // after the first column (which previously had index 0) had been removed.
        // Alternatively, the implementation is allowed to remove columns from the end, which means
        // that the column index given in the event is steadily increasing.
        CPPUNIT_ASSERT_MESSAGE(
            "unexpected column removal event column index",
            (removedIndex == 0)
                || (removedIndex == (sal_Int32(removalEvents.size()) - 1 - sal_Int32(i))));
    }

    // for the insertion events, check the indexes as well
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "wrong number of insertion events when setting default columns over existing columns",
        moreDefaultColumnsCount, sal_Int32(insertionEvents.size()));
    for (size_t i = 0; i < insertionEvents.size(); ++i)
    {
        const ContainerEvent event = insertionEvents[i];
        const sal_Int32 index = impl_assertInteger(event.Accessor);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(i), index);
    }

    // okay, remove all those columns
    while (m_columnModel->getColumnCount() != 0)
    {
        const int columnCount = m_columnModel->getColumnCount();
        const int removeColumnIndex = comphelper::rng::uniform_int_distribution(0, columnCount - 1);
        m_columnModel->removeColumn(removeColumnIndex);
        events = listener->assertExclusiveRemovalEvents();
        listener->reset();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("removing a single column should notify a single event", sal_Int32(1),
                                     sal_Int32(events.size()));
        const ContainerEvent& event = events[0];
        const int removalIndex = impl_assertInteger(event.Accessor);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "removing an arbitrary column does not notify the proper accessor", removeColumnIndex,
            removalIndex);
    }

    // calling addColumn with a column not created by the given model/implementation should not succeed
    bool caughtExpected = false;
    try
    {
        m_columnModel->addColumn(new DummyColumn);
    }
    catch (const IllegalArgumentException e)
    {
        CPPUNIT_ASSERT(impl_areSameInterface(e.Context, m_columnModel));
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE(
        "adding a dummy (self-implemented) grid column to the model should not succeed",
        caughtExpected);

    // adding a single column to the end should succeed, properly notify, and still be consistent
    const Reference<XGridColumn> newColumn = m_columnModel->createColumn();
    m_columnModel->addColumn(newColumn);
    impl_assertColumnModelConsistency();
    events = listener->assertExclusiveInsertionEvents();
    listener->reset();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("addColumn notifies the wrong number of insertion events", sal_Int32(1),
                                 sal_Int32(events.size()));
    const sal_Int32 insertionIndex = impl_assertInteger(events[0].Accessor);
    CPPUNIT_ASSERT_EQUAL(insertionIndex, newColumn->getIndex());
}

void GridControlTest::testDataModel()
{
    impl_recreateGridModel();

    // ensure that getCellData and getRowData have the same opinion on the data they deliver
    const Sequence<Sequence<Any>> data{
        { Any(sal_Int32(15)), Any(sal_Int32(17)), Any(sal_Int32(0)) },
        { Any(sal_Int32(9)), Any(sal_Int32(8)), Any(sal_Int32(14)) },
        { Any(sal_Int32(17)), Any(sal_Int32(2)), Any(sal_Int32(16)) },
        { Any(sal_Int32(0)), Any(sal_Int32(7)), Any(sal_Int32(14)) },
        { Any(sal_Int32(10)), Any(sal_Int32(16)), Any(sal_Int32(16)) },
    };
    m_dataModel->addRows(Sequence<Any>(data.getLength()), data);

    for (sal_Int32 row = 0; row < data.getLength(); ++row)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getRowData delivers wrong data in row " + std::to_string(row),
                                     data[row], m_dataModel->getRowData(row));
        for (sal_Int32 col = 0; col < data[row].getLength(); ++col)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("getCellData delivers wrong data at position ("
                                             + std::to_string(col) + ", " + std::to_string(row)
                                             + ")",
                                         data[row][col], m_dataModel->getCellData(col, row));
        }
    }
}

void GridControlTest::testSortableDataModel()
{
    impl_recreateGridModel();

    const sal_Int32 colCount = 3;
    const sal_Int32 rowCount = 10;
    // initialize with some data
    const Sequence<Sequence<Any>> data{
        { Any(sal_Int32(15)), Any(sal_Int32(17)), Any(sal_Int32(0)) },
        { Any(sal_Int32(9)), Any(sal_Int32(8)), Any(sal_Int32(14)) },
        { Any(sal_Int32(17)), Any(sal_Int32(2)), Any(sal_Int32(16)) },
        { Any(sal_Int32(0)), Any(sal_Int32(7)), Any(sal_Int32(14)) },
        { Any(sal_Int32(10)), Any(sal_Int32(16)), Any(sal_Int32(16)) },
        { Any(sal_Int32(2)), Any(sal_Int32(8)), Any(sal_Int32(10)) },
        { Any(sal_Int32(4)), Any(sal_Int32(8)), Any(sal_Int32(3)) },
        { Any(sal_Int32(7)), Any(sal_Int32(9)), Any(sal_Int32(0)) },
        { Any(sal_Int32(15)), Any(sal_Int32(6)), Any(sal_Int32(19)) },
        { Any(sal_Int32(2)), Any(sal_Int32(14)), Any(sal_Int32(19)) }
    };
    const Sequence<Any> rowHeadings{ Any(sal_Int32(0)), Any(sal_Int32(1)), Any(sal_Int32(2)), Any(sal_Int32(3)), Any(sal_Int32(4)),
                                     Any(sal_Int32(5)), Any(sal_Int32(6)), Any(sal_Int32(7)), Any(sal_Int32(8)), Any(sal_Int32(9)) };
    // ensure consistency of the test data
    CPPUNIT_ASSERT_EQUAL(rowCount, rowHeadings.getLength());
    CPPUNIT_ASSERT_EQUAL(rowCount, data.getLength());
    for (const auto& rowData : data)
        CPPUNIT_ASSERT_EQUAL(colCount, rowData.getLength());

    // add the test data
    m_dataModel->addRows(rowHeadings, data);
    CPPUNIT_ASSERT_EQUAL(rowCount, m_dataModel->getRowCount());
    CPPUNIT_ASSERT_EQUAL(colCount, m_dataModel->getColumnCount());

    // sort by each column
    for (sal_Int32 colIndex = 0; colIndex < colCount; ++colIndex)
    {
        for (bool ascending : { true, false })
        {
            m_dataModel->sortByColumn(colIndex, ascending);
            css::beans::Pair<sal_Int32, bool> currentSortOrder = m_dataModel->getCurrentSortOrder();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("invalid current sort column (column "
                                             + std::to_string(colIndex) + ")",
                                         currentSortOrder.First, colIndex);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("invalid current sort direction", currentSortOrder.Second,
                                         ascending);

            // verify the data is actually sorted by this column
            for (sal_Int32 rowIndex = 0; rowIndex < rowCount - 1; ++rowIndex)
            {
                const Any currentValue = m_dataModel->getCellData(colIndex, rowIndex);
                const sal_Int32 currentIntValue = impl_assertInteger(currentValue);
                const Any nextValue = m_dataModel->getCellData(colIndex, rowIndex + 1);
                const sal_Int32 nextIntValue = impl_assertInteger(nextValue);
                CPPUNIT_ASSERT_MESSAGE(
                    "data in row " + std::to_string(rowIndex) + " is actually not sorted "
                        + (ascending ? "ascending" : "descending"),
                    ascending ? currentIntValue <= nextIntValue : currentIntValue >= nextIntValue);

                // ensure the data in the other columns, and the row headings, are sorted as well
                const Any rowHeading = m_dataModel->getRowHeading(rowIndex);
                const sal_Int32 unsortedRowIndex = impl_assertInteger(rowHeading);
                for (sal_Int32 innerColIndex = 0; innerColIndex < colCount; ++innerColIndex)
                {
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(
                        "sorted row " + std::to_string(rowIndex) + ", unsorted row "
                            + std::to_string(unsortedRowIndex) + ", col "
                            + std::to_string(innerColIndex) + ": wrong data",
                        data[unsortedRowIndex][innerColIndex],
                        m_dataModel->getCellData(innerColIndex, rowIndex));
                }
            }
        }
    }
}

void GridControlTest::testView()
{
    const Reference<XControl> control = impl_createDialogWithGridControl();
    const Reference<XPropertySet> gridModelProps(control->getModel(), UNO_QUERY);

    // in the current implementation (not sure this is a good idea at all), the control (more precise: the peer)
    // ensures that if there are no columns in the column model, but in the data model, then the column model
    // will implicitly have the needed columns added.
    // To ensure that clients which rely on this do not break in the future, check this here.
    const Reference<XMutableGridDataModel> dataModel(
        gridModelProps->getPropertyValue(u"GridDataModel"_ustr), UNO_QUERY);
    CPPUNIT_ASSERT(dataModel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), dataModel->getColumnCount());

    const Reference<XGridColumnModel> columnModel(gridModelProps->getPropertyValue(u"ColumnModel"_ustr),
                                                  UNO_QUERY);
    CPPUNIT_ASSERT(columnModel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), columnModel->getColumnCount());

    const sal_Int32 columnCount = 3;
    const sal_Int32 rowCount = 2;
    dataModel->addRow(Any(), Sequence<Any>{ Any(sal_Int32(1)), Any(sal_Int32(2)), Any(sal_Int32(3)) });
    dataModel->addRow(Any(), Sequence<Any>{ Any(sal_Int32(6)), Any(sal_Int32(5)), Any(sal_Int32(4)) });

    CPPUNIT_ASSERT_EQUAL(columnCount, dataModel->getColumnCount());
    CPPUNIT_ASSERT_EQUAL(columnCount, columnModel->getColumnCount());

    // some cursor traveling
    const Reference<XGridControl> gridControl(control, UNO_QUERY);
    gridControl->goToCell(0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'current column' (1)", sal_Int32(0), gridControl->getCurrentColumn());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'current row' (1)", sal_Int32(0), gridControl->getCurrentRow());
    gridControl->goToCell(columnCount - 1, rowCount - 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'current column' (2)", dataModel->getColumnCount() - 1,
                                 gridControl->getCurrentColumn());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'current row' (2)", dataModel->getRowCount() - 1,
                                 gridControl->getCurrentRow());

    // removing the last column, while the active cell is in this very last column, is expected to adjust
    // the active cell
    columnModel->removeColumn(columnCount - 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "removed the last and active column, active column was not adjusted!", columnCount - 2,
        gridControl->getCurrentColumn());
    // same holds for rows
    dataModel->removeRow(rowCount - 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("removed the last and active row, active row was not adjusted!",
                                 rowCount - 2, gridControl->getCurrentRow());
}

Reference<XControl> GridControlTest::impl_createDialogWithGridControl()
{
    // create a simple dialog model/control/peer trinity
    const Reference<XControlModel> dialogModel
        = createInstance<XControlModel>(u"com.sun.star.awt.UnoControlDialogModel"_ustr);
    m_disposables.push_back(dialogModel);
    const Reference<XPropertySet> dialogProps(dialogModel, UNO_QUERY);
    dialogProps->setPropertyValue(u"Width"_ustr, Any(sal_Int32(200)));
    dialogProps->setPropertyValue(u"Height"_ustr, Any(sal_Int32(100)));
    dialogProps->setPropertyValue(u"Title"_ustr, Any(u"Grid Control Unit Test"_ustr));
    const Reference<XControl> dialogControl
        = createInstance<XControl>(u"com.sun.star.awt.UnoControlDialog"_ustr);
    m_disposables.push_back(dialogControl);
    dialogControl->setModel(dialogModel);
    dialogControl->createPeer(createInstance<XToolkit>(u"com.sun.star.awt.Toolkit"_ustr), nullptr);

    // insert a grid control model
    const Reference<XMultiServiceFactory> controlModelFactory(dialogModel, UNO_QUERY);
    const Reference<XPropertySet> gridModelProps(
        controlModelFactory->createInstance(u"com.sun.star.awt.grid.UnoControlGridModel"_ustr),
        UNO_QUERY);
    m_disposables.push_back(gridModelProps);
    gridModelProps->setPropertyValue(u"PositionX"_ustr, Any(sal_Int32(6)));
    gridModelProps->setPropertyValue(u"PositionY"_ustr, Any(sal_Int32(6)));
    gridModelProps->setPropertyValue(u"Width"_ustr, Any(sal_Int32(188)));
    gridModelProps->setPropertyValue(u"Height"_ustr, Any(sal_Int32(88)));
    const Reference<XNameContainer> modelContainer(dialogModel, UNO_QUERY);
    modelContainer->insertByName(u"grid"_ustr, Any(gridModelProps));

    // check the respective control has been created
    const Reference<XControlContainer> controlContainer(dialogControl, UNO_QUERY);
    const Reference<XControl> gridControl = controlContainer->getControl(u"grid"_ustr);
    CPPUNIT_ASSERT_MESSAGE("no grid control created in the dialog", gridControl);

    return gridControl;
}

int GridControlTest::impl_assertInteger(const Any& i_object)
{
    sal_Int32 aInt = 0;
    CPPUNIT_ASSERT(i_object >>= aInt);
    return aInt;
}

void GridControlTest::impl_assertColumnModelConsistency()
{
    for (sal_Int32 col = 0; col < m_columnModel->getColumnCount(); ++col)
    {
        const Reference<XGridColumn> column = m_columnModel->getColumn(col);
        CPPUNIT_ASSERT(column);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("column/model inconsistency: column " + std::to_string(col)
                                         + " has a wrong index!",
                                     col, column->getIndex());
    }

    const Sequence<Reference<XGridColumn>> allColumns = m_columnModel->getColumns();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getColumns returns the wrong number of column objects",
                                 m_columnModel->getColumnCount(), allColumns.getLength());
    for (sal_Int32 col = 0; col < m_columnModel->getColumnCount(); ++col)
    {
        CPPUNIT_ASSERT_MESSAGE(
            "getColumns inconsistency",
            impl_areSameInterface(allColumns[col], m_columnModel->getColumn(col)));
    }
}

void GridControlTest::impl_assertEquality(const Reference<XGridDataModel>& i_reference,
                                          const Reference<XGridDataModel>& i_compare)
{
    CPPUNIT_ASSERT(i_reference);
    CPPUNIT_ASSERT(i_compare);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("data model comparison: wrong column counts",
                                 i_reference->getColumnCount(), i_compare->getColumnCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("data model comparison: wrong row counts",
                                 i_reference->getRowCount(), i_compare->getRowCount());

    for (sal_Int32 row = 0; row < i_reference->getRowCount(); ++row)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "data model comparison: wrong row heading content in row " + std::to_string(row),
            i_reference->getRowHeading(row), i_compare->getRowHeading(row));
        for (sal_Int32 col = 0; col < i_reference->getRowCount(); ++col)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "data model comparison: wrong cell content in cell (" + std::to_string(col) + ", "
                    + std::to_string(row) + ")",
                i_reference->getCellData(col, row), i_compare->getCellData(col, row));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "data model comparison: wrong tooltip content in cell (" + std::to_string(col)
                    + ", " + std::to_string(row) + ")",
                i_reference->getCellToolTip(col, row), i_compare->getCellToolTip(col, row));
        }
    }
}

void GridControlTest::impl_assertEquality(const Reference<XGridColumnModel>& i_reference,
                                          const Reference<XGridColumnModel>& i_compare)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("column model comparison: wrong column counts",
                                 i_reference->getColumnCount(), i_compare->getColumnCount());
    for (sal_Int32 col = 0; col < i_reference->getColumnCount(); ++col)
    {
        const Reference<XGridColumn> referenceColumn = i_reference->getColumn(col);
        const Reference<XGridColumn> compareColumn = i_compare->getColumn(col);
        impl_assertEquality(referenceColumn, compareColumn);
    }
}

void GridControlTest::impl_assertEquality(const Reference<XGridColumn>& i_reference,
                                          const Reference<XGridColumn>& i_compare)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "grid column comparison: column attribute 'Identifier' does not match",
        i_reference->getIdentifier(), i_compare->getIdentifier());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "grid column comparison: column attribute 'ColumnWidth' does not match",
        i_reference->getColumnWidth(), i_compare->getColumnWidth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "grid column comparison: column attribute 'MinWidth' does not match",
        i_reference->getMinWidth(), i_compare->getMinWidth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "grid column comparison: column attribute 'MaxWidth' does not match",
        i_reference->getMaxWidth(), i_compare->getMaxWidth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "grid column comparison: column attribute 'Resizeable' does not match",
        i_reference->getResizeable(), i_compare->getResizeable());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "grid column comparison: column attribute 'Flexibility' does not match",
        i_reference->getFlexibility(), i_compare->getFlexibility());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "grid column comparison: column attribute 'HorizontalAlign' does not match",
        i_reference->getHorizontalAlign(), i_compare->getHorizontalAlign());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("grid column comparison: column attribute 'Title' does not match",
                                 i_reference->getTitle(), i_compare->getTitle());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "grid column comparison: column attribute 'HelpText' does not match",
        i_reference->getHelpText(), i_compare->getHelpText());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("grid column comparison: column attribute 'Index' does not match",
                                 i_reference->getIndex(), i_compare->getIndex());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "grid column comparison: column attribute 'DataColumnIndex' does not match",
        i_reference->getDataColumnIndex(), i_compare->getDataColumnIndex());
}

bool GridControlTest::impl_areSameInterface(const Reference<XInterface>& i_lhs,
                                            const Reference<XInterface>& i_rhs)
{
    return i_lhs == i_rhs;
}

bool GridControlTest::impl_areSameInterface(const Any& i_lhs, const Any& i_rhs)
{
    const Reference<XInterface> lhs(i_lhs, UNO_QUERY);
    const Reference<XInterface> rhs(i_rhs, UNO_QUERY);
    return lhs == rhs;
}

void GridControlTest::setUp()
{
    UnoApiTest::setUp();
    m_disposables.clear();
}

void GridControlTest::tearDown()
{
    for (auto& i : m_disposables)
        impl_dispose(i);
    UnoApiTest::tearDown();
}

Reference<XInterface> GridControlTest::createInstance(const OUString& i_serviceName)
{
    Reference<XInterface> instance
        = m_xContext->getServiceManager()->createInstanceWithContext(i_serviceName, m_xContext);
    CPPUNIT_ASSERT_MESSAGE("could not create an instance of '"
                               + std::string(i_serviceName.toUtf8()) + "'",
                           instance);
    return instance;
}

CPPUNIT_TEST_SUITE_REGISTRATION(GridControlTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
