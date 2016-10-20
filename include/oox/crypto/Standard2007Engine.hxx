/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_CRYPTO_STANDARD2007ENGINE_HXX
#define INCLUDED_OOX_CRYPTO_STANDARD2007ENGINE_HXX

#include <filter/msfilter/mscodec.hxx>
#include <oox/crypto/CryptoEngine.hxx>
#include <rtl/digest.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace oox {
    class BinaryXInputStream;
    class BinaryXOutputStream;
}

namespace oox {
namespace core {

class Standard2007Engine : public CryptoEngine
{
    msfilter::StandardEncryptionInfo mInfo;

    bool generateVerifier();
    bool calculateEncryptionKey(const OUString& rPassword);

public:
    Standard2007Engine();
    virtual ~Standard2007Engine();

    msfilter::StandardEncryptionInfo& getInfo() { return mInfo;}

    virtual bool generateEncryptionKey(const OUString& rPassword) override;

    virtual void writeEncryptionInfo(
                    const OUString& rPassword,
                    BinaryXOutputStream& rStream) override;

    virtual bool decrypt(
                    BinaryXInputStream& aInputStream,
                    BinaryXOutputStream& aOutputStream) override;

    virtual void encrypt(
                    BinaryXInputStream& aInputStream,
                    BinaryXOutputStream& aOutputStream) override;

};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
