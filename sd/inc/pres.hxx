/*************************************************************************
 *
 *  $RCSfile: pres.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2001-04-04 16:35:12 $
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
 *
 ************************************************************************/
#ifndef _PRESENTATION_HXX
#define _PRESENTATION_HXX

enum AutoLayout
{
    AUTOLAYOUT_TITLE,
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
    AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART
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

