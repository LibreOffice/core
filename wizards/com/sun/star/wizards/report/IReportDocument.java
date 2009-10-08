/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IReportDocument.java,v $
 *
 * $Revision: 1.3.6.1 $
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
package com.sun.star.wizards.report;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import java.util.ArrayList;
import java.util.Vector;

/**
 * New Interface which gives us the possibility to switch on the fly between the old
 * Wizard and the new Sun Report Builder Wizard, which use the same UI.
 *
 * @author ll93751
 */
public interface IReportDocument
{
    // public ReportTextDocument getDoc();
    // -------------------------------------------------------------------------
    // initialisation
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // opening the dialog
    // -------------------------------------------------------------------------
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
    public boolean liveupdate_addGroupNametoDocument(String[] GroupNames, String CurGroupTitle, Vector GroupFieldVector, ArrayList ReportPath, int iSelCount);

    public void refreshGroupFields(String[] _sNewNames);
    // public boolean isGroupField(String _FieldName);
    /**
     * Called by press ('less then') Removes an already set Groupname out of the list
     * @param NewSelGroupNames
     * @param CurGroupTitle
     * @param GroupFieldVector
     */
    public void liveupdate_removeGroupName(String[] NewSelGroupNames, String CurGroupTitle, java.util.Vector GroupFieldVector);

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

    public ArrayList getReportPath();

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
    public void store(String Name, int OpenMode);

    /**
     * The current report is added to the DB View under the given name
     *
     * TODO: add Name to this functionality
     */
    public void addReportToDBView(/* String Name */);

    public void importReportData(ReportWizard aWizard);

    /**
     * Create the final Report document
     * @param Name
     * @param _bAsTemplate
     * @param _bOpenInDesign
     * @return
     */
    public XComponent[] createFinalReportDocument(String Name, boolean _bAsTemplate, boolean _bOpenInDesign);

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
}
