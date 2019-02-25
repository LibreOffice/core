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

#ifndef INCLUDED_REGISTRY_REGISTRY_HXX
#define INCLUDED_REGISTRY_REGISTRY_HXX

#include <registry/regdllapi.h>
#include <registry/regtype.h>
#include <rtl/ustring.hxx>

extern "C" {

/** specifies a collection of function pointers which represents the complete registry C-API.

    These function pointers are used by the C++ wrapper to call the C-API.
*/
struct Registry_Api
{
    void        (REGISTRY_CALLTYPE *acquire)            (RegHandle);
    void        (REGISTRY_CALLTYPE *release)            (RegHandle);
    sal_Bool    (REGISTRY_CALLTYPE *isReadOnly)         (RegHandle);
    RegError    (REGISTRY_CALLTYPE *openRootKey)        (RegHandle, RegKeyHandle*);
    RegError    (REGISTRY_CALLTYPE *getName)            (RegHandle, rtl_uString**);
    RegError    (REGISTRY_CALLTYPE *createRegistry)     (rtl_uString*, RegHandle*);
    RegError    (REGISTRY_CALLTYPE *openRegistry)       (rtl_uString*, RegHandle*, RegAccessMode);
    RegError    (REGISTRY_CALLTYPE *closeRegistry)      (RegHandle);
    RegError    (REGISTRY_CALLTYPE *destroyRegistry)    (RegHandle, rtl_uString*);
    RegError    (REGISTRY_CALLTYPE *mergeKey)           (RegHandle, RegKeyHandle, rtl_uString*, rtl_uString*, sal_Bool, sal_Bool);
    void        (REGISTRY_CALLTYPE *acquireKey)         (RegKeyHandle);
    void        (REGISTRY_CALLTYPE *releaseKey)         (RegKeyHandle);
    sal_Bool    (REGISTRY_CALLTYPE *isKeyReadOnly)      (RegKeyHandle);
    RegError    (REGISTRY_CALLTYPE *getKeyName)         (RegKeyHandle, rtl_uString**);
    RegError    (REGISTRY_CALLTYPE *createKey)          (RegKeyHandle, rtl_uString*, RegKeyHandle*);
    RegError    (REGISTRY_CALLTYPE *openKey)            (RegKeyHandle, rtl_uString*, RegKeyHandle*);
    RegError    (REGISTRY_CALLTYPE *openSubKeys)        (RegKeyHandle, rtl_uString*, RegKeyHandle**, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *closeSubKeys)       (RegKeyHandle*, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *deleteKey)          (RegKeyHandle, rtl_uString*);
    RegError    (REGISTRY_CALLTYPE *closeKey)           (RegKeyHandle);
    RegError    (REGISTRY_CALLTYPE *setValue)           (RegKeyHandle, rtl_uString*, RegValueType, RegValue, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *setLongListValue)   (RegKeyHandle, rtl_uString*, sal_Int32 const *, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *setStringListValue) (RegKeyHandle, rtl_uString*, sal_Char**, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *setUnicodeListValue)(RegKeyHandle, rtl_uString*, sal_Unicode**, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *getValueInfo)       (RegKeyHandle, rtl_uString*, RegValueType*, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *getValue)           (RegKeyHandle, rtl_uString*, RegValue);
    RegError    (REGISTRY_CALLTYPE *getLongListValue)   (RegKeyHandle, rtl_uString*, sal_Int32**, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *getStringListValue) (RegKeyHandle, rtl_uString*, sal_Char***, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *getUnicodeListValue)(RegKeyHandle, rtl_uString*, sal_Unicode***, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *freeValueList)      (RegValueType, RegValue, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *getResolvedKeyName) (RegKeyHandle, rtl_uString*, sal_Bool, rtl_uString**);
    RegError    (REGISTRY_CALLTYPE *getKeyNames)        (RegKeyHandle, rtl_uString*, rtl_uString***, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *freeKeyNames)       (rtl_uString**, sal_uInt32);
};

/** the API initialization function.
*/
REG_DLLPUBLIC Registry_Api* REGISTRY_CALLTYPE initRegistry_Api();

}

class RegistryKey;


/** The Registry provides the functionality to read and write information in a registry file.

    The class is implemented inline and use a C-Api.
*/
class Registry final
{
public:
    /** Default constructor.
     */
    inline Registry();

    /// Copy constructor
    inline Registry(const Registry& toCopy);

    Registry(Registry && other): m_pApi(other.m_pApi), m_hImpl(other.m_hImpl)
    { other.m_hImpl = nullptr; }

    /// Destructor. The Destructor close the registry if it is open.
    inline ~Registry();

    /// Assign operator
    inline Registry& operator = (const Registry& toAssign);

    Registry & operator =(Registry && other) {
        if (m_hImpl != nullptr) {
            m_pApi->release(m_hImpl);
        }
        m_hImpl = other.m_hImpl;
        other.m_hImpl = nullptr;
        return *this;
    }

    /// checks if the registry points to a valid registry data file.
    inline bool isValid() const;

    /** returns the access mode of the registry.

        @return TRUE if the access mode is readonly else FALSE.
    */
    inline bool     isReadOnly() const;

    /** opens the root key of the registry.

        @param  rRootKey reference to a RegistryKey which is filled with the rootkey.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError openRootKey(RegistryKey& rRootKey);

    /// returns the name of the current registry data file.
    inline OUString getName();

    /** creates a new registry with the specified name and creates a root key.

        @param  registryName specifies the name of the new registry.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError create(const OUString& registryName);

    /** opens a registry with the specified name.

        If the registry already points to a valid registry, the old registry will be closed.
        @param  registryName specifies a registry name.
        @param  accessMode specifies the access mode for the registry, RegAccessMode::READONLY or RegAccessMode::READWRITE.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError open(const OUString& registryName,
                            RegAccessMode accessMode);

    /// closes explicitly the current registry data file.
    inline RegError close();

    /** destroys a registry.

        @param registryName specifies a registry name, if the name is an empty string the registry
                            itself will be destroyed.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError destroy(const OUString& registryName);

    /** merges the registry information of the specified key with the registry
        information of the specified file.

        All existing keys will be extended and existing key values will be overwritten.
        @param  rKey references a currently open key. The key which information is merged by this
                     function is a subkey of this key
        @param  keyName specifies the name of the key which will be merged.
                        If keyName is an empty string the registry information under the key specified
                        by rKey is merged with the information from the specified file.
        @param  regFileName specifies the file containing the registry information.
        @param  bReport if TRUE the function reports warnings on stdout if a key already exists.
        @return RegError::NO_ERROR if succeeds else an error code. If it returns an error the registry will
                restore the state before merging.
    */
    inline RegError mergeKey(RegistryKey& rKey,
                                const OUString& keyName,
                                const OUString& regFileName,
                                bool bReport);

    friend class RegistryKey;
    friend class RegistryKeyArray;
    friend class RegistryKeyNames;

    /// returns the used registry Api.
    const Registry_Api* getApi() const { return m_pApi; }

private:
    /// stores the used and initialized registry Api.
    const Registry_Api*                          m_pApi;
    /// stores the handle of the underlying registry file on which most of the functions work.
    RegHandle                                    m_hImpl;
};


/** RegistryKeyArray represents an array of open keys.

    RegistryKeyArray is a helper class to work with an array of keys.
*/
class RegistryKeyArray
{
public:
    /// Default constructor
    inline RegistryKeyArray();

    /// Destructor, all subkeys will be closed.
    inline ~RegistryKeyArray();

    /// returns the open key specified by index.
    inline RegistryKey getElement(sal_uInt32 index);

    /// returns the length of the array.
    inline sal_uInt32 getLength() const;

    friend class RegistryKey;
protected:
    /** sets the data of the key array.

        @param registry specifies the registry files where the keys are located.
        @param phKeys points to an array of open keys.
        @param length specifies the length of the array specified by phKeys.
     */
    inline void setKeyHandles(Registry const & registry, RegKeyHandle* phKeys, sal_uInt32 length);
private:
    /// stores the number of open subkeys, the number of elements.
    sal_uInt32      m_length;
    /// stores an array of open subkeys.
    RegKeyHandle*   m_phKeys;
    /// stores the handle to the registry file where the appropriate keys are located.
    Registry        m_registry;
};


/** RegistryKeyNames represents an array of key names.

    RegistryKeyNames is a helper class to work with an array of key names.
*/
class RegistryKeyNames
{
public:
    /// Default constructor
    inline RegistryKeyNames();

    /// Destructor, the internal array with key names will be deleted.
    inline ~RegistryKeyNames();

    /// returns the name of the key specified by index.
    inline OUString getElement(sal_uInt32 index);

    /// returns the length of the array.
    inline sal_uInt32 getLength() const;

    friend class RegistryKey;
protected:
    /** sets the data of the array.

        @param registry specifies the registry files where the keys are located.
        @param pKeyNames points to an array of key names.
        @param length specifies the length of the array specified by pKeyNames.
     */
    inline void setKeyNames(Registry const & registry, rtl_uString** pKeyNames, sal_uInt32 length);
private:
    /// stores the number of key names, the number of elements.
    sal_uInt32      m_length;
    /// stores an array of key names.
    rtl_uString**   m_pKeyNames;
    /// stores the handle to the registry file where the appropriate keys are located.
    Registry        m_registry;
};


/** RegistryValueList represents a value list of the specified type.

    RegistryValueList is a helper class to work with a list value.
*/
template<class ValueType>
class RegistryValueList final
{
public:
    /// Default constructor
    RegistryValueList()
        : m_length(0)
        , m_pValueList(nullptr)
        , m_valueType(RegValueType::NOT_DEFINED)
        {}

    /// Destructor, the internal value list will be freed.
    ~RegistryValueList()
    {
        if (m_pValueList)
        {
            m_registry.getApi()->freeValueList(m_valueType, m_pValueList, m_length);
        }
    }

    /// returns the value of the list specified by index.
    ValueType getElement(sal_uInt32 index)
    {
        if (m_registry.isValid() && index < m_length)
        {
            return m_pValueList[index];
        } else
        {
            return 0;
        }
    }

    /// returns the length of the list.
    sal_uInt32 getLength()
    {
        return m_length;
    }

    friend class RegistryKey;

private:
    /** sets the data of the value list.

        @param registry specifies the registry files where the appropriate key is located.
        @param valueType specifies the type of the list values.
        @param pValueList points to a value list.
        @param length specifies the length of the list.
     */
    void setValueList(Registry& registry, RegValueType valueType,
                      ValueType* pValueList, sal_uInt32 length)
    {
        m_length = length;
        m_pValueList = pValueList;
        m_valueType = valueType;
        m_registry = registry;
    }

    /// stores the length of the list, the number of elements.
    sal_uInt32      m_length;
    /// stores the value list.
    ValueType*      m_pValueList;
    /// stores the type of the list elements
    RegValueType    m_valueType;
    /** stores the handle to the registry file where the appropriate key to this
        value is located.
    */
    Registry        m_registry;
};


/** RegistryKey reads or writes information of the underlying key in a registry.

    Class is inline and use a load on call C-Api.
*/
class RegistryKey
{
public:
    /// Default constructor
    inline RegistryKey();

    /// Copy constructor
    inline RegistryKey(const RegistryKey& toCopy);

    /// Destructor, close the key if it references an open one.
    inline ~RegistryKey();

    /// Assign operator
    inline RegistryKey& operator = (const RegistryKey& toAssign);

    /// checks if the key points to a valid registry key.
    inline bool isValid() const;

    /** returns the access mode of the key.

        @return TRUE if access mode is read only else FALSE.
    */
    inline bool     isReadOnly() const;

    /// returns the full qualified name of the key beginning with the rootkey.
    inline OUString getName();

    /** creates a new key or opens a key if the specified key already exists.

        The specified keyname is relative to this key.
        @param  keyName specifies the name of the key which will be opened or created.
        @param  rNewKey references a RegistryKey which will be filled with the new or open key.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError createKey(const OUString& keyName,
                              RegistryKey& rNewKey);

    /** opens the specified key.

        The specified keyname is relative to this key.
        @param  keyName specifies the name of the key which will be opened.
        @param  rOpenKey references a RegistryKey which will be filled with the open key.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError openKey(const OUString& keyName,
                              RegistryKey& rOpenKey);

    /** opens all subkeys of the specified key.

        The specified keyname is relative to this key.
        @param  keyName specifies the name of the key which subkeys will be opened.
        @param  rSubKeys reference a RegistryKeyArray which will be filled with the open subkeys.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError openSubKeys(const OUString& keyName,
                                    RegistryKeyArray& rSubKeys);

    /** returns an array with the names of all subkeys of the specified key.

        The specified keyname is relative to this key.
        @param  keyName specifies the name of the key which subkey names will be returned.
        @param  rSubKeyNames reference a RegistryKeyNames array which will be filled with the subkey names.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError getKeyNames(const OUString& keyName,
                                    RegistryKeyNames& rSubKeyNames);

    /** deletes the specified key.

        @param  keyName specifies the name of the key which will be deleted.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError deleteKey(const OUString& keyName);

    /// closes explicitly the current key
    inline RegError closeKey();

    /// releases the current key
    inline void releaseKey();

    /** sets a value of a key.

        @param  keyName specifies the name of the key which value will be set.
                        If keyName is an empty string, the value will be set for the key
                        specified by hKey.
        @param  valueType specifies the type of the value.
        @param  pValue points to a memory block containing the data for the value.
        @param  valueSize specifies the size of pData in bytes
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError setValue(const OUString& keyName,
                                RegValueType valueType,
                                RegValue pValue,
                                sal_uInt32 valueSize);

    /** sets a long list value of a key.

        @param  keyName specifies the name of the key which value will be set.
                        If keyName is an empty string, the value will be set for the key
                        specified by hKey.
        @param  pValueList points to an array of longs containing the data for the value.
        @param  len specifies the length of the list (the array referenced by pValueList).
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError setLongListValue(const OUString& keyName,
                                         sal_Int32 const * pValueList,
                                         sal_uInt32 len);

    /** sets an ascii list value of a key.

        @param  keyName specifies the name of the key which value will be set.
                        If keyName is an empty string, the value will be set for the key
                        specified by hKey.
        @param  pValueList points to an array of sal_Char* containing the data for the value.
        @param  len specifies the length of the list (the array referenced by pValueList).
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError setStringListValue(const OUString& keyName,
                                           sal_Char** pValueList,
                                           sal_uInt32 len);

    /** sets an unicode string list value of a key.

        @param  keyName specifies the name of the key which value will be set.
                        If keyName is an empty string, the value will be set for the key
                        specified by hKey.
        @param  pValueList points to an array of sal_Unicode* containing the data for the value.
        @param  len specifies the length of the list (the array referenced by pValueList).
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError setUnicodeListValue(const OUString& keyName,
                                            sal_Unicode** pValueList,
                                              sal_uInt32 len);

    /** gets info about type and size of a value.

        @param  keyName specifies the name of the key which value info will be returned.
                        If keyName is an empty string, the value info of the key
                        specified by hKey will be returned.
        @param  pValueType returns the type of the value.
        @param  pValueSize returns the size of the value in bytes or the length of a list value.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError getValueInfo(const OUString& keyName,
                                    RegValueType* pValueType,
                                    sal_uInt32* pValueSize);

    /** gets the value of a key.

        @param  keyName specifies the name of the key which value will be returned.
                        If keyName is an empty string, the value is get from the key
                        specified by hKey.
        @param  pValue points to an allocated memory block receiving the data of the value.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError getValue(const OUString& keyName,
                                RegValue pValue);

    /** gets a long list value of a key.

        @param  keyName specifies the name of the key which value will be returned.
                        If keyName is an empty string, the value is get from the key
                        specified by hKey.
        @param  rValueList references a RegistryValueList which will be filled with the long values.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError getLongListValue(const OUString& keyName,
                                          RegistryValueList<sal_Int32>& rValueList);

    /** gets an ascii list value of a key.

        @param  keyName specifies the name of the key which value will be returned.
                        If keyName is an empty string, the value is get from the key
                        specified by hKey.
        @param  rValueList references a RegistryValueList which will be filled with the ascii values.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError getStringListValue(const OUString& keyName,
                                           RegistryValueList<sal_Char*>& rValueList);

    /** gets a unicode value of a key.

        @param  keyName specifies the name of the key which value will be returned.
                        If keyName is an empty string, the value is get from the key
                        specified by hKey.
        @param  rValueList reference a RegistryValueList which will be filled with the unicode values.
        @return RegError::NO_ERROR if succeeds else an error code.
    */
    inline RegError getUnicodeListValue(const OUString& keyName,
                                              RegistryValueList<sal_Unicode*>& rValueList);

    /** resolves a keyname.

        @param[in]  keyName specifies the name of the key which will be resolved relative to this key.
                        The resolved name will be prefixed with the name of this key.
        @param[out] rResolvedName the resolved name.
        @return RegError::NO_ERROR if succeeds else an error code.
     */
    inline RegError getResolvedKeyName(const OUString& keyName,
                                           OUString& rResolvedName) const;

    /// returns the name of the registry in which the key is defined.
    inline OUString getRegistryName();

    friend class Registry;
public:
    /// @cond INTERNAL

    /** Constructor, which initialize a RegistryKey with registry and an valid key handle.

        This constructor is internal only.
    */
    inline RegistryKey(Registry const & registry,
                       RegKeyHandle hKey);

protected:
    /** sets the internal registry on which this key should work.
     */
    inline void setRegistry(Registry const & registry);

    /// @endcond

    /// stores the registry on which this key works
    Registry        m_registry;
    /// stores the current key handle of this key
    RegKeyHandle    m_hImpl;
};


inline RegistryKeyArray::RegistryKeyArray()
    : m_length(0)
    , m_phKeys(nullptr)
{
}

inline RegistryKeyArray::~RegistryKeyArray()
{
    if (m_phKeys)
        m_registry.m_pApi->closeSubKeys(m_phKeys, m_length);
}

inline RegistryKey RegistryKeyArray::getElement(sal_uInt32 index)
{
    if (m_registry.isValid() && index < m_length)
        return RegistryKey(m_registry, m_phKeys[index]);
    else
        return RegistryKey();
}

inline sal_uInt32 RegistryKeyArray::getLength() const
{
    return m_length;
}

inline void RegistryKeyArray::setKeyHandles(Registry const & registry,
                                            RegKeyHandle* phKeys,
                                            sal_uInt32 length)
{
    m_phKeys = phKeys;
    m_length = length;
    m_registry = registry;
}

inline RegistryKeyNames::RegistryKeyNames()
    : m_length(0)
    , m_pKeyNames(nullptr)
{
}

inline RegistryKeyNames::~RegistryKeyNames()
{
    if (m_pKeyNames)
        m_registry.m_pApi->freeKeyNames(m_pKeyNames, m_length);
}

inline OUString RegistryKeyNames::getElement(sal_uInt32 index)
{

    if (m_pKeyNames && index < m_length)
        return m_pKeyNames[index];
    else
        return OUString();
}

inline sal_uInt32 RegistryKeyNames::getLength() const
{
    return m_length;
}

inline void RegistryKeyNames::setKeyNames(Registry const & registry,
                                          rtl_uString** pKeyNames,
                                          sal_uInt32 length)
{
    m_pKeyNames = pKeyNames;
    m_length = length;
    m_registry = registry;
}

inline RegistryKey::RegistryKey()
    : m_hImpl(nullptr)
    { }

/// @cond INTERNAL
inline RegistryKey::RegistryKey(Registry const & registry, RegKeyHandle hKey)
    : m_registry(registry)
    , m_hImpl(hKey)
    {
        if (m_hImpl)
            m_registry.m_pApi->acquireKey(m_hImpl);
    }
/// @endcond

inline RegistryKey::RegistryKey(const RegistryKey& toCopy)
    : m_registry(toCopy.m_registry)
    , m_hImpl(toCopy.m_hImpl)
    {
        if (m_hImpl)
            m_registry.m_pApi->acquireKey(m_hImpl);
    }

/// @cond INTERNAL
inline void RegistryKey::setRegistry(Registry const & registry)
    {
        m_registry = registry;
    }
/// @endcond

inline RegistryKey::~RegistryKey()
    {
        if (m_hImpl)
            m_registry.m_pApi->releaseKey(m_hImpl);
    }

inline RegistryKey& RegistryKey::operator = (const RegistryKey& toAssign)
{
    m_registry = toAssign.m_registry;

    if (toAssign.m_hImpl)
        m_registry.m_pApi->acquireKey(toAssign.m_hImpl);
    if (m_hImpl)
        m_registry.m_pApi->releaseKey(m_hImpl);
    m_hImpl = toAssign.m_hImpl;

    return *this;
}

inline bool RegistryKey::isValid() const
    {  return (m_hImpl != nullptr); }

inline bool RegistryKey::isReadOnly() const
    {
        if  (m_registry.isValid())
            return m_registry.m_pApi->isKeyReadOnly(m_hImpl);
        else
            return false;
    }

inline OUString RegistryKey::getName()
    {
        OUString sRet;
        if (m_registry.isValid())
            m_registry.m_pApi->getKeyName(m_hImpl, &sRet.pData);
        return sRet;
    }

inline RegError RegistryKey::createKey(const OUString& keyName,
                                          RegistryKey& rNewKey)
    {
        if (rNewKey.isValid()) rNewKey.closeKey();
        if (m_registry.isValid())
        {
            RegError ret = m_registry.m_pApi->createKey(m_hImpl, keyName.pData, &rNewKey.m_hImpl);
            if (ret == RegError::NO_ERROR) rNewKey.setRegistry(m_registry);
            return ret;
        } else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::openKey(const OUString& keyName,
                                     RegistryKey& rOpenKey)
    {
        if (rOpenKey.isValid()) rOpenKey.closeKey();
        if (m_registry.isValid())
        {
            RegError ret = m_registry.m_pApi->openKey(m_hImpl, keyName.pData,
                                                    &rOpenKey.m_hImpl);
            if (ret == RegError::NO_ERROR) rOpenKey.setRegistry(m_registry);
            return ret;
        } else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::openSubKeys(const OUString& keyName,
                                         RegistryKeyArray& rSubKeys)
    {
        if (m_registry.isValid())
        {
            RegError        ret = RegError::NO_ERROR;
            RegKeyHandle*   pSubKeys;
            sal_uInt32      nSubKeys;
            ret = m_registry.m_pApi->openSubKeys(m_hImpl, keyName.pData,
                                                      &pSubKeys, &nSubKeys);
            if ( ret != RegError::NO_ERROR)
            {
                return ret;
            } else
            {
                rSubKeys.setKeyHandles(m_registry, pSubKeys, nSubKeys);
                return ret;
            }
        } else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::getKeyNames(const OUString& keyName,
                                             RegistryKeyNames& rSubKeyNames)
    {
        if (m_registry.isValid())
        {
            RegError        ret = RegError::NO_ERROR;
            rtl_uString**   pSubKeyNames;
            sal_uInt32      nSubKeys;
            ret = m_registry.m_pApi->getKeyNames(m_hImpl, keyName.pData,
                                                  &pSubKeyNames, &nSubKeys);
            if ( ret != RegError::NO_ERROR)
            {
                return ret;
            } else
            {
                rSubKeyNames.setKeyNames(m_registry, pSubKeyNames, nSubKeys);
                return ret;
            }
        } else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::deleteKey(const OUString& keyName)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->deleteKey(m_hImpl, keyName.pData);
        else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::closeKey()
    {
        if (m_registry.isValid())
        {
            RegError ret = m_registry.m_pApi->closeKey(m_hImpl);
            if (ret == RegError::NO_ERROR)
            {
                m_hImpl = nullptr;
                m_registry = Registry();
            }
            return ret;
        } else
            return RegError::INVALID_KEY;
    }

inline void RegistryKey::releaseKey()
{
    if (m_registry.isValid() && (m_hImpl != nullptr))
    {
        m_registry.m_pApi->releaseKey(m_hImpl);
        m_hImpl = nullptr;
    }
}

inline RegError RegistryKey::setValue(const OUString& keyName,
                                              RegValueType valueType,
                                           RegValue pValue,
                                              sal_uInt32 valueSize)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->setValue(m_hImpl, keyName.pData, valueType,
                                                pValue, valueSize);
        else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::setLongListValue(const OUString& keyName,
                                                  sal_Int32 const * pValueList,
                                                  sal_uInt32 len)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->setLongListValue(m_hImpl, keyName.pData,
                                                           pValueList, len);
        else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::setStringListValue(const OUString& keyName,
                                                   sal_Char** pValueList,
                                                   sal_uInt32 len)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->setStringListValue(m_hImpl, keyName.pData,
                                                            pValueList, len);
        else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::setUnicodeListValue(const OUString& keyName,
                                                        sal_Unicode** pValueList,
                                                        sal_uInt32 len)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->setUnicodeListValue(m_hImpl, keyName.pData,
                                                              pValueList, len);
        else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::getValueInfo(const OUString& keyName,
                                                  RegValueType* pValueType,
                                                  sal_uInt32* pValueSize)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->getValueInfo(m_hImpl, keyName.pData, pValueType, pValueSize);
        else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::getValue(const OUString& keyName,
                                        RegValue pValue)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->getValue(m_hImpl, keyName.pData, pValue);
        else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::getLongListValue(const OUString& keyName,
                                              RegistryValueList<sal_Int32>& rValueList)
    {
        if (m_registry.isValid())
        {
            RegError    ret = RegError::NO_ERROR;
            sal_Int32*  pValueList;
            sal_uInt32  length;
            ret = m_registry.m_pApi->getLongListValue(m_hImpl, keyName.pData,
                                                      &pValueList, &length);
            if ( ret != RegError::NO_ERROR)
            {
                return ret;
            } else
            {
                rValueList.setValueList(m_registry, RegValueType::LONGLIST,
                                        pValueList, length);
                return ret;
            }
        } else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::getStringListValue(const OUString& keyName,
                                                      RegistryValueList<sal_Char*>& rValueList)
    {
        if (m_registry.isValid())
        {
            RegError    ret = RegError::NO_ERROR;
            sal_Char**  pValueList;
            sal_uInt32  length;
            ret = m_registry.m_pApi->getStringListValue(m_hImpl, keyName.pData,
                                                      &pValueList, &length);
            if ( ret != RegError::NO_ERROR )
            {
                return ret;
            } else
            {
                rValueList.setValueList(m_registry, RegValueType::STRINGLIST,
                                        pValueList, length);
                return ret;
            }
        } else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::getUnicodeListValue(const OUString& keyName,
                                              RegistryValueList<sal_Unicode*>& rValueList)
    {
        if (m_registry.isValid())
        {
            RegError        ret = RegError::NO_ERROR;
            sal_Unicode**   pValueList;
            sal_uInt32      length;
            ret = m_registry.m_pApi->getUnicodeListValue(m_hImpl, keyName.pData,
                                                      &pValueList, &length);
            if ( ret != RegError::NO_ERROR )
            {
                return ret;
            } else
            {
                rValueList.setValueList(m_registry, RegValueType::UNICODELIST,
                                        pValueList, length);
                return ret;
            }
        } else
            return RegError::INVALID_KEY;
    }

inline RegError RegistryKey::getResolvedKeyName(const OUString& keyName,
                                                      OUString& rResolvedName) const
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->getResolvedKeyName(m_hImpl,
                                                         keyName.pData,
                                                         true,
                                                         &rResolvedName.pData);
        else
            return RegError::INVALID_KEY;
    }

inline OUString RegistryKey::getRegistryName()
    {
        if (m_registry.isValid())
        {
            return m_registry.getName();
        } else
            return OUString();
    }


inline Registry::Registry()
    : m_pApi(initRegistry_Api())
    , m_hImpl(nullptr)
    { }

inline Registry::Registry(const Registry& toCopy)
    : m_pApi(toCopy.m_pApi)
    , m_hImpl(toCopy.m_hImpl)
    {
        if (m_hImpl)
            m_pApi->acquire(m_hImpl);
    }


inline Registry::~Registry()
    {
        if (m_hImpl)
            m_pApi->release(m_hImpl);
    }

inline Registry& Registry::operator = (const Registry& toAssign)
{
    if (toAssign.m_hImpl)
        toAssign.m_pApi->acquire(toAssign.m_hImpl);
    if (m_hImpl)
        m_pApi->release(m_hImpl);

    m_pApi  = toAssign.m_pApi;
    m_hImpl = toAssign.m_hImpl;

    return *this;
}

inline bool Registry::isValid() const
    {  return ( m_hImpl != nullptr ); }

inline bool Registry::isReadOnly() const
    {  return m_pApi->isReadOnly(m_hImpl); }

inline RegError Registry::openRootKey(RegistryKey& rRootKey)
    {
        rRootKey.setRegistry(*this);
        return m_pApi->openRootKey(m_hImpl, &rRootKey.m_hImpl);
    }

inline OUString Registry::getName()
    {
        OUString sRet;
        m_pApi->getName(m_hImpl, &sRet.pData);
        return sRet;
    }

inline RegError Registry::create(const OUString& registryName)
    {
        if (m_hImpl)
            m_pApi->release(m_hImpl);
        return m_pApi->createRegistry(registryName.pData, &m_hImpl);
    }

inline RegError Registry::open(const OUString& registryName,
                                  RegAccessMode accessMode)
    {
        if (m_hImpl)
            m_pApi->release(m_hImpl);
        return m_pApi->openRegistry(registryName.pData, &m_hImpl, accessMode);
    }

inline RegError Registry::close()
    {
        RegError ret = m_pApi->closeRegistry(m_hImpl);
        if (ret == RegError::NO_ERROR)
            m_hImpl = nullptr;
        return ret;
    }

inline RegError Registry::destroy(const OUString& registryName)
    {
        RegError ret = m_pApi->destroyRegistry(m_hImpl, registryName.pData);
        if ( ret == RegError::NO_ERROR && registryName.isEmpty() )
            m_hImpl = nullptr;
        return ret;
    }

inline RegError Registry::mergeKey(RegistryKey& rKey,
                                         const OUString& keyName,
                                         const OUString& regFileName,
                                         bool bReport)
    {  return m_pApi->mergeKey(m_hImpl, rKey.m_hImpl, keyName.pData, regFileName.pData, false/*bWarnings*/, bReport); }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
