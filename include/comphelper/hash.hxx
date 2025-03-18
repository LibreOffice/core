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
#include <rtl/digest.h>

#include <memory>
#include <string>
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
    SHA384,
    SHA512
};

const sal_uInt32 MD5_HASH_LENGTH = RTL_DIGEST_LENGTH_MD5;
const sal_uInt32 SHA1_HASH_LENGTH = RTL_DIGEST_LENGTH_SHA1;
const sal_uInt32 SHA256_HASH_LENGTH = 32;
const sal_uInt32 SHA384_HASH_LENGTH = 48;
const sal_uInt32 SHA512_HASH_LENGTH = 64;

struct HashImpl;

COMPHELPER_DLLPUBLIC std::string hashToString(const std::vector<unsigned char>& rHash);

class COMPHELPER_DLLPUBLIC Hash
{
private:
    std::unique_ptr<HashImpl> mpImpl;

public:

    enum class IterCount
    {
        NONE,       /// Iteration count not added to hash iterations.
        PREPEND,    /// Iteration count prepended to hash iterations.
        APPEND      /// Iteration count appended to hash iterations.
    };

    Hash(HashType eType);
    ~Hash();

    void update(const unsigned char* pInput, size_t length);

    void update(std::vector<unsigned char> const& rInput)
    {
        update(rInput.data(), rInput.size());
    }

    void initialize();

    std::vector<unsigned char> finalize();

    static std::vector<unsigned char> calculateHash(const unsigned char* pInput, size_t length, HashType eType);

    /** Calculate hash value with salt (pSalt,nSaltLen) prepended to password
        (pInput,nLength) and repeated iterations run if nSpinCount>0.

        This implements the algorithms as specified in
        https://msdn.microsoft.com/en-us/library/dd920692 or
        https://msdn.microsoft.com/en-us/library/dd924776 and
        https://msdn.microsoft.com/en-us/library/dd925430

        @param  pSalt
                may be nullptr thus no salt prepended

        @param  nSpinCount
                If >0, repeat nSpinCount iterations. For each iteration, the
                previous iteration's result plus a 4 byte value (0-based,
                little endian) containing the number of the iteration prepended
                or appended to the hash value is the input for the next
                iteration.

        @param  eIterCount
                If IterCount::APPEND, append iteration count as per
                https://msdn.microsoft.com/en-us/library/dd920692
                If IterCount::PREPEND, prepend iteration count as per
                https://msdn.microsoft.com/en-us/library/dd924776 and
                https://msdn.microsoft.com/en-us/library/dd925430
                If IterCount::NONE, do not add the iteration count to hash
                iterations.

        @return the raw hash value
     */
    static std::vector<unsigned char> calculateHash(
            const unsigned char* pInput, size_t nLength,
            const unsigned char* pSalt, size_t nSaltLen,
            sal_uInt32 nSpinCount,
            IterCount eIterCount,
            HashType eType);

    /** Convenience function to calculate a salted hash with iterations.

        @param  rPassword
                UTF-16 encoded string, hashed byte-wise as unsigned char.

        @param  rSaltValue
                Salt that will be prepended to password data.
     */
    static std::vector<unsigned char> calculateHash(
            const rtl::OUString& rPassword,
            const std::vector<unsigned char>& rSaltValue,
            sal_uInt32 nSpinCount,
            IterCount eIterCount,
            HashType eType);

    size_t getLength() const;
};

}

#endif // INCLUDED_COMPHELPER_HASH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
