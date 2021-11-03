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

//  conversion programmatic <-> display (visible) name
//  currently, the core always has the visible names
//  the api is required to use programmatic names for default styles
//  these programmatic names must never change!

inline constexpr OUStringLiteral SC_STYLE_PROG_STANDARD = u"Default";
inline constexpr OUStringLiteral SC_STYLE_PROG_RESULT = u"Result";
inline constexpr OUStringLiteral SC_STYLE_PROG_RESULT1 = u"Result2";
inline constexpr OUStringLiteral SC_STYLE_PROG_HEADING = u"Heading";
inline constexpr OUStringLiteral SC_STYLE_PROG_HEADING1 = u"Heading1";
inline constexpr OUStringLiteral SC_STYLE_PROG_REPORT = u"Report";

inline constexpr OUStringLiteral SC_PIVOT_STYLE_PROG_INNER = u"Pivot Table Value";
inline constexpr OUStringLiteral SC_PIVOT_STYLE_PROG_RESULT = u"Pivot Table Result";
inline constexpr OUStringLiteral SC_PIVOT_STYLE_PROG_CATEGORY = u"Pivot Table Category";
inline constexpr OUStringLiteral SC_PIVOT_STYLE_PROG_TITLE = u"Pivot Table Title";
inline constexpr OUStringLiteral SC_PIVOT_STYLE_PROG_FIELDNAME = u"Pivot Table Field";
inline constexpr OUStringLiteral SC_PIVOT_STYLE_PROG_TOP = u"Pivot Table Corner";

enum class SfxStyleFamily;

class ScStyleNameConversion
{
public:
    static OUString DisplayToProgrammaticName(const OUString& rDispName, SfxStyleFamily nType);
    static OUString ProgrammaticToDisplayName(const OUString& rProgName, SfxStyleFamily nType);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
