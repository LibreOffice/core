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
#ifndef INCLUDED_RTL_BOOTSTRAP_H
#define INCLUDED_RTL_BOOTSTRAP_H

#include "sal/config.h"

#include "rtl/ustring.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
   @file

   The described concept provides a platform independent way to access
   minimum bootstrap settings for every application by explicitly or
   implicitly passing the values to the application.

   <strong>MULTI-LEVEL STRATEGY FOR RETRIEVAL OF BOOTSTRAP VALUES:</strong>

   The 1st level is tried first. On failure,
   the next level is tried. Every query starts at the first level again, so
   that one setting may be taken from the 3rd and one from the 1st level.

   1st level: explicitly set variables via rtl_bootstrap_set()

   2nd level: command line arguments. A `-env:SETTINGNAME=value` is given on
   command line. This allows giving an application a certain setting, even
   if an ini-file exists (especially useful for e.g. daemons that want to
   start an executable with dynamical changing settings).

   3rd level: environment variables. The application tries to get the
   setting from the environment.

   4th level: executable ini-file. Every application looks for an ini-file.
   The filename defaults to `/absolute/path/to/executable[rc|.ini]`
   without .bin or .exe suffix. The ini-filename can be
   set by the special command line parameter
   `-env:INIFILENAME=/absolute/path/to/inifile` at runtime or it may
   be set at compile time by an API-call.

   5th level: URE_BOOTSTRAP ini-file. If the bootstrap variable URE_BOOTSTRAP
   expands to the URL of an ini-file, that ini-file is searched.

   6th level: default. An application can have some default settings decided
   at compile time, which allow the application to run even with no
   deployment settings.

   If neither of the above levels leads to a successful retrieval of the value
   (no default possible), the application may fail to start.

   <strong>NAMING CONVENTIONS</strong>

   Naming conventions for names of bootstrap values:
   Names may only include characters, that are allowed characters for
   environment variables. This excludes '.', ' ', ';', ':' and any non-ascii
   character. Names are case insensitive.

   An ini-file is only allowed to have one section, which must be named
   `[Bootstrap]` with the square brackets.
   The section may be omitted.
   The section name does not appear in the name of the corresponding
   environment variable or commandline arg.
   Values may be arbitrary unicode strings, they must be encoded in UTF8.

   <em>Example:</em>

   in an ini-file:
   <code>
   [Sectionname]
   Name=value
   </code>

   as commandline arg:
   <code>-env:Name=value</code>

   as environment:
   - <code>setenv Name value</code>
   - <code>set Name=value</code>

   <strong>SPECIAL VARIABLES:</strong>

   - INIFILENAME<br>
     This variable allows to set the inifilename. This makes only sense, if the filename
     is different than the executable file name. It must be given on command line. If it is
     given the executable ini-file is ignored.
*/

/** may be called by an application to set an ini-filename.

    Must be called before rtl_bootstrap_get(). May not be called twice.
    If it is never called, the filename is based on the name of the executable,
    with the suffix ".ini" on Windows or "rc" on Unix.

    @param pFileUri URL of the inifile with path but WITHOUT suffix (.ini or rc)
*/
SAL_DLLPUBLIC void SAL_CALL rtl_bootstrap_setIniFileName( rtl_uString *pFileUri );

/**
   @param pName
           The name of the bootstrap setting to be retrieved.
   @param[out] ppValue
           Contains always a valid rtl_uString pointer.
   @param pDefault
           maybe <code>NULL</code>. If once the default is
           returned, successive calls always return this
           default value, even when called with different
           defaults.

   @retval sal_True when a value could be retrieved successfully.
           When a <code>pDefault</code> value is given,
           the function always returns <code>sal_True</code>.
   @retval sal_False when none of the 4 methods gave a value.
           <code>ppValue</code> then contains an empty string.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL rtl_bootstrap_get(
        rtl_uString *pName, rtl_uString **ppValue, rtl_uString *pDefault );

/** Sets a bootstrap parameter.

   @param pName
          name of bootstrap parameter
   @param pValue
          value of bootstrap parameter
*/
SAL_DLLPUBLIC void SAL_CALL rtl_bootstrap_set(
        rtl_uString * pName, rtl_uString * pValue );


typedef void * rtlBootstrapHandle;

/**
   Opens a bootstrap argument container.
   @param[in] pIniName    The name of the ini-file to use, if <code>NULL</code> defaults
                          to the executables name
   @return                Handle for a bootstrap argument container
*/
SAL_DLLPUBLIC rtlBootstrapHandle SAL_CALL rtl_bootstrap_args_open(rtl_uString * pIniName);

/**
   Closes a bootstrap argument container.
   @param[in] handle      The handle got by rtl_bootstrap_args_open()
*/
SAL_DLLPUBLIC void SAL_CALL rtl_bootstrap_args_close(rtlBootstrapHandle handle)
    SAL_THROW_EXTERN_C();

/**
   @param[in]  handle       The handle got by rtl_bootstrap_args_open()
   @param[in]  pName        The name of the variable to be retrieved
   @param[out] ppValue      The result of the retrieval. *ppValue may be null in case of failure.
   @param[in]  pDefault     The default value for the retrieval, may be <code>NULL</code>

   @return                  The status of the retrieval, <code>sal_True</code> on success.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL rtl_bootstrap_get_from_handle(
        rtlBootstrapHandle handle, rtl_uString *pName, rtl_uString **ppValue, rtl_uString *pDefault);


/** Returns the name of the inifile associated with this handle.

   @param[in]  handle       The handle got by rtl_bootstrap_args_open()
   @param[out] ppIniName    contains after the call the name of the ini-filename.
*/
SAL_DLLPUBLIC void SAL_CALL rtl_bootstrap_get_iniName_from_handle(
        rtlBootstrapHandle handle, rtl_uString ** ppIniName);

/** Expands a macro using bootstrap variables.

    @param[in]     handle   The handle got by rtl_bootstrap_args_open()
    @param[in,out] macro    The macro to be expanded
*/
SAL_DLLPUBLIC void SAL_CALL rtl_bootstrap_expandMacros_from_handle(
    rtlBootstrapHandle handle, rtl_uString ** macro );

/** Expands a macro using default bootstrap variables.

    @param[in,out] macro    The macro to be expanded
*/
SAL_DLLPUBLIC void SAL_CALL rtl_bootstrap_expandMacros(
    rtl_uString ** macro);

/** Escapes special characters ("$" and "\").

    @param value
    an arbitrary, non-NULL value

    @param[out] encoded
    the given value with all occurrences of special characters ("$" and "\") escaped

    @since UDK 3.2.9
*/
SAL_DLLPUBLIC void SAL_CALL rtl_bootstrap_encode(
    rtl_uString const * value, rtl_uString ** encoded );

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
