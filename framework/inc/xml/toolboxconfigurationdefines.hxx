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

#include <rtl/ustring.hxx>

#define XMLNS_TOOLBAR               "http://openoffice.org/2001/toolbar"
#define XMLNS_XLINK                 "http://www.w3.org/1999/xlink"
inline constexpr OUString XMLNS_TOOLBAR_PREFIX = u"toolbar:"_ustr;
#define XMLNS_XLINK_PREFIX          "xlink:"

#define XMLNS_FILTER_SEPARATOR      "^"

inline constexpr OUString ATTRIBUTE_UINAME = u"uiname"_ustr;
inline constexpr OUString ATTRIBUTE_TEXT = u"text"_ustr;
inline constexpr OUString ATTRIBUTE_URL = u"href"_ustr;
inline constexpr OUString ATTRIBUTE_VISIBLE = u"visible"_ustr;
inline constexpr OUString ATTRIBUTE_ITEMSTYLE = u"style"_ustr;

inline constexpr OUString ELEMENT_NS_TOOLBAR = u"toolbar:toolbar"_ustr;
inline constexpr OUString ELEMENT_NS_TOOLBARITEM = u"toolbar:toolbaritem"_ustr;
inline constexpr OUString ELEMENT_NS_TOOLBARSPACE = u"toolbar:toolbarspace"_ustr;
inline constexpr OUString ELEMENT_NS_TOOLBARBREAK = u"toolbar:toolbarbreak"_ustr;
inline constexpr OUString ELEMENT_NS_TOOLBARSEPARATOR = u"toolbar:toolbarseparator"_ustr;

inline constexpr OUString ATTRIBUTE_XMLNS_TOOLBAR = u"xmlns:toolbar"_ustr;
inline constexpr OUString ATTRIBUTE_XMLNS_XLINK = u"xmlns:xlink"_ustr;

#define ATTRIBUTE_BOOLEAN_TRUE      "true"
inline constexpr OUString ATTRIBUTE_BOOLEAN_FALSE  = u"false"_ustr;

inline constexpr OUString ATTRIBUTE_ITEMSTYLE_RADIO = u"radio"_ustr;
inline constexpr OUString ATTRIBUTE_ITEMSTYLE_AUTO = u"auto"_ustr;
inline constexpr OUString ATTRIBUTE_ITEMSTYLE_LEFT = u"left"_ustr;
inline constexpr OUString ATTRIBUTE_ITEMSTYLE_AUTOSIZE = u"autosize"_ustr;
inline constexpr OUString ATTRIBUTE_ITEMSTYLE_DROPDOWN = u"dropdown"_ustr;
inline constexpr OUString ATTRIBUTE_ITEMSTYLE_REPEAT = u"repeat"_ustr;
inline constexpr OUString ATTRIBUTE_ITEMSTYLE_TEXT = u"text"_ustr;
inline constexpr OUString ATTRIBUTE_ITEMSTYLE_DROPDOWNONLY = u"dropdownonly"_ustr;
inline constexpr OUString ATTRIBUTE_ITEMSTYLE_IMAGE = u"image"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
