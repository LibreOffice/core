/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StarFunctionDescription.java,v $
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
import com.sun.star.report.meta.XFunctionCategory;
import com.sun.star.sheet.FunctionArgument;
import java.util.Locale;
import java.util.MissingResourceException;
import org.pentaho.reporting.libraries.formula.DefaultFormulaContext;
import org.pentaho.reporting.libraries.formula.function.FunctionDescription;
import org.pentaho.reporting.libraries.formula.function.FunctionRegistry;

public final class StarFunctionDescription extends WeakBase
        implements com.sun.star.report.meta.XFunctionDescription
{

    private final XComponentContext m_xContext;
    private final PropertySetMixin m_prophlp;
    // attributes
//     final private com.sun.star.report.meta.XFunctionCategory m_Category;
    private final FunctionDescription functionDescription;
    private final FunctionRegistry functionRegistry;
    private final XFunctionCategory category;
    private final Locale defaultLocale;

    public StarFunctionDescription(final DefaultFormulaContext defaultContext, final XComponentContext context, final XFunctionCategory category, final FunctionRegistry functionRegistry, final FunctionDescription functionDescription)
    {
        m_xContext = context;
        this.category = category;
        Locale locale;
        try
        {
            functionDescription.getDisplayName(defaultContext.getLocalizationContext().getLocale());
            locale = defaultContext.getLocalizationContext().getLocale();
        } catch ( MissingResourceException e )
        {
            locale = Locale.ENGLISH;
        }
        this.defaultLocale = locale;

        this.functionDescription = functionDescription;
        this.functionRegistry = functionRegistry;
        // use the last parameter of the PropertySetMixin constructor
        // for your optional attributes if necessary. See the documentation
        // of the PropertySetMixin helper for further information.
        // Ensure that your attributes are initialized correctly!
        m_prophlp = new PropertySetMixin(m_xContext, this,
                                         new Type(com.sun.star.report.meta.XFunctionDescription.class), null);
    }
    ;

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

    // com.sun.star.report.meta.XFunctionDescription:
    public com.sun.star.report.meta.XFunctionCategory getCategory()
    {
        return category;
    }

    public String getName()
    {
        return functionDescription.getDisplayName(defaultLocale);
    }

    public String getDescription()
    {
        return functionDescription.getDescription(defaultLocale);
    }

    public String getSignature()
    {
        final int count = functionDescription.getParameterCount();
        final StringBuffer signature = new StringBuffer(getName());
        signature.append("(");
        for (int i = 0; i < count; i++)
        {
            signature.append(functionDescription.getParameterDisplayName(i, defaultLocale));
            if ( i != (count - 1) )
                signature.append(";");
        }
        signature.append(")");
        return signature.toString();
    }

    public com.sun.star.sheet.FunctionArgument[] getArguments()
    {
        int count = functionDescription.getParameterCount();
        final boolean infinite = functionDescription.isInfiniteParameterCount();
        if ( infinite )
        {
            count = 30;
        }
        final FunctionArgument[] args = new FunctionArgument[count];
        for (int i = 0; i < args.length; i++)
        {
            final int pos = infinite ? 0 : i;
            args[i] = new FunctionArgument();
            args[i].Description = functionDescription.getParameterDescription(pos, defaultLocale);
            args[i].Name = functionDescription.getParameterDisplayName(pos, defaultLocale);
            args[i].IsOptional = !functionDescription.isParameterMandatory(pos);
        }
        return args;
    }

    public String createFormula(String[] arguments) throws com.sun.star.lang.DisposedException, com.sun.star.lang.IllegalArgumentException, com.sun.star.uno.Exception
    {
        final boolean infinite = functionDescription.isInfiniteParameterCount();
        final int count = functionDescription.getParameterCount();
        if ( !infinite && arguments.length > count )
            throw new com.sun.star.lang.IllegalArgumentException();

        final StringBuffer formula = new StringBuffer(getName());
        formula.append("(");
        for (int i = 0; i < arguments.length; ++i)
        {
            if ( arguments[i].length() == 0 )
                break;
            formula.append(arguments[i]);
            if ( i < (arguments.length - 1) && arguments[i+1].length() != 0 )
                formula.append(";");
        }
        formula.append(")");
        return formula.toString();
    }
}
