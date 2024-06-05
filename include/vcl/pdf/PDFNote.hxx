/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <vcl/dllapi.h>
#include <vcl/pdf/PDFAnnotationSubType.hxx>

#include <com/sun/star/util/DateTime.hpp>

#include <vector>

namespace vcl::pdf
{
struct PDFNote
{
    vcl::pdf::PDFAnnotationSubType meType = vcl::pdf::PDFAnnotationSubType::Text;

    OUString maTitle; // optional title for the popup containing the note
    OUString maContents; // contents of the note
    css::util::DateTime maModificationDate;
    std::vector<basegfx::B2DPolygon> maPolygons;
    Color maAnnotationColor;
    Color maInteriorColor;
    float mfWidth = 0.0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
