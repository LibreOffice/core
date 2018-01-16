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

    size_t getLength() const;
};

}

#endif // INCLUDED_COMPHELPER_HASH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
