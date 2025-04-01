/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


#include "sal/config.h"

/** @cond INTERNAL */

/* Some additions to the osl file functions for LibreOffice internal
   use. Needed for details in the Android support.
 */

/* More flags needed for semantics that match the open() call that
   used to be in SvFileStream::Open(), and for temp files:
*/
 0x00000010L
 0x00000020L
 0x00000040L

/** @endcond */



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
