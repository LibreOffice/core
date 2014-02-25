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


#include <registry/registry.h>
#include <registry/registry.hxx>

#include <osl/process.h>

#include "keyimpl.hxx"
#include "regimpl.hxx"
#include "regkey.hxx"

#if defined(WIN32) || defined(WNT)
#include <io.h>
#endif

#include <string.h>
#if defined(UNX)
#include <stdlib.h>
#include <unistd.h>
#endif

extern "C" {


//  acquire

static void REGISTRY_CALLTYPE acquire(RegHandle hReg)
{
    ORegistry* pReg = (ORegistry*) hReg;

    if (pReg != NULL)
        pReg->acquire();
}



//  release

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



//  getName

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



//  isReadOnly

static sal_Bool REGISTRY_CALLTYPE isReadOnly(RegHandle hReg)
{
    if (hReg)
        return ((ORegistry*)hReg)->isReadOnly();
    else
        return sal_False;
}



//  createRegistry

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


//  openRootKey

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



//  openRegistry

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


//  closeRegistry

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



//  destroyRegistry

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



//  loadRegKey

static RegError REGISTRY_CALLTYPE loadKey(RegHandle hReg,
                                              RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName)
{

    ORegistry* pReg = static_cast< ORegistry* >(hReg);
    if (!pReg)
        return REG_INVALID_REGISTRY;

    if (!pReg->isOpen())
        return REG_REGISTRY_NOT_OPEN;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return REG_INVALID_KEY;

    if (pKey->getRegistry() != pReg)
        return REG_INVALID_KEY;
    if (pKey->isDeleted())
        return REG_INVALID_KEY;
    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;


    ORegKey* pNewKey = 0;
    RegError _ret = pKey->openKey(keyName, (RegKeyHandle*)&pNewKey);
    if (_ret == REG_NO_ERROR)
    {
        pKey->releaseKey(pNewKey);
        pKey->deleteKey(keyName);
    }

    _ret = pKey->createKey(keyName, (RegKeyHandle*)&pNewKey);
    if (_ret != REG_NO_ERROR)
        return _ret;

    _ret = pReg->loadKey(pNewKey, regFileName);
    if (_ret != REG_NO_ERROR)
    {
        pKey->releaseKey(pNewKey);
        pKey->deleteKey(keyName);
        return _ret;
    }

    return pKey->closeKey(pNewKey);
}


//  saveKey

static RegError REGISTRY_CALLTYPE saveKey(RegHandle hReg,
                                              RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName)
{

    ORegistry* pReg = static_cast< ORegistry* >(hReg);
    if (!pReg)
        return REG_INVALID_REGISTRY;

    if (!pReg->isOpen())
        return REG_REGISTRY_NOT_OPEN;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return REG_INVALID_KEY;

    if (pKey->getRegistry() != pReg)
        return REG_INVALID_KEY;
    if (pKey->isDeleted())
        return REG_INVALID_KEY;

    ORegKey* pNewKey = 0;
    RegError _ret = pKey->openKey(keyName, (RegKeyHandle*)&pNewKey);
    if (_ret != REG_NO_ERROR)
        return _ret;

    _ret = pReg->saveKey(pNewKey, regFileName);
    if (_ret != REG_NO_ERROR)
    {
        (void) pKey->releaseKey(pNewKey);
        return _ret;
    }

    return pKey->releaseKey(pNewKey);
}


//  mergeKey

static RegError REGISTRY_CALLTYPE mergeKey(RegHandle hReg,
                                              RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName,
                                           sal_Bool bWarnings,
                                           sal_Bool bReport)
{
    ORegistry* pReg = static_cast< ORegistry* >(hReg);
    if (!pReg)
        return REG_INVALID_REGISTRY;
    if (!pReg->isOpen())
        return REG_REGISTRY_NOT_OPEN;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return REG_INVALID_KEY;
    if (pKey->getRegistry() != pReg)
        return REG_INVALID_KEY;
    if (pKey->isDeleted())
        return REG_INVALID_KEY;
    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    if (keyName->length)
    {
        ORegKey* pNewKey = 0;
        RegError _ret = pKey->createKey(keyName, (RegKeyHandle*)&pNewKey);
        if (_ret != REG_NO_ERROR)
            return _ret;

        _ret = pReg->loadKey(pNewKey, regFileName, bWarnings, bReport);
        if (_ret != REG_NO_ERROR && (_ret != REG_MERGE_CONFLICT || bWarnings))
        {
            if (pNewKey != pKey)
                (void) pKey->closeKey(pNewKey);
            else
                (void) pKey->releaseKey(pNewKey);
            return _ret;
        }

        return (pNewKey != pKey) ? pKey->closeKey(pNewKey) : pKey->releaseKey(pNewKey);
    }

    return pReg->loadKey(pKey, regFileName, bWarnings, bReport);
}


//  dumpRegistry

static RegError REGISTRY_CALLTYPE dumpRegistry(RegHandle hReg,
                                               RegKeyHandle hKey)
{
    ORegistry* pReg = static_cast< ORegistry* >(hReg);
    if (!pReg)
        return REG_INVALID_REGISTRY;
    if (!pReg->isOpen())
        return REG_REGISTRY_NOT_OPEN;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return REG_INVALID_KEY;
    if (pKey->getRegistry() != pReg)
        return REG_INVALID_KEY;
    if (pKey->isDeleted())
        return REG_INVALID_KEY;

    return pReg->dumpRegistry(hKey);
}


//  initRegistry_Api

REG_DLLPUBLIC Registry_Api* REGISTRY_CALLTYPE initRegistry_Api(void)
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


//  reg_loadRegKey

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_loadKey(RegKeyHandle hKey,
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


//  reg_saveKey

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_saveKey(RegKeyHandle hKey,
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


//  reg_mergeKey

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_mergeKey(RegKeyHandle hKey,
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


//  reg_createRegistry

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_createRegistry(rtl_uString* registryName,
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


//  reg_openRootKey

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_openRootKey(RegHandle hRegistry,
                                          RegKeyHandle* phRootKey)
{
    return openRootKey(hRegistry, phRootKey);
}



//  reg_getName

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_getName(RegHandle hRegistry, rtl_uString** pName)
{
    return getName(hRegistry, pName);
}



//  reg_isReadOnly

REG_DLLPUBLIC sal_Bool REGISTRY_CALLTYPE reg_isReadOnly(RegHandle hRegistry)
{
    return isReadOnly(hRegistry);
}



//  reg_openRegistry

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_openRegistry(rtl_uString* registryName,
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


//  reg_closeRegistry

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_closeRegistry(RegHandle hRegistry)
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



//  reg_destroyRegistry

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_destroyRegistry(RegHandle hRegistry,
                                               rtl_uString* registryName)
{
    return destroyRegistry(hRegistry, registryName);
}



//  reg_dumpRegistry

REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_dumpRegistry(RegKeyHandle hKey)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return dumpRegistry(pKey->getRegistry(), hKey);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
