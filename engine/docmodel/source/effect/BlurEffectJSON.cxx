/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <docmodel/effect/BlurEffectJSON.hxx>
#include <EffectJSONTools.hxx>

using boost::property_tree::ptree;

namespace model::effect
{
void convertToJSONTree(tools::JsonWriter& rTree, model::BlurEffect const& rEffect)
{
    rTree.put("Radius", rEffect.maRadius.as_emu<sal_Int64>());
    rTree.put("Grow", rEffect.mbGrow);
}

OString convertToJSON(model::BlurEffect const& rEffect)
{
    tools::JsonWriter aTree;
    convertToJSONTree(aTree, rEffect);
    return aTree.finishAndGetAsOString();
}

bool convertFromJSON(OString const& rJSON, model::BlurEffect& rEffect)
{
    return readEffect(rJSON, rEffect, [](ptree const& rTree, model::BlurEffect& rRead) {
        readLength(rTree, "Radius", rRead.maRadius);
        rRead.mbGrow = readValue(rTree, "Grow", rRead.mbGrow);
        return true;
    });
}

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
