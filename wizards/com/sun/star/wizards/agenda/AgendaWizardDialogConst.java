/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AgendaWizardDialogConst.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:15:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.agenda;

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

   final String LISTPAGEDESIGN_HID =          "HID:" + (HID + 6);
   final String CHKMINUTES_HID =              "HID:" + (HID + 7);
   final String TXTTIME_HID =                 "HID:" + (HID + 8);
   final String TXTDATE_HID =                 "HID:" + (HID + 9);
   final String TXTTITLE_HID =                "HID:" + (HID + 10);
   final String CBLOCATION_HID =              "HID:" + (HID + 11);

   final String CHKMEETINGTITLE_HID =         "HID:" + (HID + 12);
   final String CHKREAD_HID =                 "HID:" + (HID + 13);
   final String CHKBRING_HID =                "HID:" + (HID + 14);
   final String CHKNOTES_HID =                "HID:" + (HID + 15);

   final String CHKCONVENEDBY_HID =           "HID:" + (HID + 16);
   final String CHKPRESIDING_HID =            "HID:" + (HID + 17);
   final String CHKNOTETAKER_HID =            "HID:" + (HID + 18);
   final String CHKTIMEKEEPER_HID =           "HID:" + (HID + 19);
   final String CHKATTENDEES_HID =            "HID:" + (HID + 20);
   final String CHKOBSERVERS_HID =            "HID:" + (HID + 21);
   final String CHKRESOURCEPERSONS_HID =      "HID:" + (HID + 22);

   final String TXTTEMPLATENAME_HID =         "HID:" + (HID + 23);
   final String TXTTEMPLATEPATH_HID =         "HID:" + (HID + 24);
   final String BTNTEMPLATEPATH_HID =         "HID:" + (HID + 25);

   final String OPTCREATEAGENDA_HID =         "HID:" + (HID + 26);
   final String OPTMAKECHANGES_HID =          "HID:" + (HID + 27);

   final String BTNINSERT_HID =               "HID:" + (HID + 28);
   final String BTNREMOVE_HID =               "HID:" + (HID + 29);
   final String BTNUP_HID =                   "HID:" + (HID + 30);
   final String BTNDOWN_HID =                 "HID:" + (HID + 31);

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
  