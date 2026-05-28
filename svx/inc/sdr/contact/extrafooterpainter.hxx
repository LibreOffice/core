/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_EXTRAFOOTERPAINTER_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_EXTRAFOOTERPAINTER_HXX

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>

class SdrCaptionObj;

namespace sdr::contact
{
/// Builds the author/date footer band for SC cell-note captions
/// (tdf#161973). Returns an empty reference if the caption has no
/// footer state set.
drawinglayer::primitive2d::Primitive2DReference
createExtraFooterPrimitive(const SdrCaptionObj& rCaptionObj);
} // namespace sdr::contact

#endif // INCLUDED_SVX_INC_SDR_CONTACT_EXTRAFOOTERPAINTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
