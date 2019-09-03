/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_CRYPTO_STRONGENCRYPTINDATASPACE_HXX
#define INCLUDED_OOX_CRYPTO_STRONGENCRYPTINDATASPACE_HXX

#include <oox/dllapi.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <oox/crypto/CryptoEngine.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace oox
{
namespace crypto
{
class OOX_DLLPUBLIC StrongEncryptionDataSpace final
    : public cppu::WeakImplHelper<css::packages::XPackageEncryption>
{
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    std::unique_ptr<CryptoEngine> mCryptoEngine;

    css::uno::Reference<css::io::XInputStream>
    getStream(const css::uno::Sequence<css::beans::NamedValue>& rStreams,
              const rtl::OUString sStreamName);

public:
    StrongEncryptionDataSpace(const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    // Decryption

    virtual sal_Bool SAL_CALL generateEncryptionKey(const OUString& rPassword) override;
    virtual sal_Bool SAL_CALL
    readEncryptionInfo(const css::uno::Sequence<css::beans::NamedValue>& aStreams) override;
    virtual sal_Bool SAL_CALL
    decrypt(const css::uno::Reference<css::io::XInputStream>& rxInputStream,
            css::uno::Reference<css::io::XOutputStream>& rxOutputStream) override;

    virtual sal_Bool SAL_CALL checkDataIntegrity() override;

    // Encryption

    virtual css::uno::Sequence<css::beans::NamedValue>
        SAL_CALL encrypt(const css::uno::Reference<css::io::XInputStream>& rxInputStream) override;

    virtual sal_Bool SAL_CALL
    setupEncryption(const css::uno::Sequence<css::beans::NamedValue>& rMediaEncData) override;

    virtual css::uno::Sequence<css::beans::NamedValue>
        SAL_CALL createEncryptionData(const OUString& rPassword) override;
};

} // namespace crypto
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
