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

class FaxWizardDialogResources(object):

    RID_FAXWIZARDDIALOG_START = 3200
    RID_FAXWIZARDCOMMUNICATION_START = 3270
    RID_FAXWIZARDGREETING_START = 3280
    RID_FAXWIZARDSALUTATION_START = 3290
    RID_FAXWIZARDROADMAP_START = 3300
    RID_RID_COMMON_START = 500

    def __init__(self, oWizardResource):

        self.resFaxWizardDialog_title = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 1)
        self.resoptBusinessFax_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 3)
        self.resoptPrivateFax_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 4)
        self.reschkUseLogo_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 5)
        self.reschkUseSubject_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 6)
        self.reschkUseSalutation_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 7)
        self.reschkUseGreeting_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 8)
        self.reschkUseFooter_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 9)
        self.resoptSenderPlaceholder_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 10)
        self.resoptSenderDefine_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 11)
        self.restxtTemplateName_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 12)
        self.resoptCreateFax_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 13)
        self.resoptMakeChanges_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 14)
        self.reslblBusinessStyle_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 15)
        self.reslblPrivateStyle_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 16)
        self.reslblIntroduction_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 17)
        self.reslblSenderAddress_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 18)
        self.reslblSenderName_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 19)
        self.reslblSenderStreet_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 20)
        self.reslblPostCodeCity_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 21)
        self.reslblFooter_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 22)
        self.reslblFinalExplanation1_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 23)
        self.reslblFinalExplanation2_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 24)
        self.reslblTemplateName_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 25)
        self.reslblTemplatePath_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 26)
        self.reslblProceed_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 27)
        self.reslblTitle1_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 28)
        self.reslblTitle3_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 29)
        self.reslblTitle4_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 30)
        self.reslblTitle5_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 31)
        self.reslblTitle6_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 32)
        self.reschkFooterNextPages_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 33)
        self.reschkFooterPageNumbers_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 34)
        self.reschkUseDate_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 35)
        self.reschkUseCommunicationType_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 36)
        self.resLabel1_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 37)
        self.resoptReceiverPlaceholder_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 38)
        self.resoptReceiverDatabase_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 39)
        self.resLabel2_value = oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 40)

        #Create a Dictionary for the constants values.
        self.dictConstants = {
        "#to#" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 41),
        "#from#" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 42),
        "#faxconst#" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 43),
        "#telconst#" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 44),
        "#emailconst#" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 45),
        "#consist1#" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 46),
        "#consist2#" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 47),
        "#consist3#" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 48)}

        #Create a dictionary for localising the private template
        self.dictPrivateTemplate = {
        "Bottle" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 49),
        "Fax" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 56),
        "Lines" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 50),
        "Marine" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 51)}

        #Create a dictionary for localising the business template
        self.dictBusinessTemplate = {
        "Classic Fax" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 52),
        "Classic Fax from Private" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 53),
        "Modern Fax" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 54),
        "Modern Fax from Private" : oWizardResource.getResText(
            FaxWizardDialogResources.RID_FAXWIZARDDIALOG_START + 55)}

        #Common Resources
        self.resOverwriteWarning = oWizardResource.getResText(
            FaxWizardDialogResources.RID_RID_COMMON_START + 19)
        self.resTemplateDescription = oWizardResource.getResText(
            FaxWizardDialogResources.RID_RID_COMMON_START + 20)

        self.RoadmapLabels = oWizardResource.getResArray(
            FaxWizardDialogResources.RID_FAXWIZARDROADMAP_START + 1 , 5)
        self.SalutationLabels = oWizardResource.getResArray(
            FaxWizardDialogResources.RID_FAXWIZARDSALUTATION_START + 1 , 4)
        self.GreetingLabels = oWizardResource.getResArray(
            FaxWizardDialogResources.RID_FAXWIZARDGREETING_START + 1 , 4)
        self.CommunicationLabels = oWizardResource.getResArray(
            FaxWizardDialogResources.RID_FAXWIZARDCOMMUNICATION_START + 1 , 3)
