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

import com.sun.star.lib.uno.helper.PropertySetMixin;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.report.meta.XFunctionDescription;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

import java.util.Locale;
import java.util.MissingResourceException;

import org.pentaho.reporting.libraries.formula.DefaultFormulaContext;
import org.pentaho.reporting.libraries.formula.function.FunctionCategory;
import org.pentaho.reporting.libraries.formula.function.FunctionRegistry;

public final class StarFunctionCategory extends WeakBase
        implements com.sun.star.report.meta.XFunctionCategory
{

    private final XComponentContext m_xContext;
    private final PropertySetMixin m_prophlp;
    // attributes
    private final int m_Number;
    private final FunctionCategory category;
    private final FunctionRegistry functionRegistry;
    private final String functions[];
    private final DefaultFormulaContext defaultContext;
    private final Locale defaultLocale;

    public StarFunctionCategory(DefaultFormulaContext defaultContext, final XComponentContext context, final FunctionRegistry functionRegistry, final int _number, final FunctionCategory category)
    {
        this.defaultContext = defaultContext;
        m_xContext = context;
        m_Number = _number;
        this.category = category;
        this.functionRegistry = functionRegistry;
        Locale locale;
        try
        {
            category.getDisplayName(defaultContext.getLocalizationContext().getLocale());
            locale = defaultContext.getLocalizationContext().getLocale();
        }
        catch (MissingResourceException e)
        {
            locale = Locale.ENGLISH;
            try
            {
                category.getDisplayName(locale);
            }
            catch (MissingResourceException e2)
            {
            }
        }
        this.defaultLocale = locale;

        functions = functionRegistry.getFunctionNamesByCategory(category);
        // use the last parameter of the PropertySetMixin constructor
        // for your optional attributes if necessary. See the documentation
        // of the PropertySetMixin helper for further information.
        // Ensure that your attributes are initialized correctly!
        m_prophlp = new PropertySetMixin(m_xContext, this,
                new Type(com.sun.star.report.meta.XFunctionCategory.class), null);
    }

    // com.sun.star.beans.XPropertySet:
    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
    {
        return m_prophlp.getPropertySetInfo();
    }

    public void setPropertyValue(String aPropertyName, Object aValue) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.beans.PropertyVetoException, com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.setPropertyValue(aPropertyName, aValue);
    }

    public Object getPropertyValue(String aPropertyName) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        return m_prophlp.getPropertyValue(aPropertyName);
    }

    public void addPropertyChangeListener(String aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.addPropertyChangeListener(aPropertyName, xListener);
    }

    public void removePropertyChangeListener(String aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.removePropertyChangeListener(aPropertyName, xListener);
    }

    public void addVetoableChangeListener(String aPropertyName, com.sun.star.beans.XVetoableChangeListener xListener) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.addVetoableChangeListener(aPropertyName, xListener);
    }

    public void removeVetoableChangeListener(String aPropertyName, com.sun.star.beans.XVetoableChangeListener xListener) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.removeVetoableChangeListener(aPropertyName, xListener);
    }

    // com.sun.star.container.XElementAccess:
    public com.sun.star.uno.Type getElementType()
    {
        return new com.sun.star.uno.Type(XFunctionDescription.class);
    }

    public boolean hasElements()
    {
        return functions.length != 0;
    }

    // com.sun.star.container.XIndexAccess:
    public int getCount()
    {
        return functions.length;
    }

    public Object getByIndex(int Index) throws com.sun.star.lang.IndexOutOfBoundsException, com.sun.star.lang.WrappedTargetException
    {
        return getFunction(Index);
    }

    // com.sun.star.report.meta.XFunctionCategory:
    public int getNumber()
    {
        return m_Number;
    }

    public String getName()
    {
        try
        {
            return category.getDisplayName(defaultLocale);
        }
        catch(Exception ex)
        {
        }
        return "Missing category for number " + m_Number;
    }

    public com.sun.star.report.meta.XFunctionDescription getFunction(int position) throws com.sun.star.lang.IndexOutOfBoundsException, com.sun.star.lang.WrappedTargetException
    {
        if (position >= functions.length)
        {
            throw new IndexOutOfBoundsException();
        }
        return new StarFunctionDescription(defaultContext, m_xContext, this, functionRegistry.getMetaData(functions[position]));
    }
}
