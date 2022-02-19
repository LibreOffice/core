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

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"

namespace chart::TitleProperties
{
enum
{
    PROP_TITLE_PARA_ADJUST = FAST_PROPERTY_ID_START_TITLE_PROP,
    PROP_TITLE_PARA_LAST_LINE_ADJUST,
    PROP_TITLE_PARA_LEFT_MARGIN,
    PROP_TITLE_PARA_RIGHT_MARGIN,
    PROP_TITLE_PARA_TOP_MARGIN,
    PROP_TITLE_PARA_BOTTOM_MARGIN,
    PROP_TITLE_PARA_IS_HYPHENATION,
    PROP_TITLE_VISIBLE,

    PROP_TITLE_TEXT_ROTATION,
    PROP_TITLE_TEXT_STACKED,
    PROP_TITLE_REL_POS,

    PROP_TITLE_REF_PAGE_SIZE
};

} // namespace chart::LineProperties
