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

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.wizards.common.Resource;
import java.util.ArrayList;

/**
 * New Interface which gives us the possibility to switch on the fly between the old
 * Wizard and the new Sun Report Builder Wizard, which use the same UI.
 * 
 * @author ll93751
 */
public interface IReportDocument
{
    // -------------------------------------------------------------------------
    // initialisation
    // -------------------------------------------------------------------------
    public void initialize(
            final XDatabaseDocumentUI i_documentUI,
            final Resource i_resource
        );

    // -------------------------------------------------------------------------
    // Access Helper
    // -------------------------------------------------------------------------    
    /**
     * Gives access to the DB Values
     * @return
     */
    public com.sun.star.wizards.db.RecordParser getRecordParser();

    /**
     * Give access to the parent document
     * It is a document in the old Wizard
     * It is a Report Builder in the new Wizard
     * @return
     */
    public com.sun.star.awt.XWindowPeer getWizardParent();

    /**
     * 
     * @return the Frame of the document Window or Report Builder Window
     */
    public com.sun.star.frame.XFrame getFrame();

    public XComponent getComponent();

    // -------------------------------------------------------------------------
    // First step: After entering the table name, select fields
    // -------------------------------------------------------------------------    
    /**
     * Is called after first step, set Tablename and the fields, which should occur in the Report.
     * @param _aType 
     * @param TableName
     * @param FieldNames
     */
    public void initializeFieldColumns(final int _aType, final String TableName, final String[] FieldNames);

    /**
     * Empties the report document
     */
    public void clearDocument();

    /**
     * Empties the report document, if we called back, don't remove Grouping/Sorting
     */
    public void removeTextTableAndTextSection();

    // -------------------------------------------------------------------------
    // Second step: Label field titles
    // -------------------------------------------------------------------------    
    /**
     * Set new names for the titles
     * @param sFieldTitles
     */
    public void setFieldTitles(final String[] sFieldTitles);

    /**
     * Change a the name of the 'title' of one field.
     * It is possible to give all element names new names which are used as
     * element title of a given element name.
     * This is only used as a preview
     * @param FieldName
     * @param TitleName
     */
    public void liveupdate_changeUserFieldContent(final String FieldName, final String TitleName);
    // -------------------------------------------------------------------------
    // Third step: Grouping
    // -------------------------------------------------------------------------    
    /* Grouping Page */
    // Document should not hold the grouping information!
    /**
     * Called by press ('greater then') add a group to the group list
     * @param GroupNames
     * @param CurGroupTitle
     * @param GroupFieldVector
     * @param ReportPath
     * @param iSelCount
     * @return
     */
    public boolean liveupdate_addGroupNametoDocument(String[] GroupNames, String CurGroupTitle, ArrayList<String> GroupFieldVector, ArrayList<String> ReportPath, int iSelCount);

    public void refreshGroupFields(String[] _sNewNames);
    // public boolean isGroupField(String _FieldName);
    /**
     * Called by press ('less then') Removes an already set Groupname out of the list
     * @param NewSelGroupNames
     * @param CurGroupTitle
     * @param GroupFieldVector
     */
    public void liveupdate_removeGroupName(String[] NewSelGroupNames, String CurGroupTitle, java.util.ArrayList<String> GroupFieldVector);

    /**
     * set the list how to group
     * @param aGroupList
     */
    public void setGrouping(String[] aGroupList);
    // -------------------------------------------------------------------------
    // Fourth step: Sorting
    // -------------------------------------------------------------------------    
    /**
     * Set the list how to sort
     * @param aSort
     */
    public void setSorting(String[][] aSort);
    // -------------------------------------------------------------------------
    // Fivth step: Templates / Layout
    // -------------------------------------------------------------------------    
    /* Template Page */
    public void setPageOrientation(int nOrientation) throws com.sun.star.lang.IllegalArgumentException;

    public int getDefaultPageOrientation();

    public ArrayList<String> getReportPath();

    public String getLayoutPath();

    public String getContentPath();

    /**
     * Called if a new Layout is selected
     * @param LayoutTemplatePath
     */
    public void liveupdate_changeLayoutTemplate(String LayoutTemplatePath/*, String BitmapPath*/);

    /**
     * Called if a new Template is selected
     * @param ContentTemplatePath
     */
    public void liveupdate_changeContentTemplate(String ContentTemplatePath);

    //    public String[] getLayoutTemplates();
    //    public String[] getContentTemplates();
    public void layout_selectFirstPage();

    public void layout_setupRecordSection(String TemplateName);
    // -------------------------------------------------------------------------
    // finishing
    // -------------------------------------------------------------------------    
    // preview (update titlenames)
    // addTextListener
    /**
     * Set the Title into the document from the 'Create Report Page'
     * BUG: The Title is empty after create Report.
     * @param _sTitleName
     */
    public void liveupdate_updateReportTitle(String _sTitleName);

    /**
     * Store the document by the given name
     * @param Name
     * @param OpenMode
     */
    public void store(String Name, int OpenMode) throws com.sun.star.uno.Exception;

    /**
     * The current report is added to the DB View under the given name
     *
     * TODO: add Name to this functionality
     */
    public void addReportToDBView();

    public void importReportData(ReportWizard aWizard);

    /**
     * Create the final Report document
     * @param Name
     * @param _bAsTemplate
     * @param _bOpenInDesign
     */
    public void createAndOpenReportDocument(
        final String Name,
        final boolean _bAsTemplate,
        final boolean _bOpenInDesign
    );

    public void dispose();
    // -------------------------------------------------------------------------
    // Garbage dump
    // -------------------------------------------------------------------------    
    /* DataImport */
    // ???
    // public void addTextSectionCopies();
    // ???
    public boolean reconnectToDatabase(XMultiServiceFactory xMSF, PropertyValue[] Properties);
    // ???
    public void insertDatabaseDatatoReportDocument(XMultiServiceFactory xMSF);
    // ???
    // public com.sun.star.lang.XMultiServiceFactory getDocumentServiceFactory();
    /**
     * set a internal variable to stop a maybe longer DB access.
     */
    public void StopProcess(); // cancel

    /**
     * Returns a string list of layouts.
     * @return
     */
    public String[][] getDataLayout();

    /**
     * Returns a string list of header layouts
     * @return
     */
    public String[][] getHeaderLayout();

    public void setCommandType(int CommandType);

    public void setCommand(String Command);

    /**
     * check internal invariants
     * @throws java.lang.Exception
     */
    public void checkInvariants() throws java.lang.Exception;
}
