/*************************************************************************
 *
 *  $RCSfile: registry.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:42 $
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

#ifndef _REGISTRY_REGISTRY_H_
#define _REGISTRY_REGISTRY_H_

#include <stddef.h>

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _REGISTRY_REGTYPE_H_
#include <registry/regtype.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/** This function creates the specified key. If the key already exists in the registry,
    the function opens it.
    @param  hKey    Identifies a currently open key. The key opened or created by this function
                    is a subkey of the key identified by hKey.
    @param  keyName Points to a null terminated string specifying the name of a key that this
                    function opens or creates.
    @param  phNewKey    Points to a variable that receives the handle of the opened or created key.
                        Memory to store this variable is allocated and is freed with the function
                        closeRegKey. If the function fails, phNewKey is NULL.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_createKey(RegKeyHandle hKey,
                                         rtl_uString* keyName,
                                         RegKeyHandle* phNewKey);


/** This function opens the specified key.
    @param  hKey    Identifies a currently open key. The key opened by this function
                    is a subkey of the key identified by hKey
    @param  keyName Points to a null terminated string specifying the name of a key that this
                    function opens.
    @param  phNewKey    Points to a variable that receives the handle of the opened key.
                        Memory to store this variable is allocated and is freed with the function
                        reg_closeKey. If the function fails, phNewKey is NULL.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_openKey(RegKeyHandle hKey,
                                         rtl_uString* keyName,
                                         RegKeyHandle* phOpenKey);



/** This function opens all subkeys of the specified key.
    @param  hKey    Identifies a currently open key. The key that subkeys opened by this
                    function is a subkey of the key identified by hKey
    @param  keyName Points to a null terminated string specifying the name of a key that subkeys
                    this function opens.
    @param  pphSubKeys  Points to a variable that receives an array of of all opened subkeys.
                        Memory to store this variable is allocated and is freed with the function
                        reg_closeSubKeys. If the function fails, pphSubKeys is NULL.
    @param  pnSubKeys   Specified the length of the array (the number of open subkeys).
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_openSubKeys(RegKeyHandle hKey,
                                             rtl_uString* keyName,
                                             RegKeyHandle** pphSubKeys,
                                             sal_uInt32* pnSubKeys);


/** This function close all subkeys specified in the array.
    @param  phSubKeys   Points to a variable that containss an array of of all opened subkeys.
                        The allocated memory of pphSubKeys and all open subkeys is freed.
    @param  nSubKeys    Specified the length of the array (the number of subkeys to closed).
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_closeSubKeys(RegKeyHandle* phSubKeys,
                                               sal_uInt32 nSubKeys);


/** This function deletes the specified key.
    @param  hKey    Identifies a currently open key. The key deleted by this function
                    is a subkey of the key identified by hKey
    @param  keyName Points to a null terminated string specifying the name of a key that this
                    function deletes.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_deleteKey(RegKeyHandle hKey,
                                         rtl_uString* keyName);


/** This function closes the specified key.
    @param  hKey    Identifies a currently open key. The key is closed by this function.
                    The memory of the variable specifying the key is freeing.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_closeKey(RegKeyHandle hKey);


/** This function returns the name of the key.
    @param  hKey     Identifies a currently open key which name will returned.
    @param  pKeyName contains the keyname if succeeds else an empty string.
*/
const RegError REGISTRY_CALLTYPE reg_getKeyName(RegKeyHandle hKey, rtl_uString** pKeyName);


/** This function sets an value under the specified key.
    @param  hKey    Identifies a currently open key. The key which value is setted by this
                    function is a subkey of the key identified by hKey.
    @param  keyName Points to a null terminated string specifying the name of a key which value
                    is set by this function. If keyName is NULL, the value is set for the key
                    specified by hKey.
    @param  valueType   Specified the type of the value.
    @param  pData   Points to a memory block containing the current data for the value.
    @param  valueSize   Specified the size of pData in bytes
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_setValue(RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           RegValueType valueType,
                                           RegValue pData,
                                           sal_uInt32 valueSize);


/** This function sets an long list value under the key.
    @param  keyName Points to a null terminated string specifying the name of a key which value
                    is set by this function. If keyName is NULL, the value is set for the key
                    specified by hKey.
    @param  pValueList  Points to an array of longs containing the current data for the value.
    @param  len Specified the len of pValueList.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_setLongListValue(RegKeyHandle hKey,
                                                      rtl_uString* keyName,
                                                      sal_Int32* pValueList,
                                                      sal_uInt32 len);


/** This function sets an ascii list value under the  key.
    @param  keyName Points to a null terminated string specifying the name of a key which value
                    is set by this function. If keyName is NULL, the value is set for the key
                    specified by hKey.
    @param  pValueList  Points to an array of sal_Char* containing the current data for the value.
    @param  len Specified the len of pValueList.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_setStringListValue(RegKeyHandle hKey,
                                                          rtl_uString* keyName,
                                                          sal_Char** pValueList,
                                                          sal_uInt32 len);


/** This function sets an unicode string list value under the  key.
    @param  keyName Points to a null terminated string specifying the name of a key which value
                    is set by this function. If keyName is NULL, the value is set for the key
                    specified by hKey.
    @param  pValueList  Points to an array of sal_Unicode* containing the current data for the value.
    @param  len Specified the len of pValueList.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_setUnicodeListValue(RegKeyHandle hKey,
                                                            rtl_uString* keyName,
                                                            sal_Unicode** pValueList,
                                                            sal_uInt32 len);


/** This function gets info about type and size of the value.
    @param  hKey    Identifies a currently open key. The key which value info is getted by this
                    function is a subkey of the key identified by hKey.
    @param  keyName Points to a null terminated string specifying the name of a key which value
                    is get by this function. If keyName is NULL, the value info is get from the
                    key specified by hKey.
    @param  pValueType  Specified the type of the value.
    @param  pValueSize  Specified the size of pData in bytes
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_getValueInfo(RegKeyHandle hKey,
                                               rtl_uString* keyName,
                                               RegValueType* pValueType,
                                               sal_uInt32* pValueSize);


/** This function gets the value under the specified key.
    @param  hKey    Identifies a currently open key. The key which value is getted by this
                    function is a subkey of the key identified by hKey.
    @param  keyName Points to a null terminated string specifying the name of a key which value
                    is get by this function. If keyName is NULL, the value is get from the key
                    specified by hKey.
    @param  pData   Points to an allocated  memory block receiving the current data for the value.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_getValue(RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           RegValue pData);


/** This function gets the value under the specified key.
    @param  keyName Points to a null terminated string specifying the name of a key which value
                    is get by this function. If keyName is NULL, the value is get from the key
                    specified by hKey.
    @param  pValueList  A Pointer to a long value list.
    @param  pLen        Specifies the length of the value list.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_getLongListValue(RegKeyHandle hKey,
                                                      rtl_uString* keyName,
                                                      sal_Int32** pValueList,
                                                      sal_uInt32* pLen);


/** This function gets the value under the specified key.
    @param  keyName Points to a null terminated string specifying the name of a key which value
                    is get by this function. If keyName is NULL, the value is get from the key
                    specified by hKey.
    @param  pValueList  A Pointer to a ascii value list.
    @param  pLen        Specifies the length of the value list.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_getStringListValue(RegKeyHandle hKey,
                                                       rtl_uString* keyName,
                                                       sal_Char*** pValueList,
                                                       sal_uInt32* pLen);


/** This function gets the value under the specified key.
    @param  keyName Points to a null terminated string specifying the name of a key which value
                    is get by this function. If keyName is NULL, the value is get from the key
                    specified by hKey.
    @param  pValueList  A Pointer to a unicode value list.
    @param  pLen        Specifies the length of the value list.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_getUnicodeListValue(RegKeyHandle hKey,
                                                         rtl_uString* keyName,
                                                         sal_Unicode*** pValueList,
                                                         sal_uInt32* pLen);


/** This function free a value list.
    @param  valueType   Specifies the type of the list members.
    @param  pValueList  A Pointer to value list.
    @param  pLen        Specifies the length of the value list.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_freeValueList(RegValueType valueType,
                                                  RegValue pValueList,
                                                  sal_uInt32 len);

RegError REGISTRY_CALLTYPE reg_createLink(RegKeyHandle hKey,
                                               rtl_uString* linkName,
                                          rtl_uString* linkTarget);

RegError REGISTRY_CALLTYPE reg_deleteLink(RegKeyHandle hKey,
                                            rtl_uString* linkName);

RegError REGISTRY_CALLTYPE reg_getKeyType(RegKeyHandle hKey,
                                          rtl_uString* keyName,
                                             RegKeyType* pKeyType);

RegError REGISTRY_CALLTYPE reg_getLinkTarget(RegKeyHandle hKey,
                                             rtl_uString* linkName,
                                               rtl_uString** pLinkTarget);

/** This function will resolved all or only the first link of a keyname
    and returns the resolved keyName in pResolvedName.
    @param  hKey    Identifies a currently open key.
    @param  keyName Points to a null terminated string specifying the relativ name of a key.
                    The name of rKey together with keyName will be resolved from links.
                    If keyName is NULL the registry information under the key specified by rKey
                    is saved in the specified file.
    @return REG_NO_ERROR if succeeds else an error code.
 */
RegError REGISTRY_CALLTYPE reg_getResolvedKeyName(RegKeyHandle hKey,
                                                    rtl_uString* keyName,
                                                   sal_Bool firstLinkOnly,
                                                      rtl_uString** pResolvedName);

/** This function loads registry information from a specified file and save it under the
    specified keyName.
    @param  hKey    Specifies the key where the subkey will be created.
    @param  keyName Points to a null terminated string specifying the name of the created subkey.
                    If keyName is NULL the registry information is saved under the key specified
                    by hKey.
    @param  regFileName Points to a null terminated string specifying the file containing the
            registry information.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_loadKey(RegKeyHandle hKey,
                                         rtl_uString* keyName,
                                         rtl_uString* regFileName);


/** This function saves the registry information from the specified key and all subkeys and save
    it in the specified file.
    @param  hKey    Identifies a currently open key. The key which inforamtion is saved by this
                    function is a subkey of the key identified by hKey.
    @param  keyName Points to a null terminated string specifying the name of the subkey.
                    If keyName is NULL the registry information under the key specified by hKey
                    is saved in the specified file.
    @param  regFileName Points to a null terminated string specifying the file containing the
            registry information.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_saveKey(RegKeyHandle hKey,
                                         rtl_uString* keyName,
                                         rtl_uString* regFileName);


/** This function merges the registry information from the specified key and the registry
    information of the specified file. Existing keys will extended.
    @param  hKey    Identifies a currently open key. The key which inforamtion is merged by this
                    function is a subkey of the key identified by hKey.
    @param  keyName Points to a null terminated string specifying the name of the subkey.
                    If keyName is NULL the registry information under the key specified by hKey
                    is merged with the information from the specified file.
    @param  regFileName Points to a null terminated string specifying the file containing the
            registry information.
    @param  bWarnings if TRUE the function returns an error if a key already exists.
    @param  bReport if TRUE the function reports warnings on stdout if a key already exists.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_mergeKey(RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName,
                                           sal_Bool bWarnings,
                                           sal_Bool bReport);


/** This function creates a new registry with the specified name and creates a root key.
    @param  registryName    Points to a null terminated string specifying the name of the new registry.
    @param  phRegistry  Points to a variable that receives the handle of the created registry.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_createRegistry(rtl_uString* registryName,
                                               RegHandle* phRegistry);


/** This function opens the root key of the sepcified registry.
    @param  hReg        Identifies a currently open registry which rootKey will returned.
    @param  phRootKey   Points to a variable that receives the handle of the open root key.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_openRootKey(RegHandle hRegistry,
                                             RegKeyHandle* phRootKey);


/** This function returns the name of the registry.
    @param  hReg    Identifies a currently open registry which name will returned.
    @param  pName   name if succeeds else an empty string.
*/
const RegError REGISTRY_CALLTYPE reg_getName(RegHandle hRegistry, rtl_uString** pName);


/** This function returns the access mode of the registry.
    @param  hReg    Identifies a currently open registry.
    @return TRUE if accessmode is read only else FALSE.
*/
sal_Bool REGISTRY_CALLTYPE reg_isReadOnly(RegHandle hReg);


/** This function opens a registry with the specified name.
    @param  registryName Identifies a registry name.
    @param  phRegistry Points to a variable that receives the handle of the opened registry.
    @param  accessMode specify the accessmode for the registry, REG_READONLY or REG_READWRITE.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_openRegistry(rtl_uString* registryName,
                                               RegHandle* phRegistry,
                                               RegAccessMode accessMode);


/** This function close the specified registry.
    @param  hRegistry   Identifies a currently open registry.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_closeRegistry(RegHandle hRegistry);


/** This function destroy the specified registry.
    @param  hRegistry   Identifies a currently open registry.
    @param  registryName Identifies a registry name, if the name is NULL the registry
            itselfs will be destroyed.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_destroyRegistry(RegHandle hRegistry,
                                               rtl_uString* registryName);


/** This function dump the content under the open key to stdout
    @param  hKey    Identifies a currently open key which subtree is dumped.
    @return REG_NO_ERROR if succeeds else an error code.
*/
RegError REGISTRY_CALLTYPE reg_dumpRegistry(RegKeyHandle hKey);

#ifdef __cplusplus
}
#endif

#endif

