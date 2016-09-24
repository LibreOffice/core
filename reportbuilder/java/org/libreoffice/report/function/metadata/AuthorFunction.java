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
package org.libreoffice.report.function.metadata;

import org.libreoffice.report.ReportEngineParameterNames;

import org.pentaho.reporting.libraries.formula.EvaluationException;
import org.pentaho.reporting.libraries.formula.FormulaContext;
import org.pentaho.reporting.libraries.formula.LibFormulaErrorValue;
import org.pentaho.reporting.libraries.formula.function.Function;
import org.pentaho.reporting.libraries.formula.function.ParameterCallback;
import org.pentaho.reporting.libraries.formula.lvalues.TypeValuePair;
import org.pentaho.reporting.libraries.formula.typing.coretypes.TextType;

public class AuthorFunction implements Function
{

    public String getCanonicalName()
    {
        return "AUTHOR";
    }

    public TypeValuePair evaluate(final FormulaContext context, final ParameterCallback parameters)
            throws EvaluationException
    {
        if (parameters.getParameterCount() != 0)
        {
            throw new EvaluationException(LibFormulaErrorValue.ERROR_ARGUMENTS_VALUE);
        }

        return new TypeValuePair(TextType.TYPE, context.getConfiguration().getConfigProperty(ReportEngineParameterNames.AUTHOR));
    }
}
