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

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;


/**
 * A report element is the base class for all content generating elements in a
 * report.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public abstract class ReportElement extends Element
{

    private final List formatConditions;

    protected ReportElement()
    {
        formatConditions = new ArrayList();
    }

    /**
     * Checks the current group and prints this element only if the current row is
     * the first row for that particular group.
     *
     * @return true, if the element should only be printed in the first row of the
     *         current group, false otherwise.
     */
    public boolean isPrintWhenGroupChanges()
    {
        return OfficeToken.TRUE.equals(getAttribute(OfficeNamespaces.OOREPORT_NS, "print-when-group-changes"));
    }

    public void setPrintWhenGroupChanges(final boolean printWhenGroupChanges)
    {
        setAttribute(OfficeNamespaces.OOREPORT_NS, "print-when-group-changes",
                String.valueOf(printWhenGroupChanges));
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
        return (FormatCondition[]) this.formatConditions.toArray(new FormatCondition[this.formatConditions.size()]);
    }

    public int getFormatConditionCount()
    {
        return formatConditions.size();
    }
}
