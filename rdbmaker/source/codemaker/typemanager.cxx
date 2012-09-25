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

#include    <rtl/alloc.h>
#include    <osl/file.hxx>
#include    <codemaker/typemanager.hxx>

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;
TypeManager::TypeManager()
{
    m_pImpl = new TypeManagerImpl();
    acquire();
}

TypeManager::~TypeManager()
{
    release();
}

sal_Int32 TypeManager::acquire()
{
    return osl_atomic_increment(&m_pImpl->m_refCount);
}

sal_Int32 TypeManager::release()
{
    sal_Int32 refCount = 0;
    if (0 == (refCount = osl_atomic_decrement(&m_pImpl->m_refCount)) )
    {
        delete m_pImpl;
    }
    return refCount;
}

RegistryTypeManager::RegistryTypeManager()
{
    m_pImpl = new RegistryTypeManagerImpl();
    acquire();
}

RegistryTypeManager::~RegistryTypeManager()
{
    release();
}

void RegistryTypeManager::acquire()
{
    TypeManager::acquire();
}

void RegistryTypeManager::release()
{
    if (0 == TypeManager::release())
    {
        if (m_pImpl->m_pMergedRegistry)
        {
            if (m_pImpl->m_pMergedRegistry->isValid())
            {
                m_pImpl->m_pMergedRegistry->destroy(OUString());
            }

            delete m_pImpl->m_pMergedRegistry;
        }

        if (m_pImpl->m_registries.size() > 0)
        {
            freeRegistries();
        }

        delete m_pImpl;
    }
}

sal_Bool RegistryTypeManager::init(sal_Bool bMerged, const StringVector& regFiles)
{
    m_pImpl->m_isMerged = bMerged && (regFiles.size() > 1);

    if (regFiles.empty())
        return sal_False;

    StringVector::const_iterator iter = regFiles.begin();

    Registry tmpReg;
    while (iter != regFiles.end())
    {
        if (!tmpReg.open( convertToFileUrl(*iter), REG_READONLY))
            m_pImpl->m_registries.push_back(new Registry(tmpReg));
        else
        {
            freeRegistries();
            return sal_False;
        }
        ++iter;
    }

    if (m_pImpl->m_isMerged)
    {
        Registry *pTmpReg = new Registry;
        OUString tmpName;
        osl::FileBase::createTempFile(0, 0, &tmpName);
        if (!pTmpReg->create(tmpName))
        {
            RegistryKey rootKey;
            RegError ret = REG_NO_ERROR;
            OUString aRoot( RTL_CONSTASCII_USTRINGPARAM("/") );
            iter = regFiles.begin();
            pTmpReg->openRootKey(rootKey);

            while (iter != regFiles.end())
            {
                if ( (ret = pTmpReg->mergeKey(rootKey, aRoot, convertToFileUrl( *iter ))) )
                {
                    if (ret != REG_MERGE_CONFLICT)
                    {
                        freeRegistries();
                        rootKey.closeKey();
                        pTmpReg->destroy( OUString() );
                        delete pTmpReg;
                        return sal_False;
                    }
                }
                ++iter;
            }

            m_pImpl->m_pMergedRegistry = pTmpReg;
            freeRegistries();
        } else
        {
            delete pTmpReg;
            freeRegistries();
            return sal_False;
        }
    }

    return sal_True;
}

TypeReader RegistryTypeManager::getTypeReader(const OString& name)
{
    TypeReader reader;
    RegistryKey key(searchTypeKey(name));

    if (key.isValid())
    {
        RegValueType    valueType;
        sal_uInt32      valueSize;

        if (!key.getValueInfo(OUString(), &valueType, &valueSize))
        {
            sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
            if (!key.getValue(OUString(), pBuffer))
            {
                reader = TypeReader(pBuffer, valueSize, true);
            }
            rtl_freeMemory(pBuffer);
        }
    }
    return reader;
}

RTTypeClass RegistryTypeManager::getTypeClass(const OString& name)
{
    if (m_pImpl->m_t2TypeClass.count(name) > 0)
    {
        return m_pImpl->m_t2TypeClass[name];
    } else
    {
        RegistryKey key(searchTypeKey(name));

        if (key.isValid())
        {
            RegValueType    valueType;
            sal_uInt32      valueSize;

            if (!key.getValueInfo(OUString(), &valueType, &valueSize))
            {
                sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
                if (!key.getValue(OUString(), pBuffer))
                {
                    TypeReader reader(pBuffer, valueSize, false);

                    RTTypeClass ret = reader.getTypeClass();

                    rtl_freeMemory(pBuffer);

                    m_pImpl->m_t2TypeClass[name] = ret;
                    return ret;
                }
                rtl_freeMemory(pBuffer);
            }
        }
    }

    return RT_TYPE_INVALID;
}

void RegistryTypeManager::setBase(const OString& base)
{
    m_pImpl->m_base = base;

    if (base.lastIndexOf('/') != (base.getLength() - 1))
    {
        m_pImpl->m_base += "/";
    }
}

void RegistryTypeManager::freeRegistries()
{
    RegistryList::const_iterator iter = m_pImpl->m_registries.begin();

    while (iter != m_pImpl->m_registries.end())
    {
        delete *iter;

        ++iter;
    }

}

RegistryKey RegistryTypeManager::searchTypeKey(const OString& name)
{
    RegistryKey key, rootKey;

    if (m_pImpl->m_isMerged)
    {
        if (!m_pImpl->m_pMergedRegistry->openRootKey(rootKey))
        {
            rootKey.openKey(OStringToOUString(m_pImpl->m_base + name, RTL_TEXTENCODING_UTF8), key);
        }
    } else
    {
        RegistryList::const_iterator iter = m_pImpl->m_registries.begin();

        while (iter != m_pImpl->m_registries.end())
        {
            if (!(*iter)->openRootKey(rootKey))
            {
                if (!rootKey.openKey(OStringToOUString(m_pImpl->m_base + name, RTL_TEXTENCODING_UTF8), key))
                    break;
            }

            ++iter;
        }
    }

    return key;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
