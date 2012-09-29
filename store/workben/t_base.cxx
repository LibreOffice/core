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


#include <string.h>
#include "sal/types.h"
#include "osl/diagnose.h"
#include "osl/thread.h"
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
    memset (&it, 0, sizeof(it));

    sal_Int32 n = rtl_str_getLength (pszFilename);
    n = __store_convertTextToUnicode (
        hConvert, pszFilename, n,
        it.m_pszName, STORE_MAXIMUM_NAMESIZE - 1);
    if (it.m_nLength > n)
        memset (
            &it.m_pszName[n], 0, ((it.m_nLength - n) * sizeof(sal_Unicode)));
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
    memset (pBuffer, 0, sizeof (pBuffer));
    memcpy (pBuffer, argv[0], rtl_str_getLength(argv[0]) + 1);

    eErrCode = xBIOS->write (TEST_PAGESIZE, pBuffer, sizeof (pBuffer));
    if (eErrCode != store_E_None)
        return eErrCode;

    xBIOS.clear();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
