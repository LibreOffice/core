/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PasswordHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:14:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"


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

