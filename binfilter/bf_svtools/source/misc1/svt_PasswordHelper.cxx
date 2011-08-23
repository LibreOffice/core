/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove



#ifndef GCC
#endif

#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include "PasswordHelper.hxx"
#endif

#ifndef _RTL_DIGEST_H_
#include <rtl/digest.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

using namespace com::sun::star;

namespace binfilter
{

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

}
