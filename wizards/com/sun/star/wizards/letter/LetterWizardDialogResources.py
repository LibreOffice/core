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

class LetterWizardDialogResources(object):

    RID_LETTERWIZARDDIALOG_START = 3000
    RID_LETTERWIZARDGREETING_START = 3080
    RID_LETTERWIZARDSALUTATION_START = 3090
    RID_LETTERWIZARDROADMAP_START = 3100
    RID_RID_COMMON_START = 500

    def __init__(self, oWizardResource):

        self.resLetterWizardDialog_title = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 1)
        self.resLabel9_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 2)
        self.resoptBusinessLetter_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 3)
        self.resoptPrivOfficialLetter_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 4)
        self.resoptPrivateLetter_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 5)
        self.reschkBusinessPaper_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 6)
        self.reschkPaperCompanyLogo_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 7)
        self.reschkPaperCompanyAddress_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 8)
        self.reschkPaperFooter_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 9)
        self.reschkCompanyReceiver_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 10)
        self.reschkUseLogo_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 11)
        self.reschkUseAddressReceiver_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 12)
        self.reschkUseSigns_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 13)
        self.reschkUseSubject_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 14)
        self.reschkUseSalutation_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 15)
        self.reschkUseBendMarks_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 16)
        self.reschkUseGreeting_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 17)
        self.reschkUseFooter_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 18)
        self.resoptSenderPlaceholder_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 19)
        self.resoptSenderDefine_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 20)
        self.resoptReceiverPlaceholder_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 21)
        self.resoptReceiverDatabase_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 22)
        self.reschkFooterNextPages_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 23)
        self.reschkFooterPageNumbers_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 24)
        self.restxtTemplateName_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 25)
        self.resoptCreateLetter_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 26)
        self.resoptMakeChanges_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 27)
        self.reslblBusinessStyle_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 28)
        self.reslblPrivOfficialStyle_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 29)
        self.reslblPrivateStyle_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 30)
        self.reslblIntroduction_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 31)
        self.reslblLogoHeight_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 32)
        self.reslblLogoWidth_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 33)
        self.reslblLogoX_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 34)
        self.reslblLogoY_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 35)
        self.reslblAddressHeight_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 36)
        self.reslblAddressWidth_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 37)
        self.reslblAddressX_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 38)
        self.reslblAddressY_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 39)
        self.reslblFooterHeight_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 40)
        self.reslblSenderAddress_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 42)
        self.reslblSenderName_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 43)
        self.reslblSenderStreet_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 44)
        self.reslblPostCodeCity_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 45)
        self.reslblReceiverAddress_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 46)
        self.reslblFooter_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 47)
        self.reslblFinalExplanation1_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 48)
        self.reslblFinalExplanation2_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 49)
        self.reslblTemplateName_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 50)
        self.reslblTemplatePath_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 51)
        self.reslblProceed_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 52)
        self.reslblTitle1_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 53)
        self.reslblTitle3_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 54)
        self.reslblTitle2_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 55)
        self.reslblTitle4_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 56)
        self.reslblTitle5_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 57)
        self.reslblTitle6_value = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 58)

        #Create a Dictionary for the constants values.
        self.dictConstants = {
        "#subjectconst#" : oWizardResource.getResText(
            LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 59)}

        #Create a dictionary for localising the business templates
        self.dictBusinessTemplate = {
        "Elegant" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 60),
        "Modern" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 61),
        "Office" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 62)}

        #Create a dictionary for localising the official templates
        self.dictOfficialTemplate = {
        "Elegant" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 60),
        "Modern" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 61),
        "Office" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 62)}

        #Create a dictionary for localising the private templates
        self.dictPrivateTemplate = {
        "Bottle" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 63),
        "Mail" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 64),
        "Marine" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 65),
        "Red Line" : oWizardResource.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 66)}

        #Common Resources
        self.resOverwriteWarning = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_RID_COMMON_START + 19)
        self.resTemplateDescription = \
            oWizardResource.getResText(
                LetterWizardDialogResources.RID_RID_COMMON_START + 20)

        self.RoadmapLabels = oWizardResource.getResArray(
            LetterWizardDialogResources.RID_LETTERWIZARDROADMAP_START + 1, 6)
        self.SalutationLabels = oWizardResource.getResArray(
            LetterWizardDialogResources.RID_LETTERWIZARDSALUTATION_START + 1, 3)
        self.GreetingLabels = oWizardResource.getResArray(
            LetterWizardDialogResources.RID_LETTERWIZARDGREETING_START + 1, 3)
