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
from wizards.common.Resource import Resource

class LetterWizardDialogResources(Resource):

    MODULE_NAME = "dbw"
    RID_LETTERWIZARDDIALOG_START = 3000
    RID_LETTERWIZARDGREETING_START = 3080
    RID_LETTERWIZARDSALUTATION_START = 3090
    RID_LETTERWIZARDROADMAP_START = 3100
    RID_LETTERWIZARDLANGUAGE_START = 3110
    RID_RID_COMMON_START = 500

    def __init__(self, xmsf):
        super(LetterWizardDialogResources,self).__init__(
            xmsf, LetterWizardDialogResources.MODULE_NAME)

        self.RoadmapLabels = ()
        self.SalutationLabels = ()
        self.GreetingLabels = ()
        self.LanguageLabels = ()
        self.resLetterWizardDialog_title = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 1)
        self.resLabel9_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 2)
        self.resoptBusinessLetter_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 3)
        self.resoptPrivOfficialLetter_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 4)
        self.resoptPrivateLetter_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 5)
        self.reschkBusinessPaper_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 6)
        self.reschkPaperCompanyLogo_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 7)
        self.reschkPaperCompanyAddress_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 8)
        self.reschkPaperFooter_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 9)
        self.reschkCompanyReceiver_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 10)
        self.reschkUseLogo_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 11)
        self.reschkUseAddressReceiver_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 12)
        self.reschkUseSigns_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 13)
        self.reschkUseSubject_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 14)
        self.reschkUseSalutation_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 15)
        self.reschkUseBendMarks_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 16)
        self.reschkUseGreeting_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 17)
        self.reschkUseFooter_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 18)
        self.resoptSenderPlaceholder_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 19)
        self.resoptSenderDefine_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 20)
        self.resoptReceiverPlaceholder_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 21)
        self.resoptReceiverDatabase_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 22)
        self.reschkFooterNextPages_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 23)
        self.reschkFooterPageNumbers_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 24)
        self.restxtTemplateName_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 25)
        self.resoptCreateLetter_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 26)
        self.resoptMakeChanges_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 27)
        self.reslblBusinessStyle_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 28)
        self.reslblPrivOfficialStyle_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 29)
        self.reslblPrivateStyle_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 30)
        self.reslblIntroduction_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 31)
        self.reslblLogoHeight_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 32)
        self.reslblLogoWidth_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 33)
        self.reslblLogoX_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 34)
        self.reslblLogoY_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 35)
        self.reslblAddressHeight_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 36)
        self.reslblAddressWidth_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 37)
        self.reslblAddressX_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 38)
        self.reslblAddressY_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 39)
        self.reslblFooterHeight_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 40)
        self.reslblLetterNorm_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 41)
        self.reslblSenderAddress_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 42)
        self.reslblSenderName_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 43)
        self.reslblSenderStreet_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 44)
        self.reslblPostCodeCity_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 45)
        self.reslblReceiverAddress_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 46)
        self.reslblFooter_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 47)
        self.reslblFinalExplanation1_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 48)
        self.reslblFinalExplanation2_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 49)
        self.reslblTemplateName_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 50)
        self.reslblTemplatePath_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 51)
        self.reslblProceed_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 52)
        self.reslblTitle1_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 53)
        self.reslblTitle3_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 54)
        self.reslblTitle2_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 55)
        self.reslblTitle4_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 56)
        self.reslblTitle5_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 57)
        self.reslblTitle6_value = \
            self.getResText(
                LetterWizardDialogResources.RID_LETTERWIZARDDIALOG_START + 58)
        self.loadRoadmapResources()
        self.loadSalutationResources()
        self.loadGreetingResources()
        self.loadCommonResources()

    def loadCommonResources(self):
        self.resOverwriteWarning = \
            self.getResText(
                LetterWizardDialogResources.RID_RID_COMMON_START + 19)
        self.resTemplateDescription = \
            self.getResText(
                LetterWizardDialogResources.RID_RID_COMMON_START + 20)

    def loadRoadmapResources(self):
        i = 1
        for i in xrange(6):
            self.RoadmapLabels = self.RoadmapLabels + (self.getResText(LetterWizardDialogResources.RID_LETTERWIZARDROADMAP_START + i + 1),)

    def loadSalutationResources(self):
        for i in xrange(3):
            self.SalutationLabels = self.SalutationLabels + (self.getResText(LetterWizardDialogResources.RID_LETTERWIZARDSALUTATION_START + i + 1),)

    def loadGreetingResources(self):
        for i in xrange(3):
            self.GreetingLabels = self.GreetingLabels + (self.getResText(LetterWizardDialogResources.RID_LETTERWIZARDGREETING_START + i + 1),)
