/*
 * ***********************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 * **********************************************************************
 */
package com.sun.star.wizards.reportbuilder.layout;

import com.sun.star.awt.Rectangle;
import com.sun.star.report.XGroup;
import com.sun.star.report.XSection;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.report.IReportDefinitionReadAccess;
import com.sun.star.wizards.ui.*;

/**
 *
 * @author ll93751
 */
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
        final String sLocalizedName = getResource().getResText(UIConsts.RID_REPORT + 84);
        return sLocalizedName;
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
        // int x = 0;
        aRectLabelFields.Y = 0;
        aRectLabelFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

        SectionObject aSOLabel = getDesignTemplate().getDetailLabel();
        aSOLabel.setFontToBold();
        SectionObject aSOTextField = getDesignTemplate().getDetailTextField();
        int nLastHeight = 0;
        while (nCount > 0)
        {
            final String sLabel = aFieldTitleNames[i];
//            nLabelWidth = 3000;
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
