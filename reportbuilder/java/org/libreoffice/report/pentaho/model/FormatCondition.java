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
 * The format condition defines, what style-format is applied to an element.
 *
 * @since 02.03.2007
 */
public class FormatCondition
{

    private final FormulaExpression formula;
    private final String styleName;
    private final boolean enabled;

    public FormatCondition(final FormulaExpression formula,
            final String styleName,
            final boolean enabled)
    {
        this.formula = formula;
        this.styleName = styleName;
        this.enabled = enabled;
    }

    public FormulaExpression getFormula()
    {
        return formula;
    }

    public String getStyleName()
    {
        return styleName;
    }

    public boolean isEnabled()
    {
        return enabled;
    }
}
