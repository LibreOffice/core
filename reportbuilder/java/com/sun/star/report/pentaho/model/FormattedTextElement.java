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


package com.sun.star.report.pentaho.model;

import org.jfree.report.expressions.FormulaExpression;

/**
 * A formatted text element takes a formula, evaluates it and formats the
 * given element using some arbitary style. (As this is totally undocumented,
 * we leave this out for now. Either we'll insert a field there or we call
 * an UNO-component to do the formatting.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class FormattedTextElement extends ReportElement
{

    private FormulaExpression valueExpression;

    public FormattedTextElement()
    {
    }

    public FormulaExpression getValueExpression()
    {
        return valueExpression;
    }

    public void setValueExpression(final FormulaExpression valueExpression)
    {
        this.valueExpression = valueExpression;
    }
}
