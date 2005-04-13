/*************************************************************************
 *
 *  $RCSfile: PasswordHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:26:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#ifndef GCC
#pragma hdrstop
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

