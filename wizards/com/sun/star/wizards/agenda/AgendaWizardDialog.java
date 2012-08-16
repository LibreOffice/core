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
package com.sun.star.wizards.agenda; 
    
import com.sun.star.awt.FontDescriptor;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XDateField;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XTimeField;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.WizardDialog;
    
/**
 * This class is generated Automatically....
 * bla bla bla
 */      
public abstract class AgendaWizardDialog  extends WizardDialog implements AgendaWizardDialogConst, UIConsts {
      //GUI Components as Class members.
  XFixedText lblTitle1;
  XCheckBox chkMinutes;
  XFixedText lblHelp1;
  XFixedText lblPageDesign;
  XListBox listPageDesign; 
  //Image Control
  XControl imgHelp1;
  XControl imgHelp2;
  XControl imgHelp3;
  XControl imgHelp4;
  XControl imgHelp6;
  XFixedText lblTitle2;
  XFixedText lblTime;
  XFixedText lblTitle;
  XFixedText lblLocation;
  XTextComponent cbLocation;
  XTimeField txtTime;
  XTextComponent txtTitle;
  XFixedText lblHelp2;
  XFixedText lblDate;
  XDateField txtDate;
  XFixedText lblTitle3;
  XCheckBox chkMeetingTitle;
  XCheckBox chkRead;
  XCheckBox chkBring;
  XCheckBox chkNotes;
  XFixedText lblHelp3;
  XFixedText lblTitle5;
  XCheckBox chkConvenedBy;
  XCheckBox chkPresiding;
  XCheckBox chkNoteTaker;
  XCheckBox chkTimekeeper;
  XCheckBox chkAttendees;
  XCheckBox chkObservers;
  XCheckBox chkResourcePersons;
  XFixedText lblHelp4;
  XFixedText lblTitle4;
  XFixedText lblTopic;
  XFixedText lblResponsible;
  XFixedText lblDuration;
  //XFixedText lblHelp5;
  XTextComponent txtTemplateName;
  XRadioButton optCreateAgenda; 
  XRadioButton optMakeChanges; 
  XFixedText lblProceed;
  XFixedText lblTemplateName;
  XFixedText lblTemplatePath;
  XFixedText lblTitle6;
  XTextComponent txtTemplatePath;
  XButton btnTemplatePath;
  XFixedText lblHelp6;
  XFixedText lblHelpPg6;
  XButton btnInsert;
  XButton btnRemove;
  XButton btnUp;
  XButton btnDown;
  
  final private String IMGHELP1_HID = PropertyNames.EMPTY_STRING;
  
  
  //Font Descriptors as Class members.
  FontDescriptor fontDescriptor1 = new FontDescriptor();
  FontDescriptor fontDescriptor2 = new FontDescriptor();
  FontDescriptor fontDescriptor4 = new FontDescriptor();

  //Resources Object
  AgendaWizardDialogResources resources;
  
  private String[] PROPS_LIST = new String[] {"Dropdown", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH};
  private String[] PROPS_LABEL_B = new String[] {PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH};
  private String[] PROPS_CHECK = new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH};
  private String[] PROPS_BUTTON = new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH};
  private String[] PROPS_X = new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH};
  private String[] PROPS_TEXTAREA = new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH};
  private String[] PROPS_TEXT = new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH};
  private String[] PROPS_IMAGE = new String[] {PropertyNames.PROPERTY_BORDER, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH};
  
  private static final Short NO_BORDER = new Short((short)0);
  
  public AgendaWizardDialog(XMultiServiceFactory xmsf) {
    super(xmsf,HID);
    //Load Resources
    resources =  new AgendaWizardDialogResources(xmsf);
   
    //set dialog properties...
    Helper.setUnoPropertyValues(xDialogModel, 
      new String[] { PropertyNames.PROPERTY_CLOSEABLE,PropertyNames.PROPERTY_HEIGHT,PropertyNames.PROPERTY_MOVEABLE,PropertyNames.PROPERTY_POSITION_X,PropertyNames.PROPERTY_POSITION_Y,PropertyNames.PROPERTY_STEP,PropertyNames.PROPERTY_TABINDEX,PropertyNames.PROPERTY_TITLE,PropertyNames.PROPERTY_WIDTH},
      new Object[] { Boolean.TRUE,210,Boolean.TRUE,200,52,INTEGERS[1],new Short((short)1),resources.resAgendaWizardDialog_title,310}
    );
    
    //Set member- FontDescriptors...
    fontDescriptor1.Weight = 150;
    fontDescriptor1.Underline = com.sun.star.awt.FontUnderline.SINGLE;
    fontDescriptor2.Weight = 100;
    fontDescriptor4.Weight = 150;
  }
  
  //build components
  public void buildStep1() {
    lblTitle1 = insertLabel("lblTitle1", 
      PROPS_LABEL_B,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblTitle1_value,Boolean.TRUE,91,INTEGERS[8],INTEGERS[1],new Short((short)100),212}
    );
    lblPageDesign = insertLabel("lblPageDesign", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblPageDesign_value,97,32,INTEGERS[1],new Short((short)101),66}
    );
    listPageDesign = insertListBox("listPageDesign", null, null,
      PROPS_LIST,
      new Object[] { Boolean.TRUE,INTEGER_12,LISTPAGEDESIGN_HID,166,30,INTEGERS[1],new Short((short)102),70}
    );
    chkMinutes = insertCheckBox("chkMinutes", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[9],CHKMINUTES_HID,resources.reschkMinutes_value,97,50,new Short((short)0),INTEGERS[1],new Short((short)103),203}
    );
    imgHelp1 = insertImage("imgHelp1", 
      PROPS_IMAGE,
      new Object[] { NO_BORDER, INTEGERS[10],IMGHELP1_HID, AgendaWizardDialogConst.INFO_IMAGE_URL, 92,145, Boolean.FALSE, INTEGERS[1], new Short((short)104),INTEGERS[10]}
    );
    lblHelp1 = insertLabel("lblHelp1", 
            PROPS_TEXTAREA,
            new Object[] { 39,resources.reslblHelp1_value,Boolean.TRUE,104,145,INTEGERS[1],new Short((short)105),199}
          );

  }

  public void buildStep2() {
    lblTitle2 = insertLabel("lblTitle2", 
      PROPS_LABEL_B,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblTitle2_value,Boolean.TRUE,91,INTEGERS[8],INTEGERS[2],new Short((short)200),212}
    );
    lblDate = insertLabel("lblDate", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblDate_value,97,32,INTEGERS[2],new Short((short)201),66}
    );
    txtDate = insertDateField("txtDate", null,
      PROPS_LIST,
      new Object[] { Boolean.TRUE,INTEGER_12,TXTDATE_HID,166,30,INTEGERS[2],new Short((short)202),70}
    );
    lblTime = insertLabel("lblTime", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblTime_value,97,50,INTEGERS[2],new Short((short)203),66}
    );
    txtTime = insertTimeField("txtTime", null,
      new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, "StrictFormat", PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH},
     new Object[] { INTEGER_12,TXTTIME_HID,166,48,INTEGERS[2],Boolean.TRUE,new Short((short)204),70}
    );
    
    lblTitle = insertLabel("lblTitle", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblTitle_value,97,68,INTEGERS[2],new Short((short)205),66}
    );
    txtTitle = insertTextField("txtTitle", null,
      new String[] {PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH},
      new Object[] { 26,TXTTITLE_HID,Boolean.TRUE,166,66,INTEGERS[2],new Short((short)206),138}
    );
    lblLocation = insertLabel("lblLocation", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblLocation_value,97,100,INTEGERS[2],new Short((short)207),66}
    );
    cbLocation = insertTextField("cbLocation", null,null,
      new String[] { PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH},
      new Object[] { 34,CBLOCATION_HID,Boolean.TRUE,166,98,INTEGERS[2],new Short((short)208),138}
    );
    
    imgHelp2 = insertImage("imgHelp2", 
      PROPS_IMAGE,
      new Object[] { NO_BORDER, INTEGERS[10],IMGHELP1_HID,AgendaWizardDialogConst.INFO_IMAGE_URL, 92,145,Boolean.FALSE, INTEGERS[2],new Short((short)209),INTEGERS[10]}
    );
    lblHelp2 = insertLabel("lblHelp2", 
      PROPS_TEXTAREA,
      new Object[] { 39,resources.reslblHelp2_value,Boolean.TRUE,104,145,INTEGERS[2],new Short((short)210),199}
    );
    
  }

  public void buildStep3() {
    lblTitle3 = insertLabel("lblTitle3", 
      PROPS_LABEL_B,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblTitle3_value,Boolean.TRUE,91,INTEGERS[8],INTEGERS[3],new Short((short)300),212}
    );
    chkMeetingTitle = insertCheckBox("chkMeetingTitle", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKMEETINGTITLE_HID,resources.reschkMeetingTitle_value,97,32,new Short((short)1),INTEGERS[3],new Short((short)301),69}
    );
    chkRead = insertCheckBox("chkRead", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKREAD_HID,resources.reschkRead_value,97,46,new Short((short)0),INTEGERS[3],new Short((short)302),162}
    );
    chkBring = insertCheckBox("chkBring", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKBRING_HID,resources.reschkBring_value,97,60,new Short((short)0),INTEGERS[3],new Short((short)303),162}
    );
    chkNotes = insertCheckBox("chkNotes", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKNOTES_HID,resources.reschkNotes_value,97,74,new Short((short)1),INTEGERS[3],new Short((short)304),160}
    );
    imgHelp3 = insertImage("imgHelp3", 
            PROPS_IMAGE,
            new Object[] { NO_BORDER, INTEGERS[10],IMGHELP1_HID,AgendaWizardDialogConst.INFO_IMAGE_URL, 92,145,Boolean.FALSE, INTEGERS[3],new Short((short)305),INTEGERS[10]}
          );
    lblHelp3 = insertLabel("lblHelp3", 
      PROPS_TEXTAREA,
      new Object[] { 39,resources.reslblHelp3_value,Boolean.TRUE,104,145,INTEGERS[3],new Short((short)306),199}
    );
  }

  public void buildStep4() {
    lblTitle5 = insertLabel("lblTitle5", 
      PROPS_LABEL_B,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblTitle5_value,Boolean.TRUE,91,INTEGERS[8],INTEGERS[4],new Short((short)400),212}
    );
    chkConvenedBy = insertCheckBox("chkConvenedBy", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKCONVENEDBY_HID,resources.reschkConvenedBy_value,97,32,new Short((short)1),INTEGERS[4],new Short((short)401),150}
    );
    chkPresiding = insertCheckBox("chkPresiding", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKPRESIDING_HID,resources.reschkPresiding_value,97,46,new Short((short)0),INTEGERS[4],new Short((short)402),150}
    );
    chkNoteTaker = insertCheckBox("chkNoteTaker", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKNOTETAKER_HID,resources.reschkNoteTaker_value,97,60,new Short((short)0),INTEGERS[4],new Short((short)403),150}
    );
    chkTimekeeper = insertCheckBox("chkTimekeeper", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKTIMEKEEPER_HID,resources.reschkTimekeeper_value,97,74,new Short((short)0),INTEGERS[4],new Short((short)404),150}
    );
    chkAttendees = insertCheckBox("chkAttendees", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKATTENDEES_HID,resources.reschkAttendees_value,97,88,new Short((short)1),INTEGERS[4],new Short((short)405),150}
    );
    chkObservers = insertCheckBox("chkObservers", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKOBSERVERS_HID,resources.reschkObservers_value,97,102,new Short((short)0),INTEGERS[4],new Short((short)406),150}
    );
    chkResourcePersons = insertCheckBox("chkResourcePersons", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],CHKRESOURCEPERSONS_HID,resources.reschkResourcePersons_value,97,116,new Short((short)0),INTEGERS[4],new Short((short)407),150}
    );
    imgHelp4 = insertImage("imgHelp4", 
            PROPS_IMAGE,
            new Object[] { NO_BORDER, INTEGERS[10],IMGHELP1_HID,AgendaWizardDialogConst.INFO_IMAGE_URL, 92,145,Boolean.FALSE, INTEGERS[4],new Short((short)408),INTEGERS[10]}
          );
    lblHelp4 = insertLabel("lblHelp4", 
      PROPS_TEXTAREA,
      new Object[] { 39,resources.reslblHelp4_value,Boolean.TRUE,104,145,INTEGERS[4],new Short((short)409),199}
    );
  }

  public void buildStep5() {
    lblTitle4 = insertLabel("lblTitle4", 
      PROPS_LABEL_B,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblTitle4_value,Boolean.TRUE,91,INTEGERS[8],INTEGERS[5],new Short((short)500),212}
    );
    lblTopic = insertLabel("lblTopic", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblTopic_value,107,28,INTEGERS[5],new Short((short)71),501}
    );
    lblResponsible = insertLabel("lblResponsible", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblResponsible_value,195,28,INTEGERS[5],new Short((short)72),502}
    );
    lblDuration = insertLabel("lblDuration", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblDuration_value,267,28,INTEGERS[5],new Short((short)73),503}
    );
  
    
    
    
    
    btnInsert = insertButton("btnInsert", BTNINSERT_ACTION_PERFORMED,
            PROPS_BUTTON,
            new Object[] { INTEGER_14,BTNINSERT_HID,resources.resButtonInsert,92,136,INTEGERS[5],new Short((short)580),INTEGER_40}
          );
    btnRemove = insertButton("btnRemove", BTNREMOVE_ACTION_PERFORMED,
            PROPS_BUTTON,
            new Object[] { INTEGER_14,BTNREMOVE_HID,resources.resButtonRemove,134,136,INTEGERS[5],new Short((short)581),INTEGER_40}
          );
    btnUp = insertButton("btnUp", BTNUP_ACTION_PERFORMED,
            PROPS_BUTTON,
            new Object[] { INTEGER_14,BTNUP_HID,resources.resButtonUp,202,136,INTEGERS[5],new Short((short)582),INTEGER_50}
          );
    btnDown = insertButton("btnDown", BTNDOWN_ACTION_PERFORMED,
            PROPS_BUTTON,
            new Object[] { INTEGER_14,BTNDOWN_HID,resources.resButtonDown,254,136,INTEGERS[5],new Short((short)583),INTEGER_50}
          );
          
    
  }

  public void buildStep6() {

    lblTitle6 = insertLabel("lblTitle6", 
      PROPS_LABEL_B,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblTitle6_value,Boolean.TRUE,91,INTEGERS[8],INTEGERS[6],new Short((short)600),212}
    );
    lblHelpPg6 = insertLabel("lblHelpPg6", 
            PROPS_TEXTAREA,
            new Object[] { 24,resources.reslblHelpPg6_value,Boolean.TRUE,97,32,INTEGERS[6],new Short((short)601),204}
          );
    
    lblTemplateName = insertLabel("lblTemplateName", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblTemplateName_value,97,62,INTEGERS[6],new Short((short)602),101}
    );
    txtTemplateName = insertTextField("txtTemplateName", TXTTEMPLATENAME_TEXT_CHANGED,
      PROPS_X,
      new Object[] { INTEGER_12,TXTTEMPLATENAME_HID,202,60,INTEGERS[6],new Short((short)603),100}
    );
    lblProceed = insertLabel("lblProceed", 
      PROPS_TEXT,
      new Object[] { INTEGERS[8],resources.reslblProceed_value,97,101,INTEGERS[6],new Short((short)607),204}
    );
    optCreateAgenda = insertRadioButton("optCreateAgenda", null,
      PROPS_CHECK,
      new Object[] { INTEGERS[8],OPTCREATEAGENDA_HID,resources.resoptCreateAgenda_value,103,113,new Short((short)1),INTEGERS[6],new Short((short)608),198}
    );
    optMakeChanges = insertRadioButton("optMakeChanges", null,
      PROPS_BUTTON,
      new Object[] { INTEGERS[8],OPTMAKECHANGES_HID,resources.resoptMakeChanges_value,103,125,INTEGERS[6],new Short((short)609),198}
    );
    imgHelp6 = insertImage("imgHelp6", 
            PROPS_IMAGE,
            new Object[] { NO_BORDER, INTEGERS[10],IMGHELP1_HID,AgendaWizardDialogConst.INFO_IMAGE_URL, 92,145,Boolean.FALSE, INTEGERS[6],new Short((short)610),INTEGERS[10]}
          );
    lblHelp6 = insertLabel("lblHelp6", 
            PROPS_TEXTAREA,
            new Object[] { 39,resources.reslblHelp6_value,Boolean.TRUE,104,145,INTEGERS[6],new Short((short)611),199}
          );
          
  }

}
