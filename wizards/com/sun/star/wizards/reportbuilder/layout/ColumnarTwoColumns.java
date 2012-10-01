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
package com.sun.star.wizards.reportbuilder.layout;

import com.sun.star.awt.Rectangle;
import com.sun.star.report.XSection;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.report.IReportDefinitionReadAccess;
import com.sun.star.wizards.ui.UIConsts;

public class ColumnarTwoColumns extends ReportBuilderLayouter
{

    public ColumnarTwoColumns(IReportDefinitionReadAccess _xDefinitionAccess, Resource _aResource)
    {
        super(_xDefinitionAccess, _aResource);
    }

    public String getName()
    {
        return "ColumnarTwoColumnsLayoutOfData";
    }

    public String getLocalizedName()
    {
        return getResource().getResText(UIConsts.RID_REPORT + 82);
    }

    protected void insertDetailFields()
    {
        insertDetailFields(2);
    }

    protected void insertDetailFields(int _nColumns)
    {
        copyDetailProperties();

        if (_nColumns < 1)
        {
            return;
        }
        final String[] aFieldTitleNames = getFieldTitleNames();
        if (aFieldTitleNames == null)
        {
            return;
        }
        final String[] aFieldNames = getFieldNames();
        if (aFieldNames == null)
        {
            return;
        }

        final XSection xSection = getReportDefinition().getDetail();

        Rectangle aRect = new Rectangle();

        final int nLabelWidth = getMaxLabelWidth(); // 3000;

        final int nUsablePageWidth = getPageWidth() - getLeftPageIndent() - getRightPageIndent() - getLeftGroupIndent(getCountOfGroups());
        int i = 0;
        int nRows = aFieldNames.length / _nColumns;
        if ((aFieldNames.length % _nColumns) != 0)
        {
            ++nRows;
        }
        final int nWidth = (nUsablePageWidth - nLabelWidth * _nColumns) / _nColumns;
        if (nWidth < 0)
        {
            // TODO: error message in logging
            return;
        }

        final SectionObject aSOLabel = getDesignTemplate().getDetailLabel();
        aSOLabel.setFontToBold();
        final SectionObject aSOTextField = getDesignTemplate().getDetailTextField();
        int nMaxHeight = 0;
        for (int x = 0; x < _nColumns; x++)
        {
            aRect.Y = 0;
            for (int y = 0; y < nRows; y++)
            {
                aRect.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups()) + x * (nWidth + nLabelWidth);
                if (i < aFieldNames.length)
                {
                    final String sLabel = aFieldTitleNames[i];
                    aRect = insertLabel(xSection, sLabel, aRect, nLabelWidth, aSOLabel);
                    final String sFieldName = convertToFieldName(aFieldNames[i]);
                    aRect = insertFormattedField(xSection, sFieldName, aRect, nWidth, aSOTextField);

                    aRect.Y += Math.max(aSOLabel.getHeight(LayoutConstants.LabelHeight), aRect.Height);
                    ++i;
                }
            }
            nMaxHeight = Math.max(aRect.Y, nMaxHeight);
        }
        aRect.Y = Math.max(aSOLabel.getHeight(LayoutConstants.LabelHeight) * nRows, nMaxHeight);
        aRect.Y += aSOLabel.getHeight(LayoutConstants.EmptyLineHeight); // one empty line
        xSection.setHeight(aRect.Y);
        doNotBreakInTable(xSection);
    }

    protected void insertDetailFieldTitles(int lastGroupPostion)
    {
        // we won't extra field titles
    }
}
