/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svl/PasswordHelper.hxx>
#include <rtl/digest.h>
#include <boost/scoped_array.hpp>

using namespace com::sun::star;

void SvPasswordHelper::GetHashPassword(uno::Sequence<sal_Int8>& rPassHash, const sal_Char* pPass, sal_uInt32 nLen)
{
    rPassHash.realloc(RTL_DIGEST_LENGTH_SHA1);

    rtlDigestError aError = rtl_digest_SHA1 (pPass, nLen, reinterpret_cast<sal_uInt8*>(rPassHash.getArray()), rPassHash.getLength());
    if (aError != rtl_Digest_E_None)
    {
        rPassHash.realloc(0);
    }
}

void SvPasswordHelper::GetHashPasswordLittleEndian(uno::Sequence<sal_Int8>& rPassHash, const OUString& sPass)
{
    sal_Int32 nSize(sPass.getLength());
    boost::scoped_array<sal_Char> pCharBuffer(new sal_Char[nSize * sizeof(sal_Unicode)]);

    for (sal_Int32 i = 0; i < nSize; ++i)
    {
        sal_Unicode ch(sPass[ i ]);
        pCharBuffer[2 * i] = static_cast< sal_Char >(ch & 0xFF);
        pCharBuffer[2 * i + 1] = static_cast< sal_Char >(ch >> 8);
    }

    GetHashPassword(rPassHash, pCharBuffer.get(), nSize * sizeof(sal_Unicode));
}

void SvPasswordHelper::GetHashPasswordBigEndian(uno::Sequence<sal_Int8>& rPassHash, const OUString& sPass)
{
    sal_Int32 nSize(sPass.getLength());
    boost::scoped_array<sal_Char> pCharBuffer(new sal_Char[nSize * sizeof(sal_Unicode)]);

    for (sal_Int32 i = 0; i < nSize; ++i)
    {
        sal_Unicode ch(sPass[ i ]);
        pCharBuffer[2 * i] = static_cast< sal_Char >(ch >> 8);
        pCharBuffer[2 * i + 1] = static_cast< sal_Char >(ch & 0xFF);
    }

    GetHashPassword(rPassHash, pCharBuffer.get(), nSize * sizeof(sal_Unicode));
}

void SvPasswordHelper::GetHashPassword(uno::Sequence<sal_Int8>& rPassHash, const OUString& sPass)
{
    GetHashPasswordLittleEndian(rPassHash, sPass);
}

bool SvPasswordHelper::CompareHashPassword(const uno::Sequence<sal_Int8>& rOldPassHash, const OUString& sNewPass)
{
    bool bResult = false;

    uno::Sequence<sal_Int8> aNewPass(RTL_DIGEST_LENGTH_SHA1);
    GetHashPasswordLittleEndian(aNewPass, sNewPass);
    if (aNewPass == rOldPassHash)
        bResult = true;
    else
    {
        GetHashPasswordBigEndian(aNewPass, sNewPass);
        bResult = (aNewPass == rOldPassHash);
    }

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
