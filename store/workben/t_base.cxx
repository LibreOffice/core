/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_store.hxx"

#include "sal/types.h"
#include "osl/diagnose.h"
#include "osl/thread.h"
#include "rtl/memory.h"
#include "rtl/ustring.hxx"

#include "object.hxx"
#include "storbase.hxx"
#include "storbios.hxx"
#include "lockbyte.hxx"

using namespace store;

#define TEST_PAGESIZE 1024

/*========================================================================
 *
 * OTestObject.
 *
 *======================================================================*/
class OTestObject : public store::OStoreObject
{
public:
    OTestObject (void);

    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId);

protected:
    virtual ~OTestObject (void);
};

OTestObject::OTestObject (void)
{
}

OTestObject::~OTestObject (void)
{
}

sal_Bool SAL_CALL OTestObject::isKindOf (sal_uInt32 nTypeId)
{
    return (nTypeId == 42);
}

namespace store
{
static OTestObject* SAL_CALL query (IStoreHandle *pHandle, OTestObject*)
{
    if (pHandle && pHandle->isKindOf (42))
        return static_cast<OTestObject*>(pHandle);
    else
        return 0;
}
}

/*========================================================================
 *
 * OTestBIOS.
 *
 *======================================================================*/
namespace store
{

class OTestBIOS : public store::OStorePageBIOS
{
    typedef store::OStorePageBIOS base;

    friend OTestBIOS* SAL_CALL query<> (IStoreHandle * pHandle, OTestBIOS *);

public:
    OTestBIOS (void);

    virtual storeError initialize (
        ILockBytes *    pLockBytes,
        storeAccessMode eAccessMode,
        sal_uInt16 &    rnPageSize);

    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId);

protected:
    virtual ~OTestBIOS (void);
};

} // namespace store

OTestBIOS::OTestBIOS (void)
{
}

OTestBIOS::~OTestBIOS (void)
{
}

sal_Bool SAL_CALL OTestBIOS::isKindOf (sal_uInt32 nTypeId)
{
    return (nTypeId == 4242);
}

storeError OTestBIOS::initialize (
    ILockBytes *pLockBytes, storeAccessMode eAccessMode, sal_uInt16 & rnPageSize)
{
    return base::initialize (pLockBytes, eAccessMode, rnPageSize);
}

namespace store
{
template<> OTestBIOS* SAL_CALL query (IStoreHandle *pHandle, OTestBIOS*)
{
    if (pHandle && pHandle->isKindOf (4242))
        return static_cast<OTestBIOS*>(pHandle);
    else
        return 0;
}
}

/*========================================================================
 *
 * __store_test_handle.
 *
 *======================================================================*/
static void __store_test_handle (void* Handle)
{
    IStoreHandle *pHandle = static_cast<IStoreHandle*>(Handle);
    if (pHandle)
    {
        pHandle->acquire();
        pHandle->isKindOf (42);
        pHandle->release();
    }

    OTestObject *pObj = query (pHandle, static_cast<OTestObject*>(0));
    if (pObj)
    {
        pObj->acquire();
        pObj->isKindOf (42);
        pObj->release();
    }
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

#if 0  /* UNSUSED */
static void __store_string_newFromUnicode (
    rtl_String **newString, const rtl_uString *value)
{
    __store_string_newFromUnicode_WithLength (
        newString, value->buffer, value->length);
}
#endif /* UNUSED */

static void __store_string_newFromUnicode (
    rtl_String **newString, const sal_Unicode *value)
{
    __store_string_newFromUnicode_WithLength (
        newString, value, rtl_ustr_getLength (value));
}

static storeError __store_namei (
    const sal_Unicode *pszPath,
    const sal_Unicode *pszName,
    OStorePageKey     &rKey)
{
    rtl::OString aName (
        pszName, rtl_ustr_getLength (pszName), RTL_TEXTENCODING_UTF8);

    rtl_String *pszNameA = 0;
    __store_string_newFromUnicode (&pszNameA, pszName);

    storeError eErrCode = store_E_NameTooLong;
    if (pszNameA->length < sal_Int32(sizeof(sal_Char[STORE_MAXIMUM_NAMESIZE])))
    {
        rtl_String *pszPathA = 0;
        __store_string_newFromUnicode (&pszPathA, pszPath);

        rKey.m_nLow  = rtl_crc32 (0, pszNameA->buffer, pszNameA->length);
        rKey.m_nHigh = rtl_crc32 (0, pszPathA->buffer, pszPathA->length);

        rtl_string_release (pszPathA);
        eErrCode = store_E_None;
    }

    rtl_string_release (pszNameA);
    return eErrCode;
}

static sal_Size __store_convertTextToUnicode (
    rtl_TextToUnicodeConverter hConvert,
    const sal_Char *pszText, sal_Size nTextLen,
    sal_Unicode    *pBuffer, sal_Size nBuffer)
{
    sal_uInt32 nInfo = 0;
    sal_Size   nSrcLen = 0;

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
 * main.
 *
 *======================================================================*/
int SAL_CALL main (int argc, char **argv)
{
    OSL_PRECOND(argc > 1, "t_base: error: insufficient number of arguments.");
    if (argc < 2)
        return 0;

    __store_testUnicode (argv[1]);

    rtl::Reference<ILockBytes> xLockBytes;

    rtl::OUString aFilename (
        argv[1], rtl_str_getLength(argv[1]),
        osl_getThreadTextEncoding());

    storeError eErrCode = FileLockBytes_createInstance (
        xLockBytes, aFilename.pData, store_AccessReadCreate);
    if (eErrCode != store_E_None)
        return eErrCode;


    rtl::Reference<OTestObject> xObject (new OTestObject());
    __store_test_handle (&*xObject);

    rtl::Reference<OTestBIOS> xBIOS (new OTestBIOS());
    __store_test_handle (&*xBIOS);


    if (!xBIOS.is())
        return 0;

    sal_uInt16 nPageSize = TEST_PAGESIZE;
    eErrCode = xBIOS->initialize (&*xLockBytes, store_AccessReadWrite, nPageSize);
    if (eErrCode != store_E_None)
    {
        // Check reason.
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        // Create.
        eErrCode = xBIOS->initialize (&*xLockBytes, store_AccessReadCreate, nPageSize);
        if (eErrCode != store_E_None)
            return eErrCode;
    }
    xLockBytes.clear();

    sal_Char pBuffer[TEST_PAGESIZE];
    rtl_zeroMemory (pBuffer, sizeof (pBuffer));
    rtl_copyMemory (pBuffer, argv[0], rtl_str_getLength(argv[0]) + 1);

    eErrCode = xBIOS->write (TEST_PAGESIZE, pBuffer, sizeof (pBuffer));
    if (eErrCode != store_E_None)
        return eErrCode;

    xBIOS.clear();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
