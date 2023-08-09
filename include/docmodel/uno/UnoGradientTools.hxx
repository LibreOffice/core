/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <docmodel/dllapi.h>
#include <basegfx/utils/bgradient.hxx>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/awt/ColorStopSequence.hpp>
#include <com/sun/star/uno/Any.hxx>

namespace model::gradient
{
DOCMODEL_DLLPUBLIC css::awt::Gradient2 createUnoGradient2(basegfx::BGradient const& rGradient);

DOCMODEL_DLLPUBLIC basegfx::BGradient getFromUnoGradient2(css::awt::Gradient2 const& rGradient2);
DOCMODEL_DLLPUBLIC basegfx::BGradient getFromAny(css::uno::Any const& rAny);

DOCMODEL_DLLPUBLIC css::awt::ColorStopSequence
createColorStopSequence(basegfx::BColorStops const& rColorStops);

DOCMODEL_DLLPUBLIC basegfx::BColorStops
getColorStopsFromUno(css::awt::ColorStopSequence const& rColorStopSequence);
DOCMODEL_DLLPUBLIC basegfx::BColorStops getColorStopsFromAny(css::uno::Any const& rAny);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
