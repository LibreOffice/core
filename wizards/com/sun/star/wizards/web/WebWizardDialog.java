/*************************************************************************
 *
 *  $RCSfile: WebWizardDialog.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:15:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */
package com.sun.star.wizards.web;

import com.sun.star.awt.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.web.data.CGLayout;

/**
 * This class is generated Automatically....
 * bla bla bla
 */
public abstract class WebWizardDialog  extends WizardDialog implements WebWizardConst, UIConsts, WWHID {
      //GUI Components as Class members.
  XFixedText lbIntroTitle;
  XFixedText lblIntroduction;
  //Fixed Line
  XControl lnLoadSettings;
  XFixedText lblLoadSettings;
  XListBox lstLoadSettings;
  XButton btnLoadSession;
  XButton btnDelSession;
  XFixedText lblContentTitle;
  //Fixed Line
  XControl lblSiteContent;
  XFixedText lblDocuments;
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
  XControl lnDisplay;
  XRadioButton optOptimize640x480;
  XRadioButton optOptimize800x600;
  XRadioButton optOptimize1024x768;
  XFixedText lblLayout2Title;
  //Fixed Line
  XControl lblOptimizeFor;
  XFixedText lblStyle;
  XFixedText lblStyleTitle;
  XListBox lstStyles;
  //Image Control
  XControl imgPreview;
  XButton btnBackgrounds;
  XButton btnIconSets;
  //XFixedText lblBodyPreview;
  //XFixedText lblTitlePreview;
  /*
  //Image Control
  XControl imgBackPreview;
  //Image Control
  XControl imgTopPreview;
  //Image Control
  XControl imgUpPreview;
  //Image Control
  XControl imgHomePreview;
  //Image Control
  XControl imgEmailPreview;*/
  XFixedText lblSiteTitle;
  XFixedText lblFavIcon;
  XFixedText lblSiteKeywords;
  XFixedText lblSiteCreated;
  XFixedText lblRevisit;
  XFixedText lblEmail;
  XFixedText lblSiteDesc;
  XTextComponent txtSiteTitle;
  XTextComponent txtSiteDesc;
  XTextComponent txtSiteKeywords;
  XTextComponent txtEmail;
  XDateField dateSiteCreated;
  XDateField dateSiteUpdate;
  XFixedText lblSiteUpdated;
  XFixedText lblRevisitDays;
  XTextComponent txtCopyright;
  XFixedText lblCopyright;
  XFixedText lblTitleGeneralPage;
  XTextComponent txtFavIcon;
  XButton btnFavIcon;
  XNumericField txtRevisitAfter;
  XButton btnPreview;
  XButton btnFTP;
  XCheckBox chkLocalDir;
  //Fixed Line
  XControl lblCreateSite;
  XCheckBox chkFTP;
  XTextComponent txtFTP;
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
  FontDescriptor fontDescriptor7 = new FontDescriptor();

  //private static String[] PROPNAMES_LBL_NOFOCUS =     new String[] {"Height", "Label", "Name", "PositionX", "PositionY", "Step", "Width"};
  private static String[] PROPNAMES_LBL =           new String[] { "Height", "Label", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"};
  private static String[] PROPNAMES_CHKBOX =        new String[] { "Height", "HelpURL", "Label", "Name", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width" };
  private static String[] PROPNAMES_BUTTON =        new String[] { "Height", "HelpURL", "Label", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width" };
  private static String[] PROPNAMES_TXT =           new String[] { "Height", "HelpURL", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width" };
  private static String[] PROPNAMES_TXT_2 =         new String[] { "Height", "HelpURL", "Name", "PositionX", "PositionY", "ReadOnly", "Step", "TabIndex", "Width" };
  private static String[] PROPNAMES_TITLE =         new String[] { "FontDescriptor", "Height", "Label", "MultiLine", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width" };



    //Resources Object
WebWizardDialogResources resources;

  public WebWizardDialog(XMultiServiceFactory xmsf) {
    super(xmsf, HID0_WEBWIZARD);
    //Load Resources
    resources =  new WebWizardDialogResources(xmsf);


    //set dialog properties...
    Helper.setUnoPropertyValues(xDialogModel,
      new String[] { "Closeable","Height", "HelpURL", "Moveable","Name","PositionX","PositionY","Step","TabIndex","Title","Width"},
      new Object[] { Boolean.TRUE,new Integer(210), "HID:" + HID0_WEBWIZARD , Boolean.TRUE,"WebWizardDialog",new Integer(102),new Integer(52),INTEGERS[1],new Short((short)6),resources.resWebWizardDialog_title,new Integer(310)}
    );



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
  public void buildStep1() {
    lbIntroTitle = insertLabel("lbIntroTitle",
      PROPNAMES_TITLE ,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslbIntroTitle_value,Boolean.TRUE,"lbIntroTitle",new Integer(91),INTEGERS[8],INTEGERS[1],new Short((short)1),new Integer(212)}
    );
    lblIntroduction = insertLabel("lblIntroduction",
      new String[] {"Height", "Label", "MultiLine", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { new Integer(119),resources.reslblIntroduction_value,Boolean.TRUE,"lblIntroduction",new Integer(97),new Integer(28),INTEGERS[1],new Short((short)2),new Integer(206)}
    );
    lnLoadSettings = insertFixedLine("lnLoadSettings",
      PROPNAMES_TXT,
      new Object[] { INTEGERS[2],"", "lnLoadSettings",new Integer(91),new Integer(149),INTEGERS[1],new Short((short)3),new Integer(212)}
    );
    lblLoadSettings = insertLabel("lblLoadSettings",
      new String[] {"Height", "Label", "MultiLine", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { INTEGERS[8],resources.reslblLoadSettings_value,Boolean.TRUE,"lblLoadSettings",new Integer(97),new Integer(155),INTEGERS[1],new Short((short)4),new Integer(206)}
    );
    lstLoadSettings = insertListBox("lstLoadSettings", null ,LSTLOADSETTINGS_ITEM_CHANGED,
      new String[] {"Dropdown", "Height", "HelpURL","LineCount", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { Boolean.TRUE,INTEGER_12, "HID:" + HID1_LST_SESSIONS , new Short((short)7),"lstLoadSettings",new Integer(97),new Integer(167),INTEGERS[1],new Short((short)5),new Integer(118)}
    );
    btnLoadSession = insertButton("btnLoadSession", BTNLOADSESSION_ACTION_PERFORMED,
      new String[] {"Enabled","Height", "HelpURL", "ImageAlign", "Label", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { Boolean.FALSE, new Integer(15), "HID:" + HID1_BTN_LOAD_SES, new Short(com.sun.star.awt.ImageAlign.LEFT),resources.resbtnLoadSession_value,"btnLoadSession",new Integer(219),new Integer(165),INTEGERS[1],new Short((short)6),INTEGER_40}
    );
    btnDelSession = insertButton("btnDelSession", BTNDELSESSION_ACTION_PERFORMED,
      new String[] {"Enabled","Height", "HelpURL", "ImageAlign", "Label", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { Boolean.FALSE, INTEGER_14, "HID:" + HID1_BTN_DEL_SES, new Short(com.sun.star.awt.ImageAlign.LEFT),resources.resbtnDelSession_value,"btnDelSession",new Integer(263),new Integer(165),INTEGERS[1],new Short((short)7),INTEGER_40}
    );
  }

  public void buildStep2() {
    lblContentTitle = insertLabel("lblContentTitle",
      PROPNAMES_TITLE,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblContentTitle_value,Boolean.TRUE, "lblContentTitle",new Integer(91),INTEGERS[8],INTEGERS[2],new Short((short)10),new Integer(212)}
    );
    lblSiteContent = insertFixedLine("lblSiteContent",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblSiteContent_value,"lblSiteContent",new Integer(97),new Integer(28),INTEGERS[2],new Short((short)11),new Integer(105)}
    );
    lblDocuments = insertLabel("lblDocuments",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblDocuments_value,"lblDocuments",new Integer(97),new Integer(38),INTEGERS[2],new Short((short)12),new Integer(82)}
    );
    lstDocuments = insertListBox("lstDocuments", null,null,
      new String[] {"Height", "HelpURL", "LineCount", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { new Integer(113),  "HID:" + HID2_LST_DOCS, new Short((short)9),"lstDocuments",new Integer(97),new Integer(48),INTEGERS[2],new Short((short)13),new Integer(83)}
    );
    btnAddDoc = insertButton("btnAddDoc", BTNADDDOC_ACTION_PERFORMED,
      PROPNAMES_BUTTON,
      new Object[] { INTEGER_14, "HID:" + HID2_BTN_ADD_DOC, resources.resbtnAddDoc_value,"btnAddDoc",new Integer(97),new Integer(165),INTEGERS[2],new Short((short)14),INTEGER_40}
    );
    btnRemoveDoc = insertButton("btnRemoveDoc", BTNREMOVEDOC_ACTION_PERFORMED,
      PROPNAMES_BUTTON,
      new Object[] { INTEGER_14, "HID:" + HID2_BTN_REM_DOC, resources.resbtnRemoveDoc_value,"btnRemoveDoc",new Integer(140),new Integer(165),INTEGERS[2],new Short((short)15),INTEGER_40}
    );
    btnDocUp = insertButton("btnDocUp", BTNDOCUP_ACTION_PERFORMED,
    new String[] {"FontDescriptor", "Height", "HelpURL","Label", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
      new Object[] { fontDescriptor7,INTEGER_14, "HID:" + HID2_BTN_DOC_UP,  resources.resbtnDocUp_value,"btnDocUp",new Integer(185),new Integer(87),INTEGERS[2],new Short((short)16),new Integer(18)}
    );
    btnDocDown = insertButton("btnDocDown", BTNDOCDOWN_ACTION_PERFORMED,
      new String[] {"FontDescriptor", "Height", "HelpURL","Label", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
      new Object[] { fontDescriptor7,INTEGER_14, "HID:" + HID2_BTN_DOC_DOWN, resources.resbtnDocDown_value,"btnDocDown",new Integer(185),new Integer(105),INTEGERS[2],new Short((short)17),new Integer(18)}
    );
    lnDocsInfo = insertFixedLine("lnDocsInfo",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslnDocsInfo_value,"lnDocsInfo",new Integer(215),new Integer(28),INTEGERS[2],new Short((short)18),new Integer(90)}
    );
    lblDocTitle = insertLabel("lblDocTitle",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblDocTitle_value,"lblDocTitle",new Integer(215),new Integer(38),INTEGERS[2],new Short((short)19),new Integer(90)}
    );
    txtDocTitle = insertTextField("txtDocTitle", null,
      PROPNAMES_TXT,
      new Object[] { INTEGER_12, "HID:" + HID2_TXT_DOC_TITLE , "txtDocTitle",new Integer(215),new Integer(48),INTEGERS[2],new Short((short)20),new Integer(90)}
    );
    lblDocInfo = insertLabel("lblDocInfo",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblDocInfo_value,"lblDocInfo",new Integer(215),new Integer(63),INTEGERS[2],new Short((short)21),new Integer(90)}
    );
    txtDocInfo = insertTextField("txtDocInfo", null,
      new String[] {"Height", "HelpURL", "MultiLine", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { INTEGER_40, "HID:" + HID2_TXT_DOC_DESC , Boolean.TRUE,"txtDocInfo",new Integer(215),new Integer(73),INTEGERS[2],new Short((short)22),new Integer(90)}
    );
    lblDocAuthor = insertLabel("lblDocAuthor",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblDocAuthor_value,"lblDocAuthor",new Integer(215),new Integer(115),INTEGERS[2],new Short((short)23),new Integer(90)}
    );
    txtDocAuthor = insertTextField("txtDocAuthor", null,
      PROPNAMES_TXT,
      new Object[] { INTEGER_12, "HID:" + HID2_TXT_DOC_AUTHOR, "txtDocAuthor",new Integer(215),new Integer(125),INTEGERS[2],new Short((short)24),new Integer(90)}
    );
    lblDocExportFormat = insertLabel("lblDocExportFormat",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblDocExportFormat_value,"lblDocExportFormat",new Integer(215),new Integer(139),INTEGERS[2],new Short((short)25),new Integer(90)}
    );
    lstDocTargetType = insertListBox("lstDocTargetType", null,null,
      new String[] {"Dropdown", "Height", "HelpURL",  "LineCount", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { Boolean.TRUE, INTEGER_12,  "HID:" + HID2_LST_DOC_EXPORT , new Short((short)7),"lstDocTargetType",new Integer(215),new Integer(149),INTEGERS[2],new Short((short)26),new Integer(90)}
    );
  }

  public void buildStep3() {
    lblLayoutTitle = insertLabel("lblLayoutTitle",
      PROPNAMES_TITLE,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblLayoutTitle_value,Boolean.TRUE,"lblLayoutTitle",new Integer(91),INTEGERS[8],INTEGERS[3],new Short((short)29),new Integer(212)}
    );
    lblLayouts = insertLabel("lblLayouts",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblLayouts_value,"lblLayouts",new Integer(97),new Integer(28),INTEGERS[3],new Short((short)30),new Integer(206)}
    );

  }

  public void buildStep4() {
    lblLayout2Title = insertLabel("lblLayout2Title",
      PROPNAMES_TITLE,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblLayout2Title_value,Boolean.TRUE,"lblLayout2Title",new Integer(91),INTEGERS[8],INTEGERS[4],new Short((short)33),new Integer(212)}
    );
    lnDisplay = insertFixedLine("lblDisplay",
      new String[] {"Height", "Label", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { INTEGER_16,resources.reslblDisplay_value,"lblDisplay",new Integer(97),new Integer(26),INTEGERS[4],new Short((short)34),new Integer(206)}
    );
    chkDocFilename = insertCheckBox("chkDocFilename", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID4_CHK_DISPLAY_FILENAME , resources.reschkDocFilename_value,"chkDocFilename",new Integer(97),new Integer(45),new Short((short)0),INTEGERS[4],new Short((short)35),new Integer(89)}
    );
    chbDocDesc = insertCheckBox("chbDocDesc", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID4_CHK_DISPLAY_DESCRIPTION, resources.reschbDocDesc_value,"chbDocDesc",new Integer(97),new Integer(55),new Short((short)0),INTEGERS[4],new Short((short)36),new Integer(89)}
    );
    chbDocAuthor = insertCheckBox("chbDocAuthor", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID4_CHK_DISPLAY_AUTHOR, resources.reschbDocAuthor_value,"chbDocAuthor",new Integer(97),new Integer(65),new Short((short)0),INTEGERS[4],new Short((short)37),new Integer(89)}
    );
    chkDocCreated = insertCheckBox("chkDocCreated", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID4_CHK_DISPLAY_CR_DATE,resources.reschkDocCreated_value,"chkDocCreated",new Integer(97),new Integer(75),new Short((short)0),INTEGERS[4],new Short((short)38),new Integer(89)}
    );
    chkDocChanged = insertCheckBox("chkDocChanged", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID4_CHK_DISPLAY_UP_DATE, resources.reschkDocChanged_value,"chkDocChanged",new Integer(97),new Integer(85),new Short((short)0),INTEGERS[4],new Short((short)39),new Integer(89)}
    );

    chkDocFormat = insertCheckBox("chkDocFormat", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8],"HID:" + HID4_CHK_DISPLAY_FORMAT, resources.reschkDocFormat_value,"chkDocFormat",new Integer(192),new Integer(45),new Short((short)0),INTEGERS[4],new Short((short)40),new Integer(110)}
    );
    chkDocFormatIcon = insertCheckBox("chkDocFormatIcon", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID4_CHK_DISPLAY_F_ICON, resources.reschkDocFormatIcon_value,"chkDocFormatIcon",new Integer(192),new Integer(55),new Short((short)0),INTEGERS[4],new Short((short)41),new Integer(110)}
    );
    chkDocPages = insertCheckBox("chkDocPages", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID4_CHK_DISPLAY_PAGES , resources.reschkDocPages_value,"chkDocPages",new Integer(192),new Integer(65),new Short((short)0),INTEGERS[4],new Short((short)42),new Integer(110)}
    );
    chkDocSize = insertCheckBox("chkDocSize", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID4_CHK_DISPLAY_SIZE, resources.reschkDocSize_value,"chkDocSize",new Integer(192),new Integer(75),new Short((short)0),INTEGERS[4],new Short((short)43),new Integer(110)}
    );
    lblOptimizeFor = insertFixedLine("lblOptimizeFor",
      PROPNAMES_LBL,
      new Object[] { new Integer(17),resources.reslblOptimizeFor_value,"lblOptimizeFor",new Integer(97),new Integer(113),INTEGERS[4],new Short((short)44), new Integer(203)}
    );
    optOptimize640x480 = insertRadioButton("optOptimize640x480", null,
      PROPNAMES_BUTTON,
      new Object[] { INTEGERS[10], "HID:" + HID4_GRP_OPTIMAIZE_640, resources.resoptOptimize640x480_value,"optOptimize640x480",new Integer(103),new Integer(133),INTEGERS[4],new Short((short)45),new Integer(44)}
    );
    optOptimize800x600 = insertRadioButton("optOptimize800x600", null,
      PROPNAMES_BUTTON,
      new Object[] { INTEGERS[10], "HID:" + HID4_GRP_OPTIMAIZE_800, resources.resoptOptimize800x600_value,"optOptimize800x600",new Integer(103),new Integer(146),INTEGERS[4],new Short((short)46),new Integer(44)}
    );
    optOptimize1024x768 = insertRadioButton("optOptimize1024x768", null,
      PROPNAMES_BUTTON,
      new Object[] { INTEGERS[10], "HID:" + HID4_GRP_OPTIMAIZE_1024, resources.resoptOptimize1024x768_value,"optOptimize1024x768",new Integer(103),new Integer(158),INTEGERS[4],new Short((short)47),new Integer(44)}
    );

  }

  public void buildStep5() {
    lblStyleTitle = insertLabel("lblStyleTitle",
      PROPNAMES_TITLE,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblStyleTitle_value,Boolean.TRUE,"lblStyleTitle",new Integer(91),INTEGERS[8],INTEGERS[5],new Short((short)50),new Integer(212)}
    );
    lblStyle = insertLabel("lblStyle",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblStyle_value,"lblStyle",new Integer(97),new Integer(32),INTEGERS[5],new Short((short)51),new Integer(45)}
    );
    lstStyles = insertListBox("lstStyles", null,LSTSTYLES_ITEM_CHANGED,
      new String[] {"Dropdown", "Height", "HelpURL", "LineCount", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { Boolean.TRUE,INTEGER_12, "HID:" + HID5_LST_STYLES, new Short((short)7),"lstStyles",new Integer(146),new Integer(30),INTEGERS[5],new Short((short)52),new Integer(64)}
    );
    btnBackgrounds = insertButton("btnBackgrounds", BTNBACKGROUNDS_ACTION_PERFORMED,
      PROPNAMES_BUTTON,
      new Object[] { INTEGER_14, "HID:" + HID5_BTN_BACKGND, resources.resbtnBackgrounds_value,"btnBackgrounds",new Integer(214),new Integer(29),INTEGERS[5],new Short((short)53),INTEGER_40}
    );
    btnIconSets = insertButton("btnIconSets", BTNICONSETS_ACTION_PERFORMED,
      PROPNAMES_BUTTON,
      new Object[] { INTEGER_14, "HID:" + HID5_BTN_ICONS, resources.resbtnIconSets_value,"btnIconSets",new Integer(258),new Integer(29),INTEGERS[5],new Short((short)54),INTEGER_40 }
    );

    imgPreview = insertImage("imgPreview",
      new String[] {"BackgroundColor", "Enabled","Height", "ImageURL", "Name", "PositionX", "PositionY", "ScaleImage", "Step", "TabIndex", "Width"},
      new Object[] { new Integer(0), Boolean.FALSE, new Integer(118),"" ,"imgPreview",new Integer(97),new Integer(60),Boolean.FALSE,INTEGERS[5],new Short((short)55),new Integer(201)}
    );

  }

  public void buildStep6() {
    lblTitleGeneralPage = insertLabel("lblTitleGeneralPage",
      PROPNAMES_TITLE,
      new Object[] { fontDescriptor4,INTEGERS[8],resources.reslblTitleGeneralPage_value,Boolean.TRUE,"lblTitleGeneralPage",new Integer(90),INTEGERS[9],INTEGERS[6],new Short((short)60),new Integer(212)}
    );
    lblSiteTitle = insertLabel("lblSiteTitle",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblSiteTitle_value,"lblSiteTitle",new Integer(97),new Integer(33),INTEGERS[6],new Short((short)61),new Integer(70)}
    );
    txtSiteTitle = insertTextField("txtSiteTitle", null,
      PROPNAMES_TXT,
      new Object[] { INTEGER_12, "HID:" + HID6_TXT_SITE_TITLE, "txtSiteTitle",new Integer(169),new Integer(31),INTEGERS[6],new Short((short)62),new Integer(125)}
    );
    lblFavIcon = insertLabel("lblFavIcon",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblFavIcon_value,"lblFavIcon",new Integer(97),new Integer(49),INTEGERS[6],new Short((short)63),new Integer(70)}
    );
    txtFavIcon = insertTextField("txtFavIcon", null,
      PROPNAMES_TXT_2,
      new Object[] { INTEGER_12, "HID:" + HID6_TXT_SITE_ICON , "txtFavIcon",new Integer(169),new Integer(47),Boolean.TRUE,INTEGERS[6],new Short((short)64),new Integer(105)}
    );
    btnFavIcon = insertButton("btnFavIcon", BTNFAVICON_ACTION_PERFORMED,
      PROPNAMES_BUTTON,
      new Object[] { INTEGER_14, "HID:" + HID6_BTN_SITE_ICON, resources.resbtnFavIcon_value,"btnFavIcon",new Integer(277),new Integer(46),INTEGERS[6],new Short((short)65),INTEGER_16}
    );
    lblSiteDesc = insertLabel("lblSiteDesc",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblSiteDesc_value,"lblSiteDesc",new Integer(97),new Integer(65),INTEGERS[6],new Short((short)66),new Integer(70)}
    );
    txtSiteDesc = insertTextField("txtSiteDesc", null,
      PROPNAMES_TXT,
      new Object[] { INTEGER_12, "HID:" + HID6_TXT_SITE_DESC ,  "txtSiteDesc",new Integer(169),new Integer(63),INTEGERS[6],new Short((short)67),new Integer(125)}
    );
    lblSiteKeywords = insertLabel("lblSiteKeywords",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblSiteKeywords_value,"lblSiteKeywords",new Integer(97),new Integer(83),INTEGERS[6],new Short((short)68),new Integer(70)}
    );
    txtSiteKeywords = insertTextField("txtSiteKeywords", null,
      PROPNAMES_TXT,
      new Object[] { INTEGER_12, "HID:" + HID6_TXT_SITE_KEYWRDS , "txtSiteKeywords",new Integer(169),new Integer(81),INTEGERS[6],new Short((short)69),new Integer(125)}
    );

    lblSiteCreated = insertLabel("lblSiteCreated",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblSiteCreated_value,"lblSiteCreated",new Integer(97),new Integer(101),INTEGERS[6],new Short((short)70),new Integer(70)}
    );
    dateSiteCreated = insertDateField("dateSiteCreated", null,
      new String[] {"Dropdown", "Height", "HelpURL", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { Boolean.TRUE,INTEGER_12, "HID:" + HID6_DATE_SITE_CREATED , "dateSiteCreated",new Integer(169),new Integer(99),INTEGERS[6],new Short((short)71),new Integer(38)}
    );
    lblSiteUpdated = insertLabel("lblSiteUpdated",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblSiteUpdated_value,"lblSiteUpdated",new Integer(211),new Integer(101),INTEGERS[6],new Short((short)72),new Integer(41)}
    );

    dateSiteUpdate = insertDateField("dateSiteUpdate", null,
      new String[] {"Dropdown", "Height", "HelpURL", "Name", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
      new Object[] { Boolean.TRUE,INTEGER_12, "HID:" + HID6_DATE_SITE_UPDATED , "dateSiteUpdate",new Integer(255),new Integer(99),INTEGERS[6],new Short((short)73),new Integer(38)}
    );

    lblRevisit = insertLabel("lblRevisit",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblRevisit_value,"lblRevisit",new Integer(97),new Integer(119),INTEGERS[6],new Short((short)74),new Integer(70)}
    );
    txtRevisitAfter = insertNumericField("txtRevisitAfter", null,
      new String[] {"DecimalAccuracy", "Height", "HelpURL", "Name", "PositionX", "PositionY", "Spin", "Step", "StrictFormat", "TabIndex", "ValueMax", "ValueMin", "Width"},
      new Object[] { new Short((short)0),INTEGER_12, "HID:" + HID6_NUM_SITE_REVISTS , "txtRevisitAfter",new Integer(169),new Integer(117),Boolean.TRUE,INTEGERS[6],Boolean.TRUE,new Short((short)75),new Integer(31),INTEGERS[0],new Integer(38)}
    );
    lblRevisitDays = insertLabel("lblRevisitDays",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblRevisitDays_value,"lblRevisitDays",new Integer(211),new Integer(119),INTEGERS[6],new Short((short)76),new Integer(56)}
    );

    lblEmail = insertLabel("lblEmail",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblEmail_value,"lblEmail",new Integer(97),new Integer(135),INTEGERS[6],new Short((short)77),new Integer(70)}
    );
    txtEmail = insertTextField("txtEmail", null,
      PROPNAMES_TXT,
      new Object[] { INTEGER_12, "HID:" + HID6_TXT_SITE_EMAIL , "txtEmail",new Integer(169),new Integer(133),INTEGERS[6],new Short((short)78),new Integer(125)}
    );
    lblCopyright = insertLabel("lblCopyright",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8],resources.reslblCopyright_value,"lblCopyright",new Integer(97),new Integer(153),INTEGERS[6],new Short((short)79),new Integer(70)}
    );
    txtCopyright = insertTextField("txtCopyright", null,
      PROPNAMES_TXT,
      new Object[] { INTEGER_12, "HID:" + HID6_TXT_SITE_COPYRIGHT , "txtCopyright",new Integer(169),new Integer(151),INTEGERS[6],new Short((short)80),new Integer(125)}
    );

  }

  public void buildStep7() {
    lblTitlePublish = insertLabel("lblTitlePublish",
      PROPNAMES_TITLE,
      new Object[] { fontDescriptor4,INTEGER_16,resources.reslblTitlePublish_value,Boolean.TRUE, "lblTitlePublish",new Integer(91),INTEGERS[8],INTEGERS[7],new Short((short)90), new Integer(212)}
    );

    FixedLine1 = insertFixedLine("FixedLine1",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[9],resources.resFixedLine1_value,"FixedLine1",new Integer(97),new Integer(26),INTEGERS[7],new Short((short)91),new Integer(208)}
    );

    btnPreview = insertButton("btnPreview", BTNPREVIEW_ACTION_PERFORMED,
      PROPNAMES_BUTTON,
      new Object[] { INTEGER_14, "HID:" + HID7_BTN_PREVIEW ,resources.resbtnPreview_value,"btnPreview",new Integer(103),new Integer(38),INTEGERS[7],new Short((short)92),INTEGER_40}
    );

    lblCreateSite = insertFixedLine("lblCreateSite",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[9],resources.reslblCreateSite_value,"lblCreateSite",new Integer(97),new Integer(56),INTEGERS[7],new Short((short)93),new Integer(208)}
    );

    chkLocalDir = insertCheckBox("chkLocalDir", CHKLOCALDIR_ITEM_CHANGED,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID7_CHK_PUBLISH_LOCAL, resources.reschkLocalDir_value,"chkLocalDir",new Integer(103),new Integer(68),new Short((short)0),INTEGERS[7],new Short((short)94),new Integer(195)}
    );
    txtLocalDir = insertTextField("txtLocalDir", null,
      PROPNAMES_TXT_2,
      new Object[] { INTEGER_12, "HID:" + HID7_TXT_LOCAL , "txtLocalDir",new Integer(113),new Integer(78),Boolean.TRUE,INTEGERS[7],new Short((short)95),new Integer(142)}
    );
    btnLocalDir = insertButton("btnLocalDir", BTNLOCALDIR_ACTION_PERFORMED,
      PROPNAMES_BUTTON,
      new Object[] { INTEGER_14, "HID:" + HID7_BTN_LOCAL , resources.resbtnLocalDir_value,"btnLocalDir",new Integer(260),new Integer(78),INTEGERS[7],new Short((short)96),INTEGER_16}
    );

    chkZip = insertCheckBox("chkZip", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[9], "HID:" + HID7_CHK_PUBLISH_ZIP, resources.reschkZip_value,"chkZip",new Integer(103),new Integer(96),new Short((short)0),INTEGERS[7],new Short((short)97),new Integer(151)}
    );
    txtZip = insertTextField("txtZip", null,
      PROPNAMES_TXT_2,
      new Object[] { INTEGER_12, "HID:" + HID7_TXT_ZIP, "txtZip",new Integer(113),new Integer(108),Boolean.TRUE,INTEGERS[7],new Short((short)98),new Integer(142)}
    );
    btnZip = insertButton("btnZip", BTNZIP_ACTION_PERFORMED,
      PROPNAMES_BUTTON,
      new Object[] { INTEGER_14, "HID:" + HID7_BTN_ZIP, resources.resbtnZip_value,"btnZip",new Integer(260),new Integer(106),INTEGERS[7],new Short((short)99),INTEGER_16}
    );

    chkFTP = insertCheckBox("chkFTP", null,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[8], "HID:" + HID7_CHK_PUBLISH_FTP, resources.reschkFTP_value,"chkFTP",new Integer(103),new Integer(124),new Short((short)0),INTEGERS[7],new Short((short)100),new Integer(192)}
    );
    txtFTP = insertTextField("txtFTP", null,
      PROPNAMES_TXT_2,
      new Object[] { INTEGER_12, "HID:" + HID7_TXT_FTP, "txtFTP",new Integer(113),new Integer(134),Boolean.TRUE,INTEGERS[7],new Short((short)101),new Integer(142)}
    );
    btnFTP = insertButton("btnFTP", BTNFTP_ACTION_PERFORMED,
      PROPNAMES_BUTTON,
      new Object[] { INTEGER_14, "HID:" + HID7_BTN_FTP, resources.resbtnFTP_value,"btnFTP",new Integer(260),new Integer(134),INTEGERS[7],new Short((short)102),INTEGER_40}
    );


    lnSaveSetting = insertFixedLine("lnSaveSetting",
      PROPNAMES_TXT,
      new Object[] { INTEGERS[2],"", "lnSaveSetting",new Integer(97),new Integer(151),INTEGERS[7],new Short((short)103),new Integer(208)}
    );
    chkSaveSettings = insertCheckBox("chkSaveSettings", CHKSAVESETTINGS_ITEM_CHANGED,
      PROPNAMES_CHKBOX,
      new Object[] { INTEGERS[9], "HID:" + HID7_CHK_SAVE , resources.reschkSaveSettings_value,"chkSaveSettings",new Integer(97),new Integer(157),new Short((short)1),INTEGERS[7],new Short((short)104),new Integer(192)}
    );
    lblSaveSettings = insertLabel("lblSaveSettings",
      PROPNAMES_LBL,
      new Object[] { INTEGERS[8], resources.reslblSaveSettings_value,"lblSaveSettings",new Integer(107),new Integer(169),INTEGERS[7],new Short((short)105),new Integer(45)}
    );
    cbSaveSettings = insertComboBox("txtSaveSettings", TXTSAVESETTINGS_TEXT_CHANGED,TXTSAVESETTINGS_TEXT_CHANGED, TXTSAVESETTINGS_TEXT_CHANGED,
      new String[] {"Autocomplete", "Dropdown", "Height","HelpURL","LineCount","Name","PositionX","PositionY","Step","TabIndex","Width"},
      new Object[] { Boolean.TRUE, Boolean.TRUE, INTEGER_12, "HID:" + HID7_TXT_SAVE , new Short((short)7), "txtSaveSettings",new Integer(157),new Integer(167),INTEGERS[7],new Short((short)106),new Integer(140)}
    );

  }

  XControl[] imgIconsPrev = new XControl[8];
  ImageList ilLayouts;

  /**
   * builds the layouts image list.
   */
  public void buildStepX() {
      ilLayouts = new ImageList();
      ilLayouts.setPos(new Size(97,38));
      ilLayouts.setImageSize(new Size(34,30));
      ilLayouts.setCols(5);
      ilLayouts.setRows(3);
      ilLayouts.setStep(new Short((short)3));
      ilLayouts.setShowButtons(false);
      ilLayouts.setRenderer(new LayoutRenderer());
      ilLayouts.scaleImages = Boolean.FALSE;
      ilLayouts.tabIndex = 31;
      ilLayouts.helpURL = HID3_IL_LAYOUTS_IMG1;

      /*for (int i = 0; i<8; i++)
        imgIconsPrev[i] = insertImage("imgIconPrev" + i,
        new String[] { "BackgroundColor","Border","Height","ImageURL", "PositionX", "PositionY", "ScaleImage", "Step", "Tabstop", "Width"},
        new Object[] { new Integer(-1), new Short((short)0),new Integer(14),"file:///c:/bludisk.gif", new Integer(97 + i * 20 + 7 ),new Integer(147),Boolean.FALSE,INTEGERS[5],Boolean.FALSE,new Integer(14)});
        */
  }

  /**
   * renders the images and titles of
   * the layouts
   * @author rpiterman
   *
   */
  private class LayoutRenderer implements ImageList.ImageRenderer {

    public String getImageUrl(Object listItem) {
        String s = listItem==null ? "" : ((CGLayout)listItem).getImageURL();
        return s;
    }

    public String render(Object listItem) {
        return listItem==null ? "" : ((CGLayout)listItem).cp_Name;
    }

  }

}