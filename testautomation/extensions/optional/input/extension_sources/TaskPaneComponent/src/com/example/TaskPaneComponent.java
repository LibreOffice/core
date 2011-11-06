/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
