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

import org.pentaho.reporting.libraries.base.config.Configuration;
import org.pentaho.reporting.libraries.formula.ContextEvaluationException;
import org.pentaho.reporting.libraries.formula.FormulaContext;
import org.pentaho.reporting.libraries.formula.LocalizationContext;
import org.pentaho.reporting.libraries.formula.function.FunctionRegistry;
import org.pentaho.reporting.libraries.formula.operators.OperatorFactory;
import org.pentaho.reporting.libraries.formula.typing.Type;
import org.pentaho.reporting.libraries.formula.typing.TypeRegistry;

public class PentahoFormulaContext implements FormulaContext
{

    final private FormulaContext backend;
    final private Configuration config;

    public PentahoFormulaContext(final FormulaContext backend, final Configuration _config)
    {
        this.backend = backend;
        config = _config;
    }

    public LocalizationContext getLocalizationContext()
    {
        return backend.getLocalizationContext();
    }

    public Configuration getConfiguration()
    {
        return config;
    }

    public FunctionRegistry getFunctionRegistry()
    {
        return backend.getFunctionRegistry();
    }

    public TypeRegistry getTypeRegistry()
    {
        return backend.getTypeRegistry();
    }

    public OperatorFactory getOperatorFactory()
    {
        return backend.getOperatorFactory();
    }

    public Type resolveReferenceType(final Object name) throws ContextEvaluationException
    {
        return backend.resolveReferenceType(name);
    }

    public Object resolveReference(final Object name) throws ContextEvaluationException
    {
        if (name == null)
        {
            throw new NullPointerException();
        }
        return backend.resolveReference(name);
    }

    public boolean isReferenceDirty(final Object name) throws ContextEvaluationException
    {
        return backend.isReferenceDirty(name);
    }
}
