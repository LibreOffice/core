/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.wizards.agenda;

import com.sun.star.wizards.common.HelpIds;

public interface AgendaWizardDialogConst {

   /**
    * These are event method names, used with reflection
    * to call the methods under these names uppon
    * events.
    */

   final String LISTPAGEDESIGN_ACTION_PERFORMED =  "pageDesignChanged";
   final String TXTTEMPLATENAME_TEXT_CHANGED =     "templateTitleChanged";
   final String BTNTEMPLATEPATH_ACTION_PERFORMED = "saveAs";
   final String BTNINSERT_ACTION_PERFORMED =       "insertRow";
   final String BTNREMOVE_ACTION_PERFORMED =       "removeRow";
   final String BTNUP_ACTION_PERFORMED =           "rowUp";
   final String BTNDOWN_ACTION_PERFORMED =         "rowDown";

   final String INFO_IMAGE_URL = "private:resource/dbu/image/19205";
  /* ====================================
   *                 Help IDs
   * ====================================
   */
   final int HID = 41051;

   /*
   * "HID:(hid)"   - the dialog
   * "HID:(hid+1)  - the help button
   * "HID:(hid+2)" - the back button
   * "HID:(hid+3)" - the next button
   * "HID:(hid+4)" - the create button
   * "HID:(hid+5)" - the cancel button
   */

   final String LISTPAGEDESIGN_HID =          HelpIds.getHelpIdString((HID + 6));
   final String CHKMINUTES_HID =              HelpIds.getHelpIdString((HID + 7));
   final String TXTTIME_HID =                 HelpIds.getHelpIdString((HID + 8));
   final String TXTDATE_HID =                 HelpIds.getHelpIdString((HID + 9));
   final String TXTTITLE_HID =                HelpIds.getHelpIdString((HID + 10));
   final String CBLOCATION_HID =              HelpIds.getHelpIdString((HID + 11));

   final String CHKMEETINGTITLE_HID =         HelpIds.getHelpIdString((HID + 12));
   final String CHKREAD_HID =                 HelpIds.getHelpIdString((HID + 13));
   final String CHKBRING_HID =                HelpIds.getHelpIdString((HID + 14));
   final String CHKNOTES_HID =                HelpIds.getHelpIdString((HID + 15));

   final String CHKCONVENEDBY_HID =           HelpIds.getHelpIdString((HID + 16));
   final String CHKPRESIDING_HID =            HelpIds.getHelpIdString((HID + 17));
   final String CHKNOTETAKER_HID =            HelpIds.getHelpIdString((HID + 18));
   final String CHKTIMEKEEPER_HID =           HelpIds.getHelpIdString((HID + 19));
   final String CHKATTENDEES_HID =            HelpIds.getHelpIdString((HID + 20));
   final String CHKOBSERVERS_HID =            HelpIds.getHelpIdString((HID + 21));
   final String CHKRESOURCEPERSONS_HID =      HelpIds.getHelpIdString((HID + 22));

   final String TXTTEMPLATENAME_HID =         HelpIds.getHelpIdString((HID + 23));
   final String TXTTEMPLATEPATH_HID =         HelpIds.getHelpIdString((HID + 24));
   final String BTNTEMPLATEPATH_HID =         HelpIds.getHelpIdString((HID + 25));

   final String OPTCREATEAGENDA_HID =         HelpIds.getHelpIdString((HID + 26));
   final String OPTMAKECHANGES_HID =          HelpIds.getHelpIdString((HID + 27));

   final String BTNINSERT_HID =               HelpIds.getHelpIdString((HID + 28));
   final String BTNREMOVE_HID =               HelpIds.getHelpIdString((HID + 29));
   final String BTNUP_HID =                   HelpIds.getHelpIdString((HID + 30));
   final String BTNDOWN_HID =                 HelpIds.getHelpIdString((HID + 31));

   final int LAST_HID = HID + 32;

   /*
    * 32 : scroll bar
    * 33 - 35 first topics row
    * 36 - 38 second
    * 39 - 41
    * 42 - 44
    * 45 - 47
    * total 5 rows / 3 controls.
    */


}

