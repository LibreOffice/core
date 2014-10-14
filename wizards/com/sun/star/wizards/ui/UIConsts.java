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

package com.sun.star.wizards.ui;

public interface UIConsts
{

    int RID_COMMON = 500;
    int RID_DB_COMMON = 1000;
    int RID_FORM = 2200;
    int RID_QUERY = 2300;
    int RID_REPORT = 2400;
    int RID_TABLE = 2600;
    int RID_IMG_REPORT = 1000;
    int RID_IMG_FORM = 1100;
    int RID_IMG_WEB = 1200;
    Integer INVISIBLESTEP = 99;
    String INFOIMAGEURL = "private:resource/dbu/image/19205";
    String INFOIMAGEURL_HC = "private:resource/dbu/image/19230";
    /**
     * The tabindex of the navigation buttons in a wizard must be assigned a very
     * high tabindex because on every step their taborder must appear at the end
     */
    short SOFIRSTWIZARDNAVITABINDEX = 30000;
    Integer INTEGER_8 = 8;
    Integer INTEGER_12 = 12;
    Integer INTEGER_14 = 14;
    Integer INTEGER_16 = 16;
    Integer INTEGER_40 = 40;
    Integer INTEGER_50 = 50;
    /**Steps of the QueryWizard
     *
     */
    int SOFIELDSELECTIONPAGE = 1;
    int SOSORTINGPAGE = 2;
    int SOFILTERPAGE = 3;
    int SOAGGREGATEPAGE = 4;
    int SOGROUPSELECTIONPAGE = 5;
    int SOGROUPFILTERPAGE = 6;
    int SOTITLESPAGE = 7;
    int SOSUMMARYPAGE = 8;
    Integer[] INTEGERS = new Integer[]
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    };

    class CONTROLTYPE
    {

        public static final int BUTTON = 1;
        public static final int IMAGECONTROL = 2;
        public static final int LISTBOX = 3;
        public static final int COMBOBOX = 4;
        public static final int CHECKBOX = 5;
        public static final int RADIOBUTTON = 6;
        public static final int DATEFIELD = 7;
        public static final int EDITCONTROL = 8;
        public static final int FILECONTROL = 9;
        public static final int FIXEDLINE = 10;
        public static final int FIXEDTEXT = 11;
        public static final int FORMATTEDFIELD = 12;
        public static final int GROUPBOX = 13;
        public static final int HYPERTEXT = 14;
        public static final int NUMERICFIELD = 15;
        public static final int PATTERNFIELD = 16;
        public static final int PROGRESSBAR = 17;
        public static final int ROADMAP = 18;
        public static final int SCROLLBAR = 19;
        public static final int TIMEFIELD = 20;
        public static final int CURRENCYFIELD = 21;
        public static final int UNKNOWN = -1;
    }
}
