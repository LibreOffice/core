/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cryptocredentials.hxx>

namespace sc
{
CredentialEncryption::CredentialEncryption()
{
    // Setup input
    aPassword = "Password";
    OString aTestString = OUStringToOString("1234567890ABCDEFGH", RTL_TEXTENCODING_UTF8);

    xInputStream = new utl::OSeekableInputStreamWrapper(aUnencryptedInput);

    aUnencryptedInput.WriteBytes(aTestString.getStr(), aTestString.getLength() + 1);
    aUnencryptedInput.Seek(STREAM_SEEK_TO_BEGIN);

    // Setup output
    xOutputStream = new utl::OSeekableOutputStreamWrapper(aEncryptedStream);
}

bool CredentialEncryption::encrypt()
{
    // Write content
    aEngine.setupEncryption(aPassword);
    aEngine.encrypt(xInputStream, xOutputStream, aUnencryptedInput.GetSize());
    xOutputStream->flush();

    // Setup and write encryption info
    oox::BinaryXOutputStream aBinaryEncryptionInfoOutputStream(
        new utl::OSeekableOutputStreamWrapper(aEncryptionInfo), true);
    aEngine.writeEncryptionInfo(aBinaryEncryptionInfoOutputStream);
    aBinaryEncryptionInfoOutputStream.close();
}

CredentialDecryption::CredentialDecryption() { aPassword = "Password"; }

bool CredentialDecryption::decrypt()
{
    oox::core::AgileEngine aEngine;

    // Read encryption info
    uno::Reference<io::XInputStream> xEncryptionInfo(
        new utl::OSeekableInputStreamWrapper(aEncryptionInfo));

    xEncryptionInfo->skipBytes(4); // Encryption type -> Agile

    aEngine.readEncryptionInfo(xEncryptionInfo);

    // Setup password
    aEngine.generateEncryptionKey(aPassword);

    // Setup encrypted input stream
    oox::BinaryXInputStream aBinaryInputStream(
        new utl::OSeekableInputStreamWrapper(aEncryptedStream), true);

    // Setup output stream
    SvMemoryStream aUnencryptedOutput;
    oox::BinaryXOutputStream aBinaryOutputStream(
        new utl::OSeekableOutputStreamWrapper(aUnencryptedOutput), true);

    // Decrypt
    aEngine.decrypt(aBinaryInputStream, aBinaryOutputStream);
    aBinaryOutputStream.close();
    aBinaryInputStream.close();

    // Get original string
    OString aString(static_cast<const sal_Char*>(aUnencryptedOutput.GetData()));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
