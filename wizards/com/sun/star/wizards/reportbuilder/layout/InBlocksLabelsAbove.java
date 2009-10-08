/*
 * ***********************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InBlocksLabelsAbove.java,v $
 *
 * $Revision: 1.2.36.1 $
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
        final String sLocalizedName = getResource().getResText(UIConsts.RID_REPORT + 85);
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

        Rectangle aRectLabels = new Rectangle();
        Rectangle aRectFields = new Rectangle();


        final int nUsablePageWidth = getPageWidth() - getLeftPageIndent() - getRightPageIndent() - getLeftGroupIndent(getCountOfGroups());

        int i = 0;
        int nCount = aFieldTitleNames.length;
        // int x = 0;
        aRectLabels.Y = 0;
        aRectLabels.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

        aRectFields.Y = 500;
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

        aRectFields.Y = aSOLabel.getHeight(500);
        aRectFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());
        i = 0;
        nCount = aFieldTitleNames.length;
        while (nCount > 0)
        {
            final String sLabel = aFieldTitleNames[i];
            final int nLabelWidth = 3000 + nDelta;
            aRectLabels = insertLabel(xSection, sLabel, aRectLabels, nLabelWidth, aSOLabel);
            final String sFieldName = convertToFieldName(aFieldNames[i]);
            nFieldWidth = 3000 + nDelta;
            aRectFields = insertFormattedField(xSection, sFieldName, aRectFields, nFieldWidth, aSOTextField);

            int nNextX = aRectFields.X + nFieldWidth;
            if (nNextX > (getPageWidth() - getRightPageIndent()) & nCount > 1)
            {
                aRectLabels.Y += (2 * aSOLabel.getHeight(500)); // 2 * label height
                aRectLabels.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

                aRectFields.Y += (2 * aSOTextField.getHeight(500));
                aRectFields.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());
            }
            ++i;
            --nCount;
        }
        aRectFields.Y += aSOTextField.getHeight(500);
        aRectFields.Y += aSOTextField.getHeight(500); // one empty line
        xSection.setHeight(aRectFields.Y);
        doNotBreakInTable(xSection);
    }

    protected void insertDetailFieldTitles()
    {
        // we won't extra field titles
    }
}
