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

class AgendaWizardDialogResources(object):

    RID_AGENDAWIZARDDIALOG_START = 5000
    RID_AGENDAWIZARDROADMAP_START = 5049
    RID_COMMON_START = 500
    SECTION_ITEMS = "AGENDA_ITEMS"
    SECTION_TOPICS = "AGENDA_TOPICS"
    SECTION_MINUTES_ALL = "MINUTES_ALL"
    SECTION_MINUTES = "MINUTES"

    def __init__(self, oWizardResource):
        self.resAgendaWizardDialog_title = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 1)
        self.resoptMakeChanges_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 2)
        self.reslblTemplateName_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 3)
        self.reslblTemplatePath_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 4)
        self.reslblProceed_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 5)
        self.reslblTitle1_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 6)
        self.reslblTitle3_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 7)
        self.reslblTitle2_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 8)
        self.reslblTitle4_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 9)
        self.reslblTitle5_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 10)
        self.reslblTitle6_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 11)
        self.reschkMinutes_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 12)
        self.reslblHelp1_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 13)
        self.reslblTime_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 14)
        self.reslblTitle_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 15)
        self.reslblLocation_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 16)
        self.reslblHelp2_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 17)
        self.resbtnTemplatePath_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 18)
        self.resoptCreateAgenda_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 19)
        self.reslblHelp6_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 20)
        self.reslblTopic_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 21)
        self.reslblResponsible_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 22)
        self.reslblDuration_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 23)
        self.reschkConvenedBy_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 24)
        self.reschkPresiding_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 25)
        self.reschkNoteTaker_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 26)
        self.reschkTimekeeper_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 27)
        self.reschkAttendees_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 28)
        self.reschkObservers_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 29)
        self.reschkResourcePersons_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 30)
        self.reslblHelp4_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 31)
        self.reschkMeetingTitle_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 32)
        self.reschkRead_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 33)
        self.reschkBring_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 34)
        self.reschkNotes_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 35)
        self.reslblHelp3_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 36)
        self.reslblDate_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 38)
        self.reslblHelpPg6_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 39)
        self.reslblPageDesign_value = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 40)
        self.resDefaultFilename = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 41)
        self.resDefaultFilename = self.resDefaultFilename[:-4] + ".ott"
        self.resDefaultTitle = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 42)
        self.resErrSaveTemplate = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 43)
        self.resPlaceHolderTitle = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 44)
        self.resPlaceHolderDate = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 45)
        self.resPlaceHolderTime = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 46)
        self.resPlaceHolderLocation = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 47)
        self.resPlaceHolderHint = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 48)
        self.resErrOpenTemplate = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 56)
        self.itemMeetingType = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 57)
        self.itemBring = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 58)
        self.itemRead = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 59)
        self.itemNote = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 60)
        self.itemCalledBy = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 61)
        self.itemFacilitator = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 62)
        self.itemAttendees = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 63)
        self.itemNotetaker = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 64)
        self.itemTimekeeper = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 65)
        self.itemObservers = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 66)
        self.itemResource = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 67)
        self.resButtonInsert = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 68)
        self.resButtonRemove = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 69)
        self.resButtonUp = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 70)
        self.resButtonDown = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 71)

        #Create a dictionary for localised string in the template
        self.dictConstants = {
        "#datetitle#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 72),
        "#timetitle#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 73),
        "#locationtitle#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 74),
        "#topics#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 75),
        "#num.#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 76),
        "#topicheader#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 77),
        "#responsibleheader#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 78),
        "#timeheader#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 79),
        "#additional-information#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 80),
        "#minutes-for#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 81),
        "#discussion#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 82),
        "#conclusion#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 83),
        "#to-do#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 84),
        "#responsible-party#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 85),
        "#deadline#" : oWizardResource.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 86)}

        #Create a dictionary for localising the page design
        self.dictPageDesign = {
        "Blue" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 87),
        "Classic" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 88),
        "Colorful" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 89),
        "Elegant" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 90),
        "Green" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 91),
        "Grey" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 92),
        "Modern" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 93),
        "Orange" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 94),
        "Red" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 95),
        "Simple" : oWizardResource.getResText(
                AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 96)}

        #Common Resources
        self.resOverwriteWarning = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_COMMON_START + 19)
        self.resTemplateDescription = oWizardResource.getResText(
            AgendaWizardDialogResources.RID_COMMON_START + 20)

        self.RoadmapLabels = oWizardResource.getResArray(
            AgendaWizardDialogResources.RID_AGENDAWIZARDROADMAP_START + 1 , 6)
