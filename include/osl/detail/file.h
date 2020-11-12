/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OSL_DETAIL_FILE_H
#define INCLUDED_OSL_DETAIL_FILE_H

#include "sal/config.h"

/** @cond INTERNAL */

/* Some additions to the osl file functions for LibreOffice internal
   use. Needed for details in the Android support.
 */

/* More flags needed for semantics that match the open() call that
   used to be in SvFileStream::Open(), and for temp files:
*/
#define osl_File_OpenFlag_Trunc 0x00000010L
#define osl_File_OpenFlag_NoExcl 0x00000020L
#define osl_File_OpenFlag_Private 0x00000040L

/** @endcond */

#endif /* INCLUDED_OSL_DETAIL_FILE_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
