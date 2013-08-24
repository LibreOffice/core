/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef CRYPTO_ENGINE_HXX
#define CRYPTO_ENGINE_HXX

#include <vector>

#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"

namespace oox {
namespace core {

class CryptoEngine
{
protected:
    std::vector<sal_uInt8> mKey;

public:
    CryptoEngine()
    {}

    virtual ~CryptoEngine()
    {}

    virtual std::vector<sal_uInt8>& getKey()
    {
        return mKey;
    }

    virtual bool writeEncryptionInfo(
                    const OUString& rPassword,
                    BinaryXOutputStream& rStream) = 0;

    virtual bool generateEncryptionKey(const OUString& rPassword) = 0;

    virtual bool decrypt(
                    BinaryXInputStream& aInputStream,
                    BinaryXOutputStream& aOutputStream) = 0;

    virtual bool encrypt(
                    BinaryXInputStream& aInputStream,
                    BinaryXOutputStream& aOutputStream) = 0;
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
