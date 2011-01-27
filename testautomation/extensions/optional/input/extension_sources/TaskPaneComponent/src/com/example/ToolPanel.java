/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package com.example;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XWindow;
import com.sun.star.ui.XToolPanel;
import com.sun.star.uno.UnoRuntime;

/**
 *
 * @author frank.schoenheit@oracle.com
 */
public class ToolPanel implements XToolPanel
{
    ToolPanel( final XWindow i_panelWindow )
    {
        m_panelWindow = i_panelWindow;
    }

    public XWindow getWindow()
    {
        return m_panelWindow;
    }

    public XAccessible createAccessible( XAccessible i_parentAccessible )
    {
        return UnoRuntime.queryInterface( XAccessible.class, m_panelWindow );
    }

    private XWindow m_panelWindow;
}
