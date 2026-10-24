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
#include <com/sun/star/awt/grid/XGridDataListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>

using namespace cpo::uno;
using namespace css::awt::grid;
using namespace css::lang;

class GridDataListener : public cppu::WeakImplHelper<XGridDataListener>
{
public:
    virtual void rowsInserted(const GridDataEvent& i_event) override
    {
        CPPUNIT_ASSERT(!m_rowInsertionEvent);
        m_rowInsertionEvent = i_event;
    }

    virtual void rowsRemoved(const GridDataEvent& i_event) override
    {
        CPPUNIT_ASSERT(!m_rowRemovalEvent);
        m_rowRemovalEvent = i_event;
    }

    virtual void dataChanged(const GridDataEvent& i_event) override
    {
        CPPUNIT_ASSERT(!m_dataChangeEvent);
        m_dataChangeEvent = i_event;
    }

    virtual void rowHeadingChanged(const GridDataEvent& i_event) override
    {
        CPPUNIT_ASSERT(!m_rowHeadingChangeEvent);
        m_rowHeadingChangeEvent = i_event;
    }

    virtual void disposing(const EventObject&) override { m_disposed = true; }

    GridDataEvent assertSingleRowInsertionEvent()
    {
        CPPUNIT_ASSERT(m_rowInsertionEvent);
        CPPUNIT_ASSERT(!m_rowRemovalEvent);
        CPPUNIT_ASSERT(!m_dataChangeEvent);
        CPPUNIT_ASSERT(!m_rowHeadingChangeEvent);
        CPPUNIT_ASSERT(!m_disposed);
        return *m_rowInsertionEvent;
    }

    GridDataEvent assertSingleRowRemovalEvent()
    {
        CPPUNIT_ASSERT(!m_rowInsertionEvent);
        CPPUNIT_ASSERT(m_rowRemovalEvent);
        CPPUNIT_ASSERT(!m_dataChangeEvent);
        CPPUNIT_ASSERT(!m_rowHeadingChangeEvent);
        CPPUNIT_ASSERT(!m_disposed);
        return *m_rowRemovalEvent;
    }

    GridDataEvent assertSingleDataChangeEvent()
    {
        CPPUNIT_ASSERT(!m_rowInsertionEvent);
        CPPUNIT_ASSERT(!m_rowRemovalEvent);
        CPPUNIT_ASSERT(m_dataChangeEvent);
        CPPUNIT_ASSERT(!m_rowHeadingChangeEvent);
        CPPUNIT_ASSERT(!m_disposed);
        return *m_dataChangeEvent;
    }

    GridDataEvent assertSingleRowHeadingChangeEvent()
    {
        CPPUNIT_ASSERT(!m_rowInsertionEvent);
        CPPUNIT_ASSERT(!m_rowRemovalEvent);
        CPPUNIT_ASSERT(!m_dataChangeEvent);
        CPPUNIT_ASSERT(m_rowHeadingChangeEvent);
        CPPUNIT_ASSERT(!m_disposed);
        return *m_rowHeadingChangeEvent;
    }

    void reset()
    {
        m_rowInsertionEvent.reset();
        m_rowRemovalEvent.reset();
        m_dataChangeEvent.reset();
        m_rowHeadingChangeEvent.reset();
        // m_disposed is not reset intentionally
    }

private:
    std::optional<GridDataEvent> m_rowInsertionEvent;
    std::optional<GridDataEvent> m_rowRemovalEvent;
    std::optional<GridDataEvent> m_dataChangeEvent;
    std::optional<GridDataEvent> m_rowHeadingChangeEvent;
    bool m_disposed = false;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
