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


#include "regkey.hxx"

#include <osl/diagnose.h>
#include "regimpl.hxx"
#include "keyimpl.hxx"


//  acquireKey

void REGISTRY_CALLTYPE acquireKey(RegKeyHandle hKey)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (pKey != nullptr)
    {
        ORegistry* pReg = pKey->getRegistry();
        (void) pReg->acquireKey(pKey);
    }
}


//  releaseKey

void REGISTRY_CALLTYPE releaseKey(RegKeyHandle hKey)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (pKey != nullptr)
    {
        ORegistry* pReg = pKey->getRegistry();
        (void) pReg->releaseKey(pKey);
    }
}


//  isKeyReadOnly

sal_Bool REGISTRY_CALLTYPE isKeyReadOnly(RegKeyHandle hKey)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    return pKey != nullptr && pKey->isReadOnly();
}


//  getKeyName

RegError REGISTRY_CALLTYPE getKeyName(RegKeyHandle hKey, rtl_uString** pKeyName)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (pKey)
    {
        rtl_uString_assign( pKeyName, pKey->getName().pData );
        return RegError::NO_ERROR;
    } else
    {
        rtl_uString_new(pKeyName);
        return RegError::INVALID_KEY;
    }
}


//  createKey

RegError REGISTRY_CALLTYPE createKey(RegKeyHandle hKey,
                                     rtl_uString* keyName,
                                     RegKeyHandle* phNewKey)
{
    *phNewKey = nullptr;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    if (pKey->isReadOnly())
        return RegError::REGISTRY_READONLY;

    return pKey->createKey(keyName, phNewKey);
}


//  openKey

RegError REGISTRY_CALLTYPE openKey(RegKeyHandle hKey,
                                   rtl_uString* keyName,
                                   RegKeyHandle* phOpenKey)
{
    *phOpenKey = nullptr;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    return pKey->openKey(keyName, phOpenKey);
}


//  openSubKeys

RegError REGISTRY_CALLTYPE openSubKeys(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       RegKeyHandle** pphSubKeys,
                                       sal_uInt32* pnSubKeys)
{
    *pphSubKeys = nullptr;
    *pnSubKeys = 0;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    return pKey->openSubKeys(keyName, pphSubKeys, pnSubKeys);
}


//  closeSubKeys

RegError REGISTRY_CALLTYPE closeSubKeys(RegKeyHandle* phSubKeys,
                                        sal_uInt32 nSubKeys)
{
    if (phSubKeys == nullptr || nSubKeys == 0)
        return RegError::INVALID_KEY;

    ORegistry* pReg = static_cast<ORegKey*>(phSubKeys[0])->getRegistry();
    for (sal_uInt32 i = 0; i < nSubKeys; i++)
    {
        (void) pReg->closeKey(phSubKeys[i]);
    }
    std::free(phSubKeys);

    return RegError::NO_ERROR;
}


//  deleteKey

RegError REGISTRY_CALLTYPE deleteKey(RegKeyHandle hKey,
                                     rtl_uString* keyName)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    if (pKey->isReadOnly())
        return RegError::REGISTRY_READONLY;

    return pKey->deleteKey(keyName);
}


//  closeKey

RegError REGISTRY_CALLTYPE closeKey(RegKeyHandle hKey)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    return pKey->closeKey(hKey);
}


//  setValue

RegError REGISTRY_CALLTYPE setValue(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       RegValueType valueType,
                                       RegValue pData,
                                       sal_uInt32 valueSize)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    if (pKey->isReadOnly())
        return RegError::REGISTRY_READONLY;

    OUString valueName("value");
    if (keyName->length)
    {
        ORegKey* pSubKey = nullptr;
        RegError _ret1 = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pSubKey));
        if (_ret1 != RegError::NO_ERROR)
            return _ret1;

        _ret1 = pSubKey->setValue(valueName, valueType, pData, valueSize);
        if (_ret1 != RegError::NO_ERROR)
        {
            RegError _ret2 = pKey->closeKey(pSubKey);
            if (_ret2 != RegError::NO_ERROR)
                return _ret2;
            else
                return _ret1;
        }

        return pKey->closeKey(pSubKey);
    }

    return pKey->setValue(valueName, valueType, pData, valueSize);
}


//  setLongValueList

RegError REGISTRY_CALLTYPE setLongListValue(RegKeyHandle hKey,
                                                  rtl_uString* keyName,
                                                  sal_Int32 const * pValueList,
                                                  sal_uInt32 len)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    if (pKey->isReadOnly())
        return RegError::REGISTRY_READONLY;

    OUString valueName("value");
    if (keyName->length)
    {
        ORegKey* pSubKey = nullptr;
        RegError _ret1 = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pSubKey));
        if (_ret1 != RegError::NO_ERROR)
            return _ret1;

        _ret1 = pSubKey->setLongListValue(valueName, pValueList, len);
        if (_ret1 != RegError::NO_ERROR)
        {
            RegError _ret2 = pKey->closeKey(pSubKey);
            if (_ret2 != RegError::NO_ERROR)
                return _ret2;
            else
                return _ret1;
        }

        return pKey->closeKey(pSubKey);
    }

    return pKey->setLongListValue(valueName, pValueList, len);
}


//  setStringValueList

RegError REGISTRY_CALLTYPE setStringListValue(RegKeyHandle hKey,
                                                   rtl_uString* keyName,
                                                   sal_Char** pValueList,
                                                   sal_uInt32 len)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    if (pKey->isReadOnly())
        return RegError::REGISTRY_READONLY;

    OUString valueName("value");
    if (keyName->length)
    {
        ORegKey* pSubKey = nullptr;
        RegError _ret1 = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pSubKey));
        if (_ret1 != RegError::NO_ERROR)
            return _ret1;

        _ret1 = pSubKey->setStringListValue(valueName, pValueList, len);
        if (_ret1 != RegError::NO_ERROR)
        {
            RegError _ret2 = pKey->closeKey(pSubKey);
            if (_ret2 != RegError::NO_ERROR)
                return _ret2;
            else
                return _ret1;
        }

        return pKey->closeKey(pSubKey);
    }

    return pKey->setStringListValue(valueName, pValueList, len);
}


//  setUnicodeValueList

RegError REGISTRY_CALLTYPE setUnicodeListValue(RegKeyHandle hKey,
                                                     rtl_uString* keyName,
                                                     sal_Unicode** pValueList,
                                                     sal_uInt32 len)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    if (pKey->isReadOnly())
        return RegError::REGISTRY_READONLY;

    OUString valueName("value");
    if (keyName->length)
    {
        ORegKey* pSubKey = nullptr;
        RegError _ret1 = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pSubKey));
        if (_ret1 != RegError::NO_ERROR)
            return _ret1;

        _ret1 = pSubKey->setUnicodeListValue(valueName, pValueList, len);
        if (_ret1 != RegError::NO_ERROR)
        {
            RegError _ret2 = pKey->closeKey(pSubKey);
            if (_ret2 != RegError::NO_ERROR)
                return _ret2;
            else
                return _ret1;
        }

        return pKey->closeKey(pSubKey);
    }

    return pKey->setUnicodeListValue(valueName, pValueList, len);
}


//  getValueInfo

RegError REGISTRY_CALLTYPE getValueInfo(RegKeyHandle hKey,
                                        rtl_uString* keyName,
                                        RegValueType* pValueType,
                                        sal_uInt32* pValueSize)
{
    *pValueType = RegValueType::NOT_DEFINED;
    *pValueSize = 0;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    RegValueType valueType;
    sal_uInt32   valueSize;

    OUString valueName("value");
    if (keyName->length)
    {
        ORegKey* pSubKey = nullptr;
        RegError _ret = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pSubKey));
        if (_ret != RegError::NO_ERROR)
            return _ret;

        if (pSubKey->getValueInfo(valueName, &valueType, &valueSize) != RegError::NO_ERROR)
        {
            (void) pKey->releaseKey(pSubKey);
            return RegError::INVALID_VALUE;
        }

        *pValueType = valueType;
        *pValueSize = valueSize;

        return pKey->releaseKey(pSubKey);
    }


    if (pKey->getValueInfo(valueName, &valueType, &valueSize) != RegError::NO_ERROR)
    {
        return RegError::INVALID_VALUE;
    }

    *pValueType = valueType;
    *pValueSize = valueSize;

    return RegError::NO_ERROR;
}


//  getValueInfo

RegError REGISTRY_CALLTYPE getValue(RegKeyHandle hKey,
                                    rtl_uString* keyName,
                                    RegValue pValue)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    OUString valueName("value");
    if (keyName->length)
    {
        ORegKey* pSubKey = nullptr;
        RegError _ret1 = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pSubKey));
        if (_ret1 != RegError::NO_ERROR)
            return _ret1;

        _ret1 = pSubKey->getValue(valueName, pValue);
        if (_ret1 != RegError::NO_ERROR)
        {
            (void) pKey->releaseKey(pSubKey);
            return _ret1;
        }

        return pKey->releaseKey(pSubKey);
    }

    return pKey->getValue(valueName, pValue);
}


//  getLongValueList

RegError REGISTRY_CALLTYPE getLongListValue(RegKeyHandle hKey,
                                            rtl_uString* keyName,
                                            sal_Int32** pValueList,
                                            sal_uInt32* pLen)
{
    assert((pValueList != nullptr) && (pLen != nullptr) && "registry::getLongListValue(): invalid parameter");
    *pValueList = nullptr;
    *pLen = 0;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    OUString valueName("value");
    if (keyName->length)
    {
        ORegKey* pSubKey = nullptr;
        RegError _ret1 = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pSubKey));
        if (_ret1 != RegError::NO_ERROR)
            return _ret1;

        _ret1 = pSubKey->getLongListValue(valueName, pValueList, pLen);
        if (_ret1 != RegError::NO_ERROR)
        {
            (void) pKey->releaseKey(pSubKey);
            return _ret1;
        }

        return pKey->releaseKey(pSubKey);
    }

    return pKey->getLongListValue(valueName, pValueList, pLen);
}


//  getStringValueList

RegError REGISTRY_CALLTYPE getStringListValue(RegKeyHandle hKey,
                                              rtl_uString* keyName,
                                              sal_Char*** pValueList,
                                              sal_uInt32* pLen)
{
    OSL_PRECOND((pValueList != nullptr) && (pLen != nullptr), "registry::getStringListValue(): invalid parameter");
    *pValueList = nullptr;
    *pLen = 0;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    OUString valueName("value");
    if (keyName->length)
    {
        ORegKey* pSubKey = nullptr;
        RegError _ret1 = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pSubKey));
        if (_ret1 != RegError::NO_ERROR)
            return _ret1;

        _ret1 = pSubKey->getStringListValue(valueName, pValueList, pLen);
        if (_ret1 != RegError::NO_ERROR)
        {
            (void) pKey->releaseKey(pSubKey);
            return _ret1;
        }

        return pKey->releaseKey(pSubKey);
    }

    return pKey->getStringListValue(valueName, pValueList, pLen);
}

//  getUnicodeListValue
RegError REGISTRY_CALLTYPE getUnicodeListValue(RegKeyHandle hKey,
                                               rtl_uString* keyName,
                                               sal_Unicode*** pValueList,
                                               sal_uInt32* pLen)
{
    assert((pValueList != nullptr) && (pLen != nullptr) && "registry::getUnicodeListValue(): invalid parameter");
    *pValueList = nullptr;
    *pLen = 0;

    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    OUString valueName("value");
    if (keyName->length)
    {
        ORegKey* pSubKey = nullptr;
        RegError _ret1 = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pSubKey));
        if (_ret1 != RegError::NO_ERROR)
            return _ret1;

        _ret1 = pSubKey->getUnicodeListValue(valueName, pValueList, pLen);
        if (_ret1 != RegError::NO_ERROR)
        {
            (void) pKey->releaseKey(pSubKey);
            return _ret1;
        }

        return pKey->releaseKey(pSubKey);
    }

    return pKey->getUnicodeListValue(valueName, pValueList, pLen);
}


//  freeValueList

RegError REGISTRY_CALLTYPE freeValueList(RegValueType valueType,
                                              RegValue pValueList,
                                              sal_uInt32 len)
{
    switch (valueType)
    {
        case RegValueType::LONGLIST:
            {
                std::free(pValueList);
            }
            break;
        case RegValueType::STRINGLIST:
            {
                sal_Char** pVList = static_cast<sal_Char**>(pValueList);
                for (sal_uInt32 i=0; i < len; i++)
                {
                    std::free(pVList[i]);
                }

                std::free(pVList);
            }
            break;
        case RegValueType::UNICODELIST:
            {
                sal_Unicode** pVList = static_cast<sal_Unicode**>(pValueList);
                for (sal_uInt32 i=0; i < len; i++)
                {
                    std::free(pVList[i]);
                }

                std::free(pVList);
            }
            break;
        default:
            return RegError::INVALID_VALUE;
    }

    pValueList = nullptr;
    return RegError::NO_ERROR;
}


//  getName

RegError REGISTRY_CALLTYPE getResolvedKeyName(RegKeyHandle hKey,
                                              rtl_uString* keyName,
                                              SAL_UNUSED_PARAMETER sal_Bool,
                                                rtl_uString** pResolvedName)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    OUString resolvedName;
    RegError _ret = pKey->getResolvedKeyName(keyName, resolvedName);
    if (_ret == RegError::NO_ERROR)
        rtl_uString_assign(pResolvedName, resolvedName.pData);
    return _ret;
}


//  getKeyNames

RegError REGISTRY_CALLTYPE getKeyNames(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                         rtl_uString*** pSubKeyNames,
                                         sal_uInt32* pnSubKeys)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    if (pKey->isDeleted())
        return RegError::INVALID_KEY;

    return pKey->getKeyNames(keyName, pSubKeyNames, pnSubKeys);
}


//  freeKeyNames

RegError REGISTRY_CALLTYPE freeKeyNames(rtl_uString** pKeyNames,
                                          sal_uInt32 nKeys)
{
    for (sal_uInt32 i=0; i < nKeys; i++)
    {
        rtl_uString_release(pKeyNames[i]);
    }

    std::free(pKeyNames);

    return RegError::NO_ERROR;
}


//  C API


//  reg_openKey

RegError REGISTRY_CALLTYPE reg_openKey(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       RegKeyHandle* phOpenKey)
{
    if (!hKey)
        return RegError::INVALID_KEY;

    return openKey(hKey, keyName, phOpenKey);
}


//  reg_closeKey

RegError REGISTRY_CALLTYPE reg_closeKey(RegKeyHandle hKey)
{
    if (!hKey)
        return RegError::INVALID_KEY;

    return closeKey(hKey);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
