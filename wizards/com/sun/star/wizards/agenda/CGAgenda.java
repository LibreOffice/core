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
