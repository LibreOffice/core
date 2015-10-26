/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SAL_INC_INTERNAL_MISC_H
#define INCLUDED_SAL_INC_INTERNAL_MISC_H

#include <rtl/ustring.hxx>

/// Build a debugging backtrace from current PC location.
rtl_uString *osl_backtraceAsString(void);

#endif // INCLUDED_SAL_INC_INTERNAL_MISC_H

