/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <pdf/EncryptionHashTransporter.hxx>

using namespace css;

namespace vcl::pdf
{
EncryptionHashTransporter::EncryptionHashTransporter()
    : m_pDigest(new ::comphelper::Hash(::comphelper::HashType::MD5))
    , maID(reinterpret_cast<sal_IntPtr>(this))
{
}

EncryptionHashTransporter::~EncryptionHashTransporter() {}

EncryptionHashTransporter* EncryptionHashTransporter::getEncHashTransporter(
    const uno::Reference<beans::XMaterialHolder>& xReference)
{
    EncryptionHashTransporter* pResult = dynamic_cast<EncryptionHashTransporter*>(xReference.get());
    return pResult;
}

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
