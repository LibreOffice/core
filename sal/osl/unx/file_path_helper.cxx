/*************************************************************************
 *
 *  $RCSfile: file_path_helper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2002-11-29 10:38:29 $
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

 /*******************************************
     Includes
  ******************************************/

 #ifndef _OSL_PATH_HELPER_H_
 #include "file_path_helper.h"
 #endif

 #ifndef _OSL_DIAGNOSE_H_
 #include <osl/diagnose.h>
 #endif

 #ifndef _RTL_USTRING_HXX_
 #include <rtl/ustring.hxx>
 #endif

 /*******************************************
     Constants
  ******************************************/

  const sal_Unicode FPH_CHAR_PATH_SEPARATOR = (sal_Unicode)'/';
  const sal_Unicode FPH_CHAR_DOT            = (sal_Unicode)'.';

  const rtl::OUString FPH_PATH_SEPARATOR   = rtl::OUString::createFromAscii("/");
  const rtl::OUString FPH_LOCAL_DIR_ENTRY  = rtl::OUString::createFromAscii(".");
  const rtl::OUString FPH_PARENT_DIR_ENTRY = rtl::OUString::createFromAscii("..");

 /*******************************************
  *  osl_systemPathRemoveSeparator
  ******************************************/

 void SAL_CALL osl_systemPathRemoveSeparator(rtl_uString* pustrPath)
 {
     OSL_PRECOND(pustrPath, "osl_systemPathRemoveSeparator: Invalid parameter");

    // maybe there are more than one separator at end
    // so we run in a loop
    while ((pustrPath->length > 1) && (FPH_CHAR_PATH_SEPARATOR == pustrPath->buffer[pustrPath->length - 1]))
    {
        pustrPath->length--;
        pustrPath->buffer[pustrPath->length] = (sal_Unicode)'\0';
    }

    OSL_POSTCOND((pustrPath->length > 0) && pustrPath->buffer[pustrPath->length - 1] != FPH_CHAR_PATH_SEPARATOR, \
                 "osl_systemPathRemoveSeparator: Post condition failed");
 }

 /*******************************************
    osl_systemPathEnsureSeparator
    Adds a trailing path separator to the
    given system path if not already there
    and if the path is not the root path '/'

      @param    pustrPath [inout] a system path
            if the path is not the root path
            '/' and has no trailing separator
            a separator will be added
            pustrPath must not be NULL

    @returns nothing

  ******************************************/

 void SAL_CALL osl_systemPathEnsureSeparator(rtl_uString** ppustrPath)
 {
     OSL_PRECOND(ppustrPath && (NULL != *ppustrPath), \
                "osl_systemPathEnsureSeparator: Invalid parameter");

     rtl::OUString path(*ppustrPath);
    sal_Int32     lp = path.getLength();

    if ((lp > 1) && (path.lastIndexOf(FPH_PATH_SEPARATOR) != (lp - 1)))
    {
        path += FPH_PATH_SEPARATOR;
        rtl_uString_acquire(path.pData);
        *ppustrPath = path.pData;
    }

    OSL_POSTCOND(path.lastIndexOf(FPH_PATH_SEPARATOR) == (path.getLength() - 1), \
                 "osl_systemPathEnsureSeparator: Post condition failed");
 }

 /*******************************************
  *  osl_systemPathIsRelativePath
  ******************************************/

 sal_Bool SAL_CALL osl_systemPathIsRelativePath(const rtl_uString* pustrPath)
 {
     OSL_PRECOND(pustrPath, "osl_systemPathIsRelativePath: Invalid parameter");
     return ((0 == pustrPath->length) || (pustrPath->buffer[0] != FPH_CHAR_PATH_SEPARATOR));
 }

 /******************************************
  *  osl_systemPathIsAbsolutePath
  *****************************************/

 sal_Bool SAL_CALL osl_systemPathIsAbsolutePath(const rtl_uString* pustrPath)
 {
     OSL_PRECOND(pustrPath, "osl_systemPathIsAbsolutePath: Invalid parameter");
     return (!osl_systemPathIsRelativePath(pustrPath));
 }

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
    rtl_uString**      ppustrAbsolutePath)
{
    rtl::OUString base(rtl_uString_getStr(const_cast<rtl_uString*>(pustrBasePath)));
    rtl::OUString rel(const_cast<rtl_uString*>(pustrRelPath));

    if (base.getLength() > 0)
        osl_systemPathEnsureSeparator(&base.pData);

    base += rel;

    rtl_uString_acquire(base.pData);
    *ppustrAbsolutePath = base.pData;
}


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

 sal_Int32 SAL_CALL osl_systemPathGetParent(rtl_uString* pustrPath)
 {
     return 0;
 }

 /*****************************************
     osl_systemPathGetFileOrLastDirectoryPart
    Returns the file or the directory part
    of the given path

    @param pustrPath [in] a system path

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
    rtl_uString**       ppustrFileNameOrLastDirPart)
{
    OSL_PRECOND(pustrPath && ppustrFileNameOrLastDirPart, \
                "osl_systemPathGetFileNameOrLastDirectoryPart: Invalid parameter");

    sal_Unicode* p = rtl_uString_getStr(const_cast<rtl_uString*>(pustrPath));
    sal_Int32    i = rtl_ustr_lastIndexOfChar(p, FPH_CHAR_PATH_SEPARATOR);

    if (-1 != i)
        p += (i + 1);

    rtl_uString_newFromStr(ppustrFileNameOrLastDirPart, p);
}


 /********************************************
     osl_systemPathIsHiddenFileOrDirectoryEntry
    Returns sal_True if the last part of
    given system path is not '.' or '..'
    alone and starts with a '.'

    @param pustrPath [in] a system path,
           must not be 0

    @returns sal_True if the last part of
             the given system path starts
             with '.' or sal_False the last
             part is '.' or '..' alone or
             doesn't start with a dot

 *********************************************/

 sal_Bool SAL_CALL osl_systemPathIsHiddenFileOrDirectoryEntry(
     const rtl_uString* pustrPath)
{
    OSL_PRECOND(pustrPath, "osl_systemPathIsHiddenFileOrDirectoryEntry: Invalid parameter");

    sal_Bool is_hidden = sal_False;

    if (pustrPath->length > 0)
    {
        rtl::OUString fdp;

        osl_systemPathGetFileNameOrLastDirectoryPart(pustrPath, &fdp.pData);

        is_hidden = ((fdp.pData->length > 0) && (fdp.pData->buffer[0] == FPH_CHAR_DOT) &&
                     !osl_systemPathIsLocalOrParentDirectoryEntry(fdp.pData));
     }

    return is_hidden;
}


 /************************************************
     osl_systemPathIsLocalOrParentDirectoryEntry
    Returns sal_True if the last part of the given
    system path is the local directory entry '.'
    or the parent directory entry '..'

    @param pustrPath [in] a system path,
           must not be 0

    @returns sal_True if the last part of the
             given system path is '.' or '..'
             else sal_False

 ************************************************/

sal_Bool SAL_CALL osl_systemPathIsLocalOrParentDirectoryEntry(
    const rtl_uString* pustrPath)
{
    OSL_PRECOND(pustrPath, "osl_systemPathIsLocalOrParentDirectoryEntry: Invalid argument");

    rtl::OUString dirent;

    osl_systemPathGetFileNameOrLastDirectoryPart(pustrPath, &dirent.pData);

    return ((dirent == FPH_LOCAL_DIR_ENTRY) || (dirent == FPH_PARENT_DIR_ENTRY));
}



