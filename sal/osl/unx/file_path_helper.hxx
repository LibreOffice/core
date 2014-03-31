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

#ifndef _OSL_FILE_PATH_HELPER_HXX_
#define _OSL_FILE_PATH_HELPER_HXX_


#include "file_path_helper.h"

#include <rtl/ustring.hxx>


namespace osl
{

 /*******************************************
    systemPathRemoveSeparator
    Removes the last separator from the
    given system path if any and if the path
    is not the root path '/'

    @param  ppustrPath [inout] a system path
            if the path is not the root path
            and the last character is a
            path separator it will be cut off
               ppustrPath must not be NULL and
            must point to a valid rtl_uString

    @returns nothing

  ******************************************/

 inline void systemPathRemoveSeparator(/*inout*/ rtl::OUString& Path)
 {
     osl_systemPathRemoveSeparator(Path.pData);
 }

 /*******************************************
    systemPathEnsureSeparator
    Adds a trailing path separator to the
    given system path if not already there
    and if the path is not the root path '/'

      @param    pustrPath [inout] a system path
            if the path is not the root path
            '/' and has no trailing separator
            a separator will be added
            ppustrPath must not be NULL and
            must point to a valid rtl_uString

    @returns nothing

  ******************************************/

 inline void systemPathEnsureSeparator(/*inout*/ rtl::OUString& Path)
 {
     osl_systemPathEnsureSeparator(&Path.pData);
 }

 /*******************************************
    systemPathIsRelativePath
    Returns true if the given path is a
    relative path and so starts not with '/'

    @param  pustrPath [in] a system path
            pustrPath must not be NULL

    @returns sal_True if the given path
             doesn't start with a separator
             else sal_False will be returned

  ******************************************/

 inline bool systemPathIsRelativePath(const rtl::OUString& Path)
 {
    return osl_systemPathIsRelativePath(Path.pData);
 }

 /******************************************
    systemPathMakeAbsolutePath
    Append a relative path to a base path

    @param  pustrBasePath [in] a system
            path that will be considered as
            base path
            pustrBasePath must not be NULL

    @param  pustrRelPath [in] a system path
            that will be considered as
            relative path
            pustrBasePath must not be NULL

    @param  ppustrAbsolutePath [out] the
            resulting path which is a
            concatination of the base and
            the relative path
            if base path is empty the
            resulting absolute path is the
            relative path
            if relative path is empty the
            resulting absolute path is the
            base path
            if base and relative path are
            empty the resulting absolute
            path is also empty
            ppustrAbsolutePath must not be
            NULL and *ppustrAbsolutePath
            must be 0 or point to a valid
            rtl_uString

  *****************************************/

 inline void systemPathMakeAbsolutePath(
     const rtl::OUString& BasePath,
    const rtl::OUString& RelPath,
    rtl::OUString&       AbsolutePath)
 {
    osl_systemPathMakeAbsolutePath(
        BasePath.pData, RelPath.pData, &AbsolutePath.pData);
 }

 /********************************************
     systemPathIsHiddenFileOrDirectoryEntry
    Returns sal_True if the last part of
    given system path is not '.' or '..'
    alone and starts with a '.'

    @param pustrPath [in] a system path,
           must not be NULL

    @returns sal_True if the last part of
             the given system path starts
             with '.' or sal_False the last
             part is '.' or '..' alone or
             doesn't start with a dot

 *********************************************/

 inline bool systemPathIsHiddenFileOrDirectoryEntry(
     const rtl::OUString& Path)
 {
    return osl_systemPathIsHiddenFileOrDirectoryEntry(Path.pData);
 }

 /************************************************
  searchPath
  ***********************************************/

 inline bool searchPath(
     const rtl::OUString& ustrFilePath,
    const rtl::OUString& ustrSearchPathList,
    rtl::OUString& ustrPathFound)
 {
     return osl_searchPath(
        ustrFilePath.pData,
        ustrSearchPathList.pData,
        &ustrPathFound.pData);
 }


 } // namespace osl


 #endif /* #ifndef _OSL_PATH_HELPER_HXX_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
