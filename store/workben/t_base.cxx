/*************************************************************************
 *
 *  $RCSfile: t_base.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:32 $
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

#define _T_BASE_CXX "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _STORE_FILELCKB_HXX_
#include <store/filelckb.hxx>
#endif

#ifndef _STORE_STORBASE_HXX_
#include "storbase.hxx"
#endif

#ifdef _USE_NAMESPACE
using namespace store;
#endif

#define TEST_PAGESIZE 1024

/*========================================================================
 *
 * OTestBIOS.
 *
 *======================================================================*/
class OTestBIOS : public NAMESPACE_STORE(OStorePageBIOS)
{
    typedef NAMESPACE_STORE(OStorePageBIOS) base;

public:
    OTestBIOS (void);

    virtual storeError initialize (
        ILockBytes      *pLockBytes,
        storeAccessMode  eAccessMode);

protected:
    virtual ~OTestBIOS (void);
};

OTestBIOS::OTestBIOS (void)
{
}

OTestBIOS::~OTestBIOS (void)
{
}

storeError OTestBIOS::initialize (
    ILockBytes *pLockBytes, storeAccessMode eAccessMode)
{
    storeError eErrCode = base::initialize (pLockBytes, eAccessMode);
    if (eErrCode != store_E_None)
    {
        if (eAccessMode == store_AccessReadWrite)
            return eErrCode;
        if (eAccessMode == store_AccessReadOnly)
            return eErrCode;
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        eErrCode = base::create (TEST_PAGESIZE);
    }
    return eErrCode;
}

/*========================================================================
 *
 * unicode.
 *
 *======================================================================*/
static void __store_string_newFromUnicode_WithLength (
    rtl_String **newString, const sal_Unicode *value, sal_Int32 length)
{
    rtl_uString2String (
        newString,
        value, length,
        RTL_TEXTENCODING_UTF8,
        OUSTRING_TO_OSTRING_CVTFLAGS);
}

static void __store_string_newFromUnicode (
    rtl_String **newString, const rtl_uString *value)
{
    __store_string_newFromUnicode_WithLength (
        newString, value->buffer, value->length);
}

static void __store_string_newFromUnicode (
    rtl_String **newString, const sal_Unicode *value)
{
    __store_string_newFromUnicode_WithLength (
        newString, value, rtl_ustr_getLength (value));
}

static storeError __store_namei (
    const NAMESPACE_RTL(OString) &rPath,
    const NAMESPACE_RTL(OString) &rName,
    OStorePageKey     &rKey)
{
}

static storeError __store_namei (
    const sal_Unicode *pszPath,
    const sal_Unicode *pszName,
    OStorePageKey     &rKey)
{
    NAMESPACE_RTL(OString) aName (
        pszName, rtl_ustr_getLength (pszName), RTL_TEXTENCODING_UTF8);


    rtl_String *pszNameA = 0;
    __store_string_newFromUnicode (&pszNameA, pszName);

    storeError eErrCode = store_E_NameTooLong;
    if (pszNameA->length < sizeof(sal_Char[STORE_MAXIMUM_NAMESIZE]))
    {
        rtl_String *pszPathA = 0;
        __store_string_newFromUnicode (&pszPathA, pszPath);

        typedef OStorePageGuard G;
        rKey.m_nLow  = G::crc32 (0, pszNameA->buffer, pszNameA->length);
        rKey.m_nHigh = G::crc32 (0, pszPathA->buffer, pszPathA->length);

        rtl_string_release (pszPathA);
        eErrCode = store_E_None;
    }

    rtl_string_release (pszNameA);
    return eErrCode;
}

static sal_Int32 __store_convertTextToUnicode (
    rtl_TextToUnicodeConverter hConvert,
    const sal_Char *pszText, sal_Int32 nTextLen,
    sal_Unicode    *pBuffer, sal_Int32 nBuffer)
{
    sal_uInt32 nInfo, nSrcLen = 0;

    sal_Int32 nDstLen = rtl_convertTextToUnicode (
        hConvert, 0,
        pszText, nTextLen,
        pBuffer, nBuffer,
        OSTRING_TO_OUSTRING_CVTFLAGS,
        &nInfo, &nSrcLen);

    pBuffer[nDstLen] = 0;
    return nDstLen;
}

struct MyFindData
{
    sal_Unicode m_pszName[STORE_MAXIMUM_NAMESIZE];
    sal_Int32   m_nLength;
    sal_uInt32  m_nAttrib;
    sal_uInt32  m_nSize;
    sal_uInt32  m_nReserved;
};

static void __store_testUnicode (const sal_Char *pszFilename)
{
    // ...
    rtl_TextToUnicodeConverter hConvert;
    hConvert = rtl_createTextToUnicodeConverter (RTL_TEXTENCODING_UTF8);

    MyFindData it;
    rtl_zeroMemory (&it, sizeof(it));

    sal_Int32 n = rtl_str_getLength (pszFilename);
    n = __store_convertTextToUnicode (
        hConvert, pszFilename, n,
        it.m_pszName, STORE_MAXIMUM_NAMESIZE - 1);
    if (it.m_nLength > n)
        rtl_zeroMemory (
            &it.m_pszName[n], ((it.m_nLength - n) * sizeof(sal_Unicode)));
    it.m_nLength = n;

    rtl_destroyTextToUnicodeConverter (hConvert);

    // ...
    rtl_String  *pszFileA = NULL;
    rtl_uString *pszFileW = NULL;

    // rtl_uString_newFromAscii (&pszFileW, pszFilename);

    // ...
    rtl_string_newFromStr (&pszFileA, pszFilename);

    rtl_string2UString (
        &pszFileW,
        pszFileA->buffer, pszFileA->length,
        RTL_TEXTENCODING_MS_1252,
        OSTRING_TO_OUSTRING_CVTFLAGS);

    rtl_string_release (pszFileA);

    // ...
    OStorePageKey aKey;
    __store_namei (pszFileW->buffer, pszFileW->buffer, aKey);

    // ...
    rtl_uString2String (
        &pszFileA,
        pszFileW->buffer, pszFileW->length,
        RTL_TEXTENCODING_UTF8,
        OUSTRING_TO_OSTRING_CVTFLAGS);

    rtl_uString_release (pszFileW);

    // ...
    rtl_string_release (pszFileA);
}

/*========================================================================
 *
 * __store_getProcessTextEncoding.
 *
 *======================================================================*/
inline rtl_TextEncoding __store_getProcessTextEncoding (void)
{
    rtl_TextEncoding eEncoding;
#if defined(SAL_OS2)
    eEncoding = RTL_TEXTENCODING_IBM850;
#elif defined(SAL_UNX)
    eEncoding = RTL_TEXTENCODING_ISO8859_1;
#elif defined(SAL_W32)
    eEncoding = RTL_TEXTENCODING_MS_1252;
#else
    eEncoding = RTL_TEXTENCODING_ASCII_US;
#endif
    return eEncoding;
}

/*========================================================================
 *
 * main.
 *
 *======================================================================*/
int SAL_CALL main (int argc, char **argv)
{
    if (argc < 2)
        return 0;

#if 0  /* EXP */
    __store_testUnicode (argv[1]);
#endif /* EXP */

    NAMESPACE_VOS(ORef)<OFileLockBytes> xLockBytes (new OFileLockBytes());
    if (!xLockBytes.isValid())
        return 0;

    NAMESPACE_RTL(OUString) aFilename (
        argv[1], rtl_str_getLength(argv[1]),
        __store_getProcessTextEncoding());

    storeError eErrCode = xLockBytes->create (
        aFilename.pData, store_AccessReadCreate);
    if (eErrCode != store_E_None)
        return eErrCode;

    NAMESPACE_VOS(ORef)<OTestBIOS> xBIOS (new OTestBIOS());
    if (!xBIOS.isValid())
        return 0;

    eErrCode = xBIOS->initialize (&*xLockBytes, store_AccessReadWrite);
    if (eErrCode != store_E_None)
    {
        // Check reason.
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        // Create.
        eErrCode = xBIOS->initialize (&*xLockBytes, store_AccessReadCreate);
        if (eErrCode != store_E_None)
            return eErrCode;
    }
    xLockBytes.unbind();

    sal_Char pBuffer[TEST_PAGESIZE];
    rtl_zeroMemory (pBuffer, sizeof (pBuffer));
    rtl_copyMemory (pBuffer, argv[0], rtl_str_getLength(argv[0]) + 1);

    eErrCode = xBIOS->acquireLock (TEST_PAGESIZE, sizeof(pBuffer));
    if (eErrCode != store_E_None)
        return eErrCode;

    eErrCode = xBIOS->write (TEST_PAGESIZE, pBuffer, sizeof (pBuffer));
    if (eErrCode != store_E_None)
    {
        xBIOS->releaseLock (TEST_PAGESIZE, sizeof(pBuffer));
        return eErrCode;
    }

    eErrCode = xBIOS->releaseLock (TEST_PAGESIZE, sizeof(pBuffer));
    if (eErrCode != store_E_None)
        return eErrCode;

    xBIOS.unbind();
    return 0;
}

