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

#include <rtl/ustring.h>
#include <registry/regtype.h>
#include <registry/regdllapi.h>

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


/** This function closes the specified key.

    @param  hKey identifies a currently open key which will be closed by this function.
                 The memory of the variable specifying the key will be freed.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_closeKey(RegKeyHandle hKey);


/** This function opens the root key of a registry.

    @param  hRegistry identifies a currently open registry whose rootKey will be returned.
    @param  phRootKey points to a handle of the open root key if the function succeeds otherwise NULL.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_openRootKey(RegHandle hRegistry,
                                             RegKeyHandle* phRootKey);


/** This function opens a registry with the specified name. in readonly mode.

    @param  registryName points to a null terminated string specifying the name of the registry.
    @param  phRegistry points to a handle of the opened registry if the function succeeds otherwise NULL.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_openRegistry(rtl_uString* registryName,
                                               RegHandle* phRegistry);


/** This function closes a registry.

    @param  hRegistry identifies a currently open registry which should be closed.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_closeRegistry(RegHandle hRegistry);


/** This function reports the complete registry information of a key and all of its subkeys.

    All information which are available (keynames, value types, values, ...)
    will be printed to stdout for report issues only.
    @param  hKey identifies a currently open key which content will be reported.
    @return REG_NO_ERROR if succeeds else an error code.
*/
REG_DLLPUBLIC RegError REGISTRY_CALLTYPE reg_dumpRegistry(RegKeyHandle hKey);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
