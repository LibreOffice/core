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

/**
 * @author rpiterman
 *
 */
public interface TemplateConsts
{

    public final static String FILLIN_TITLE = "<title>";
    public final static String FILLIN_DATE = "<date>";
    public final static String FILLIN_TIME = "<time>";
    public final static String FILLIN_LOCATION = "<location>";
    /**
     * section name <b>prefix</b> for sections that contain items.
     * this is also used as table name prefix, since each items section
     * must contain a table whos name is identical name to the section's name.
     */
    public final static String SECTION_ITEMS = "AGENDA_ITEMS";
    /**
     * the name of the section which contains the topics.
     */
    public final static String SECTION_TOPICS = "AGENDA_TOPICS";
    /**
     * the name of the parent minutes section.
     */
    public final static String SECTION_MINUTES_ALL = "MINUTES_ALL";
    /**
     * the name of the child nimutes section.
     * This section will be duplicated for each topic.
     */
    public final static String SECTION_MINUTES = "MINUTES";    //public final static String AGENDA_ITEM = "AGENDA_ITEM";
    /**
     * taged headings and names.
     * These will be searched in item tables (in the template) and will be
     * replaced with resource strings.
     *
     * headings...
     */
    public final static String FILLIN_MEETING_TYPE = "<meeting-type>";
    public final static String FILLIN_BRING = "<bring>";
    public final static String FILLIN_READ = "<read>";
    public final static String FILLIN_NOTES = "<notes>";
    /**
     * names...
     */
    public final static String FILLIN_CALLED_BY = "<called-by>";
    public final static String FILLIN_FACILITATOR = "<facilitator>";
    public final static String FILLIN_PARTICIPANTS = "<attendees>";
    public final static String FILLIN_NOTETAKER = "<notetaker>";
    public final static String FILLIN_TIMEKEEPER = "<timekeeper>";
    public final static String FILLIN_OBSERVERS = "<observers>";
    public final static String FILLIN_RESOURCE_PERSONS = "<resource-persons>";
    /**
     * Styles (paragraph styles) used for agenda items.
     * headings styles
     *
     */
    public final static String STYLE_MEETING_TYPE = "MeetingType";
    public final static String STYLE_BRING = "Bring";
    public final static String STYLE_READ = "Read";
    public final static String STYLE_NOTES = "Notes";
    /**
     * names styles
     */
    public final static String STYLE_CALLED_BY = "CalledBy";
    public final static String STYLE_FACILITATOR = "Facilitator";
    public final static String STYLE_PARTICIPANTS = "Attendees";
    public final static String STYLE_NOTETAKER = "Notetaker";
    public final static String STYLE_TIMEKEEPER = "Timekeeper";
    public final static String STYLE_OBSERVERS = "Observers";
    public final static String STYLE_RESOURCE_PERSONS = "ResourcePersons";
    /**
     * Styles (paragraph styles) used for the <b>text</b> of agenda items
     * The agenda wizard creates fill-in fields with the given styles...)
     *
     * headings fields styles
     */
    public final static String STYLE_MEETING_TYPE_TEXT = "MeetingTypeText";
    public final static String STYLE_BRING_TEXT = "BringText";
    public final static String STYLE_READ_TEXT = "ReadText";
    public final static String STYLE_NOTES_TEXT = "NotesText";
    /**
     * names field styles
     */
    public final static String STYLE_CALLED_BY_TEXT = "CalledByText";
    public final static String STYLE_FACILITATOR_TEXT = "FacilitatorText";
    public final static String STYLE_PARTICIPANTS_TEXT = "AttendeesText";
    public final static String STYLE_NOTETAKER_TEXT = "NotetakerText";
    public final static String STYLE_TIMEKEEPER_TEXT = "TimekeeperText";
    public final static String STYLE_OBSERVERS_TEXT = "ObserversText";
    public final static String STYLE_RESOURCE_PERSONS_TEXT = "ResourcePersonsText";
    /**
     * Fillins for the topic table.
     * These strings will be searched inside the topic table as
     * part of detecting its structure.
     */
    public final static String FILLIN_TOPIC_NUMBER = "<num>";
    public final static String FILLIN_TOPIC_TOPIC = "<topic>";
    public final static String FILLIN_TOPIC_RESPONSIBLE = "<responsible>";
    public final static String FILLIN_TOPIC_TIME = "<topic-time>";
    /**
     * fillins for minutes.
     * These will be searched in the minutes section and will be replaced
     * with the appropriate data.
     */
    public final static String FILLIN_MINUTES_TITLE = "<minutes-title>";
    public final static String FILLIN_MINUTES_LOCATION = "<minutes-location>";
    public final static String FILLIN_MINUTES_DATE = "<minutes-date>";
    public final static String FILLIN_MINUTES_TIME = "<minutes-time>";
    /**
     * Minutes-topic fillins
     * These will be searched in the minutes-child-section, and
     * will be replaced with topic data.
     */
    public final static String FILLIN_MINUTE_NUM = "<mnum>";
    public final static String FILLIN_MINUTE_TOPIC = "<mtopic>";
    public final static String FILLIN_MINUTE_RESPONSIBLE = "<mresponsible>";
    public final static String FILLIN_MINUTE_TIME = "<mtime>";

//
    public final static String FILLIN_DATETITLE = "#datetitle#";
    public final static String FILLIN_TIMETITLE = "#timetitle#";
    public final static String FILLIN_LOCATIONTITLE = "#locationtitle#";
    public final static String FILLIN_TOPICS = "#topics#";
    public final static String FILLIN_NUM = "#num.#";
    public final static String FILLIN_TOPIC = "#topicheader#";
    public final static String FILLIN_RESPONSIBLE = "#responsibleheader#";
    public final static String FILLIN_TIME_HEADER = "#timeheader#";
    public final static String FILLIN_ADDITIONAL_INFORMATION = "#additional-information#";
    public final static String FILLIN_MINUTES_FOR = "#minutes-for#";
    public final static String FILLIN_DISCUSSION = "#discussion#";
    public final static String FILLIN_CONCLUSION = "#conclusion#";
    public final static String FILLIN_TO_DO = "#to-do#";
    public final static String FILLIN_RESPONSIBLE_PARTY = "#responsible-party#";
    public final static String FILLIN_DEADLINE = "#deadline#";
//
}
