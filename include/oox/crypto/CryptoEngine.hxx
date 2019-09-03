/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_CRYPTO_CRYPTOENGINE_HXX
#define INCLUDED_OOX_CRYPTO_CRYPTOENGINE_HXX

#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

namespace oox {
    class BinaryXInputStream;
    class BinaryXOutputStream;
}

namespace oox {
namespace crypto {

class CryptoEngine
{
protected:
    std::vector<sal_uInt8> mKey;

public:
    CryptoEngine()
    {}

    virtual ~CryptoEngine()
    {}

    // Decryption
    virtual bool readEncryptionInfo(css::uno::Reference<css::io::XInputStream> & rxInputStream) = 0;

    virtual bool generateEncryptionKey(const OUString& rPassword) = 0;

    virtual bool decrypt(
                    BinaryXInputStream& aInputStream,
                    BinaryXOutputStream& aOutputStream) = 0;

    // Encryption
    virtual void writeEncryptionInfo(BinaryXOutputStream & rStream) = 0;

    virtual bool setupEncryption(const OUString& rPassword) = 0;

    virtual void encrypt(const css::uno::Reference<css::io::XInputStream> & rxInputStream,
                         css::uno::Reference<css::io::XOutputStream> & rxOutputStream,
                         sal_uInt32 nSize) = 0;

    virtual bool checkDataIntegrity() = 0;
};

} // namespace crypto
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
