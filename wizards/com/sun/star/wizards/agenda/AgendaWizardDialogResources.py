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

class AgendaWizardDialogResources(Resource):
    MODULE_NAME = "dbw"
    RID_AGENDAWIZARDDIALOG_START = 5000
    RID_COMMON_START = 500

    def __init__(self, xmsf):
        super(AgendaWizardDialogResources,self).__init__(xmsf,
            AgendaWizardDialogResources.MODULE_NAME)
        #Delete the String, uncomment the getResText method
        self.resAgendaWizardDialog_title = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 1)
        self.resoptMakeChanges_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 2)
        self.reslblTemplateName_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 3)
        self.reslblTemplatePath_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 4)
        self.reslblProceed_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 5)
        self.reslblTitle1_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 6)
        self.reslblTitle3_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 7)
        self.reslblTitle2_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 8)
        self.reslblTitle4_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 9)
        self.reslblTitle5_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 10)
        self.reslblTitle6_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 11)
        self.reschkMinutes_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 12)
        self.reslblHelp1_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 13)
        self.reslblTime_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 14)
        self.reslblTitle_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 15)
        self.reslblLocation_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 16)
        self.reslblHelp2_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 17)
        self.resbtnTemplatePath_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 18)
        self.resoptCreateAgenda_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 19)
        self.reslblHelp6_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 20)
        self.reslblTopic_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 21)
        self.reslblResponsible_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 22)
        self.reslblDuration_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 23)
        self.reschkConvenedBy_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 24)
        self.reschkPresiding_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 25)
        self.reschkNoteTaker_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 26)
        self.reschkTimekeeper_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 27)
        self.reschkAttendees_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 28)
        self.reschkObservers_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 29)
        self.reschkResourcePersons_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 30)
        self.reslblHelp4_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 31)
        self.reschkMeetingTitle_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 32)
        self.reschkRead_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 33)
        self.reschkBring_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 34)
        self.reschkNotes_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 35)
        self.reslblHelp3_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 36)
        self.reslblDate_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 38)
        self.reslblHelpPg6_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 39)
        self.reslblPageDesign_value = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 40)
        self.resDefaultFilename = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 41)
        self.resDefaultFilename = self.resDefaultFilename[:-4] + ".ott"
        self.resDefaultTitle = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 42)
        self.resErrSaveTemplate = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 43)
        self.resPlaceHolderTitle = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 44)
        self.resPlaceHolderDate = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 45)
        self.resPlaceHolderTime = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 46)
        self.resPlaceHolderLocation = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 47)
        self.resPlaceHolderHint = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 48)
        self.resStep1 = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 50)
        self.resStep2 = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 51)
        self.resStep3 = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 52)
        self.resStep4 = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 53)
        self.resStep5 = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 54)
        self.resStep6 = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 55)
        self.resErrOpenTemplate = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 56)
        self.itemMeetingType = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 57)
        self.itemBring = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 58)
        self.itemRead = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 59)
        self.itemNote = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 60)
        self.itemCalledBy = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 61)
        self.itemFacilitator = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 62)
        self.itemAttendees = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 63)
        self.itemNotetaker = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 64)
        self.itemTimekeeper = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 65)
        self.itemObservers = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 66)
        self.itemResource = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 67)
        self.resButtonInsert = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 68)
        self.resButtonRemove = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 69)
        self.resButtonUp = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 70)
        self.resButtonDown = self.getResText(
            AgendaWizardDialogResources.RID_AGENDAWIZARDDIALOG_START + 71)

        #Create a dictionary for localising the template placeholders
        self.dictTemplatePlaceHolders = {
            "<title>" : self.resPlaceHolderTitle,
            "<date>" : self.resPlaceHolderDate,
            "<time>" : self.resPlaceHolderTime,
            "<location>" : self.resPlaceHolderLocation,
            "<meeting-type>" : self.reschkMeetingTitle_value,
            "<bring>" : self.reschkBring_value,
            "<read>" : self.reschkRead_value,
            "<notes>" : self.reschkNotes_value,
            "<called-by>" : self.itemCalledBy,
            "<facilitator>" : self.itemFacilitator,
            "<attendees>" : self.itemAttendees,
            "<notetaker>" : self.itemNotetaker,
            "<timekeeper>" : self.itemTimekeeper,
            "<observers>" : self.itemObservers,
            "<resource-persons>" : self.itemResource,
            "<num>" : "",
            "<topic>" : "",
            "<responsible>" : "",
            "<topic-time>" : "",
            "<mnum>" : "",
            "<mtopic>" : "",
            "<mresponsible>" : "",
            "<mtime>" : ""}            
            
        '''
        section name <b>prefix</b> for sections that contain items.
        this is also used as table name prefix, since each items section
        must contain a table whos name is identical name to the section's name.
        '''
        SECTION_ITEMS = "AGENDA_ITEMS"
        '''
        the name of the section which contains the topics.
        '''
        SECTION_TOPICS = "AGENDA_TOPICS"
        '''
        the name of the parent minutes section.
        '''
        SECTION_MINUTES_ALL = "MINUTES_ALL"
        '''
        the name of the child nimutes section.
        This section will be duplicated for each topic.
        '''
        SECTION_MINUTES = "MINUTES"


        #Common Resources
        self.resOverwriteWarning = self.getResText(
            AgendaWizardDialogResources.RID_COMMON_START + 19)
