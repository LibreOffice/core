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

// import java.util.Vector;
import com.sun.star.awt.TextEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XTextListener;
import com.sun.star.beans.PropertyValue;

import com.sun.star.container.XContentEnumerationAccess;
import com.sun.star.deployment.XPackageInformationProvider;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.logging.XLogger;
import com.sun.star.logging.XLoggerPool;
import com.sun.star.sdb.CommandType;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

import com.sun.star.logging.LogLevel;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.DBMetaData;
import com.sun.star.wizards.db.DatabaseObjectWizard;
import com.sun.star.wizards.db.RecordParser;
import com.sun.star.wizards.db.SQLQueryComposer;
import com.sun.star.wizards.ui.CommandFieldSelection;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.SortingComponent;
import com.sun.star.wizards.ui.TitlesComponent;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import java.lang.reflect.Method;
import java.net.URI;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Map;

public class ReportWizard extends DatabaseObjectWizard implements XTextListener
{

    protected FieldSelection CurGroupFieldSelection;
    private SortingComponent CurSortingComponent;
    private TitlesComponent CurTitlesComponent;
    private CommandFieldSelection CurDBCommandFieldSelection;
    private GroupFieldHandler CurGroupFieldHandler;
    private ReportLayouter CurReportLayouter;
    private ReportFinalizer CurReportFinalizer;
    private int nReportMode = ReportFinalizer.SOCREATEDOCUMENT;
    private String m_sReportName = PropertyNames.EMPTY_STRING;
    protected static final String SOREPORTFORMNAME = "ReportSource";
    private static final int SOMAINPAGE = 1;
    private static final int SOTITLEPAGE = 2;
    protected static final int SOGROUPPAGE = 3;
    private static final int SOSORTPAGE = 4;
    protected static final int SOTEMPLATEPAGE = 5;
    protected static final int SOSTOREPAGE = 6;
    private IReportDocument m_reportDocument;
    private static String sMsgWizardName;
    private static String slblFields;
    private static String slblSelFields;
    private static String sShowBinaryFields;
    private String[] WizardHeaderText = new String[6];
    private static String slblColumnTitles;
    private static String slblColumnNames;
    private static String slblTables;
    protected static boolean bCloseDocument;
    private boolean bHasEscapeProcessing = true;

    public ReportWizard(XMultiServiceFactory i_serviceFactory, final PropertyValue[] i_wizardContext)
    {
        super(i_serviceFactory, 34320, i_wizardContext);
        super.addResourceHandler("Report Wizard", "dbw");
        if (getReportResources(false))
        {
            Helper.setUnoPropertyValues(xDialogModel,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT,
                        PropertyNames.PROPERTY_MOVEABLE,
                        PropertyNames.PROPERTY_NAME,
                        PropertyNames.PROPERTY_POSITION_X,
                        PropertyNames.PROPERTY_POSITION_Y,
                        PropertyNames.PROPERTY_STEP,
                        PropertyNames.PROPERTY_TABINDEX,
                        PropertyNames.PROPERTY_TITLE,
                        PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Integer.valueOf(210),
                        Boolean.TRUE,
                        "DialogReport",
                        102,
                        41,
                        1,
                        new Short((short) 0),
                        sMsgWizardName,
                        310
                    });
            drawNaviBar();
            setRightPaneHeaders(this.WizardHeaderText);
        }
    }

    protected void enterStep(int nOldStep, int nNewStep)
    {
        if ((nOldStep >= SOTEMPLATEPAGE) && (nNewStep < SOTEMPLATEPAGE))
        {
            // CurReportDocument.getDoc().oTextSectionHandler.removeTextSectionbyName("RecordSection");
            // CurReportDocument.getDoc().oTextTableHandler.removeTextTablebyName("Tbl_RecordSection");
            m_reportDocument.removeTextTableAndTextSection();
        }
        switch (nNewStep)
        {
            case SOMAINPAGE:
                CurDBCommandFieldSelection.setModified(false);
                break;

            case SOTITLEPAGE:
                String[] aFieldNames = m_reportDocument.getRecordParser().getFieldNames();
                Map<String, String> aFieldTitleSet = m_reportDocument.getRecordParser().getFieldTitleSet();
                CurTitlesComponent.initialize(aFieldNames, aFieldTitleSet);
                break;

            case SOGROUPPAGE:
                CurGroupFieldHandler.initialize();
                break;

            case SOSORTPAGE:
                String[] aFieldNames2 = m_reportDocument.getRecordParser().getFieldNames();
                String[][] aSortFieldNames = m_reportDocument.getRecordParser().getSortFieldNames();
                CurSortingComponent.initialize(aFieldNames2, aSortFieldNames);
                int nLength = m_reportDocument.getRecordParser().GroupFieldNames.length;
                CurSortingComponent.setReadOnlyUntil(nLength, false);
                break;

            case SOTEMPLATEPAGE:
                break;

            case SOSTOREPAGE:
                //TODO initialize with suitable PathName
                CurReportFinalizer.initialize(m_reportDocument.getRecordParser());
                break;

            default:
                break;
        }
    }

    protected void leaveStep(int nOldStep, int nNewStep)
    {

        switch (nOldStep)
        {
            case SOMAINPAGE:
                String[] aSelectedFieldNames = CurDBCommandFieldSelection.getSelectedFieldNames();
                String aTableName = CurDBCommandFieldSelection.getSelectedCommandName();
                int nType = CurDBCommandFieldSelection.getSelectedCommandType();
                m_reportDocument.initializeFieldColumns(nType, aTableName, aSelectedFieldNames);
                if (CurDBCommandFieldSelection.isModified())
                {
                    // cleanup document
                    m_reportDocument.clearDocument();
                    m_reportDocument.getRecordParser().setGroupFieldNames(new String[]
                            {
                            });
                    CurGroupFieldHandler.removeGroupFieldNames();
                }
                break;

            case SOTITLEPAGE:
                String[] sFieldTitles = CurTitlesComponent.getFieldTitles();
                // set new field name titles
                // CurReportDocument.getRecordParser().setFieldTitles(sFieldTitles);
                m_reportDocument.setFieldTitles(sFieldTitles);
                break;

            case SOGROUPPAGE:
                // TODO: DESIGN!!! a getter should return a value!!!
                CurGroupFieldHandler.getGroupFieldNames(m_reportDocument.getRecordParser());
                String[] aGroupFieldNames = m_reportDocument.getRecordParser().GroupFieldNames;
                // CurReportDocument.getRecordParser().prependSortFieldNames(aGroupFieldNames);
                m_reportDocument.setGrouping(aGroupFieldNames);
                break;

            case SOSORTPAGE:
                String[][] aSortFieldNames = CurSortingComponent.getSortFieldNames();
                // CurReportDocument.getRecordParser().SortFieldNames = aSortFieldNames;
                m_reportDocument.setSorting(aSortFieldNames);
                // TODO: why do we make a switch here
                super.enablefromStep(SOTEMPLATEPAGE, true);
                break;

            case SOTEMPLATEPAGE:
                break;

            case SOSTOREPAGE:
                break;

            default:
                break;
        }

        if ((nOldStep < SOTEMPLATEPAGE) && (super.getNewStep() >= SOTEMPLATEPAGE))
        {
// this is called before SOTEMPLATEPAGE, after SOGROUPPAGE
            m_reportDocument.getRecordParser().createRecordFieldNames();
            CurReportLayouter.initialize(m_reportDocument.getContentPath());
        }
    }

    private void dialogFinish()
    {
        this.xComponent.dispose();
        if (bCloseDocument)
        {
            m_reportDocument.dispose();
            return;
        }

        if ((nReportMode == ReportFinalizer.SOCREATETEMPLATE)
                || (nReportMode == ReportFinalizer.SOUSETEMPLATE))
        {
            m_reportDocument.addReportToDBView();
            boolean bOpenInDesign = (nReportMode == ReportFinalizer.SOCREATETEMPLATE);
            m_reportDocument.createAndOpenReportDocument(m_sReportName, true, bOpenInDesign);
        }
        else
        {
            m_reportDocument.importReportData(this);
            m_reportDocument.createAndOpenReportDocument(m_sReportName, false, false);
        }
    }

    private boolean executeQuery()
    {
        boolean bQueryCreated = false;
        final RecordParser recordParser = m_reportDocument.getRecordParser();
        final SQLQueryComposer sqlQueryComposer = recordParser.getSQLQueryComposer();
        if (this.CurDBCommandFieldSelection.getSelectedCommandType() == CommandType.TABLE)
        {
            if (m_reportDocument instanceof ReportTextImplementation)
            {
                bQueryCreated = sqlQueryComposer.setQueryCommand(this.xWindow, false, false);
                m_reportDocument.setCommandType(CommandType.COMMAND);
                String sQuery = sqlQueryComposer.getQuery();
                m_reportDocument.setCommand(sQuery);
            }
            else
            {
                bQueryCreated = true;
            }
        }
        else
        {
            try
            {
                String sQueryName = CurDBCommandFieldSelection.getSelectedCommandName();
                DBMetaData.CommandObject oCommand = recordParser.getQueryByName(sQueryName);
                bHasEscapeProcessing = recordParser.hasEscapeProcessing(oCommand.getPropertySet());
                String sCommand = (String) oCommand.getPropertySet().getPropertyValue(PropertyNames.COMMAND);
                if (bHasEscapeProcessing)
                {
                    // String sCommand = (String) oCommand.xPropertySet.getPropertyValue(PropertyNames.COMMAND);
                    bQueryCreated = (!sCommand.equals(PropertyNames.EMPTY_STRING));
                    if (m_reportDocument instanceof ReportTextImplementation)
                    {
                        sqlQueryComposer.m_xQueryAnalyzer.setQuery(sCommand);
                        sqlQueryComposer.prependSortingCriteria();
                        m_reportDocument.setCommandType(CommandType.COMMAND);
                        m_reportDocument.setCommand(sqlQueryComposer.getQuery());
                    }
                }
                else
                {
                    m_reportDocument.setCommandType(CommandType.COMMAND);
                    m_reportDocument.setCommand(sCommand);
                }
                bQueryCreated = true;
            }
            catch (Exception e)
            {
                e.printStackTrace(System.err);
            }
        }
        if (!bQueryCreated)
        {
            super.vetoableChange(null);
        }
        return bQueryCreated;
    }

    public void buildSteps()
    {
        // CurReportDocument.getDoc().xProgressBar.setValue(30);
        CurDBCommandFieldSelection = new CommandFieldSelection(this, m_reportDocument.getRecordParser(), 100, slblFields, slblSelFields, slblTables, true, 34330);
        CurDBCommandFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
        if (!isReportBuilderInstalled())
        {
            insertLabel("lblBinaryFields",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        16, sShowBinaryFields, 95, 162, 1, 210
                    });
        }
        // CurReportDocument.getDoc().xProgressBar.setValue(40);
        CurTitlesComponent = new TitlesComponent(this, SOTITLEPAGE, 97, 37, 210, 7, slblColumnNames, slblColumnTitles, 34381);
        CurTitlesComponent.addTextListener(this);
        // CurReportDocument.getDoc().xProgressBar.setValue(50);
        CurGroupFieldHandler = new GroupFieldHandler(m_reportDocument, this);
        // CurReportDocument.getDoc().xProgressBar.setValue(60);
        CurSortingComponent = new SortingComponent(this, SOSORTPAGE, 95, 30, 210, 34346);
        // CurReportDocument.getDoc().xProgressBar.setValue(70);
        CurReportLayouter = new ReportLayouter(xMSF, m_reportDocument, this, isReportBuilderInstalled());
        // CurReportDocument.getDoc().xProgressBar.setValue(80);
        CurReportFinalizer = new ReportFinalizer(xMSF, m_reportDocument, this);
        // CurReportDocument.getDoc().xProgressBar.setValue(100);
        bCloseDocument = true;
        // CurReportDocument.getDoc().xProgressBar.end();
        enableNavigationButtons(false, false, false);
    }

    public boolean finishWizard()
    {
        final int ncurStep = getCurrentStep();
        if ((switchToStep(ncurStep, SOSTOREPAGE)) || (ncurStep == SOSTOREPAGE))
        {
            if (this.executeQuery())
            {
                if (CurReportFinalizer.finish())
                {
                    nReportMode = CurReportFinalizer.getReportOpenMode();
                    m_sReportName = CurReportFinalizer.getStoreName();
                    xDialog.endExecute();
                    return true;
                }
            }
        }
        return false;
    }

    public void cancelWizard()
    {
        xDialog.endExecute();
    }

    public void insertQueryRelatedSteps()
    {
        setRMItemLabels(m_oResource, UIConsts.RID_QUERY + 80);
        addRoadmap();
        int i = 0;
        i = insertRoadmapItem(0, true, m_oResource.getResText(UIConsts.RID_QUERY + 80), SOMAINPAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_REPORT + 68), SOTITLEPAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_REPORT + 11), SOGROUPPAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_REPORT + 12), SOSORTPAGE);       // Orderby is always supported
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_REPORT + 13), SOTEMPLATEPAGE);
        i = insertRoadmapItem(i, false, m_oResource.getResText(UIConsts.RID_REPORT + 14), SOSTOREPAGE);
        setRoadmapInteractive(true);
        setRoadmapComplete(true);
        setCurrentRoadmapItemID((short) 1);
    }

    private boolean isReportBuilderInstalled()
    {
        //! Check if the new Report Builder Extension is available
        XContentEnumerationAccess a = com.sun.star.uno.UnoRuntime.queryInterface(XContentEnumerationAccess.class, xMSF);
        com.sun.star.container.XEnumeration e = a.createContentEnumeration("com.sun.star.report.pentaho.SOReportJobFactory");
        if (e == null)
        {
            return false;
        }
        return e.hasMoreElements();
        }

    /**
     * Return the path to the "com.sun.reportdesigner" extension
     * @param _xMSF
     * @return
     */
    public static String getPathToExtension(XMultiServiceFactory _xMSF)
    {
        // Get the path to the extension and try to add the path to the class loader
        final XComponentContext xComponentContext = Helper.getComponentContext(_xMSF);
        final Object aSingleton = xComponentContext.getValueByName("/singletons/com.sun.star.deployment.PackageInformationProvider");
        XPackageInformationProvider xProvider = UnoRuntime.queryInterface(XPackageInformationProvider.class, aSingleton);
        return xProvider.getPackageLocation("com.sun.reportdesigner");
    }
    private static XLogger m_xLogger;

    private static void initializeLogger(XMultiServiceFactory _xMSF)
    {
        XComponentContext xContext = Helper.getComponentContext(_xMSF);

        final Object aLoggerPool = xContext.getValueByName("/singletons/com.sun.star.logging.LoggerPool");
        if (aLoggerPool == null)
        {
            System.out.println("Can't get singleton from logging");
        }
        final XLoggerPool xLoggerPool = UnoRuntime.queryInterface(XLoggerPool.class, aLoggerPool);
        m_xLogger = xLoggerPool.getNamedLogger("com.sun.star.wizards.ReportBuilder");
    }

    public static XLogger getLogger()
    {
        return m_xLogger;
    }

    public void startReportWizard()
    {
        initializeLogger(xMSF);
        getLogger().log(LogLevel.SEVERE, "Start Report Wizard");

        if (isReportBuilderInstalled())
        {
            // Get the path to the extension and try to add the path to the class loader
            String sLocation = getPathToExtension(xMSF);
            // TODO: Umlaut in filename!
            if (sLocation.length() > 0)
            {
                try
                {
                    URI aLocationURI = URI.create(sLocation + "/" + "reportbuilderwizard.jar");

                    URL[] aURLs = new URL[1];
                    aURLs[0] = aLocationURI.toURL();
                    URLClassLoader aClassLoader = new URLClassLoader(aURLs, this.getClass().getClassLoader());
                    Class<?> a = aClassLoader.loadClass("com.sun.star.wizards.reportbuilder.ReportBuilderImplementation");
                    Method aMethod = a.getMethod("create", new Class[]
                            {
                                XMultiServiceFactory.class
                            });
                    m_reportDocument = (IReportDocument) aMethod.invoke(a, xMSF
                            );
                }
                catch (Exception e)
                {
                    // Maybe problems in URI create() if a wrong char is used like '[' ']', ...
                    System.out.println("There could be a problem with the path '" + sLocation + "'");
                }
            }
        }

        try
        {
            if (m_reportDocument == null)
            {
                // Fallback, if there is no reportbuilder wizard implementation, we use the old wizard
                m_reportDocument = new ReportTextImplementation(xMSF);
            }

            m_reportDocument.initialize(m_docUI, m_oResource);

            if (m_reportDocument.getRecordParser().getConnection(m_wizardContext))
            {
                buildSteps();

                if(!isReportBuilderInstalled()) CurReportLayouter.drawConstants();

                m_reportDocument.checkInvariants();

                this.CurDBCommandFieldSelection.preselectCommand(m_wizardContext, false);

                createWindowPeer(m_reportDocument.getWizardParent());

                m_reportDocument.getRecordParser().setWindowPeer(this.xControl.getPeer());
                insertQueryRelatedSteps();
                short RetValue = executeDialog(m_reportDocument.getFrame().getComponentWindow().getPosSize());
                if (RetValue == 0)
                {
                    dialogFinish();
                }
            }
            m_reportDocument.getRecordParser().dispose();
        }
        catch (java.io.IOException e)
        {
            String sMessage = e.getMessage();
            if (sMessage.equals("default.otr"))
            {
                sMessage = m_oResource.getResText(UIConsts.RID_REPORT + 92);
            }
            else
            {
            }
            // show a dialog with the error message
            SystemDialog.showMessageBox(xMSF, "ErrorBox", VclWindowPeerAttribute.OK, sMessage);
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.err);
        }
    }

    public void importReportData(final XMultiServiceFactory xMSF, final Dataimport CurDataimport)
    {
        boolean bDocisStored = false;
        try
        {
            boolean bexecute = false;
            if (!bHasEscapeProcessing)
            {
                bexecute = m_reportDocument.getRecordParser().executeCommand(com.sun.star.sdb.CommandType.QUERY);   //            sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot))
            }
            else
            {
                bexecute = m_reportDocument.getRecordParser().executeCommand(com.sun.star.sdb.CommandType.COMMAND);   //            sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot))
            }
            if (bexecute)
            {
                bexecute = m_reportDocument.getRecordParser().getFields(m_reportDocument.getRecordParser().getFieldNames(), false);
            }
            if (bexecute)
            {
                // CurDataimport.insertDatabaseDatatoReportDocument(xMSF);
                m_reportDocument.insertDatabaseDatatoReportDocument(xMSF);
            }

            if (CurReportFinalizer.getReportOpenMode() == ReportFinalizer.SOCREATEDOCUMENT)
            {
                bDocisStored = m_reportDocument.getRecordParser().storeDatabaseDocumentToTempPath(m_reportDocument.getComponent(), CurReportFinalizer.getStoreName());
            }
        }
        catch (com.sun.star.wizards.common.InvalidQueryException queryexception)
        {
        }
        CurDataimport.xComponent.dispose();
        if (bDocisStored)
        {
            m_reportDocument.getRecordParser().addReportDocument(m_reportDocument.getComponent(), false);
        }
        m_reportDocument.getRecordParser().dispose();
    }

    public boolean getReportResources(boolean bgetProgressResourcesOnly)
    {
        sMsgWizardName = super.m_oResource.getResText(UIConsts.RID_REPORT);
        if (!bgetProgressResourcesOnly)
        {
            sShowBinaryFields = m_oResource.getResText(UIConsts.RID_REPORT + 60);
            slblTables = m_oResource.getResText(UIConsts.RID_FORM + 6);
            slblFields = m_oResource.getResText(UIConsts.RID_FORM + 12);
            slblSelFields = m_oResource.getResText(UIConsts.RID_REPORT + 9);
            WizardHeaderText[0] = m_oResource.getResText(UIConsts.RID_REPORT + 28);
            WizardHeaderText[1] = m_oResource.getResText(UIConsts.RID_REPORT + 69);
            WizardHeaderText[2] = m_oResource.getResText(UIConsts.RID_REPORT + 29);
            WizardHeaderText[3] = m_oResource.getResText(UIConsts.RID_REPORT + 30);
            WizardHeaderText[4] = m_oResource.getResText(UIConsts.RID_REPORT + 31);
            WizardHeaderText[5] = m_oResource.getResText(UIConsts.RID_REPORT + 32);
        }
        slblColumnTitles = m_oResource.getResText(UIConsts.RID_REPORT + 70);
        slblColumnNames = m_oResource.getResText(UIConsts.RID_REPORT + 71);
        return true;
    }

    public static String getBlindTextNote(Object _aDocument, Resource _oResource)
    {
        String sBlindTextNote = PropertyNames.EMPTY_STRING;
        if (_aDocument instanceof ReportTextImplementation)
        {
            sBlindTextNote = _oResource.getResText(UIConsts.RID_REPORT + 75);
            sBlindTextNote = JavaTools.replaceSubString(sBlindTextNote, String.valueOf((char) 13), "<BR>");
        }
        return sBlindTextNote;
    }

    public void enableRoadmapItems(boolean _bEnabled)
    {
        try
        {
            Object oRoadmapItem = null;
            int CurStep = AnyConverter.toInt(Helper.getUnoPropertyValue(xDialogModel, PropertyNames.PROPERTY_STEP));
            boolean bEnabled = false;
            int CurItemID;
            for (int i = 0; i < getRMItemCount(); i++)
            {
                oRoadmapItem = this.xIndexContRoadmap.getByIndex(i);
                CurItemID = AnyConverter.toInt(Helper.getUnoPropertyValue(oRoadmapItem, "ID"));
                if (CurItemID > CurStep)
                {
                    bEnabled = _bEnabled;
                }
                else
                {
                    bEnabled = true;
                }

                Helper.setUnoPropertyValue(oRoadmapItem, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bEnabled));
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    private void enableWizardSteps(String[] NewItems)
    {
        boolean bEnabled = NewItems.length > 0;
        setControlProperty("btnWizardNext", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bEnabled));
        setControlProperty("btnWizardFinish", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bEnabled));
        enableRoadmapItems(bEnabled);   // Note: Performancewise this could be improved
    }

    public void textChanged(TextEvent xTextEvent)
    {
        try
        {
            Object oModel = UnoDialog.getModel(xTextEvent.Source);
            String sContent = (String) Helper.getUnoPropertyValue(oModel, "Text");
            String fieldname = this.CurTitlesComponent.getFieldNameByTitleControl(oModel);
            // CurReportDocument.getDoc().oTextFieldHandler.changeUserFieldContent(fieldname, sfieldtitle);
            m_reportDocument.liveupdate_changeUserFieldContent(fieldname, sContent);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void disposing(EventObject EventObject)
    {
    }

    public void setmodified(int _ndialogpage, Object ooldValue, Object onewValue)
    {
        switch (_ndialogpage)
        {
            case SOMAINPAGE:
                break;
            default:
                break;
        }
    }

    private void toggleSortingPage()
    {
        int nlCommandType = this.CurDBCommandFieldSelection.getSelectedCommandType();
        boolean bdoenable = (nlCommandType == CommandType.TABLE);
        if (!bdoenable)
        {
            String sQueryName = CurDBCommandFieldSelection.getSelectedCommandName();
            DBMetaData.CommandObject oCommand = m_reportDocument.getRecordParser().getQueryByName(sQueryName);
            bdoenable = m_reportDocument.getRecordParser().hasEscapeProcessing(oCommand.getPropertySet());
        }
        super.setStepEnabled(SOSORTPAGE, bdoenable);

        // int nCommandType = this.CurDBCommandFieldSelection.getSelectedCommandType();
        // super.setStepEnabled(SOSORTPAGE, (nCommandType == CommandType.TABLE));
    }

    public class FieldSelectionListener implements com.sun.star.wizards.ui.XFieldSelectionListener
    {

        protected int m_nID;

        public int getID()
        {
            return m_nID;
        }

        public void setID(String sIncSuffix)
        {
            m_nID = 1;
            if (sIncSuffix != null)
            {
                if ((!sIncSuffix.equals(PropertyNames.EMPTY_STRING)) && (!sIncSuffix.equals("_")))
                {
                    String sID = JavaTools.ArrayoutofString(sIncSuffix, "_")[1];
                    m_nID = Integer.parseInt(sID);
                }
            }
        }

        public void shiftFromLeftToRight(String[] SelItems, String[] NewItems)
        {
            if (m_nID == 1)
            {
                CurDBCommandFieldSelection.setModified(true);
                enableWizardSteps(NewItems);
                toggleSortingPage();
            }
            else
            {
                boolean bEnabled = (CurGroupFieldSelection.getSelectedFieldNames().length > 0);
                Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPPAGE), PropertyNames.PROPERTY_ENABLED, bEnabled);
            }
        }

        public void shiftFromRightToLeft(String[] SelItems, String[] NewItems)
        {
            // TODO When the ListFieldbox is refilled only fields of the current Command may be merged into the Listbox
            if (m_nID == 1)
            {
                enableWizardSteps(NewItems);
                CurDBCommandFieldSelection.setModified(true);
            }
            else
            {
                boolean bEnabled = (CurGroupFieldSelection.getSelectedFieldNames().length > 0);
                Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPPAGE), PropertyNames.PROPERTY_ENABLED, bEnabled);
            }
        }

        public void moveItemDown(String item)
        {
        }

        public void moveItemUp(String item)
        {
        }
    }

//    public void tests()
//    {
//
//        Calendar aCalendar = new GregorianCalendar();
//        aCalendar.setTimeInMillis(1202382900000L);
//        String aCalStr = aCalendar.toString();
//
//        Date aDate = new Date();
//        aDate.setSeconds(0);
//        aDate.setMinutes(15);
//        aDate.setHours(12);
//        // aDate.setMonth(2);
//        // aDate.setYear(2008);
//        // aDate.setDay(7);
//        long nTime = aDate.getTime();
//        Long aLong = new Long(nTime);
//        String aStr = aLong.toString();
//
//        Date aNewDate = new Date(1202382900000L);
////         aNewDate.
//        String aDateStr = aNewDate.toString();
////         Datetime aNewTime = new Time(1202382900);
////         String aTimeStr = aNewTime.toString();
//
//    }
}
