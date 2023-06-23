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

#define CELL_BORDER_PROPERTIES \
    { SC_UNONAME_BOTTBORDER, ATTR_BORDER, cppu::UnoType<table::BorderLine>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS }, \
    { SC_UNONAME_BOTTBORDER2, ATTR_BORDER, cppu::UnoType<table::BorderLine2>::get(), 0, BOTTOM_BORDER | CONVERT_TWIPS }, \
    { SC_UNONAME_LEFTBORDER, ATTR_BORDER, cppu::UnoType<table::BorderLine>::get(), 0, LEFT_BORDER | CONVERT_TWIPS }, \
    { SC_UNONAME_LEFTBORDER2, ATTR_BORDER, cppu::UnoType<table::BorderLine2>::get(), 0, LEFT_BORDER | CONVERT_TWIPS }, \
    { SC_UNONAME_RIGHTBORDER, ATTR_BORDER, cppu::UnoType<table::BorderLine>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS }, \
    { SC_UNONAME_RIGHTBORDER2, ATTR_BORDER, cppu::UnoType<table::BorderLine2>::get(), 0, RIGHT_BORDER | CONVERT_TWIPS }, \
    { SC_UNONAME_TOPBORDER, ATTR_BORDER, cppu::UnoType<table::BorderLine>::get(), 0, TOP_BORDER | CONVERT_TWIPS }, \
    { SC_UNONAME_TOPBORDER2, ATTR_BORDER, cppu::UnoType<table::BorderLine2>::get(), 0, TOP_BORDER | CONVERT_TWIPS }, \
    { SC_UNONAME_BOTTOM_BORDER_COMPLEX_COLOR, ATTR_BORDER, cppu::UnoType<util::XComplexColor>::get(), 0, MID_BORDER_BOTTOM_COLOR }, \
    { SC_UNONAME_LEFT_BORDER_COMPLEX_COLOR, ATTR_BORDER, cppu::UnoType<util::XComplexColor>::get(), 0, MID_BORDER_LEFT_COLOR }, \
    { SC_UNONAME_RIGHT_BORDER_COMPLEX_COLOR, ATTR_BORDER, cppu::UnoType<util::XComplexColor>::get(), 0, MID_BORDER_RIGHT_COLOR }, \
    { SC_UNONAME_TOP_BORDER_COMPLEX_COLOR, ATTR_BORDER, cppu::UnoType<util::XComplexColor>::get(), 0, MID_BORDER_TOP_COLOR }, \

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
