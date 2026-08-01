/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <hb-ot.h>

namespace vcl::font
{
// These are "private" HarfBuzz metrics tags, they are supported but not exposed in the public
// header. They are safe to use, HarfBuzz just does not want to advertise them.
constexpr auto ASCENT_OS2 = static_cast<hb_ot_metrics_tag_t>(HB_TAG('O', 'a', 's', 'c'));
constexpr auto DESCENT_OS2 = static_cast<hb_ot_metrics_tag_t>(HB_TAG('O', 'd', 's', 'c'));
constexpr auto LINEGAP_OS2 = static_cast<hb_ot_metrics_tag_t>(HB_TAG('O', 'l', 'g', 'p'));
constexpr auto ASCENT_HHEA = static_cast<hb_ot_metrics_tag_t>(HB_TAG('H', 'a', 's', 'c'));
constexpr auto DESCENT_HHEA = static_cast<hb_ot_metrics_tag_t>(HB_TAG('H', 'd', 's', 'c'));
constexpr auto LINEGAP_HHEA = static_cast<hb_ot_metrics_tag_t>(HB_TAG('H', 'l', 'g', 'p'));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
