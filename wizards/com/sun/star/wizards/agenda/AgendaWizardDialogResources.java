/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.wizards.agenda;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Resource;

public class AgendaWizardDialogResources extends Resource  {

  final static String UNIT_NAME = "dbwizres";
  final static String MODULE_NAME = "dbw";
  final static int RID_AGENDAWIZARDDIALOG_START = 5000;
  final static int RID_COMMON_START = 500;


  String resAgendaWizardDialog_title;
  String resoptMakeChanges_value;
  String reslblTemplateName_value;
  String reslblTemplatePath_value;
  String reslblProceed_value;
  String reslblTitle1_value;
  String reslblTitle3_value;
  String reslblTitle2_value;
  String reslblTitle4_value;
  String reslblTitle5_value;
  String reslblTitle6_value;
  String reschkMinutes_value;
  String reslblHelp1_value;
  String reslblTime_value;
  String reslblTitle_value;
  String reslblLocation_value;
  String reslblHelp2_value;
  String resbtnTemplatePath_value;
  String resoptCreateAgenda_value;
  String reslblHelp6_value;
  String reslblTopic_value;
  String reslblResponsible_value;
  String reslblDuration_value;
  String reschkConvenedBy_value;
  String reschkPresiding_value;
  String reschkNoteTaker_value;
  String reschkTimekeeper_value;
  String reschkAttendees_value;
  String reschkObservers_value;
  String reschkResourcePersons_value;
  String reslblHelp4_value;
  String reschkMeetingTitle_value;
  String reschkRead_value;
  String reschkBring_value;
  String reschkNotes_value;
  String reslblHelp3_value;

  String reslblDate_value;
  String reslblHelpPg6_value;
  String reslblPageDesign_value;

  String resDefaultFilename;
  String resDefaultTitle;

  String resErrSaveTemplate;

  String resPlaceHolderTitle;
  String resPlaceHolderDate;
  String resPlaceHolderTime;
  String resPlaceHolderLocation;
  String resPlaceHolderHint;

  String resStep1;
  String resStep2;
  String resStep3;
  String resStep4;
  String resStep5;
  String resStep6;

  String resErrOpenTemplate;

  String itemMeetingType;
  String itemRead;
  String itemBring;
  String itemNote;
  String itemCalledBy;
  String itemFacilitator;
  String itemNotetaker;
  String itemTimekeeper;
  String itemAttendees;
  String itemObservers;
  String itemResource;

  String resButtonInsert;
  String resButtonRemove;
  String resButtonUp;
  String resButtonDown;

  String resPlaceHolderDateTitle;
  String resPlaceHolderTimeTitle;
  String resPlaceHolderLocationTitle;
  String resPlaceHolderTopics;
  String resPlaceHolderNum;
  String resPlaceHolderTopic;
  String resPlaceHolderResponsible;
  String resPlaceHolderTimeHeader;
  String resPlaceHolderAdditionalInformation;
  String resPlaceHolderMinutesFor;
  String resPlaceHolderDiscussion;
  String resPlaceHolderConclusion;
  String resPlaceHolderToDo;
  String resPlaceHolderResponsibleParty;
  String resPlaceHolderDeadline;
//


  // *** common resources ***

  String resFileExists;

public AgendaWizardDialogResources(XMultiServiceFactory xmsf) {
  super(xmsf,UNIT_NAME,MODULE_NAME);

  /**
   * Delete the String, uncomment the getResText method
   *
   */
    resAgendaWizardDialog_title =           getResText(RID_AGENDAWIZARDDIALOG_START + 1);
    resoptMakeChanges_value =               getResText(RID_AGENDAWIZARDDIALOG_START + 2);
    reslblTemplateName_value =              getResText(RID_AGENDAWIZARDDIALOG_START + 3);
    reslblTemplatePath_value =              getResText(RID_AGENDAWIZARDDIALOG_START + 4);
    reslblProceed_value =                   getResText(RID_AGENDAWIZARDDIALOG_START + 5);
    reslblTitle1_value =                    getResText(RID_AGENDAWIZARDDIALOG_START + 6);
    reslblTitle3_value =                    getResText(RID_AGENDAWIZARDDIALOG_START + 7);
    reslblTitle2_value =                    getResText(RID_AGENDAWIZARDDIALOG_START + 8);
    reslblTitle4_value =                    getResText(RID_AGENDAWIZARDDIALOG_START + 9);
    reslblTitle5_value =                    getResText(RID_AGENDAWIZARDDIALOG_START + 10);
    reslblTitle6_value =                    getResText(RID_AGENDAWIZARDDIALOG_START + 11);
    reschkMinutes_value =                   getResText(RID_AGENDAWIZARDDIALOG_START + 12);
    reslblHelp1_value =                     getResText(RID_AGENDAWIZARDDIALOG_START + 13);
    reslblTime_value =                      getResText(RID_AGENDAWIZARDDIALOG_START + 14);
    reslblTitle_value =                     getResText(RID_AGENDAWIZARDDIALOG_START + 15);
    reslblLocation_value =                  getResText(RID_AGENDAWIZARDDIALOG_START + 16);
    reslblHelp2_value =                     getResText(RID_AGENDAWIZARDDIALOG_START + 17);
    resbtnTemplatePath_value =              getResText(RID_AGENDAWIZARDDIALOG_START + 18);
    resoptCreateAgenda_value =              getResText(RID_AGENDAWIZARDDIALOG_START + 19);
    reslblHelp6_value =                     getResText(RID_AGENDAWIZARDDIALOG_START + 20);
    reslblTopic_value =                     getResText(RID_AGENDAWIZARDDIALOG_START + 21);
    reslblResponsible_value =               getResText(RID_AGENDAWIZARDDIALOG_START + 22);
    reslblDuration_value =                  getResText(RID_AGENDAWIZARDDIALOG_START + 23);
    reschkConvenedBy_value =                getResText(RID_AGENDAWIZARDDIALOG_START + 24);
    reschkPresiding_value =                 getResText(RID_AGENDAWIZARDDIALOG_START + 25);
    reschkNoteTaker_value =                 getResText(RID_AGENDAWIZARDDIALOG_START + 26);
    reschkTimekeeper_value =                getResText(RID_AGENDAWIZARDDIALOG_START + 27);
    reschkAttendees_value =                 getResText(RID_AGENDAWIZARDDIALOG_START + 28);
    reschkObservers_value =                 getResText(RID_AGENDAWIZARDDIALOG_START + 29);
    reschkResourcePersons_value =           getResText(RID_AGENDAWIZARDDIALOG_START + 30);
    reslblHelp4_value =                     getResText(RID_AGENDAWIZARDDIALOG_START + 31);
    reschkMeetingTitle_value =              getResText(RID_AGENDAWIZARDDIALOG_START + 32);
    reschkRead_value =                      getResText(RID_AGENDAWIZARDDIALOG_START + 33);
    reschkBring_value =                     getResText(RID_AGENDAWIZARDDIALOG_START + 34);
    reschkNotes_value =                     getResText(RID_AGENDAWIZARDDIALOG_START + 35);
    reslblHelp3_value =                     getResText(RID_AGENDAWIZARDDIALOG_START + 36);
    reslblDate_value =                      getResText(RID_AGENDAWIZARDDIALOG_START + 38);
    reslblHelpPg6_value =                   getResText(RID_AGENDAWIZARDDIALOG_START + 39);
    reslblPageDesign_value =                getResText(RID_AGENDAWIZARDDIALOG_START + 40);

    resDefaultFilename =                    getResText(RID_AGENDAWIZARDDIALOG_START + 41);
    resDefaultFilename = resDefaultFilename.substring(0,resDefaultFilename.length()-4) + ".ott";

    resDefaultTitle =                       getResText(RID_AGENDAWIZARDDIALOG_START + 42);

    resErrSaveTemplate =                    getResText(RID_AGENDAWIZARDDIALOG_START + 43);

    resPlaceHolderTitle =                   getResText(RID_AGENDAWIZARDDIALOG_START + 44);
    resPlaceHolderDate =                    getResText(RID_AGENDAWIZARDDIALOG_START + 45);
    resPlaceHolderTime =                    getResText(RID_AGENDAWIZARDDIALOG_START + 46);
    resPlaceHolderLocation =                getResText(RID_AGENDAWIZARDDIALOG_START + 47);
    resPlaceHolderHint =                    getResText(RID_AGENDAWIZARDDIALOG_START + 48);


    resStep1 = getResText(RID_AGENDAWIZARDDIALOG_START + 50);
    resStep2 = getResText(RID_AGENDAWIZARDDIALOG_START + 51);
    resStep3 = getResText(RID_AGENDAWIZARDDIALOG_START + 52);
    resStep4 = getResText(RID_AGENDAWIZARDDIALOG_START + 53);
    resStep5 = getResText(RID_AGENDAWIZARDDIALOG_START + 54);
    resStep6 = getResText(RID_AGENDAWIZARDDIALOG_START + 55);

    resErrOpenTemplate = getResText(RID_AGENDAWIZARDDIALOG_START + 56);

     itemMeetingType = getResText(RID_AGENDAWIZARDDIALOG_START + 57);
     itemBring = getResText(RID_AGENDAWIZARDDIALOG_START + 58);
     itemRead = getResText(RID_AGENDAWIZARDDIALOG_START + 59);
     itemNote = getResText(RID_AGENDAWIZARDDIALOG_START + 60);

     itemCalledBy = getResText(RID_AGENDAWIZARDDIALOG_START + 61);
     itemFacilitator = getResText(RID_AGENDAWIZARDDIALOG_START + 62);
     itemAttendees = getResText(RID_AGENDAWIZARDDIALOG_START + 63);
     itemNotetaker = getResText(RID_AGENDAWIZARDDIALOG_START + 64);
     itemTimekeeper = getResText(RID_AGENDAWIZARDDIALOG_START + 65);
     itemObservers = getResText(RID_AGENDAWIZARDDIALOG_START + 66);
     itemResource = getResText(RID_AGENDAWIZARDDIALOG_START + 67);

     resButtonInsert = getResText(RID_AGENDAWIZARDDIALOG_START + 68);
     resButtonRemove  = getResText(RID_AGENDAWIZARDDIALOG_START + 69);
     resButtonUp  = getResText(RID_AGENDAWIZARDDIALOG_START + 70);
     resButtonDown  = getResText(RID_AGENDAWIZARDDIALOG_START + 71);

//
     resPlaceHolderDateTitle  = getResText(RID_AGENDAWIZARDDIALOG_START + 72);
     resPlaceHolderTimeTitle  = getResText(RID_AGENDAWIZARDDIALOG_START + 73);
     resPlaceHolderLocationTitle  = getResText(RID_AGENDAWIZARDDIALOG_START + 74);
     resPlaceHolderTopics  = getResText(RID_AGENDAWIZARDDIALOG_START + 75);
     resPlaceHolderNum  = getResText(RID_AGENDAWIZARDDIALOG_START + 76);
     resPlaceHolderTopic  = getResText(RID_AGENDAWIZARDDIALOG_START + 77);
     resPlaceHolderResponsible  = getResText(RID_AGENDAWIZARDDIALOG_START + 78);
     resPlaceHolderTimeHeader  = getResText(RID_AGENDAWIZARDDIALOG_START + 79);
     resPlaceHolderAdditionalInformation  = getResText(RID_AGENDAWIZARDDIALOG_START + 80);
     resPlaceHolderMinutesFor  = getResText(RID_AGENDAWIZARDDIALOG_START + 81);
     resPlaceHolderDiscussion  = getResText(RID_AGENDAWIZARDDIALOG_START + 82);
     resPlaceHolderConclusion  = getResText(RID_AGENDAWIZARDDIALOG_START + 83);
     resPlaceHolderToDo  = getResText(RID_AGENDAWIZARDDIALOG_START + 84);
     resPlaceHolderResponsibleParty  = getResText(RID_AGENDAWIZARDDIALOG_START + 85);
     resPlaceHolderDeadline  = getResText(RID_AGENDAWIZARDDIALOG_START + 86);


//

    /*
     * Common resources
     */

    resFileExists = getResText(RID_COMMON_START + 19);

  }
}
