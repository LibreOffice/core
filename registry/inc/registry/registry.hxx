/*************************************************************************
 *
 *  $RCSfile: registry.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-14 09:37:08 $
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

#ifndef _REGISTRY_REGISTRY_HXX_
#define _REGISTRY_REGISTRY_HXX_

#ifndef _REGISTRY_REGTYPE_H_
#include <registry/regtype.h>
#endif
#ifndef _SALHELPER_DYNLOAD_HXX_
#include <salhelper/dynload.hxx>
#endif

#ifdef __cplusplus
extern "C" {
#endif

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
    RegError    (REGISTRY_CALLTYPE *loadKey)            (RegHandle, RegKeyHandle, rtl_uString*, rtl_uString*);
    RegError    (REGISTRY_CALLTYPE *saveKey)            (RegHandle, RegKeyHandle, rtl_uString*, rtl_uString*);
    RegError    (REGISTRY_CALLTYPE *mergeKey)           (RegHandle, RegKeyHandle, rtl_uString*, rtl_uString*, sal_Bool, sal_Bool);
    RegError    (REGISTRY_CALLTYPE *dumpRegistry)       (RegHandle, RegKeyHandle);
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
    RegError    (REGISTRY_CALLTYPE *setLongListValue)   (RegKeyHandle, rtl_uString*, sal_Int32*, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *setStringListValue) (RegKeyHandle, rtl_uString*, sal_Char**, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *setUnicodeListValue)(RegKeyHandle, rtl_uString*, sal_Unicode**, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *getValueInfo)       (RegKeyHandle, rtl_uString*, RegValueType*, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *getValue)           (RegKeyHandle, rtl_uString*, RegValue);
    RegError    (REGISTRY_CALLTYPE *getLongListValue)   (RegKeyHandle, rtl_uString*, sal_Int32**, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *getStringListValue) (RegKeyHandle, rtl_uString*, sal_Char***, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *getUnicodeListValue)(RegKeyHandle, rtl_uString*, sal_Unicode***, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *freeValueList)      (RegValueType, RegValue, sal_uInt32);
    RegError    (REGISTRY_CALLTYPE *createLink)         (RegKeyHandle, rtl_uString*, rtl_uString*);
    RegError    (REGISTRY_CALLTYPE *deleteLink)         (RegKeyHandle, rtl_uString*);
    RegError    (REGISTRY_CALLTYPE *getKeyType)         (RegKeyHandle, rtl_uString*, RegKeyType*);
    RegError    (REGISTRY_CALLTYPE *getLinkTarget)      (RegKeyHandle, rtl_uString*, rtl_uString**);
    RegError    (REGISTRY_CALLTYPE *getResolvedKeyName) (RegKeyHandle, rtl_uString*, sal_Bool, rtl_uString**);
    RegError    (REGISTRY_CALLTYPE *getKeyNames)        (RegKeyHandle, rtl_uString*, rtl_uString***, sal_uInt32*);
    RegError    (REGISTRY_CALLTYPE *freeKeyNames)       (rtl_uString**, sal_uInt32);
};

typedef Registry_Api* (REGISTRY_CALLTYPE *InitRegistry_Api)(void);

#define REGISTRY_INIT_FUNCTION_NAME "initRegistry_Api"

#ifdef __cplusplus
}
#endif


class RegistryKey;

//-----------------------------------------------------------------------------

/** RegistryLoader load the needed DLL for the registry.
    The loader can be checked if the DLL is loaded. If the DLL is loaded the
    loader provides a valid Api for the registry.
*/
class RegistryLoader : public ::salhelper::ODynamicLoader<Registry_Api>
{
public:
    /// Default constructor, try to load the registry DLL and initialize the needed api.
    RegistryLoader()
        : ::salhelper::ODynamicLoader<Registry_Api>
            (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SAL_MODULENAME( "reg" LIBRARY_VERSION ) ) ),
             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(REGISTRY_INIT_FUNCTION_NAME) ))
        {}

    /// Destructor, decrease the refcount and unload the DLL if the refcount is 0.
    ~RegistryLoader()
        {}
};

/** Registry read and write information in a registry file.
    Class is inline and use a load on call C-Api.
*/
class Registry
{
public:
    /** Constructor.
        This constructor is called with a valid api for the registry.
        The constructor is used if the api is known. Use open() or create()
        to initialize the registry with a valid registry data file.
     */
    inline Registry(const Registry_Api* pApi);

    /** Constructor.
        This constructor is called with a RegisterLoader for the registry.
        The RegistryLoader load the needed DLL and provide the needed api for
        the registry. Use open() or create() to initialize the registry with a
        valid registry data file.
     */
    inline Registry(const RegistryLoader& rLoader);

    /// Copy constructcor
    inline Registry(const Registry& toCopy);

    /// Destructor. The Destructor close the registry if it is open.
    inline ~Registry();

    /// Assign operator
    inline Registry& operator = (const Registry& toAssign);

    /// isValid checks if the registry points to a valid registry data file.
    inline sal_Bool isValid() const;

    /** isReadOnly returns the access mode of the registry.
        @return TRUE if accessmode is readonly else FALSE.
    */
    inline sal_Bool     isReadOnly() const;

    /** openRootKey opens the root key of the registry.
        @param  rRootKey reference on a RegistryKey which is filled with the rootkey.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError openRootKey(RegistryKey& rRootKey);

    /// getName returns the name of the current registry data file.
    inline ::rtl::OUString getName();

    /** This function creates a new registry with the specified name and creates a root key.
        @param  registryName specifies the name of the new registry.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError create(const ::rtl::OUString& registryName);

    /** This function opens a registry with the specified name. If the registry alreday points
        to a valid registry, the old regitry will be closed.
        @param  registryName Identifies a registry name.
        @param  accessMode Specifies the accessmode for the registry, REG_READONLY or REG_READWRITE.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError open(const ::rtl::OUString& registryName,
                            RegAccessMode accessMode);

    /// close explicitly the current registry data file
    inline RegError close();

    /** This function destroy the specified registry.
        @param registryName Identifies a registry name, if the name is an empty string the registry
               itselfs will be destroyed.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError destroy(const ::rtl::OUString& registryName);

    /** This function loads registry information from a specified file and save it under the
        specified keyName.
        @param  rKey    Specifies the key where the subkey will be created.
        @param  keyName specifies the name of the subkey.
                        If keyName is an empty string the registry information is saved under the key
                        specified by rKey.
        @param  regFileName specifies the file containing the registry information.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError loadKey(RegistryKey& rKey,
                               const ::rtl::OUString& keyName,
                               const ::rtl::OUString& regFileName);

    /** This function saves the registry information from the specified key and all subkeys and save
        it in the specified file.
        @param  rKey    Identifies a currently open key. The key which inforamtion is saved by this
                        function is a subkey of the key identified by hKey.
        @param  keyName specifies the name of the subkey.
                        If keyName is an empty string the registry information under the key specified
                        by rKey is saved in the specified file.
        @param  regFileName specifies the file containing the registry information.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError saveKey(RegistryKey& rKey,
                               const ::rtl::OUString& keyName,
                               const ::rtl::OUString& regFileName);

    /** This function merges the registry information from the specified key and the registry
        information of the specified file. Existing keys will extended.
        @param  rKey    Identifies a currently open key. The key which inforamtion is merged by this
                        function is a subkey of the key identified by hKey.
        @param  keyName specifies the name of the subkey.
                        If keyName is an empty string the registry information under the key specified
                        by rKey is merged with the information from the specified file.
        @param  regFileName specifies the file containing the registry information.
        @param  bWarnings if TRUE the function returns an error if a key already exists.
        @param  bReport if TRUE the function reports warnings on stdout if a key already exists.
        @return REG_NO_ERROR if succeeds else an error code. If returns an error the registry will
                restore the state before merging.
    */
    inline RegError mergeKey(RegistryKey& rKey,
                                const ::rtl::OUString& keyName,
                                const ::rtl::OUString& regFileName,
                             sal_Bool bWarnings = sal_False,
                                sal_Bool bReport = sal_False);

    /** This function dump the content under the open key to stdout
        @param  rKey    Identifies a currently open key which subtree is dumped.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError dumpRegistry(RegistryKey& rKey);

    friend class RegistryKey;
    friend class RegistryKeyArray;
    friend class RegistryKeyNames;

    const Registry_Api* getApi() { return m_pApi; }
protected:

    const Registry_Api*                          m_pApi;
    ::salhelper::ODynamicLoader< Registry_Api >  m_Api;
    RegHandle                                    m_hImpl;
};


//-----------------------------------------------------------------------------

/** RegistryKeyArray presents an array of open keys.
    RegistryKeyArray is a helper class to work with an array of subkeys.
*/
class RegistryKeyArray
{
public:
    /// Default constructor
    inline RegistryKeyArray();

    /// Destructor
    inline ~RegistryKeyArray();

    /// This function returns the open key sepecified with index.
    inline RegistryKey getElement(sal_uInt32 index);

    /// This function returns the length of the array.
    inline sal_uInt32 getLength();

    friend class RegistryKey;
protected:
    inline void setKeyHandles(Registry& registry, RegKeyHandle* phKeys, sal_uInt32 length);
    inline RegError closeKeyHandles();

    sal_uInt32      m_length;
    RegKeyHandle*   m_phKeys;
    Registry        m_registry;
};


/** RegistryKeyNames presents an array of subkey names.
    RegistryKeyNames is a helper class to work with an array of subkeys.
*/
class RegistryKeyNames
{
public:
    /// Default constructor
    inline RegistryKeyNames();

    /// Destructor
    inline ~RegistryKeyNames();

    /// This function returns the name of the key sepecified with index.
    inline ::rtl::OUString getElement(sal_uInt32 index);

    /// This function returns the length of the array.
    inline sal_uInt32 getLength();

    friend class RegistryKey;
protected:
    inline void setKeyNames(Registry& registry, rtl_uString** pKeyNames, sal_uInt32 length);
    inline RegError freeKeyNames();

    sal_uInt32      m_length;
    rtl_uString**   m_pKeyNames;
    Registry        m_registry;
};

//-----------------------------------------------------------------------------

/** RegistryValueList presents an value list of the specified type.
    RegistryValueList is a helper class to work with a list value.
*/
template<class ValueType>
class RegistryValueList
{
public:
    /// Default constructor
    RegistryValueList()
        : m_length(0)
        , m_pValueList(NULL)
        , m_registry(NULL)
        {}

    /// Destructor
    ~RegistryValueList()
    {
        if (m_pValueList)
        {
            m_registry.getApi()->freeValueList(m_valueType, m_pValueList, m_length);
        }
    }

    /// This function returns the value of the list with the sepecified index.
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

    /// This function returns the length of the list.
    sal_uInt32 getLength()
    {
        return m_length;
    }

    friend class RegistryKey;
protected:
    void setValueList(Registry& registry, RegValueType valueType,
                      ValueType* pValueList, sal_uInt32 length)
    {
        m_length = length;
        m_pValueList = pValueList;
        m_valueType = valueType;
        m_registry = registry;
    }

    sal_uInt32      m_length;
    ValueType*      m_pValueList;
    RegValueType    m_valueType;
    Registry        m_registry;
};

//-----------------------------------------------------------------------------

/** RegistryKey read and write information for the specified key in a registry.
    Class is inline and use a load on call C-Api.
*/
class RegistryKey
{
public:
    /// Default constructor
    inline RegistryKey();

    /// Copy constructor
    inline RegistryKey(const RegistryKey& toCopy);

    /// Destructor. The Destructor close the key if it is open.
    inline ~RegistryKey();

    /// Assign operator
    inline RegistryKey& operator = (const RegistryKey& toAssign);

    /// isValid checks if the key points to a valid registry key.
    inline sal_Bool isValid() const;

    /** isReadOnly returns the access mode of the key.
        @return TRUE if accessmode is read only else FALSE.
    */
    inline sal_Bool     isReadOnly() const;

    /// getName returns the full name of the key beginning with the rootkey.
    inline ::rtl::OUString getName();

    /** This function creates the specified key. If the key already exists in the registry,
        the function opens it.
        @param  keyName specifies the name of the key which will be opened or created.
        @param  rNewKey Reference a RegistryKey which will be filled with the new or open key.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError createKey(const ::rtl::OUString& keyName,
                              RegistryKey& rNewKey);

    /** This function opens the specified key.
        @param  keyName Points to a null terminated string specifying the name of a key that this
                        function opens.
        @param  rOpenKey Reference a RegistryKey which will be filled with the open key.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError openKey(const ::rtl::OUString& keyName,
                              RegistryKey& rOpenKey);

    /** This function returns the names of all subkeys of the key.
        @param  keyName Points to a null terminated string specifying the name of a key.
        @param  rSubKeys Reference a RegistryKeyArray which will be filled with the names of the subkeys.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError openSubKeys(const ::rtl::OUString& keyName,
                                    RegistryKeyArray& rSubKeys);

    /** This function opens all subkeys of the key.
        @param  keyName Points to a null terminated string specifying the name of a key that subkeys
                        this function opens.
        @param  rSubKeyNames Reference a RegistryKeyNames array which will be filled with the open subkeys.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError getKeyNames(const ::rtl::OUString& keyName,
                                    RegistryKeyNames& rSubKeyNames);

    /** This function close all subkeys specified in the array.
        @param  rSubKeys Reference a RegistryKeyArray which contains the open subkeys.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError closeSubKeys(RegistryKeyArray& rSubKeys);

    /** This function deletes the specified key.
        @param  keyName specifies the name of the key which will be deleted.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError deleteKey(const ::rtl::OUString& keyName);

    /// close explicitly the current key
    inline RegError closeKey();

    /** This function sets an value under the  key.
        @param  keyName specifies the name of the key which value is set by this function.
                        If keyName is an empty string, the value is set for the key
                        specified by hKey.
        @param  valueType   Specified the type of the value.
        @param  pData   Points to a memory block containing the current data for the value.
        @param  valueSize   Specified the size of pData in bytes
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError setValue(const ::rtl::OUString& keyName,
                                RegValueType valueType,
                                RegValue pValue,
                                sal_uInt32 valueSize);

    /** This function sets an long list value under the  key.
        @param  keyName specifies the name of the key which value is set by this function.
                        If keyName is an empty string, the value is set for the key
                        specified by hKey.
        @param  pValueList  Points to an array of longs containing the current data for the value.
        @param  len Specified the len of pValueList.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError setLongListValue(const ::rtl::OUString& keyName,
                                         sal_Int32* pValueList,
                                         sal_uInt32 len);

    /** This function sets an ascii list value under the  key.
        @param  keyName specifies the name of the key which value is set by this function.
                        If keyName is an empty string, the value is set for the key
                        specified by hKey.
        @param  pValueList  Points to an array of sal_Char* containing the current data for the value.
        @param  len Specified the len of pValueList.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError setStringListValue(const ::rtl::OUString& keyName,
                                           sal_Char** pValueList,
                                           sal_uInt32 len);

    /** This function sets an unicode string list value under the  key.
        @param  keyName specifies the name of the key which value is set by this function.
                        If keyName is an empty string, the value is set for the key
                        specified by hKey.
        @param  pValueList  Points to an array of sal_Unicode* containing the current data for the value.
        @param  len Specified the len of pValueList.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError setUnicodeListValue(const ::rtl::OUString& keyName,
                                            sal_Unicode** pValueList,
                                              sal_uInt32 len);

    /** This function gets info about type and size of the value.
        @param  keyName specifies the name of the key which value is set by this function.
                        If keyName is an empty string, the value is set for the key
                        specified by hKey.
        @param  pValueType  Specified the type of the value.
        @param  pValueSize  Specified the size of pData in bytes or the length of a list value.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError getValueInfo(const ::rtl::OUString& keyName,
                                    RegValueType* pValueType,
                                    sal_uInt32* pValueSize);

    /** This function gets the value under the specified key.
        @param  keyName specifies the name of the key which value is get by this function.
                        If keyName is an empty string, the value is get from the key
                        specified by hKey.
        @param  pValue  Points to an allocated  memory block receiving the current data for the value.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError getValue(const ::rtl::OUString& keyName,
                                RegValue pValue);

    /** This function gets the value under the specified key.
        @param  keyName specifies the name of the key which value is get by this function.
                        If keyName is an empty string, the value is get from the key
                        specified by hKey.
        @param  rValueList  Reference a RegistryValueList which will be filled with the ascii values.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError getLongListValue(const ::rtl::OUString& keyName,
                                          RegistryValueList<sal_Int32>& rValueList);

    /** This function gets the value under the specified key.
        @param  keyName specifies the name of the key which value is get by this function.
                        If keyName is an empty string, the value is get from the key
                        specified by hKey.
        @param  rValueList  Reference a RegistryValueList which will be filled with the ascii values.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError getStringListValue(const ::rtl::OUString& keyName,
                                           RegistryValueList<sal_Char*>& rValueList);

    /** This function gets the value under the specified key.
        @param  keyName specifies the name of the key which value is get by this function.
                        If keyName is an empty string, the value is get from the key
                        specified by hKey.
        @param  rValueList  Reference a RegistryValueList which will be filled with the unicode values.
        @return REG_NO_ERROR if succeeds else an error code.
    */
    inline RegError getUnicodeListValue(const ::rtl::OUString& keyName,
                                              RegistryValueList<sal_Unicode*>& rValueList);


    inline RegError createLink(const ::rtl::OUString& linkName,
                               const ::rtl::OUString& linkTarget);

    inline RegError deleteLink(const ::rtl::OUString& linkName);

    inline RegError getKeyType(const ::rtl::OUString& name,
                                  RegKeyType* pKeyType) const;

    inline RegError getLinkTarget(const ::rtl::OUString& linkName,
                                    ::rtl::OUString& rLinkTarget) const;

    /** This function will resolved all or only the first link of a keyname
        and returns the resolved keyName in rResolvedName.
        @param  keyName specifies the relativ name of the key which name will be resolved.
        @return REG_NO_ERROR if succeeds else an error code.
     */
    inline RegError getResolvedKeyName(const ::rtl::OUString& keyName,
                                       sal_Bool firstLinkOnly,
                                           ::rtl::OUString& rResolvedName) const;

    /** getRegistryName returns the name of the current registry in which
           the key is defined.
     */
    inline ::rtl::OUString getRegistryName();

    /// getRegistry returns the registry in which the key is defined.
    Registry getRegistry() const { return m_registry; }

    friend class Registry;
public:
    inline RegistryKey(Registry&    registry,
                       RegKeyHandle hKey);

    RegKeyHandle getKeyHandle() const { return m_hImpl; }

protected:
    inline void setRegistry(Registry& registry);

    Registry        m_registry;
    RegKeyHandle    m_hImpl;
};


//-----------------------------------------------------------------------------

inline RegistryKeyArray::RegistryKeyArray()
    : m_length(0)
    , m_phKeys(NULL)
    , m_registry(NULL)
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

inline sal_uInt32 RegistryKeyArray::getLength()
{
    return m_length;
}

inline void RegistryKeyArray::setKeyHandles(Registry& registry,
                                            RegKeyHandle* phKeys,
                                            sal_uInt32 length)
{
    m_phKeys = phKeys;
    m_length = length;
    m_registry = registry;
}

inline RegError RegistryKeyArray::closeKeyHandles()
{
    if (m_registry.isValid() && m_phKeys)
    {
        RegError ret;
        ret = m_registry.m_pApi->closeSubKeys(m_phKeys, m_length);
        m_registry = Registry(m_registry.m_pApi);
        m_length = 0;
        m_phKeys = NULL;
        return ret;
    } else
        return(REG_INVALID_KEY);
}

//-----------------------------------------------------------------------------

inline RegistryKeyNames::RegistryKeyNames()
    : m_length(0)
    , m_pKeyNames(NULL)
    , m_registry(NULL)
{
}

inline RegistryKeyNames::~RegistryKeyNames()
{
    if (m_pKeyNames)
        m_registry.m_pApi->freeKeyNames(m_pKeyNames, m_length);
}

inline ::rtl::OUString RegistryKeyNames::getElement(sal_uInt32 index)
{

    if (m_pKeyNames && index < m_length)
        return m_pKeyNames[index];
    else
        return ::rtl::OUString();
}

inline sal_uInt32 RegistryKeyNames::getLength()
{
    return m_length;
}

inline void RegistryKeyNames::setKeyNames(Registry& registry,
                                          rtl_uString** pKeyNames,
                                          sal_uInt32 length)
{
    m_pKeyNames = pKeyNames;
    m_length = length;
    m_registry = registry;
}

inline RegError RegistryKeyNames::freeKeyNames()
{
    if (m_registry.isValid() && m_pKeyNames)
    {
        RegError ret = REG_NO_ERROR;
        ret = m_registry.m_pApi->freeKeyNames(m_pKeyNames, m_length);
        m_registry = Registry(m_registry.m_pApi);
        m_length = 0;
        m_pKeyNames = NULL;
        return ret;
    } else
        return REG_INVALID_KEY;
}

//-----------------------------------------------------------------------------

inline RegistryKey::RegistryKey()
    : m_registry(NULL)
    , m_hImpl(NULL)
    { }

inline RegistryKey::RegistryKey(Registry& registry, RegKeyHandle hKey)
    : m_registry(registry)
    , m_hImpl(hKey)
    {
        if (m_hImpl)
            m_registry.m_pApi->acquireKey(m_hImpl);
    }

inline RegistryKey::RegistryKey(const RegistryKey& toCopy)
    : m_registry(toCopy.m_registry)
    , m_hImpl(toCopy.m_hImpl)
    {
        if (m_hImpl)
            m_registry.m_pApi->acquireKey(m_hImpl);
    }

inline void RegistryKey::setRegistry(Registry& registry)
    {
        m_registry = registry;
    }

inline RegistryKey::~RegistryKey()
    {
        if (m_hImpl)
            m_registry.m_pApi->closeKey(m_hImpl);
    }

inline RegistryKey& RegistryKey::operator = (const RegistryKey& toAssign)
{
    m_registry = toAssign.m_registry;

    if (m_hImpl != toAssign.m_hImpl)
    {
        m_registry.m_pApi->releaseKey(m_hImpl);
        m_hImpl = toAssign.m_hImpl;
        m_registry.m_pApi->acquireKey(m_hImpl);
    }

    return *this;
}

inline sal_Bool RegistryKey::isValid() const
    {  return (m_hImpl != NULL); }

inline sal_Bool RegistryKey::isReadOnly() const
    {
        if  (m_registry.isValid())
            return (m_registry.m_pApi)->isKeyReadOnly(m_hImpl);
        else
            return sal_False;
    }

inline ::rtl::OUString RegistryKey::getName()
    {
        ::rtl::OUString sRet;
        if (m_registry.isValid())
            m_registry.m_pApi->getKeyName(m_hImpl, &sRet.pData);
        return sRet;;
    }

inline RegError RegistryKey::createKey(const ::rtl::OUString& keyName,
                                          RegistryKey& rNewKey)
    {
        if (rNewKey.isValid()) rNewKey.closeKey();
        if (m_registry.isValid())
        {
            RegError ret = m_registry.m_pApi->createKey(m_hImpl, keyName.pData, &rNewKey.m_hImpl);
            if (!ret) rNewKey.setRegistry(m_registry);
            return ret;
        } else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::openKey(const ::rtl::OUString& keyName,
                                     RegistryKey& rOpenKey)
    {
        if (rOpenKey.isValid()) rOpenKey.closeKey();
        if (m_registry.isValid())
        {
            RegError ret = m_registry.m_pApi->openKey(m_hImpl, keyName.pData,
                                                    &rOpenKey.m_hImpl);
            if (!ret) rOpenKey.setRegistry(m_registry);
            return ret;
        } else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::openSubKeys(const ::rtl::OUString& keyName,
                                         RegistryKeyArray& rSubKeys)
    {
        if (m_registry.isValid())
        {
            RegError        ret = REG_NO_ERROR;
            RegKeyHandle*   pSubKeys;
            sal_uInt32      nSubKeys;
             if (ret = m_registry.m_pApi->openSubKeys(m_hImpl, keyName.pData,
                                                      &pSubKeys, &nSubKeys))
            {
                return ret;
            } else
            {
                rSubKeys.setKeyHandles(m_registry, pSubKeys, nSubKeys);
                return ret;
            }
        } else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::getKeyNames(const ::rtl::OUString& keyName,
                                             RegistryKeyNames& rSubKeyNames)
    {
        if (m_registry.isValid())
        {
            RegError        ret = REG_NO_ERROR;
            rtl_uString**   pSubKeyNames;
            sal_uInt32      nSubKeys;
             if (ret = m_registry.m_pApi->getKeyNames(m_hImpl, keyName.pData,
                                                      &pSubKeyNames, &nSubKeys))
            {
                return ret;
            } else
            {
                rSubKeyNames.setKeyNames(m_registry, pSubKeyNames, nSubKeys);
                return ret;
            }
        } else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::closeSubKeys(RegistryKeyArray& rSubKeys)
    {
        if (m_registry.isValid())
            return rSubKeys.closeKeyHandles();
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::deleteKey(const ::rtl::OUString& keyName)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->deleteKey(m_hImpl, keyName.pData);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::closeKey()
    {
        if (m_registry.isValid())
        {
            RegError ret = m_registry.m_pApi->closeKey(m_hImpl);
            if (!ret)
            {
                m_hImpl = NULL;
                m_registry = Registry(m_registry.m_pApi);
            }
            return ret;
        } else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::setValue(const ::rtl::OUString& keyName,
                                              RegValueType valueType,
                                           RegValue pValue,
                                              sal_uInt32 valueSize)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->setValue(m_hImpl, keyName.pData, valueType,
                                                pValue, valueSize);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::setLongListValue(const ::rtl::OUString& keyName,
                                                  sal_Int32* pValueList,
                                                  sal_uInt32 len)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->setLongListValue(m_hImpl, keyName.pData,
                                                           pValueList, len);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::setStringListValue(const ::rtl::OUString& keyName,
                                                   sal_Char** pValueList,
                                                   sal_uInt32 len)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->setStringListValue(m_hImpl, keyName.pData,
                                                            pValueList, len);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::setUnicodeListValue(const ::rtl::OUString& keyName,
                                                        sal_Unicode** pValueList,
                                                        sal_uInt32 len)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->setUnicodeListValue(m_hImpl, keyName.pData,
                                                              pValueList, len);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::getValueInfo(const ::rtl::OUString& keyName,
                                                  RegValueType* pValueType,
                                                  sal_uInt32* pValueSize)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->getValueInfo(m_hImpl, keyName.pData, pValueType, pValueSize);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::getValue(const ::rtl::OUString& keyName,
                                        RegValue pValue)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->getValue(m_hImpl, keyName.pData, pValue);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::getLongListValue(const ::rtl::OUString& keyName,
                                              RegistryValueList<sal_Int32>& rValueList)
    {
        if (m_registry.isValid())
        {
            RegError    ret = REG_NO_ERROR;
            sal_Int32*  pValueList;
            sal_uInt32  length;
             if (ret = m_registry.m_pApi->getLongListValue(m_hImpl, keyName.pData,
                                                      &pValueList, &length))
            {
                return ret;
            } else
            {
                rValueList.setValueList(m_registry, RG_VALUETYPE_LONGLIST,
                                        pValueList, length);
                return ret;
            }
        } else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::getStringListValue(const ::rtl::OUString& keyName,
                                                      RegistryValueList<sal_Char*>& rValueList)
    {
        if (m_registry.isValid())
        {
            RegError    ret = REG_NO_ERROR;
            sal_Char**  pValueList;
            sal_uInt32  length;
             if (ret = m_registry.m_pApi->getStringListValue(m_hImpl, keyName.pData,
                                                      &pValueList, &length))
            {
                return ret;
            } else
            {
                rValueList.setValueList(m_registry, RG_VALUETYPE_STRINGLIST,
                                        pValueList, length);
                return ret;
            }
        } else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::getUnicodeListValue(const ::rtl::OUString& keyName,
                                              RegistryValueList<sal_Unicode*>& rValueList)
    {
        if (m_registry.isValid())
        {
            RegError        ret = REG_NO_ERROR;
            sal_Unicode**   pValueList;
            sal_uInt32      length;
             if (ret = m_registry.m_pApi->getUnicodeListValue(m_hImpl, keyName.pData,
                                                      &pValueList, &length))
            {
                return ret;
            } else
            {
                rValueList.setValueList(m_registry, RG_VALUETYPE_UNICODELIST,
                                        pValueList, length);
                return ret;
            }
        } else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::createLink(const ::rtl::OUString& linkName,
                                           const ::rtl::OUString& linkTarget)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->createLink(m_hImpl, linkName.pData, linkTarget.pData);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::deleteLink(const ::rtl::OUString& linkName)
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->deleteLink(m_hImpl, linkName.pData);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::getKeyType(const ::rtl::OUString& keyName,
                                              RegKeyType* pKeyType) const
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->getKeyType(m_hImpl, keyName.pData, pKeyType);
        else
            return REG_INVALID_KEY;
    }

inline RegError RegistryKey::getLinkTarget(const ::rtl::OUString& linkName,
                                               ::rtl::OUString& rLinkTarget) const
    {
        if (m_registry.isValid())
        {
            return m_registry.m_pApi->getLinkTarget(m_hImpl,
                                                    linkName.pData,
                                                    &rLinkTarget.pData);
        } else
            return REG_INVALID_KEY;
    }


inline RegError RegistryKey::getResolvedKeyName(const ::rtl::OUString& keyName,
                                                   sal_Bool firstLinkOnly,
                                                       ::rtl::OUString& rResolvedName) const
    {
        if (m_registry.isValid())
            return m_registry.m_pApi->getResolvedKeyName(m_hImpl,
                                                         keyName.pData,
                                                         firstLinkOnly,
                                                         &rResolvedName.pData);
        else
            return REG_INVALID_KEY;
    }

inline ::rtl::OUString RegistryKey::getRegistryName()
    {
        if (m_registry.isValid())
        {
            return m_registry.getName();
        } else
            return ::rtl::OUString();
    }

//-----------------------------------------------------------------------------

inline Registry::Registry(const Registry_Api* pApi)
    : m_pApi(pApi)
    , m_Api()
    , m_hImpl(NULL)
    { }

inline Registry::Registry(const RegistryLoader& rLoader)
    : m_pApi(NULL)
    , m_Api(rLoader)
    , m_hImpl(NULL)
    {
        m_pApi = m_Api.getApi();
    }

inline Registry::Registry(const Registry& toCopy)
    : m_pApi(toCopy.m_pApi)
    , m_Api(toCopy.m_Api)
    , m_hImpl(toCopy.m_hImpl)
    {
        if (m_hImpl)
            m_pApi->acquire(m_hImpl);
    }


inline Registry::~Registry()
    {
        if (m_pApi && m_hImpl)
            m_pApi->release(m_hImpl);
    }

inline Registry& Registry::operator = (const Registry& toAssign)
{

    if (m_hImpl != toAssign.m_hImpl)
    {
        if (m_pApi) m_pApi->release(m_hImpl);
        m_pApi = toAssign.m_pApi;
        m_Api = toAssign.m_Api;
        m_hImpl = toAssign.m_hImpl;
    }
    if (m_hImpl && m_pApi)
        m_pApi->acquire(m_hImpl);

    return *this;
}

inline sal_Bool Registry::isValid() const
    {  return ( m_hImpl != NULL && m_pApi != NULL ); }

inline sal_Bool Registry::isReadOnly() const
    {  return m_pApi->isReadOnly(m_hImpl); }

inline RegError Registry::openRootKey(RegistryKey& rRootKey)
    {
        rRootKey.setRegistry(*this);
        return m_pApi->openRootKey(m_hImpl, &rRootKey.m_hImpl);
    }

inline ::rtl::OUString Registry::getName()
    {
        ::rtl::OUString sRet;
        m_pApi->getName(m_hImpl, &sRet.pData);
        return sRet;
    }

inline RegError Registry::create(const ::rtl::OUString& registryName)
    {
        if (m_hImpl)
            m_pApi->release(m_hImpl);
        return m_pApi->createRegistry(registryName.pData, &m_hImpl);
    }

inline RegError Registry::open(const ::rtl::OUString& registryName,
                                  RegAccessMode accessMode)
    {
        if (m_hImpl)
            m_pApi->release(m_hImpl);
        return m_pApi->openRegistry(registryName.pData, &m_hImpl, accessMode);
    }

inline RegError Registry::close()
    {
        RegError ret = m_pApi->closeRegistry(m_hImpl);
        if (!ret)
            m_hImpl = NULL;
        return ret;
    }

inline RegError Registry::destroy(const ::rtl::OUString& registryName)
    {
        RegError ret = m_pApi->destroyRegistry(m_hImpl, registryName.pData);
        if ( !ret && (registryName.getLength() == 0) )
            m_hImpl = NULL;
        return ret;
    }

inline RegError Registry::loadKey(RegistryKey& rKey,
                                      const ::rtl::OUString& keyName,
                                      const ::rtl::OUString& regFileName)
    {  return m_pApi->loadKey(m_hImpl, rKey.m_hImpl, keyName.pData, regFileName.pData); }

inline RegError Registry::saveKey(RegistryKey& rKey,
                                     const ::rtl::OUString& keyName,
                                     const ::rtl::OUString& regFileName)
    {  return m_pApi->saveKey(m_hImpl, rKey.m_hImpl, keyName.pData, regFileName.pData); }

inline RegError Registry::mergeKey(RegistryKey& rKey,
                                         const ::rtl::OUString& keyName,
                                         const ::rtl::OUString& regFileName,
                                         sal_Bool bWarnings,
                                         sal_Bool bReport)
    {  return m_pApi->mergeKey(m_hImpl, rKey.m_hImpl, keyName.pData, regFileName.pData, bWarnings, bReport); }

inline RegError Registry::dumpRegistry(RegistryKey& rKey)
    {  return m_pApi->dumpRegistry(m_hImpl, rKey.m_hImpl); }


#endif
