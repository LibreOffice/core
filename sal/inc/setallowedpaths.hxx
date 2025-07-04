/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <string_view>

#if defined UNX
/** Resets the list of paths and associated permissions for osl

    @param[in] aPaths
    Contains a ':' delimited list of control strings and paths.
    Control segments are a short path that refers to the following
    segments and contain either:

    r: read-only paths follow (the default)
    w: read & write paths follow
    x: executable paths follow

    Any real paths (ie. having resolved all symlinks)
    accessed outside of these paths will cause an
    osl_File_E_ACCESS error in the relevant method calls.

    This method is Unix specific.

    @see isForbidden in sal/osl/unx/file_impl.hxx
*/
void setAllowedPaths(std::u16string_view aPaths);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
