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
#ifndef _PRESENTATION_HXX
#define _PRESENTATION_HXX

enum PresObjKind
{
    PRESOBJ_NONE = 0,
    PRESOBJ_TITLE,
    PRESOBJ_OUTLINE,
    PRESOBJ_TEXT,
    PRESOBJ_GRAPHIC,
    PRESOBJ_OBJECT,
    PRESOBJ_CHART,
    PRESOBJ_ORGCHART,
    PRESOBJ_TABLE,
    PRESOBJ_IMAGE,
    PRESOBJ_PAGE,
    PRESOBJ_HANDOUT,
    PRESOBJ_NOTES,
    PRESOBJ_HEADER,
    PRESOBJ_FOOTER,
    PRESOBJ_DATETIME,
    PRESOBJ_SLIDENUMBER,
    PRESOBJ_CALC,
    PRESOBJ_MEDIA,

    PRESOBJ_MAX
};

enum AutoLayout
{
    // new layouts with enum/text/chart/org/clip merged to content
    AUTOLAYOUT__START = 0,
    AUTOLAYOUT_TITLE = 0,                       // Title Slide
    AUTOLAYOUT_TITLE_CONTENT = 1,               // Title, Content
    AUTOLAYOUT_TITLE_2CONTENT = 3,              // Title and 2 Content
    AUTOLAYOUT_TITLE_ONLY = 19,                 // Title Only
    AUTOLAYOUT_NONE = 20,                       // Blank Slide
    AUTOLAYOUT_ONLY_TEXT = 32,                  // Centered Text
    AUTOLAYOUT_TITLE_CONTENT_2CONTENT = 12,     // Title, Content and 2 Content
    AUTOLAYOUT_TITLE_2CONTENT_CONTENT = 15,     // Title, 2 Content and Content
    AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT = 16,// Title, 2 Content over Content
    AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT = 14, // Title, Content over Content
    AUTOLAYOUT_TITLE_4CONTENT = 18,             // Title, 4 Content
    AUTOLAYOUT_TITLE_6CONTENT = 34,             // Title, 6 Content
    AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT = 27, // Vertical Title, Vertical Content over Vertical Content
    AUTOLAYOUT_VTITLE_VCONTENT = 28,            // Vertical Title, Vertical Content over Vertical Content
    AUTOLAYOUT_TITLE_VCONTENT = 29,             // Title, Vertical Content
    AUTOLAYOUT_TITLE_2VTEXT = 30,               // Title, 2 Vertical Content

    // deprecated
    AUTOLAYOUT_ENUM = 1,
    AUTOLAYOUT_CHART = 2,
    AUTOLAYOUT_2TEXT = 3,
    AUTOLAYOUT_TEXTCHART = 4,
    AUTOLAYOUT_ORG = 5,
    AUTOLAYOUT_TEXTCLIP = 6,
    AUTOLAYOUT_CHARTTEXT = 7,
    AUTOLAYOUT_TAB = 8,
    AUTOLAYOUT_CLIPTEXT = 9,
    AUTOLAYOUT_TEXTOBJ = 10,
    AUTOLAYOUT_OBJ = 11,
    AUTOLAYOUT_TEXT2OBJ = 12,
    AUTOLAYOUT_OBJTEXT = 13,
    AUTOLAYOUT_OBJOVERTEXT = 14,
    AUTOLAYOUT_2OBJTEXT = 15,
    AUTOLAYOUT_2OBJOVERTEXT = 16,
    AUTOLAYOUT_TEXTOVEROBJ = 17,
    AUTOLAYOUT_4OBJ = 18,
    AUTOLAYOUT_ONLY_TITLE = 19,
    AUTOLAYOUT_NOTES = 21,
    AUTOLAYOUT_HANDOUT1 = 22,
    AUTOLAYOUT_HANDOUT2 = 23,
    AUTOLAYOUT_HANDOUT3 = 24,
    AUTOLAYOUT_HANDOUT4 = 25,
    AUTOLAYOUT_HANDOUT6 = 26,
    AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART = 27,
    AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE = 28,
    AUTOLAYOUT_TITLE_VERTICAL_OUTLINE = 29,
    AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART = 30,
    AUTOLAYOUT_HANDOUT9 = 31,
    AUTOLAYOUT_4CLIPART = 33,
    AUTOLAYOUT_6CLIPART = 34,
    AUTOLAYOUT__END
};

enum PageKind
{
    PK_STANDARD,
    PK_NOTES,
    PK_HANDOUT
};

enum EditMode
{
    EM_PAGE,
    EM_MASTERPAGE
};

enum DocumentType
{
    DOCUMENT_TYPE_IMPRESS,
    DOCUMENT_TYPE_DRAW
};

enum NavigatorDragType
{
    NAVIGATOR_DRAGTYPE_NONE,
    NAVIGATOR_DRAGTYPE_URL,
    NAVIGATOR_DRAGTYPE_LINK,
    NAVIGATOR_DRAGTYPE_EMBEDDED
};
#define NAVIGATOR_DRAGTYPE_COUNT 4

#endif  // _PRESENTATION_HXX

