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

#pragma once

#include <cppuhelper/implbase.hxx>

#include <optional>

namespace model::effect
{
/// Holds an effect for UNO.
template <class Interface, class Effect>
class UnoEffect final : public cppu::WeakImplHelper<Interface>
{
    Effect maEffect;

public:
    explicit UnoEffect(Effect const& rEffect)
        : maEffect(rEffect)
    {
    }

    Effect const& getEffect() const { return maEffect; }
};

/// The effect of an object that is a UnoEffect, or none for any other object.
template <class Interface, class Effect>
std::optional<Effect> getEffect(cpo::uno::Reference<Interface> const& rxEffect)
{
    auto const* pUnoEffect = dynamic_cast<UnoEffect<Interface, Effect> const*>(rxEffect.get());
    if (!pUnoEffect)
        return std::nullopt;
    return pUnoEffect->getEffect();
}

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
