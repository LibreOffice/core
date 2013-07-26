/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/documentconstants.hxx>
#include <rtl/ustring.hxx>
#include "unotest/detail/unotestdllapi.hxx"

namespace test {

enum filterStatus
{
    fail = 0,
    pass = 1,
    indeterminate = 2
};

/*
 * NOTE, any files beginning with CVE-, BID- or EDB- will be assumed to be
 * encrypted using arcfour with key 0x435645, this is to silence panicky
 * virus/malware-checkers
 *
 * e.g.  m[de]crypt --bare -a arcfour -o hex -k 435645 -s 3
 */
/* Implementation of Filters test */
class OOO_DLLPUBLIC_UNOTEST FiltersTest
{
public:
    void testDir(
        //filter name
        const OUString &rFilter,
        //root dir of test files, must contain pass, fail, indeterminate
        const OUString &rURL,
        //additional filter data for SfxFilter
        const OUString &rUserData = OUString(),
        //SfxFilterFlags for SfxFilter
        unsigned int nFilterFlags = SFX_FILTER_IMPORT,
        //Clipboard id for SfxFilter
        unsigned int nClipboardID = 0,
        //additional filter version for SfxFilter
        unsigned int nFilterVersion = 0);

    virtual bool load(
        const OUString &rFilter,
        const OUString &rURL,
        const OUString &rUserData,
        unsigned int nFilterFlags,
        unsigned int nClipboardID,
        unsigned int nFilterVersion) = 0;

protected:
    ~FiltersTest() {}

    void recursiveScan(
        filterStatus nExpected,
        const OUString &rFilter,
        const OUString &rURL,
        const OUString &rUserData,
        unsigned int nFilterFlags,
        unsigned int nClipboardID,
        unsigned int nFilterVersion);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
