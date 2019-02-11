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


#include "regimpl.hxx"

#include <memory>
#include <set>
#include <vector>
#include <string.h>
#include <stdio.h>

#if defined(UNX)
#include <unistd.h>
#endif

#include "reflread.hxx"

#include "reflwrit.hxx"

#include <registry/reader.hxx>
#include <registry/refltype.hxx>
#include <registry/types.hxx>

#include "reflcnst.hxx"
#include "keyimpl.hxx"

#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>

using namespace osl;
using namespace store;


namespace {

void printString(OUString const & s) {
    printf("\"");
    for (sal_Int32 i = 0; i < s.getLength(); ++i) {
        sal_Unicode c = s[i];
        if (c == '"' || c == '\\') {
            printf("\\%c", static_cast< char >(c));
        } else if (s[i] >= ' ' && s[i] <= '~') {
            printf("%c", static_cast< char >(c));
        } else {
            printf("\\u%04X", static_cast< unsigned int >(c));
        }
    }
    printf("\"");
}

void printFieldOrReferenceFlag(
    RTFieldAccess * flags, RTFieldAccess flag, char const * name, bool * first)
{
    if ((*flags & flag) != RTFieldAccess::NONE) {
        if (!*first) {
            printf("|");
        }
        *first = false;
        printf("%s", name);
        *flags &= ~flag;
    }
}

void printFieldOrReferenceFlags(RTFieldAccess flags) {
    if (flags == RTFieldAccess::NONE) {
        printf("none");
    } else {
        bool first = true;
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::READONLY, "readonly", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::OPTIONAL, "optional", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::MAYBEVOID, "maybevoid", &first);
        printFieldOrReferenceFlag(&flags, RTFieldAccess::BOUND, "bound", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::CONSTRAINED, "constrained", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::TRANSIENT, "transient", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::MAYBEAMBIGUOUS, "maybeambiguous", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::MAYBEDEFAULT, "maybedefault", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::REMOVABLE, "removable", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::ATTRIBUTE, "attribute", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::PROPERTY, "property", &first);
        printFieldOrReferenceFlag(&flags, RTFieldAccess::CONST, "const", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::READWRITE, "readwrite", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::PARAMETERIZED_TYPE, "parameterized type", &first);
        printFieldOrReferenceFlag(
            &flags, RTFieldAccess::PUBLISHED, "published", &first);
        if (flags != RTFieldAccess::NONE) {
            if (!first) {
                printf("|");
            }
            printf("<invalid (0x%04X)>", static_cast< unsigned int >(flags));
        }
    }
}

void dumpType(typereg::Reader const & reader, OString const & indent) {
    if (reader.isValid()) {
        printf("version: %ld\n", static_cast< long >(reader.getVersion()));
        printf("%sdocumentation: ", indent.getStr());
        printString(reader.getDocumentation());
        printf("\n");
        printf("%sfile name: ", indent.getStr());
        printString(reader.getFileName());
        printf("\n");
        printf("%stype class: ", indent.getStr());
        if (reader.isPublished()) {
            printf("published ");
        }
        switch (reader.getTypeClass()) {
        case RT_TYPE_INTERFACE:
            printf("interface");
            break;

        case RT_TYPE_MODULE:
            printf("module");
            break;

        case RT_TYPE_STRUCT:
            printf("struct");
            break;

        case RT_TYPE_ENUM:
            printf("enum");
            break;

        case RT_TYPE_EXCEPTION:
            printf("exception");
            break;

        case RT_TYPE_TYPEDEF:
            printf("typedef");
            break;

        case RT_TYPE_SERVICE:
            printf("service");
            break;

        case RT_TYPE_SINGLETON:
            printf("singleton");
            break;

        case RT_TYPE_CONSTANTS:
            printf("constants");
            break;

        default:
            printf(
                "<invalid (%ld)>", static_cast< long >(reader.getTypeClass()));
            break;
        }
        printf("\n");
        printf("%stype name: ", indent.getStr());
        printString(reader.getTypeName());
        printf("\n");
        printf(
            "%ssuper type count: %u\n", indent.getStr(),
            static_cast< unsigned int >(reader.getSuperTypeCount()));
        for (sal_uInt16 i = 0; i < reader.getSuperTypeCount(); ++i) {
            printf(
                "%ssuper type name %u: ", indent.getStr(),
                static_cast< unsigned int >(i));
            printString(reader.getSuperTypeName(i));
            printf("\n");
        }
        printf(
            "%sfield count: %u\n", indent.getStr(),
            static_cast< unsigned int >(reader.getFieldCount()));
        for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
            printf(
                "%sfield %u:\n", indent.getStr(),
                static_cast< unsigned int >(i));
            printf("%s    documentation: ", indent.getStr());
            printString(reader.getFieldDocumentation(i));
            printf("\n");
            printf("%s    file name: ", indent.getStr());
            printString(reader.getFieldFileName(i));
            printf("\n");
            printf("%s    flags: ", indent.getStr());
            printFieldOrReferenceFlags(reader.getFieldFlags(i));
            printf("\n");
            printf("%s    name: ", indent.getStr());
            printString(reader.getFieldName(i));
            printf("\n");
            printf("%s    type name: ", indent.getStr());
            printString(reader.getFieldTypeName(i));
            printf("\n");
            printf("%s    value: ", indent.getStr());
            RTConstValue value(reader.getFieldValue(i));
            switch (value.m_type) {
            case RT_TYPE_NONE:
                printf("none");
                break;

            case RT_TYPE_BOOL:
                printf("boolean %s", value.m_value.aBool ? "true" : "false");
                break;

            case RT_TYPE_BYTE:
                printf("byte %d", static_cast< int >(value.m_value.aByte));
                break;

            case RT_TYPE_INT16:
                printf("short %d", static_cast< int >(value.m_value.aShort));
                break;

            case RT_TYPE_UINT16:
                printf(
                    "unsigned short %u",
                    static_cast< unsigned int >(value.m_value.aUShort));
                break;

            case RT_TYPE_INT32:
                printf("long %ld", static_cast< long >(value.m_value.aLong));
                break;

            case RT_TYPE_UINT32:
                printf(
                    "unsigned long %lu",
                    static_cast< unsigned long >(value.m_value.aULong));
                break;

            case RT_TYPE_INT64:
                // TODO: no portable way to print hyper values
                printf("hyper");
                break;

            case RT_TYPE_UINT64:
                // TODO: no portable way to print unsigned hyper values
                printf("unsigned hyper");
                break;

            case RT_TYPE_FLOAT:
                // TODO: no portable way to print float values
                printf("float");
                break;

            case RT_TYPE_DOUBLE:
                // TODO: no portable way to print double values
                printf("double");
                break;

            case RT_TYPE_STRING:
                printf("string ");
                printString(value.m_value.aString);
                break;

            default:
                printf("<invalid (%ld)>", static_cast< long >(value.m_type));
                break;
            }
            printf("\n");
        }
        printf(
            "%smethod count: %u\n", indent.getStr(),
            static_cast< unsigned int >(reader.getMethodCount()));
        for (sal_uInt16 i = 0; i < reader.getMethodCount(); ++i) {
            printf(
                "%smethod %u:\n", indent.getStr(),
                static_cast< unsigned int >(i));
            printf("%s    documentation: ", indent.getStr());
            printString(reader.getMethodDocumentation(i));
            printf("\n");
            printf("%s    flags: ", indent.getStr());
            switch (reader.getMethodFlags(i)) {
            case RTMethodMode::ONEWAY:
                printf("oneway");
                break;

            case RTMethodMode::TWOWAY:
                printf("synchronous");
                break;

            case RTMethodMode::ATTRIBUTE_GET:
                printf("attribute get");
                break;

            case RTMethodMode::ATTRIBUTE_SET:
                printf("attribute set");
                break;

            default:
                printf(
                    "<invalid (%ld)>",
                    static_cast< long >(reader.getMethodFlags(i)));
                break;
            }
            printf("\n");
            printf("%s    name: ", indent.getStr());
            printString(reader.getMethodName(i));
            printf("\n");
            printf("%s    return type name: ", indent.getStr());
            printString(reader.getMethodReturnTypeName(i));
            printf("\n");
            printf(
                "%s    parameter count: %u\n", indent.getStr(),
                static_cast< unsigned int >(reader.getMethodParameterCount(i)));
            // coverity[tainted_data] - cid#1215304 unhelpfully warns about untrusted loop bound
            for (sal_uInt16 j = 0; j < reader.getMethodParameterCount(i); ++j)
            {
                printf(
                    "%s    parameter %u:\n", indent.getStr(),
                    static_cast< unsigned int >(j));
                printf("%s        flags: ", indent.getStr());
                RTParamMode flags = reader.getMethodParameterFlags(i, j);
                bool rest = (flags & RT_PARAM_REST) != 0;
                switch (flags & ~RT_PARAM_REST) {
                case RT_PARAM_IN:
                    printf("in");
                    break;

                case RT_PARAM_OUT:
                    printf("out");
                    break;

                case RT_PARAM_INOUT:
                    printf("inout");
                    break;

                default:
                    printf("<invalid (%ld)>", static_cast< long >(flags));
                    rest = false;
                    break;
                }
                if (rest) {
                    printf("|rest");
                }
                printf("\n");
                printf("%s        name: ", indent.getStr());
                printString(reader.getMethodParameterName(i, j));
                printf("\n");
                printf("%s        type name: ", indent.getStr());
                printString(reader.getMethodParameterTypeName(i, j));
                printf("\n");
            }
            printf(
                "%s    exception count: %u\n", indent.getStr(),
                static_cast< unsigned int >(reader.getMethodExceptionCount(i)));
            // coverity[tainted_data] - cid#1215304 unhelpfully warns about untrusted loop bound
            for (sal_uInt16 j = 0; j < reader.getMethodExceptionCount(i); ++j)
            {
                printf(
                    "%s    exception type name %u: ", indent.getStr(),
                    static_cast< unsigned int >(j));
                printString(reader.getMethodExceptionTypeName(i, j));
                printf("\n");
            }
        }
        printf(
            "%sreference count: %u\n", indent.getStr(),
            static_cast< unsigned int >(reader.getReferenceCount()));
        for (sal_uInt16 i = 0; i < reader.getReferenceCount(); ++i) {
            printf(
                "%sreference %u:\n", indent.getStr(),
                static_cast< unsigned int >(i));
            printf("%s    documentation: ", indent.getStr());
            printString(reader.getReferenceDocumentation(i));
            printf("\n");
            printf("%s    flags: ", indent.getStr());
            printFieldOrReferenceFlags(reader.getReferenceFlags(i));
            printf("\n");
            printf("%s    sort: ", indent.getStr());
            switch (reader.getReferenceSort(i)) {
            case RTReferenceType::SUPPORTS:
                printf("supports");
                break;

            case RTReferenceType::EXPORTS:
                printf("exports");
                break;

            case RTReferenceType::TYPE_PARAMETER:
                printf("type parameter");
                break;

            default:
                printf(
                    "<invalid (%ld)>",
                    static_cast< long >(reader.getReferenceSort(i)));
                break;
            }
            printf("\n");
            printf("%s    type name: ", indent.getStr());
            printString(reader.getReferenceTypeName(i));
            printf("\n");
        }
    } else {
        printf("<invalid>\n");
    }
}

}

ORegistry::ORegistry()
    : m_refCount(1)
    , m_readOnly(false)
    , m_isOpen(false)
{
}

ORegistry::~ORegistry()
{
    ORegKey* pRootKey = m_openKeyTable[ROOT];
    if (pRootKey != nullptr)
        (void) releaseKey(pRootKey);

    if (m_file.isValid())
        m_file.close();
}

RegError ORegistry::initRegistry(const OUString& regName, RegAccessMode accessMode, bool bCreate)
{
    RegError eRet = RegError::INVALID_REGISTRY;
    OStoreFile      rRegFile;
    storeAccessMode sAccessMode = storeAccessMode::ReadWrite;
    storeError      errCode;

    if (bCreate)
    {
        sAccessMode = storeAccessMode::Create;
    }
    else if (accessMode & RegAccessMode::READONLY)
    {
        sAccessMode = storeAccessMode::ReadOnly;
        m_readOnly = true;
    }

    if (regName.isEmpty() &&
        storeAccessMode::Create == sAccessMode)
    {
        errCode = rRegFile.createInMemory();
    }
    else
    {
        errCode = rRegFile.create(regName, sAccessMode);
    }

    if (errCode)
    {
        switch (errCode)
        {
            case store_E_NotExists:
                eRet = RegError::REGISTRY_NOT_EXISTS;
                break;
            case store_E_LockingViolation:
                eRet = RegError::CANNOT_OPEN_FOR_READWRITE;
                break;
            default:
                eRet = RegError::INVALID_REGISTRY;
                break;
        }
    }
    else
    {
        OStoreDirectory rStoreDir;
        storeError _err = rStoreDir.create(rRegFile, OUString(), OUString(), sAccessMode);

        if (_err == store_E_None)
        {
            m_file = rRegFile;
            m_name = regName;
            m_isOpen = true;

            m_openKeyTable[ROOT] = new ORegKey(ROOT, this);
            eRet = RegError::NO_ERROR;
        }
        else
            eRet = RegError::INVALID_REGISTRY;
    }

    return eRet;
}

RegError ORegistry::closeRegistry()
{
    REG_GUARD(m_mutex);

    if (m_file.isValid())
    {
        (void) releaseKey(m_openKeyTable[ROOT]);
        m_file.close();
        m_isOpen = false;
        return RegError::NO_ERROR;
    } else
    {
        return RegError::REGISTRY_NOT_EXISTS;
    }
}

RegError ORegistry::destroyRegistry(const OUString& regName)
{
    REG_GUARD(m_mutex);

    if (!regName.isEmpty())
    {
        std::unique_ptr<ORegistry> pReg(new ORegistry());

        if (pReg->initRegistry(regName, RegAccessMode::READWRITE) == RegError::NO_ERROR)
        {
            pReg.reset();

            OUString systemName;
            if (FileBase::getSystemPathFromFileURL(regName, systemName) != FileBase::E_None)
                systemName = regName;

            OString name(OUStringToOString(systemName, osl_getThreadTextEncoding()));
            if (unlink(name.getStr()) != 0)
            {
                return RegError::DESTROY_REGISTRY_FAILED;
            }
        } else
        {
            return RegError::DESTROY_REGISTRY_FAILED;
        }
    } else
    {
        if (m_refCount != 1 || isReadOnly())
        {
            return RegError::DESTROY_REGISTRY_FAILED;
        }

        if (m_file.isValid())
        {
            releaseKey(m_openKeyTable[ROOT]);
            m_file.close();
            m_isOpen = false;

            if (!m_name.isEmpty())
            {
                OUString systemName;
                if (FileBase::getSystemPathFromFileURL(m_name, systemName) != FileBase::E_None)
                    systemName = m_name;

                OString name(OUStringToOString(systemName, osl_getThreadTextEncoding()));
                if (unlink(name.getStr()) != 0)
                {
                    return RegError::DESTROY_REGISTRY_FAILED;
                }
            }
        } else
        {
            return RegError::REGISTRY_NOT_EXISTS;
        }
    }

    return RegError::NO_ERROR;
}

RegError ORegistry::acquireKey (RegKeyHandle hKey)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    REG_GUARD(m_mutex);
    pKey->acquire();

    return RegError::NO_ERROR;
}

RegError ORegistry::releaseKey (RegKeyHandle hKey)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (!pKey)
        return RegError::INVALID_KEY;

    REG_GUARD(m_mutex);
    if (pKey->release() == 0)
    {
        m_openKeyTable.erase(pKey->getName());
        delete pKey;
    }
    return RegError::NO_ERROR;
}

RegError ORegistry::createKey(RegKeyHandle hKey, const OUString& keyName,
                              RegKeyHandle* phNewKey)
{
    ORegKey*    pKey;

    *phNewKey = nullptr;

    if (keyName.isEmpty())
        return RegError::INVALID_KEYNAME;

    REG_GUARD(m_mutex);

    if (hKey)
        pKey = static_cast<ORegKey*>(hKey);
    else
        pKey = m_openKeyTable[ROOT];

    OUString sFullKeyName = pKey->getFullPath(keyName);

    if (m_openKeyTable.count(sFullKeyName) > 0)
    {
        *phNewKey = m_openKeyTable[sFullKeyName];
        static_cast<ORegKey*>(*phNewKey)->acquire();
        static_cast<ORegKey*>(*phNewKey)->setDeleted(false);
        return RegError::NO_ERROR;
    }

    OStoreDirectory rStoreDir;
    OUStringBuffer  sFullPath(sFullKeyName.getLength());
    OUString        token;

    sFullPath.append('/');

    sal_Int32 nIndex = 0;
    do
    {
        token = sFullKeyName.getToken(0, '/', nIndex);
        if (!token.isEmpty())
        {
            if (rStoreDir.create(pKey->getStoreFile(), sFullPath.getStr(), token, storeAccessMode::Create))
            {
                return RegError::CREATE_KEY_FAILED;
            }

            sFullPath.append(token);
            sFullPath.append('/');
        }
    } while(nIndex != -1);


    pKey = new ORegKey(sFullKeyName, this);
    *phNewKey = pKey;
    m_openKeyTable[sFullKeyName] = pKey;

    return RegError::NO_ERROR;
}

RegError ORegistry::openKey(RegKeyHandle hKey, const OUString& keyName,
                            RegKeyHandle* phOpenKey)
{
    ORegKey*        pKey;

    *phOpenKey = nullptr;

    if (keyName.isEmpty())
    {
        return RegError::INVALID_KEYNAME;
    }

    REG_GUARD(m_mutex);

    if (hKey)
        pKey = static_cast<ORegKey*>(hKey);
    else
        pKey = m_openKeyTable[ROOT];

    OUString path(pKey->getFullPath(keyName));
    KeyMap::iterator i(m_openKeyTable.find(path));
    if (i == m_openKeyTable.end()) {
        sal_Int32 n = path.lastIndexOf('/') + 1;
        switch (OStoreDirectory().create(
                    pKey->getStoreFile(), path.copy(0, n), path.copy(n),
                    isReadOnly() ? storeAccessMode::ReadOnly : storeAccessMode::ReadWrite))
        {
        case store_E_NotExists:
            return RegError::KEY_NOT_EXISTS;
        case store_E_WrongFormat:
            return RegError::INVALID_KEY;
        default:
            break;
        }

        std::unique_ptr< ORegKey > p(new ORegKey(path, this));
        i = m_openKeyTable.insert(std::make_pair(path, p.get())).first;
        p.release();
    } else {
        i->second->acquire();
    }
    *phOpenKey = i->second;
    return RegError::NO_ERROR;
}

RegError ORegistry::closeKey(RegKeyHandle hKey)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);

    REG_GUARD(m_mutex);

    OUString const aKeyName (pKey->getName());
    if (m_openKeyTable.count(aKeyName) <= 0)
        return RegError::KEY_NOT_OPEN;

    if (pKey->isModified())
    {
        ORegKey * pRootKey = getRootKey();
        if (pKey != pRootKey)
        {
            // propagate "modified" state to RootKey.
            pRootKey->setModified();
        }
        else
        {
            // closing modified RootKey, flush registry file.
            (void) m_file.flush();
        }
        pKey->setModified(false);
        (void) releaseKey(pRootKey);
    }

    return releaseKey(pKey);
}

RegError ORegistry::deleteKey(RegKeyHandle hKey, const OUString& keyName)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);
    if (keyName.isEmpty())
        return RegError::INVALID_KEYNAME;

    REG_GUARD(m_mutex);

    if (!pKey)
        pKey = m_openKeyTable[ROOT];

    OUString sFullKeyName(pKey->getFullPath(keyName));
    return eraseKey(m_openKeyTable[ROOT], sFullKeyName);
}

RegError ORegistry::eraseKey(ORegKey* pKey, const OUString& keyName)
{
    RegError _ret = RegError::NO_ERROR;

    if (keyName.isEmpty())
    {
        return RegError::INVALID_KEYNAME;
    }

    OUString     sFullKeyName(pKey->getName());
    OUString     sFullPath(sFullKeyName);
    OUString     sRelativKey;
    sal_Int32    lastIndex = keyName.lastIndexOf('/');

    if (lastIndex >= 0)
    {
        sRelativKey += keyName.copy(lastIndex + 1);

        if (sFullKeyName.getLength() > 1)
            sFullKeyName += keyName;
        else
            sFullKeyName += keyName.copy(1);

        sFullPath = sFullKeyName.copy(0, keyName.lastIndexOf('/') + 1);
    } else
    {
        if (sFullKeyName.getLength() > 1)
            sFullKeyName += ROOT;

        sRelativKey += keyName;
        sFullKeyName += keyName;

        if (sFullPath.getLength() > 1)
            sFullPath += ROOT;
    }

    ORegKey* pOldKey = nullptr;
    _ret = pKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pOldKey));
    if (_ret != RegError::NO_ERROR)
        return _ret;

    _ret = deleteSubkeysAndValues(pOldKey);
    if (_ret != RegError::NO_ERROR)
    {
        pKey->closeKey(pOldKey);
        return _ret;
    }

    OUString tmpName(sRelativKey);
    tmpName += ROOT;

    OStoreFile sFile(pKey->getStoreFile());
    if (sFile.isValid() && sFile.remove(sFullPath, tmpName))
    {
        return RegError::DELETE_KEY_FAILED;
    }
    pOldKey->setModified();

    // set flag deleted !!!
    pOldKey->setDeleted(true);

    return pKey->closeKey(pOldKey);
}

RegError ORegistry::deleteSubkeysAndValues(ORegKey* pKey)
{
    OStoreDirectory::iterator   iter;
    RegError                    _ret = RegError::NO_ERROR;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    while (_err == store_E_None)
    {
        OUString const keyName = iter.m_pszName;

        if (iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            _ret = eraseKey(pKey, keyName);
            if (_ret != RegError::NO_ERROR)
                return _ret;
        }
        else
        {
            OUString sFullPath(pKey->getName());

            if (sFullPath.getLength() > 1)
                sFullPath += ROOT;

            if (const_cast<OStoreFile&>(pKey->getStoreFile()).remove(sFullPath, keyName))
            {
                return RegError::DELETE_VALUE_FAILED;
            }
            pKey->setModified();
        }

        _err = rStoreDir.next(iter);
    }

    return RegError::NO_ERROR;
}

RegError ORegistry::loadKey(RegKeyHandle hKey, const OUString& regFileName,
                            bool bWarnings, bool bReport)
{
    ORegKey* pKey = static_cast< ORegKey* >(hKey);

    std::unique_ptr< ORegistry > pReg (new ORegistry());
    RegError _ret = pReg->initRegistry(regFileName, RegAccessMode::READONLY);
    if (_ret != RegError::NO_ERROR)
        return _ret;
    ORegKey* pRootKey = pReg->getRootKey();

    REG_GUARD(m_mutex);

    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir(pRootKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    while (_err == store_E_None)
    {
        OUString const keyName = iter.m_pszName;

        if (iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            _ret = loadAndSaveKeys(pKey, pRootKey, keyName, 0, bWarnings, bReport);
        }
        else
        {
            _ret = loadAndSaveValue(pKey, pRootKey, keyName, 0, bWarnings, bReport);
        }

        if (_ret == RegError::MERGE_ERROR)
            break;
        if (_ret == RegError::MERGE_CONFLICT && bWarnings)
            break;

        _err = rStoreDir.next(iter);
    }

    rStoreDir = OStoreDirectory();
    (void) pReg->releaseKey(pRootKey);
    return _ret;
}

RegError ORegistry::loadAndSaveValue(ORegKey* pTargetKey,
                                     ORegKey const * pSourceKey,
                                     const OUString& valueName,
                                     sal_uInt32 nCut,
                                     bool bWarnings,
                                     bool bReport)
{
    OStoreStream    rValue;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    sal_uInt32      nSize;
    storeAccessMode sourceAccess = storeAccessMode::ReadWrite;
    OUString        sTargetPath(pTargetKey->getName());
    OUString        sSourcePath(pSourceKey->getName());

    if (pSourceKey->isReadOnly())
    {
        sourceAccess = storeAccessMode::ReadOnly;
    }

    if (nCut)
    {
        sTargetPath = sSourcePath.copy(nCut);
    } else
    {
        if (sTargetPath.getLength() > 1)
        {
            if (sSourcePath.getLength() > 1)
                sTargetPath += sSourcePath;
        } else
            sTargetPath = sSourcePath;
    }

    if (sTargetPath.getLength() > 1) sTargetPath += ROOT;
    if (sSourcePath.getLength() > 1) sSourcePath += ROOT;

    if (rValue.create(pSourceKey->getStoreFile(), sSourcePath, valueName, sourceAccess))
    {
        return RegError::VALUE_NOT_EXISTS;
    }

    std::vector<sal_uInt8> aBuffer(VALUE_HEADERSIZE);

    sal_uInt32  rwBytes;
    if (rValue.readAt(0, aBuffer.data(), VALUE_HEADERSIZE, rwBytes))
    {
        return RegError::INVALID_VALUE;
    }
    if (rwBytes != VALUE_HEADERSIZE)
    {
        return RegError::INVALID_VALUE;
    }

    RegError _ret = RegError::NO_ERROR;
    sal_uInt8   type = aBuffer[0];
    valueType = static_cast<RegValueType>(type);
    readUINT32(aBuffer.data() + VALUE_TYPEOFFSET, valueSize);

    nSize = VALUE_HEADERSIZE + valueSize;
    aBuffer.resize(nSize);

    if (rValue.readAt(0, aBuffer.data(), nSize, rwBytes))
    {
        return RegError::INVALID_VALUE;
    }
    if (rwBytes != nSize)
    {
        return RegError::INVALID_VALUE;
    }

    OStoreFile  rTargetFile(pTargetKey->getStoreFile());

    if (!rValue.create(rTargetFile, sTargetPath, valueName, storeAccessMode::ReadWrite))
    {
        if (valueType == RegValueType::BINARY)
        {
            _ret = checkBlop(
                rValue, sTargetPath, valueSize, aBuffer.data() + VALUE_HEADEROFFSET,
                bReport);
            if (_ret != RegError::NO_ERROR)
            {
                if (_ret == RegError::MERGE_ERROR ||
                    (_ret == RegError::MERGE_CONFLICT && bWarnings))
                {
                    return _ret;
                }
            } else
            {
                return _ret;
            }
        }
    }

    if (rValue.create(rTargetFile, sTargetPath, valueName, storeAccessMode::Create))
    {
        return RegError::INVALID_VALUE;
    }
    if (rValue.writeAt(0, aBuffer.data(), nSize, rwBytes))
    {
        return RegError::INVALID_VALUE;
    }

    if (rwBytes != nSize)
    {
        return RegError::INVALID_VALUE;
    }
    pTargetKey->setModified();

    return _ret;
}

RegError ORegistry::checkBlop(OStoreStream& rValue,
                              const OUString& sTargetPath,
                              sal_uInt32 srcValueSize,
                              sal_uInt8 const * pSrcBuffer,
                              bool bReport)
{
    RegistryTypeReader reader(pSrcBuffer, srcValueSize);

    if (reader.getTypeClass() == RT_TYPE_INVALID)
    {
        return RegError::INVALID_VALUE;
    }

    std::vector<sal_uInt8> aBuffer(VALUE_HEADERSIZE);
    RegValueType    valueType;
    sal_uInt32      valueSize;
    sal_uInt32      rwBytes;
    OString         targetPath(OUStringToOString(sTargetPath, RTL_TEXTENCODING_UTF8));

    if (!rValue.readAt(0, aBuffer.data(), VALUE_HEADERSIZE, rwBytes) &&
        (rwBytes == VALUE_HEADERSIZE))
    {
        sal_uInt8 type = aBuffer[0];
        valueType = static_cast<RegValueType>(type);
        readUINT32(aBuffer.data() + VALUE_TYPEOFFSET, valueSize);

        if (valueType == RegValueType::BINARY)
        {
            aBuffer.resize(valueSize);
            if (!rValue.readAt(VALUE_HEADEROFFSET, aBuffer.data(), valueSize, rwBytes) &&
                (rwBytes == valueSize))
            {
                RegistryTypeReader reader2(aBuffer.data(), valueSize);

                if ((reader.getTypeClass() != reader2.getTypeClass())
                    || reader2.getTypeClass() == RT_TYPE_INVALID)
                {
                    if (bReport)
                    {
                        fprintf(stdout, "ERROR: values of blop from key \"%s\" has different types.\n",
                                targetPath.getStr());
                    }
                    return RegError::MERGE_ERROR;
                }

                if (reader.getTypeClass() == RT_TYPE_MODULE)
                {
                    if (reader.getFieldCount() > 0 &&
                        reader2.getFieldCount() > 0)
                    {
                        mergeModuleValue(rValue, reader, reader2);

                        return RegError::NO_ERROR;
                    } else
                    if (reader2.getFieldCount() > 0)
                    {
                        return RegError::NO_ERROR;
                    } else
                    {
                        return RegError::MERGE_CONFLICT;
                    }
                } else
                {
                    if (bReport)
                    {
                        fprintf(stderr, "WARNING: value of key \"%s\" already exists.\n",
                                targetPath.getStr());
                    }
                    return RegError::MERGE_CONFLICT;
                }
            } else
            {
                if (bReport)
                {
                    fprintf(stderr, "ERROR: values of key \"%s\" contains bad data.\n",
                            targetPath.getStr());
                }
                return RegError::MERGE_ERROR;
            }
        } else
        {
            if (bReport)
            {
                fprintf(stderr, "ERROR: values of key \"%s\" has different types.\n",
                        targetPath.getStr());
            }
            return RegError::MERGE_ERROR;
        }
    } else
    {
        return RegError::INVALID_VALUE;
    }
}

static sal_uInt32 checkTypeReaders(RegistryTypeReader const & reader1,
                                   RegistryTypeReader const & reader2,
                                   std::set< OUString >& nameSet)
{
    sal_uInt32 count=0;
    for (sal_uInt32 i=0 ; i < reader1.getFieldCount(); i++)
    {
        nameSet.insert(reader1.getFieldName(i));
        count++;
    }
    for (sal_uInt32 i=0 ; i < reader2.getFieldCount(); i++)
    {
        if (nameSet.find(reader2.getFieldName(i)) == nameSet.end())
        {
            nameSet.insert(reader2.getFieldName(i));
            count++;
        }
    }
    return count;
}

RegError ORegistry::mergeModuleValue(OStoreStream& rTargetValue,
                                     RegistryTypeReader const & reader,
                                     RegistryTypeReader const & reader2)
{
    std::set< OUString > nameSet;
    sal_uInt32 count = checkTypeReaders(reader, reader2, nameSet);

    if (count != reader.getFieldCount())
    {
        sal_uInt16 index = 0;

        RegistryTypeWriter writer(reader.getTypeClass(),
                                  reader.getTypeName(),
                                  reader.getSuperTypeName(),
                                  static_cast<sal_uInt16>(count));

        for (sal_uInt32 i=0 ; i < reader.getFieldCount(); i++)
        {
            writer.setFieldData(index,
                               reader.getFieldName(i),
                               reader.getFieldType(i),
                               reader.getFieldDoku(i),
                               reader.getFieldFileName(i),
                               reader.getFieldAccess(i),
                               reader.getFieldConstValue(i));
            index++;
        }
        for (sal_uInt32 i=0 ; i < reader2.getFieldCount(); i++)
        {
            if (nameSet.find(reader2.getFieldName(i)) == nameSet.end())
            {
                writer.setFieldData(index,
                                   reader2.getFieldName(i),
                                   reader2.getFieldType(i),
                                   reader2.getFieldDoku(i),
                                   reader2.getFieldFileName(i),
                                   reader2.getFieldAccess(i),
                                   reader2.getFieldConstValue(i));
                index++;
            }
        }

        const sal_uInt8*    pBlop = writer.getBlop();
        sal_uInt32          aBlopSize = writer.getBlopSize();

        sal_uInt8   type = sal_uInt8(RegValueType::BINARY);
        std::vector<sal_uInt8> aBuffer(VALUE_HEADERSIZE + aBlopSize);

        memcpy(aBuffer.data(), &type, 1);
        writeUINT32(aBuffer.data() + VALUE_TYPEOFFSET, aBlopSize);
        memcpy(aBuffer.data() + VALUE_HEADEROFFSET, pBlop, aBlopSize);

        sal_uInt32  rwBytes;
        if (rTargetValue.writeAt(0, aBuffer.data(), VALUE_HEADERSIZE+aBlopSize, rwBytes))
        {
            return RegError::INVALID_VALUE;
        }

        if (rwBytes != VALUE_HEADERSIZE+aBlopSize)
        {
            return RegError::INVALID_VALUE;
        }
    }
    return RegError::NO_ERROR;
}

RegError ORegistry::loadAndSaveKeys(ORegKey* pTargetKey,
                                    ORegKey* pSourceKey,
                                    const OUString& keyName,
                                    sal_uInt32 nCut,
                                    bool bWarnings,
                                    bool bReport)
{
    RegError    _ret = RegError::NO_ERROR;
    OUString    sRelPath(pSourceKey->getName().copy(nCut));
    OUString    sFullPath;

    if(pTargetKey->getName().getLength() > 1)
        sFullPath += pTargetKey->getName();
    sFullPath += sRelPath;
    if (sRelPath.getLength() > 1 || sFullPath.isEmpty())
        sFullPath += ROOT;

    OUString sFullKeyName = sFullPath;
    sFullKeyName += keyName;

    OStoreDirectory rStoreDir;
    if (rStoreDir.create(pTargetKey->getStoreFile(), sFullPath, keyName, storeAccessMode::Create))
    {
        return RegError::CREATE_KEY_FAILED;
    }

    if (m_openKeyTable.count(sFullKeyName) > 0)
    {
        m_openKeyTable[sFullKeyName]->setDeleted(false);
    }

    ORegKey* pTmpKey = nullptr;
    _ret = pSourceKey->openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pTmpKey));
    if (_ret != RegError::NO_ERROR)
        return _ret;

    OStoreDirectory::iterator   iter;
    OStoreDirectory             rTmpStoreDir(pTmpKey->getStoreDir());
    storeError                  _err = rTmpStoreDir.first(iter);

    while (_err == store_E_None)
    {
        OUString const sName = iter.m_pszName;

        if (iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            _ret = loadAndSaveKeys(pTargetKey, pTmpKey,
                                   sName, nCut, bWarnings, bReport);
        } else
        {
            _ret = loadAndSaveValue(pTargetKey, pTmpKey,
                                    sName, nCut, bWarnings, bReport);
        }

        if (_ret == RegError::MERGE_ERROR)
            break;
        if (_ret == RegError::MERGE_CONFLICT && bWarnings)
            break;

        _err = rTmpStoreDir.next(iter);
    }

    pSourceKey->releaseKey(pTmpKey);
    return _ret;
}

ORegKey* ORegistry::getRootKey()
{
    m_openKeyTable[ROOT]->acquire();
    return m_openKeyTable[ROOT];
}

RegError ORegistry::dumpRegistry(RegKeyHandle hKey) const
{
    ORegKey                     *pKey = static_cast<ORegKey*>(hKey);
    OUString                    sName;
    RegError                    _ret = RegError::NO_ERROR;
    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    OString regName(OUStringToOString(getName(), osl_getThreadTextEncoding()));
    OString keyName(OUStringToOString(pKey->getName(), RTL_TEXTENCODING_UTF8));
    fprintf(stdout, "Registry \"%s\":\n\n%s\n", regName.getStr(), keyName.getStr());

    while (_err == store_E_None)
    {
        sName = iter.m_pszName;

        if (iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            _ret = dumpKey(pKey->getName(), sName, 1);
        } else
        {
            _ret = dumpValue(pKey->getName(), sName, 1);
        }

        if (_ret != RegError::NO_ERROR)
        {
            return _ret;
        }

        _err = rStoreDir.next(iter);
    }

    return RegError::NO_ERROR;
}

RegError ORegistry::dumpValue(const OUString& sPath, const OUString& sName, sal_Int16 nSpc) const
{
    OStoreStream    rValue;
    sal_uInt32      valueSize;
    RegValueType    valueType;
    OUString        sFullPath(sPath);
    OString         sIndent;
    storeAccessMode accessMode = storeAccessMode::ReadWrite;

    if (isReadOnly())
    {
        accessMode = storeAccessMode::ReadOnly;
    }

    for (int i= 0; i < nSpc; i++) sIndent += " ";

    if (sFullPath.getLength() > 1)
    {
        sFullPath += ROOT;
    }
    if (rValue.create(m_file, sFullPath, sName, accessMode))
    {
        return RegError::VALUE_NOT_EXISTS;
    }

    std::vector<sal_uInt8> aBuffer(VALUE_HEADERSIZE);

    sal_uInt32  rwBytes;
    if (rValue.readAt(0, aBuffer.data(), VALUE_HEADERSIZE, rwBytes))
    {
        return RegError::INVALID_VALUE;
    }
    if (rwBytes != (VALUE_HEADERSIZE))
    {
        return RegError::INVALID_VALUE;
    }

    sal_uInt8 type = aBuffer[0];
    valueType = static_cast<RegValueType>(type);
    readUINT32(aBuffer.data() + VALUE_TYPEOFFSET, valueSize);

    aBuffer.resize(valueSize);
    if (rValue.readAt(VALUE_HEADEROFFSET, aBuffer.data(), valueSize, rwBytes))
    {
        return RegError::INVALID_VALUE;
    }
    if (rwBytes != valueSize)
    {
        return RegError::INVALID_VALUE;
    }

    const sal_Char* indent = sIndent.getStr();
    switch (valueType)
    {
        case RegValueType::NOT_DEFINED:
            fprintf(stdout, "%sValue: Type = VALUETYPE_NOT_DEFINED\n", indent);
            break;
        case RegValueType::LONG:
            {
                fprintf(stdout, "%sValue: Type = RegValueType::LONG\n", indent);
                fprintf(
                    stdout, "%s       Size = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(valueSize));
                fprintf(stdout, "%s       Data = ", indent);

                sal_Int32 value;
                readINT32(aBuffer.data(), value);
                fprintf(stdout, "%ld\n", sal::static_int_cast< long >(value));
            }
            break;
        case RegValueType::STRING:
            {
                sal_Char* value = static_cast<sal_Char*>(std::malloc(valueSize));
                readUtf8(aBuffer.data(), value, valueSize);
                fprintf(stdout, "%sValue: Type = RegValueType::STRING\n", indent);
                fprintf(
                    stdout, "%s       Size = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(valueSize));
                fprintf(stdout, "%s       Data = \"%s\"\n", indent, value);
                std::free(value);
            }
            break;
        case RegValueType::UNICODE:
            {
                sal_uInt32 size = (valueSize / 2) * sizeof(sal_Unicode);
                fprintf(stdout, "%sValue: Type = RegValueType::UNICODE\n", indent);
                fprintf(
                    stdout, "%s       Size = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(valueSize));
                fprintf(stdout, "%s       Data = ", indent);

                std::unique_ptr<sal_Unicode[]> value(new sal_Unicode[size]);
                readString(aBuffer.data(), value.get(), size);

                OString uStr = OUStringToOString(value.get(), RTL_TEXTENCODING_UTF8);
                fprintf(stdout, "L\"%s\"\n", uStr.getStr());
            }
            break;
        case RegValueType::BINARY:
            {
                fprintf(stdout, "%sValue: Type = RegValueType::BINARY\n", indent);
                fprintf(
                    stdout, "%s       Size = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(valueSize));
                fprintf(stdout, "%s       Data = ", indent);
                dumpType(
                    typereg::Reader(aBuffer.data(), valueSize),
                    sIndent + "              ");
            }
            break;
        case RegValueType::LONGLIST:
            {
                sal_uInt32 offset = 4; // initial 4 bytes for the size of the array
                sal_uInt32 len = 0;

                readUINT32(aBuffer.data(), len);

                fprintf(stdout, "%sValue: Type = RegValueType::LONGLIST\n", indent);
                fprintf(
                    stdout, "%s       Size = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(valueSize));
                fprintf(
                    stdout, "%s       Len  = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(len));
                fprintf(stdout, "%s       Data = ", indent);

                sal_Int32 longValue;
                for (sal_uInt32 i=0; i < len; i++)
                {
                    readINT32(aBuffer.data() + offset, longValue);

                    if (offset > 4)
                        fprintf(stdout, "%s              ", indent);

                    fprintf(
                        stdout, "%lu = %ld\n",
                        sal::static_int_cast< unsigned long >(i),
                        sal::static_int_cast< long >(longValue));
                    offset += 4; // 4 Bytes for sal_Int32
                }
            }
            break;
        case RegValueType::STRINGLIST:
            {
                sal_uInt32 offset = 4; // initial 4 bytes for the size of the array
                sal_uInt32 sLen = 0;
                sal_uInt32 len = 0;

                readUINT32(aBuffer.data(), len);

                fprintf(stdout, "%sValue: Type = RegValueType::STRINGLIST\n", indent);
                fprintf(
                    stdout, "%s       Size = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(valueSize));
                fprintf(
                    stdout, "%s       Len  = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(len));
                fprintf(stdout, "%s       Data = ", indent);

                for (sal_uInt32 i=0; i < len; i++)
                {
                    readUINT32(aBuffer.data() + offset, sLen);

                    offset += 4; // 4 bytes (sal_uInt32) for the string size

                    sal_Char *pValue = static_cast<sal_Char*>(std::malloc(sLen));
                    readUtf8(aBuffer.data() + offset, pValue, sLen);

                    if (offset > 8)
                        fprintf(stdout, "%s              ", indent);

                    fprintf(
                        stdout, "%lu = \"%s\"\n",
                        sal::static_int_cast< unsigned long >(i), pValue);
                    std::free(pValue);
                    offset += sLen;
                }
            }
            break;
        case RegValueType::UNICODELIST:
            {
                sal_uInt32 offset = 4; // initial 4 bytes for the size of the array
                sal_uInt32 sLen = 0;
                sal_uInt32 len = 0;

                readUINT32(aBuffer.data(), len);

                fprintf(stdout, "%sValue: Type = RegValueType::UNICODELIST\n", indent);
                fprintf(
                    stdout, "%s       Size = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(valueSize));
                fprintf(
                    stdout, "%s       Len  = %lu\n", indent,
                    sal::static_int_cast< unsigned long >(len));
                fprintf(stdout, "%s       Data = ", indent);

                OString uStr;
                for (sal_uInt32 i=0; i < len; i++)
                {
                    readUINT32(aBuffer.data() + offset, sLen);

                    offset += 4; // 4 bytes (sal_uInt32) for the string size

                    sal_Unicode *pValue = static_cast<sal_Unicode*>(std::malloc((sLen / 2) * sizeof(sal_Unicode)));
                    readString(aBuffer.data() + offset, pValue, sLen);

                    if (offset > 8)
                        fprintf(stdout, "%s              ", indent);

                    uStr = OUStringToOString(pValue, RTL_TEXTENCODING_UTF8);
                    fprintf(
                        stdout, "%lu = L\"%s\"\n",
                        sal::static_int_cast< unsigned long >(i),
                        uStr.getStr());

                    offset += sLen;

                    std::free(pValue);
                }
            }
            break;
    }

    fprintf(stdout, "\n");

    return RegError::NO_ERROR;
}

RegError ORegistry::dumpKey(const OUString& sPath, const OUString& sName, sal_Int16 nSpace) const
{
    OStoreDirectory     rStoreDir;
    OUString            sFullPath(sPath);
    OString             sIndent;
    storeAccessMode     accessMode = storeAccessMode::ReadWrite;
    RegError            _ret = RegError::NO_ERROR;

    if (isReadOnly())
    {
        accessMode = storeAccessMode::ReadOnly;
    }

    for (int i= 0; i < nSpace; i++) sIndent += " ";

    if (sFullPath.getLength() > 1)
        sFullPath += ROOT;

    storeError _err = rStoreDir.create(m_file, sFullPath, sName, accessMode);

    if (_err == store_E_NotExists)
        return RegError::KEY_NOT_EXISTS;
    else if (_err == store_E_WrongFormat)
        return RegError::INVALID_KEY;

    fprintf(stdout, "%s/ %s\n", sIndent.getStr(), OUStringToOString(sName, RTL_TEXTENCODING_UTF8).getStr());

    OUString sSubPath(sFullPath);
    OUString sSubName;
    sSubPath += sName;

    OStoreDirectory::iterator   iter;

    _err = rStoreDir.first(iter);

    while (_err == store_E_None)
    {
        sSubName = iter.m_pszName;

        if (iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            _ret = dumpKey(sSubPath, sSubName, nSpace+2);
        } else
        {
            _ret = dumpValue(sSubPath, sSubName, nSpace+2);
        }

        if (_ret != RegError::NO_ERROR)
        {
            return _ret;
        }

        _err = rStoreDir.next(iter);
    }

    return RegError::NO_ERROR;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
