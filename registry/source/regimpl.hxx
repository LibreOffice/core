/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _REGIMPL_HXX_
#define _REGIMPL_HXX_

#include <set>
#include <hash_map>

#include    <registry/registry.h>
#include    <rtl/ustring.hxx>
#include    <osl/mutex.hxx>
#include    <store/store.hxx>

#define REG_PAGESIZE 512

#define REG_MODE_CREATE     store_AccessCreate
#define REG_MODE_OPEN       store_AccessReadWrite
#define REG_MODE_OPENREAD   store_AccessReadOnly

#define KEY_MODE_CREATE     store_AccessCreate
#define KEY_MODE_OPEN       store_AccessReadWrite
#define KEY_MODE_OPENREAD   store_AccessReadOnly


#define VALUE_MODE_CREATE   store_AccessCreate
#define VALUE_MODE_OPEN     store_AccessReadWrite
#define VALUE_MODE_OPENREAD store_AccessReadOnly

// 5 Bytes = 1 (Byte fuer den Typ) + 4 (Bytes fuer die Groesse der Daten)
#define VALUE_HEADERSIZE    5
#define VALUE_TYPEOFFSET    1
#define VALUE_HEADEROFFSET  5

#define REG_CREATE      0x0004  // allow write accesses

#define REG_GUARD(mutex) \
    osl::Guard< osl::Mutex > aGuard( mutex );

// @@@ using namespace rtl;
// @@@ using namespace osl;

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

    RegError    initRegistry(const rtl::OUString& name,
                             RegAccessMode accessMode);

    RegError    closeRegistry();

    RegError    destroyRegistry(const rtl::OUString& name);

    RegError    acquireKey(RegKeyHandle hKey);
    RegError    releaseKey(RegKeyHandle hKey);

    RegError    createKey(RegKeyHandle hKey,
                          const rtl::OUString& keyName,
                          RegKeyHandle* phNewKey);

    RegError    openKey(RegKeyHandle hKey,
                        const rtl::OUString& keyName,
                        RegKeyHandle* phOpenKey);

    RegError    closeKey(RegKeyHandle hKey);

    RegError    deleteKey(RegKeyHandle hKey, const rtl::OUString& keyName);

    RegError    loadKey(RegKeyHandle hKey,
                        const rtl::OUString& regFileName,
                        sal_Bool bWarings=sal_False,
                        sal_Bool bReport=sal_False);

    RegError    saveKey(RegKeyHandle hKey,
                        const rtl::OUString& regFileName,
                        sal_Bool bWarings=sal_False,
                        sal_Bool bReport=sal_False);

    RegError    dumpRegistry(RegKeyHandle hKey) const;

    ~ORegistry();

    sal_Bool            isReadOnly() const
        { return m_readOnly; }

    sal_Bool            isOpen() const
        { return m_isOpen; }

    ORegKey*    getRootKey();

    const store::OStoreFile& getStoreFile()
        { return m_file; }

    const rtl::OUString&    getName() const
        { return m_name; }

    friend class ORegKey;

private:
    RegError    eraseKey(ORegKey* pKey, const rtl::OUString& keyName);

    RegError    deleteSubkeysAndValues(ORegKey* pKey);

    RegError    loadAndSaveValue(ORegKey* pTargetKey,
                                 ORegKey* pSourceKey,
                                 const rtl::OUString& valueName,
                                 sal_uInt32 nCut,
                                 sal_Bool bWarnings=sal_False,
                                 sal_Bool bReport=sal_False);

    RegError    checkBlop(store::OStoreStream& rValue,
                          const rtl::OUString& sTargetPath,
                          sal_uInt32 srcValueSize,
                          sal_uInt8* pSrcBuffer,
                          sal_Bool bReport=sal_False);

    RegError    mergeModuleValue(store::OStoreStream& rTargetValue,
                                 RegistryTypeReader& reader,
                                 RegistryTypeReader& reader2);

    RegError    loadAndSaveKeys(ORegKey* pTargetKey,
                                ORegKey* pSourceKey,
                                const rtl::OUString& keyName,
                                sal_uInt32 nCut,
                                sal_Bool bWarnings=sal_False,
                                sal_Bool bReport=sal_False);

    RegError    dumpValue(const rtl::OUString& sPath,
                          const rtl::OUString& sName,
                          sal_Int16 nSpace) const;

    RegError    dumpKey(const rtl::OUString& sPath,
                        const rtl::OUString& sName,
                        sal_Int16 nSpace) const;

    typedef std::hash_map< rtl::OUString, ORegKey*, rtl::OUStringHash > KeyMap;

    sal_uInt32      m_refCount;
    osl::Mutex          m_mutex;
    sal_Bool        m_readOnly;
    sal_Bool        m_isOpen;
    rtl::OUString       m_name;
    store::OStoreFile   m_file;
    KeyMap          m_openKeyTable;

    const rtl::OUString ROOT;
};

#endif

