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
package com.sun.star.wizards.letter;

import com.sun.star.awt.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.*;

public abstract class LetterWizardDialog extends WizardDialog implements LetterWizardDialogConst, UIConsts
{
    //GUI Components as Class members.
    XRadioButton optBusinessLetter;
    XListBox lstBusinessStyle;
    XCheckBox chkBusinessPaper;
    XRadioButton optPrivOfficialLetter;
    XListBox lstPrivOfficialStyle;
    XRadioButton optPrivateLetter;
    XListBox lstPrivateStyle;
    XFixedText lblBusinessStyle;
    XFixedText lblPrivOfficialStyle;
    XFixedText lblTitle1;
    XFixedText lblPrivateStyle;
    XFixedText lblIntroduction;
    //Image Control
    XControl ImageControl3;
    XCheckBox chkPaperCompanyLogo;
    XNumericField numLogoHeight;
    XNumericField numLogoX;
    XNumericField numLogoWidth;
    XNumericField numLogoY;
    XCheckBox chkPaperCompanyAddress;
    XNumericField numAddressHeight;
    XNumericField numAddressX;
    XNumericField numAddressWidth;
    XNumericField numAddressY;
    XCheckBox chkCompanyReceiver;
    XCheckBox chkPaperFooter;
    XNumericField numFooterHeight;
    XFixedText lblLogoHeight;
    XFixedText lblLogoWidth;
    //Fixed Line
    XControl FixedLine5;
    //Fixed Line
    XControl FixedLine6;
    XFixedText lblFooterHeight;
    XFixedText lblLogoX;
    XFixedText lblLogoY;
    XFixedText lblAddressHeight;
    XFixedText lblAddressWidth;
    XFixedText lblAddressX;
    XFixedText lblAddressY;
    XFixedText lblTitle2;
    XCheckBox chkUseLogo;
    XCheckBox chkUseAddressReceiver;
    XCheckBox chkUseSigns;
    XCheckBox chkUseSubject;
    XCheckBox chkUseSalutation;
    XComboBox lstSalutation;
    XCheckBox chkUseBendMarks;
    XCheckBox chkUseGreeting;
    XComboBox lstGreeting;
    XCheckBox chkUseFooter;
    XFixedText lblTitle3;
    XRadioButton optSenderPlaceholder;
    XRadioButton optSenderDefine;
    XTextComponent txtSenderName;
    XTextComponent txtSenderStreet;
    XTextComponent txtSenderPostCode;
    XTextComponent txtSenderState;
    XTextComponent txtSenderCity;
    XRadioButton optReceiverPlaceholder;
    XRadioButton optReceiverDatabase;
    XFixedText lblSenderAddress;
    //Fixed Line
    XControl FixedLine2;
    XFixedText lblReceiverAddress;
    XFixedText lblSenderName;
    XFixedText lblSenderStreet;
    XFixedText lblPostCodeCity;
    XFixedText lblTitle4;
    XTextComponent txtFooter;
    XCheckBox chkFooterNextPages;
    XCheckBox chkFooterPageNumbers;
    XFixedText lblFooter;
    XFixedText lblTitle5;
    XTextComponent txtTemplateName;
    //File Control
    XControl fileTemplatePath;
    XRadioButton optCreateLetter;
    XRadioButton optMakeChanges;
    XFixedText lblFinalExplanation1;
    XFixedText lblProceed;
    XFixedText lblFinalExplanation2;
    //Image Control
    XControl ImageControl2;
    XFixedText lblTemplateName;
    XFixedText lblTemplatePath;
    XFixedText lblTitle6;
    XFixedText Label9;
    //Font Descriptors as Class members.
    FontDescriptor fontDescriptor1 = new FontDescriptor();
    FontDescriptor fontDescriptor2 = new FontDescriptor();
    FontDescriptor fontDescriptor5 = new FontDescriptor();
    FontDescriptor fontDescriptor6 = new FontDescriptor();    //Resources Object
    LetterWizardDialogResources resources;

    public LetterWizardDialog(XMultiServiceFactory xmsf)
    {
        super(xmsf, HIDMAIN);
        //Load Resources
        resources = new LetterWizardDialogResources(xmsf);


        //set dialog properties...
        Helper.setUnoPropertyValues(xDialogModel,
                new String[]
                {
                    PropertyNames.PROPERTY_CLOSEABLE, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_MOVEABLE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_TITLE, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, 210, Boolean.TRUE, "LetterWizardDialog", 104, 52, INTEGERS[1], new Short((short) 1), resources.resLetterWizardDialog_title, 310
                });



        //Set member- FontDescriptors...
        fontDescriptor1.Weight = 150;
        fontDescriptor1.Underline = com.sun.star.awt.FontUnderline.SINGLE;
        fontDescriptor2.Weight = 100;
        fontDescriptor5.Weight = 100;
        fontDescriptor6.Weight = 150;
    }
    //build components
    public void buildStep1()
    {
        optBusinessLetter = insertRadioButton("optBusinessLetter", OPTBUSINESSLETTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 1), resources.resoptBusinessLetter_value, "optBusinessLetter", 97, 28, INTEGERS[1], new Short((short) 1), 184
                });
        optPrivOfficialLetter = insertRadioButton("optPrivOfficialLetter", OPTPRIVOFFICIALLETTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 2), resources.resoptPrivOfficialLetter_value, "optPrivOfficialLetter", 97, 74, INTEGERS[1], new Short((short) 2), 184
                });
        optPrivateLetter = insertRadioButton("optPrivateLetter", OPTPRIVATELETTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 3), resources.resoptPrivateLetter_value, "optPrivateLetter", 97, 106, INTEGERS[1], new Short((short) 3), 184
                });
        lstBusinessStyle = insertListBox("lstBusinessStyle", LSTBUSINESSSTYLE_ACTION_PERFORMED, LSTBUSINESSSTYLE_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 4), "lstBusinessStyle", 180, INTEGER_40, INTEGERS[1], new Short((short) 4), 74
                });
        chkBusinessPaper = insertCheckBox("chkBusinessPaper", CHKBUSINESSPAPER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 5), resources.reschkBusinessPaper_value, "chkBusinessPaper", 110, 56, new Short((short) 0), INTEGERS[1], new Short((short) 5), 168
                });
        lstPrivOfficialStyle = insertListBox("lstPrivOfficialStyle", LSTPRIVOFFICIALSTYLE_ACTION_PERFORMED, LSTPRIVOFFICIALSTYLE_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 6), "lstPrivOfficialStyle", 180, 86, INTEGERS[1], new Short((short) 6), 74
                });
        lstPrivateStyle = insertListBox("lstPrivateStyle", LSTPRIVATESTYLE_ACTION_PERFORMED, LSTPRIVATESTYLE_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 7), "lstPrivateStyle", 180, 118, INTEGERS[1], new Short((short) 7), 74
                });
        lblBusinessStyle = insertLabel("lblBusinessStyle",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblBusinessStyle_value, "lblBusinessStyle", 110, 42, INTEGERS[1], new Short((short) 48), 60
                });
        lblPrivOfficialStyle = insertLabel("lblPrivOfficialStyle",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblPrivOfficialStyle_value, "lblPrivOfficialStyle", 110, 88, INTEGERS[1], new Short((short) 49), 60
                });
        lblTitle1 = insertLabel("lblTitle1",
                new String[]
                {
                    PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle1_value, Boolean.TRUE, "lblTitle1", 91, INTEGERS[8], INTEGERS[1], new Short((short) 55), 212
                });
        lblPrivateStyle = insertLabel("lblPrivateStyle",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblPrivateStyle_value, "lblPrivateStyle", 110, 120, INTEGERS[1], new Short((short) 74), 60
                });
        lblIntroduction = insertLabel("lblIntroduction",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    39, resources.reslblIntroduction_value, Boolean.TRUE, "lblIntroduction", 104, 145, INTEGERS[1], new Short((short) 80), 199
                });
        ImageControl3 = insertInfoImage(92, 145, 1);

//    ImageControl3 = insertImage("ImageControl3",
//      new String[] {PropertyNames.PROPERTY_BORDER, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH},
//      new Object[] { new Short((short)0), INTEGERS[10],"private:resource/dbu/image/19205","ImageControl3",92,145,Boolean.FALSE,INTEGERS[1],new Short((short)81),INTEGERS[10]}
//    );
    }

    public void buildStep2()
    {
        chkPaperCompanyLogo = insertCheckBox("chkPaperCompanyLogo", CHKPAPERCOMPANYLOGO_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 8), resources.reschkPaperCompanyLogo_value, "chkPaperCompanyLogo", 97, 28, new Short((short) 0), INTEGERS[2], new Short((short) 8), 68
                });
        numLogoHeight = insertNumericField("numLogoHeight", NUMLOGOHEIGHT_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, "StrictFormat", PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 9), "numLogoHeight", 138, INTEGER_40, Boolean.TRUE, INTEGERS[2], Boolean.TRUE, new Short((short) 9), INTEGERS[3], 30
                });
        numLogoX = insertNumericField("numLogoX", NUMLOGOX_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 10), "numLogoX", 266, INTEGER_40, Boolean.TRUE, INTEGERS[2], new Short((short) 10), INTEGERS[0], 30
                });
        numLogoWidth = insertNumericField("numLogoWidth", NUMLOGOWIDTH_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 11), "numLogoWidth", 138, 56, Boolean.TRUE, INTEGERS[2], new Short((short) 11), new Double(3.8), 30
                });
        numLogoY = insertNumericField("numLogoY", NUMLOGOY_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 12), "numLogoY", 266, 56, Boolean.TRUE, INTEGERS[2], new Short((short) 12), new Double(-3.4), 30
                });
        chkPaperCompanyAddress = insertCheckBox("chkPaperCompanyAddress", CHKPAPERCOMPANYADDRESS_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 13), resources.reschkPaperCompanyAddress_value, "chkPaperCompanyAddress", 98, 84, new Short((short) 0), INTEGERS[2], new Short((short) 13), 68
                });
        numAddressHeight = insertNumericField("numAddressHeight", NUMADDRESSHEIGHT_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, "StrictFormat", PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 14), "numAddressHeight", 138, 96, Boolean.TRUE, INTEGERS[2], Boolean.TRUE, new Short((short) 14), INTEGERS[3], 30
                });
        numAddressX = insertNumericField("numAddressX", NUMADDRESSX_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 15), "numAddressX", 266, 96, Boolean.TRUE, INTEGERS[2], new Short((short) 15), new Double(3.8), 30
                });
        numAddressWidth = insertNumericField("numAddressWidth", NUMADDRESSWIDTH_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 16), "numAddressWidth", 138, 112, Boolean.TRUE, INTEGERS[2], new Short((short) 16), new Double(13.8), 30
                });
        numAddressY = insertNumericField("numAddressY", NUMADDRESSY_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 17), "numAddressY", 266, 112, Boolean.TRUE, INTEGERS[2], new Short((short) 17), new Double(-3.4), 30
                });
        chkCompanyReceiver = insertCheckBox("chkCompanyReceiver", CHKCOMPANYRECEIVER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 18), resources.reschkCompanyReceiver_value, "chkCompanyReceiver", 103, 131, new Short((short) 0), INTEGERS[2], new Short((short) 18), 185
                });
        chkPaperFooter = insertCheckBox("chkPaperFooter", CHKPAPERFOOTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 19), resources.reschkPaperFooter_value, "chkPaperFooter", 97, 158, new Short((short) 0), INTEGERS[2], new Short((short) 19), 68
                });
        numFooterHeight = insertNumericField("numFooterHeight", NUMFOOTERHEIGHT_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 20), "numFooterHeight", 236, 156, Boolean.TRUE, INTEGERS[2], new Short((short) 20), INTEGERS[5], 30
                });
        lblLogoHeight = insertLabel("lblLogoHeight",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblLogoHeight_value, "lblLogoHeight", 103, 42, INTEGERS[2], new Short((short) 68), 32
                });
        lblLogoWidth = insertLabel("lblLogoWidth",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblLogoWidth_value, "lblLogoWidth", 103, 58, INTEGERS[2], new Short((short) 69), 32
                });
        FixedLine5 = insertFixedLine("FixedLine5",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[2], "FixedLine5", 90, 78, INTEGERS[2], new Short((short) 70), 215
                });
        FixedLine6 = insertFixedLine("FixedLine6",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[2], "FixedLine6", 90, 150, INTEGERS[2], new Short((short) 71), 215
                });
        lblFooterHeight = insertLabel("lblFooterHeight",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblFooterHeight_value, "lblFooterHeight", 200, 158, INTEGERS[2], new Short((short) 72), 32
                });
        lblLogoX = insertLabel("lblLogoX",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblLogoX_value, "lblLogoX", 170, 42, INTEGERS[2], new Short((short) 84), 94
                });
        lblLogoY = insertLabel("lblLogoY",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblLogoY_value, "lblLogoY", 170, 58, INTEGERS[2], new Short((short) 85), 94
                });
        lblAddressHeight = insertLabel("lblAddressHeight",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblAddressHeight_value, "lblAddressHeight", 103, 98, INTEGERS[2], new Short((short) 86), 32
                });
        lblAddressWidth = insertLabel("lblAddressWidth",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblAddressWidth_value, "lblAddressWidth", 103, 114, INTEGERS[2], new Short((short) 87), 32
                });
        lblAddressX = insertLabel("lblAddressX",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblAddressX_value, "lblAddressX", 170, 98, INTEGERS[2], new Short((short) 88), 94
                });
        lblAddressY = insertLabel("lblAddressY",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblAddressY_value, "lblAddressY", 170, 114, INTEGERS[2], new Short((short) 89), 94
                });
        lblTitle2 = insertLabel("lblTitle2",
                new String[]
                {
                    PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle2_value, Boolean.TRUE, "lblTitle2", 91, INTEGERS[8], INTEGERS[2], new Short((short) 91), 212
                });
    }

    public void buildStep3()
    {
        chkUseLogo = insertCheckBox("chkUseLogo", CHKUSELOGO_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 22), resources.reschkUseLogo_value, "chkUseLogo", 97, 54, new Short((short) 0), INTEGERS[3], new Short((short) 22), 212
                });
        chkUseAddressReceiver = insertCheckBox("chkUseAddressReceiver", CHKUSEADDRESSRECEIVER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 23), resources.reschkUseAddressReceiver_value, "chkUseAddressReceiver", 97, 69, new Short((short) 0), INTEGERS[3], new Short((short) 23), 212
                });
        chkUseSigns = insertCheckBox("chkUseSigns", CHKUSESIGNS_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 24), resources.reschkUseSigns_value, "chkUseSigns", 97, 82, new Short((short) 0), INTEGERS[3], new Short((short) 24), 212
                });
        chkUseSubject = insertCheckBox("chkUseSubject", CHKUSESUBJECT_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 25), resources.reschkUseSubject_value, "chkUseSubject", 97, 98, new Short((short) 0), INTEGERS[3], new Short((short) 25), 212
                });
        chkUseSalutation = insertCheckBox("chkUseSalutation", CHKUSESALUTATION_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 26), resources.reschkUseSalutation_value, "chkUseSalutation", 97, 113, new Short((short) 0), INTEGERS[3], new Short((short) 26), 66
                });
        lstSalutation = insertComboBox("lstSalutation", LSTSALUTATION_ACTION_PERFORMED, LSTSALUTATION_ITEM_CHANGED, LSTSALUTATION_TEXT_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 27), "lstSalutation", 210, 110, INTEGERS[3], new Short((short) 27), 74
                });
        chkUseBendMarks = insertCheckBox("chkUseBendMarks", CHKUSEBENDMARKS_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 28), resources.reschkUseBendMarks_value, "chkUseBendMarks", 97, 127, new Short((short) 0), INTEGERS[3], new Short((short) 28), 212
                });
        chkUseGreeting = insertCheckBox("chkUseGreeting", CHKUSEGREETING_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 29), resources.reschkUseGreeting_value, "chkUseGreeting", 97, 142, new Short((short) 0), INTEGERS[3], new Short((short) 29), 66
                });
        lstGreeting = insertComboBox("lstGreeting", LSTGREETING_ACTION_PERFORMED, LSTGREETING_ITEM_CHANGED, LSTGREETING_TEXT_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 30), "lstGreeting", 210, 141, INTEGERS[3], new Short((short) 30), 74
                });
        chkUseFooter = insertCheckBox("chkUseFooter", CHKUSEFOOTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 31), resources.reschkUseFooter_value, "chkUseFooter", 97, 158, new Short((short) 0), INTEGERS[3], new Short((short) 31), 212
                });
        lblTitle3 = insertLabel("lblTitle3",
                new String[]
                {
                    PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle3_value, Boolean.TRUE, "lblTitle3", 91, INTEGERS[8], INTEGERS[3], new Short((short) 90), 212
                });
    }

    public void buildStep4()
    {
        optSenderPlaceholder = insertRadioButton("optSenderPlaceholder", OPTSENDERPLACEHOLDER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 32), resources.resoptSenderPlaceholder_value, "optSenderPlaceholder", 104, 42, INTEGERS[4], new Short((short) 32), 149
                });
        optSenderDefine = insertRadioButton("optSenderDefine", OPTSENDERDEFINE_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 33), resources.resoptSenderDefine_value, "optSenderDefine", 104, 54, INTEGERS[4], new Short((short) 33), 149
                });
        txtSenderName = insertTextField("txtSenderName", TXTSENDERNAME_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 34), "txtSenderName", 182, 67, INTEGERS[4], new Short((short) 34), 119
                });
        txtSenderStreet = insertTextField("txtSenderStreet", TXTSENDERSTREET_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 35), "txtSenderStreet", 182, 81, INTEGERS[4], new Short((short) 35), 119
                });
        txtSenderPostCode = insertTextField("txtSenderPostCode", TXTSENDERPOSTCODE_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 36), "txtSenderPostCode", 182, 95, INTEGERS[4], new Short((short) 36), 25
                });
        txtSenderState = insertTextField("txtSenderState", TXTSENDERSTATE_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 37), "txtSenderState", 211, 95, INTEGERS[4], new Short((short) 37), 21
                });
        txtSenderCity = insertTextField("txtSenderCity", TXTSENDERCITY_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 38), "txtSenderCity", 236, 95, INTEGERS[4], new Short((short) 38), 65
                });
        optReceiverPlaceholder = insertRadioButton("optReceiverPlaceholder", OPTRECEIVERPLACEHOLDER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 39), resources.resoptReceiverPlaceholder_value, "optReceiverPlaceholder", 104, 145, INTEGERS[4], new Short((short) 39), 200
                });
        optReceiverDatabase = insertRadioButton("optReceiverDatabase", OPTRECEIVERDATABASE_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 40), resources.resoptReceiverDatabase_value, "optReceiverDatabase", 104, 157, INTEGERS[4], new Short((short) 40), 200
                });
        lblSenderAddress = insertLabel("lblSenderAddress",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblSenderAddress_value, "lblSenderAddress", 97, 28, INTEGERS[4], new Short((short) 64), 136
                });
        FixedLine2 = insertFixedLine("FixedLine2",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[5], "FixedLine2", 90, 126, INTEGERS[4], new Short((short) 75), 212
                });
        lblReceiverAddress = insertLabel("lblReceiverAddress",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblReceiverAddress_value, "lblReceiverAddress", 97, 134, INTEGERS[4], new Short((short) 76), 136
                });
        lblSenderName = insertLabel("lblSenderName",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblSenderName_value, "lblSenderName", 113, 69, INTEGERS[4], new Short((short) 77), 68
                });
        lblSenderStreet = insertLabel("lblSenderStreet",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblSenderStreet_value, "lblSenderStreet", 113, 82, INTEGERS[4], new Short((short) 78), 68
                });
        lblPostCodeCity = insertLabel("lblPostCodeCity",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblPostCodeCity_value, "lblPostCodeCity", 113, 97, INTEGERS[4], new Short((short) 79), 68
                });
        lblTitle4 = insertLabel("lblTitle4",
                new String[]
                {
                    PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle4_value, Boolean.TRUE, "lblTitle4", 91, INTEGERS[8], INTEGERS[4], new Short((short) 92), 212
                });
    }

    public void buildStep5()
    {
        txtFooter = insertTextField("txtFooter", TXTFOOTER_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    47, HelpIds.getHelpIdString(HID + 41), Boolean.TRUE, "txtFooter", 97, INTEGER_40, INTEGERS[5], new Short((short) 41), 203
                });
        chkFooterNextPages = insertCheckBox("chkFooterNextPages", CHKFOOTERNEXTPAGES_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 42), resources.reschkFooterNextPages_value, "chkFooterNextPages", 97, 92, new Short((short) 0), INTEGERS[5], new Short((short) 42), 202
                });
        chkFooterPageNumbers = insertCheckBox("chkFooterPageNumbers", CHKFOOTERPAGENUMBERS_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 43), resources.reschkFooterPageNumbers_value, "chkFooterPageNumbers", 97, 106, new Short((short) 0), INTEGERS[5], new Short((short) 43), 201
                });
        lblFooter = insertLabel("lblFooter",
                new String[]
                {
                    PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor5, INTEGERS[8], resources.reslblFooter_value, "lblFooter", 97, 28, INTEGERS[5], new Short((short) 52), 116
                });
        lblTitle5 = insertLabel("lblTitle5",
                new String[]
                {
                    PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle5_value, Boolean.TRUE, "lblTitle5", 91, INTEGERS[8], INTEGERS[5], new Short((short) 93), 212
                });
    }

    public void buildStep6()
    {
        txtTemplateName = insertTextField("txtTemplateName", TXTTEMPLATENAME_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Text", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 44), "txtTemplateName", 202, 56, INTEGERS[6], new Short((short) 44), resources.restxtTemplateName_value, 100
                });
        optCreateLetter = insertRadioButton("optCreateLetter", OPTCREATELETTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 45), resources.resoptCreateLetter_value, "optCreateLetter", 104, 111, INTEGERS[6], new Short((short) 50), 198
                });
        optMakeChanges = insertRadioButton("optMakeChanges", OPTMAKECHANGES_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 46), resources.resoptMakeChanges_value, "optMakeChanges", 104, 123, INTEGERS[6], new Short((short) 51), 198
                });
        lblFinalExplanation1 = insertLabel("lblFinalExplanation1",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    26, resources.reslblFinalExplanation1_value, Boolean.TRUE, "lblFinalExplanation1", 97, 28, INTEGERS[6], new Short((short) 52), 205
                });
        lblProceed = insertLabel("lblProceed",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblProceed_value, "lblProceed", 97, 100, INTEGERS[6], new Short((short) 53), 204
                });
        lblFinalExplanation2 = insertLabel("lblFinalExplanation2",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    33, resources.reslblFinalExplanation2_value, Boolean.TRUE, "lblFinalExplanation2", 104, 145, INTEGERS[6], new Short((short) 54), 199
                });
        ImageControl2 = insertImage("ImageControl2",
                new String[]
                {
                    PropertyNames.PROPERTY_BORDER, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short((short) 0), INTEGERS[10], "private:resource/dbu/image/19205", "ImageControl2", 92, 145, Boolean.FALSE, INTEGERS[6], new Short((short) 66), INTEGERS[10]
                });
        lblTemplateName = insertLabel("lblTemplateName",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblTemplateName_value, "lblTemplateName", 97, 58, INTEGERS[6], new Short((short) 82), 101
                });
        lblTitle6 = insertLabel("lblTitle6",
                new String[]
                {
                    PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle6_value, Boolean.TRUE, "lblTitle6", 91, INTEGERS[8], INTEGERS[6], new Short((short) 94), 212
                });

    }
}
