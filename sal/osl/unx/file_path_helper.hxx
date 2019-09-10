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

#ifndef INCLUDED_SAL_OSL_UNX_FILE_PATH_HELPER_HXX
#define INCLUDED_SAL_OSL_UNX_FILE_PATH_HELPER_HXX

#include <sal/types.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>

/**
   Removes the last separator from the given system path if any and if the path
   is not the root path '/'

   @param  ppstrPath[inout]    a system path if the path is not the root path
                                and the last character is a path separator it
                                will be cut off ppstrPath must not be NULL and
                                must point to a valid rtl_String

   @returns nothing

*/
void osl_systemPathRemoveSeparator(rtl_String* pstrPath);

/**
   Returns true if the given path is a relative path and so starts not with '/'

   @param  pustrPath [in]       a system path - must not be NULL

   @retval  sal_True    the given path doesn't start with a separator
   @retval  sal_False   the given path starts with a separator

*/
bool osl_systemPathIsRelativePath(
    const rtl_uString* pustrPath);

/**
   Returns the file or the directory part of the given path

   @param pstrPath [in]        a system path, must not be NULL

   @param ppstrFileOrDirPart [out] on return receives the last part of the
                                given directory or the file name if pstrPath is the
                                root path '/' an empty string will be returned if
                                pstrPath has a trailing '/' the last part before the
                                '/' will be returned else the part after the last '/'
                                will be returned

   @returns nothing

*/
void osl_systemPathGetFileNameOrLastDirectoryPart(
    const rtl_String*  pstrPath,
    rtl_String**       ppstrFileNameOrLastDirPart);

/**
   @param   pustrPath [in] a system path, must not be NULL

   @retval  sal_True the last part of the given system path starts with '.'
   @retval  sal_False the last part of the given system path is '.' or '..'
                alone or doesn't start with a dot

*/
bool osl_systemPathIsHiddenFileOrDirectoryEntry(
    const rtl_String* pustrPath);

/************************************************
   osl_systemPathIsLocalOrParentDirectoryEntry
   Returns sal_True if the last part of the given
   system path is the local directory entry '.'
   or the parent directory entry '..'

   @param   pstrPath [in] a system path,
            must not be NULL

   @returns sal_True if the last part of the
            given system path is '.' or '..'
            else sal_False

************************************************/

bool osl_systemPathIsLocalOrParentDirectoryEntry(
    const rtl_String* pstrPath);

/************************************************
   osl_searchPath
   Searches for a file name or path name in all
   directories specified by a given path list.
   Symbolic links in the resulting path will not be
   resolved, it's up to the caller to do this.

   @param   pustrFilePath [in] a file name or
            directory name to search for, the name must
            be provided as system path not as a file URL

   @param   pustrSearchPathList [in] a ':'
            separated list of paths in which to search for
            the file or directory name

   @param   ppustrPathFound [out] on success receives the
            complete path of the file or directory found
            as a system path

   @returns sal_True if the specified file or
   directory was found else sal_False
 ***********************************************/

bool osl_searchPath(
    const rtl_uString* pustrFilePath,
    const rtl_uString* pustrSearchPathList,
    rtl_uString**      ppustrPathFound);

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

 inline void systemPathRemoveSeparator(/*inout*/ OString& Path)
 {
     osl_systemPathRemoveSeparator(Path.pData);
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

 inline bool systemPathIsRelativePath(const OUString& Path)
 {
    return osl_systemPathIsRelativePath(Path.pData);
 }

 /******************************************
    systemPathMakeAbsolutePath
    Append a relative path to a base path

    @param  BasePath [in] a system
            path that will be considered as
            base path

    @param  RelPath [in] a system path
            that will be considered as
            relative path

    @return the
            resulting path which is a
            concatenation of the base and
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

  *****************************************/

 OString systemPathMakeAbsolutePath(
     const OString& BasePath,
    const OString& RelPath);

 OUString systemPathMakeAbsolutePath(
     const OUString& BasePath,
    const OUString& RelPath);

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
     const OString& Path)
 {
    return osl_systemPathIsHiddenFileOrDirectoryEntry(Path.pData);
 }

 /************************************************
  searchPath
  ***********************************************/

 inline bool searchPath(
     const OUString& ustrFilePath,
    const OUString& ustrSearchPathList,
    OUString& ustrPathFound)
 {
     return osl_searchPath(
        ustrFilePath.pData,
        ustrSearchPathList.pData,
        &ustrPathFound.pData);
 }

 } // namespace osl

 #endif /* #ifndef _OSL_PATH_HELPER_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
