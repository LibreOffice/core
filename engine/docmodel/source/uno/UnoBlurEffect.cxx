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

#include <docmodel/uno/UnoBlurEffect.hxx>
#include <UnoEffect.hxx>

using namespace css;
using namespace ::cpo;

namespace model::effect
{
uno::Reference<util::XBlurEffect> createXBlurEffect(model::BlurEffect const& rEffect)
{
    return new UnoEffect<util::XBlurEffect, model::BlurEffect>(rEffect);
}

std::optional<model::BlurEffect>
getFromXBlurEffect(uno::Reference<util::XBlurEffect> const& rxEffect)
{
    return getEffect<util::XBlurEffect, model::BlurEffect>(rxEffect);
}

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
