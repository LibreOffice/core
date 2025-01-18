/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>
#include <memory>
#include <vector>
#include <osl/file.hxx>

namespace vcl
{
namespace file
{
VCL_DLLPUBLIC bool readFileStatus(osl::FileStatus& rStatus, const OUString& rFile);
}

class VCL_DLLPUBLIC UserResourceScanner
{
protected:
    /** Scans the provided directory for the resource.
     *
     * The returned strings will contain the URLs to the resources.
     */
    std::vector<OUString> readFilesFromPath(const OUString& dir);

    /** Return true if the filename is a valid resource */
    virtual bool isValidResource(const OUString& rFilename) = 0;

    /** Adds the provided resource by path. */
    virtual bool addResource(const OUString& /*path*/) = 0;

public:
    UserResourceScanner();
    virtual ~UserResourceScanner() {}

    /** Provide a semicolon-separated list of paths to search for resource.
     *
     * There are several cases when scan will fail:
     * - The directory does not exist
     * - There are no files which have a valid resource
     */

    void addPaths(std::u16string_view paths);
};

} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
