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

#include <docmodel/effect/ReflectionEffectJSON.hxx>
#include <EffectJSONTools.hxx>

using boost::property_tree::ptree;

namespace model::effect
{
void convertToJSONTree(tools::JsonWriter& rTree, model::ReflectionEffect const& rEffect)
{
    putOffset(rTree, rEffect);
    putTransform(rTree, rEffect);
    rTree.put("StartAlpha", rEffect.mnStartAlpha);
    rTree.put("StartPosition", rEffect.mnStartPosition);
    rTree.put("EndAlpha", rEffect.mnEndAlpha);
    rTree.put("EndPosition", rEffect.mnEndPosition);
    rTree.put("FadeDirection", rEffect.mnFadeDirection.get());
}

OString convertToJSON(model::ReflectionEffect const& rEffect)
{
    tools::JsonWriter aTree;
    convertToJSONTree(aTree, rEffect);
    return aTree.finishAndGetAsOString();
}

bool convertFromJSON(OString const& rJSON, model::ReflectionEffect& rEffect)
{
    return readEffect(rJSON, rEffect, [](ptree const& rTree, model::ReflectionEffect& rRead) {
        readOffset(rTree, rRead);
        readTransform(rTree, rRead);
        rRead.mnStartAlpha = readValue(rTree, "StartAlpha", rRead.mnStartAlpha);
        rRead.mnStartPosition = readValue(rTree, "StartPosition", rRead.mnStartPosition);
        rRead.mnEndAlpha = readValue(rTree, "EndAlpha", rRead.mnEndAlpha);
        rRead.mnEndPosition = readValue(rTree, "EndPosition", rRead.mnEndPosition);
        readAngle(rTree, "FadeDirection", rRead.mnFadeDirection);
        return true;
    });
}

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
