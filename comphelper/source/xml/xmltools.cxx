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
#include <tools/Guid.hxx>
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
        // Generate some 'chaff' of varying length to be the body of an
        // XML comment to put at the start of encrypted content to make
        // document content a little less predictable.
        // See SvXMLExport::addChaffWhenEncryptedStorage
        OString makeXMLChaff()
        {
            sal_Int8 n;
            (void)rtl_random_getBytes(nullptr, &n, 1);

            sal_Int32 nLength = 1024+n;
            // coverity[tainted_data] - 1024 deliberate random minus max -127/plus max 128
            std::vector<sal_uInt8> aChaff(nLength);
            (void)rtl_random_getBytes(nullptr, aChaff.data(), nLength);

            encodeChaff(aChaff);

            return OString(reinterpret_cast<const char*>(aChaff.data()), nLength);
        }

        OString generateGUIDString()
        {
            tools::Guid aGuid(tools::Guid::Generate);
            return aGuid.getString();
        }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
