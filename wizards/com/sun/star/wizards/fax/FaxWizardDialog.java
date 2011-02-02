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
package com.sun.star.wizards.fax;

import com.sun.star.awt.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.*;

public abstract class FaxWizardDialog extends WizardDialog implements FaxWizardDialogConst, UIConsts
{

    XRadioButton optBusinessFax;
    XListBox lstBusinessStyle;
    XRadioButton optPrivateFax;
    XListBox lstPrivateStyle;
    XFixedText lblBusinessStyle;
    XFixedText lblTitle1;
    XFixedText lblPrivateStyle;
    XFixedText lblIntroduction;
    //Image Control
    XControl ImageControl3;
    XCheckBox chkUseLogo;
    XCheckBox chkUseDate;
    XCheckBox chkUseCommunicationType;
    XComboBox lstCommunicationType;
    XCheckBox chkUseSubject;
    XCheckBox chkUseSalutation;
    XComboBox lstSalutation;
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
    XTextComponent txtSenderFax;
    XRadioButton optReceiverDatabase;
    XRadioButton optReceiverPlaceholder;
    XFixedText lblSenderAddress;
    //Fixed Line
    XControl FixedLine2;
    XFixedText lblSenderName;
    XFixedText lblSenderStreet;
    XFixedText lblPostCodeCity;
    XFixedText lblTitle4;
    XFixedText Label1;
    XFixedText Label2;
    XTextComponent txtFooter;
    XCheckBox chkFooterNextPages;
    XCheckBox chkFooterPageNumbers;
    XFixedText lblFooter;
    XFixedText lblTitle5;
    XTextComponent txtTemplateName;
    //File Control
    XControl fileTemplatePath;
    XRadioButton optCreateFax;
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
    FontDescriptor fontDescriptor4 = new FontDescriptor();
    FontDescriptor fontDescriptor5 = new FontDescriptor();    //Resources Object
    FaxWizardDialogResources resources;

    public FaxWizardDialog(XMultiServiceFactory xmsf)
    {
        super(xmsf, HIDMAIN);
        //Load Resources
        resources = new FaxWizardDialogResources(xmsf);


        //set dialog properties...
        Helper.setUnoPropertyValues(xDialogModel,
                new String[]
                {
                    "Closeable", PropertyNames.PROPERTY_HEIGHT, "Moveable", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Title", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, new Integer(210), Boolean.TRUE, new Integer(104), new Integer(52), INTEGERS[1], new Short((short) 1), resources.resFaxWizardDialog_title, new Integer(310)
                });



        //Set member- FontDescriptors...
        fontDescriptor1.Weight = 150;
        fontDescriptor1.Underline = com.sun.star.awt.FontUnderline.SINGLE;
        fontDescriptor2.Weight = 100;
        fontDescriptor4.Weight = 100;
        fontDescriptor5.Weight = 150;
    }
    //build components
    public void buildStep1()
    {
        optBusinessFax = insertRadioButton("optBusinessFax", OPTBUSINESSFAX_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], OPTBUSINESSFAX_HID, resources.resoptBusinessFax_value, new Integer(97), new Integer(28), INTEGERS[1], new Short((short) 1), new Integer(184)
                });
        lstBusinessStyle = insertListBox("lstBusinessStyle", LSTBUSINESSSTYLE_ACTION_PERFORMED, LSTBUSINESSSTYLE_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, LSTBUSINESSSTYLE_HID, new Integer(180), INTEGER_40, INTEGERS[1], new Short((short) 3), new Integer(74)
                });
        optPrivateFax = insertRadioButton("optPrivateFax", OPTPRIVATEFAX_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], OPTPRIVATEFAX_HID, resources.resoptPrivateFax_value, new Integer(97), new Integer(81), INTEGERS[1], new Short((short) 2), new Integer(184)
                });
        lstPrivateStyle = insertListBox("lstPrivateStyle", LSTPRIVATESTYLE_ACTION_PERFORMED, LSTPRIVATESTYLE_ITEM_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, LSTPRIVATESTYLE_HID, new Integer(180), new Integer(95), INTEGERS[1], new Short((short) 4), new Integer(74)
                });
        lblBusinessStyle = insertLabel("lblBusinessStyle",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblBusinessStyle_value, new Integer(110), new Integer(42), INTEGERS[1], new Short((short) 32), new Integer(60)
                });
        lblTitle1 = insertLabel("lblTitle1",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor5, INTEGER_16, resources.reslblTitle1_value, Boolean.TRUE, new Integer(91), INTEGERS[8], INTEGERS[1], new Short((short) 37), new Integer(212)
                });
        lblPrivateStyle = insertLabel("lblPrivateStyle",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblPrivateStyle_value, new Integer(110), new Integer(95), INTEGERS[1], new Short((short) 50), new Integer(60)
                });
        lblIntroduction = insertLabel("lblIntroduction",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(39), resources.reslblIntroduction_value, Boolean.TRUE, new Integer(104), new Integer(145), INTEGERS[1], new Short((short) 55), new Integer(199)
                });
        ImageControl3 = insertInfoImage(92, 145, 1);
//  ImageControl3 = insertImage("ImageControl3",
//      new String[] {"Border", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH},
//      new Object[] { new Short((short)0),INTEGERS[10],"private:resource/dbu/image/19205",new Integer(92),new Integer(145),Boolean.FALSE,INTEGERS[1],new Short((short)56),INTEGERS[10]}
//    );
    }

    public void buildStep2()
    {
        chkUseLogo = insertCheckBox("chkUseLogo", CHKUSELOGO_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], CHKUSELOGO_HID, resources.reschkUseLogo_value, new Integer(97), new Integer(28), new Short((short) 0), INTEGERS[2], new Short((short) 5), new Integer(212)
                });
        chkUseDate = insertCheckBox("chkUseDate", CHKUSEDATE_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], CHKUSEDATE_HID, resources.reschkUseDate_value, new Integer(97), new Integer(43), new Short((short) 0), INTEGERS[2], new Short((short) 6), new Integer(212)
                });
        chkUseCommunicationType = insertCheckBox("chkUseCommunicationType", CHKUSECOMMUNICATIONTYPE_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], CHKUSECOMMUNICATIONTYPE_HID, resources.reschkUseCommunicationType_value, new Integer(97), new Integer(57), new Short((short) 0), INTEGERS[2], new Short((short) 7), new Integer(100)
                });
        lstCommunicationType = insertComboBox("lstCommunicationType", LSTCOMMUNICATIONTYPE_ACTION_PERFORMED, LSTCOMMUNICATIONTYPE_ITEM_CHANGED, LSTCOMMUNICATIONTYPE_TEXT_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, LSTCOMMUNICATIONTYPE_HID, new Integer(105), new Integer(68), INTEGERS[2], new Short((short) 8), new Integer(174)
                });
        chkUseSubject = insertCheckBox("chkUseSubject", CHKUSESUBJECT_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], CHKUSESUBJECT_HID, resources.reschkUseSubject_value, new Integer(97), new Integer(87), new Short((short) 0), INTEGERS[2], new Short((short) 9), new Integer(212)
                });
        chkUseSalutation = insertCheckBox("chkUseSalutation", CHKUSESALUTATION_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], CHKUSESALUTATION_HID, resources.reschkUseSalutation_value, new Integer(97), new Integer(102), new Short((short) 0), INTEGERS[2], new Short((short) 10), new Integer(100)
                });
        lstSalutation = insertComboBox("lstSalutation", LSTSALUTATION_ACTION_PERFORMED, LSTSALUTATION_ITEM_CHANGED, LSTSALUTATION_TEXT_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, LSTSALUTATION_HID, new Integer(105), new Integer(113), INTEGERS[2], new Short((short) 11), new Integer(174)
                });
        chkUseGreeting = insertCheckBox("chkUseGreeting", CHKUSEGREETING_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], CHKUSEGREETING_HID, resources.reschkUseGreeting_value, new Integer(97), new Integer(132), new Short((short) 0), INTEGERS[2], new Short((short) 12), new Integer(100)
                });
        lstGreeting = insertComboBox("lstGreeting", LSTGREETING_ACTION_PERFORMED, LSTGREETING_ITEM_CHANGED, LSTGREETING_TEXT_CHANGED,
                new String[]
                {
                    "Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, INTEGER_12, LSTGREETING_HID, new Integer(105), new Integer(143), INTEGERS[2], new Short((short) 13), new Integer(174)
                });
        chkUseFooter = insertCheckBox("chkUseFooter", CHKUSEFOOTER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], CHKUSEFOOTER_HID, resources.reschkUseFooter_value, new Integer(97), new Integer(163), new Short((short) 0), INTEGERS[2], new Short((short) 14), new Integer(212)
                });
        lblTitle3 = insertLabel("lblTitle3",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor5, INTEGER_16, resources.reslblTitle3_value, Boolean.TRUE, new Integer(91), INTEGERS[8], INTEGERS[2], new Short((short) 59), new Integer(212)
                });
    }

    public void buildStep3()
    {
        optSenderPlaceholder = insertRadioButton("optSenderPlaceholder", OPTSENDERPLACEHOLDER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], OPTSENDERPLACEHOLDER_HID, resources.resoptSenderPlaceholder_value, new Integer(104), new Integer(42), INTEGERS[3], new Short((short) 15), new Integer(149)
                });
        optSenderDefine = insertRadioButton("optSenderDefine", OPTSENDERDEFINE_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], OPTSENDERDEFINE_HID, resources.resoptSenderDefine_value, new Integer(104), new Integer(54), INTEGERS[3], new Short((short) 16), new Integer(149)
                });
        txtSenderName = insertTextField("txtSenderName", TXTSENDERNAME_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, TXTSENDERNAME_HID, new Integer(182), new Integer(67), INTEGERS[3], new Short((short) 17), new Integer(119)
                });
        txtSenderStreet = insertTextField("txtSenderStreet", TXTSENDERSTREET_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, TXTSENDERSTREET_HID, new Integer(182), new Integer(81), INTEGERS[3], new Short((short) 18), new Integer(119)
                });
        txtSenderPostCode = insertTextField("txtSenderPostCode", TXTSENDERPOSTCODE_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, TXTSENDERPOSTCODE_HID, new Integer(182), new Integer(95), INTEGERS[3], new Short((short) 19), new Integer(25)
                });
        txtSenderState = insertTextField("txtSenderState", TXTSENDERSTATE_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, TXTSENDERSTATE_HID, new Integer(211), new Integer(95), INTEGERS[3], new Short((short) 20), new Integer(21)
                });
        txtSenderCity = insertTextField("txtSenderCity", TXTSENDERCITY_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, TXTSENDERCITY_HID, new Integer(236), new Integer(95), INTEGERS[3], new Short((short) 21), new Integer(65)
                });
        txtSenderFax = insertTextField("txtSenderFax", TXTSENDERFAX_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, TXTSENDERFAX_HID, new Integer(182), new Integer(109), INTEGERS[3], new Short((short) 22), new Integer(119)
                });
        optReceiverPlaceholder = insertRadioButton("optReceiverPlaceholder", OPTRECEIVERPLACEHOLDER_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], OPTRECEIVERPLACEHOLDER_HID, resources.resoptReceiverPlaceholder_value, new Integer(104), new Integer(148), INTEGERS[3], new Short((short) 23), new Integer(200)
                });
        optReceiverDatabase = insertRadioButton("optReceiverDatabase", OPTRECEIVERDATABASE_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], OPTRECEIVERDATABASE_HID, resources.resoptReceiverDatabase_value, new Integer(104), new Integer(160), INTEGERS[3], new Short((short) 24), new Integer(200)
                });
        lblSenderAddress = insertLabel("lblSenderAddress",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblSenderAddress_value, new Integer(97), new Integer(28), INTEGERS[3], new Short((short) 46), new Integer(136)
                });
        FixedLine2 = insertFixedLine("FixedLine2",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[5], new Integer(90), new Integer(126), INTEGERS[3], new Short((short) 51), new Integer(212)
                });
        lblSenderName = insertLabel("lblSenderName",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblSenderName_value, new Integer(113), new Integer(69), INTEGERS[3], new Short((short) 52), new Integer(68)
                });
        lblSenderStreet = insertLabel("lblSenderStreet",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblSenderStreet_value, new Integer(113), new Integer(82), INTEGERS[3], new Short((short) 53), new Integer(68)
                });
        lblPostCodeCity = insertLabel("lblPostCodeCity",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblPostCodeCity_value, new Integer(113), new Integer(97), INTEGERS[3], new Short((short) 54), new Integer(68)
                });
        lblTitle4 = insertLabel("lblTitle4",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor5, INTEGER_16, resources.reslblTitle4_value, Boolean.TRUE, new Integer(91), INTEGERS[8], INTEGERS[3], new Short((short) 60), new Integer(212)
                });
        Label1 = insertLabel("lblSenderFax",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.resLabel1_value, new Integer(113), new Integer(111), INTEGERS[3], new Short((short) 68), new Integer(68)
                });
        Label2 = insertLabel("Label2",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.resLabel2_value, new Integer(97), new Integer(137), INTEGERS[3], new Short((short) 69), new Integer(136)
                });
    }

    public void buildStep4()
    {
        txtFooter = insertTextField("txtFooter", TXTFOOTER_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(47), TXTFOOTER_HID, Boolean.TRUE, new Integer(97), INTEGER_40, INTEGERS[4], new Short((short) 25), new Integer(203)
                });
        chkFooterNextPages = insertCheckBox("chkFooterNextPages", CHKFOOTERNEXTPAGES_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], CHKFOOTERNEXTPAGES_HID, resources.reschkFooterNextPages_value, new Integer(97), new Integer(92), new Short((short) 0), INTEGERS[4], new Short((short) 26), new Integer(202)
                });
        chkFooterPageNumbers = insertCheckBox("chkFooterPageNumbers", CHKFOOTERPAGENUMBERS_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], CHKFOOTERPAGENUMBERS_HID, resources.reschkFooterPageNumbers_value, new Integer(97), new Integer(106), new Short((short) 0), INTEGERS[4], new Short((short) 27), new Integer(201)
                });
        lblFooter = insertLabel("lblFooter",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor4, INTEGERS[8], resources.reslblFooter_value, new Integer(97), new Integer(28), INTEGERS[4], new Short((short) 33), new Integer(116)
                });
        lblTitle5 = insertLabel("lblTitle5",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor5, INTEGER_16, resources.reslblTitle5_value, Boolean.TRUE, new Integer(91), INTEGERS[8], INTEGERS[4], new Short((short) 61), new Integer(212)
                });
    }

    public void buildStep5()
    {
        txtTemplateName = insertTextField("txtTemplateName", TXTTEMPLATENAME_TEXT_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Text", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, TXTTEMPLATENAME_HID, new Integer(202), new Integer(56), INTEGERS[5], new Short((short) 28), resources.restxtTemplateName_value, new Integer(100)
                });
        /*
        fileTemplatePath = insertFileControl("fileTemplatePath", FILETEMPLATEPATH_TEXT_CHANGED,
        new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH},
        new Object[] { INTEGER_12,FILETEMPLATEPATH_HID,new Integer(172),new Integer(74),INTEGERS[5],new Short((short)29),new Integer(130)}
        );
         */
        optCreateFax = insertRadioButton("optCreateFax", OPTCREATEFAX_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], OPTCREATEFAX_HID, resources.resoptCreateFax_value, new Integer(104), new Integer(111), INTEGERS[5], new Short((short) 30), new Integer(198)
                });
        optMakeChanges = insertRadioButton("optMakeChanges", OPTMAKECHANGES_ITEM_CHANGED,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], OPTMAKECHANGES_HID, resources.resoptMakeChanges_value, new Integer(104), new Integer(123), INTEGERS[5], new Short((short) 31), new Integer(198)
                });
        lblFinalExplanation1 = insertLabel("lblFinalExplanation1",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(28), resources.reslblFinalExplanation1_value, Boolean.TRUE, new Integer(97), new Integer(28), INTEGERS[5], new Short((short) 34), new Integer(205)
                });
        lblProceed = insertLabel("lblProceed",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblProceed_value, new Integer(97), new Integer(100), INTEGERS[5], new Short((short) 35), new Integer(204)
                });
        lblFinalExplanation2 = insertLabel("lblFinalExplanation2",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Integer(33), resources.reslblFinalExplanation2_value, Boolean.TRUE, new Integer(104), new Integer(145), INTEGERS[5], new Short((short) 36), new Integer(199)
                });
        ImageControl2 = insertImage("ImageControl2",
                new String[]
                {
                    "Border", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short((short) 0), INTEGERS[10], "private:resource/dbu/image/19205", new Integer(92), new Integer(145), Boolean.FALSE, INTEGERS[5], new Short((short) 47), INTEGERS[10]
                });
        lblTemplateName = insertLabel("lblTemplateName",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGERS[8], resources.reslblTemplateName_value, new Integer(97), new Integer(58), INTEGERS[5], new Short((short) 57), new Integer(101)
                });
        /*
        lblTemplatePath = insertLabel("lblTemplatePath",
        new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH},
        new Object[] { INTEGERS[8],resources.reslblTemplatePath_value,new Integer(97),new Integer(77),INTEGERS[5],new Short((short)58),new Integer(71)}
        );
         */
        lblTitle6 = insertLabel("lblTitle6",
                new String[]
                {
                    "FontDescriptor", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    fontDescriptor5, INTEGER_16, resources.reslblTitle6_value, Boolean.TRUE, new Integer(91), INTEGERS[8], INTEGERS[5], new Short((short) 62), new Integer(212)
                });

    }
}