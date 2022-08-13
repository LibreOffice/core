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

#ifndef INCLUDED_REGISTRY_SOURCE_KEYIMPL_HXX
#define INCLUDED_REGISTRY_SOURCE_KEYIMPL_HXX

#include <sal/config.h>

#include <string_view>

#include "regimpl.hxx"
#include <rtl/ustring.hxx>

class ORegKey
{
public:

    ORegKey(OUString keyName, ORegistry* pReg);
    ~ORegKey();

    void        acquire()
        { ++m_refCount; }

    sal_uInt32  release()
        { return --m_refCount; }

    RegError    releaseKey(RegKeyHandle hKey);

    RegError    createKey(std::u16string_view keyName, RegKeyHandle* phNewKey);

    RegError    openKey(std::u16string_view keyName, RegKeyHandle* phOpenKey);

    RegError    openSubKeys(std::u16string_view keyName,
                            RegKeyHandle** phOpenSubKeys,
                            sal_uInt32* pnSubKeys);

    RegError    getKeyNames(std::u16string_view keyName,
                            rtl_uString*** pSubKeyNames,
                            sal_uInt32* pnSubKeys);

    RegError    closeKey(RegKeyHandle hKey);

    RegError    deleteKey(std::u16string_view keyName);

    RegError    getValueInfo(std::u16string_view valueName,
                             RegValueType* pValueTye,
                             sal_uInt32* pValueSize) const;

    RegError    setValue(std::u16string_view valueName,
                         RegValueType vType,
                         RegValue value,
                         sal_uInt32 vSize);

    RegError    setLongListValue(std::u16string_view valueName,
                                 sal_Int32 const * pValueList,
                                 sal_uInt32 len);

    RegError    setStringListValue(std::u16string_view valueName,
                                   char** pValueList,
                                  sal_uInt32 len);

    RegError    setUnicodeListValue(std::u16string_view valueName,
                                   sal_Unicode** pValueList,
                                  sal_uInt32 len);

    RegError    getValue(std::u16string_view valueName, RegValue value) const;

    RegError    getLongListValue(std::u16string_view valueName,
                                  sal_Int32** pValueList,
                                 sal_uInt32* pLen) const;

    RegError    getStringListValue(std::u16string_view valueName,
                                    char*** pValueList,
                                   sal_uInt32* pLen) const;

    RegError    getUnicodeListValue(std::u16string_view valueName,
                                     sal_Unicode*** pValueList,
                                    sal_uInt32* pLen) const;

    RegError    getResolvedKeyName(std::u16string_view keyName,
                                   OUString& resolvedName) const;

    bool isDeleted() const
        { return m_bDeleted; }

    void setDeleted (bool bKeyDeleted)
        { m_bDeleted = bKeyDeleted; }

    bool isModified() const
        { return m_bModified; }

    void setModified (bool bModified = true)
        { m_bModified = bModified; }

    bool        isReadOnly() const
                    { return m_pRegistry->isReadOnly(); }

    sal_uInt32      countSubKeys();

    ORegistry* getRegistry() const
                    { return m_pRegistry; }

    const store::OStoreFile& getStoreFile() const
                    { return m_pRegistry->getStoreFile(); }

    store::OStoreDirectory getStoreDir() const;

    const OUString& getName() const
                    { return m_name; }

    OUString getFullPath(std::u16string_view path) const;

private:
    sal_uInt32               m_refCount;
    OUString                 m_name;
    bool                     m_bDeleted:1;
    bool                     m_bModified:1;
    ORegistry*               m_pRegistry;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
