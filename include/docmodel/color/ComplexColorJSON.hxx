/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <docmodel/dllapi.h>
#include <docmodel/color/ComplexColor.hxx>

namespace tools
{
class JsonWriter;
}

namespace model::color
{
DOCMODEL_DLLPUBLIC OString convertToJSON(model::ComplexColor const& rComplexColor);
DOCMODEL_DLLPUBLIC void convertToJSONTree(tools::JsonWriter& rTree,
                                          model::ComplexColor const& rComplexColor);
DOCMODEL_DLLPUBLIC bool convertFromJSON(OString const& rJsonString,
                                        model::ComplexColor& rComplexColor);

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
