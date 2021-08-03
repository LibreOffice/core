/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <rtl/ustring.hxx>

namespace sfx2
{
/** Open a URI via com.sun.star.system.SystemShellExecute

    Handles XSystemShellExecute.execute's IllegalArgumentException (throwing a
    RuntimeException if it is unexpected, i.e., not caused by the given uri not
    being an absolute URI reference).

    Handles XSystemShellExecute.execute's SystemShellExecuteException unless the
    given handleSystemShellExecuteException is false (in which case the
    exception is re-thrown).
*/
void openUriExternally(const OUString& sURI, bool bHandleSystemShellExecuteException);
};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
