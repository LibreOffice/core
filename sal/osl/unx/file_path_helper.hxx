/*************************************************************************
 *
 *  $RCSfile: file_path_helper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 16:46:02 $
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

#ifndef _OSL_FILE_PATH_HELPER_HXX_
#define _OSL_FILE_PATH_HELPER_HXX_


#ifndef _OSL_FILE_PATH_HELPER_H_
#include "file_path_helper.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif


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
    systemPathIsAbsolutePath
    Returns true if the given path is an
    absolute path and so starts with a '/'

    @param pustrPath [in] a system path
           pustrPath must not be NULL

    @returns sal_True if the given path
             start's with a separator else
             sal_False will be returned

  *****************************************/

 inline bool systemPathIsAbsolutePath(const rtl::OUString& Path)
 {
     return osl_systemPathIsAbsolutePath(Path.pData);
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
    systemPathGetParent
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

 inline sal_Int32 systemPathGetParent(/*inout*/ rtl::OUString& Path)
 {
    return osl_systemPathGetParent(Path.pData);
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

