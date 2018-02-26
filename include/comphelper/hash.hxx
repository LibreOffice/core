/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_HASH_HXX
#define INCLUDED_COMPHELPER_HASH_HXX

#include <comphelper/comphelperdllapi.h>

#include <memory>
#include <vector>

namespace rtl {
    class OUString;
}

namespace comphelper {

enum class HashType
{
    MD5,
    SHA1,
    SHA256,
    SHA512
};

struct HashImpl;

class COMPHELPER_DLLPUBLIC Hash
{
private:
    std::unique_ptr<HashImpl> mpImpl;

public:

    Hash(HashType eType);
    ~Hash();

    void update(const unsigned char* pInput, size_t length);

    std::vector<unsigned char> finalize();

    static std::vector<unsigned char> calculateHash(const unsigned char* pInput, size_t length, HashType eType);

    /** Calculate hash value with salt (pSalt,nSaltLen) prepended to password
        (pInput,nLength) and repeated iterations run if nSpinCount>0.

        For repeated iterations, each iteration's result plus a 4 byte value
        (0-based, little endian) containing the number of the iteration
        appended to the hash value is the input for the next iteration.

        This implements the algorithms as specified in
        https://msdn.microsoft.com/en-us/library/dd920692 or
        https://msdn.microsoft.com/en-us/library/dd924776 and
        https://msdn.microsoft.com/en-us/library/dd925430

        @param  pSalt
                may be nullptr thus no salt prepended

        @param  bPrependNotAppend
                If <FALSE/>, append spin count in iterations as per
                https://msdn.microsoft.com/en-us/library/dd920692
                If <TRUE/>, prepend spin count in iterations as per
                https://msdn.microsoft.com/en-us/library/dd924776 and
                https://msdn.microsoft.com/en-us/library/dd925430

        @return the raw hash value
     */
    static std::vector<unsigned char> calculateHash(
            const unsigned char* pInput, size_t nLength,
            const unsigned char* pSalt, size_t nSaltLen,
            sal_uInt32 nSpinCount,
            bool bPrependNotAppend,
            HashType eType);

    /** Convenience function to calculate a salted hash with iterations.

        @param  rPassword
                UTF-16LE encoded string, hashed byte-wise as unsigned char.

        @param  rSaltValue
                Salt that will be prepended to password data.
     */
    static std::vector<unsigned char> calculateHash(
            const rtl::OUString& rPassword,
            const std::vector<unsigned char>& rSaltValue,
            sal_uInt32 nSpinCount,
            bool bPrependNotAppend,
            HashType eType);

    size_t getLength() const;
};

}

#endif // INCLUDED_COMPHELPER_HASH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
