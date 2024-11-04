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
{
    maID = reinterpret_cast<sal_IntPtr>(this);
    while (sTransporters.find(maID) != sTransporters.end()) // paranoia mode
        maID++;
    sTransporters[maID] = this;
}

EncryptionHashTransporter::~EncryptionHashTransporter()
{
    sTransporters.erase(maID);
    SAL_INFO("vcl", "EncryptionHashTransporter freed");
}

EncryptionHashTransporter* EncryptionHashTransporter::getEncHashTransporter(
    const uno::Reference<beans::XMaterialHolder>& xReference)
{
    EncryptionHashTransporter* pResult = nullptr;
    if (xReference.is())
    {
        uno::Any aMat(xReference->getMaterial());
        sal_Int64 nMat = 0;
        if (aMat >>= nMat)
        {
            std::map<sal_IntPtr, EncryptionHashTransporter*>::iterator it
                = sTransporters.find(static_cast<sal_IntPtr>(nMat));
            if (it != sTransporters.end())
                pResult = it->second;
        }
    }
    return pResult;
}

std::map<sal_IntPtr, EncryptionHashTransporter*> EncryptionHashTransporter::sTransporters;

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
