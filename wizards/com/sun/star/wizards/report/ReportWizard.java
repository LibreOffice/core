/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportWizard.java,v $
 * $Revision: 1.76.18.1 $
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

// import java.util.Vector;

// import com.sun.star.wizards.reportbuilder.ReportBuilderImplementation;
import com.sun.star.awt.Size;
import com.sun.star.awt.TextEvent;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XLayoutConstrains;
import com.sun.star.awt.XTextListener;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;

// import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XContentEnumerationAccess;
import com.sun.star.deployment.XPackageInformationProvider;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sdb.CommandType;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.db.*;
import com.sun.star.lang.XComponent;

import java.lang.reflect.Method;
import java.net.URI;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Map;

public class ReportWizard extends WizardDialog implements XTextListener, XCompletion
{
    // XMultiServiceFactory xMSF;
    // QueryMetaData CurDBMetaData;
    FieldSelection CurGroupFieldSelection;
    SortingComponent CurSortingComponent;
    UnoDialog CurUnoProgressDialog;
    TitlesComponent CurTitlesComponent;
    CommandFieldSelection CurDBCommandFieldSelection;
    GroupFieldHandler CurGroupFieldHandler;
    ReportLayouter CurReportLayouter;
    ReportFinalizer CurReportFinalizer;
    PropertyValue[] DBGPROPERTYVALUE;
    String sCommandName = "";
    int nCommandType = -1;
    int nReportMode = ReportFinalizer.SOCREATEDOCUMENT;
    private String m_sReportName = "";
    public static final String SOREPORTFORMNAME = "ReportSource";
    final int SOSELGROUPLST = 33;
    final int SOTXTCOLTITLE = 48;
    final int SOTITLESCROLLBAR = 49;
    public static final int SONULLPAGE = 0;
    public static final int SOMAINPAGE = 1;
    public static final int SOTITLEPAGE = 2;
    public static final int SOGROUPPAGE = 3;
    public static final int SOSORTPAGE = 4;
    public static final int SOTEMPLATEPAGE = 5;
    public static final int SOSTOREPAGE = 6;    // ReportTextDocument CurReportDocument;
    // ReportTextImplementation CurReportDocument;
    IReportDocument CurReportDocument;
    static String sMsgWizardName;
    static String slblFields;
    static String slblSelFields;
    static String sShowBinaryFields;
    static String sGroupings;
    String[] WizardHeaderText = new String[6];
    static String[] WizardTitle = new String[6];
    static String sWriterFilterName;
    static String slstDatabasesDefaultText;
    static String slstTablesDefaultText;
    static String sMsgErrorOccured;
    static String sMsgSavingImpossible;
    static String sMsgNoConnection;
    // Progress display relevant Strings
    static String slblColumnTitles;
    static String slblColumnNames;
    static String sMsgNoConnectionforDataimport;
    static String sMsgQueryCreationImpossible;
    public static String sMsgFilePathInvalid;
    static String slblTables;
//    public static String sBlindTextNote;
    public static boolean bCloseDocument;
    public boolean bHasEscapeProcessing = true;

    public ReportWizard(XMultiServiceFactory xMSF)
    {
        super(xMSF, 34320);
        super.addResourceHandler("Report Wizard", "dbw");
        if (getReportResources(false) == true)
        {
            Helper.setUnoPropertyValues(xDialogModel,
                    new String[]
                    {
                        "Height", "Moveable", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Title", "Width"
                    },
                    new Object[]
                    {
                        new Integer(210), Boolean.TRUE, "DialogReport", new Integer(102), new Integer(41), new Integer(1), new Short((short) 0), sMsgWizardName, new Integer(310)
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
            CurReportDocument.removeTextTableAndTextSection();
        }
        switch (nNewStep)
        {
            case SOMAINPAGE:
                CurDBCommandFieldSelection.setModified(false);
                break;

            case SOTITLEPAGE:
                String[] aFieldNames = CurReportDocument.getRecordParser().getFieldNames();
                Map aFieldTitleSet = CurReportDocument.getRecordParser().getFieldTitleSet();
                CurTitlesComponent.initialize(aFieldNames, aFieldTitleSet);
                break;

            case SOGROUPPAGE:
                CurGroupFieldHandler.initialize();
                break;

            case SOSORTPAGE:
                String[] aFieldNames2 = CurReportDocument.getRecordParser().getFieldNames();
                String[][] aSortFieldNames = CurReportDocument.getRecordParser().getSortFieldNames();
                CurSortingComponent.initialize(aFieldNames2, aSortFieldNames);
                int nLength = CurReportDocument.getRecordParser().GroupFieldNames.length;
                CurSortingComponent.setReadOnlyUntil(nLength, false);
                break;

            case SOTEMPLATEPAGE:
                break;

            case SOSTOREPAGE:
                //TODO initialize with suitable PathName
                CurReportFinalizer.initialize(CurReportDocument.getRecordParser());
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
                // set all selected field names, DB Table name
                // CurReportDocument.getRecordParser().initializeFieldColumns(aSelectedFieldNames, aTableName);
                int nType = CurDBCommandFieldSelection.getSelectedCommandType();
                // nType = com.sun.star.sdb.CommandType.TABLE;
                CurReportDocument.initializeFieldColumns(nType, aTableName, aSelectedFieldNames);
                // CurReportDocument.initializeFieldColumns(aSelectedFieldNames, aSelectedCommandName);
                // CurReportDocument.getRecordParser().setAllIncludedFieldNames(false);
                if (CurDBCommandFieldSelection.isModified())
                {
                    // cleanup document
                    CurReportDocument.clearDocument();
                    // CurReportDocument.getDoc().oTextSectionHandler.removeAllTextSections();
                    // CurReportDocument.getDoc().oTextTableHandler.removeAllTextTables();
                    // CurReportDocument.getDoc().DBColumnsVector = new Vector();
                    CurReportDocument.getRecordParser().setGroupFieldNames(new String[]
                            {
                            });
                    CurGroupFieldHandler.removeGroupFieldNames();
                }
                break;

            case SOTITLEPAGE:
                String[] sFieldTitles = CurTitlesComponent.getFieldTitles();
                // set new field name titles
                // CurReportDocument.getRecordParser().setFieldTitles(sFieldTitles);
                CurReportDocument.setFieldTitles(sFieldTitles);
                break;

            case SOGROUPPAGE:
                // TODO: DESIGN!!! a getter should return a value!!!
                CurGroupFieldHandler.getGroupFieldNames(CurReportDocument.getRecordParser());
                String[] aGroupFieldNames = CurReportDocument.getRecordParser().GroupFieldNames;
                // CurReportDocument.getRecordParser().prependSortFieldNames(aGroupFieldNames);
                CurReportDocument.setGrouping(aGroupFieldNames);
                break;

            case SOSORTPAGE:
                String[][] aSortFieldNames = CurSortingComponent.getSortFieldNames();
                // CurReportDocument.getRecordParser().SortFieldNames = aSortFieldNames;
                CurReportDocument.setSorting(aSortFieldNames);
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
            CurReportDocument.getRecordParser().createRecordFieldNames();
            CurReportLayouter.initialize(CurReportDocument.getContentPath());
        }
    }

    private XComponent[] dialogFinish(short RetValue)
    {
        XComponent[] ret = null;
        // Report Wizard Dialog is done.
        boolean bdisposeDialog = true;
        switch (RetValue)
        {
            case 0:
                // via Cancelbutton or via sourceCode with "endExecute"
                this.xComponent.dispose();
                if (bCloseDocument == true)
                {
                    // OfficeDocument.dispose(xMSF, CurReportDocument.getDoc().xComponent);
                    CurReportDocument.dispose();
                    return ret;
                }
                if ((nReportMode == ReportFinalizer.SOCREATETEMPLATE) || (nReportMode == ReportFinalizer.SOUSETEMPLATE))
                {
                    bdisposeDialog = false;
                    // Add Report to the DB View
                    // old: CurReportDocument.getRecordParser().addReportDocument(CurReportDocument.getComponent(), true);
                    CurReportDocument.addReportToDBView();
                    boolean bOpenInDesign = (nReportMode == ReportFinalizer.SOCREATETEMPLATE);
                    // Create Report
                    // old: ret = CurReportDocument.getRecordParser().openReportDocument(sReportName, true, bOpenInDesign);
                    ret = CurReportDocument.createFinalReportDocument(m_sReportName, true, bOpenInDesign);
                }
                else
                {
                    bdisposeDialog = false;
                    CurReportDocument.importReportData(this);
                    // Dataimport CurDataimport = new Dataimport(xMSF);
                    // CurDataimport.CurReportDocument = CurReportDocument;
                    // CurDataimport.showProgressDisplay(xMSF, false);
                    // importReportData(xMSF, CurDataimport);
                    // old: ret = CurReportDocument.getRecordParser().openReportDocument(sReportName, false, false);
                    ret = CurReportDocument.createFinalReportDocument(m_sReportName, false, false);
                }
                return ret;
            case 1:
                if (bdisposeDialog == true)
                {
                    // CurReportDocument.getDoc().unlockallControllers();
                }
                break;
        }
        return null;
    }

    private boolean executeQuery()
    {
        boolean bQueryCreated = false;
        if (this.CurDBCommandFieldSelection.getSelectedCommandType() == CommandType.TABLE)
        {
            bQueryCreated = CurReportDocument.getRecordParser().oSQLQueryComposer.setQueryCommand(sMsgWizardName, this.xWindow, false, false);

            CurReportDocument.setCommandType(CommandType.COMMAND);
            String sQuery = CurReportDocument.getRecordParser().oSQLQueryComposer.getQuery();
            CurReportDocument.setCommand(sQuery);
        }
        else
        {
            try
            {
                String sQueryName = CurDBCommandFieldSelection.getSelectedCommandName();
                DBMetaData.CommandObject oCommand = CurReportDocument.getRecordParser().getQueryByName(sQueryName);
                bHasEscapeProcessing = CurReportDocument.getRecordParser().hasEscapeProcessing(oCommand.xPropertySet);
                String sCommand = (String) oCommand.xPropertySet.getPropertyValue("Command");
                if (bHasEscapeProcessing)
                {
                    // String sCommand = (String) oCommand.xPropertySet.getPropertyValue("Command");
                    bQueryCreated = (!sCommand.equals(""));
                    CurReportDocument.getRecordParser().oSQLQueryComposer.m_xQueryAnalyzer.setQuery(sCommand);
                    CurReportDocument.getRecordParser().oSQLQueryComposer.prependSortingCriteria();
// TODO: check with query
                    CurReportDocument.setCommandType(CommandType.COMMAND);
                    CurReportDocument.setCommand(CurReportDocument.getRecordParser().oSQLQueryComposer.getQuery());
                    bQueryCreated = true;
                }
                else
                {
                    CurReportDocument.setCommandType(CommandType.COMMAND);
                    CurReportDocument.setCommand(sCommand);
                    bQueryCreated = true;
                }
            }
            catch (Exception e)
            {
                e.printStackTrace(System.out);
            }
        }
        if (!bQueryCreated)
        {
            super.vetoableChange(null);
        }
        return bQueryCreated;
    }

    public static void main(String args[])
    {
        String ConnectStr = "uno:socket,host=localhost,port=8107;urp;StarOffice.NamingService";   //localhost  ;Lo-1.Germany.sun.com; 10.16.65.155
        try
        {
            XMultiServiceFactory xLocMSF = com.sun.star.wizards.common.Desktop.connect(ConnectStr);

            tests(xLocMSF);

            ReportWizard CurReportWizard = new ReportWizard(xLocMSF);
            if (xLocMSF != null)
            {
                System.out.println("Connected to " + ConnectStr);
                PropertyValue[] curproperties = new PropertyValue[1];
                // curproperties[0] = Properties.createProperty(
                // "DatabaseLocation",
                // "file:///localhome/bc93774/NewDatabase2" +
                // "C:/Documents and Settings/ll93751/My Documents/RptWizard01_DB.odb");
                // "file://C:/Documents%20and%20Settings/ll93751/My%20Documents/RptWizard01_DB.odb");
//                    "C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/MyHSQL.odb"); //MyDocAssign.odb; baseLocation ); "DataSourceName", "db1");
                // curproperties[0] = Properties.createProperty("DataSourceName", "Bibliography");
                curproperties[0] = Properties.createProperty("DataSourceName", "RptWizard01_DB");
                CurReportWizard.startReportWizard(xLocMSF, curproperties, true);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
        System.exit(1);
    }

    private static void tests(XMultiServiceFactory _xMSF)
    {
        try
        {
//                String[] sServices = _xMSF.getAvailableServiceNames();
//                File aFile = new File("C:/temp/services.txt");
//                aFile.delete();
//                FileWriter aRAF = new FileWriter(aFile);
//                for (int i=0;i<sServices.length;i++)
//                {
//                    aRAF.write(sServices[i]);
//                    aRAF.write("\n");
//                }
//                aRAF.close();


//                XServiceInfo xServiceInfo = (XServiceInfo)UnoRuntime.queryInterface(XServiceInfo.class, _xMSF);
//                String[] sServices = xServiceInfo.getSupportedServiceNames();

//                XControl xControl = (XControl)UnoRuntime.queryInterface(XControl.class, xFormattedField);
//                Object aPeer = xControl.getPeer();
//                XTextConstraints xTC = (XTextConstraints)UnoRuntime.queryInterface(XTextConstraints.class, aPeer);
//                int nHeight = xTC.getTextHeight();
//                int nWidth = xTC.getTextWidth("Blah Fasel");

//                Object aTextShapeObj = _xMSF.createInstance("com.sun.star.drawing.TextShape");
//                XText xText = (XText)UnoRuntime.queryInterface(XText.class, aTextShapeObj);
//                xText.setString("Blah fasel");
//
//                XServiceInfo xServiceInfo2 = (XServiceInfo)UnoRuntime.queryInterface(XServiceInfo.class, aTextShapeObj);
//                String[] sServices2 = xServiceInfo2.getSupportedServiceNames();

//                Object aToolkitObj = _xMSF.createInstance("com.sun.star.awt.Toolkit");
//                XToolkit xToolkit = (XToolkit)UnoRuntime.queryInterface(XToolkit.class, aToolkitObj);
//                WindowDescriptor aDescriptor = new WindowDescriptor();
//                aDescriptor.Bounds = new Rectangle(0,0,640,480);
//
//                XWindowPeer aWindowPeer = xToolkit.createWindow(aDescriptor);
//                XWindow xWindow = (XWindow)UnoRuntime.queryInterface(XWindow.class, aWindowPeer);
//                xWindow.setVisible(true);
//                aWindowPeer.setBackground(0x00000000);

            Object aControlContainer = _xMSF.createInstance("com.sun.star.awt.UnoControlContainer");
            // XControlContainer xControlContainer = (XControlContainer)UnoRuntime.queryInterface(XControlContainer.class, aControlContainer);

            Object aFixedTextModel = _xMSF.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
            XControlModel xFixedTextModel = (XControlModel) UnoRuntime.queryInterface(XControlModel.class, aFixedTextModel);
// nicht das Model, sondern gleich den FixedText nehmen??

//                XMultiServiceFactory xMSF = (XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class, xFixedTextModel);

            Object aFixedText = _xMSF.createInstance("com.sun.star.awt.UnoControlFixedText");
            XServiceInfo xServiceInfo2 = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, aFixedText);
            String[] sServices2 = xServiceInfo2.getSupportedServiceNames();

            XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, aFixedText);
            xWindow.setVisible(true);

            XFixedText xFixedText = (XFixedText) UnoRuntime.queryInterface(XFixedText.class, aFixedText);
            xFixedText.setText("Dies ist ein String");

            XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, xFixedText);
            xControl.setModel(xFixedTextModel);

            XLayoutConstrains xLayoutConstrains = (XLayoutConstrains) UnoRuntime.queryInterface(XLayoutConstrains.class, aFixedText);
            Size aSize = xLayoutConstrains.getPreferredSize();

            // xToolkit.createScreenCompatibleDevice(_nWidth, _nWidth).
            // XWindow x = getReportDefinition().getCurrentController().getFrame().getContainerWindow();
            // Object aObj = _xSection.getParent();
            int dummy = 0;
        }
        catch (Exception e)
        {
            int dummy = 0;
        }

    }

    public void buildSteps()
    {
        // CurReportDocument.getDoc().xProgressBar.setValue(30);
        CurDBCommandFieldSelection = new CommandFieldSelection(this, CurReportDocument.getRecordParser(), 100, slblFields, slblSelFields, slblTables, true, 34330);
        CurDBCommandFieldSelection.addFieldSelectionListener(new FieldSelectionListener());
        insertLabel("lblBinaryFields",
                new String[]
                {
                    "Height", "Label", "PositionX", "PositionY", "Step", "Width"
                },
                new Object[]
                {
                    new Integer(16), sShowBinaryFields, new Integer(95), new Integer(162), new Integer(1), new Integer(210)
                });
        // CurReportDocument.getDoc().xProgressBar.setValue(40);
        CurTitlesComponent = new TitlesComponent(this, SOTITLEPAGE, 97, 37, 210, 7, slblColumnNames, slblColumnTitles, 34381);
        CurTitlesComponent.addTextListener(this);
        // CurReportDocument.getDoc().xProgressBar.setValue(50);
        CurGroupFieldHandler = new GroupFieldHandler(CurReportDocument, this);
        // CurReportDocument.getDoc().xProgressBar.setValue(60);
        CurSortingComponent = new SortingComponent(this, SOSORTPAGE, 95, 30, 210, 34346);
        // CurReportDocument.getDoc().xProgressBar.setValue(70);
        CurReportLayouter = new ReportLayouter(xMSF, CurReportDocument, this);
        // CurReportDocument.getDoc().xProgressBar.setValue(80);
        CurReportFinalizer = new ReportFinalizer(xMSF, CurReportDocument, this);
        // CurReportDocument.getDoc().xProgressBar.setValue(100);
        bCloseDocument = true;
        // CurReportDocument.getDoc().xProgressBar.end();
        enableNavigationButtons(false, false, false);
    }

    public void finishWizard()
    {
        int ncurStep = getCurrentStep();
        if ((switchToStep(ncurStep, SOSTOREPAGE)) || (ncurStep == SOSTOREPAGE))
        {
            if (this.executeQuery())
            {
                if (CurReportFinalizer.finish())
                {
                    nReportMode = CurReportFinalizer.getReportOpenMode();
                    m_sReportName = CurReportFinalizer.getStoreName();
                    xDialog.endExecute();
                }
            }
        }
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
        XContentEnumerationAccess a = (XContentEnumerationAccess) com.sun.star.uno.UnoRuntime.queryInterface(XContentEnumerationAccess.class, xMSF);
        com.sun.star.container.XEnumeration e = a.createContentEnumeration("com.sun.star.report.pentaho.SOReportJobFactory");
        if (e == null)
        {
            return false;
        }
        if (e.hasMoreElements())
        {
            return true;
        }
        return false;
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
        XPackageInformationProvider xProvider = (XPackageInformationProvider) UnoRuntime.queryInterface(XPackageInformationProvider.class, aSingleton);
        // String[][] aStrListList = xProvider.getExtensionList();
        final String sLocation = xProvider.getPackageLocation("com.sun.reportdesigner");
        return sLocation;
    }

    public XComponent[] startReportWizard(XMultiServiceFactory _xMSF, PropertyValue[] CurPropertyValue)
    {
        return startReportWizard(_xMSF, CurPropertyValue, false);
    }

    public XComponent[] startReportWizard(XMultiServiceFactory _xMSF, PropertyValue[] CurPropertyValue, boolean _bDebug)
    {
        XComponent[] ret = null;
        this.xMSF = _xMSF;
        DBGPROPERTYVALUE = CurPropertyValue;
        // CurReportDocument = new ReportTextDocument(xMSF, ReportPath + "/stl-default.ott", m_oResource );
        // if (isReportBuilderInstalled())
        // {
        //     CurReportDocument = ReportBuilderImplementation.create(xMSF, m_oResource);
        // }
        // else
        // {
        //     CurReportDocument = ReportTextImplementation.create(xMSF, m_oResource  );
        // }
        boolean bUseOld = false;
        if (!isReportBuilderInstalled())
        {
            bUseOld = true;
        }
        if (_bDebug == true && !bUseOld)
        {
            try
            {
                Class a = Class.forName("com.sun.star.wizards.reportbuilder.ReportBuilderImplementation");
                Method aMethod = a.getMethod("create", new Class[]
                        {
                            XMultiServiceFactory.class, Resource.class
                        });
                CurReportDocument = (IReportDocument) aMethod.invoke(a, new Object[]
                        {
                            xMSF, m_oResource
                        });
            }
            catch (Exception e)
            {
                int dummy = 0;
            }
        }
        else
        {
            if (!bUseOld)
            {
                // debug == false

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
                        Class a = aClassLoader.loadClass("com.sun.star.wizards.reportbuilder.ReportBuilderImplementation");
                        Method aMethod = a.getMethod("create", new Class[]
                                {
                                    XMultiServiceFactory.class, Resource.class
                                });
                        CurReportDocument = (IReportDocument) aMethod.invoke(a, new Object[]
                                {
                                    xMSF, m_oResource
                                });
                    }
                    catch (Exception e)
                    {
                        // TODO: Exception not handled.
                        int dummy = 0;
                        // Maybe problems in URI create() if a wrong char is used like '[' ']', ...
                        System.out.println("There could be a problem with the path '" + sLocation + "'");
                    }
                }
            }
        }
        try
        {
            if (CurReportDocument == null)
            {
                // Fallback, if there is no reportbuilder wizard implementation, we use the old wizard
                CurReportDocument = ReportTextImplementation.create(xMSF, m_oResource);
            }

            //        CurDBMetaData = CurReportDocument.getRecordParser();
//                tests();
            if (CurReportDocument.getRecordParser().getConnection(CurPropertyValue))
            {
                // CurReportDocument.getDoc().xProgressBar.setValue(20);
                CurReportDocument.getRecordParser().oSQLQueryComposer = new SQLQueryComposer(CurReportDocument.getRecordParser());
                buildSteps();
                this.CurDBCommandFieldSelection.preselectCommand(CurPropertyValue, false);

                createWindowPeer(CurReportDocument.getWizardParent());

                CurReportDocument.getRecordParser().setWindowPeer(this.xControl.getPeer());
                insertQueryRelatedSteps();
                short RetValue = executeDialog(CurReportDocument.getFrame().getComponentWindow().getPosSize());
                ret = dialogFinish(RetValue);
            }
            CurReportDocument.getRecordParser().dispose();
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.out);
        }
        return ret;
    }

    public void importReportData(final XMultiServiceFactory xMSF, final Dataimport CurDataimport)
    {
        boolean bDocisStored = false;
        try
        {
            boolean bexecute = false;
            if (!bHasEscapeProcessing)
            {
                bexecute = CurReportDocument.getRecordParser().executeCommand(com.sun.star.sdb.CommandType.QUERY);   //            sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot))
            }
            else
            {
                bexecute = CurReportDocument.getRecordParser().executeCommand(com.sun.star.sdb.CommandType.COMMAND);   //            sMsgQueryCreationImpossible + (char) 13 + sMsgEndAutopilot))
            }
            if (bexecute)
            {
                bexecute = CurReportDocument.getRecordParser().getFields(CurReportDocument.getRecordParser().getFieldNames(), false);
            }
            if (bexecute)
            {
                // CurDataimport.insertDatabaseDatatoReportDocument(xMSF);
                CurReportDocument.insertDatabaseDatatoReportDocument(xMSF);
            }

            if (CurReportFinalizer.getReportOpenMode() == ReportFinalizer.SOCREATEDOCUMENT)
            {
                bDocisStored = CurReportDocument.getRecordParser().storeDatabaseDocumentToTempPath(CurReportDocument.getComponent(), CurReportFinalizer.getStoreName());
            }
        }
        catch (com.sun.star.wizards.common.InvalidQueryException queryexception)
        {
        }
        CurDataimport.xComponent.dispose();
        if (bDocisStored)
        {
            CurReportDocument.getRecordParser().addReportDocument(CurReportDocument.getComponent(), false);
        }

        CurReportDocument.getRecordParser().dispose();
    }

    public boolean getReportResources(boolean bgetProgressResourcesOnly)
    {
        sMsgWizardName = super.m_oResource.getResText(UIConsts.RID_REPORT);
        if (bgetProgressResourcesOnly == false)
        {
            sShowBinaryFields = m_oResource.getResText(UIConsts.RID_REPORT + 60);
            slstDatabasesDefaultText = m_oResource.getResText(UIConsts.RID_DB_COMMON + 37);
            slstTablesDefaultText = m_oResource.getResText(UIConsts.RID_DB_COMMON + 38);
            sMsgErrorOccured = m_oResource.getResText(UIConsts.RID_DB_COMMON + 6);
            slblTables = m_oResource.getResText(UIConsts.RID_FORM + 6);
            slblFields = m_oResource.getResText(UIConsts.RID_FORM + 12);
            slblSelFields = m_oResource.getResText(UIConsts.RID_REPORT + 9);
            WizardHeaderText[0] = m_oResource.getResText(UIConsts.RID_REPORT + 28);
            WizardHeaderText[1] = m_oResource.getResText(UIConsts.RID_REPORT + 69);
            WizardHeaderText[2] = m_oResource.getResText(UIConsts.RID_REPORT + 29);
            WizardHeaderText[3] = m_oResource.getResText(UIConsts.RID_REPORT + 30);
            WizardHeaderText[4] = m_oResource.getResText(UIConsts.RID_REPORT + 31);
            WizardHeaderText[5] = m_oResource.getResText(UIConsts.RID_REPORT + 32);
            sMsgSavingImpossible = m_oResource.getResText(UIConsts.RID_DB_COMMON + 30);
        }
        sMsgFilePathInvalid = m_oResource.getResText(UIConsts.RID_DB_COMMON + 36);
        slblColumnTitles = m_oResource.getResText(UIConsts.RID_REPORT + 70);
        slblColumnNames = m_oResource.getResText(UIConsts.RID_REPORT + 71);
//            sBlindTextNote = m_oResource.getResText(UIConsts.RID_REPORT + 75);
//            sBlindTextNote = JavaTools.replaceSubString( sBlindTextNote, String.valueOf((char)13), "<BR>");
        return true;
    }

    public static String getBlindTextNote(Object _aDocument, Resource _oResource)
    {
        String sBlindTextNote = "";
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
            int CurStep = AnyConverter.toInt(Helper.getUnoPropertyValue(xDialogModel, "Step"));
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

                Helper.setUnoPropertyValue(oRoadmapItem, "Enabled", new Boolean(bEnabled));
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    private void enableWizardSteps(String[] NewItems)
    {
        boolean bEnabled = NewItems.length > 0;
        setControlProperty("btnWizardNext", "Enabled", new Boolean(bEnabled));
        setControlProperty("btnWizardFinish", "Enabled", new Boolean(bEnabled));
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
            CurReportDocument.liveupdate_changeUserFieldContent(fieldname, sContent);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
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
            DBMetaData.CommandObject oCommand = CurReportDocument.getRecordParser().getQueryByName(sQueryName);
            bdoenable = CurReportDocument.getRecordParser().hasEscapeProcessing(oCommand.xPropertySet);
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
                if ((!sIncSuffix.equals("")) && (!sIncSuffix.equals("_")))
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
                Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPPAGE), "Enabled", new Boolean(bEnabled));
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
                Helper.setUnoPropertyValue(getRoadmapItemByID(SOGROUPPAGE), "Enabled", new Boolean(bEnabled));
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
