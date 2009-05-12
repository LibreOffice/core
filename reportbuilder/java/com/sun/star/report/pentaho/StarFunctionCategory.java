/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StarFunctionCategory.java,v $
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

import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.Type;
import com.sun.star.lib.uno.helper.PropertySetMixin;
import com.sun.star.report.meta.XFunctionDescription;
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

    public StarFunctionCategory( DefaultFormulaContext defaultContext,final XComponentContext context,final FunctionRegistry functionRegistry,final int _number,final FunctionCategory category )
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
        catch(MissingResourceException e)
        {
            locale = Locale.ENGLISH;
        }
        this.defaultLocale = locale;

        functions = functionRegistry.getFunctionNamesByCategory(category);
        // use the last parameter of the PropertySetMixin constructor
        // for your optional attributes if necessary. See the documentation
        // of the PropertySetMixin helper for further information.
        // Ensure that your attributes are initialized correctly!
        m_prophlp = new PropertySetMixin(m_xContext, this,
            new Type(com.sun.star.report.meta.XFunctionCategory.class), null);
    };

    // com.sun.star.beans.XPropertySet:
    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
    {
        return m_prophlp.getPropertySetInfo();
    }

    public void setPropertyValue(String aPropertyName, Object aValue) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.beans.PropertyVetoException, com.sun.star.lang.IllegalArgumentException,com.sun.star.lang.WrappedTargetException
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
        return category.getDisplayName(defaultLocale);
    }

    public com.sun.star.report.meta.XFunctionDescription getFunction(int position) throws com.sun.star.lang.IndexOutOfBoundsException, com.sun.star.lang.WrappedTargetException
    {
        if ( position >= functions.length )
            throw new IndexOutOfBoundsException();
        return new StarFunctionDescription(defaultContext,m_xContext,this,functionRegistry,functionRegistry.getMetaData(functions[position]));
    }

}
