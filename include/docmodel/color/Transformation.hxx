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
#include <o3tl/hash_combine.hxx>

namespace model
{
/** Color transformation type */
enum class TransformationType
{
    Undefined,
    Red,
    RedMod,
    RedOff,
    Green,
    GreenMod,
    GreenOff,
    Blue,
    BlueMod,
    BlueOff,
    Alpha,
    AlphaMod,
    AlphaOff,
    Hue,
    HueMod,
    HueOff,
    Sat,
    SatMod,
    SatOff,
    Lum,
    LumMod,
    LumOff,
    Shade,
    Tint,
    Gray,
    Comp,
    Inv,
    Gamma,
    InvGamma
};

/** Definition of a color transformation.
 *
 * This just defines how a color should be transformed (changed). The
 * type defines what kind of transformation should occur and the value
 * defines by how much.
 */
struct DOCMODEL_DLLPUBLIC Transformation
{
    TransformationType meType = TransformationType::Undefined;

    sal_Int16 mnValue = 0; /// percentage value -10000 to +10000

    bool operator==(const Transformation& rTransformation) const
    {
        return meType == rTransformation.meType && mnValue == rTransformation.mnValue;
    }

    std::size_t getHash() const
    {
        std::size_t seed = 0;
        o3tl::hash_combine(seed, meType);
        o3tl::hash_combine(seed, mnValue);
        return seed;
    }
};

} // end of namespace model

namespace std
{
template <> struct hash<model::Transformation>
{
    std::size_t operator()(model::Transformation const& rTransformation) const
    {
        return rTransformation.getHash();
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
