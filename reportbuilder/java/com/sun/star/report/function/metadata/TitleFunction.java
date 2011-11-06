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


package com.sun.star.report.function.metadata;

import com.sun.star.report.ReportEngineParameterNames;

import org.pentaho.reporting.libraries.formula.EvaluationException;
import org.pentaho.reporting.libraries.formula.FormulaContext;
import org.pentaho.reporting.libraries.formula.LibFormulaErrorValue;
import org.pentaho.reporting.libraries.formula.function.Function;
import org.pentaho.reporting.libraries.formula.function.ParameterCallback;
import org.pentaho.reporting.libraries.formula.lvalues.TypeValuePair;
import org.pentaho.reporting.libraries.formula.typing.coretypes.TextType;

/**
 *
 * @author Ocke Janssen
 */
public class TitleFunction implements Function
{

    public TitleFunction()
    {
    }

    public String getCanonicalName()
    {
        return "TITLE";
    }

    public TypeValuePair evaluate(final FormulaContext context, final ParameterCallback parameters)
            throws EvaluationException
    {
        if (parameters.getParameterCount() != 0)
        {
            throw new EvaluationException(LibFormulaErrorValue.ERROR_ARGUMENTS_VALUE);
        }

        return new TypeValuePair(TextType.TYPE, context.getConfiguration().getConfigProperty(ReportEngineParameterNames.TITLE));
    }
}
