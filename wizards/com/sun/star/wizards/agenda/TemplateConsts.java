/*************************************************************************
 *
 *  $RCSfile: TemplateConsts.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $  $Date: 2004-09-08 14:00:13 $
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

    public final static String SECTION_ITEMS = "AGENDA_ITEMS";
    public final static String SECTION_TOPICS = "AGENDA_TOPICS";
    public final static String SECTION_MINUTES_ALL = "MINUTES_ALL";
    public final static String SECTION_MINUTES = "MINUTES";

    //public final static String AGENDA_ITEM = "AGENDA_ITEM";

    /**
     * taged headings and names.
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
     * headings styles
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
     * topic...
     */
    public final static String FILLIN_TOPIC_NUMBER = "<num>";
    public final static String FILLIN_TOPIC_TOPIC = "<topic>";
    public final static String FILLIN_TOPIC_RESPONSIBLE = "<responsible>";
    public final static String FILLIN_TOPIC_TIME = "<topic-time>";

    /**
     * Minutes...
     */
    public final static String FILLIN_MINUTES_TITLE = "<minutes-title>";
    public final static String FILLIN_MINUTES_LOCATION = "<minutes-location>";
    public final static String FILLIN_MINUTES_DATE = "<minutes-date>";
    public final static String FILLIN_MINUTES_TIME = "<minutes-time>";

    public final static String FILLIN_MINUTE_NUM = "<mnum>";
    public final static String FILLIN_MINUTE_TOPIC = "<mtopic>";
    public final static String FILLIN_MINUTE_RESPONSIBLE = "<mresponsible>";
    public final static String FILLIN_MINUTE_TIME = "<mtime>";


}
