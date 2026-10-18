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

#include <com/sun/star/util/XBlurEffect.hpp>

#include <docmodel/dllapi.h>
#include <docmodel/effect/BlurEffect.hxx>

#include <optional>

namespace model::effect
{
DOCMODEL_DLLPUBLIC cpo::uno::Reference<css::util::XBlurEffect>
createXBlurEffect(model::BlurEffect const& rEffect);

/// The blur of an object that createXBlurEffect made, or none for any other object.
DOCMODEL_DLLPUBLIC std::optional<model::BlurEffect>
getFromXBlurEffect(cpo::uno::Reference<css::util::XBlurEffect> const& rxEffect);

} // end of namespace model::effect

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
