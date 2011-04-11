/*************************************************************************
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
 ************************************************************************/
package com.sun.star.wizards.web;

import com.sun.star.awt.FontDescriptor;
import com.sun.star.awt.Size;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XComboBox;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XDateField;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.ImageList;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.WizardDialog;
import com.sun.star.wizards.web.data.CGLayout;

/**
 * This class is generated Automatically....
 * bla bla bla
 */
public abstract class WebWizardDialog extends WizardDialog implements WebWizardConst, UIConsts, WWHID
{
    //GUI Components as Class members.
    XFixedText lbIntroTitle;
    XFixedText lblIntroduction;
    //Fixed Line
    XControl lnLoadSettings;
    XFixedText lblLoadSettings;
    XListBox lstLoadSettings;
    XButton btnDelSession;
    XFixedText lblContentTitle;
    //Fixed Line
    XFixedText lblSiteContent;
    XListBox lstDocuments;
    XButton btnAddDoc;
    XButton btnRemoveDoc;
    XButton btnDocUp;
    XButton btnDocDown;
    //Fixed Line
    XControl lnDocsInfo;
    XFixedText lblDocTitle;
    XTextComponent txtDocTitle;
    XFixedText lblDocInfo;
    XTextComponent txtDocInfo;
    XFixedText lblDocAuthor;
    XTextComponent txtDocAuthor;
    XFixedText lblDocExportFormat;
    XListBox lstDocTargetType;
    XFixedText lblLayoutTitle;
    XFixedText lblLayouts;
    /*XFixedText Label1;
    //Image Control
    XControl ImageControl1;
    //Image Control
    XControl ImageControl3;
    //Image Control
    XControl ImageControl4;
    //Image Control
    XControl ImageControl5;
    //Image Control
    XControl ImageControl6;
    XFixedText Label3;
    //titled box
    XControl FrameControl1;
    //Image Control
    XControl ImageControl2;
    //Image Control
    XControl ImageControl7;
    //Image Control
    XControl ImageControl8;
    //Image Control
    XControl ImageControl9;
    //Image Control
    XControl ImageControl10;
    //Image Control
    XControl ImageControl11;
    //Image Control
    XControl ImageControl12;
    //Image Control
    XControl ImageControl13;
    //Image Control
    XControl ImageControl14;
    //Image Control
    XControl ImageControl15;
     */
    XCheckBox chbDocDesc;
    XCheckBox chkDocFilename;
    XCheckBox chbDocAuthor;
    XCheckBox chkDocFormat;
    XCheckBox chkDocFormatIcon;
    XCheckBox chkDocCreated;
    XCheckBox chkDocChanged;
    XCheckBox chkDocPages;
    XCheckBox chkDocSize;
    XFixedText lnDisplay;
    XRadioButton optOptimize640x480;
    XRadioButton optOptimize800x600;
    XRadioButton optOptimize1024x768;
    XFixedText lblLayout2Title;
    XFixedText lblOptimizeFor;    // Step 5
    XFixedText lblStyle;
    XFixedText lblStyleTitle;
    XListBox lstStyles;    //Image Control
    XFixedText txtBackground;
    XFixedText txtIconset;
    XButton btnBackgrounds;
    XButton btnIconSets;
    XControl imgPreview;    // Step 6
    XFixedText lblSiteTitle;
    XFixedText lblSiteCreated;
    XFixedText lblEmail;
    XFixedText lblSiteDesc;
    XTextComponent txtSiteTitle;
    XTextComponent txtSiteDesc;
    XTextComponent txtEmail;
    XDateField dateSiteCreated;
    XDateField dateSiteUpdate;
    XFixedText lblSiteUpdated;
    XTextComponent txtCopyright;
    XFixedText lblCopyright;
    XFixedText lblTitleGeneralPage;
    XButton btnPreview;
    XButton btnFTP;
    XCheckBox chkLocalDir;
    //Fixed Line
    XControl lblCreateSite;
    XCheckBox chkFTP;
    XFixedText lblFTP;
    XCheckBox chkZip;
    XCheckBox chkSaveSettings;
    XFixedText lblSaveSettings;
    //Fixed Line
    XControl lnSaveSetting;
    XComboBox cbSaveSettings;
    XTextComponent txtLocalDir;
    XTextComponent txtZip;
    XButton btnLocalDir;
    XButton btnZip;
    XFixedText lblTitlePublish;
    //Fixed Line
    XControl FixedLine1;
    //Font Descriptors as Class members.
    FontDescriptor fontDescriptor0 = new FontDescriptor();
    FontDescriptor fontDescriptor1 = new FontDescriptor();
    FontDescriptor fontDescriptor4 = new FontDescriptor();
    FontDescriptor fontDescriptor5 = new FontDescriptor();
    FontDescriptor fontDescriptor6 = new FontDescriptor();
    FontDescriptor fontDescriptor7 = new FontDescriptor();    //private static String[] PROPNAMES_LBL_NOFOCUS =     new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH};
    private static String[] PROPNAMES_LBL = new String[]
    {
        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
    };
    private static String[] PROPNAMES_CHKBOX = new String[]
    {
        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
    };
    private static String[] PROPNAMES_BUTTON = new String[]
    {
        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
    };
    private static String[] PROPNAMES_TXT = new String[]
    {
        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
    };
    private static String[] PROPNAMES_TXT_2 = new String[]
    {
        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ReadOnly", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
    };
    private static String[] PROPNAMES_TITLE = new String[]
    {
        "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
    };
    private static Integer INTEGER_91 = new Integer(91);
    private static Integer INTEGER_97 = new Integer(97);
    private static Integer INTEGER_103 = new Integer(103);
    private static Integer INTEGER_169 = new Integer(169);    //Resources Object
    WebWizardDialogResources resources;

    public WebWizardDialog(XMultiServiceFactory xmsf)
    {
        super(xmsf, HID0_WEBWIZARD);
        //Load Resources
        resources = new WebWizardDialogResources(xmsf, super.getResource());


        //set dialog properties...
        Helper.setUnoPropertyValues(xDialogModel,
                new String[]
                {
                    "Closeable", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "Moveable", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Title", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, new Integer(210), HelpIds.getHelpIdString(HID0_WEBWIZARD), Boolean.TRUE, "WebWizardDialog", new Integer(102), new Integer(52), INTEGERS[1], new Short((short) 6), resources.resWebWizardDialog_title, new Integer(330)
                });

        //Set member- FontDescriptors...
        fontDescriptor0.Weight = 100;
        fontDescriptor1.Weight = 150;
        fontDescriptor1.Underline = com.sun.star.awt.FontUnderline.SINGLE;
        fontDescriptor4.Weight = 150;
        fontDescriptor5.Name = "Angsana New";
        fontDescriptor5.Height = 16;
        fontDescriptor5.StyleName = "Regular";
        fontDescriptor5.Family = com.sun.star.awt.FontFamily.ROMAN;
        fontDescriptor5.Weight = 100;
        fontDescriptor6.Name = "Andalus";
        fontDescriptor6.Height = 22;
        fontDescriptor6.StyleName = "Bold Italic";
        fontDescriptor6.Weight = 150;
        fontDescriptor6.Slant = com.sun.star.awt.FontSlant.ITALIC;
        fontDescriptor7.Name = "StarSymbol";
        fontDescriptor7.Height = 9;
        fontDescriptor7.StyleName = "Regular";
        fontDescriptor7.Weight = 100;
    }
    //build components
    public void buildStep1()
    {
        short tabIndex = 100;
        lbIntroTitle = insertLabel("lbIntroTitle",
                PROPNAMES_TITLE,
                new Object[]
                {
                    fontDescriptor4, INTEGER_16, resources.reslbIntroTitle_value, Boolean.TRUE, "lbIntroTitle", new Integer(91), INTEGERS[8], INTEGERS[1], new Short(tabIndex++), new Integer(232)
                });
        lblIntroduction = insertLabel("lblIntroduction",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(119), resources.reslblIntroduction_value, Boolean.TRUE, "lblIntroduction", new Integer(97), new Integer(28), INTEGERS[1], new Short(tabIndex++), new Integer(226)
                });

        lnLoadSettings = insertFixedLine("lnLoadSettings",
                PROPNAMES_TXT,
                new Object[]
                {
                    INTEGERS[2], "", "lnLoadSettings", new Integer(91), new Integer(147), INTEGERS[1], new Short(tabIndex++), new Integer(234)
                });
        lblLoadSettings = insertLabel("lblLoadSettings",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblLoadSettings_value, Boolean.TRUE, "lblLoadSettings", new Integer(97), new Integer(153), INTEGERS[1], new Short(tabIndex++), new Integer(226)
                });
        lstLoadSettings = insertListBox("lstLoadSettings", null, LSTLOADSETTINGS_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID1_LST_SESSIONS), new Short((short) 14), "lstLoadSettings", new Integer(97), new Integer(165), INTEGERS[1], new Short(tabIndex++), new Integer(173)
                });
        btnDelSession = insertButton("btnDelSession", BTNDELSESSION_ACTION_PERFORMED,
                new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "ImageAlign", PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, INTEGER_14, HelpIds.getHelpIdString(HID1_BTN_DEL_SES), new Short(com.sun.star.awt.ImageAlign.LEFT), resources.resbtnDelSession_value, "btnDelSession", new Integer(274), new Integer(164), INTEGERS[1], new Short(tabIndex++), INTEGER_50
                });
    }

    public void buildStep2()
    {
        short tabIndex = 200;

        lblContentTitle = insertLabel("lblContentTitle",
                PROPNAMES_TITLE,
                new Object[]
                {
                    fontDescriptor4, INTEGER_16, resources.reslblContentTitle_value, Boolean.TRUE, "lblContentTitle", new Integer(91), INTEGERS[8], INTEGERS[2], new Short(tabIndex++), new Integer(232)
                });
        lblSiteContent = insertLabel("lblSiteContent",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblSiteContent_value, "lblSiteContent", new Integer(97), new Integer(28), INTEGERS[2], new Short(tabIndex++), new Integer(105)
                });
        lstDocuments = insertListBox("lstDocuments", null, null,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(123), HelpIds.getHelpIdString(HID2_LST_DOCS), new Short((short) 9), "lstDocuments", new Integer(97), new Integer(38), INTEGERS[2], new Short(tabIndex++), new Integer(103)
                });
        btnAddDoc = insertButton("btnAddDoc", BTNADDDOC_ACTION_PERFORMED,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID2_BTN_ADD_DOC), resources.resbtnAddDoc_value, "btnAddDoc", new Integer(97), new Integer(165), INTEGERS[2], new Short(tabIndex++), INTEGER_50
                });
        btnRemoveDoc = insertButton("btnRemoveDoc", BTNREMOVEDOC_ACTION_PERFORMED,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID2_BTN_REM_DOC), resources.resbtnRemoveDoc_value, "btnRemoveDoc", new Integer(150), new Integer(165), INTEGERS[2], new Short(tabIndex++), INTEGER_50
                });
        btnDocUp = insertButton("btnDocUp", BTNDOCUP_ACTION_PERFORMED,
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor7, INTEGER_14, HelpIds.getHelpIdString(HID2_BTN_DOC_UP), resources.resbtnDocUp_value, "btnDocUp", new Integer(205), new Integer(87), INTEGERS[2], new Short(tabIndex++), new Integer(18)
                });
        btnDocDown = insertButton("btnDocDown", BTNDOCDOWN_ACTION_PERFORMED,
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor7, INTEGER_14, HelpIds.getHelpIdString(HID2_BTN_DOC_DOWN), resources.resbtnDocDown_value, "btnDocDown", new Integer(205), new Integer(105), INTEGERS[2], new Short(tabIndex++), new Integer(18)
                });

        lblDocExportFormat = insertLabel("lblDocExportFormat",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblDocExportFormat_value, "lblDocExportFormat", new Integer(235), new Integer(28), INTEGERS[2], new Short(tabIndex++), new Integer(89)
                });
        lstDocTargetType = insertListBox("lstDocTargetType", null, null,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID2_LST_DOC_EXPORT), new Short((short) 14), "lstDocTargetType", new Integer(235), new Integer(38), INTEGERS[2], new Short(tabIndex++), new Integer(89)
                });




        lnDocsInfo = insertFixedLine("lnDocsInfo",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslnDocsInfo_value, "lnDocsInfo", new Integer(235), new Integer(66), INTEGERS[2], new Short(tabIndex++), new Integer(90)
                });
        lblDocTitle = insertLabel("lblDocTitle",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblDocTitle_value, "lblDocTitle", new Integer(235), new Integer(78), INTEGERS[2], new Short(tabIndex++), new Integer(89)
                });
        txtDocTitle = insertTextField("txtDocTitle", null,
                PROPNAMES_TXT,
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID2_TXT_DOC_TITLE), "txtDocTitle", new Integer(235), new Integer(88), INTEGERS[2], new Short(tabIndex++), new Integer(89)
                });
        lblDocInfo = insertLabel("lblDocInfo",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblDocInfo_value, "lblDocInfo", new Integer(235), new Integer(103), INTEGERS[2], new Short(tabIndex++), new Integer(89)
                });
        txtDocInfo = insertTextField("txtDocInfo", null,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_40, HelpIds.getHelpIdString(HID2_TXT_DOC_DESC), Boolean.TRUE, "txtDocInfo", new Integer(235), new Integer(113), INTEGERS[2], new Short(tabIndex++), new Integer(89)
                });
        lblDocAuthor = insertLabel("lblDocAuthor",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblDocAuthor_value, "lblDocAuthor", new Integer(235), new Integer(155), INTEGERS[2], new Short(tabIndex++), new Integer(89)
                });
        txtDocAuthor = insertTextField("txtDocAuthor", null,
                PROPNAMES_TXT,
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID2_TXT_DOC_AUTHOR), "txtDocAuthor", new Integer(235), new Integer(165), INTEGERS[2], new Short(tabIndex++), new Integer(89)
                });
    }

    public void buildStep3()
    {
        lblLayoutTitle = insertLabel("lblLayoutTitle",
                PROPNAMES_TITLE,
                new Object[]
                {
                    fontDescriptor4, INTEGER_16, resources.reslblLayoutTitle_value, Boolean.TRUE, "lblLayoutTitle", new Integer(91), INTEGERS[8], INTEGERS[3], new Short((short) 29), new Integer(232)
                });
        lblLayouts = insertLabel("lblLayouts",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblLayouts_value, "lblLayouts", new Integer(97), new Integer(28), INTEGERS[3], new Short((short) 30), new Integer(206)
                });

    }

    public void buildStep4()
    {
        lblLayout2Title = insertLabel("lblLayout2Title",
                PROPNAMES_TITLE,
                new Object[]
                {
                    fontDescriptor4, INTEGER_16, resources.reslblLayout2Title_value, Boolean.TRUE, "lblLayout2Title", new Integer(91), INTEGERS[8], INTEGERS[4], new Short((short) 33), new Integer(232)
                });
        lnDisplay = insertLabel("lblDisplay",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_16, resources.reslblDisplay_value, Boolean.TRUE, "lblDisplay", new Integer(97), new Integer(28), INTEGERS[4], new Short((short) 34), new Integer(226)
                });
        chkDocFilename = insertCheckBox("chkDocFilename", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID4_CHK_DISPLAY_FILENAME), resources.reschkDocFilename_value, "chkDocFilename", new Integer(103), new Integer(50), new Short((short) 0), INTEGERS[4], new Short((short) 35), new Integer(99)
                });
        chbDocDesc = insertCheckBox("chbDocDesc", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID4_CHK_DISPLAY_DESCRIPTION), resources.reschbDocDesc_value, "chbDocDesc", new Integer(103), new Integer(60), new Short((short) 0), INTEGERS[4], new Short((short) 36), new Integer(99)
                });
        chbDocAuthor = insertCheckBox("chbDocAuthor", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID4_CHK_DISPLAY_AUTHOR), resources.reschbDocAuthor_value, "chbDocAuthor", new Integer(103), new Integer(70), new Short((short) 0), INTEGERS[4], new Short((short) 37), new Integer(99)
                });
        chkDocCreated = insertCheckBox("chkDocCreated", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID4_CHK_DISPLAY_CR_DATE), resources.reschkDocCreated_value, "chkDocCreated", new Integer(103), new Integer(80), new Short((short) 0), INTEGERS[4], new Short((short) 38), new Integer(99)
                });
        chkDocChanged = insertCheckBox("chkDocChanged", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID4_CHK_DISPLAY_UP_DATE), resources.reschkDocChanged_value, "chkDocChanged", new Integer(103), new Integer(90), new Short((short) 0), INTEGERS[4], new Short((short) 39), new Integer(99)
                });

        chkDocFormat = insertCheckBox("chkDocFormat", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID4_CHK_DISPLAY_FORMAT), resources.reschkDocFormat_value, "chkDocFormat", new Integer(200), new Integer(50), new Short((short) 0), INTEGERS[4], new Short((short) 40), new Integer(110)
                });
        chkDocFormatIcon = insertCheckBox("chkDocFormatIcon", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID4_CHK_DISPLAY_F_ICON), resources.reschkDocFormatIcon_value, "chkDocFormatIcon", new Integer(200), new Integer(60), new Short((short) 0), INTEGERS[4], new Short((short) 41), new Integer(110)
                });
        chkDocPages = insertCheckBox("chkDocPages", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID4_CHK_DISPLAY_PAGES), resources.reschkDocPages_value, "chkDocPages", new Integer(200), new Integer(70), new Short((short) 0), INTEGERS[4], new Short((short) 42), new Integer(110)
                });
        chkDocSize = insertCheckBox("chkDocSize", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID4_CHK_DISPLAY_SIZE), resources.reschkDocSize_value, "chkDocSize", new Integer(200), new Integer(80), new Short((short) 0), INTEGERS[4], new Short((short) 43), new Integer(110)
                });
        lblOptimizeFor = insertLabel("lblOptimizeFor",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblOptimizeFor_value, "lblOptimizeFor", new Integer(97), new Integer(113), INTEGERS[4], new Short((short) 44), new Integer(226)
                });
        optOptimize640x480 = insertRadioButton("optOptimize640x480", null,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGERS[10], HelpIds.getHelpIdString(HID4_GRP_OPTIMAIZE_640), resources.resoptOptimize640x480_value, "optOptimize640x480", new Integer(103), new Integer(133), INTEGERS[4], new Short((short) 45), new Integer(44)
                });
        optOptimize800x600 = insertRadioButton("optOptimize800x600", null,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGERS[10], HelpIds.getHelpIdString(HID4_GRP_OPTIMAIZE_800), resources.resoptOptimize800x600_value, "optOptimize800x600", new Integer(103), new Integer(146), INTEGERS[4], new Short((short) 46), new Integer(44)
                });
        optOptimize1024x768 = insertRadioButton("optOptimize1024x768", null,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGERS[10], HelpIds.getHelpIdString(HID4_GRP_OPTIMAIZE_1024), resources.resoptOptimize1024x768_value, "optOptimize1024x768", new Integer(103), new Integer(158), INTEGERS[4], new Short((short) 47), new Integer(44)
                });

    }

    public void buildStep5()
    {
        lblStyleTitle = insertLabel("lblStyleTitle", PROPNAMES_TITLE,
                new Object[]
                {
                    fontDescriptor4, INTEGER_16, resources.reslblStyleTitle_value, Boolean.TRUE, "lblStyleTitle", new Integer(91), INTEGERS[8], INTEGERS[5], new Short((short) 50), new Integer(232)
                });

        lblStyle = insertLabel("lblStyle", PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblStyle_value, "lblStyle", new Integer(97), new Integer(28), INTEGERS[5], new Short((short) 51), new Integer(80)
                });
        lstStyles = insertListBox("lstStyles", null, LSTSTYLES_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID5_LST_STYLES), new Short((short) 14), "lstStyles", new Integer(179), new Integer(26), INTEGERS[5], new Short((short) 52), new Integer(145)
                });

        insertLabel("lblBackground", PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblBackground, "lblBackground", new Integer(97), new Integer(46), INTEGERS[5], new Short((short) 51), new Integer(70)
                });
        txtBackground = insertLabel("txtBackground",
                new String[]
                {
                    "Border", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short((short) 1), INTEGER_12, resources.resBackgroundNone, "txtBackground", new Integer(179), new Integer(44), INTEGERS[5], new Short((short) 52), new Integer(90)
                });
        btnBackgrounds = insertButton("btnBackgrounds", BTNBACKGROUNDS_ACTION_PERFORMED, PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID5_BTN_BACKGND), resources.resBtnChooseBackground, "btnBackgrounds", new Integer(274), new Integer(43), INTEGERS[5], new Short((short) 53), INTEGER_50
                });

        insertLabel("lblIconset", PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblIconset, "lblIconset", new Integer(97), new Integer(64), INTEGERS[5], new Short((short) 51), new Integer(70)
                });
        txtIconset = insertLabel("txtIconset",
                new String[]
                {
                    "Border", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short((short) 1), INTEGER_12, resources.resIconsetNone, "txtIconset", new Integer(179), new Integer(62), INTEGERS[5], new Short((short) 52), new Integer(90)
                });
        btnIconSets = insertButton("btnIconSets", BTNICONSETS_ACTION_PERFORMED,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID5_BTN_ICONS), resources.resBtnChooseIconset, "btnIconSets", new Integer(274), new Integer(61), INTEGERS[5], new Short((short) 54), INTEGER_50
                });
        insertLabel("lblIconsetInfo",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_16, resources.reslblIconSetInfo, Boolean.TRUE, "lblIconsetInfo", new Integer(179), new Integer(78), INTEGERS[5], new Short((short) 51), new Integer(145)
                });


        imgPreview = insertImage("imgPreview",
                new String[]
                {
                    "BackgroundColor", PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(0), Boolean.FALSE, new Integer(78), "", "imgPreview", new Integer(91), new Integer(100), Boolean.FALSE, INTEGERS[5], new Short((short) 55), new Integer(232)
                });

    }

    public void buildStep6()
    {
        short tabIndex = 60;
        lblTitleGeneralPage = insertLabel("lblTitleGeneralPage",
                PROPNAMES_TITLE,
                new Object[]
                {
                    fontDescriptor4, INTEGERS[8], resources.reslblTitleGeneralPage_value, Boolean.TRUE, "lblTitleGeneralPage", new Integer(90), INTEGERS[9], INTEGERS[6], new Short(tabIndex++), new Integer(232)
                });
        lblSiteTitle = insertLabel("lblSiteTitle",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblSiteTitle_value, "lblSiteTitle", new Integer(97), new Integer(28), INTEGERS[6], new Short(tabIndex++), new Integer(80)
                });
        txtSiteTitle = insertTextField("txtSiteTitle", null,
                PROPNAMES_TXT,
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID6_TXT_SITE_TITLE), "txtSiteTitle", new Integer(179), new Integer(26), INTEGERS[6], new Short(tabIndex++), new Integer(145)
                });


        insertFixedLine("FixedLineMetaData",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[9], resources.reslblMetaData, "FixedLineMetaData", new Integer(97), new Integer(56), INTEGERS[6], new Short(tabIndex++), new Integer(228)
                });


        lblSiteDesc = insertLabel("lblSiteDesc",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblSiteDesc_value, "lblSiteDesc", new Integer(103), new Integer(72), INTEGERS[6], new Short(tabIndex++), new Integer(80)
                });
        txtSiteDesc = insertTextField("txtSiteDesc", null,
                PROPNAMES_TXT,
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID6_TXT_SITE_DESC), "txtSiteDesc", new Integer(179), new Integer(70), INTEGERS[6], new Short(tabIndex++), new Integer(145)
                });

        lblEmail = insertLabel("lblEmail", PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblEmail_value, "lblEmail", new Integer(103), new Integer(90), INTEGERS[6], new Short(tabIndex++), new Integer(80)
                });
        txtEmail = insertTextField("txtEmail", null, PROPNAMES_TXT,
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID6_TXT_SITE_EMAIL), "txtEmail", new Integer(179), new Integer(87), INTEGERS[6], new Short(tabIndex++), new Integer(145)
                });

        lblCopyright = insertLabel("lblCopyright", PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblCopyright_value, "lblCopyright", new Integer(103), new Integer(108), INTEGERS[6], new Short(tabIndex++), new Integer(80)
                });
        txtCopyright = insertTextField("txtCopyright", null, PROPNAMES_TXT,
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID6_TXT_SITE_COPYRIGHT), "txtCopyright", new Integer(179), new Integer(106), INTEGERS[6], new Short(tabIndex++), new Integer(145)
                });


        lblSiteCreated = insertLabel("lblSiteCreated",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblSiteCreated_value, "lblSiteCreated", new Integer(103), new Integer(126), INTEGERS[6], new Short(tabIndex++), new Integer(80)
                });
        dateSiteCreated = insertDateField("dateSiteCreated", null,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID6_DATE_SITE_CREATED), "dateSiteCreated", new Integer(179), new Integer(124), INTEGERS[6], new Short(tabIndex++), new Integer(49)
                });
        lblSiteUpdated = insertLabel("lblSiteUpdated",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblSiteUpdated_value, "lblSiteUpdated", new Integer(103), new Integer(144), INTEGERS[6], new Short(tabIndex++), new Integer(80)
                });

        dateSiteUpdate = insertDateField("dateSiteUpdate", null,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID6_DATE_SITE_UPDATED), "dateSiteUpdate", new Integer(179), new Integer(142), INTEGERS[6], new Short(tabIndex++), new Integer(49)
                });



    }

    public void buildStep7(boolean disableFTP, String exclamationURL)
    {
        short tabIndex = 700;

        lblTitlePublish = insertLabel("lblTitlePublish",
                PROPNAMES_TITLE,
                new Object[]
                {
                    fontDescriptor4, INTEGER_16, resources.reslblTitlePublish_value, Boolean.TRUE, "lblTitlePublish", new Integer(91), INTEGERS[8], INTEGERS[7], new Short(tabIndex++), new Integer(232)
                });

        FixedLine1 = insertFixedLine("FixedLine1",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[9], resources.resFixedLine1_value, "FixedLine1", new Integer(97), new Integer(28), INTEGERS[7], new Short(tabIndex++), new Integer(228)
                });

        btnPreview = insertButton("btnPreview", BTNPREVIEW_ACTION_PERFORMED,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID7_BTN_PREVIEW), resources.resbtnPreview_value, "btnPreview", new Integer(103), new Integer(40), INTEGERS[7], new Short(tabIndex++), INTEGER_50
                });

        lblCreateSite = insertFixedLine("lblCreateSite",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[9], resources.reslblCreateSite_value, "lblCreateSite", new Integer(97), new Integer(56), INTEGERS[7], new Short(tabIndex++), new Integer(228)
                });

        chkLocalDir = insertCheckBox("chkLocalDir", CHKLOCALDIR_ITEM_CHANGED,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID7_CHK_PUBLISH_LOCAL), resources.reschkLocalDir_value, "chkLocalDir", new Integer(103), new Integer(68), new Short((short) 0), INTEGERS[7], new Short(tabIndex++), new Integer(215)
                });
        txtLocalDir = insertTextField("txtLocalDir", null,
                PROPNAMES_TXT,
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID7_TXT_LOCAL), "txtLocalDir", new Integer(113), new Integer(78), INTEGERS[7], new Short(tabIndex++), new Integer(190)
                });
        btnLocalDir = insertButton("btnLocalDir", BTNLOCALDIR_ACTION_PERFORMED,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID7_BTN_LOCAL), resources.resbtnLocalDir_value, "btnLocalDir", new Integer(308), new Integer(77), INTEGERS[7], new Short(tabIndex++), INTEGER_16
                });

        chkZip = insertCheckBox("chkZip", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[9], HelpIds.getHelpIdString(HID7_CHK_PUBLISH_ZIP), resources.reschkZip_value, "chkZip", new Integer(103), new Integer(96), new Short((short) 0), INTEGERS[7], new Short(tabIndex++), new Integer(215)
                });
        txtZip = insertTextField("txtZip", null,
                PROPNAMES_TXT,
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID7_TXT_ZIP), "txtZip", new Integer(113), new Integer(108), INTEGERS[7], new Short(tabIndex++), new Integer(190)
                });
        btnZip = insertButton("btnZip", BTNZIP_ACTION_PERFORMED,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID7_BTN_ZIP), resources.resbtnZip_value, "btnZip", new Integer(308), new Integer(107), INTEGERS[7], new Short(tabIndex++), INTEGER_16
                });

        chkFTP = insertCheckBox("chkFTP", null,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID7_CHK_PUBLISH_FTP), resources.reschkFTP_value, "chkFTP", new Integer(103), new Integer(124), new Short((short) 0), INTEGERS[7], new Short(tabIndex++), new Integer(215)
                });

        Integer FTP_STEP = disableFTP ? new Integer(99) : INTEGERS[7];

        lblFTP = insertLabel("lblFTP",
                new String[]
                {
                    "Border", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short((short) 1), INTEGER_12, "txtFTP", new Integer(113), new Integer(134), FTP_STEP, new Short(tabIndex++), new Integer(156)
                });

        btnFTP = insertButton("btnFTP", BTNFTP_ACTION_PERFORMED,
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID7_BTN_FTP), resources.resbtnFTP_value, "btnFTP", new Integer(274), new Integer(133), FTP_STEP, new Short(tabIndex++), INTEGER_50
                });

        if (disableFTP)
        {

            insertImage("imgFTPDisabled",
                    new String[]
                    {
                        "BackgroundColor", "Border", PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        new Integer(-1), new Short((short) 0), Boolean.FALSE, new Integer(10), exclamationURL, "imgFTPDisabled", new Integer(115), new Integer(135), Boolean.FALSE, INTEGERS[7], new Short(tabIndex++), new Integer(8)
                    });
            insertLabel("lblFTPDisabled",
                    new String[]
                    {
                        "BackgroundColor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        new Integer(-1), INTEGER_8, resources.reslblFTPDisabled, Boolean.TRUE, "lblFTPDisabled", new Integer(125), new Integer(136), INTEGERS[7], new Short(tabIndex++), new Integer(226)
                    });
        //FTP_STEP = new Integer(99);
        }
        //else

        lnSaveSetting = insertFixedLine("lnSaveSetting",
                PROPNAMES_TXT,
                new Object[]
                {
                    INTEGERS[2], "", "lnSaveSetting", new Integer(97), new Integer(151), INTEGERS[7], new Short(tabIndex++), new Integer(228)
                });
        chkSaveSettings = insertCheckBox("chkSaveSettings", CHKSAVESETTINGS_ITEM_CHANGED,
                PROPNAMES_CHKBOX,
                new Object[]
                {
                    INTEGERS[9], HelpIds.getHelpIdString(HID7_CHK_SAVE), resources.reschkSaveSettings_value, "chkSaveSettings", new Integer(97), new Integer(157), new Short((short) 1), INTEGERS[7], new Short(tabIndex++), new Integer(215)
                });
        lblSaveSettings = insertLabel("lblSaveSettings",
                PROPNAMES_LBL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblSaveSettings_value, "lblSaveSettings", new Integer(107), new Integer(169), INTEGERS[7], new Short(tabIndex++), new Integer(45)
                });
        cbSaveSettings = insertComboBox("txtSaveSettings", TXTSAVESETTINGS_TEXT_CHANGED, TXTSAVESETTINGS_TEXT_CHANGED, TXTSAVESETTINGS_TEXT_CHANGED,
                new String[]
                {
                    "Autocomplete", "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID7_TXT_SAVE), new Short((short) 14), "txtSaveSettings", new Integer(179), new Integer(167), INTEGERS[7], new Short(tabIndex++), new Integer(145)
                });

    }
    XControl[] imgIconsPrev = new XControl[8];
    ImageList ilLayouts;

    /**
     * builds the layouts image list.
     */
    public void buildStepX()
    {
        ilLayouts = new ImageList();
        ilLayouts.setPos(new Size(97, 38));
        ilLayouts.setImageSize(new Size(34, 30));
        ilLayouts.setCols(5);
        ilLayouts.setRows(3);
        ilLayouts.setStep(new Short((short) 3));
        ilLayouts.setShowButtons(false);
        ilLayouts.setRenderer(new LayoutRenderer());
        ilLayouts.scaleImages = Boolean.FALSE;
        ilLayouts.tabIndex = 31;
        ilLayouts.helpURL = HID3_IL_LAYOUTS_IMG1;

    /*for (int i = 0; i<8; i++)
    imgIconsPrev[i] = insertImage("imgIconPrev" + i,
    new String[] { "BackgroundColor","Border",PropertyNames.PROPERTY_HEIGHT,PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, "Tabstop", PropertyNames.PROPERTY_WIDTH},
    new Object[] { new Integer(-1), new Short((short)0),new Integer(14),"file:///c:/bludisk.gif", new Integer(97 + i * 20 + 7 ),new Integer(147),Boolean.FALSE,INTEGERS[5],Boolean.FALSE,new Integer(14)});
     */
    }

    /**
     * renders the images and titles of
     * the layouts
     * @author rpiterman

     */
    private class LayoutRenderer implements ImageList.IImageRenderer
    {

        public Object[] getImageUrls(Object listItem)
        {
            Object[] oResIds = null;
            if (listItem != null)
            {
                oResIds = ((CGLayout) listItem).getImageUrls();
            }
            return oResIds;
        }

        public String render(Object listItem)
        {
            return listItem == null ? "" : ((CGLayout) listItem).cp_Name;
        }
    }
}
