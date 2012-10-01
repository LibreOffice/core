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
import com.sun.star.wizards.ui.*;

public class InBlocksLabelsLeft extends ColumnarTwoColumns
{

    public InBlocksLabelsLeft(IReportDefinitionReadAccess _xDefinitionAccess, Resource _aResource)
    {
        super(_xDefinitionAccess, _aResource);
    }

    public String getName()
    {
        return "InBlocksLabelsLeftLayoutOfData";
    }

    public String getLocalizedName()
    {
        return getResource().getResText(UIConsts.RID_REPORT + 84);
    }

    protected void insertDetailFields()
    {
        copyDetailProperties();

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

        Rectangle aRectLabelFields = new Rectangle();

        final int nUsablePageWidth = getPageWidth() - getLeftPageIndent() - getRightPageIndent() - getLeftGroupIndent(getCountOfGroups());

        final int nLabelWidth = getMaxLabelWidth(); // 3000;
        int nFieldWidth = 3000;

        final int nFactor = nUsablePageWidth / (nLabelWidth + nFieldWidth);
        int nDeltaTotal = 0;
        int nDelta = 0;
        if (nFactor > 0)
        {
            nDeltaTotal = nUsablePageWidth - (nFactor * (nLabelWidth + nFieldWidth));
            nDelta = nDeltaTotal / nFactor;
        }

        int i = 0;
        int nCount = aFieldTitleNames.length;
        aRectLabelFields.Y = 0;
        aRectLabelFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

        SectionObject aSOLabel = getDesignTemplate().getDetailLabel();
        aSOLabel.setFontToBold();
        SectionObject aSOTextField = getDesignTemplate().getDetailTextField();
        int nLastHeight = 0;
        while (nCount > 0)
        {
            final String sLabel = aFieldTitleNames[i];
            aRectLabelFields = insertLabel(xSection, sLabel, aRectLabelFields, nLabelWidth, aSOLabel);
            final String sFieldName = convertToFieldName(aFieldNames[i]);
            nFieldWidth = 3000 + nDelta;
            aRectLabelFields = insertFormattedField(xSection, sFieldName, aRectLabelFields, nFieldWidth, aSOTextField);
            nLastHeight = Math.max(aRectLabelFields.Height, nLastHeight);
            final int nNextX = aRectLabelFields.X + nLabelWidth + nFieldWidth;
            if (nNextX > (getPageWidth() - getRightPageIndent()))
            {
                // TODO: label height is fix
                aRectLabelFields.Y += Math.max(aSOTextField.getHeight(LayoutConstants.FormattedFieldHeight), nLastHeight);
                nLastHeight = 0;
                aRectLabelFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());
            }
            ++i;
            --nCount;
        }
        aRectLabelFields.Y += Math.max(aSOLabel.getHeight(LayoutConstants.EmptyLineHeight), nLastHeight); // one empty line
        xSection.setHeight(aRectLabelFields.Y);
        doNotBreakInTable(xSection);
    }

    protected void insertDetailFieldTitles(int lastGroupPostion)
    {
        // we won't extra field titles
    }
}
