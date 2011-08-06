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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

 /*******************************************
     Includes
  ******************************************/

 #ifndef _OSL_FILE_PATH_HELPER_H_
 #include "file_path_helper.h"
 #endif

 #ifndef _OSL_FILE_PATH_HELPER_HXX_
 #include "file_path_helper.hxx"
 #endif

 #ifndef _OSL_UUNXAPI_HXX_
 #include "uunxapi.hxx"
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
  const sal_Unicode FPH_CHAR_COLON          = (sal_Unicode)':';

  inline const rtl::OUString FPH_PATH_SEPARATOR()
      { return rtl::OUString(FPH_CHAR_PATH_SEPARATOR); }
  inline const rtl::OUString FPH_LOCAL_DIR_ENTRY()
      { return rtl::OUString(FPH_CHAR_PATH_SEPARATOR); }
  inline const rtl::OUString FPH_PARENT_DIR_ENTRY()
      { return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("..")); }

 /*******************************************
  *  osl_systemPathRemoveSeparator
  ******************************************/

 void SAL_CALL osl_systemPathRemoveSeparator(rtl_uString* pustrPath)
 {
     OSL_PRECOND(0 != pustrPath, "osl_systemPathRemoveSeparator: Invalid parameter");
    if (0 != pustrPath)
    {
        // maybe there are more than one separator at end
        // so we run in a loop
        while ((pustrPath->length > 1) && (FPH_CHAR_PATH_SEPARATOR == pustrPath->buffer[pustrPath->length - 1]))
        {
            pustrPath->length--;
            pustrPath->buffer[pustrPath->length] = (sal_Unicode)'\0';
        }

        OSL_POSTCOND((0 == pustrPath->length) || (1 == pustrPath->length) || \
                     (pustrPath->length > 1 && pustrPath->buffer[pustrPath->length - 1] != FPH_CHAR_PATH_SEPARATOR), \
                     "osl_systemPathRemoveSeparator: Post condition failed");
    }
 }

 /*******************************************
    osl_systemPathEnsureSeparator
  ******************************************/

 void SAL_CALL osl_systemPathEnsureSeparator(rtl_uString** ppustrPath)
 {
     OSL_PRECOND((0 != ppustrPath) && (0 != *ppustrPath), "osl_systemPathEnsureSeparator: Invalid parameter");
     if ((0 != ppustrPath) && (0 != *ppustrPath))
     {
         rtl::OUString path(*ppustrPath);
         sal_Int32    lp = path.getLength();
         sal_Int32    i  = path.lastIndexOf(FPH_CHAR_PATH_SEPARATOR);

         if ((lp > 1 && i != (lp - 1)) || ((lp < 2) && i < 0))
         {
             path += FPH_PATH_SEPARATOR();
             rtl_uString_assign(ppustrPath, path.pData);
         }

         OSL_POSTCOND(path.lastIndexOf(FPH_CHAR_PATH_SEPARATOR) == (path.getLength() - 1), \
                      "osl_systemPathEnsureSeparator: Post condition failed");
     }
 }

 /*******************************************
  *  osl_systemPathIsRelativePath
  ******************************************/

 sal_Bool SAL_CALL osl_systemPathIsRelativePath(const rtl_uString* pustrPath)
 {
     OSL_PRECOND(0 != pustrPath, "osl_systemPathIsRelativePath: Invalid parameter");
     return ((0 == pustrPath) || (0 == pustrPath->length) || (pustrPath->buffer[0] != FPH_CHAR_PATH_SEPARATOR));
 }

 /******************************************
    osl_systemPathMakeAbsolutePath
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


 /*******************************************
     osl_systemPathGetFileOrLastDirectoryPart
  ******************************************/

 void SAL_CALL osl_systemPathGetFileNameOrLastDirectoryPart(
     const rtl_uString*     pustrPath,
    rtl_uString**       ppustrFileNameOrLastDirPart)
{
    OSL_PRECOND(pustrPath && ppustrFileNameOrLastDirPart, \
                "osl_systemPathGetFileNameOrLastDirectoryPart: Invalid parameter");

    rtl::OUString path(const_cast<rtl_uString*>(pustrPath));

    osl_systemPathRemoveSeparator(path.pData);

    rtl::OUString last_part;

    if (path.getLength() > 1 || (1 == path.getLength() && *path.getStr() != FPH_CHAR_PATH_SEPARATOR))
    {
        sal_Int32 idx_ps = path.lastIndexOf(FPH_CHAR_PATH_SEPARATOR);
        idx_ps++; // always right to increment by one even if idx_ps == -1!
        last_part = rtl::OUString(path.getStr() + idx_ps);
    }
    rtl_uString_assign(ppustrFileNameOrLastDirPart, last_part.pData);
}


 /********************************************
     osl_systemPathIsHiddenFileOrDirectoryEntry
 *********************************************/

 sal_Bool SAL_CALL osl_systemPathIsHiddenFileOrDirectoryEntry(
     const rtl_uString* pustrPath)
{
    OSL_PRECOND(0 != pustrPath, "osl_systemPathIsHiddenFileOrDirectoryEntry: Invalid parameter");
    if ((0 == pustrPath) || (0 == pustrPath->length))
        return sal_False;

    rtl::OUString fdp;
    osl_systemPathGetFileNameOrLastDirectoryPart(pustrPath, &fdp.pData);

    return ((fdp.pData->length > 0) &&
            (fdp.pData->buffer[0] == FPH_CHAR_DOT) &&
            !osl_systemPathIsLocalOrParentDirectoryEntry(fdp.pData));
}


 /************************************************
     osl_systemPathIsLocalOrParentDirectoryEntry
 ************************************************/

sal_Bool SAL_CALL osl_systemPathIsLocalOrParentDirectoryEntry(
    const rtl_uString* pustrPath)
{
    OSL_PRECOND(pustrPath, "osl_systemPathIsLocalOrParentDirectoryEntry: Invalid parameter");

    rtl::OUString dirent;

    osl_systemPathGetFileNameOrLastDirectoryPart(pustrPath, &dirent.pData);

    return (
            (dirent == FPH_LOCAL_DIR_ENTRY()) ||
            (dirent == FPH_PARENT_DIR_ENTRY())
           );
}

/***********************************************
 Simple iterator for a path list separated by
 the specified character
 **********************************************/

class path_list_iterator
{
public:

    /******************************************
      constructor

     after construction get_current_item
     returns the first path in list, no need
     to call reset first
     *****************************************/
    path_list_iterator(const rtl::OUString& path_list, sal_Unicode list_separator = FPH_CHAR_COLON) :
        m_path_list(path_list),
        m_end(m_path_list.getStr() + m_path_list.getLength() + 1),
        m_separator(list_separator)
    {
        reset();
    }

    /******************************************
     reset the iterator
     *****************************************/
    void reset()
    {
        m_path_segment_begin = m_path_segment_end = m_path_list.getStr();
        advance();
    }

    /******************************************
     move the iterator to the next position
     *****************************************/
    void next()
    {
        OSL_PRECOND(!done(), "path_list_iterator: Already done!");

        m_path_segment_begin = ++m_path_segment_end;
        advance();
    }

    /******************************************
     check if done
     *****************************************/
    bool done() const
    {
        return (m_path_segment_end >= m_end);
    }

    /******************************************
     return the current item
     *****************************************/
    rtl::OUString get_current_item() const
    {
        return rtl::OUString(
            m_path_segment_begin,
            (m_path_segment_end - m_path_segment_begin));
    }

private:

    /******************************************
     move m_path_end to the next separator or
     to the edn of the string
     *****************************************/
    void advance()
    {
        while (!done() && *m_path_segment_end && (*m_path_segment_end != m_separator))
            ++m_path_segment_end;

        OSL_ASSERT(m_path_segment_end <= m_end);
    }

private:
    rtl::OUString       m_path_list;
    const sal_Unicode*  m_end;
    const sal_Unicode   m_separator;
    const sal_Unicode*  m_path_segment_begin;
    const sal_Unicode*  m_path_segment_end;

// prevent copy and assignment
private:
    /******************************************
     copy constructor
     remember: do not simply copy m_path_begin
     and m_path_end because they point to
     the memory of other.m_path_list!
     *****************************************/
    path_list_iterator(const path_list_iterator& other);

    /******************************************
     assignment operator
      remember: do not simply copy m_path_begin
     and m_path_end because they point to
     the memory of other.m_path_list!
     *****************************************/
    path_list_iterator& operator=(const path_list_iterator& other);
};

 /************************************************
      osl_searchPath
  ***********************************************/

sal_Bool SAL_CALL osl_searchPath(
     const rtl_uString* pustrFilePath,
    const rtl_uString* pustrSearchPathList,
    rtl_uString**      ppustrPathFound)
{
    OSL_PRECOND(pustrFilePath && pustrSearchPathList && ppustrPathFound, "osl_searchPath: Invalid parameter");

    bool               bfound = false;
    rtl::OUString      fp(const_cast<rtl_uString*>(pustrFilePath));
    rtl::OUString      pl = rtl::OUString(const_cast<rtl_uString*>(pustrSearchPathList));
    path_list_iterator pli(pl);

    while (!pli.done())
    {
        rtl::OUString p = pli.get_current_item();
        osl::systemPathEnsureSeparator(p);
        p += fp;

        if (osl::access(p, F_OK) > -1)
        {
            bfound = true;
            rtl_uString_assign(ppustrPathFound, p.pData);
            break;
        }
        pli.next();
    }
    return bfound;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
