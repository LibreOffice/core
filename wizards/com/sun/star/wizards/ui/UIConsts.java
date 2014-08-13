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
    int RID_IMG_FORM = 1100;
    Integer INVISIBLESTEP = 99;
    String INFOIMAGEURL = "private:resource/dbu/image/19205";
    String INFOIMAGEURL_HC = "private:resource/dbu/image/19230";
    /**
     * The tabindex of the navigation buttons in a wizard must be assigned a very
     * high tabindex because on every step their taborder must appear at the end
     */
    short SOFIRSTWIZARDNAVITABINDEX = 30000;
    Integer INTEGER_12 = 12;
    /**Steps of the QueryWizard
     *
     */
    int SOGROUPSELECTIONPAGE = 5;
    int SOGROUPFILTERPAGE = 6;
    int SOTITLESPAGE = 7;
    Integer[] INTEGERS = new Integer[]
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    };

    public interface CONTROLTYPE
    {

        int BUTTON = 1;
        int IMAGECONTROL = 2;
        int LISTBOX = 3;
        int COMBOBOX = 4;
        int CHECKBOX = 5;
        int RADIOBUTTON = 6;
        int DATEFIELD = 7;
        int EDITCONTROL = 8;
        int FIXEDLINE = 10;
        int FIXEDTEXT = 11;
        int FORMATTEDFIELD = 12;
        int HYPERTEXT = 14;
        int NUMERICFIELD = 15;
        int PATTERNFIELD = 16;
        int PROGRESSBAR = 17;
        int ROADMAP = 18;
        int SCROLLBAR = 19;
        int TIMEFIELD = 20;
        int CURRENCYFIELD = 21;
        int UNKNOWN = -1;
    }
}
