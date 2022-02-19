/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"

namespace chart::LabelProperties
{
enum
{
    PROP_LABEL_BORDER_STYLE = FAST_PROPERTY_ID_START_LABEL_PROP,
    PROP_LABEL_BORDER_WIDTH,
    PROP_LABEL_BORDER_COLOR,
    PROP_LABEL_BORDER_TRANS,
    PROP_LABEL_FILL_STYLE,
    PROP_LABEL_FILL_COLOR,
    PROP_LABEL_FILL_BACKGROUND,
    PROP_LABEL_FILL_HATCH_NAME
};

} // namespace chart::LabelProperties
