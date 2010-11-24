/*
 ************************************************************************
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
    XListBox lstLetterNorm;
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
    XFixedText lblLetterNorm;
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
                    "Closeable", PropertyNames.PROPERTY_HEIGHT, "Moveable", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Title", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, new Integer(210), Boolean.TRUE, "LetterWizardDialog", new Integer(104), new Integer(52), INTEGERS[1], new Short((short) 1), resources.resLetterWizardDialog_title, new Integer(310)
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
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 1), resources.resoptBusinessLetter_value, "optBusinessLetter", new Integer(97), new Integer(28), INTEGERS[1], new Short((short) 1), new Integer(184)
                });
        optPrivOfficialLetter = insertRadioButton("optPrivOfficialLetter", OPTPRIVOFFICIALLETTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 2), resources.resoptPrivOfficialLetter_value, "optPrivOfficialLetter", new Integer(97), new Integer(74), INTEGERS[1], new Short((short) 2), new Integer(184)
                });
        optPrivateLetter = insertRadioButton("optPrivateLetter", OPTPRIVATELETTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 3), resources.resoptPrivateLetter_value, "optPrivateLetter", new Integer(97), new Integer(106), INTEGERS[1], new Short((short) 3), new Integer(184)
                });
        lstBusinessStyle = insertListBox("lstBusinessStyle", LSTBUSINESSSTYLE_ACTION_PERFORMED, LSTBUSINESSSTYLE_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 4), "lstBusinessStyle", new Integer(180), INTEGER_40, INTEGERS[1], new Short((short) 4), new Integer(74)
                });
        chkBusinessPaper = insertCheckBox("chkBusinessPaper", CHKBUSINESSPAPER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 5), resources.reschkBusinessPaper_value, "chkBusinessPaper", new Integer(110), new Integer(56), new Short((short) 0), INTEGERS[1], new Short((short) 5), new Integer(168)
                });
        lstPrivOfficialStyle = insertListBox("lstPrivOfficialStyle", LSTPRIVOFFICIALSTYLE_ACTION_PERFORMED, LSTPRIVOFFICIALSTYLE_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 6), "lstPrivOfficialStyle", new Integer(180), new Integer(86), INTEGERS[1], new Short((short) 6), new Integer(74)
                });
        lstPrivateStyle = insertListBox("lstPrivateStyle", LSTPRIVATESTYLE_ACTION_PERFORMED, LSTPRIVATESTYLE_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 7), "lstPrivateStyle", new Integer(180), new Integer(118), INTEGERS[1], new Short((short) 7), new Integer(74)
                });
        lblBusinessStyle = insertLabel("lblBusinessStyle",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblBusinessStyle_value, "lblBusinessStyle", new Integer(110), new Integer(42), INTEGERS[1], new Short((short) 48), new Integer(60)
                });
        lblPrivOfficialStyle = insertLabel("lblPrivOfficialStyle",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblPrivOfficialStyle_value, "lblPrivOfficialStyle", new Integer(110), new Integer(88), INTEGERS[1], new Short((short) 49), new Integer(60)
                });
        lblTitle1 = insertLabel("lblTitle1",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle1_value, Boolean.TRUE, "lblTitle1", new Integer(91), INTEGERS[8], INTEGERS[1], new Short((short) 55), new Integer(212)
                });
        lblPrivateStyle = insertLabel("lblPrivateStyle",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblPrivateStyle_value, "lblPrivateStyle", new Integer(110), new Integer(120), INTEGERS[1], new Short((short) 74), new Integer(60)
                });
        lblIntroduction = insertLabel("lblIntroduction",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(39), resources.reslblIntroduction_value, Boolean.TRUE, "lblIntroduction", new Integer(104), new Integer(145), INTEGERS[1], new Short((short) 80), new Integer(199)
                });
        ImageControl3 = insertInfoImage(92, 145, 1);

//    ImageControl3 = insertImage("ImageControl3",
//      new String[] {"Border", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH},
//      new Object[] { new Short((short)0), INTEGERS[10],"private:resource/dbu/image/19205","ImageControl3",new Integer(92),new Integer(145),Boolean.FALSE,INTEGERS[1],new Short((short)81),INTEGERS[10]}
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
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 8), resources.reschkPaperCompanyLogo_value, "chkPaperCompanyLogo", new Integer(97), new Integer(28), new Short((short) 0), INTEGERS[2], new Short((short) 8), new Integer(68)
                });
        numLogoHeight = insertNumericField("numLogoHeight", NUMLOGOHEIGHT_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, "StrictFormat", PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 9), "numLogoHeight", new Integer(138), INTEGER_40, Boolean.TRUE, INTEGERS[2], Boolean.TRUE, new Short((short) 9), INTEGERS[3], new Integer(30)
                });
        numLogoX = insertNumericField("numLogoX", NUMLOGOX_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 10), "numLogoX", new Integer(266), INTEGER_40, Boolean.TRUE, INTEGERS[2], new Short((short) 10), INTEGERS[0], new Integer(30)
                });
        numLogoWidth = insertNumericField("numLogoWidth", NUMLOGOWIDTH_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 11), "numLogoWidth", new Integer(138), new Integer(56), Boolean.TRUE, INTEGERS[2], new Short((short) 11), new Double(3.8), new Integer(30)
                });
        numLogoY = insertNumericField("numLogoY", NUMLOGOY_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 12), "numLogoY", new Integer(266), new Integer(56), Boolean.TRUE, INTEGERS[2], new Short((short) 12), new Double(-3.4), new Integer(30)
                });
        chkPaperCompanyAddress = insertCheckBox("chkPaperCompanyAddress", CHKPAPERCOMPANYADDRESS_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 13), resources.reschkPaperCompanyAddress_value, "chkPaperCompanyAddress", new Integer(98), new Integer(84), new Short((short) 0), INTEGERS[2], new Short((short) 13), new Integer(68)
                });
        numAddressHeight = insertNumericField("numAddressHeight", NUMADDRESSHEIGHT_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, "StrictFormat", PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 14), "numAddressHeight", new Integer(138), new Integer(96), Boolean.TRUE, INTEGERS[2], Boolean.TRUE, new Short((short) 14), INTEGERS[3], new Integer(30)
                });
        numAddressX = insertNumericField("numAddressX", NUMADDRESSX_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 15), "numAddressX", new Integer(266), new Integer(96), Boolean.TRUE, INTEGERS[2], new Short((short) 15), new Double(3.8), new Integer(30)
                });
        numAddressWidth = insertNumericField("numAddressWidth", NUMADDRESSWIDTH_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 16), "numAddressWidth", new Integer(138), new Integer(112), Boolean.TRUE, INTEGERS[2], new Short((short) 16), new Double(13.8), new Integer(30)
                });
        numAddressY = insertNumericField("numAddressY", NUMADDRESSY_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 17), "numAddressY", new Integer(266), new Integer(112), Boolean.TRUE, INTEGERS[2], new Short((short) 17), new Double(-3.4), new Integer(30)
                });
        chkCompanyReceiver = insertCheckBox("chkCompanyReceiver", CHKCOMPANYRECEIVER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 18), resources.reschkCompanyReceiver_value, "chkCompanyReceiver", new Integer(103), new Integer(131), new Short((short) 0), INTEGERS[2], new Short((short) 18), new Integer(185)
                });
        chkPaperFooter = insertCheckBox("chkPaperFooter", CHKPAPERFOOTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 19), resources.reschkPaperFooter_value, "chkPaperFooter", new Integer(97), new Integer(158), new Short((short) 0), INTEGERS[2], new Short((short) 19), new Integer(68)
                });
        numFooterHeight = insertNumericField("numFooterHeight", NUMFOOTERHEIGHT_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "Spin", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Value", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 20), "numFooterHeight", new Integer(236), new Integer(156), Boolean.TRUE, INTEGERS[2], new Short((short) 20), INTEGERS[5], new Integer(30)
                });
        lblLogoHeight = insertLabel("lblLogoHeight",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblLogoHeight_value, "lblLogoHeight", new Integer(103), new Integer(42), INTEGERS[2], new Short((short) 68), new Integer(32)
                });
        lblLogoWidth = insertLabel("lblLogoWidth",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblLogoWidth_value, "lblLogoWidth", new Integer(103), new Integer(58), INTEGERS[2], new Short((short) 69), new Integer(32)
                });
        FixedLine5 = insertFixedLine("FixedLine5",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[2], "FixedLine5", new Integer(90), new Integer(78), INTEGERS[2], new Short((short) 70), new Integer(215)
                });
        FixedLine6 = insertFixedLine("FixedLine6",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[2], "FixedLine6", new Integer(90), new Integer(150), INTEGERS[2], new Short((short) 71), new Integer(215)
                });
        lblFooterHeight = insertLabel("lblFooterHeight",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblFooterHeight_value, "lblFooterHeight", new Integer(200), new Integer(158), INTEGERS[2], new Short((short) 72), new Integer(32)
                });
        lblLogoX = insertLabel("lblLogoX",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblLogoX_value, "lblLogoX", new Integer(170), new Integer(42), INTEGERS[2], new Short((short) 84), new Integer(94)
                });
        lblLogoY = insertLabel("lblLogoY",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblLogoY_value, "lblLogoY", new Integer(170), new Integer(58), INTEGERS[2], new Short((short) 85), new Integer(94)
                });
        lblAddressHeight = insertLabel("lblAddressHeight",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblAddressHeight_value, "lblAddressHeight", new Integer(103), new Integer(98), INTEGERS[2], new Short((short) 86), new Integer(32)
                });
        lblAddressWidth = insertLabel("lblAddressWidth",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblAddressWidth_value, "lblAddressWidth", new Integer(103), new Integer(114), INTEGERS[2], new Short((short) 87), new Integer(32)
                });
        lblAddressX = insertLabel("lblAddressX",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblAddressX_value, "lblAddressX", new Integer(170), new Integer(98), INTEGERS[2], new Short((short) 88), new Integer(94)
                });
        lblAddressY = insertLabel("lblAddressY",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblAddressY_value, "lblAddressY", new Integer(170), new Integer(114), INTEGERS[2], new Short((short) 89), new Integer(94)
                });
        lblTitle2 = insertLabel("lblTitle2",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle2_value, Boolean.TRUE, "lblTitle2", new Integer(91), INTEGERS[8], INTEGERS[2], new Short((short) 91), new Integer(212)
                });
    }

    public void buildStep3()
    {
        lstLetterNorm = insertListBox("lstLetterNorm", LSTLETTERNORM_ACTION_PERFORMED, LSTLETTERNORM_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 21), "lstLetterNorm", new Integer(210), new Integer(34), INTEGERS[3], new Short((short) 21), new Integer(74)
                });
        chkUseLogo = insertCheckBox("chkUseLogo", CHKUSELOGO_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 22), resources.reschkUseLogo_value, "chkUseLogo", new Integer(97), new Integer(54), new Short((short) 0), INTEGERS[3], new Short((short) 22), new Integer(212)
                });
        chkUseAddressReceiver = insertCheckBox("chkUseAddressReceiver", CHKUSEADDRESSRECEIVER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 23), resources.reschkUseAddressReceiver_value, "chkUseAddressReceiver", new Integer(97), new Integer(69), new Short((short) 0), INTEGERS[3], new Short((short) 23), new Integer(212)
                });
        chkUseSigns = insertCheckBox("chkUseSigns", CHKUSESIGNS_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 24), resources.reschkUseSigns_value, "chkUseSigns", new Integer(97), new Integer(82), new Short((short) 0), INTEGERS[3], new Short((short) 24), new Integer(212)
                });
        chkUseSubject = insertCheckBox("chkUseSubject", CHKUSESUBJECT_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 25), resources.reschkUseSubject_value, "chkUseSubject", new Integer(97), new Integer(98), new Short((short) 0), INTEGERS[3], new Short((short) 25), new Integer(212)
                });
        chkUseSalutation = insertCheckBox("chkUseSalutation", CHKUSESALUTATION_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 26), resources.reschkUseSalutation_value, "chkUseSalutation", new Integer(97), new Integer(113), new Short((short) 0), INTEGERS[3], new Short((short) 26), new Integer(66)
                });
        lstSalutation = insertComboBox("lstSalutation", LSTSALUTATION_ACTION_PERFORMED, LSTSALUTATION_ITEM_CHANGED, LSTSALUTATION_TEXT_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 27), "lstSalutation", new Integer(210), new Integer(110), INTEGERS[3], new Short((short) 27), new Integer(74)
                });
        chkUseBendMarks = insertCheckBox("chkUseBendMarks", CHKUSEBENDMARKS_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 28), resources.reschkUseBendMarks_value, "chkUseBendMarks", new Integer(97), new Integer(127), new Short((short) 0), INTEGERS[3], new Short((short) 28), new Integer(212)
                });
        chkUseGreeting = insertCheckBox("chkUseGreeting", CHKUSEGREETING_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 29), resources.reschkUseGreeting_value, "chkUseGreeting", new Integer(97), new Integer(142), new Short((short) 0), INTEGERS[3], new Short((short) 29), new Integer(66)
                });
        lstGreeting = insertComboBox("lstGreeting", LSTGREETING_ACTION_PERFORMED, LSTGREETING_ITEM_CHANGED, LSTGREETING_TEXT_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, HelpIds.getHelpIdString(HID + 30), "lstGreeting", new Integer(210), new Integer(141), INTEGERS[3], new Short((short) 30), new Integer(74)
                });
        chkUseFooter = insertCheckBox("chkUseFooter", CHKUSEFOOTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 31), resources.reschkUseFooter_value, "chkUseFooter", new Integer(97), new Integer(158), new Short((short) 0), INTEGERS[3], new Short((short) 31), new Integer(212)
                });
        lblLetterNorm = insertLabel("lblLetterNorm",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_16, resources.reslblLetterNorm_value, Boolean.TRUE, "lblLetterNorm", new Integer(97), new Integer(28), INTEGERS[3], new Short((short) 50), new Integer(109)
                });
        lblTitle3 = insertLabel("lblTitle3",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle3_value, Boolean.TRUE, "lblTitle3", new Integer(91), INTEGERS[8], INTEGERS[3], new Short((short) 90), new Integer(212)
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
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 32), resources.resoptSenderPlaceholder_value, "optSenderPlaceholder", new Integer(104), new Integer(42), INTEGERS[4], new Short((short) 32), new Integer(149)
                });
        optSenderDefine = insertRadioButton("optSenderDefine", OPTSENDERDEFINE_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 33), resources.resoptSenderDefine_value, "optSenderDefine", new Integer(104), new Integer(54), INTEGERS[4], new Short((short) 33), new Integer(149)
                });
        txtSenderName = insertTextField("txtSenderName", TXTSENDERNAME_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 34), "txtSenderName", new Integer(182), new Integer(67), INTEGERS[4], new Short((short) 34), new Integer(119)
                });
        txtSenderStreet = insertTextField("txtSenderStreet", TXTSENDERSTREET_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 35), "txtSenderStreet", new Integer(182), new Integer(81), INTEGERS[4], new Short((short) 35), new Integer(119)
                });
        txtSenderPostCode = insertTextField("txtSenderPostCode", TXTSENDERPOSTCODE_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 36), "txtSenderPostCode", new Integer(182), new Integer(95), INTEGERS[4], new Short((short) 36), new Integer(25)
                });
        txtSenderState = insertTextField("txtSenderState", TXTSENDERSTATE_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 37), "txtSenderState", new Integer(211), new Integer(95), INTEGERS[4], new Short((short) 37), new Integer(21)
                });
        txtSenderCity = insertTextField("txtSenderCity", TXTSENDERCITY_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID + 38), "txtSenderCity", new Integer(236), new Integer(95), INTEGERS[4], new Short((short) 38), new Integer(65)
                });
        optReceiverPlaceholder = insertRadioButton("optReceiverPlaceholder", OPTRECEIVERPLACEHOLDER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 39), resources.resoptReceiverPlaceholder_value, "optReceiverPlaceholder", new Integer(104), new Integer(145), INTEGERS[4], new Short((short) 39), new Integer(200)
                });
        optReceiverDatabase = insertRadioButton("optReceiverDatabase", OPTRECEIVERDATABASE_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 40), resources.resoptReceiverDatabase_value, "optReceiverDatabase", new Integer(104), new Integer(157), INTEGERS[4], new Short((short) 40), new Integer(200)
                });
        lblSenderAddress = insertLabel("lblSenderAddress",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblSenderAddress_value, "lblSenderAddress", new Integer(97), new Integer(28), INTEGERS[4], new Short((short) 64), new Integer(136)
                });
        FixedLine2 = insertFixedLine("FixedLine2",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[5], "FixedLine2", new Integer(90), new Integer(126), INTEGERS[4], new Short((short) 75), new Integer(212)
                });
        lblReceiverAddress = insertLabel("lblReceiverAddress",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblReceiverAddress_value, "lblReceiverAddress", new Integer(97), new Integer(134), INTEGERS[4], new Short((short) 76), new Integer(136)
                });
        lblSenderName = insertLabel("lblSenderName",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblSenderName_value, "lblSenderName", new Integer(113), new Integer(69), INTEGERS[4], new Short((short) 77), new Integer(68)
                });
        lblSenderStreet = insertLabel("lblSenderStreet",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblSenderStreet_value, "lblSenderStreet", new Integer(113), new Integer(82), INTEGERS[4], new Short((short) 78), new Integer(68)
                });
        lblPostCodeCity = insertLabel("lblPostCodeCity",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblPostCodeCity_value, "lblPostCodeCity", new Integer(113), new Integer(97), INTEGERS[4], new Short((short) 79), new Integer(68)
                });
        lblTitle4 = insertLabel("lblTitle4",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle4_value, Boolean.TRUE, "lblTitle4", new Integer(91), INTEGERS[8], INTEGERS[4], new Short((short) 92), new Integer(212)
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
                    new Integer(47), HelpIds.getHelpIdString(HID + 41), Boolean.TRUE, "txtFooter", new Integer(97), INTEGER_40, INTEGERS[5], new Short((short) 41), new Integer(203)
                });
        chkFooterNextPages = insertCheckBox("chkFooterNextPages", CHKFOOTERNEXTPAGES_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 42), resources.reschkFooterNextPages_value, "chkFooterNextPages", new Integer(97), new Integer(92), new Short((short) 0), INTEGERS[5], new Short((short) 42), new Integer(202)
                });
        chkFooterPageNumbers = insertCheckBox("chkFooterPageNumbers", CHKFOOTERPAGENUMBERS_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 43), resources.reschkFooterPageNumbers_value, "chkFooterPageNumbers", new Integer(97), new Integer(106), new Short((short) 0), INTEGERS[5], new Short((short) 43), new Integer(201)
                });
        lblFooter = insertLabel("lblFooter",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor5, INTEGERS[8], resources.reslblFooter_value, "lblFooter", new Integer(97), new Integer(28), INTEGERS[5], new Short((short) 52), new Integer(116)
                });
        lblTitle5 = insertLabel("lblTitle5",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle5_value, Boolean.TRUE, "lblTitle5", new Integer(91), INTEGERS[8], INTEGERS[5], new Short((short) 93), new Integer(212)
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
                    INTEGER_12, HelpIds.getHelpIdString(HID + 44), "txtTemplateName", new Integer(202), new Integer(56), INTEGERS[6], new Short((short) 44), resources.restxtTemplateName_value, new Integer(100)
                });
        optCreateLetter = insertRadioButton("optCreateLetter", OPTCREATELETTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 45), resources.resoptCreateLetter_value, "optCreateLetter", new Integer(104), new Integer(111), INTEGERS[6], new Short((short) 50), new Integer(198)
                });
        optMakeChanges = insertRadioButton("optMakeChanges", OPTMAKECHANGES_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], HelpIds.getHelpIdString(HID + 46), resources.resoptMakeChanges_value, "optMakeChanges", new Integer(104), new Integer(123), INTEGERS[6], new Short((short) 51), new Integer(198)
                });
        lblFinalExplanation1 = insertLabel("lblFinalExplanation1",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(26), resources.reslblFinalExplanation1_value, Boolean.TRUE, "lblFinalExplanation1", new Integer(97), new Integer(28), INTEGERS[6], new Short((short) 52), new Integer(205)
                });
        lblProceed = insertLabel("lblProceed",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblProceed_value, "lblProceed", new Integer(97), new Integer(100), INTEGERS[6], new Short((short) 53), new Integer(204)
                });
        lblFinalExplanation2 = insertLabel("lblFinalExplanation2",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(33), resources.reslblFinalExplanation2_value, Boolean.TRUE, "lblFinalExplanation2", new Integer(104), new Integer(145), INTEGERS[6], new Short((short) 54), new Integer(199)
                });
        ImageControl2 = insertImage("ImageControl2",
                new String[]
                {
                    "Border", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short((short) 0), INTEGERS[10], "private:resource/dbu/image/19205", "ImageControl2", new Integer(92), new Integer(145), Boolean.FALSE, INTEGERS[6], new Short((short) 66), INTEGERS[10]
                });
        lblTemplateName = insertLabel("lblTemplateName",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblTemplateName_value, "lblTemplateName", new Integer(97), new Integer(58), INTEGERS[6], new Short((short) 82), new Integer(101)
                });
        lblTitle6 = insertLabel("lblTitle6",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor6, INTEGER_16, resources.reslblTitle6_value, Boolean.TRUE, "lblTitle6", new Integer(91), INTEGERS[8], INTEGERS[6], new Short((short) 94), new Integer(212)
                });

    }
}
