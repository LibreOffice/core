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


#include <registry/registry.hxx>

#include "keyimpl.hxx"
#include "regimpl.hxx"
#include "regkey.hxx"

#if defined(_WIN32)
#include <io.h>
#endif

extern "C" {


//  acquire

static void REGISTRY_CALLTYPE acquire(RegHandle hReg)
{
    ORegistry* pReg = static_cast<ORegistry*>(hReg);

    if (pReg != nullptr)
        pReg->acquire();
}


//  release

static void REGISTRY_CALLTYPE release(RegHandle hReg)
{
    ORegistry* pReg = static_cast<ORegistry*>(hReg);

    if (pReg && pReg->release() == 0)
    {
        delete pReg;
        hReg = nullptr;
    }
}


//  getName

static RegError REGISTRY_CALLTYPE getName(RegHandle hReg, rtl_uString** pName)
{
    if (hReg)
    {
        ORegistry*  pReg = static_cast<ORegistry*>(hReg);
        if ( pReg->isOpen() )
        {
            rtl_uString_assign(pName, pReg->getName().pData);
            return RegError::NO_ERROR;
        } else
        {
            rtl_uString_new(pName);
            return RegError::REGISTRY_NOT_OPEN;
        }
    }

    rtl_uString_new(pName);
    return RegError::INVALID_REGISTRY;
}


//  isReadOnly

static sal_Bool REGISTRY_CALLTYPE isReadOnly(RegHandle hReg)
{
    if (hReg)
        return static_cast<ORegistry*>(hReg)->isReadOnly();
    else
        return false;
}


//  createRegistry

static RegError REGISTRY_CALLTYPE createRegistry(rtl_uString* registryName,
                                                 RegHandle* phRegistry)
{
    RegError ret;

    ORegistry* pReg = new ORegistry();
    if ((ret = pReg->initRegistry(registryName, RegAccessMode::READWRITE, true/*bCreate*/)) != RegError::NO_ERROR)
    {
        delete pReg;
        *phRegistry = nullptr;
        return ret;
    }

    *phRegistry = pReg;

    return RegError::NO_ERROR;
}


//  openRootKey

static RegError REGISTRY_CALLTYPE openRootKey(RegHandle hReg,
                                              RegKeyHandle* phRootKey)
{
    ORegistry* pReg;

    if (hReg)
    {
        pReg = static_cast<ORegistry*>(hReg);
        if (!pReg->isOpen())
            return RegError::REGISTRY_NOT_OPEN;
    } else
    {
        phRootKey = nullptr;
        return RegError::INVALID_REGISTRY;
    }

    *phRootKey = pReg->getRootKey();

    return RegError::NO_ERROR;
}


//  openRegistry

static RegError REGISTRY_CALLTYPE openRegistry(rtl_uString* registryName,
                                               RegHandle* phRegistry,
                                               RegAccessMode accessMode)
{
    RegError _ret;

    ORegistry* pReg = new ORegistry();
    if ((_ret = pReg->initRegistry(registryName, accessMode)) != RegError::NO_ERROR)
    {
        *phRegistry = nullptr;
        delete pReg;
        return _ret;
    }


    *phRegistry = pReg;

    return RegError::NO_ERROR;
}


//  closeRegistry

static RegError REGISTRY_CALLTYPE closeRegistry(RegHandle hReg)
{
    ORegistry   *pReg;

    if (hReg)
    {
        pReg = static_cast<ORegistry*>(hReg);
        if (!pReg->isOpen())
            return RegError::REGISTRY_NOT_OPEN;

        RegError ret = RegError::NO_ERROR;
        if (pReg->release() == 0)
        {
            delete pReg;
            hReg = nullptr;
        }
        else
            ret = pReg->closeRegistry();

        return ret;
    } else
    {
        return RegError::INVALID_REGISTRY;
    }
}


//  destroyRegistry

static RegError REGISTRY_CALLTYPE destroyRegistry(RegHandle hReg,
                                                  rtl_uString* registryName)
{
    ORegistry   *pReg;

    if (hReg)
    {
        pReg = static_cast<ORegistry*>(hReg);
        if (!pReg->isOpen())
            return RegError::INVALID_REGISTRY;

        RegError ret = pReg->destroyRegistry(registryName);
        if (ret == RegError::NO_ERROR)
        {
            if (!registryName->length)
            {
                delete pReg;
                hReg = nullptr;
            }
        }
        return ret;
    } else
    {
        return RegError::INVALID_REGISTRY;
    }
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
        return RegError::INVALID_REGISTRY;
    if (!pReg->isOpen())
        return RegError::REGISTRY_NOT_OPEN;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;
    if (pKey->getRegistry() != pReg)
        return RegError::INVALID_KEY;
    if (pKey->isDeleted())
        return RegError::INVALID_KEY;
    if (pKey->isReadOnly())
        return RegError::REGISTRY_READONLY;

    if (keyName->length)
    {
        ORegKey* pNewKey = nullptr;
        RegError _ret = pKey->createKey(keyName, reinterpret_cast<RegKeyHandle*>(&pNewKey));
        if (_ret != RegError::NO_ERROR)
            return _ret;

        _ret = pReg->loadKey(pNewKey, regFileName, bWarnings, bReport);
        if (_ret != RegError::NO_ERROR && (_ret != RegError::MERGE_CONFLICT || bWarnings))
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
        return RegError::INVALID_REGISTRY;
    if (!pReg->isOpen())
        return RegError::REGISTRY_NOT_OPEN;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;
    if (pKey->getRegistry() != pReg)
        return RegError::INVALID_KEY;
    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    return pReg->dumpRegistry(hKey);
}


//  initRegistry_Api

Registry_Api* REGISTRY_CALLTYPE initRegistry_Api()
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
                               &mergeKey,
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
                               &getResolvedKeyName,
                               &getKeyNames,
                               &freeKeyNames};

    return (&aApi);
}

}


//  reg_openRootKey

RegError REGISTRY_CALLTYPE reg_openRootKey(RegHandle hRegistry,
                                          RegKeyHandle* phRootKey)
{
    return openRootKey(hRegistry, phRootKey);
}


//  reg_openRegistry

RegError REGISTRY_CALLTYPE reg_openRegistry(rtl_uString* registryName,
                                            RegHandle* phRegistry)
{
    RegError _ret;

    ORegistry* pReg = new ORegistry();
    if ((_ret = pReg->initRegistry(registryName, RegAccessMode::READONLY)) != RegError::NO_ERROR)
    {
        delete pReg;
        *phRegistry = nullptr;
        return _ret;
    }

    *phRegistry = pReg;

    return RegError::NO_ERROR;
}


//  reg_closeRegistry

RegError REGISTRY_CALLTYPE reg_closeRegistry(RegHandle hRegistry)
{
    if (hRegistry)
    {
        ORegistry* pReg = static_cast<ORegistry*>(hRegistry);
        delete pReg;
        return RegError::NO_ERROR;
    } else
    {
        return RegError::REGISTRY_NOT_OPEN;
    }
}


//  reg_dumpRegistry

RegError REGISTRY_CALLTYPE reg_dumpRegistry(RegKeyHandle hKey)
{
    ORegKey *pKey;

    if (hKey)
        pKey = static_cast<ORegKey*>(hKey);
    else
        return RegError::INVALID_KEY;

    return dumpRegistry(pKey->getRegistry(), hKey);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
