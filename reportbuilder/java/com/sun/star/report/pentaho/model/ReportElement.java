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
package com.sun.star.report.pentaho.model;

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;


/**
 * A report element is the base class for all content generating elements in a
 * report.
 *
 * @since 02.03.2007
 */
public abstract class ReportElement extends Element
{

    private final List<FormatCondition> formatConditions;

    protected ReportElement()
    {
        formatConditions = new ArrayList<FormatCondition>();
    }

    /**
     * Checks the current group and prints this element only if the current row is
     * the first row for that particular group.
     *
     * @return true, if the element should only be printed in the first row of the
     *         current group, false otherwise.
     */
    public boolean isPrintWhenGroupChange()
    {
        return OfficeToken.TRUE.equals(getAttribute(OfficeNamespaces.OOREPORT_NS, "print-when-group-change"));
    }

    public void setPrintWhenGroupChange(final boolean printWhenGroupChange)
    {
        setAttribute(OfficeNamespaces.OOREPORT_NS, "print-when-group-change",
                String.valueOf(printWhenGroupChange));
    }

    /**
     * Checks, whether the printed value has been changed since the last run. The
     * element will only be printed, if there was at least one change.
     *
     * @return true, if repeated values should be printed, false if repeated
     *         values should be surpressed.
     */
    public boolean isPrintRepeatedValues()
    {
        return OfficeToken.TRUE.equals(getAttribute(OfficeNamespaces.OOREPORT_NS, "print-repeated-values"));
    }

    public void setPrintRepeatedValues(final boolean printRepeatedValues)
    {
        setAttribute(OfficeNamespaces.OOREPORT_NS, "print-repeated-values",
                String.valueOf(printRepeatedValues));
    }

    public void addFormatCondition(final FormatCondition formatCondition)
    {
        if (formatCondition == null)
        {
            throw new NullPointerException();
        }

        this.formatConditions.add(formatCondition);
    }

    public FormatCondition[] getFormatConditions()
    {
        return this.formatConditions.toArray(new FormatCondition[this.formatConditions.size()]);
    }

    public int getFormatConditionCount()
    {
        return formatConditions.size();
    }
}
