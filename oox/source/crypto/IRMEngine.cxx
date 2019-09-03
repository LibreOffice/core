/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <oox/crypto/IRMEngine.hxx>

#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>

#include <sax/tools/converter.hxx>

#include <comphelper/hash.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/random.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/sequence.hxx>

#include <filter/msfilter/mscodec.hxx>
#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>

#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>

#include <msipc.h>

using namespace css;
using namespace css::beans;
using namespace css::io;
using namespace css::lang;
using namespace css::uno;
using namespace css::xml::sax;
using namespace css::xml;

namespace oox
{
namespace core
{
IRMEngine::IRMEngine() {}

bool IRMEngine::checkDataIntegrity() { return true; }

bool IRMEngine::decrypt(BinaryXInputStream& aInputStream, BinaryXOutputStream& aOutputStream)
{
    HRESULT hr = IpcInitialize();

    IPC_BUFFER licenseBuffer;
    licenseBuffer.pvBuffer = (void*)mInfo.license.getStr();
    licenseBuffer.cbBuffer = mInfo.license.getLength();

    IPC_PROMPT_CTX ctx;
    ctx.cbSize = sizeof(ctx);
    ctx.hwndParent = NULL;
    ctx.dwFlags = 0;
    ctx.hCancelEvent = NULL;
    ctx.pcCredential = NULL;
    ctx.wszApplicationScenarioId = NULL;

    IPC_KEY_HANDLE key;

    hr = IpcGetKey(&licenseBuffer, 0, &ctx, NULL, &key);

    sal_Int64 inputSize = aInputStream.size();
    unsigned char* inputBuffer = new unsigned char[inputSize];
    memset(inputBuffer, 0, inputSize);
    aInputStream.readArray(inputBuffer, inputSize);

    // Get out size
    ULONG bytes;
    hr = IpcDecrypt(key, 0, TRUE, inputBuffer, inputSize, NULL, 0, &bytes);

    sal_Int64 outputSize = bytes * 5;
    unsigned char* outputBuffer = new unsigned char[outputSize];
    memset(outputBuffer, 0, bytes);

    // Decode
    hr = IpcDecrypt(key, 0, TRUE, inputBuffer, inputSize, outputBuffer, outputSize, &bytes);

    aOutputStream.writeArray(outputBuffer, bytes);

    delete[] outputBuffer;
    delete[] inputBuffer;
    return true;
}

bool IRMEngine::readEncryptionInfo(uno::Reference<io::XInputStream>& rxInputStream)
{
    // MS-OFFCRYPTO 2.2.6: XrMLLicense
    BinaryXInputStream aBinaryStream(rxInputStream, true);
    sal_uInt32 aStringLength = aBinaryStream.readuInt32();
    mInfo.license = aBinaryStream.readCharArray(aStringLength);

    if (mInfo.license[0] != 0x0ef) // BOM is missing?
    {
        mInfo.license = "\x0ef\x0bb\x0bf" + mInfo.license;
    }

    // TODO: CHECK info data

    return true;
}

bool IRMEngine::setupEncryption(OUString const& rPassword) { return true; }

void IRMEngine::writeEncryptionInfo(BinaryXOutputStream& rStream) {}

void IRMEngine::encrypt(css::uno::Reference<css::io::XInputStream>& rxInputStream,
                        css::uno::Reference<css::io::XOutputStream>& rxOutputStream,
                        sal_uInt32 nSize)
{
}

bool IRMEngine::generateEncryptionKey(const OUString& password) { return true; }

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
