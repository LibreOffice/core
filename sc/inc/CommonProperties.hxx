/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#define CELL_BACKGROUND_COLOR_PROPERTIES \
    { SC_UNONAME_CELLBACK, ATTR_BACKGROUND, cppu::UnoType<sal_Int32>::get(), 0, MID_BACK_COLOR }, \
    { SC_UNONAME_CELL_BACKGROUND_COMPLEX_COLOR, ATTR_BACKGROUND, cppu::UnoType<css::util::XComplexColor>::get(), 0, MID_BACKGROUND_COMPLEX_COLOR }, \

#define CHAR_COLOR_PROPERTIES \
    { SC_UNONAME_CCOLOR, ATTR_FONT_COLOR, cppu::UnoType<sal_Int32>::get(), 0, MID_COLOR_RGB }, \
    { SC_UNONAME_CHAR_COMPLEX_COLOR, ATTR_FONT_COLOR, cppu::UnoType<css::util::XComplexColor>::get(), 0, MID_COMPLEX_COLOR }, \

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
