#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
class UIConsts():

    RID_COMMON = 500
    RID_DB_COMMON = 1000
    RID_FORM = 2200
    RID_QUERY = 2300
    RID_REPORT = 2400
    RID_TABLE = 2500
    RID_IMG_REPORT = 1000
    RID_IMG_FORM = 1100
    RID_IMG_WEB = 1200
    INVISIBLESTEP = 99
    INFOIMAGEURL = "private:resource/dbu/image/19205"

    '''
    The tabindex of the navigation buttons in a wizard must be assigned a very
    high tabindex because on every step their taborder must appear at the end
    '''
    SOFIRSTWIZARDNAVITABINDEX = 30000

    # Steps of the QueryWizard

    SOFIELDSELECTIONPAGE = 1
    SOSORTINGPAGE = 2
    SOFILTERPAGE = 3
    SOAGGREGATEPAGE = 4
    SOGROUPSELECTIONPAGE = 5
    SOGROUPFILTERPAGE = 6
    SOTITLESPAGE = 7
    SOSUMMARYPAGE = 8

    class CONTROLTYPE():

        BUTTON = 1
        IMAGECONTROL = 2
        LISTBOX = 3
        COMBOBOX = 4
        CHECKBOX = 5
        RADIOBUTTON = 6
        DATEFIELD = 7
        EDITCONTROL = 8
        FILECONTROL = 9
        FIXEDLINE = 10
        FIXEDTEXT = 11
        FORMATTEDFIELD = 12
        GROUPBOX = 13
        HYPERTEXT = 14
        NUMERICFIELD = 15
        PATTERNFIELD = 16
        PROGRESSBAR = 17
        ROADMAP = 18
        SCROLLBAR = 19
        TIMEFIELD = 20
        CURRENCYFIELD = 21
        UNKNOWN = -1
