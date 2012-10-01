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
package com.sun.star.wizards.report;

/**
 * This Interface separate the ReportBuilderImplementation from the layout.
 * Only these less methods are allowed to access from the ReportBuilderImplementation.
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
