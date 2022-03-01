/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace i18nutil
{
/** This number shows month days without Jan and Feb.
 *  According to the article, it is calculated as (365-31-28)/10 = 30.6, but because
 *  of a floating point bug, it was used as 30.6001 as a workaround.
 *
 *  "30.6001, 25 year old hack?"
 *  https://www.hpmuseum.org/cgi-sys/cgiwrap/hpmuseum/archv011.cgi?read=31650 */
constexpr double monthDaysWithoutJanFeb = (365 - 31 - 28) / 10.0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
