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
{

    // ReportTextDocument CurReportDocument;
    IReportDocument CurReportDocument;

    private String sProgressDBConnection;
    private String sProgressDataImport;
    private String sProgressTitle;
    private String sStop;

    public Dataimport(XMultiServiceFactory _xMSF)
    {
        super(_xMSF);
        super.addResourceHandler("dbw");
        sProgressDBConnection = m_oResource.getResText(UIConsts.RID_DB_COMMON + 34);
        sProgressDataImport = m_oResource.getResText(UIConsts.RID_REPORT + 67);
        sProgressTitle = m_oResource.getResText(UIConsts.RID_REPORT + 62);
        m_oResource.getResText(UIConsts.RID_REPORT + 63);
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

    private void showProgressDisplay(boolean bgetConnection)
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
                        14, HelpIds.getHelpIdString(34321), 74, 58, 0, Short.valueOf((short) 1), 40, sStop
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

    private void importReportData(final XMultiServiceFactory _xMSF, PropertyValue[] _properties)
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
        showProgressDisplay(true);
        importReportData(xMSF, properties);
    }
}
