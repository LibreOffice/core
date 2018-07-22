/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_DATAPROVIDER_CRYPTOCREDENTIALS_HXX
#define INCLUDED_SC_SOURCE_UI_DATAPROVIDER_CRYPTOCREDENTIALS_HXX

#include <oox/crypto/CryptTools.hxx>
#include <oox/crypto/AgileEngine.hxx>
#include <tools/stream.hxx>

namespace sc
{
class CredentialEncryption
{
private:
    SvMemoryStream aEncryptionInfo;
    SvMemoryStream aEncryptedStream;
    OUString aPassword;
    oox::core::AgileEngine aEngine;
    SvMemoryStream aUnencryptedInput;
    uno::Reference<io::XInputStream> xInputStream;
    uno::Reference<io::XOutputStream> xOutputStream;

public:
    CredentialEncryption();
    bool encrypt();
};

class CredentialDecryption
{
private:
    SvMemoryStream aEncryptionInfo;
    SvMemoryStream aEncryptedStream;
    oox::core::AgileEngine aEngine;

public:
    CredentialDecryption();
    bool decrypt();
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
