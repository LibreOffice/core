/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/xmltools.hxx>
#include <rtl/random.h>
#include <rtl/uuid.h>
#include <vector>

using namespace com::sun::star;

namespace
{
    //Will be inside an xml comment, so can't use '-' in case '--' appears in
    //output, etc. Despite what *is* legal in an xml comment, just using the
    //base-64 subset to avoid pain with simplistic third-party parsers
    const sal_uInt8 aChaffEncoder[] =
    {
        'A', 'Q', 'g', 'w', 'B', 'R', 'h', 'x',
        'C', 'S', 'i', 'y', 'D', 'T', 'j', 'z',
        'E', 'U', 'k', '0', 'F', 'V', 'l', '1',
        'G', 'W', 'm', '2', 'H', 'X', 'n', '3',
        'I', 'Y', 'o', '4', 'J', 'Z', 'p', '5',
        'K', 'a', 'q', '6', 'L', 'b', 'r', '7',
        'M', 'c', 's', '8', 'N', 'd', 't', '9',
        'O', 'e', 'u', '+', 'P', 'f', 'v', '/',

        'A', 'Q', 'g', 'w', 'B', 'R', 'h', 'x',
        'C', 'S', 'i', 'y', 'D', 'T', 'j', 'z',
        'E', 'U', 'k', '0', 'F', 'V', 'l', '1',
        'G', 'W', 'm', '2', 'H', 'X', 'n', '3',
        'I', 'Y', 'o', '4', 'J', 'Z', 'p', '5',
        'K', 'a', 'q', '6', 'L', 'b', 'r', '7',
        'M', 'c', 's', '8', 'N', 'd', 't', '9',
        'O', 'e', 'u', '+', 'P', 'f', 'v', '/',

        'A', 'Q', 'g', 'w', 'B', 'R', 'h', 'x',
        'C', 'S', 'i', 'y', 'D', 'T', 'j', 'z',
        'E', 'U', 'k', '0', 'F', 'V', 'l', '1',
        'G', 'W', 'm', '2', 'H', 'X', 'n', '3',
        'I', 'Y', 'o', '4', 'J', 'Z', 'p', '5',
        'K', 'a', 'q', '6', 'L', 'b', 'r', '7',
        'M', 'c', 's', '8', 'N', 'd', 't', '9',
        'O', 'e', 'u', '+', 'P', 'f', 'v', '/',

        'A', 'Q', 'g', 'w', 'B', 'R', 'h', 'x',
        'C', 'S', 'i', 'y', 'D', 'T', 'j', 'z',
        'E', 'U', 'k', '0', 'F', 'V', 'l', '1',
        'G', 'W', 'm', '2', 'H', 'X', 'n', '3',
        'I', 'Y', 'o', '4', 'J', 'Z', 'p', '5',
        'K', 'a', 'q', '6', 'L', 'b', 'r', '7',
        'M', 'c', 's', '8', 'N', 'd', 't', '9',
        'O', 'e', 'u', '+', 'P', 'f', 'v', '/'
    };

    void encodeChaff(std::vector<sal_uInt8> &rChaff)
    {
        static_assert(sizeof(aChaffEncoder) == 256, "this has to cover all chars");

        for (auto & elem : rChaff)
        {
            elem = aChaffEncoder[elem];
        }
    }
}

namespace comphelper::xml
{
        OString makeXMLChaff()
        {
            rtlRandomPool pool = rtl_random_createPool();

            sal_Int8 n;
            rtl_random_getBytes(pool, &n, 1);

            sal_Int32 nLength = 1024+n;
            // coverity[tainted_data] - 1024 deliberate random minus max -127/plus max 128
            std::vector<sal_uInt8> aChaff(nLength);
            rtl_random_getBytes(pool, aChaff.data(), nLength);

            rtl_random_destroyPool(pool);

            encodeChaff(aChaff);

            return OString(reinterpret_cast<const char*>(aChaff.data()), nLength);
        }

        OString generateGUIDString()
        {
            sal_uInt8 aSeq[16];
            rtl_createUuid(aSeq, nullptr, true);

            char str[39];
            sprintf(str, "{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                    aSeq[0], aSeq[1], aSeq[2], aSeq[3], aSeq[4], aSeq[5], aSeq[6], aSeq[7], aSeq[8],
                    aSeq[9], aSeq[10], aSeq[11], aSeq[12], aSeq[13], aSeq[14], aSeq[15]);

            return str;
        }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
