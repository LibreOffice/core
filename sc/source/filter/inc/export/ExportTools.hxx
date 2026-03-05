/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <docmodel/color/ComplexColor.hxx>
#include <sax/fshelper.hxx>

#include <string_view>

namespace oox::xls
{
void writeComplexColor(sax_fastparser::FSHelperPtr& pFS, sal_Int32 nElement,
                       model::ComplexColor const& rComplexColor, Color const& rColor);
void writeComplexColor(sax_fastparser::FSHelperPtr& pFS, sal_Int32 nElement,
                       model::ComplexColor const& rComplexColor);

/** Writes an OOXML dateGroupItem element from a date string.
    The date string may be partial: "YYYY", "YYYY-MM", or "YYYY-MM-DD".
    The dateTimeGrouping attribute is determined by the number of date components.
    nElement is the fully-qualified element token (e.g. XML_dateGroupItem or
    FSNS(XML_x, XML_dateGroupItem) depending on the stream's namespace context). */
void writeDateGroupItem(sax_fastparser::FSHelperPtr const& pStream, sal_Int32 nElement,
                        std::string_view aDateString);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
