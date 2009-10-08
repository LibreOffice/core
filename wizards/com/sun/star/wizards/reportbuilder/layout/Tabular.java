/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Tabular.java,v $
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
 ************************************************************************/
package com.sun.star.wizards.reportbuilder.layout;

import com.sun.star.awt.Rectangle;
import com.sun.star.report.XGroup;
import com.sun.star.report.XGroups;
import com.sun.star.report.XSection;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.report.IReportDefinitionReadAccess;
import com.sun.star.wizards.ui.UIConsts;

/**
 *
 * @author ll93751
 */
public class Tabular extends ReportBuilderLayouter
{

    public Tabular(IReportDefinitionReadAccess _xDefinitionAccess, Resource _aResource)
    {
        super(_xDefinitionAccess, _aResource);
    }

    public String getName()
    {
        return "DefaultLayoutOfData";
    }

    public String getLocalizedName()
    {
        String sLocalizedName = getResource().getResText(UIConsts.RID_REPORT + 80);
        return sLocalizedName;
    }

    protected void insertDetailFields()
    {
        copyDetailProperties();

        String[] aFieldNames = getFieldNames();
        if (aFieldNames == null)
        {
            return;
        }
        if (aFieldNames.length == 0)
        {
            return;
        }
//        int nGroups = getReportDefinition().getGroups().getCount();

        final XSection xSection = getReportDefinition().getDetail();

        Rectangle aRect = new Rectangle();
        aRect.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());

        final int nWidth = calculateFieldWidth(getLeftGroupIndent(getCountOfGroups()), aFieldNames.length);
        final SectionObject aSO = getDesignTemplate().getDetailTextField();

        for (int i = 0; i < aFieldNames.length; i++)
        {
            final String sFieldName = convertToFieldName(aFieldNames[i]);
            aRect = insertFormattedField(xSection, sFieldName, aRect, nWidth, aSO);
        }
        int nHeight = aSO.getHeight(500);
        xSection.setHeight(nHeight);
    }

    protected void insertDetailFieldTitles()
    {
        final String[] aFieldTitleNames = getFieldTitleNames();
        if (aFieldTitleNames == null)
        {
            return;
        }
        if (aFieldTitleNames.length == 0)
        {
            return;
        }
        final int nGroups = getReportDefinition().getGroups().getCount();
        try
        {
            XSection xSection = null;
            SectionObject aSO = null;
            if (nGroups == 0)
            {
                // Spezial case, there is no Group.
                final XGroups xGroups = getReportDefinition().getGroups();
                final XGroup xGroup = xGroups.createGroup();
                xGroup.setHeaderOn(true);

                xGroups.insertByIndex(xGroups.getCount(), xGroup);
                xSection = xGroup.getHeader();
                copyGroupProperties(0);
                aSO = getDesignTemplate().getDetailLabel();
                aSO.setFontToBold();
            }
            else
            {
                final XGroups xGroups = getReportDefinition().getGroups();
                // we insert the titles in the last group
                final Object aGroup = xGroups.getByIndex(nGroups - 1);
                final XGroup xGroup = (XGroup) UnoRuntime.queryInterface(XGroup.class, aGroup);
                xSection = xGroup.getHeader();

                // We don't need to copy the GroupProperties, because this is done in the insertGroup() member function
                // copyGroupProperties(0);
                aSO = getDesignTemplate().getGroupLabel(nGroups - 1);
            }

            Rectangle aRect = new Rectangle();
// TODO: getCountOfGroups() == nGroups???
            aRect.X = getLeftPageIndent() + getLeftGroupIndent(getCountOfGroups());
            // TODO: group line is fix
            aRect.Y = aSO.getHeight(500) + 250;  // group height + a little empty line
            final int nWidth = calculateFieldWidth(getLeftGroupIndent(getCountOfGroups()), aFieldTitleNames.length);

            for (int i = 0; i < aFieldTitleNames.length; i++)
            {
                aRect = insertLabel(xSection, aFieldTitleNames[i], aRect, nWidth, aSO);
            }
            xSection.setHeight(aSO.getHeight(500) + 250 + aSO.getHeight(500));
        }
        catch (com.sun.star.uno.Exception e)
        {
        }
    }
}
