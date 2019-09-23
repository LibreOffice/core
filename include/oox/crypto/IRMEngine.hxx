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
#include <oox/crypto/CryptoEngine.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace oox
{
class BinaryXInputStream;
class BinaryXOutputStream;
}

namespace oox
{
namespace core
{
struct OOX_DLLPUBLIC IRMEncryptionInfo
{
    OString license;
    bool bCanRead;
};

class OOX_DLLPUBLIC IRMEngine : public CryptoEngine
{
    IRMEncryptionInfo mInfo;
    css::uno::Reference<css::uno::XComponentContext> mxContext;

    css::uno::Reference<css::io::XInputStream>
    getStream(css::uno::Sequence<css::beans::NamedValue>& rStreams, const OUString sStreamName);

public:
    IRMEngine(const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    bool readEncryptionInfo(css::uno::Sequence<css::beans::NamedValue> aStreams) override;

    virtual bool generateEncryptionKey(OUString const& rPassword) override;

    virtual bool decrypt(BinaryXInputStream& aInputStream,
                         BinaryXOutputStream& aOutputStream) override;

    bool checkDataIntegrity() override;

    void encrypt(css::uno::Reference<css::io::XInputStream>& rxInputStream,
                 css::uno::Reference<css::io::XOutputStream>& rxOutputStream,
                 sal_uInt32 nSize) override;

    virtual void writeEncryptionInfo(oox::ole::OleStorage& rOleStorage) override;

    virtual void createEncryptionData(comphelper::SequenceAsHashMap& aEncryptionData,
                                      const OUString rPassword) override;

    virtual bool
    setupEncryption(css::uno::Sequence<css::beans::NamedValue>& rMediaEncData) override;
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
