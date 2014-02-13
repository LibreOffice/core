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
from ..common.Configuration import Configuration
from ..common.Properties import Properties
from ..document.OfficeDocument import OfficeDocument

class WebWizardDialogResources(object):

    RID_WEBWIZARDDIALOG_START = 4000
    RID_COMMON_START = 500
    RID_DB_COMMON_START = 1000
    RID_WEBWIZARDROADMAP_START = 4121

    def __init__(self, xmsf, oWizardResource):
        try:
            #python 3
            self.resbtnDocUp_value = chr(8743)
            self.resbtnDocDown_value = chr(8744)
        except ValueError:
            #python 2
            self.resbtnDocUp_value = unichr(8743)
            self.resbtnDocDown_value = unichr(8744) 
                    
        self.resWebWizardDialog_title = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 1)
        self.reslblIntroduction_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 2)
        self.resbtnDelSession_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 4)
        self.reslbIntroTitle_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 5)
        self.reslblContentTitle_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 6)
        self.reslnDocsInfo_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 8)
        self.reslblDocTitle_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 9)
        self.reslblDocInfo_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 10)
        self.reslblDocAuthor_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 11)
        self.reslblDocExportFormat_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 12)
        self.reslblTitleGeneralPage_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 13)
        self.reslblSiteTitle_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 14)
        self.reslblSiteDesc_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 16)
        self.reslblSiteCreated_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 18)
        self.reslblEmail_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 20)
        self.reslblCopyright_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 21)
        self.reslblSiteUpdated_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 22)
        self.resbtnPreview_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 24)
        self.reslblTitlePublish_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 25)
        self.reslblCreateSite_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 26)
        self.reschkLocalDir_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 27)
        self.resbtnLocalDir_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 28)
        self.reschkFTP_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 29)

        self.resbtnFTP_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 31)
        self.reschkZip_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 32)
        self.resbtnZip_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 33)
        self.reschkSaveSettings_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 34)
        self.reslblSaveSettings_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 35)
        self.reslblLoadSettings_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 36)
        self.reslblSiteContent_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 37)
        self.resbtnAddDoc_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 38)
        self.resbtnRemoveDoc_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 39)
        self.reslblLayoutTitle_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 40)
        self.reslblStyleTitle_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 41)
        self.reslblStyle_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 42)

        self.reslblLayouts_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 48)

        self.reschbDocDesc_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 50)
        self.reschbDocAuthor_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 51)
        self.reschkDocCreated_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 52)
        self.reschkDocChanged_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 53)
        self.reschkDocFilename_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 54)
        self.reschkDocFormat_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 55)
        self.reschkDocFormatIcon_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 56)
        self.reschkDocPages_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 57)
        self.reschkDocSize_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 58)
        self.resFixedLine1_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 59)
        self.reslblLayout2Title_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 60)
        self.reslblDisplay_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 61)
        self.reslblOptimizeFor_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 62)
        self.resoptOptimize640x480_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 64)
        self.resoptOptimize800x600_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 65)
        self.resoptOptimize1024x768_value = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 66)

        self.resStatusDialogTitle = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 70)
        self.resCounter = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 71)
        self.resPublishDir = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 72)
        self.resFTPTargetExistsAsfile = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 73)
        self.resLocalTargetExistsAsfile = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 74)
        self.resZipTargetExists = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 75)
        self.resFTPTargetNotEmpty = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 76)
        self.resLocalTragetNotEmpty = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 77)
        self.resSessionExists = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 78)
        self.resTaskExportDocs = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 79)
        self.resTaskExportPrepare = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 80)
        self.resTaskGenerateCopy = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 81)
        self.resTaskGeneratePrepare = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 82)
        self.resTaskGenerateXsl = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 83)
        self.resTaskPrepare = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 84)
        self.resTaskPublishPrepare = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 86)
        self.resTaskPublishLocal = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 87)
        self.resTaskPublishFTP = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 88)
        self.resTaskPublishZip = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 89)
        self.resTaskFinish = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 90)

        self.resImages = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 94)
        self.resAllFiles = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 95)
        self.resZipFiles = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 96)
        self.resBackgroundsDialog = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 98)
        self.resBackgroundsDialogCaption = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 99)
        self.resIconsDialog = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 100)
        self.resIconsDialogCaption = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 101)
        self.resOther = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 102)
        self.resDeselect = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 103)
        self.resFinishedSuccess = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 104)
        self.resFinishedNoSuccess = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 105)
        self.resErrTOC = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 106)
        self.resErrTOCMedia = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 107)
        self.resErrDocInfo = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 108)
        self.resErrDocExport = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 109)
        self.resErrMkDir = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 110)
        self.resErrSecurity = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 111)
        self.resErrExportIO = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 112)
        self.resErrPublishMedia = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 113)
        self.resErrPublish = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 114)
        self.resErrUnknown = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 115)
        self.resErrDocValidate = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 116)
        self.resErrIsDirectory = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 117)
        self.resLoadingSession = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 118)
        self.resErrUnexpected = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 119)
        self.resValidatingDocuments = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 120)
        self.resDefaultArchiveFilename = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 121)
        self.resDelSessionConfirm = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 130)
        self.resPages = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 131)
        self.resSlides = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 132)
        self.resCreatedTemplate = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 133)
        self.resUpdatedTemplate = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 134)
        self.resSessionName = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 135)
        self.reslblBackground = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 136)
        self.reslblIconset = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 137)
        self.reslblIconSetInfo = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 138)
        self.reslblMetaData = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 139)
        self.resBtnChooseBackground = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 140)
        self.resBtnChooseIconset = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 141)
        self.resSessionNameNone = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 142)
        self.reslblFTPDisabled = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 143)
        self.resBackgroundNone = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 144)
        self.resIconsetNone = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 145)
        self.resFTPTargetCouldNotCreate = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 146)
        self.resFTPTargetCreate = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 147)
        self.resZipTargetIsDir = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 148)
        self.resLocalTargetCouldNotCreate = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 149)
        self.resLocalTargetCreate = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 150)
        self.resSizeTemplate = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 151)
        self.resSpecifyNewFileLocation = oWizardResource.getResText(
            WebWizardDialogResources.RID_WEBWIZARDDIALOG_START + 152)
        self.resClose = oWizardResource.getResText(
            WebWizardDialogResources.RID_COMMON_START + 17)
        self.resCancel = oWizardResource.getResText(
            WebWizardDialogResources.RID_COMMON_START + 11)
        self.resOK = oWizardResource.getResText(
            WebWizardDialogResources.RID_COMMON_START + 18)
        self.resHelp = oWizardResource.getResText(
            WebWizardDialogResources.RID_COMMON_START + 15)
            
            
        self.RoadmapLabels = oWizardResource.getResArray(
            WebWizardDialogResources.RID_WEBWIZARDROADMAP_START + 1 , 7)
                        
        try:
            self.prodName = Configuration.getProductName(xmsf)
            self.resGifFiles = Properties.getPropertyValue(
                OfficeDocument.getTypeMediaDescriptor(
                    xmsf, "gif_Graphics_Interchange"), "UIName")
        except Exception as ex:
            traceback.print_exc()
