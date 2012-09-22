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
from ..common.Resource import Resource

class FaxWizardDialogResources(Resource):
    MODULE_NAME = "dbw"
    RID_FAXWIZARDDIALOG_START = 3200
    RID_FAXWIZARDCOMMUNICATION_START = 3270
    RID_FAXWIZARDGREETING_START = 3280
    RID_FAXWIZARDSALUTATION_START = 3290
    RID_FAXWIZARDROADMAP_START = 3300
    RID_RID_COMMON_START = 500


    def __init__(self, xmsf):
        super(FaxWizardDialogResources,self).__init__(xmsf,
            FaxWizardDialogResources.MODULE_NAME)
        self.RoadmapLabels = ()
        self.SalutationLabels = ()
        self.GreetingLabels = ()
        self.CommunicationLabels = ()

        #Delete the String, uncomment the self.getResText method


        self.resFaxWizardDialog_title = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 1)
        self.resLabel9_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 2)
        self.resoptBusinessFax_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 3)
        self.resoptPrivateFax_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 4)
        self.reschkUseLogo_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 5)
        self.reschkUseSubject_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 6)
        self.reschkUseSalutation_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 7)
        self.reschkUseGreeting_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 8)
        self.reschkUseFooter_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 9)
        self.resoptSenderPlaceholder_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 10)
        self.resoptSenderDefine_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 11)
        self.restxtTemplateName_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 12)
        self.resoptCreateFax_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 13)
        self.resoptMakeChanges_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 14)
        self.reslblBusinessStyle_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 15)
        self.reslblPrivateStyle_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 16)
        self.reslblIntroduction_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 17)
        self.reslblSenderAddress_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 18)
        self.reslblSenderName_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 19)
        self.reslblSenderStreet_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 20)
        self.reslblPostCodeCity_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 21)
        self.reslblFooter_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 22)
        self.reslblFinalExplanation1_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 23)
        self.reslblFinalExplanation2_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 24)
        self.reslblTemplateName_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 25)
        self.reslblTemplatePath_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 26)
        self.reslblProceed_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 27)
        self.reslblTitle1_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 28)
        self.reslblTitle3_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 29)
        self.reslblTitle4_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 30)
        self.reslblTitle5_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 31)
        self.reslblTitle6_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 32)
        self.reschkFooterNextPages_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 33)
        self.reschkFooterPageNumbers_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 34)
        self.reschkUseDate_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 35)
        self.reschkUseCommunicationType_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 36)
        self.resLabel1_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 37)
        self.resoptReceiverPlaceholder_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 38)
        self.resoptReceiverDatabase_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 39)
        self.resLabel2_value = self.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 40)
        self.loadRoadmapResources()
        self.loadSalutationResources()
        self.loadGreetingResources()
        self.loadCommunicationResources()
        self.loadCommonResources()

    def loadCommonResources(self):
        self.resOverwriteWarning = self.getResText(
            FaxWizardDialogResources.RID_RID_COMMON_START + 19)
        self.resTemplateDescription = self.getResText(
            FaxWizardDialogResources.RID_RID_COMMON_START + 20)

    def loadRoadmapResources(self):
        for i in xrange(5):
            self.RoadmapLabels = self.RoadmapLabels + ((self.getResText(
                FaxWizardDialogResources.RID_FAXWIZARDROADMAP_START + \
                + i + 1)),)

    def loadSalutationResources(self):
        i = 1
        for i in xrange(4):
            self.SalutationLabels = self.SalutationLabels + ((self.getResText(
                FaxWizardDialogResources.RID_FAXWIZARDSALUTATION_START + \
                i + 1)),)

    def loadGreetingResources(self):
        for i in xrange(4):
            self.GreetingLabels = self.GreetingLabels + ((self.getResText(
                FaxWizardDialogResources.RID_FAXWIZARDGREETING_START + \
                i +1 )),)

    def loadCommunicationResources(self):
        for i in xrange(3):
            self.CommunicationLabels = \
            self.CommunicationLabels + ((self.getResText(
                FaxWizardDialogResources.RID_FAXWIZARDCOMMUNICATION_START + \
                i + 1)),)
