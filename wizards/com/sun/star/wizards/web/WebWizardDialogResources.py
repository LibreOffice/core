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
import traceback
from common.Resource import Resource
from common.Configuration import Configuration
from common.Properties import Properties
from document.OfficeDocument import OfficeDocument

class WebWizardDialogResources(Resource):
    UNIT_NAME = "dbwizres"
    MODULE_NAME = "dbw"
    RID_WEBWIZARDDIALOG_START = 4000
    RID_COMMON_START = 500
    RID_DB_COMMON_START = 1000

    def __init__(self, xmsf):
        super(WebWizardDialogResources,self).__init__(xmsf,
            WebWizardDialogResources.MODULE_NAME)
        self.resbtnDocUp_value = str(8743)
        self.resbtnDocDown_value = str(8744)

        #Delete the String, uncomment the getResText method
        self.resWebWizardDialog_title = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 1)
        self.reslblIntroduction_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 2)
        self.resbtnDelSession_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 4)
        self.reslbIntroTitle_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 5)
        self.reslblContentTitle_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 6)
        self.reslnDocsInfo_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 8)
        self.reslblDocTitle_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 9)
        self.reslblDocInfo_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 10)
        self.reslblDocAuthor_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 11)
        self.reslblDocExportFormat_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 12)
        self.reslblTitleGeneralPage_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 13)
        self.reslblSiteTitle_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 14)
        self.reslblSiteDesc_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 16)
        self.reslblSiteCreated_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 18)
        self.reslblEmail_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 20)
        self.reslblCopyright_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 21)
        self.reslblSiteUpdated_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 22)
        self.resbtnPreview_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 24)
        self.reslblTitlePublish_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 25)
        self.reslblCreateSite_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 26)
        self.reschkLocalDir_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 27)
        self.resbtnLocalDir_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 28)
        self.reschkFTP_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 29)

        self.resbtnFTP_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 31)
        self.reschkZip_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 32)
        self.resbtnZip_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 33)
        self.reschkSaveSettings_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 34)
        self.reslblSaveSettings_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 35)
        self.reslblLoadSettings_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 36)
        self.reslblSiteContent_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 37)
        self.resbtnAddDoc_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 38)
        self.resbtnRemoveDoc_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 39)
        self.reslblLayoutTitle_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 40)
        self.reslblStyleTitle_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 41)
        self.reslblStyle_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 42)

        self.reslblLayouts_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 48)

        self.reschbDocDesc_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 50)
        self.reschbDocAuthor_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 51)
        self.reschkDocCreated_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 52)
        self.reschkDocChanged_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 53)
        self.reschkDocFilename_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 54)
        self.reschkDocFormat_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 55)
        self.reschkDocFormatIcon_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 56)
        self.reschkDocPages_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 57)
        self.reschkDocSize_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 58)
        self.resFixedLine1_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 59)
        self.reslblLayout2Title_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 60)
        self.reslblDisplay_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 61)
        self.reslblOptimizeFor_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 62)
        self.resoptOptimize640x480_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 64)
        self.resoptOptimize800x600_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 65)
        self.resoptOptimize1024x768_value = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 66)

        self.resStatusDialogTitle = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 70)
        self.resCounter = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 71)
        self.resPublishDir = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 72)
        self.resFTPTargetExistsAsfile = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 73)
        self.resLocalTargetExistsAsfile = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 74)
        self.resZipTargetExists = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 75)
        self.resFTPTargetNotEmpty = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 76)
        self.resLocalTragetNotEmpty = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 77)
        self.resSessionExists = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 78)
        self.resTaskExportDocs = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 79)
        self.resTaskExportPrepare = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 80)
        self.resTaskGenerateCopy = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 81)
        self.resTaskGeneratePrepare = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 82)
        self.resTaskGenerateXsl = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 83)
        self.resTaskPrepare = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 84)
        self.resTaskPublishPrepare = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 86)
        self.resTaskPublishLocal = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 87)
        self.resTaskPublishFTP = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 88)
        self.resTaskPublishZip = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 89)
        self.resTaskFinish = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 90)

        self.resImages = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 94)
        self.resAllFiles = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 95)
        self.resZipFiles = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 96)
        self.resBackgroundsDialog = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 98)
        self.resBackgroundsDialogCaption = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 99)
        self.resIconsDialog = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 100)
        self.resIconsDialogCaption = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 101)
        self.resOther = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 102)
        self.resDeselect = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 103)
        self.resFinishedSuccess = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 104)
        self.resFinishedNoSuccess = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 105)
        self.resErrTOC = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 106)
        self.resErrTOCMedia = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 107)
        self.resErrDocInfo = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 108)
        self.resErrDocExport = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 109)
        self.resErrMkDir = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 110)
        self.resErrSecurity = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 111)
        self.resErrExportIO = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 112)
        self.resErrPublishMedia = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 113)
        self.resErrPublish = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 114)
        self.resErrUnknown = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 115)
        self.resErrDocValidate = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 116)
        self.resErrIsDirectory = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 117)
        self.resLoadingSession = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 118)
        self.resErrUnexpected = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 119)
        self.resValidatingDocuments = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 120)
        self.resDefaultArchiveFilename = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 121)
        self.resStep1 = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 122)
        self.resStep2 = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 123)
        self.resStep3 = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 124)
        self.resStep4 = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 125)
        self.resStep5 = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 126)
        self.resStep6 = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 127)
        self.resStep7 = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 128)
        self.resDelSessionConfirm = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 130)
        self.resPages = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 131)
        self.resSlides = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 132)
        self.resCreatedTemplate = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 133)
        self.resUpdatedTemplate = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 134)
        self.resSessionName = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 135)
        self.reslblBackground = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 136)
        self.reslblIconset = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 137)
        self.reslblIconSetInfo = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 138)
        self.reslblMetaData = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 139)
        self.resBtnChooseBackground = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 140)
        self.resBtnChooseIconset = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 141)
        self.resSessionNameNone = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 142)
        self.reslblFTPDisabled = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 143)
        self.resBackgroundNone = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 144)
        self.resIconsetNone = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 145)
        self.resFTPTargetCouldNotCreate = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 146)
        self.resFTPTargetCreate = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 147)
        self.resZipTargetIsDir = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 148)
        self.resLocalTargetCouldNotCreate = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 149)
        self.resLocalTargetCreate = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 150)
        self.resSizeTemplate = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 151)
        self.resSpecifyNewFileLocation = self.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 152)
        self.resClose = self.getResText(
            WebWizardDialogResources.RID_COMMON_START + 17)
        self.resCancel = self.getResText(
            WebWizardDialogResources.RID_COMMON_START + 11)
        self.resOK = self.getResText(
            WebWizardDialogResources.RID_COMMON_START + 18)
        self.resHelp = self.getResText(
            WebWizardDialogResources.RID_COMMON_START + 15)
        try:
            self.prodName = Configuration.getProductName(xmsf)
            self.resGifFiles = Properties.getPropertyValue(
                OfficeDocument.getTypeMediaDescriptor(
                    xmsf, "gif_Graphics_Interchange"), "UIName")
        except Exception, ex:
            traceback.print_exc()
