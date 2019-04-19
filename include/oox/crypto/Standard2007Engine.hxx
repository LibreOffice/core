/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <oox/dllapi.h>
#include <oox/crypto/CryptoEngine.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <rtl/digest.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace oox {
    class BinaryXInputStream;
    class BinaryXOutputStream;
}

namespace oox {
namespace core {

class OOX_DLLPUBLIC Standard2007Engine : public CryptoEngine
{
    msfilter::StandardEncryptionInfo mInfo;

    bool generateVerifier();
    bool calculateEncryptionKey(const OUString& rPassword);

public:
    Standard2007Engine() = default;

    bool readEncryptionInfo(css::uno::Reference<css::io::XInputStream> & rxInputStream) override;

    virtual bool generateEncryptionKey(OUString const & rPassword) override;

    virtual bool decrypt(
                    BinaryXInputStream& aInputStream,
                    BinaryXOutputStream& aOutputStream) override;

    bool checkDataIntegrity() override;

    void encrypt(css::uno::Reference<css::io::XInputStream>&  rxInputStream,
                 css::uno::Reference<css::io::XOutputStream>& rxOutputStream,
                 sal_uInt32 nSize) override;

    virtual void writeEncryptionInfo(BinaryXOutputStream& rStream) override;

    virtual bool setupEncryption(OUString const & rPassword) override;

};

} // namespace core
} // namespace oox


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
