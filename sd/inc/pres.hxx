/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pres.hxx,v $
 * $Revision: 1.9 $
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
    PRESOBJ_BACKGROUND,
    PRESOBJ_PAGE,
    PRESOBJ_HANDOUT,
    PRESOBJ_NOTES,
    PRESOBJ_HEADER,
    PRESOBJ_FOOTER,
    PRESOBJ_DATETIME,
    PRESOBJ_SLIDENUMBER,

    PRESOBJ_MAX
};

enum AutoLayout
{
    AUTOLAYOUT__START,
    AUTOLAYOUT_TITLE = AUTOLAYOUT__START,
    AUTOLAYOUT_ENUM,
    AUTOLAYOUT_CHART,
    AUTOLAYOUT_2TEXT,
    AUTOLAYOUT_TEXTCHART,
    AUTOLAYOUT_ORG,
    AUTOLAYOUT_TEXTCLIP,
    AUTOLAYOUT_CHARTTEXT,
    AUTOLAYOUT_TAB,
    AUTOLAYOUT_CLIPTEXT,
    AUTOLAYOUT_TEXTOBJ,
    AUTOLAYOUT_OBJ,
    AUTOLAYOUT_TEXT2OBJ,
    AUTOLAYOUT_OBJTEXT,
    AUTOLAYOUT_OBJOVERTEXT,
    AUTOLAYOUT_2OBJTEXT,
    AUTOLAYOUT_2OBJOVERTEXT,
    AUTOLAYOUT_TEXTOVEROBJ,
    AUTOLAYOUT_4OBJ,
    AUTOLAYOUT_ONLY_TITLE,
    AUTOLAYOUT_NONE,
    AUTOLAYOUT_NOTES,
    AUTOLAYOUT_HANDOUT1,
    AUTOLAYOUT_HANDOUT2,
    AUTOLAYOUT_HANDOUT3,
    AUTOLAYOUT_HANDOUT4,
    AUTOLAYOUT_HANDOUT6,
    AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART,
    AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE,
    AUTOLAYOUT_TITLE_VERTICAL_OUTLINE,
    AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART,
    AUTOLAYOUT_HANDOUT9,
    AUTOLAYOUT_ONLY_TEXT,
    AUTOLAYOUT_4CLIPART,
    AUTOLAYOUT_6CLIPART,
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

