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

package complex.toolkit.awtgrid;

import com.sun.star.awt.grid.GridDataEvent;
import com.sun.star.awt.grid.XGridDataListener;
import com.sun.star.lang.EventObject;
import static org.junit.Assert.*;

final public class GridDataListener implements XGridDataListener
{
    public void rowsInserted( GridDataEvent i_event )
    {
        assertNull( m_rowInsertionEvent );
        m_rowInsertionEvent = i_event;
    }

    public void rowsRemoved( GridDataEvent i_event )
    {
        assertNull( m_rowRemovalEvent );
        m_rowRemovalEvent = i_event;
    }

    public void dataChanged( GridDataEvent i_event )
    {
        assertNull( m_dataChangeEvent );
        m_dataChangeEvent = i_event;
    }

    public void rowHeadingChanged( GridDataEvent i_event )
    {
        assertNull( m_rowHeadingChangeEvent );
        m_rowHeadingChangeEvent = i_event;
    }

    public void disposing( EventObject eo )
    {
        m_disposed = true;
    }

    public final GridDataEvent assertSingleRowInsertionEvent()
    {
        assertNotNull( m_rowInsertionEvent );
        assertNull( m_rowRemovalEvent );
        assertNull( m_dataChangeEvent );
        assertNull( m_rowHeadingChangeEvent );
        assertFalse( m_disposed );
        return m_rowInsertionEvent;
    }

    public final GridDataEvent assertSingleRowRemovalEvent()
    {
        assertNull( m_rowInsertionEvent );
        assertNotNull( m_rowRemovalEvent );
        assertNull( m_dataChangeEvent );
        assertNull( m_rowHeadingChangeEvent );
        assertFalse( m_disposed );
        return m_rowRemovalEvent;
    }

    public final GridDataEvent assertSingleDataChangeEvent()
    {
        assertNull( m_rowInsertionEvent );
        assertNull( m_rowRemovalEvent );
        assertNotNull( m_dataChangeEvent );
        assertNull( m_rowHeadingChangeEvent );
        assertFalse( m_disposed );
        return m_dataChangeEvent;
    }

    public final GridDataEvent assertSingleRowHeadingChangeEvent()
    {
        assertNull( m_rowInsertionEvent );
        assertNull( m_rowRemovalEvent );
        assertNull( m_dataChangeEvent );
        assertNotNull( m_rowHeadingChangeEvent );
        assertFalse( m_disposed );
        return m_rowHeadingChangeEvent;
    }

    public final void reset()
    {
        m_rowInsertionEvent = m_rowRemovalEvent = m_dataChangeEvent = m_rowHeadingChangeEvent = null;
        // m_disposed is not reset intentionally
    }
    private GridDataEvent m_rowInsertionEvent = null;
    private GridDataEvent m_rowRemovalEvent = null;
    private GridDataEvent m_dataChangeEvent = null;
    private GridDataEvent m_rowHeadingChangeEvent = null;
    private boolean m_disposed = false;
}
