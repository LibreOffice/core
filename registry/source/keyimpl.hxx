/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _KEYIMPL_HXX_
#define _KEYIMPL_HXX_

#include    <registry/registry.h>
#include    "regimpl.hxx"
#include    <rtl/ustring.hxx>

class ORegKey
{
public:

    ORegKey(const rtl::OUString& keyName, ORegistry* pReg);
    ~ORegKey();

      sal_uInt32    acquire()
        { return ++m_refCount; }

    sal_uInt32  release()
        { return --m_refCount; }

    RegError    acquireKey(RegKeyHandle hKey);
    RegError    releaseKey(RegKeyHandle hKey);

    RegError    createKey(const rtl::OUString& keyName, RegKeyHandle* phNewKey);

    RegError    openKey(const rtl::OUString& keyName, RegKeyHandle* phOpenKey);

    RegError    openSubKeys(const rtl::OUString& keyName,
                            RegKeyHandle** phOpenSubKeys,
                            sal_uInt32* pnSubKeys);

    RegError    getKeyNames(const rtl::OUString& keyName,
                            rtl_uString*** pSubKeyNames,
                            sal_uInt32* pnSubKeys);

    RegError    closeKey(RegKeyHandle hKey);

    RegError    deleteKey(const rtl::OUString& keyName);

    RegError    getValueInfo(const rtl::OUString& valueName,
                             RegValueType* pValueTye,
                             sal_uInt32* pValueSize) const;

    RegError    setValue(const rtl::OUString& valueName,
                         RegValueType vType,
                         RegValue value,
                         sal_uInt32 vSize);

    RegError    setLongListValue(const rtl::OUString& valueName,
                                  sal_Int32* pValueList,
                                 sal_uInt32 len);

    RegError    setStringListValue(const rtl::OUString& valueName,
                                   sal_Char** pValueList,
                                  sal_uInt32 len);

    RegError    setUnicodeListValue(const rtl::OUString& valueName,
                                   sal_Unicode** pValueList,
                                  sal_uInt32 len);

    RegError    getValue(const rtl::OUString& valueName, RegValue value) const;

    RegError    getLongListValue(const rtl::OUString& valueName,
                                  sal_Int32** pValueList,
                                 sal_uInt32* pLen) const;

    RegError    getStringListValue(const rtl::OUString& valueName,
                                    sal_Char*** pValueList,
                                   sal_uInt32* pLen) const;

    RegError    getUnicodeListValue(const rtl::OUString& valueName,
                                     sal_Unicode*** pValueList,
                                    sal_uInt32* pLen) const;

    RegError    getKeyType(const rtl::OUString& name,
                           RegKeyType* pKeyType) const;

    RegError    getResolvedKeyName(const rtl::OUString& keyName,
                                   rtl::OUString& resolvedName);

    bool isDeleted() const
        { return m_bDeleted != 0; }

    void setDeleted (sal_Bool bKeyDeleted)
        { m_bDeleted = bKeyDeleted ? 1 : 0; }

    bool isModified() const
        { return m_bModified != 0; }

    void setModified (bool bModified = true)
        { m_bModified = bModified ? 1 : 0; }

    sal_Bool        isReadOnly() const
                    { return m_pRegistry->isReadOnly(); }

    sal_uInt32      countSubKeys();

    ORegistry* getRegistry() const
                    { return m_pRegistry; }

    const store::OStoreFile& getStoreFile() const
                    { return m_pRegistry->getStoreFile(); }

    store::OStoreDirectory getStoreDir();

    const rtl::OUString& getName() const
                    { return m_name; }

    sal_uInt32 getRefCount() const
                    { return m_refCount; }

    rtl::OUString getFullPath(rtl::OUString const & path) const;

private:
    sal_uInt32              m_refCount;
    rtl::OUString           m_name;
    int                     m_bDeleted:1;
    int                     m_bModified:1;
    ORegistry*              m_pRegistry;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
