/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_CRYPTO_IRMENGINE_HXX
#define INCLUDED_OOX_CRYPTO_IRMENGINE_HXX

#include <oox/dllapi.h>
#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace oox
{
namespace core
{
struct OOX_DLLPUBLIC IRMEncryptionInfo
{
    OString license;
    bool bCanRead;
};

class OOX_DLLPUBLIC IRMEngine : public cppu::WeakImplHelper<css::packages::XPackageEncryption>
{
    IRMEncryptionInfo mInfo;
    css::uno::Reference<css::uno::XComponentContext> mxContext;

    css::uno::Reference<css::io::XInputStream>
    getStream(const css::uno::Sequence<css::beans::NamedValue>& rStreams,
              const OUString sStreamName);

public:
    IRMEngine(const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    // Decryption

    sal_Bool generateEncryptionKey(const OUString& rPassword) override;
    sal_Bool
    readEncryptionInfo(const css::uno::Sequence<css::beans::NamedValue>& aStreams) override;
    sal_Bool decrypt(const css::uno::Reference<css::io::XInputStream>& rxInputStream,
                     css::uno::Reference<css::io::XOutputStream>& rxOutputStream) override;

    sal_Bool checkDataIntegrity() override;

    // Encryption

    css::uno::Sequence<css::beans::NamedValue> writeEncryptionInfo() override;

    void encrypt(const css::uno::Reference<css::io::XInputStream>& rxInputStream,
                 css::uno::Reference<css::io::XOutputStream>& rxOutputStream) override;

    sal_Bool
    setupEncryption(const css::uno::Sequence<css::beans::NamedValue>& rMediaEncData) override;

    css::uno::Sequence<css::beans::NamedValue>
    createEncryptionData(const OUString& rPassword) override;
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
