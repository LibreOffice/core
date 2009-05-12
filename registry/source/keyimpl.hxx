/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: keyimpl.hxx,v $
 * $Revision: 1.4.10.1 $
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
#include    <rtl/string.hxx>

using namespace rtl;
using namespace store;

class ORegKey
{
public:

    ORegKey(const OUString& keyName, ORegistry* pReg);

      sal_uInt32    acquire()
        { return ++m_refCount; }

    sal_uInt32  release()
        { return --m_refCount; }

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
                                  sal_Int32* pValueList,
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

    RegError    getKeyType(const OUString& name,
                           RegKeyType* pKeyType) const;

    RegError    getResolvedKeyName(const OUString& keyName,
                                   OUString& resolvedName);

    sal_Bool        isDeleted() const
                    { return m_bDeleted; }

    void        setDeleted(sal_Bool bKeyDeleted)
                    { m_bDeleted = bKeyDeleted; }

    sal_Bool        isReadOnly() const
                    { return m_pRegistry->isReadOnly(); }

    sal_uInt32      countSubKeys();

    ORegistry* getRegistry() const
                    { return m_pRegistry; }

    const OStoreFile& getStoreFile() const
                    { return m_pRegistry->getStoreFile(); }

    OStoreDirectory getStoreDir();

    const OUString& getName() const
                    { return m_name; }

    sal_uInt32 getRefCount() const
                    { return m_refCount; }

    OUString getFullPath(OUString const & path) const;

private:
    sal_uInt32              m_refCount;
    OUString                m_name;
    sal_Bool                m_bDeleted;
    ORegistry*              m_pRegistry;
};



#endif


