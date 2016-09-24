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
package org.libreoffice.report.pentaho.model;

import org.jfree.report.expressions.FormulaExpression;

/**
 * A formatted text element takes a formula, evaluates it and formats the
 * given element using some arbitrary style. (As this is totally undocumented,
 * we leave this out for now. Either we'll insert a field there or we call
 * an UNO-component to do the formatting.
 *
 * @since 02.03.2007
 */
public class FormattedTextElement extends ReportElement
{

    private FormulaExpression valueExpression;

    public FormulaExpression getValueExpression()
    {
        return valueExpression;
    }

    public void setValueExpression(final FormulaExpression valueExpression)
    {
        this.valueExpression = valueExpression;
    }
}
