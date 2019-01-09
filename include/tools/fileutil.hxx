/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TOOLS_FILEUTIL_HXX
#define INCLUDED_TOOLS_FILEUTIL_HXX

#include <tools/toolsdllapi.h>
#include <rtl/ustring.hxx>

namespace tools
{
// Tests if the path is a UNC or local (drive-based) path that redirects to
// a WebDAV resource (e.g., using redirectors on Windows).
// Currently only implemented for Windows; on other platforms, returns false.
TOOLS_DLLPUBLIC bool IsMappedWebDAVPath(const OUString& rURL, OUString* pRealURL = nullptr);
}

#endif // INCLUDED_TOOLS_FILEUTIL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
