/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XFunctionManager.java,v $
 *
 * $Revision: 1.7 $
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
package com.sun.star.report.pentaho;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.report.meta.XFunctionDescription;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.report.meta.XFunctionCategory;
import com.sun.star.report.meta.XFunctionManager;
import org.pentaho.reporting.libraries.formula.DefaultFormulaContext;
import org.pentaho.reporting.libraries.formula.function.FunctionCategory;
import org.pentaho.reporting.libraries.formula.function.FunctionDescription;
import org.pentaho.reporting.libraries.formula.function.FunctionRegistry;

/**
 * This class capsulates the class, that implements the minimal component, a factory for creating the service
 * (<CODE>__getComponentFactory</CODE>) and a method, that writes the information into the given registry key
 * (<CODE>__writeRegistryServiceInfo</CODE>).
 */
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
        if ( position >= categories.length )
            throw new com.sun.star.lang.IndexOutOfBoundsException();
        return new StarFunctionCategory(defaultContext,m_xContext, functionRegistry, position, categories[position]);
    }

    public XFunctionDescription getFunctionByName(String arg0) throws NoSuchElementException
    {
        final FunctionDescription func = functionRegistry.getMetaData(arg0);
        if ( func == null )
            throw new NoSuchElementException();
        int i = 0;
        for (; i < categories.length; i++)
        {
            if ( categories[i] == func.getCategory() )
                break;
        }
        try
        {
            return new StarFunctionDescription(defaultContext, m_xContext, getCategory(i), func);
        }
        catch ( Exception ex )
        {
        }
        return null;
    }
}
