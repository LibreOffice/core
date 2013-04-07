/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
