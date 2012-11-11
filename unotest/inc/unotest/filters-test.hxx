/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Caolán McNamara <caolanm@redhat.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
        const rtl::OUString &rFilter,
        //root dir of test files, must contain pass, fail, indeterminate
        const rtl::OUString &rURL,
        //additional filter data for SfxFilter
        const rtl::OUString &rUserData = rtl::OUString(),
        //SfxFilterFlags for SfxFilter
        unsigned int nFilterFlags = SFX_FILTER_IMPORT,
        //Clipboard id for SfxFilter
        unsigned int nClipboardID = 0,
        //additional filter version for SfxFilter
        unsigned int nFilterVersion = 0);

    virtual bool load(
        const rtl::OUString &rFilter,
        const rtl::OUString &rURL,
        const rtl::OUString &rUserData,
        unsigned int nFilterFlags,
        unsigned int nClipboardID,
        unsigned int nFilterVersion) = 0;

protected:
    ~FiltersTest() {}

    void recursiveScan(
        filterStatus nExpected,
        const rtl::OUString &rFilter,
        const rtl::OUString &rURL,
        const rtl::OUString &rUserData,
        unsigned int nFilterFlags,
        unsigned int nClipboardID,
        unsigned int nFilterVersion);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
