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

/**
 *
 * @author ll93751
 */
public class InBlocksLabelsAbove extends ColumnarTwoColumns
{

    public InBlocksLabelsAbove(IReportDefinitionReadAccess _xDefinitionAccess, Resource _aResource)
    {
        super(_xDefinitionAccess, _aResource);
    }

    public String getName()
    {
        return "InBlocksLabelsAboveLayoutOfData";
    }

    public String getLocalizedName()
    {
        return getResource().getResText(UIConsts.RID_REPORT + 85);
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
//        int nGroups = getReportDefinition().getGroups().getCount();

        final XSection xSection = getReportDefinition().getDetail();

        Rectangle aRectLabels = new Rectangle();
        Rectangle aRectFields = new Rectangle();


        final int nUsablePageWidth = getPageWidth() - getLeftPageIndent() - getRightPageIndent() - getLeftGroupIndent(getCountOfGroups());

        int i = 0;
        int nCount = aFieldTitleNames.length;
        // int x = 0;
        aRectLabels.Y = 0;
        aRectLabels.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

        aRectFields.Y = LayoutConstants.LabelHeight;
        aRectFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

        // first run only calculates the width.
        int nDelta = 0;
        int nFieldWidth = 3000;
        while (nCount > 0)
        {
            // String sLabel = aFieldTitleNames[i];
            // int nLabelWidth = 3000;
            // String sFieldName = convertToFieldName(aFieldNames[i]);
            aRectFields.X += nFieldWidth;
            // aRectLabels.X += nFieldWidth;

            final int nNextX = aRectFields.X + nFieldWidth;
            if (nNextX > (getPageWidth() - getRightPageIndent()))
            {
                // all other values are not from interest.
                break;
            }

            ++i;
            --nCount;
        }
        final int nDeltaTotal = nUsablePageWidth - i * nFieldWidth;
        nDelta = nDeltaTotal;
        if (i > 0)
        {
            nDelta = nDeltaTotal / i;
        }

        aRectLabels.Y = 0;
        aRectLabels.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

        final SectionObject aSOLabel = getDesignTemplate().getDetailLabel();
        aSOLabel.setFontToBold();
        final SectionObject aSOTextField = getDesignTemplate().getDetailTextField();

        aRectFields.Y = aSOLabel.getHeight(LayoutConstants.LabelHeight);
        aRectFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());
        i = 0;
        nCount = aFieldTitleNames.length;
        int nLastHeight = 0;
        while (nCount > 0)
        {
            final String sLabel = aFieldTitleNames[i];
            final int nLabelWidth = 3000 + nDelta;
            aRectLabels = insertLabel(xSection, sLabel, aRectLabels, nLabelWidth, aSOLabel);
            final String sFieldName = convertToFieldName(aFieldNames[i]);
            nFieldWidth = 3000 + nDelta;
            aRectFields = insertFormattedField(xSection, sFieldName, aRectFields, nFieldWidth, aSOTextField);
            nLastHeight = Math.max(nLastHeight, aRectFields.Height);
            int nNextX = aRectFields.X + nFieldWidth;
            if (nNextX > (getPageWidth() - getRightPageIndent()) & nCount > 1)
            {
                int nHeight = (aSOLabel.getHeight(LayoutConstants.LabelHeight) + Math.max(aSOTextField.getHeight(LayoutConstants.FormattedFieldHeight), nLastHeight));
                nLastHeight = 0;
                aRectLabels.Y += nHeight; // 2 * label height
                aRectLabels.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

                aRectFields.Y += nHeight;
                aRectFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());
            }
            ++i;
            --nCount;
        }
        aRectFields.Y += Math.max(aSOTextField.getHeight(LayoutConstants.FormattedFieldHeight), nLastHeight);
        aRectFields.Y += aSOTextField.getHeight(LayoutConstants.EmptyLineHeight); // one empty line
        xSection.setHeight(aRectFields.Y);
        doNotBreakInTable(xSection);
    }

    protected void insertDetailFieldTitles(int lastGroupPostion)
    {
        // we won't extra field titles
    }
}
