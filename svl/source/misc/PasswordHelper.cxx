/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <svl/PasswordHelper.hxx>
#include <rtl/digest.h>
#include <tools/string.hxx>

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

void SvPasswordHelper::GetHashPasswordLittleEndian(uno::Sequence<sal_Int8>& rPassHash, const String& sPass)
{
    xub_StrLen nSize(sPass.Len());
    sal_Char* pCharBuffer = new sal_Char[nSize * sizeof(sal_Unicode)];

    for (xub_StrLen i = 0; i < nSize; ++i)
    {
        sal_Unicode ch(sPass.GetChar(i));
        pCharBuffer[2 * i] = static_cast< sal_Char >(ch & 0xFF);
        pCharBuffer[2 * i + 1] = static_cast< sal_Char >(ch >> 8);
    }

    GetHashPassword(rPassHash, pCharBuffer, nSize * sizeof(sal_Unicode));

    delete[] pCharBuffer;
}

void SvPasswordHelper::GetHashPasswordBigEndian(uno::Sequence<sal_Int8>& rPassHash, const String& sPass)
{
    xub_StrLen nSize(sPass.Len());
    sal_Char* pCharBuffer = new sal_Char[nSize * sizeof(sal_Unicode)];

    for (xub_StrLen i = 0; i < nSize; ++i)
    {
        sal_Unicode ch(sPass.GetChar(i));
        pCharBuffer[2 * i] = static_cast< sal_Char >(ch >> 8);
        pCharBuffer[2 * i + 1] = static_cast< sal_Char >(ch & 0xFF);
    }

    GetHashPassword(rPassHash, pCharBuffer, nSize * sizeof(sal_Unicode));

    delete[] pCharBuffer;
}

void SvPasswordHelper::GetHashPassword(uno::Sequence<sal_Int8>& rPassHash, const String& sPass)
{
    GetHashPasswordLittleEndian(rPassHash, sPass);
}

bool SvPasswordHelper::CompareHashPassword(const uno::Sequence<sal_Int8>& rOldPassHash, const String& sNewPass)
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

