/*************************************************************************
 *
 *  $RCSfile: CGAgenda.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $  $Date: 2004-11-27 09:04:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */
package com.sun.star.wizards.agenda;


import com.sun.star.wizards.common.ConfigGroup;
import com.sun.star.wizards.common.ConfigSet;

/**
 * CGAgenda means Configuration Group Agenda.
 * This object encapsulates a OOo configuration group, used
 * to save the state of the agenda wizard for reloading uppon
 * next start.
 */
public class CGAgenda extends ConfigGroup {

    /** step 1 - design*/
    public int cp_AgendaType;
    /** step 1 - include minutes*/
    public boolean cp_IncludeMinutes;

    /** step 2  - title */
    public String cp_Title = "";
    /** step 2  - date */
    public String cp_Date;
    /** step 2  - time */
    public String cp_Time;
    /** step 2  - location */
    public String cp_Location = "";

    /** step 3  - show meeting type */
    public boolean cp_ShowMeetingType;
    /** step 3  - show read */
    public boolean cp_ShowRead;
    /** step 3  - show bring */
    public boolean cp_ShowBring;
    /** step 3  - show notes */
    public boolean cp_ShowNotes;

    /** step 4 - show called by */
    public boolean cp_ShowCalledBy;
    /** step 4 - show facilitator */
    public boolean cp_ShowFacilitator;
    /** step 4 - show notes taker */
    public boolean cp_ShowNotetaker;
    /** step 4 - show time keeper */
    public boolean cp_ShowTimekeeper;
    /** step 4 - show attendees */
    public boolean cp_ShowAttendees;
    /** step 4 - show observers */
    public boolean cp_ShowObservers;
    /** step 4 - show resource persons */
    public boolean cp_ShowResourcePersons;

    /** page 6 - template title   */
    public String cp_TemplateName;
    /** page 6 - template url    */
    public String cp_TemplatePath;

    /** page 6 - how to proceed  */
    public int cp_ProceedMethod;

    /** page 5 - topics ( a set )*/
    public ConfigSet cp_Topics = new ConfigSet(CGTopic.class);

}
