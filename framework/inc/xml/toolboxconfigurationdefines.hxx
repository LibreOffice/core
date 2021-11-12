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
#pragma once

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

#define ATTRIBUTE_UINAME            "uiname"

#define ATTRIBUTE_TEXT              "text"
#define ATTRIBUTE_URL               "href"
#define ATTRIBUTE_VISIBLE           "visible"
#define ATTRIBUTE_ITEMSTYLE         "style"

#define ELEMENT_NS_TOOLBAR          "toolbar:toolbar"
#define ELEMENT_NS_TOOLBARITEM      "toolbar:toolbaritem"
#define ELEMENT_NS_TOOLBARSPACE     "toolbar:toolbarspace"
#define ELEMENT_NS_TOOLBARBREAK     "toolbar:toolbarbreak"
#define ELEMENT_NS_TOOLBARSEPARATOR "toolbar:toolbarseparator"

#define ATTRIBUTE_XMLNS_TOOLBAR     "xmlns:toolbar"
#define ATTRIBUTE_XMLNS_XLINK       "xmlns:xlink"

#define ATTRIBUTE_TYPE_CDATA        "CDATA"

#define ATTRIBUTE_BOOLEAN_TRUE      "true"
#define ATTRIBUTE_BOOLEAN_FALSE     "false"

constexpr OUStringLiteral ATTRIBUTE_ITEMSTYLE_RADIO = u"radio";
constexpr OUStringLiteral ATTRIBUTE_ITEMSTYLE_AUTO = u"auto";
constexpr OUStringLiteral ATTRIBUTE_ITEMSTYLE_LEFT = u"left";
constexpr OUStringLiteral ATTRIBUTE_ITEMSTYLE_AUTOSIZE = u"autosize";
constexpr OUStringLiteral ATTRIBUTE_ITEMSTYLE_DROPDOWN = u"dropdown";
constexpr OUStringLiteral ATTRIBUTE_ITEMSTYLE_REPEAT = u"repeat";
constexpr OUStringLiteral ATTRIBUTE_ITEMSTYLE_TEXT = u"text";
constexpr OUStringLiteral ATTRIBUTE_ITEMSTYLE_DROPDOWNONLY = u"dropdownonly";
constexpr OUStringLiteral ATTRIBUTE_ITEMSTYLE_IMAGE = u"image";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
