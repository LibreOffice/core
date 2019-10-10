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

#include <oox/dllapi.h>
#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <filter/msfilter/mscodec.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace oox {
    class BinaryXInputStream;
    class BinaryXOutputStream;
}

namespace oox {
namespace core {

class OOX_DLLPUBLIC Standard2007Engine final : public cppu::WeakImplHelper<css::packages::XPackageEncryption>
{
    msfilter::StandardEncryptionInfo mInfo;
    std::vector<sal_uInt8> mKey;
    css::uno::Reference< css::uno::XComponentContext > mxContext;

    bool generateVerifier();
    bool calculateEncryptionKey(const OUString& rPassword);

    css::uno::Reference<css::io::XInputStream> getStream(const css::uno::Sequence<css::beans::NamedValue> & rStreams, const OUString sStreamName);

public:
    Standard2007Engine(const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    // Decryption

    virtual sal_Bool SAL_CALL generateEncryptionKey(const OUString & rPassword) override;
    virtual sal_Bool SAL_CALL readEncryptionInfo(const css::uno::Sequence<css::beans::NamedValue>& aStreams) override;
    virtual sal_Bool SAL_CALL decrypt(const css::uno::Reference<css::io::XInputStream>& rxInputStream,
                 css::uno::Reference<css::io::XOutputStream>& rxOutputStream) override;


    virtual sal_Bool SAL_CALL checkDataIntegrity() override;

    // Encryption

    virtual css::uno::Sequence<css::beans::NamedValue> SAL_CALL writeEncryptionInfo() override;

    virtual void SAL_CALL encrypt(const css::uno::Reference<css::io::XInputStream>& rxInputStream,
                 css::uno::Reference<css::io::XOutputStream>& rxOutputStream) override;

    virtual sal_Bool SAL_CALL setupEncryption(const css::uno::Sequence<css::beans::NamedValue>& rMediaEncData) override;

    virtual css::uno::Sequence<css::beans::NamedValue> SAL_CALL createEncryptionData(const OUString& rPassword) override;
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
