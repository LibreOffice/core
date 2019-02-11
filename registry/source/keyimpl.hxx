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

#include "regimpl.hxx"
#include <rtl/ustring.hxx>

class ORegKey
{
public:

    ORegKey(const OUString& keyName, ORegistry* pReg);
    ~ORegKey();

    void        acquire()
        { ++m_refCount; }

    sal_uInt32  release()
        { return --m_refCount; }

    RegError    releaseKey(RegKeyHandle hKey);

    RegError    createKey(const OUString& keyName, RegKeyHandle* phNewKey);

    RegError    openKey(const OUString& keyName, RegKeyHandle* phOpenKey);

    RegError    openSubKeys(const OUString& keyName,
                            RegKeyHandle** phOpenSubKeys,
                            sal_uInt32* pnSubKeys);

    RegError    getKeyNames(const OUString& keyName,
                            rtl_uString*** pSubKeyNames,
                            sal_uInt32* pnSubKeys);

    RegError    closeKey(RegKeyHandle hKey);

    RegError    deleteKey(const OUString& keyName);

    RegError    getValueInfo(const OUString& valueName,
                             RegValueType* pValueTye,
                             sal_uInt32* pValueSize) const;

    RegError    setValue(const OUString& valueName,
                         RegValueType vType,
                         RegValue value,
                         sal_uInt32 vSize);

    RegError    setLongListValue(const OUString& valueName,
                                 sal_Int32 const * pValueList,
                                 sal_uInt32 len);

    RegError    setStringListValue(const OUString& valueName,
                                   sal_Char** pValueList,
                                  sal_uInt32 len);

    RegError    setUnicodeListValue(const OUString& valueName,
                                   sal_Unicode** pValueList,
                                  sal_uInt32 len);

    RegError    getValue(const OUString& valueName, RegValue value) const;

    RegError    getLongListValue(const OUString& valueName,
                                  sal_Int32** pValueList,
                                 sal_uInt32* pLen) const;

    RegError    getStringListValue(const OUString& valueName,
                                    sal_Char*** pValueList,
                                   sal_uInt32* pLen) const;

    RegError    getUnicodeListValue(const OUString& valueName,
                                     sal_Unicode*** pValueList,
                                    sal_uInt32* pLen) const;

    RegError    getResolvedKeyName(const OUString& keyName,
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

    OUString getFullPath(OUString const & path) const;

private:
    sal_uInt32              m_refCount;
    OUString const           m_name;
    bool                     m_bDeleted:1;
    bool                     m_bModified:1;
    ORegistry*              m_pRegistry;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
