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
#include <tools/stream.hxx>

namespace sc
{
class CredentialEncryption
{
private:
    OUString aPassword;

public:
    CredentialEncryption();
    bool encrypt();
};

class CredentialDecryption
{
private:
    OUString aPassword;

public:
    CredentialDecryption();
    bool decrypt();
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
