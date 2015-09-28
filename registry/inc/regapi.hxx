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

#ifndef INCLUDED_REGISTRY_INC_REGAPI_HXX
#define INCLUDED_REGISTRY_INC_REGAPI_HXX

#include <stddef.h>
#include <rtl/ustring.h>
#include <registry/regtype.h>
#include <registry/regdllapi.h>

/** This function creates the specified key.

    If the key already exists in the registry, the function opens the key only.
    @param  hKey identifies a currently open key. The key which will be opened or created by this
                 function is a subkey of the key identified by hKey.
    @param  keyName points to a null terminated string specifying the name of a key.
    @param  phNewKey points to a variable that receives the handle of the opened or created key.
                     The memory to store this variable will be allocated and will be freed by the function
                     reg_closeKey. If the function fails, phNewKey is NULL.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_createKey(RegKeyHandle hKey,
                                         rtl_uString* keyName,
                                         RegKeyHandle* phNewKey);


/** This function opens the specified key.

    @param  hKey identifies a currently open key. The key which will be opened by this function
                 is a subkey of the key identified by hKey
    @param  keyName points to a null terminated string specifying the name of a key.
    @param  phOpenKey points to a variable that receives the handle of the opened key.
                     The memory to store this variable will be allocated and will be freed by the function
                     reg_closeKey. If the function fails, phNewKey is NULL.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_openKey(RegKeyHandle hKey,
                                         rtl_uString* keyName,
                                         RegKeyHandle* phOpenKey);



/** This function opens all subkeys of the specified key.

    @param  hKey identifies a currently open key. The key that subkeys will be opened by this
                 function is a subkey of the key identified by hKey
    @param  keyName points to a null terminated string specifying the name of a key whose subkeys
                    will be opened.
    @param  pphSubKeys points to a variable that receives an array of all opened subkeys.
                       The memory to store this variable will be allocated and will be freed by the function
                       reg_closeSubKeys. If the function fails, pphSubKeys is NULL.
    @param  pnSubKeys specifies the length of the array (the number of open subkeys).
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_openSubKeys(RegKeyHandle hKey,
                                             rtl_uString* keyName,
                                             RegKeyHandle** pphSubKeys,
                                             sal_uInt32* pnSubKeys);


/** This function closes all subkeys specified in the array.

    @param  phSubKeys points to a variable that containss an array of all opened subkeys.
                      The allocated memory of pphSubKeys and all open subkeys will be freed.
    @param  nSubKeys specifies the length of the array (the number of subkeys to closed).
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_closeSubKeys(RegKeyHandle* phSubKeys,
                                               sal_uInt32 nSubKeys);


/** This function deletes the specified key.

    @param  hKey identifies a currently open key. The key deleted by this function
                 is a subkey of the key identified by hKey
    @param  keyName points to a null terminated string specifying the name of a key which will
                    be deleted.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_deleteKey(RegKeyHandle hKey,
                                         rtl_uString* keyName);


/** This function closes the specified key.

    @param  hKey identifies a currently open key which will be closed by this function.
                 The memory of the variable specifying the key will be freed.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_closeKey(RegKeyHandle hKey);


/** This function returns the name of a key.

    @param  hKey identifies a currently open key which name will be returned.
    @param  pKeyName contains the keyname if succeeds else an empty string.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_getKeyName(RegKeyHandle hKey, rtl_uString** pKeyName);


/** This function sets a value of a key.

    @param  hKey identifies a currently open key. The key which value will be set by this
                 function is a subkey of the key identified by hKey.
    @param  keyName points to a null terminated string specifying the name of a key which value
                    will be set. If keyName is NULL, then the value of the key specified by
                    hKey will be set.
    @param  valueType specifies the type of the value.
    @param  pData points to a memory block containing the data of the value.
    @param  valueSize specifies the size of pData in bytes
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_setValue(RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           RegValueType valueType,
                                           RegValue pData,
                                           sal_uInt32 valueSize);


/** This function sets an long list value of a key.

    @param[in]  hKey identifies a currently open key. The key which value will be set by this
                 function is a subkey of the key identified by hKey.
    @param[in]  keyName points to a null terminated string specifying the name of a key which value
                    will be set. If keyName is NULL, then the value of the key specified by
                    hKey will be set.
    @param[out]  pValueList points to an array of longs containing the data of the value.
    @param[out]  len specifies the len of pValueList.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_setLongListValue(RegKeyHandle hKey,
                                                      rtl_uString* keyName,
                                                      sal_Int32* pValueList,
                                                      sal_uInt32 len);


/** This function sets an ascii list value of a key.

    @param[in]  hKey identifies a currently open key. The key which value will be set by this
                 function is a subkey of the key identified by hKey.
    @param[in]  keyName points to a null terminated string specifying the name of a key which value
                    will be set. If keyName is NULL, then the value of the key specified by
                    hKey will be set.
    @param[in]  pValueList points to an array of sal_Char* containing the data of the value.
    @param[in]  len specifies the len of pValueList.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_setStringListValue(RegKeyHandle hKey,
                                                          rtl_uString* keyName,
                                                          sal_Char** pValueList,
                                                          sal_uInt32 len);


/** This function sets an unicode string list value of a key.

    @param[in]  hKey identifies a currently open key. The key which value will be set by this
                 function is a subkey of the key identified by hKey.
    @param[in]  keyName points to a null terminated string specifying the name of a key which value
                    will be set. If keyName is NULL, then the value of the key specified by
                    hKey will be set.
    @param[in]  pValueList points to an array of sal_Unicode* containing the data of the value.
    @param[in]  len specifies the len of pValueList.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_setUnicodeListValue(RegKeyHandle hKey,
                                                            rtl_uString* keyName,
                                                            sal_Unicode** pValueList,
                                                            sal_uInt32 len);


/** This function gets info about type and size of a key value.

    @param  hKey identifies a currently open key. The key which value info will be got by this
                 function is a subkey of the key identified by hKey.
    @param  keyName points to a null terminated string specifying the name of a key which value
                    will be got. If keyName is NULL, then the value info of the key specified by
                    hKey will be got.
    @param  pValueType returns the type of the value.
    @param  pValueSize returns the size of the value in bytes
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_getValueInfo(RegKeyHandle hKey,
                                               rtl_uString* keyName,
                                               RegValueType* pValueType,
                                               sal_uInt32* pValueSize);


/** This function gets the value of a key.

    @param  hKey identifies a currently open key. The key which value will be got by this
                 function is a subkey of the key identified by hKey.
    @param  keyName points to a null terminated string specifying the name of a key which value
                    will be got. If keyName is NULL, then the value of the key specified by
                    hKey will be got.
    @param  pData points to an allocated memory block receiving the data of the value.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_getValue(RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           RegValue pData);


/** This function gets the long list value of a key.

    @param[in]  hKey identifies a currently open key. The key which value will be got by this
                 function is a subkey of the key identified by hKey.
    @param[in]  keyName points to a null terminated string specifying the name of a key which value
                    will be got. If keyName is NULL, then the value of the key specified by
                    hKey will be got.
    @param[out]  pValueList a Pointer to a long value list which returns the data of the value.
    @param[out]  pLen returns the length of the value list.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_getLongListValue(RegKeyHandle hKey,
                                                      rtl_uString* keyName,
                                                      sal_Int32** pValueList,
                                                      sal_uInt32* pLen);


/** This function gets the string list value of a key.

    @param[in]  hKey identifies a currently open key. The key whose value will be retrieved by this
                 function is a subkey of the key identified by hKey.
    @param[in]  keyName points to a null terminated string specifying the name of a key whose value
                    will be retrieved. If keyName is NULL, then the value of the key specified by
                    hKey will be retrieved.
    @param[out]  pValueList a Pointer to an ascii value list which returns the data of the value.
    @param[out]  pLen returns the length of the value list.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_getStringListValue(RegKeyHandle hKey,
                                                       rtl_uString* keyName,
                                                       sal_Char*** pValueList,
                                                       sal_uInt32* pLen);


/** This function gets the unicode list value of a key.

    @param[in]  hKey identifies a currently open key. The key whose value will be retrieved by this
                 function is a subkey of the key identified by hKey.
    @param[in]  keyName points to a null terminated string specifying the name of a key whose value
                    will be retrieved. If keyName is NULL, then the value of the key specified by
                    hKey will be retrieved.
    @param[out]  pValueList a Pointer to an unicode value list which returns the data of the value.
    @param[out]  pLen returns the length of the value list.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_getUnicodeListValue(RegKeyHandle hKey,
                                                         rtl_uString* keyName,
                                                         sal_Unicode*** pValueList,
                                                         sal_uInt32* pLen);


/** This function frees the memory of a value list.

    @param  valueType specifies the type of the list values.
    @param  pValueList a Pointer to the value list.
    @param  len specifies the length of the value list.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_freeValueList(RegValueType valueType,
                                                  RegValue pValueList,
                                                  sal_uInt32 len);

/** This function resolves a keyname.

    @param[in]  hKey identifies a currently open key. The key specified by keyName is a subkey
                 of the key identified by hKey.
    @param[in]  keyName points to a null terminated string specifying the relativ name of a key.
                    The name of hKey together with keyName will be generated.
    @param[in] firstLinkOnly ignored
    @param[out]  pResolvedName returns the resolved keyName.
    @return REG_NO_ERROR if succeeds else an error code.
 */
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_getResolvedKeyName(RegKeyHandle hKey,
                                                    rtl_uString* keyName,
                                                   bool firstLinkOnly,
                                                      rtl_uString** pResolvedName);

/** This function loads registry information from a file and save it under the
    specified keyName.

    @param  hKey identifies a currently open key. The key which should store the registry information
                 is a subkey of this key.
    @param  keyName points to a null terminated string specifying the name of the key which stores the
                    registry information. If keyName is NULL the registry information will be saved under
                    the key specified by hKey.
    @param  regFileName points to a null terminated string specifying the file which conains the
                        registry information.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_loadKey(RegKeyHandle hKey,
                                         rtl_uString* keyName,
                                         rtl_uString* regFileName);


/** This function saves the registry information under a specified key and all of its subkeys and save
    it in a registry file.

    @param  hKey identifies a currently open key. The key which information is saved by this
                 function is a subkey of the key identified by hKey.
    @param  keyName points to a null terminated string specifying the name of the subkey.
                    If keyName is NULL the registry information under the key specified by hKey
                    will be saved in the specified file.
    @param  regFileName points to a null terminated string specifying the file which will contain the
                        registry information.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_saveKey(RegKeyHandle hKey,
                                         rtl_uString* keyName,
                                         rtl_uString* regFileName);


/** This function merges the registry information from a specified source with the information of the
    currently open registry.

    All existing keys will be extended and existing key values will be overwritten.
    @param  hKey identifies a currently open key. The key which information is merged by this
                 function is a subkey of the key identified by hKey.
    @param  keyName points to a null terminated string specifying the name of the key which will be merged.
                    If keyName is NULL the registry information under the key specified by hKey
                    is merged with the complete information from the specified file.
    @param  regFileName points to a null terminated string specifying the file containing the
                        registry information.
    @param  bWarnings if TRUE the function returns an error if a key already exists.
    @param  bReport if TRUE the function reports warnings on stdout if a key already exists.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_mergeKey(RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName,
                                           bool bWarnings,
                                           bool bReport);


/** This function creates a new registry with the specified name and creates a root key.

    @param  registryName points to a null terminated string specifying the name of the new registry.
    @param  phRegistry points to a handle of the new registry if the function succeeds otherwise NULL.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_createRegistry(rtl_uString* registryName,
                                               RegHandle* phRegistry);


/** This function opens the root key of a registry.

    @param  hRegistry identifies a currently open registry whose rootKey will be returned.
    @param  phRootKey points to a handle of the open root key if the function succeeds otherwise NULL.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_openRootKey(RegHandle hRegistry,
                                             RegKeyHandle* phRootKey);


/** This function returns the name of a registry.

    @param  hRegistry identifies a currently open registry whose name will be returned.
    @param  pName returns the name of the registry if the function succeeds otherwise an empty string.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_getName(RegHandle hRegistry, rtl_uString** pName);


/** This function opens a registry with the specified name.

    @param  registryName points to a null terminated string specifying the name of the registry.
    @param  phRegistry points to a handle of the opened registry if the function succeeds otherwise NULL.
    @param  accessMode specifies the accessmode of the registry, RegAccessMode::READONLY or RegAccessMode::READWRITE.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_openRegistry(rtl_uString* registryName,
                                               RegHandle* phRegistry,
                                               RegAccessMode accessMode);


/** This function closes a registry.

    @param  hRegistry identifies a currently open registry which should be closed.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_closeRegistry(RegHandle hRegistry);


/** This function destroys a registry.

    @param  hRegistry identifies a currently open registry.
    @param  registryName specifies a registry name of a registry which should be destroyed. If the
                         name is NULL the registry itselfs will be destroyed.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_destroyRegistry(RegHandle hRegistry,
                                               rtl_uString* registryName);


/** This function reports the complete registry information of a key and all of its subkeys.

    All information which are available (keynames, value types, values, ...)
    will be printed to stdout for report issues only.
    @param  hKey identifies a currently open key which content will be reported.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_dumpRegistry(RegKeyHandle hKey);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
