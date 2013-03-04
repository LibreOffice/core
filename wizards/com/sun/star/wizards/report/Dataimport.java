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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.text.XTextDocument;

import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.common.*;
import com.sun.star.uno.Exception;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Dataimport extends UnoDialog2 implements com.sun.star.awt.XActionListener
{ // extends ReportWizard

    // ReportTextDocument CurReportDocument;
    IReportDocument CurReportDocument;

//    PropertyValue[] CurProperties;
    static boolean bStopProcess;
    static String sProgressDBConnection;
    static String sProgressDataImport;
    static String sProgressBaseCurRecord;
    static String sProgressCurRecord;
    static String sProgressTitle;
    static String sStop;

//    public static final String TBLRECORDSECTION = ReportTextDocument.TBLRECORDSECTION;
//    public static final String TBLGROUPSECTION = ReportTextDocument.TBLGROUPSECTION;
//    public static final String RECORDSECTION = ReportTextDocument.RECORDSECTION;
//    public static final String GROUPSECTION = ReportTextDocument.GROUPSECTION;
//    public static final String COPYOFTBLRECORDSECTION = ReportTextDocument.COPYOFTBLRECORDSECTION;
//    public static final String COPYOFTBLGROUPSECTION = ReportTextDocument.COPYOFTBLGROUPSECTION;
//    public static final String COPYOFRECORDSECTION = ReportTextDocument.COPYOFRECORDSECTION;
//    public static final String COPYOFGROUPSECTION = ReportTextDocument.COPYOFGROUPSECTION;
    public Dataimport(XMultiServiceFactory _xMSF)
    {
        super(_xMSF);
        super.addResourceHandler("ReportWizard", "dbw");
        sProgressDBConnection = m_oResource.getResText(UIConsts.RID_DB_COMMON + 34);
        sProgressDataImport = m_oResource.getResText(UIConsts.RID_REPORT + 67);
        sProgressTitle = m_oResource.getResText(UIConsts.RID_REPORT + 62);
        sProgressBaseCurRecord = m_oResource.getResText(UIConsts.RID_REPORT + 63);
        sStop = m_oResource.getResText(UIConsts.RID_DB_COMMON + 21);

    }

    public void disposing(com.sun.star.lang.EventObject eventObject)
    {
    }

    public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent)
    {
        // bStopProcess = true;
        CurReportDocument.StopProcess();
    }

//    public static void main(String args[]) 
//        {
//            String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
//            XMultiServiceFactory xMSF = null;
//            try
//            {
//                xMSF = com.sun.star.wizards.common.Desktop.connect(ConnectStr);
//                if (xMSF != null)
//                {
//                    System.out.println("Connected to " + ConnectStr);
//                }
//                PropertyValue[] curproperties = new PropertyValue[3];
//                curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/MyHSQL.odb");
////            curproperties[0] = Properties.createProperty("DatabaseLocation", "file:///C:/Documents and Settings/bc93774.EHAM02-DEV/My Documents/MyDocAssign.odb"); //baseLocation ); "DataSourceName", "db1");
////            curproperties[0] = Properties.createProperty("DataSourceName", "Bibliography");
//                curproperties[1] = Properties.createProperty(PropertyNames.COMMAND_TYPE, new Integer(CommandType.TABLE));
//                curproperties[2] = Properties.createProperty(PropertyNames.COMMAND, "Table2");
//
//                Dataimport CurDataimport = new Dataimport(xMSF);
//                TextDocument oTextDocument = new TextDocument(xMSF, true, null);
//                CurDataimport.createReport(xMSF, oTextDocument.xTextDocument, curproperties);
//
//            }
//            catch (Exception e) 
//            {
//                e.printStackTrace(System.err);
//            }
//            catch (java.lang.Exception javaexception) 
//            {
//                javaexception.printStackTrace(System.err);
//            }
//        }
    public void showProgressDisplay(XMultiServiceFactory xMSF, boolean bgetConnection)
    {
        try
        {
            Helper.setUnoPropertyValues(xDialogModel,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TITLE, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        84, 0, sProgressTitle, 180
                    });
            com.sun.star.awt.FontDescriptor oFontDesc = new com.sun.star.awt.FontDescriptor();
            oFontDesc.Weight = com.sun.star.awt.FontWeight.BOLD;
            if (bgetConnection)
            {
                insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblProgressDBConnection",
                        new String[]
                        {
                            PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            oFontDesc, 10, sProgressDBConnection, 6, 6, 0, 150
                        });

                insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblProgressDataImport",
                        new String[]
                        {
                            PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            10, sProgressDataImport, 6, 24, 0, 120
                        });
            }
            else
            {
                insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblProgressDataImport",
                        new String[]
                        {
                            PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            oFontDesc, 10, sProgressDataImport, 6, 24, 0, 120
                        });
            }
            insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblCurProgress",
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        10, PropertyNames.EMPTY_STRING, 12, 42, 0, 120
                    });

            insertButton("cmdCancel", 10000, this,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH, PropertyNames.PROPERTY_LABEL
                    },
                    new Object[]
                    {
                        14, HelpIds.getHelpIdString(34321), 74, 58, 0, new Short((short) 1), 40, sStop
                    });
            createWindowPeer(CurReportDocument.getWizardParent());
            calculateDialogPosition(CurReportDocument.getFrame().getComponentWindow().getPosSize());
            xWindow.setVisible(true);
            super.xReschedule.reschedule();
            }
        catch (Exception e)
        {
            Logger.getLogger(Dataimport.class.getName()).log(Level.SEVERE, null, e);
        // return;
        }
        catch (java.lang.Exception e)
        {
            Logger.getLogger(Dataimport.class.getName()).log(Level.SEVERE, null, e);
        // return;
        }
    }

    // public void importReportData(final XMultiServiceFactory xMSF, final Dataimport CurDataimport, final ReportTextDocument CurReportDocument.getDoc()) {
    public void importReportData(final XMultiServiceFactory _xMSF, final Dataimport _CurDataimport, IReportDocument _CurReportDocument, PropertyValue[] _properties)
    {
        if (CurReportDocument.reconnectToDatabase(_xMSF, _properties))
        {
            // The following calls to remove the Sections must occur after the connection to the database
            modifyFontWeight("lblProgressDBConnection", com.sun.star.awt.FontWeight.NORMAL);
            modifyFontWeight("lblProgressDataImport", com.sun.star.awt.FontWeight.BOLD);
            CurReportDocument.insertDatabaseDatatoReportDocument(_xMSF);
        }
        xComponent.dispose();
        CurReportDocument.getRecordParser().dispose();
    }

    public void createReport( XMultiServiceFactory xMSF, XDatabaseDocumentUI i_documentUI, XTextDocument _textDocument,
        PropertyValue[] properties)
    {
        CurReportDocument = ReportTextImplementation.create( xMSF, i_documentUI, _textDocument, m_oResource );
        showProgressDisplay(xMSF, true);
        importReportData(xMSF, this, CurReportDocument, properties);
    }
}
