/*************************************************************************
 *
 *  $RCSfile: t_base.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-13 21:15:30 $
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
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#define _T_BASE_CXX "$Revision: 1.3 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _STORE_FILELCKB_HXX_
#include <store/filelckb.hxx>
#endif

#ifndef _STORE_STORBASE_HXX_
#include "storbase.hxx"
#endif

using namespace store;

#define TEST_PAGESIZE 1024

/*========================================================================
 *
 * OTestDaemon.
 *
 *======================================================================*/

#if 1  /* EXP */

#include <osl/interlck.h>

class OTestDaemon : public rtl::IReference
{
public:
    static sal_Bool getOrCreate (
        rtl::Reference<OTestDaemon> &rxDaemon);

    virtual oslInterlockedCount SAL_CALL acquire (void);
    virtual oslInterlockedCount SAL_CALL release (void);

protected:
    OTestDaemon (void);
    virtual ~OTestDaemon (void);

private:
    static OTestDaemon *m_pThis;

    oslInterlockedCount m_nRefCount;
};

#include <osl/mutex.hxx>

OTestDaemon* OTestDaemon::m_pThis = 0;

OTestDaemon::OTestDaemon (void)
{
}

OTestDaemon::~OTestDaemon (void)
{
}

sal_Bool OTestDaemon::getOrCreate (rtl::Reference<OTestDaemon> &rxDaemon)
{
    osl::MutexGuard aGuard (osl::Mutex::getGlobalMutex());

    rxDaemon = m_pThis;
    if (!rxDaemon.is())
    {
        m_pThis  = new OTestDaemon();
        rxDaemon = m_pThis;
    }
    return rxDaemon.is();
}

oslInterlockedCount SAL_CALL OTestDaemon::acquire (void)
{
    return osl_incrementInterlockedCount (&m_nRefCount);
}

oslInterlockedCount SAL_CALL OTestDaemon::release (void)
{
    oslInterlockedCount result;

    result = osl_decrementInterlockedCount (&m_nRefCount);
    if (result == 0)
    {
        osl::MutexGuard aGuard (osl::Mutex::getGlobalMutex());
        if (m_nRefCount == 0)
        {
            m_pThis = 0;
            delete this;
        }
    }
    return (result);
}

#endif /* EXP */

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
class OTestBIOS : public store::OStorePageBIOS
{
    typedef store::OStorePageBIOS base;

public:
    OTestBIOS (void);

    virtual storeError initialize (
        ILockBytes      *pLockBytes,
        storeAccessMode  eAccessMode);

    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId);

protected:
    virtual ~OTestBIOS (void);
};

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

namespace store
{
static OTestBIOS* SAL_CALL query (IStoreHandle *pHandle, OTestBIOS*)
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

    store::OStoreHandle<OTestObject> xObj (
        store::OStoreHandle<OTestObject>::query (Handle));
    if (xObj.is())
    {
        xObj->acquire();
        xObj->isKindOf (42);
        xObj->release();
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
    const rtl::OString &rPath,
    const rtl::OString &rName,
    OStorePageKey      &rKey)
{
    return store_E_Unknown;
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

    rtl::Reference<OFileLockBytes> xLockBytes (new OFileLockBytes());
    if (!xLockBytes.is())
        return 0;

    rtl::OUString aFilename (
        argv[1], rtl_str_getLength(argv[1]),
        osl_getThreadTextEncoding());

    storeError eErrCode = xLockBytes->create (
        aFilename.pData, store_AccessReadCreate);
    if (eErrCode != store_E_None)
        return eErrCode;


    rtl::Reference<OTestObject> xObject (new OTestObject());
    __store_test_handle (&*xObject);

    rtl::Reference<OTestBIOS> xBIOS (new OTestBIOS());
    __store_test_handle (&*xBIOS);


    if (!xBIOS.is())
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
    xLockBytes.clear();

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

    xBIOS.clear();
    return 0;
}

