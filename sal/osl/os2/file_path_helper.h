/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: file_path_helper.h,v $
 * $Revision: 1.4 $
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

 #ifndef _OSL_FILE_PATH_HELPER_H_
 #define _OSL_FILE_PATH_HELPER_H_


 #ifndef _SAL_TYPES_H_
 #include <sal/types.h>
 #endif

 #ifndef _RTL_USTRING_H_
 #include <rtl/ustring.h>
 #endif


 #ifdef __cplusplus
 extern "C"
 {
 #endif


 /*******************************************
    osl_systemPathRemoveSeparator
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

 void SAL_CALL osl_systemPathRemoveSeparator(
     /*inout*/ rtl_uString* pustrPath);

 /*******************************************
    osl_systemPathEnsureSeparator
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

 void SAL_CALL osl_systemPathEnsureSeparator(
     /*inout*/ rtl_uString** ppustrPath);

 /*******************************************
    osl_systemPathIsRelativePath
    Returns true if the given path is a
    relative path and so starts not with '/'

    @param  pustrPath [in] a system path
            pustrPath must not be NULL

    @returns sal_True if the given path
             doesn't start with a separator
             else sal_False will be returned

  ******************************************/

 sal_Bool SAL_CALL osl_systemPathIsRelativePath(
     const rtl_uString* pustrPath);

 /******************************************
    osl_systemPathIsAbsolutePath
    Returns true if the given path is an
    absolute path and so starts with a '/'

    @param pustrPath [in] a system path
           pustrPath must not be NULL

    @returns sal_True if the given path
             start's with a separator else
             sal_False will be returned

  *****************************************/

 sal_Bool SAL_CALL osl_systemPathIsAbsolutePath(
     const rtl_uString* pustrPath);

 /******************************************
    osl_systemPathMakeAbsolutePath
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

 void SAL_CALL osl_systemPathMakeAbsolutePath(
     const rtl_uString* pustrBasePath,
    const rtl_uString* pustrRelPath,
    rtl_uString**      ppustrAbsolutePath);

 /*****************************************
    osl_systemPathGetParent
    Replaces the last occurrance of a path
    separator with '\0' and returns the
    position where the '/' was replaced

    @param  pustrPath [inout] a system
            path, the last separator of
            this path will be replaced by
            a '\0'
            if the path is the root path
            '/' or the path is considered
            as to have no parent, e.g.
            '/NoParent' or 'NoParent' or
            the path is empty no
            replacement will be made
            pustrPath must not be NULL

    @returns the position of the last path
             separator that was replaced
             or 0 if no replacement took
             place

  ****************************************/

 sal_Int32 SAL_CALL osl_systemPathGetParent(
    /*inout*/ rtl_uString* pustrPath);

 /*****************************************
     osl_systemPathGetFileOrLastDirectoryPart
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
 void SAL_CALL osl_systemPathGetFileNameOrLastDirectoryPart(
     const rtl_uString*     pustrPath,
    rtl_uString**       ppustrFileNameOrLastDirPart);


 /********************************************
     osl_systemPathIsHiddenFileOrDirectoryEntry
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

 sal_Bool SAL_CALL osl_systemPathIsHiddenFileOrDirectoryEntry(
     const rtl_uString* pustrPath);


 /************************************************
     osl_systemPathIsLocalOrParentDirectoryEntry
    Returns sal_True if the last part of the given
    system path is the local directory entry '.'
    or the parent directory entry '..'

    @param pustrPath [in] a system path,
           must not be NULL

    @returns sal_True if the last part of the
             given system path is '.' or '..'
             else sal_False

 ************************************************/

 sal_Bool SAL_CALL osl_systemPathIsLocalOrParentDirectoryEntry(
     const rtl_uString* pustrPath);


 /************************************************
      osl_searchPath
    Searches for a file name or path name in all
    directories specified by a given path list.
    Symbolic links in the resulting path will not be
    resolved, it's up to the caller to do this.

    @param pustrFilePath [in] a file name or
    directory name to search for, the name must
    be provided as system path not as a file URL

    @param pustrSearchPathList [in] a ':'
    separated list of paths in which to search for
    the file or directory name

    @ppustrPathFound [out] on success receives the
    complete path of the file or directory found
    as a system path

    @returns sal_True if the specified file or
    directory was found else sal_False
  ***********************************************/

 sal_Bool SAL_CALL osl_searchPath(
     const rtl_uString* pustrFilePath,
    const rtl_uString* pustrSearchPathList,
    rtl_uString**      ppustrPathFound);


 #ifdef __cplusplus
 }
 #endif


 #endif /* #ifndef _OSL_PATH_HELPER_H_ */

