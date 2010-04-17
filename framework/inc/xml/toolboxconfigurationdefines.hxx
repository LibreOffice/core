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
#ifndef __FRAMEWORK_XML_TOOLBOXCONFIGURATIONDEFINES_HXX_
#define __FRAMEWORK_XML_TOOLBOXCONFIGURATIONDEFINES_HXX_

#define XMLNS_TOOLBAR               "http://openoffice.org/2001/toolbar"
#define XMLNS_XLINK                 "http://www.w3.org/1999/xlink"
#define XMLNS_TOOLBAR_PREFIX        "toolbar:"
#define XMLNS_XLINK_PREFIX          "xlink:"

#define XMLNS_FILTER_SEPARATOR      "^"

#define ELEMENT_TOOLBAR             "toolbar"
#define ELEMENT_TOOLBARITEM         "toolbaritem"
#define ELEMENT_TOOLBARSPACE        "toolbarspace"
#define ELEMENT_TOOLBARBREAK        "toolbarbreak"
#define ELEMENT_TOOLBARSEPARATOR    "toolbarseparator"

#define ELEMENT_TOOLBARLAYOUTS      "toolbarlayouts"
#define ELEMENT_TOOLBARLAYOUT       "toolbarlayout"
#define ELEMENT_TOOLBARCONFIGITEMS  "toolbarconfigitems"
#define ELEMENT_TOOLBARCONFIGITEM   "toolbarconfigitem"

#define ATTRIBUTE_UINAME            "uiname"

#define ATTRIBUTE_BITMAP            "bitmap"
#define ATTRIBUTE_TEXT              "text"
#define ATTRIBUTE_URL               "href"
#define ATTRIBUTE_ITEMBITS          "property"
#define ATTRIBUTE_VISIBLE           "visible"
#define ATTRIBUTE_WIDTH             "width"
#define ATTRIBUTE_USER              "userdefined"
#define ATTRIBUTE_HELPID            "helpid"
#define ATTRIBUTE_TOOLTIP           "tooltip"
#define ATTRIBUTE_ITEMSTYLE         "style"

#define ATTRIBUTE_ID                "id"
#define ATTRIBUTE_FLOATINGPOSLEFT   "floatingposleft"
#define ATTRIBUTE_FLOATINGPOSTOP    "floatingpostop"
#define ATTRIBUTE_TOOLBARNAME       "toolbarname"
#define ATTRIBUTE_CONTEXT           "context"
#define ATTRIBUTE_FLOATINGLINES     "floatinglines"
#define ATTRIBUTE_DOCKINGLINES      "dockinglines"
#define ATTRIBUTE_ALIGN             "align"
#define ATTRIBUTE_FLOATING          "floating"
#define ATTRIBUTE_BUTTONTYPE        "style"
#define ATTRIBUTE_USERDEFNAME       "userdefname"

#define ELEMENT_NS_TOOLBAR          "toolbar:toolbar"
#define ELEMENT_NS_TOOLBARITEM      "toolbar:toolbaritem"
#define ELEMENT_NS_TOOLBARSPACE     "toolbar:toolbarspace"
#define ELEMENT_NS_TOOLBARBREAK     "toolbar:toolbarbreak"
#define ELEMENT_NS_TOOLBARSEPARATOR "toolbar:toolbarseparator"

#define ELEMENT_NS_TOOLBARLAYOUTS       "toolbar:toolbarlayouts"
#define ELEMENT_NS_TOOLBARLAYOUT        "toolbar:toolbarlayout"
#define ELEMENT_NS_TOOLBARCONFIGITEMS   "toolbar:toolbarconfigitems"
#define ELEMENT_NS_TOOLBARCONFIGITEM    "toolbar:toolbarconfigitem"

#define ATTRIBUTE_XMLNS_TOOLBAR     "xmlns:toolbar"
#define ATTRIBUTE_XMLNS_XLINK       "xmlns:xlink"

#define ATTRIBUTE_TYPE_CDATA        "CDATA"

#define ATTRIBUTE_BOOLEAN_TRUE      "true"
#define ATTRIBUTE_BOOLEAN_FALSE     "false"

#define ATTRIBUTE_ALIGN_LEFT        "left"
#define ATTRIBUTE_ALIGN_RIGHT       "right"
#define ATTRIBUTE_ALIGN_TOP         "top"
#define ATTRIBUTE_ALIGN_BOTTOM      "bottom"

#define ATTRIBUTE_STYLE_TEXT        "text"
#define ATTRIBUTE_STYLE_SYMBOL      "symbol"
#define ATTRIBUTE_STYLE_SYMBOLTEXT  "symboltext"
#define ATTRIBUTE_STYLE_SYMBOLPLUSTEXT  "symbol+text"

#define ATTRIBUTE_ITEMSTYLE_RADIO           "radio"
#define ATTRIBUTE_ITEMSTYLE_AUTO            "auto"
#define ATTRIBUTE_ITEMSTYLE_LEFT            "left"
#define ATTRIBUTE_ITEMSTYLE_AUTOSIZE        "autosize"
#define ATTRIBUTE_ITEMSTYLE_DROPDOWN        "dropdown"
#define ATTRIBUTE_ITEMSTYLE_REPEAT          "repeat"
#define ATTRIBUTE_ITEMSTYLE_TEXT    "text"
#define ATTRIBUTE_ITEMSTYLE_DROPDOWNONLY    "dropdownonly"
#define ATTRIBUTE_ITEMSTYLE_IMAGE    "image"

#endif // __FRAMEWORK_XML_TOOLBOXCONFIGURATIONDEFINES_HXX_
