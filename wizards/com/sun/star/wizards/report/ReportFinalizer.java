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
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.*;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XTextComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.db.RecordParser;
public class ReportFinalizer
{

    WizardDialog CurUnoDialog;
    XTextComponent xTitleTextBox;
    XTextComponent[] xSaveTextBox = new XTextComponent[2];
    Object chkTemplate;
    String CHANGEREPORTTITLE_FUNCNAME = "changeReportTitle";
    String TOGGLESUBTEMPLATECONTROLS_FUNCNAME = "toggleSubTemplateControls";
    String TemplatePath;
    String StoreName;
    boolean bfinalaskbeforeOverwrite;
    String DefaultName;
    String OldDefaultName;
    IReportDocument CurReportDocument;
    public static final int SOCREATEDOCUMENT = 1;
    public static final int SOCREATETEMPLATE = 2;
    public static final int SOUSETEMPLATE = 3;
    private XMultiServiceFactory m_xMSF;
    public ReportFinalizer(XMultiServiceFactory _xMSF, IReportDocument _CurReportDocument, WizardDialog _CurUnoDialog)
    {
        m_xMSF = _xMSF;

        this.CurUnoDialog = _CurUnoDialog;
        this.CurReportDocument = _CurReportDocument;
        short curtabindex = (short) (ReportWizard.SOSTOREPAGE * 100);
        Desktop odesktop = new Desktop();

        String sSaveAsTemplate = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 40);
        String sUseTemplate = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 41);
        String sEditTemplate = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 42);
        String sSaveAsDocument = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 43);
        String sReportTitle = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 33);
        String slblHowProceed = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 78);
        String slblChooseReportKind = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 79);

        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblTitle",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, sReportTitle, 95, 27, new Integer(ReportWizard.SOSTOREPAGE), new Short(curtabindex++), 68
                });

        xTitleTextBox = CurUnoDialog.insertTextField("txtTitle", CHANGEREPORTTITLE_FUNCNAME, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    12, "HID:WIZARDS_HID_DLGREPORT_4_TITLE", 95, 37, new Integer(ReportWizard.SOSTOREPAGE), new Short(curtabindex++), 209
                });

        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblChooseReportKind",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, slblChooseReportKind, 95, 57, new Integer(ReportWizard.SOSTOREPAGE), new Short(curtabindex++), 209
                });

        CurUnoDialog.insertRadioButton("optCreateDocument", TOGGLESUBTEMPLATECONTROLS_FUNCNAME, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    10, "HID:WIZARDS_HID_DLGREPORT_5_OPTSTATDOCUMENT", sSaveAsDocument, 95, 69, new Short((short) 0), new Integer(ReportWizard.SOSTOREPAGE), new Short(curtabindex++), 138
                });

        CurUnoDialog.insertRadioButton("optCreateReportTemplate", TOGGLESUBTEMPLATECONTROLS_FUNCNAME, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, "HID:WIZARDS_HID_DLGREPORT_5_OPTDYNTEMPLATE", sSaveAsTemplate, 95, 81, new Short((short) 1), new Integer(ReportWizard.SOSTOREPAGE), new Short(curtabindex++), 209
                });


        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblHowProceed",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, slblHowProceed, 105, 93, new Integer(ReportWizard.SOSTOREPAGE), new Short(curtabindex++), 209
                });


        CurUnoDialog.insertRadioButton("optEditTemplate", TOGGLESUBTEMPLATECONTROLS_FUNCNAME, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    10, "HID:WIZARDS_HID_DLGREPORT_5_OPTEDITTEMPLATE", sEditTemplate, 111, 105, 6, new Short(curtabindex++), 138
                });

        CurUnoDialog.insertRadioButton("optUseTemplate", TOGGLESUBTEMPLATECONTROLS_FUNCNAME, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    10, "HID:WIZARDS_HID_DLGREPORT_5_OPTUSETEMPLATE", sUseTemplate, 111, 115, new Short((short) 1), new Integer(ReportWizard.SOSTOREPAGE), new Short(curtabindex++), 138
                });
    }


    /*
     * This function is called if one of the radio buttons is pressed
     */
    public void toggleSubTemplateControls()
    {
        // String sStorePath = PropertyNames.EMPTY_STRING;
        Short iState = (Short) CurUnoDialog.getControlProperty("optCreateReportTemplate", PropertyNames.PROPERTY_STATE);
        boolean bDoTemplateEnable = iState.shortValue() == 1;
        CurUnoDialog.setControlProperty("optEditTemplate", PropertyNames.PROPERTY_ENABLED, bDoTemplateEnable);
        CurUnoDialog.setControlProperty("optUseTemplate", PropertyNames.PROPERTY_ENABLED, bDoTemplateEnable);
        CurUnoDialog.setControlProperty("lblHowProceed", PropertyNames.PROPERTY_ENABLED, bDoTemplateEnable);

        String sTitle = xTitleTextBox.getText();
        boolean bDoEnable = sTitle.equals(PropertyNames.EMPTY_STRING);
        CurUnoDialog.enableFinishButton(!bDoEnable);
    }
    public void initialize(RecordParser _CurDBMetaData)
    {
        String FirstCommandName = (_CurDBMetaData.getIncludedCommandNames())[0];
        DefaultName = Desktop.getUniqueName(_CurDBMetaData.getReportDocuments(), FirstCommandName);
        if (!DefaultName.equals(OldDefaultName))
        {
            OldDefaultName = DefaultName;
        }
        xTitleTextBox.setText(DefaultName);
    }

    public String getStoreName()
    {
        if (CurUnoDialog != null)
        {
            String LocStoreName = xTitleTextBox.getText();
            if (!LocStoreName.equals(PropertyNames.EMPTY_STRING))
            {
                StoreName = LocStoreName;
            }
        }
        return (StoreName);
    }

    public String getStorePath()
    {
        try
        {
            StoreName = getStoreName();
            String StorePath;
            XInterface xInterface = (XInterface) m_xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);
            StorePath = FileAccess.getOfficePath(m_xMSF, "Temp", xSimpleFileAccess) + "/" + StoreName;
            return StorePath;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return PropertyNames.EMPTY_STRING;
        }
    }

    public void changeReportTitle()
    {
        final String TitleName = xTitleTextBox.getText();
        CurReportDocument.liveupdate_updateReportTitle(TitleName);
        CurUnoDialog.enableFinishButton(!PropertyNames.EMPTY_STRING.equals(TitleName));
    }

    public int getReportOpenMode()
    {
        int ReportMode = SOCREATEDOCUMENT;
        boolean bcreateTemplate = ((Short) CurUnoDialog.getControlProperty("optCreateReportTemplate", PropertyNames.PROPERTY_STATE)).shortValue() == (short) 1;
        if (bcreateTemplate)
        {
            ReportMode = SOCREATETEMPLATE;
        }
        boolean buseTemplate = ((Short) CurUnoDialog.getControlProperty("optUseTemplate", PropertyNames.PROPERTY_STATE)).shortValue() == (short) 1;
        if (buseTemplate)
        {
            ReportMode = SOUSETEMPLATE;
        }
        boolean buseDocument = ((Short) CurUnoDialog.getControlProperty("optCreateDocument", PropertyNames.PROPERTY_STATE)).shortValue() == (short) 1;
        if (buseDocument)
        {
            ReportMode = SOCREATEDOCUMENT;
        }
        return ReportMode;
    }

    public boolean finish()
    {
        StoreName = getStoreName();
        if (!CurReportDocument.getRecordParser().getReportDocuments().hasByHierarchicalName(StoreName))
        {
            try
            {
                CurReportDocument.store(StoreName, getReportOpenMode());
                ReportWizard.bCloseDocument = false;
                return true;
            }
            catch(Exception e)
            {
                CurUnoDialog.showMessageBox("ErrorBox", VclWindowPeerAttribute.OK,e.getLocalizedMessage() );
                CurUnoDialog.enableFinishButton(false);
                return false;
            }
        }
        String sMsgReportDocumentNameDuplicate = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 76);
        String sShowMsgReportNameisDuplicate = JavaTools.replaceSubString(sMsgReportDocumentNameDuplicate, StoreName, "%REPORTNAME");
        /* int iMsg = */ CurUnoDialog.showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sShowMsgReportNameisDuplicate);
        CurUnoDialog.enableFinishButton(false);
        return false;
    }
}
