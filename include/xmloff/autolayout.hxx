/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_XMLOFF_AUTOLAYOUT_HXX
#define INCLUDED_XMLOFF_AUTOLAYOUT_HXX

enum AutoLayout
{
    // new layouts with enum/text/chart/org/clip merged to content
    AUTOLAYOUT_START = 0,
    AUTOLAYOUT_TITLE = 0,                       ///< Title Slide
    AUTOLAYOUT_TITLE_CONTENT = 1,               ///< Title, Content
    AUTOLAYOUT_CHART = 2,
    AUTOLAYOUT_TITLE_2CONTENT = 3,              ///< Title and 2 Content
    AUTOLAYOUT_TEXTCHART = 4,
    AUTOLAYOUT_ORG = 5,
    AUTOLAYOUT_TEXTCLIP = 6,
    AUTOLAYOUT_CHARTTEXT = 7,
    AUTOLAYOUT_TAB = 8,
    AUTOLAYOUT_CLIPTEXT = 9,
    AUTOLAYOUT_TEXTOBJ = 10,
    AUTOLAYOUT_OBJ = 11,
    AUTOLAYOUT_TITLE_CONTENT_2CONTENT = 12,     ///< Title, Content and 2 Content
    AUTOLAYOUT_OBJTEXT = 13,
    AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT = 14, ///< Title, Content over Content
    AUTOLAYOUT_TITLE_2CONTENT_CONTENT = 15,     ///< Title, 2 Content and Content
    AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT = 16,///< Title, 2 Content over Content
    AUTOLAYOUT_TEXTOVEROBJ = 17,
    AUTOLAYOUT_TITLE_4CONTENT = 18,             ///< Title, 4 Content
    AUTOLAYOUT_TITLE_ONLY = 19,                 ///< Title Only
    AUTOLAYOUT_NONE = 20,                       ///< Blank Slide
    AUTOLAYOUT_NOTES = 21,
    AUTOLAYOUT_HANDOUT1 = 22,
    AUTOLAYOUT_HANDOUT2 = 23,
    AUTOLAYOUT_HANDOUT3 = 24,
    AUTOLAYOUT_HANDOUT4 = 25,
    AUTOLAYOUT_HANDOUT6 = 26,
    AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT = 27, ///< Vertical Title, Vertical Content over Vertical Content
    AUTOLAYOUT_VTITLE_VCONTENT = 28,            ///< Vertical Title, Vertical Content over Vertical Content
    AUTOLAYOUT_TITLE_VCONTENT = 29,             ///< Title, Vertical Content
    AUTOLAYOUT_TITLE_2VTEXT = 30,               ///< Title, 2 Vertical Content
    AUTOLAYOUT_HANDOUT9 = 31,
    AUTOLAYOUT_ONLY_TEXT = 32,                  ///< Centered Text
    AUTOLAYOUT_4CLIPART = 33,
    AUTOLAYOUT_TITLE_6CONTENT = 34,             ///< Title, 6 Content
    AUTOLAYOUT_END
};

#endif // INCLUDED_XMLOFF_AUTOLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
