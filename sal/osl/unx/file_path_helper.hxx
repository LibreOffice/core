/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

 /*****************************************
     systemPathGetFileOrLastDirectoryPart
    Returns the file or the directory part
    of the given path

    @param pustrPath [in] a system path,
           must not be NULL

    @param ppustrFileOrDirPart [out] on
           return receives the last part
           of the given directory or the
           file name
           if pustrPath is the root path
           '/' an empty string will be
           returned
           if pustrPath has a trailing
           '/' the last part before the
           '/' will be returned else
           the part after the last '/'
           will be returned

    @returns nothing

  ****************************************/

 inline void systemPathGetFileNameOrLastDirectoryPart(
     const rtl::OUString& Path,
    rtl::OUString&       FileNameOrLastDirPart)
 {
    osl_systemPathGetFileNameOrLastDirectoryPart(
        Path.pData, &FileNameOrLastDirPart.pData);
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
     systemPathIsLocalOrParentDirectoryEntry
    Returns sal_True if the last part of the given
    system path is the local directory entry '.'
    or the parent directory entry '..'

    @param pustrPath [in] a system path,
           must not be NULL

    @returns sal_True if the last part of the
             given system path is '.' or '..'
             else sal_False

 ************************************************/

 inline bool systemPathIsLocalOrParentDirectoryEntry(
     const rtl::OUString& Path)
 {
    return osl_systemPathIsLocalOrParentDirectoryEntry(Path.pData);
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
