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

import com.sun.star.awt.XWindow;
import com.sun.star.frame.XFrame;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;

public final class TaskPaneComponent extends WeakBase
        implements com.sun.star.ui.XUIElementFactory,
        com.sun.star.lang.XServiceInfo
{

    private final XComponentContext m_xContext;
    private static final String m_implementationName = TaskPaneComponent.class.getName();
    private static final String[] m_serviceNames =
    {
        "com.sun.star.ui.UIElementFactory"
    };

    public TaskPaneComponent(XComponentContext context)
    {
        m_xContext = context;
    }

    public static XSingleComponentFactory __getComponentFactory(String sImplementationName)
    {
        XSingleComponentFactory xFactory = null;

        if (sImplementationName.equals(m_implementationName))
        {
            xFactory = Factory.createComponentFactory(TaskPaneComponent.class, m_serviceNames);
        }
        return xFactory;
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey xRegistryKey)
    {
        return Factory.writeRegistryServiceInfo(m_implementationName,
                m_serviceNames,
                xRegistryKey);
    }

    public com.sun.star.ui.XUIElement createUIElement(String ResourceURL, com.sun.star.beans.PropertyValue[] creationArgs) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.IllegalArgumentException
    {

        XFrame docFrame = null;
        XWindow panelParentWindow = null;
        for (int i = 0; i < creationArgs.length; ++i)
        {
            if (creationArgs[i].Name.equals("Frame"))
            {
                docFrame = UnoRuntime.queryInterface(XFrame.class, creationArgs[i].Value);
            } else if (creationArgs[i].Name.equals("ParentWindow"))
            {
                panelParentWindow = UnoRuntime.queryInterface(XWindow.class, creationArgs[i].Value);
            }
        }

        return new ToolPanelUIElement(m_xContext, docFrame, panelParentWindow);

    }

    // com.sun.star.lang.XServiceInfo:
    public String getImplementationName()
    {
        return m_implementationName;
    }

    public boolean supportsService(String sService)
    {
        int len = m_serviceNames.length;

        for (int i = 0; i < len; i++)
        {
            if (sService.equals(m_serviceNames[i]))
            {
                return true;
            }
        }
        return false;
    }

    public String[] getSupportedServiceNames()
    {
        return m_serviceNames;
    }
}
