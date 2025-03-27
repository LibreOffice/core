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

namespace oox::xls
{
void writeComplexColor(const sax_fastparser::FSHelperPtr& pFS, sal_Int32 nElement,
                       model::ComplexColor const& rComplexColor, Color const& rColor);
void writeComplexColor(const sax_fastparser::FSHelperPtr& pFS, sal_Int32 nElement,
                       model::ComplexColor const& rComplexColor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
