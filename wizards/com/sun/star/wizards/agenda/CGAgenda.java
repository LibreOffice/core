/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.wizards.agenda;

import com.sun.star.wizards.common.ConfigGroup;
import com.sun.star.wizards.common.ConfigSet;
import com.sun.star.wizards.common.PropertyNames;

/**
 * CGAgenda means Configuration Group Agenda.
 * This object encapsulates a OOo configuration group, used
 * to save the state of the agenda wizard for reloading uppon
 * next start.
 */
public class CGAgenda extends ConfigGroup
{

    /** step 1 - design*/
    public int cp_AgendaType;
    /** step 1 - include minutes*/
    public boolean cp_IncludeMinutes;
    /** step 2  - title */
    public String cp_Title = PropertyNames.EMPTY_STRING;
    /** step 2  - date */
    public String cp_Date;
    /** step 2  - time */
    public String cp_Time;
    /** step 2  - location */
    public String cp_Location = PropertyNames.EMPTY_STRING;
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
