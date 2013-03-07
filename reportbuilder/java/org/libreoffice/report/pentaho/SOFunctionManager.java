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
package org.libreoffice.report.pentaho;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.report.meta.XFunctionCategory;
import com.sun.star.report.meta.XFunctionDescription;
import com.sun.star.report.meta.XFunctionManager;
import com.sun.star.uno.XComponentContext;

import org.pentaho.reporting.libraries.formula.DefaultFormulaContext;
import org.pentaho.reporting.libraries.formula.function.FunctionCategory;
import org.pentaho.reporting.libraries.formula.function.FunctionDescription;
import org.pentaho.reporting.libraries.formula.function.FunctionRegistry;

public final class SOFunctionManager extends ComponentBase implements XFunctionManager, XServiceInfo
{

    private final XComponentContext m_xContext;
    /**
     * The service name, that must be used to get an instance of this service.
     */
    private static final String __serviceName =
            "com.sun.star.report.meta.FunctionManager";
    final private FunctionCategory[] categories;
    final private FunctionRegistry functionRegistry;
    final private DefaultFormulaContext defaultContext;

    public SOFunctionManager(XComponentContext context)
    {
        m_xContext = context;
        final ClassLoader cl = java.lang.Thread.currentThread().getContextClassLoader();
        Thread.currentThread().setContextClassLoader(this.getClass().getClassLoader());
        defaultContext = new DefaultFormulaContext();
        functionRegistry = defaultContext.getFunctionRegistry();
        categories = functionRegistry.getCategories();
        Thread.currentThread().setContextClassLoader(cl);

    }

    /**
     * This method returns an array of all supported service names.
     *
     * @return Array of supported service names.
     */
    public String[] getSupportedServiceNames()
    {
        return getServiceNames();
    }

    /**
     * This method is a simple helper function to used in the static component initialisation functions as well as
     * in getSupportedServiceNames.
     * @return
     */
    public static String[] getServiceNames()
    {
        return new String[]
                {
                    __serviceName
                };
    }

    /**
     * This method returns true, if the given service will be supported by the component.
     *
     * @param sServiceName Service name.
     * @return True, if the given service name will be supported.
     */
    public boolean supportsService(final String sServiceName)
    {
        return sServiceName.equals(__serviceName);
    }

    /**
     * Return the class name of the component.
     *
     * @return Class name of the component.
     */
    public String getImplementationName()
    {
        return SOFunctionManager.class.getName();
    }

    // com.sun.star.container.XElementAccess:
    public com.sun.star.uno.Type getElementType()
    {
        return new com.sun.star.uno.Type(XFunctionCategory.class);
    }

    public boolean hasElements()
    {
        return categories.length != 0;
    }

    // com.sun.star.container.XIndexAccess:
    public int getCount()
    {
        return categories.length;
    }

    public Object getByIndex(int Index) throws com.sun.star.lang.IndexOutOfBoundsException, com.sun.star.lang.WrappedTargetException
    {
        return getCategory(Index);
    }

    // com.sun.star.report.meta.XFunctionManager:
    public com.sun.star.report.meta.XFunctionCategory getCategory(int position) throws com.sun.star.lang.IndexOutOfBoundsException, com.sun.star.lang.WrappedTargetException
    {
        if (position >= categories.length)
        {
            throw new com.sun.star.lang.IndexOutOfBoundsException();
        }
        return new StarFunctionCategory(defaultContext, m_xContext, functionRegistry, position, categories[position]);
    }

    public XFunctionDescription getFunctionByName(String arg0) throws NoSuchElementException
    {
        final FunctionDescription func = functionRegistry.getMetaData(arg0);
        if (func == null)
        {
            throw new NoSuchElementException();
        }
        int i = 0;
        for (; i < categories.length; i++)
        {
            if (categories[i] == func.getCategory())
            {
                break;
            }
        }
        try
        {
            return new StarFunctionDescription(defaultContext, m_xContext, getCategory(i), func);
        }
        catch (Exception ex)
        {
        }
        return null;
    }
}
