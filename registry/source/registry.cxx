/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: registry.cxx,v $
 * $Revision: 1.20 $
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
#include "precompiled_registry.hxx"
#include <registry/registry.h>
#include <registry/registry.hxx>
#include <osl/process.h>
#include "regimpl.hxx"
#ifndef _REGKEY_HXX_
#include "keyimpl.hxx"
#endif
#include "regkey.hxx"

#if defined(WIN32) || defined(WNT) || defined(OS2)
#include <io.h>
#endif

#include <string.h>
#if defined(UNX)
#include <stdlib.h>
#include <unistd.h>
#endif

#if defined ( GCC ) && ( defined ( SCO ) )
ORealDynamicLoader* ODynamicLoader<Registry_Api>::m_pLoader = NULL;
#endif

OString getTempName()
{
    static OUString TMP(RTL_CONSTASCII_USTRINGPARAM("TMP"));
    static OUString TEMP(RTL_CONSTASCII_USTRINGPARAM("TEMP"));

    OUString    uTmpPattern;
    sal_Char    tmpPattern[512] = "";
    sal_Char    *pTmpName = NULL;

    if ( osl_getEnvironment(TMP.pData, &uTmpPattern.pData) != osl_Process_E_None )
    {
        if ( osl_getEnvironment(TEMP.pData, &uTmpPattern.pData) != osl_Process_E_None )
        {
#if defined(SAL_W32) || defined(SAL_OS2)
            OSL_ASSERT( sizeof(tmpPattern) > RTL_CONSTASCII_LENGTH( "." ) );
            strncpy(tmpPattern, ".", sizeof(tmpPattern)-1);
#else
            OSL_ASSERT( sizeof(tmpPattern) > RTL_CONSTASCII_LENGTH( "." ) );
            strncpy(tmpPattern, ".", sizeof(tmpPattern)-1);
#endif
        }
    }

    if ( uTmpPattern.getLength() )
    {
        OString aOStr( OUStringToOString(uTmpPattern, RTL_TEXTENCODING_UTF8) );
        OSL_ASSERT(
            sizeof(tmpPattern)
            > sal::static_int_cast< sal_uInt32 >(aOStr.getLength()) );
        strncpy(tmpPattern, aOStr.getStr(), sizeof(tmpPattern)-1);
    }

#if defined(WIN32) || defined(WNT) || defined(OS2)
    OSL_ASSERT( sizeof(tmpPattern) > ( strlen(tmpPattern)
                                       + RTL_CONSTASCII_LENGTH("\\reg_XXXXXX") ) );
    strncat(tmpPattern, "\\reg_XXXXXX", sizeof(tmpPattern)-1-strlen(tmpPattern));
    pTmpName = mktemp(tmpPattern);
#endif

#ifdef OS2__YD
    char* tmpname = tempnam(NULL, "reg_");
    OSL_ASSERT( sizeof(tmpPattern) > strlen(tmpname) );
    strncpy(tmpPattern, tmpname, sizeof(tmpPattern)-1);
    pTmpName = tmpPattern;
#endif

#ifdef UNX
    OSL_ASSERT( sizeof(tmpPattern) > ( strlen(tmpPattern)
                                       + RTL_CONSTASCII_LENGTH("/reg_XXXXXX") ) );
    strncat(tmpPattern, "/reg_XXXXXX", sizeof(tmpPattern)-1-strlen(tmpPattern));

    pTmpName = mktemp(tmpPattern);
#endif

    return OString(pTmpName);
}

extern "C" {

//*********************************************************************
//  acquire
//
static void REGISTRY_CALLTYPE acquire(RegHandle hReg)
{
    ORegistry* pReg = (ORegistry*) hReg;

    if (pReg != NULL)
        pReg->acquire();
}


//*********************************************************************
//  release
//
static void REGISTRY_CALLTYPE release(RegHandle hReg)
{
    ORegistry* pReg = (ORegistry*) hReg;

    if (pReg)
    {
        if (pReg->release() == 0)
        {
            delete pReg;
            hReg = NULL;
        }
    }
}


//*********************************************************************
//  getName
//
static RegError REGISTRY_CALLTYPE getName(RegHandle hReg, rtl_uString** pName)
{
    ORegistry*  pReg;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if ( pReg->isOpen() )
        {
            rtl_uString_assign(pName, pReg->getName().pData);
            return REG_NO_ERROR;
        } else
        {
            rtl_uString_new(pName);
            return REG_REGISTRY_NOT_OPEN;
        }
    }

    rtl_uString_new(pName);
    return REG_INVALID_REGISTRY;
}


//*********************************************************************
//  isReadOnly
//
static sal_Bool REGISTRY_CALLTYPE isReadOnly(RegHandle hReg)
{
    if (hReg)
        return ((ORegistry*)hReg)->isReadOnly();
    else
        return sal_False;
}


//*********************************************************************
//  createRegistry
//
static RegError REGISTRY_CALLTYPE createRegistry(rtl_uString* registryName,
                                                 RegHandle* phRegistry)
{
    RegError ret;

    ORegistry* pReg = new ORegistry();
    if ((ret = pReg->initRegistry(registryName, REG_CREATE)))
    {
        *phRegistry = NULL;
        return ret;
    }

    *phRegistry = pReg;

    return REG_NO_ERROR;
}

//*********************************************************************
//  openRootKey
//
static RegError REGISTRY_CALLTYPE openRootKey(RegHandle hReg,
                                              RegKeyHandle* phRootKey)
{
    ORegistry* pReg;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;
    } else
    {
        phRootKey = NULL;
        return REG_INVALID_REGISTRY;
    }

    *phRootKey = pReg->getRootKey();

    return REG_NO_ERROR;
}


//*********************************************************************
//  openRegistry
//
static RegError REGISTRY_CALLTYPE openRegistry(rtl_uString* registryName,
                                               RegHandle* phRegistry,
                                               RegAccessMode accessMode)
{
    RegError _ret;

    ORegistry* pReg = new ORegistry();
    if ((_ret = pReg->initRegistry(registryName, accessMode)))
    {
        *phRegistry = NULL;
        delete pReg;
        return _ret;
    }


    *phRegistry = pReg;

    return REG_NO_ERROR;
}

//*********************************************************************
//  closeRegistry
//
static RegError REGISTRY_CALLTYPE closeRegistry(RegHandle hReg)
{
    ORegistry   *pReg;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;

        RegError ret = REG_NO_ERROR;
        if (pReg->release() == 0)
        {
            delete(pReg);
            hReg = NULL;
        }
        else
            ret = pReg->closeRegistry();

        return ret;
    } else
    {
        return REG_INVALID_REGISTRY;
    }
}


//*********************************************************************
//  destroyRegistry
//
static RegError REGISTRY_CALLTYPE destroyRegistry(RegHandle hReg,
                                                  rtl_uString* registryName)
{
    ORegistry   *pReg;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_INVALID_REGISTRY;

        RegError ret = pReg->destroyRegistry(registryName);
        if (!ret)
        {
            if (!registryName->length)
            {
                delete(pReg);
                hReg = NULL;
            }
        }
        return ret;
    } else
    {
        return REG_INVALID_REGISTRY;
    }
}


//*********************************************************************
//  loadRegKey
//
static RegError REGISTRY_CALLTYPE loadKey(RegHandle hReg,
                                              RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName)
{
    ORegistry   *pReg;
    ORegKey     *pKey, *pNewKey;
    RegError    _ret;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;
    } else
    {
        return REG_INVALID_REGISTRY;
    }

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->getRegistry() != pReg || pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
    {
        return REG_INVALID_KEY;
    }

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    if (!pKey->openKey(keyName, (RegKeyHandle*)&pNewKey))
    {
        pKey->closeKey(pNewKey);
        pKey->deleteKey(keyName);
    }

    if ((_ret = pKey->createKey(keyName, (RegKeyHandle*)&pNewKey)))
        return _ret;

    if ((_ret = pReg->loadKey(pNewKey, regFileName)))
    {
        pKey->closeKey(pNewKey);
        pKey->deleteKey(keyName);
        return _ret;
    }

    return pKey->closeKey(pNewKey);
}

//*********************************************************************
//  saveKey
//
static RegError REGISTRY_CALLTYPE saveKey(RegHandle hReg,
                                              RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName)
{
    ORegistry   *pReg;
    ORegKey     *pKey, *pNewKey;
    RegError    _ret;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;
    } else
    {
        return REG_INVALID_REGISTRY;
    }

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->getRegistry() != pReg || pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
    {
        return REG_INVALID_KEY;
    }

    if ((_ret = pKey->openKey(keyName, (RegKeyHandle*)&pNewKey)))
        return _ret;

    if ((_ret = pReg->saveKey(pNewKey, regFileName)))
    {
        pKey->closeKey(pNewKey);
        return _ret;
    }

    return pKey->closeKey(pNewKey);
}

//*********************************************************************
//  mergeKey
//
static RegError REGISTRY_CALLTYPE mergeKey(RegHandle hReg,
                                              RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName,
                                           sal_Bool bWarnings,
                                           sal_Bool bReport)
{
    ORegistry   *pReg;
    ORegKey     *pKey, *pNewKey;
    RegError    _ret;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return(REG_REGISTRY_NOT_OPEN);
    } else
    {
        return REG_INVALID_REGISTRY;
    }

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->getRegistry() != pReg || pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
    {
        return REG_INVALID_KEY;
    }

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    if (keyName->length)
    {
        if ((_ret = pKey->createKey(keyName, (RegKeyHandle*)&pNewKey) ))
            return _ret;
    } else
    {
        pNewKey = pKey;
    }

    /*
    OString tmpName = getTempName();
    pReg->saveKey(pNewKey, tmpName, sal_False, sal_False);
    */
    _ret = pKey->getRegistry()->loadKey(pNewKey, regFileName, bWarnings, bReport);

    if (_ret == REG_MERGE_ERROR ||
        (_ret == REG_MERGE_CONFLICT && bWarnings))
    {
        pKey->closeKey(pNewKey);
        return _ret;
    }

    /*
    if (_ret)
    {
        if (_ret == REG_MERGE_ERROR ||
            (_ret == REG_MERGE_CONFLICT && bWarnings))
        {
            pKey->closeKey(pNewKey);
            pKey->deleteKey(keyName);
            pKey->createKey(keyName, (RegKeyHandle*)&pNewKey);
            pReg->loadKey(pNewKey, tmpName, sal_False, sal_False);
        }

        destroyRegistry(hReg, tmpName);

        pKey->closeKey(pNewKey);
        return _ret;
    }

    destroyRegistry(hReg, tmpName);
    */
    _ret = pKey->closeKey(pNewKey);

    return _ret;
}


//*********************************************************************
//  dumpRegistry
//
static RegError REGISTRY_CALLTYPE dumpRegistry(RegHandle hReg,
                                               RegKeyHandle hKey)
{
    ORegistry   *pReg;
    ORegKey     *pKey;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;
    } else
    {
        return REG_INVALID_REGISTRY;
    }

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->getRegistry() != pReg || pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
    {
        return REG_INVALID_KEY;
    }

    return pReg->dumpRegistry(hKey);
}


//*********************************************************************
//  initRegistry_Api
//
Registry_Api* REGISTRY_CALLTYPE initRegistry_Api(void)
{
    static Registry_Api aApi= {&acquire,
                               &release,
                               &isReadOnly,
                               &openRootKey,
                               &getName,
                               &createRegistry,
                               &openRegistry,
                               &closeRegistry,
                               &destroyRegistry,
                               &loadKey,
                               &saveKey,
                               &mergeKey,
                               &dumpRegistry,
                               &acquireKey,
                               &releaseKey,
                               &isKeyReadOnly,
                               &getKeyName,
                               &createKey,
                               &openKey,
                               &openSubKeys,
                               &closeSubKeys,
                               &deleteKey,
                               &closeKey,
                               &setValue,
                               &setLongListValue,
                               &setStringListValue,
                               &setUnicodeListValue,
                               &getValueInfo,
                               &getValue,
                               &getLongListValue,
                               &getStringListValue,
                               &getUnicodeListValue,
                               &freeValueList,
                               &createLink,
                               &deleteLink,
                               &getKeyType,
                               &getLinkTarget,
                               &getResolvedKeyName,
                               &getKeyNames,
                               &freeKeyNames};

    return (&aApi);
}

}

//*********************************************************************
//  reg_loadRegKey
//
RegError REGISTRY_CALLTYPE reg_loadKey(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       rtl_uString* regFileName)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return loadKey(pKey->getRegistry(), hKey, keyName, regFileName);
}

//*********************************************************************
//  reg_saveKey
//
RegError REGISTRY_CALLTYPE reg_saveKey(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       rtl_uString* regFileName)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return saveKey(pKey->getRegistry(), hKey, keyName, regFileName);
}

//*********************************************************************
//  reg_mergeKey
//
RegError REGISTRY_CALLTYPE reg_mergeKey(RegKeyHandle hKey,
                                        rtl_uString* keyName,
                                        rtl_uString* regFileName,
                                        sal_Bool bWarnings,
                                        sal_Bool bReport)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return mergeKey(pKey->getRegistry(), hKey, keyName, regFileName, bWarnings, bReport);
}

//*********************************************************************
//  reg_createRegistry
//
RegError REGISTRY_CALLTYPE reg_createRegistry(rtl_uString* registryName,
                                              RegHandle* phRegistry)
{
    RegError ret;

    ORegistry* pReg = new ORegistry();
    if ((ret = pReg->initRegistry(registryName, REG_CREATE)))
    {
        *phRegistry = NULL;
        return ret;
    }

    *phRegistry = pReg;

    return REG_NO_ERROR;
}

//*********************************************************************
//  reg_openRootKey
//
RegError REGISTRY_CALLTYPE reg_openRootKey(RegHandle hRegistry,
                                          RegKeyHandle* phRootKey)
{
    return openRootKey(hRegistry, phRootKey);
}


//*********************************************************************
//  reg_getName
//
const RegError REGISTRY_CALLTYPE reg_getName(RegHandle hRegistry, rtl_uString** pName)
{
    return getName(hRegistry, pName);
}


//*********************************************************************
//  reg_isReadOnly
//
sal_Bool REGISTRY_CALLTYPE reg_isReadOnly(RegHandle hRegistry)
{
    return isReadOnly(hRegistry);
}


//*********************************************************************
//  reg_openRegistry
//
RegError REGISTRY_CALLTYPE reg_openRegistry(rtl_uString* registryName,
                                            RegHandle* phRegistry,
                                            RegAccessMode accessMode)
{
    RegError _ret;

    ORegistry* pReg = new ORegistry();
    if ((_ret = pReg->initRegistry(registryName, accessMode)))
    {
        *phRegistry = NULL;
        return _ret;
    }

    *phRegistry = pReg;

    return REG_NO_ERROR;
}

//*********************************************************************
//  reg_closeRegistry
//
RegError REGISTRY_CALLTYPE reg_closeRegistry(RegHandle hRegistry)
{
    ORegistry* pReg;

    if (hRegistry)
    {
        pReg = (ORegistry*)hRegistry;
        delete(pReg);
        return REG_NO_ERROR;
    } else
    {
        return REG_REGISTRY_NOT_OPEN;
    }
}


//*********************************************************************
//  reg_destroyRegistry
//
RegError REGISTRY_CALLTYPE reg_destroyRegistry(RegHandle hRegistry,
                                               rtl_uString* registryName)
{
    return destroyRegistry(hRegistry, registryName);
}


//*********************************************************************
//  reg_dumpRegistry
//
RegError REGISTRY_CALLTYPE reg_dumpRegistry(RegKeyHandle hKey)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return dumpRegistry(pKey->getRegistry(), hKey);
}


