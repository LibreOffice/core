#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
from ..ui.WizardDialog import WizardDialog, uno, PropertyNames
from .WebWizardDialogResources import WebWizardDialogResources
from .WebWizardConst import *
from .WWHID import *
from ..ui.ControlScroller import HelpIds
from ..ui.ImageList import ImageList
from com.sun.star.awt import Size
from com.sun.star.awt.FontUnderline import SINGLE
from com.sun.star.awt.FontFamily import ROMAN
from com.sun.star.awt.FontSlant import ITALIC
from com.sun.star.awt.ImageAlign import LEFT

class WebWizardDialog(WizardDialog):
    PROPNAMES_LBL = (PropertyNames.PROPERTY_HEIGHT,
        PropertyNames.PROPERTY_LABEL,
        PropertyNames.PROPERTY_NAME,
        PropertyNames.PROPERTY_POSITION_X,
        PropertyNames.PROPERTY_POSITION_Y,
        PropertyNames.PROPERTY_STEP,
        PropertyNames.PROPERTY_TABINDEX,
        PropertyNames.PROPERTY_WIDTH)
    PROPNAMES_CHKBOX = (PropertyNames.PROPERTY_HEIGHT,
        PropertyNames.PROPERTY_HELPURL,
        PropertyNames.PROPERTY_LABEL,
        PropertyNames.PROPERTY_NAME,
        PropertyNames.PROPERTY_POSITION_X,
        PropertyNames.PROPERTY_POSITION_Y,
        PropertyNames.PROPERTY_STATE,
        PropertyNames.PROPERTY_STEP,
        PropertyNames.PROPERTY_TABINDEX,
        PropertyNames.PROPERTY_WIDTH)
    PROPNAMES_BUTTON = (PropertyNames.PROPERTY_HEIGHT,
        PropertyNames.PROPERTY_HELPURL,
        PropertyNames.PROPERTY_LABEL,
        PropertyNames.PROPERTY_NAME,
        PropertyNames.PROPERTY_POSITION_X,
        PropertyNames.PROPERTY_POSITION_Y,
        PropertyNames.PROPERTY_STEP,
        PropertyNames.PROPERTY_TABINDEX,
        PropertyNames.PROPERTY_WIDTH)
    PROPNAMES_TXT = (PropertyNames.PROPERTY_HEIGHT,
        PropertyNames.PROPERTY_HELPURL,
        PropertyNames.PROPERTY_NAME,
        PropertyNames.PROPERTY_POSITION_X,
        PropertyNames.PROPERTY_POSITION_Y,
        PropertyNames.PROPERTY_STEP,
        PropertyNames.PROPERTY_TABINDEX,
        PropertyNames.PROPERTY_WIDTH)
    PROPNAMES_TXT_2 = (PropertyNames.PROPERTY_HEIGHT,
        PropertyNames.PROPERTY_HELPURL,
        PropertyNames.PROPERTY_NAME,
        PropertyNames.PROPERTY_POSITION_X,
        PropertyNames.PROPERTY_POSITION_Y, "ReadOnly",
        PropertyNames.PROPERTY_STEP,
        PropertyNames.PROPERTY_TABINDEX,
        PropertyNames.PROPERTY_WIDTH)
    PROPNAMES_TITLE = ("FontDescriptor",
        PropertyNames.PROPERTY_HEIGHT,
        PropertyNames.PROPERTY_LABEL,
        PropertyNames.PROPERTY_MULTILINE,
        PropertyNames.PROPERTY_NAME,
        PropertyNames.PROPERTY_POSITION_X,
        PropertyNames.PROPERTY_POSITION_Y,
        PropertyNames.PROPERTY_STEP,
        PropertyNames.PROPERTY_TABINDEX,
        PropertyNames.PROPERTY_WIDTH)

    def __init__(self, xmsf):
        super(WebWizardDialog, self).__init__(xmsf, HID0_WEBWIZARD)
        
        #Load Resources
        self.resources = WebWizardDialogResources(xmsf, self.oWizardResource)
        
        #set dialog properties...
        self.setDialogProperties( True, 210, True, 102, 52, 1, 6, 
            self.resources.resWebWizardDialog_title, 330)
            
        self.fontDescriptor4 = \
            uno.createUnoStruct('com.sun.star.awt.FontDescriptor')
        self.fontDescriptor5 = \
            uno.createUnoStruct('com.sun.star.awt.FontDescriptor')
        self.fontDescriptor6 = \
            uno.createUnoStruct('com.sun.star.awt.FontDescriptor')
        self.fontDescriptor7 = \
            uno.createUnoStruct('com.sun.star.awt.FontDescriptor')
            
        self.fontDescriptor4.Weight = 150
        self.fontDescriptor5.Name = "Angsana New"
        self.fontDescriptor5.Height = 16
        self.fontDescriptor5.StyleName = "Regular"
        self.fontDescriptor5.Family = ROMAN
        self.fontDescriptor5.Weight = 100
        self.fontDescriptor6.Name = "Andalus"
        self.fontDescriptor6.Height = 22
        self.fontDescriptor6.StyleName = "Bold Italic"
        self.fontDescriptor6.Weight = 150
        self.fontDescriptor6.Slant = ITALIC
        self.fontDescriptor7.Name = "OpenSymbol"
        self.fontDescriptor7.Height = 9
        self.fontDescriptor7.StyleName = "Regular"
        self.fontDescriptor7.Weight = 100

    def buildStep1(self):
        tabIndex = 100
        self.insertLabel("lbIntroTitle", WebWizardDialog.PROPNAMES_TITLE,
            (self.fontDescriptor4, 16, self.resources.reslbIntroTitle_value,
                True, "lbIntroTitle", 91, 8, 1, tabIndex + 1, 232))
        self.insertLabel("lblIntroduction",
            (PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_MULTILINE,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (119, self.resources.reslblIntroduction_value, True,
                "lblIntroduction", 97, 28, 1, tabIndex + 1, 226))
        self.insertFixedLine("lnLoadSettings", WebWizardDialog.PROPNAMES_TXT,
            (2, "", "lnLoadSettings", 91, 147, 1, tabIndex + 1, 234))
        self.lblLoadSettings = self.insertLabel("lblLoadSettings",
            (PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_MULTILINE,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (8, self.resources.reslblLoadSettings_value, True,
                "lblLoadSettings", 97, 153, 1, tabIndex + 1, 226))
        self.lstLoadSettings = self.insertListBox(
            "lstLoadSettings", None, LSTLOADSETTINGS_ITEM_CHANGED,("Dropdown",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL, "LineCount",
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (True, 12, HelpIds.getHelpIdString(HID1_LST_SESSIONS), 14,
                "lstLoadSettings", 97, 165, 1, tabIndex + 1, 173), self)
        self.btnDelSession = self.insertButton(
            "btnDelSession", BTNDELSESSION_ACTION_PERFORMED,
            (PropertyNames.PROPERTY_ENABLED,
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_HELPURL, "ImageAlign",
                    PropertyNames.PROPERTY_LABEL,
                    PropertyNames.PROPERTY_NAME,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX,
                    PropertyNames.PROPERTY_WIDTH),
            (False, 14, HelpIds.getHelpIdString(HID1_BTN_DEL_SES), LEFT,
                self.resources.resbtnDelSession_value, "btnDelSession",
                274, 164, 1, tabIndex + 1, 50), self)

    def buildStep2(self):
        tabIndex = 200
        self.lblContentTitle = self.insertLabel(
            "lblContentTitle", WebWizardDialog.PROPNAMES_TITLE,
            (self.fontDescriptor4, 16, self.resources.reslblContentTitle_value,
                True, "lblContentTitle", 91, 8, 2, tabIndex + 1, 232))
        self.lblSiteContent = self.insertLabel(
            "lblSiteContent", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblSiteContent_value,
                "lblSiteContent", 97, 28, 2, tabIndex + 1, 105))
        self.lstDocuments = self.insertListBox(
            "lstDocuments", None, None,
            (PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL,
                "LineCount",
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (123, HelpIds.getHelpIdString(HID2_LST_DOCS),
                9, "lstDocuments", 97, 38, 2, tabIndex + 1, 103), self)
        self.btnAddDoc = self.insertButton(
            "btnAddDoc", BTNADDDOC_ACTION_PERFORMED,
            WebWizardDialog.PROPNAMES_BUTTON,
            (14, HelpIds.getHelpIdString(HID2_BTN_ADD_DOC),
            self.resources.resbtnAddDoc_value, "btnAddDoc", 97, 165, 2,
                tabIndex + 1, 50), self)
        self.btnRemoveDoc = self.insertButton(
            "btnRemoveDoc", BTNREMOVEDOC_ACTION_PERFORMED,
            WebWizardDialog.PROPNAMES_BUTTON,
            (14, HelpIds.getHelpIdString(HID2_BTN_REM_DOC),
                self.resources.resbtnRemoveDoc_value, "btnRemoveDoc", 150, 165,
                2, tabIndex + 1, 50), self)
        self.btnDocUp = self.insertButton(
            "btnDocUp", BTNDOCUP_ACTION_PERFORMED,
            ("FontDescriptor",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (self.fontDescriptor7, 14, HelpIds.getHelpIdString(HID2_BTN_DOC_UP),
                self.resources.resbtnDocUp_value, "btnDocUp",
                205, 87, 2, tabIndex + 1, 18), self)
        self.btnDocDown = self.insertButton(
            "btnDocDown", BTNDOCDOWN_ACTION_PERFORMED,(
                "FontDescriptor",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (self.fontDescriptor7, 14, HelpIds.getHelpIdString(HID2_BTN_DOC_DOWN),
                self.resources.resbtnDocDown_value, "btnDocDown", 205,
                105, 2, tabIndex + 1, 18), self)
        self.lblDocExportFormat = self.insertLabel(
            "lblDocExportFormat", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblDocExportFormat_value,
                "lblDocExportFormat", 235, 28, 2, tabIndex + 1, 89))
        self.lstDocTargetType = self.insertListBox("lstDocTargetType", None, None,
            ("Dropdown",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL, "LineCount",
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (True, 12, HelpIds.getHelpIdString(HID2_LST_DOC_EXPORT), 14,
                "lstDocTargetType", 235, 38, 2, tabIndex + 1, 89), self)
        self.lnDocsInfo = self.insertFixedLine(
            "lnDocsInfo", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslnDocsInfo_value, "lnDocsInfo",
                235, 66, 2, tabIndex + 1, 90))
        self.lblDocTitle = self.insertLabel(
            "lblDocTitle", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblDocTitle_value, "lblDocTitle",
                235, 78, 2, tabIndex + 1, 89))
        self.txtDocTitle = self.insertTextField(
            "txtDocTitle", None, WebWizardDialog.PROPNAMES_TXT,
            (12, HelpIds.getHelpIdString(HID2_TXT_DOC_TITLE), "txtDocTitle",
                235, 88, 2, tabIndex + 1, 89), self)
        self.lblDocInfo = self.insertLabel(
            "lblDocInfo", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblDocInfo_value, "lblDocInfo",
                235, 103, 2, tabIndex + 1, 89))
        self.txtDocInfo = self.insertTextField(
            "txtDocInfo", None,(PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL,
                PropertyNames.PROPERTY_MULTILINE,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (40, HelpIds.getHelpIdString(HID2_TXT_DOC_DESC), True,
                "txtDocInfo", 235, 113, 2, tabIndex + 1, 89), self)
        self.lblDocAuthor = self.insertLabel(
            "lblDocAuthor", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblDocAuthor_value, "lblDocAuthor",
                235, 155, 2, tabIndex + 1, 89))
        self.txtDocAuthor = self.insertTextField(
            "txtDocAuthor", None, WebWizardDialog.PROPNAMES_TXT,
            (12, HelpIds.getHelpIdString(HID2_TXT_DOC_AUTHOR),
                "txtDocAuthor", 235, 165, 2, tabIndex + 1, 89), self)

    def buildStep3(self):
        self.insertLabel("lblLayoutTitle", WebWizardDialog.PROPNAMES_TITLE,
            (self.fontDescriptor4, 16, self.resources.reslblLayoutTitle_value,
                True, "lblLayoutTitle", 91, 8, 3, 29, 232))
        self.lblLayouts = self.insertLabel("lblLayouts",
            WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblLayouts_value, "lblLayouts",
                97, 28, 3, 30, 206))

    def buildStep4(self):
        self.insertLabel("lblLayout2Title", WebWizardDialog.PROPNAMES_TITLE,
            (self.fontDescriptor4, 16, self.resources.reslblLayout2Title_value,
                True, "lblLayout2Title", 91, 8, 4, 33, 232))
        self.lnDisplay = self.insertLabel("lblDisplay",
            (PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_MULTILINE,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (16, self.resources.reslblDisplay_value, True,
                "lblDisplay", 97, 28, 4, 34, 226))
        self.chkDocFilename = self.insertCheckBox("chkDocFilename", None,
            WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID4_CHK_DISPLAY_FILENAME),
                self.resources.reschkDocFilename_value, "chkDocFilename",
                103, 50, 0, 4, 35, 99), self)
        self.chbDocDesc = self.insertCheckBox(
            "chbDocDesc", None, WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID4_CHK_DISPLAY_DESCRIPTION),
                self.resources.reschbDocDesc_value, "chbDocDesc",
                103, 60, 0, 4, 36, 99), self)
        self.chbDocAuthor = self.insertCheckBox(
            "chbDocAuthor", None, WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID4_CHK_DISPLAY_AUTHOR),
                self.resources.reschbDocAuthor_value, "chbDocAuthor",
                103, 70, 0, 4, 37, 99), self)
        self.chkDocCreated = self.insertCheckBox(
            "chkDocCreated", None, WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID4_CHK_DISPLAY_CR_DATE),
                self.resources.reschkDocCreated_value, "chkDocCreated",
                103, 80, 0, 4, 38, 99), self)
        self.chkDocChanged = self.insertCheckBox(
            "chkDocChanged", None, WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID4_CHK_DISPLAY_UP_DATE),
                self.resources.reschkDocChanged_value, "chkDocChanged",
                103, 90, 0, 4, 39, 99), self)
        self.chkDocFormat = self.insertCheckBox(
            "chkDocFormat", None, WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID4_CHK_DISPLAY_FORMAT),
                self.resources.reschkDocFormat_value, "chkDocFormat",
                200, 50, 0, 4, 40, 110), self)
        self.chkDocFormatIcon = self.insertCheckBox(
            "chkDocFormatIcon", None, WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID4_CHK_DISPLAY_F_ICON),
                self.resources.reschkDocFormatIcon_value, "chkDocFormatIcon",
                200, 60, 0, 4, 41, 110), self)
        self.chkDocPages = self.insertCheckBox(
            "chkDocPages", None, WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID4_CHK_DISPLAY_PAGES),
                self.resources.reschkDocPages_value, "chkDocPages",
                200,70, 0, 4, 42, 110), self)
        self.chkDocSize = self.insertCheckBox(
            "chkDocSize", None, WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID4_CHK_DISPLAY_SIZE),
                self.resources.reschkDocSize_value, "chkDocSize",
                200, 80, 0, 4, 43, 110), self)
        self.insertLabel("lblOptimizeFor", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblOptimizeFor_value, "lblOptimizeFor",
                97, 113, 4, 44, 226))
        self.optOptimize640x480 = self.insertRadioButton(
            "optOptimize640x480", None, WebWizardDialog.PROPNAMES_BUTTON,
            (10, HelpIds.getHelpIdString(HID4_GRP_OPTIMAIZE_640),
                self.resources.resoptOptimize640x480_value,
                    "optOptimize640x480", 103, 133, 4, 45, 44), self)
        self.optOptimize800x600 = self.insertRadioButton(
            "optOptimize800x600", None, WebWizardDialog.PROPNAMES_BUTTON,
            (10, HelpIds.getHelpIdString(HID4_GRP_OPTIMAIZE_800),
                self.resources.resoptOptimize800x600_value,
                "optOptimize800x600", 103, 146, 4, 46, 44), self)
        self.optOptimize1024x768 = self.insertRadioButton(
            "optOptimize1024x768", None, WebWizardDialog.PROPNAMES_BUTTON,
            (10, HelpIds.getHelpIdString(HID4_GRP_OPTIMAIZE_1024),
                self.resources.resoptOptimize1024x768_value,
                "optOptimize1024x768", 103, 158, 4, 47, 44), self)

    def buildStep5(self):
        self.lblStyleTitle = self.insertLabel(
            "lblStyleTitle", WebWizardDialog.PROPNAMES_TITLE,
            (self.fontDescriptor4, 16, self.resources.reslblStyleTitle_value,
                True, "lblStyleTitle", 91, 8, 5, 50, 232))
        self.lblStyle = self.insertLabel(
            "lblStyle", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblStyle_value,
                "lblStyle", 97, 28, 5, 51, 80))
        self.lstStyles = self.insertListBox(
            "lstStyles", None, LSTSTYLES_ITEM_CHANGED,(
                "Dropdown",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL, "LineCount",
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (True, 12, HelpIds.getHelpIdString(HID5_LST_STYLES),
                14, "lstStyles", 179, 26, 5, 52, 145), self)
        self.insertLabel("lblBackground", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblBackground, "lblBackground",
                97, 46, 5, 51, 70))
        self.txtBackground = self.insertLabel("txtBackground",
            ("Border",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (1, 12, self.resources.resBackgroundNone,
                "txtBackground", 179, 44, 5, 52, 90))
        self.btnBackgrounds = self.insertButton(
            "btnBackgrounds", BTNBACKGROUNDS_ACTION_PERFORMED,
            WebWizardDialog.PROPNAMES_BUTTON,
            (14, HelpIds.getHelpIdString(HID5_BTN_BACKGND),
                self.resources.resBtnChooseBackground, "btnBackgrounds",
                274, 43, 5, 53, 50), self)
        self.insertLabel("lblIconset", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblIconset, "lblIconset", 97, 64, 5, 51, 70))
        self.txtIconset = self.insertLabel("txtIconset",
            ("Border",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (1, 12, self.resources.resIconsetNone, "txtIconset",
                179, 62, 5, 52, 90))
        self.btnIconSets = self.insertButton(
            "btnIconSets", BTNICONSETS_ACTION_PERFORMED,
            WebWizardDialog.PROPNAMES_BUTTON,
            (14, HelpIds.getHelpIdString(HID5_BTN_ICONS),
                self.resources.resBtnChooseIconset, "btnIconSets",
                274, 61, 5, 54, 50), self)
        self.insertLabel("lblIconsetInfo",
            (PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_MULTILINE,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (16, self.resources.reslblIconSetInfo, True, "lblIconsetInfo",
                179, 78, 5, 51, 145))
        self.imgPreview = self.insertImage("imgPreview",
            ("BackgroundColor",
                PropertyNames.PROPERTY_ENABLED,
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_IMAGEURL,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y, "ScaleImage",
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (0, False, 78, "", "imgPreview", 91, 100, False, 5, 55, 232))

    def buildStep6(self):
        tabIndex = 60
        self.insertLabel("lblTitleGeneralPage",
            WebWizardDialog.PROPNAMES_TITLE,
            (self.fontDescriptor4, 8,
                self.resources.reslblTitleGeneralPage_value, True,
                "lblTitleGeneralPage", 90, 9, 6, tabIndex + 1, 232))
        self.insertLabel("lblSiteTitle", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblSiteTitle_value, "lblSiteTitle",
                97, 28, 6, tabIndex + 1, 80))
        self.txtSiteTitle = self.insertTextField(
            "txtSiteTitle", None, WebWizardDialog.PROPNAMES_TXT,
            (12, HelpIds.getHelpIdString(HID6_TXT_SITE_TITLE),
                "txtSiteTitle", 179, 26, 6, tabIndex + 1, 145), self)
        self.insertFixedLine(
            "FixedLineMetaData", WebWizardDialog.PROPNAMES_LBL,
            (9, self.resources.reslblMetaData, "FixedLineMetaData",
                97, 56, 6, tabIndex + 1, 228))
        self.insertLabel("lblSiteDesc", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblSiteDesc_value, "lblSiteDesc",
                103, 72, 6, tabIndex + 1, 80))
        self.txtSiteDesc = self.insertTextField(
            "txtSiteDesc", None, WebWizardDialog.PROPNAMES_TXT,
            (12, HelpIds.getHelpIdString(HID6_TXT_SITE_DESC), "txtSiteDesc",
                179, 70, 6, tabIndex + 1, 145), self)
        self.insertLabel(
            "lblEmail", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblEmail_value, "lblEmail",
                103, 90, 6, tabIndex + 1, 80))
        self.txtEmail = self.insertTextField(
            "txtEmail", None, WebWizardDialog.PROPNAMES_TXT,
            (12, HelpIds.getHelpIdString(HID6_TXT_SITE_EMAIL), "txtEmail",
                179, 87, 6, tabIndex + 1, 145), self)
        self.insertLabel("lblCopyright", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblCopyright_value, "lblCopyright",
                103, 108, 6, tabIndex + 1, 80))
        self.txtCopyright = self.insertTextField(
            "txtCopyright", None, WebWizardDialog.PROPNAMES_TXT,
            (12, HelpIds.getHelpIdString(HID6_TXT_SITE_COPYRIGHT),
                "txtCopyright", 179, 106, 6, tabIndex + 1, 145), self)
        self.insertLabel("lblSiteCreated", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblSiteCreated_value, "lblSiteCreated",
                103, 126, 6, tabIndex + 1, 80))
        self.dateSiteCreated = self.insertDateField("dateSiteCreated", None,
            ("Dropdown",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (True, 12, HelpIds.getHelpIdString(HID6_DATE_SITE_CREATED),
                "dateSiteCreated", 179, 124, 6, tabIndex + 1, 60), self)
        self.insertLabel("lblSiteUpdated", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblSiteUpdated_value, "lblSiteUpdated",
                103, 144, 6, tabIndex + 1, 80))
        self.dateSiteUpdate = self.insertDateField("dateSiteUpdate", None,
            ("Dropdown",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (True, 12, HelpIds.getHelpIdString(HID6_DATE_SITE_UPDATED),
                "dateSiteUpdate", 179, 142, 6, tabIndex + 1, 60), self)

    def buildStep7(self, disableFTP, exclamationURL):
        tabIndex = 700
        self.lblTitlePublish = self.insertLabel(
            "lblTitlePublish", WebWizardDialog.PROPNAMES_TITLE,
            (self.fontDescriptor4, 16, self.resources.reslblTitlePublish_value,
                True, "lblTitlePublish", 91, 8, 7, tabIndex + 1, 232))
        self.FixedLine1 = self.insertFixedLine(
            "FixedLine1", WebWizardDialog.PROPNAMES_LBL,
            (9, self.resources.resFixedLine1_value, "FixedLine1",
                97, 28, 7, tabIndex + 1, 228))
        self.btnPreview = self.insertButton(
            "btnPreview", BTNPREVIEW_ACTION_PERFORMED,
            WebWizardDialog.PROPNAMES_BUTTON,
            (14, HelpIds.getHelpIdString(HID7_BTN_PREVIEW),
                self.resources.resbtnPreview_value, "btnPreview",
                    103, 40, 7, tabIndex + 1, 50), self)
        self.insertFixedLine(
            "lblCreateSite", WebWizardDialog.PROPNAMES_LBL,
            (9, self.resources.reslblCreateSite_value, "lblCreateSite",
                97, 56, 7, tabIndex + 1, 228))
        self.chkLocalDir = self.insertCheckBox(
            "chkLocalDir", CHKLOCALDIR_ITEM_CHANGED,
            WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID7_CHK_PUBLISH_LOCAL),
                self.resources.reschkLocalDir_value, "chkLocalDir",
                103, 68, 0, 7, tabIndex + 1, 215), self)
        self.txtLocalDir = self.insertTextField(
            "txtLocalDir", None, WebWizardDialog.PROPNAMES_TXT,
            (12, HelpIds.getHelpIdString(HID7_TXT_LOCAL), "txtLocalDir",
                113, 78, 7, tabIndex + 1, 190), self)
        self.btnLocalDir = self.insertButton(
            "btnLocalDir", BTNLOCALDIR_ACTION_PERFORMED,
            WebWizardDialog.PROPNAMES_BUTTON,
            (14, HelpIds.getHelpIdString(HID7_BTN_LOCAL),
                self.resources.resbtnLocalDir_value, "btnLocalDir",
                308, 77, 7, tabIndex + 1, 16), self)
        self.chkZip = self.insertCheckBox(
            "chkZip", CHKZIP_ITEM_CHANGED, 
            WebWizardDialog.PROPNAMES_CHKBOX,
            (9, HelpIds.getHelpIdString(HID7_CHK_PUBLISH_ZIP),
                self.resources.reschkZip_value, "chkZip", 103, 96, 0, 7,
                tabIndex + 1, 215), self)
        self.txtZip = self.insertTextField(
            "txtZip", None, WebWizardDialog.PROPNAMES_TXT,
            (12, HelpIds.getHelpIdString(HID7_TXT_ZIP), "txtZip",
                113, 108, 7, tabIndex + 1, 190), self)
        self.btnZip = self.insertButton(
            "btnZip", BTNZIP_ACTION_PERFORMED,
            WebWizardDialog.PROPNAMES_BUTTON,
            (14, HelpIds.getHelpIdString(HID7_BTN_ZIP),
                self.resources.resbtnZip_value, "btnZip", 308, 107, 7,
                tabIndex + 1, 16), self)
        self.chkFTP = self.insertCheckBox(
            "chkFTP", None, WebWizardDialog.PROPNAMES_CHKBOX,
            (8, HelpIds.getHelpIdString(HID7_CHK_PUBLISH_FTP),
                self.resources.reschkFTP_value, "chkFTP", 103, 124, 0, 7,
                tabIndex + 1, 215), self)
        if disableFTP:
            FTP_STEP = 99
        else:
            FTP_STEP = 7

        self.lblFTP = self.insertLabel(
            "lblFTP", ("Border",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (1, 12, "txtFTP", 113, 134, FTP_STEP, tabIndex + 1, 156))
        self.btnFTP = self.insertButton(
            "btnFTP", BTNFTP_ACTION_PERFORMED,
            WebWizardDialog.PROPNAMES_BUTTON,
            (14, HelpIds.getHelpIdString(HID7_BTN_FTP),
                self.resources.resbtnFTP_value, "btnFTP", 274, 133,
                FTP_STEP, tabIndex + 1, 50), self)
        if disableFTP:
            self.insertImage(
                "imgFTPDisabled",("BackgroundColor", "Border",
                    PropertyNames.PROPERTY_ENABLED,
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_IMAGEURL,
                    PropertyNames.PROPERTY_NAME,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y, "ScaleImage",
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX,
                    PropertyNames.PROPERTY_WIDTH),
                (-1, 0, False, 10, exclamationURL, "imgFTPDisabled", 115,
                    135, False, 7, tabIndex + 1, 8))
            self.insertLabel(
                "lblFTPDisabled",("BackgroundColor",
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_LABEL,
                    PropertyNames.PROPERTY_MULTILINE,
                    PropertyNames.PROPERTY_NAME,
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX,
                    PropertyNames.PROPERTY_WIDTH),
                (-1, 8, self.resources.reslblFTPDisabled, True,
                    "lblFTPDisabled", 125, 136, 7, tabIndex + 1, 226))

        self.insertFixedLine(
            "lnSaveSetting", WebWizardDialog.PROPNAMES_TXT,
            (2, "", "lnSaveSetting", 97, 151, 7, tabIndex + 1, 228))
        self.chkSaveSettings = self.insertCheckBox(
            "chkSaveSettings", CHKSAVESETTINGS_ITEM_CHANGED,
            WebWizardDialog.PROPNAMES_CHKBOX,
            (9, HelpIds.getHelpIdString(HID7_CHK_SAVE),
                self.resources.reschkSaveSettings_value, "chkSaveSettings",
                97, 157, 1, 7, tabIndex + 1, 215), self)
        self.insertLabel(
            "lblSaveSettings", WebWizardDialog.PROPNAMES_LBL,
            (8, self.resources.reslblSaveSettings_value, "lblSaveSettings",
                107, 169, 7, tabIndex + 1, 45))
        self.cbSaveSettings = self.insertComboBox(
            "txtSaveSettings", TXTSAVESETTINGS_TEXT_CHANGED,
            TXTSAVESETTINGS_TEXT_CHANGED, TXTSAVESETTINGS_TEXT_CHANGED,
            ("Autocomplete", "Dropdown",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL, "LineCount",
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (True, True, 12, HelpIds.getHelpIdString(HID7_TXT_SAVE),
                14, "txtSaveSettings", 179, 167, 7, tabIndex + 1, 145), self)

    '''
    builds the layouts image list.
    '''

    def buildStepX(self):
        self.ilLayouts = ImageList()
        self.ilLayouts.pos = Size (97, 38)
        self.ilLayouts.imageSize = Size (34, 30)
        self.ilLayouts.cols = 5
        self.ilLayouts.rows = 3
        self.ilLayouts.step = 3
        self.ilLayouts.showButtons = False
        self.ilLayouts.renderer = self.LayoutRenderer()
        self.ilLayouts.scaleImages = False
        self.ilLayouts.tabIndex = 31
        self.ilLayouts.helpURL = HID3_IL_LAYOUTS_IMG1


    '''
    renders the images and titles of
    the layouts
    @author rpiterman
    '''

    class LayoutRenderer:

        def getImageUrls(self, listItem):
            oResIds = None
            if listItem is not None:
                oResIds = listItem.getImageUrls()

            return oResIds

        def render(self, listItem):
            return "" if (listItem is None) else listItem.cp_Name
