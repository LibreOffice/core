/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>

template <typename EnumT> struct SvXMLEnumMapEntry;

extern SvXMLEnumMapEntry<css::drawing::FillStyle> const aXML_FillStyle_EnumMap[];
extern SvXMLEnumMapEntry<css::drawing::RectanglePoint> const aXML_RefPoint_EnumMap[];
extern SvXMLEnumMapEntry<css::drawing::BitmapMode> const aXML_BitmapMode_EnumMap[];

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
