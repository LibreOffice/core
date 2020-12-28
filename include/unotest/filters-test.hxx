/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOTEST_FILTERS_TEST_HXX
#define INCLUDED_UNOTEST_FILTERS_TEST_HXX

#include <sal/config.h>

#include <string_view>

#include <comphelper/documentconstants.hxx>
#include <rtl/ustring.hxx>
#include <unotest/detail/unotestdllapi.hxx>
#include <sot/formats.hxx>

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
        std::u16string_view rURL,
        //additional filter data for SfxFilter
        const OUString &rUserData = OUString(),
        //SfxFilterFlags for SfxFilter
        SfxFilterFlags nFilterFlags = SfxFilterFlags::IMPORT,
        //Clipboard id for SfxFilter
        SotClipboardFormatId nClipboardID = SotClipboardFormatId::NONE,
        //additional filter version for SfxFilter
        unsigned int nFilterVersion = 0,
        //export or import?
        bool bExport = false);

    virtual bool load(
        const OUString &rFilter,
        const OUString &rURL,
        const OUString &rUserData,
        SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion) = 0;

    virtual bool save(
        const OUString &/*rFilter*/,
        const OUString &/*rURL*/,
        const OUString &/*rUserData*/,
        SfxFilterFlags /*nFilterFlags*/,
        SotClipboardFormatId /*nClipboardID*/,
        unsigned int /*nFilterVersion*/)
    {
        return true;
    }

protected:
    ~FiltersTest() {}

    void recursiveScan(
        filterStatus nExpected,
        const OUString &rFilter,
        const OUString &rURL,
        const OUString &rUserData,
        SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion,
        bool bExport);
};

}

#endif // INCLUDED_UNOTEST_FILTERS_TEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
