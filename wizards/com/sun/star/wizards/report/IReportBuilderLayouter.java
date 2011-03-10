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
package com.sun.star.wizards.report;

/**
 * This Interface separate the ReportBuilderImplementation from the layout.
 * Only these less methods are allowed to access from the ReportBuilderImplementation.
 * @author ll93751
 */
public interface IReportBuilderLayouter
{

    /**
     * Get the internal name of the current layout.
     * This name identifies the layout in the internal access list.
     * @return the internal layout name.
     */
    public String getName();

    /**
     * Get the localized name of the layout, which is shown in the layout page of the report wizard.
     * This name comes out of the resource and will be translate in different languages.
     * @return localized name of the layout.
     */
    public String getLocalizedName();

    /**
     * For Landscape give nOrientation == com.sun.star.wizards.report.ReportLayouter.SOOPTLANDSCAPE
     * All other numbers are interpreted as portrait format.
     * @param nOrientation
     */
    public void setPageOrientation(int nOrientation);

    /**
     * dispose the layouter
     */
    public void dispose();

    /**
     * Set the table name of the report
     * This is the name to the database table.
     * @param _nType something of com.sun.star.sdb.CommandType
     * @param TableName
     */
    public void setTableName(int _nType, String TableName);

    /**
     * Insert the field names, these are the field names out of a given datebase table
     * @param FieldNames
     */
    public void insertFieldNames(final String[] FieldNames);

    /**
     * Insert the field type (Varchar, int, ...) as internal an int representation.
     * @param FieldTypes
     */
    public void insertFieldTypes(int[] FieldTypes);

    /**
     * Insert the field width in count of chars as given in the database.
     * @param FieldWidths
     */
    public void insertFieldWidths(int[] FieldWidths);

    /**
     * Insert the titles of the field names. This names are free formed
     * @param _aFieldTitles
     */
    public void insertFieldTitles(String[] _aFieldTitles);

    /**
     * Insert the names of the groups, the group names are names out of the field names.
     *
     * If a group name is given here, it will not shown in the fields/titles, but must be in the field string list.
     * @param _aGroupFieldNames
     */
    public void insertGroupNames(String[] _aGroupFieldNames);

    /**
     * Insert the names of the groups which should be used as sorting, the group names are names out of the field names.
     *
     * If a group name is given here, it will not shown in the fields/titles, but must be in the field string list.
     * @param _aSortFieldNames
     */
    public void insertSortingNames(String[][] _aSortFieldNames);

    /**
     * This method redraws the whole layout with all it's content
     */
    public void layout();

    /**
     * Initialize the current Layouter with data's out of an other Layouter.
     *
     * This Method copies the internal fields, groups and titles
     * @param aOtherLayouter
     */
    public void initializeData(IReportBuilderLayouter aOtherLayouter);

    public void loadAndSetBackgroundTemplate(String LayoutTemplatePath);
}
