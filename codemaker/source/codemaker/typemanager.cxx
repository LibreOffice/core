/*************************************************************************
 *
 *  $RCSfile: typemanager.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#include    <codemaker/typemanager.hxx>
#endif

using namespace rtl;

RegistryTypeReaderLoader & getRegistryTypeReaderLoader()
{
    static RegistryTypeReaderLoader aLoader;
    return aLoader;
}

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
    return osl_incrementInterlockedCount(&m_pImpl->m_refCount);
}

sal_Int32 TypeManager::release()
{
    sal_Int32 refCount = 0;
    if (0 == (refCount = osl_decrementInterlockedCount(&m_pImpl->m_refCount)) )
    {
        delete m_pImpl;
    }
    return refCount;;
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

    RegistryLoader loader;
    Registry tmpReg(loader);
    while (iter != regFiles.end())
    {
        if (!tmpReg.open( OStringToOUString(*iter, RTL_TEXTENCODING_UTF8), REG_READONLY))
            m_pImpl->m_registries.push_back(new Registry(tmpReg));
        else
        {
            freeRegistries();
            return sal_False;
        }
        iter++;
    }

    if (m_pImpl->m_isMerged)
    {
        Registry *pTmpReg = new Registry(loader);
        OString tmpName(makeTempName(NULL));

        if (!pTmpReg->create( OStringToOUString(tmpName, RTL_TEXTENCODING_UTF8) ) )
        {
            RegistryKey rootKey;
            RegError ret = REG_NO_ERROR;
            OUString aRoot( RTL_CONSTASCII_USTRINGPARAM("/") );
            iter = regFiles.begin();
            pTmpReg->openRootKey(rootKey);

            while (iter != regFiles.end())
            {
                if ( ret = pTmpReg->mergeKey(rootKey, aRoot, OUString::createFromAscii( *iter )) )
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
                iter++;
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
                RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

                reader = TypeReader(rReaderLoader, pBuffer, valueSize, sal_True);
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
                    RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

                    TypeReader reader(rReaderLoader, pBuffer, valueSize, sal_False);

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

        iter++;
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

            iter++;
        }
    }

    return key;
}

