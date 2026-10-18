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

#include <docmodel/uno/UnoGlowEffect.hxx>
#include <UnoEffect.hxx>

using namespace css;
using namespace ::cpo;

namespace model::effect
{
uno::Reference<util::XGlowEffect> createXGlowEffect(model::GlowEffect const& rEffect)
{
    return new UnoEffect<util::XGlowEffect, model::GlowEffect>(rEffect);
}

std::optional<model::GlowEffect>
getFromXGlowEffect(uno::Reference<util::XGlowEffect> const& rxEffect)
{
    return getEffect<util::XGlowEffect, model::GlowEffect>(rxEffect);
}

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
