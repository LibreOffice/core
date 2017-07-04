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
import com.sun.star.report.meta.XFunctionCategory;
import com.sun.star.sheet.FunctionArgument;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

import java.util.Locale;
import java.util.MissingResourceException;

import org.pentaho.reporting.libraries.formula.DefaultFormulaContext;
import org.pentaho.reporting.libraries.formula.function.FunctionDescription;

public final class StarFunctionDescription extends WeakBase
        implements com.sun.star.report.meta.XFunctionDescription
{

    private final PropertySetMixin m_prophlp;
    private final FunctionDescription functionDescription;
    private final XFunctionCategory category;
    private final Locale defaultLocale;

    public StarFunctionDescription(final DefaultFormulaContext defaultContext, final XComponentContext context, final XFunctionCategory category, final FunctionDescription functionDescription)
    {
        this.category = category;
        Locale locale;
        try
        {
            functionDescription.getDisplayName(defaultContext.getLocalizationContext().getLocale());
            locale = defaultContext.getLocalizationContext().getLocale();
        }
        catch (MissingResourceException e)
        {
            locale = Locale.ENGLISH;
        }
        this.defaultLocale = locale;

        this.functionDescription = functionDescription;
        // use the last parameter of the PropertySetMixin constructor
        // for your optional attributes if necessary. See the documentation
        // of the PropertySetMixin helper for further information.
        // Ensure that your attributes are initialized correctly!
        m_prophlp = new PropertySetMixin(context, this,
                new Type(com.sun.star.report.meta.XFunctionDescription.class), null);
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

    // com.sun.star.report.meta.XFunctionDescription:
    public com.sun.star.report.meta.XFunctionCategory getCategory()
    {
        return category;
    }

    public String getName()
    {
        try
        {
            return functionDescription.getDisplayName(defaultLocale);
        }
        catch (Exception ex)
        {
        }
        return "Missing function name for " + this.getClass().getName();
    }

    public String getDescription()
    {
        try
        {
            return functionDescription.getDescription(defaultLocale);
        }
        catch (Exception ex)
        {
        }
        return "Missing function description for " + this.getClass().getName();
    }

    public String getSignature()
    {
        final int count = functionDescription.getParameterCount();
        final StringBuffer signature = new StringBuffer(getName());
        signature.append('(');
        for (int i = 0; i < count; i++)
        {
            signature.append(functionDescription.getParameterDisplayName(i, defaultLocale));
            if (i != (count - 1))
            {
                signature.append(';');
            }
        }
        signature.append(')');
        return signature.toString();
    }

    public com.sun.star.sheet.FunctionArgument[] getArguments()
    {
        int count = functionDescription.getParameterCount();
        final boolean infinite = functionDescription.isInfiniteParameterCount();
        if (infinite)
        {
            // Identical value as VAR_ARGS from formula/funcvarargs.h
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
        if (!infinite && arguments.length > count)
        {
            throw new com.sun.star.lang.IllegalArgumentException();
        }

        final StringBuffer formula = new StringBuffer(getName());
        formula.append('(');
        for (int i = 0; i < arguments.length; ++i)
        {
            if (arguments[i].length() == 0)
            {
                break;
            }
            formula.append(arguments[i]);
            if (i < (arguments.length - 1) && arguments[i + 1].length() != 0)
            {
                formula.append(';');
            }
        }
        formula.append(')');
        return formula.toString();
    }
}
