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
#include <boost/static_assert.hpp>
#include <vector>

using namespace com::sun::star;

namespace
{
    //Will be inside an xml comment, so can't use '-' in case '--' appears in
    //output, etc. Despite what *is* legal in an xml comment, just using the
    //base-64 subset to avoid pain with simplistic third-party parsers
    static const sal_uInt8 aChaffEncoder[] =
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
        BOOST_STATIC_ASSERT(sizeof(aChaffEncoder) == 256);

        for (std::vector<sal_uInt8>::iterator aI = rChaff.begin(), aEnd = rChaff.end();
            aI != aEnd; ++aI)
        {
            *aI = aChaffEncoder[*aI];
        }
    }
}

namespace comphelper
{
    namespace xml
    {
        OString makeXMLChaff()
        {
            rtlRandomPool pool = rtl_random_createPool();

            sal_Int8 n;
            rtl_random_getBytes(pool, &n, 1);

            //1024 minus max -127/plus max 128
            sal_Int32 nLength = 1024+n;
            std::vector<sal_uInt8> aChaff(nLength);
            rtl_random_getBytes(pool, &aChaff[0], nLength);

            rtl_random_destroyPool(pool);

            encodeChaff(aChaff);

            return OString(reinterpret_cast<const sal_Char*>(&aChaff[0]), nLength);
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
