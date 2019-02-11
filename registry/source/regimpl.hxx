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

#ifndef INCLUDED_REGISTRY_SOURCE_REGIMPL_HXX
#define INCLUDED_REGISTRY_SOURCE_REGIMPL_HXX

#include <unordered_map>

#include <regapi.hxx>
#include <registry/regtype.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <store/store.hxx>

// 5 bytes = 1 (byte for the type) + 4 (bytes for the size of the data)
#define VALUE_HEADERSIZE    5
#define VALUE_TYPEOFFSET    1
#define VALUE_HEADEROFFSET  5

#define REG_GUARD(mutex) \
    osl::Guard< osl::Mutex > aGuard( mutex );

class ORegKey;
class RegistryTypeReader;

class ORegistry
{
public:
    ORegistry();

    sal_uInt32  acquire()
        { return ++m_refCount; }

    sal_uInt32  release()
        { return --m_refCount; }

    RegError    initRegistry(const OUString& name,
                             RegAccessMode accessMode,
                             bool bCreate = false);

    RegError    closeRegistry();

    RegError    destroyRegistry(const OUString& name);

    RegError    acquireKey(RegKeyHandle hKey);
    RegError    releaseKey(RegKeyHandle hKey);

    RegError    createKey(RegKeyHandle hKey,
                          const OUString& keyName,
                          RegKeyHandle* phNewKey);

    RegError    openKey(RegKeyHandle hKey,
                        const OUString& keyName,
                        RegKeyHandle* phOpenKey);

    RegError    closeKey(RegKeyHandle hKey);

    RegError    deleteKey(RegKeyHandle hKey, const OUString& keyName);

    RegError    loadKey(RegKeyHandle hKey,
                        const OUString& regFileName,
                        bool bWarnings,
                        bool bReport);

    RegError    dumpRegistry(RegKeyHandle hKey) const;

    ~ORegistry();

    bool            isReadOnly() const
        { return m_readOnly; }

    bool            isOpen() const
        { return m_isOpen; }

    ORegKey*    getRootKey();

    const store::OStoreFile& getStoreFile() const
        { return m_file; }

    const OUString&    getName() const
        { return m_name; }

    friend class ORegKey;

private:
    RegError    eraseKey(ORegKey* pKey, const OUString& keyName);

    RegError    deleteSubkeysAndValues(ORegKey* pKey);

    static RegError loadAndSaveValue(ORegKey* pTargetKey,
                                 ORegKey const * pSourceKey,
                                 const OUString& valueName,
                                 sal_uInt32 nCut,
                                 bool bWarnings,
                                 bool bReport);

    static RegError checkBlop(store::OStoreStream& rValue,
                          const OUString& sTargetPath,
                          sal_uInt32 srcValueSize,
                          sal_uInt8 const * pSrcBuffer,
                          bool bReport);

    static RegError mergeModuleValue(store::OStoreStream& rTargetValue,
                                 RegistryTypeReader const & reader,
                                 RegistryTypeReader const & reader2);

    RegError    loadAndSaveKeys(ORegKey* pTargetKey,
                                ORegKey* pSourceKey,
                                const OUString& keyName,
                                sal_uInt32 nCut,
                                bool bWarnings,
                                bool bReport);

    RegError    dumpValue(const OUString& sPath,
                          const OUString& sName,
                          sal_Int16 nSpace) const;

    RegError    dumpKey(const OUString& sPath,
                        const OUString& sName,
                        sal_Int16 nSpace) const;

    typedef std::unordered_map< OUString, ORegKey* > KeyMap;

    sal_uInt32          m_refCount;
    osl::Mutex          m_mutex;
    bool                m_readOnly;
    bool                m_isOpen;
    OUString            m_name;
    store::OStoreFile   m_file;
    KeyMap              m_openKeyTable;

    static constexpr OUStringLiteral ROOT { "/" };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
