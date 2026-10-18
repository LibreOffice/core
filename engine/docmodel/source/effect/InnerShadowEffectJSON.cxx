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

#include <docmodel/effect/InnerShadowEffectJSON.hxx>
#include <EffectJSONTools.hxx>

using boost::property_tree::ptree;

namespace model::effect
{
void convertToJSONTree(tools::JsonWriter& rTree, model::InnerShadowEffect const& rEffect)
{
    putOffset(rTree, rEffect);
    putColor(rTree, rEffect.maColor);
}

OString convertToJSON(model::InnerShadowEffect const& rEffect)
{
    tools::JsonWriter aTree;
    convertToJSONTree(aTree, rEffect);
    return aTree.finishAndGetAsOString();
}

bool convertFromJSON(OString const& rJSON, model::InnerShadowEffect& rEffect)
{
    return readEffect(rJSON, rEffect, [](ptree const& rTree, model::InnerShadowEffect& rRead) {
        readOffset(rTree, rRead);
        return readColor(rTree, rRead.maColor);
    });
}

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
