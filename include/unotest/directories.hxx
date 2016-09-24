/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOTEST_DIRECTORIES_HXX
#define INCLUDED_UNOTEST_DIRECTORIES_HXX

#include <sal/config.h>

#include <rtl/ustring.hxx>
#include <unotest/detail/unotestdllapi.hxx>

namespace test {

class OOO_DLLPUBLIC_UNOTEST Directories {
private:
    OUString m_aSrcRootURL;
    OUString m_aSrcRootPath;
    OUString m_aWorkdirRootURL;
    OUString m_aWorkdirRootPath;

public:
    Directories();

    const OUString& getSrcRootURL()       { return m_aSrcRootURL; }
    const OUString& getSrcRootPath()      { return m_aSrcRootPath; }

    // return a URL to a given c-str path from the source directory
    OUString getURLFromSrc( const char *pPath );
    OUString getURLFromSrc( const OUString& rPath );

    // return a Path to a given c-str path from the source directory
    OUString getPathFromSrc( const char *pPath );

    // return a URL to a given c-str path from the workdir directory
    OUString getURLFromWorkdir( const char *pPath );

#ifdef _WIN32
    // return a Path to a given c-str path from the workdir directory
    OUString getPathFromWorkdir( const char *pPath );
#endif
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
