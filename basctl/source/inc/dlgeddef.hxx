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

#include <svx/svdobjkind.hxx>

namespace basctl
{

// control properties
#define DLGED_PROP_BACKGROUNDCOLOR   "BackgroundColor"
inline constexpr OUStringLiteral DLGED_PROP_DROPDOWN = u"Dropdown";
inline constexpr OUStringLiteral DLGED_PROP_FORMATSSUPPLIER = u"FormatsSupplier";
inline constexpr OUStringLiteral DLGED_PROP_HEIGHT = u"Height";
inline constexpr OUStringLiteral DLGED_PROP_LABEL = u"Label";
inline constexpr OUStringLiteral DLGED_PROP_NAME = u"Name";
inline constexpr OUStringLiteral DLGED_PROP_ORIENTATION = u"Orientation";
inline constexpr OUStringLiteral DLGED_PROP_POSITIONX = u"PositionX";
inline constexpr OUStringLiteral DLGED_PROP_POSITIONY = u"PositionY";
inline constexpr OUStringLiteral DLGED_PROP_STEP = u"Step";
inline constexpr OUStringLiteral DLGED_PROP_TABINDEX = u"TabIndex";
#define DLGED_PROP_TEXTCOLOR         "TextColor"
#define DLGED_PROP_TEXTLINECOLOR     "TextLineColor"
inline constexpr OUStringLiteral DLGED_PROP_WIDTH = u"Width";
inline constexpr OUStringLiteral DLGED_PROP_DECORATION = u"Decoration";


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
