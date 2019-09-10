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

#include <sal/config.h>

#include <cassert>

#include "file_path_helper.hxx"
#include "uunxapi.hxx"

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

const sal_Unicode FPH_CHAR_PATH_SEPARATOR = '/';
const sal_Unicode FPH_CHAR_DOT            = '.';
const sal_Unicode FPH_CHAR_COLON          = ':';

void osl_systemPathRemoveSeparator(rtl_String* pstrPath)
{
    OSL_PRECOND(nullptr != pstrPath, "osl_systemPathRemoveSeparator: Invalid parameter");
    if (pstrPath != nullptr)
    {
        // maybe there are more than one separator at end
        // so we run in a loop
        while ((pstrPath->length > 1) && (pstrPath->buffer[pstrPath->length - 1] == FPH_CHAR_PATH_SEPARATOR))
        {
            pstrPath->length--;
            pstrPath->buffer[pstrPath->length] = '\0';
        }

        SAL_WARN_IF( !((0 == pstrPath->length) || (1 == pstrPath->length) ||
                     (pstrPath->length > 1 && pstrPath->buffer[pstrPath->length - 1] != FPH_CHAR_PATH_SEPARATOR)),
                     "sal.osl",
                     "osl_systemPathRemoveSeparator: Post condition failed");
    }
}

namespace {

template<typename T> void systemPathEnsureSeparator(T* ppstrPath)
{
    assert(nullptr != ppstrPath);
    sal_Int32    lp = ppstrPath->getLength();
    sal_Int32    i  = ppstrPath->lastIndexOf(FPH_CHAR_PATH_SEPARATOR);

    if ((lp > 1 && i != (lp - 1)) || ((lp < 2) && i < 0))
    {
        *ppstrPath += "/";
    }

    SAL_WARN_IF( !ppstrPath->endsWith("/"),
                 "sal.osl",
                 "systemPathEnsureSeparator: Post condition failed");
}

}

bool osl_systemPathIsRelativePath(const rtl_uString* pustrPath)
{
    OSL_PRECOND(nullptr != pustrPath, "osl_systemPathIsRelativePath: Invalid parameter");
    return ((pustrPath == nullptr) || (pustrPath->length == 0) || (pustrPath->buffer[0] != FPH_CHAR_PATH_SEPARATOR));
}

namespace {

template<typename T> T systemPathMakeAbsolutePath_(
    const T& BasePath,
    const T& RelPath)
{
    T base(BasePath);

    if (!base.isEmpty())
        systemPathEnsureSeparator(&base);

    return base + RelPath;
}

}

OString osl::systemPathMakeAbsolutePath(
    const OString& BasePath,
    const OString& RelPath)
{
    return systemPathMakeAbsolutePath_(BasePath, RelPath);
}

OUString osl::systemPathMakeAbsolutePath(
    const OUString& BasePath,
    const OUString& RelPath)
{
    return systemPathMakeAbsolutePath_(BasePath, RelPath);
}

void osl_systemPathGetFileNameOrLastDirectoryPart(
    const rtl_String*     pstrPath,
    rtl_String**       ppstrFileNameOrLastDirPart)
{
    OSL_PRECOND(pstrPath && ppstrFileNameOrLastDirPart,
                "osl_systemPathGetFileNameOrLastDirectoryPart: Invalid parameter");

    OString path(const_cast<rtl_String*>(pstrPath));

    osl_systemPathRemoveSeparator(path.pData);

    OString last_part;

    if (path.getLength() > 1 || (path.getLength() == 1 && path[0] != FPH_CHAR_PATH_SEPARATOR))
    {
        sal_Int32 idx_ps = path.lastIndexOf(FPH_CHAR_PATH_SEPARATOR);
        idx_ps++; // always right to increment by one even if idx_ps == -1!
        last_part = path.copy(idx_ps);
    }
    rtl_string_assign(ppstrFileNameOrLastDirPart, last_part.pData);
}

bool osl_systemPathIsHiddenFileOrDirectoryEntry(
    const rtl_String* pstrPath)
{
    OSL_PRECOND(nullptr != pstrPath, "osl_systemPathIsHiddenFileOrDirectoryEntry: Invalid parameter");
    if ((pstrPath == nullptr) || (pstrPath->length == 0))
        return false;

    OString fdp;
    osl_systemPathGetFileNameOrLastDirectoryPart(pstrPath, &fdp.pData);

    return ((fdp.pData->length > 0) &&
            (fdp.pData->buffer[0] == FPH_CHAR_DOT) &&
            !osl_systemPathIsLocalOrParentDirectoryEntry(fdp.pData));
}

bool osl_systemPathIsLocalOrParentDirectoryEntry(
    const rtl_String* pstrPath)
{
    OSL_PRECOND(pstrPath, "osl_systemPathIsLocalOrParentDirectoryEntry: Invalid parameter");

    OString dirent;

    osl_systemPathGetFileNameOrLastDirectoryPart(pstrPath, &dirent.pData);

    return (dirent == "." ||
            dirent == "..");
}

/** Simple iterator for a path list separated by the specified character
*/
class path_list_iterator
{
public:

    /* after construction get_current_item
       returns the first path in list, no need
       to call reset first
     */
    path_list_iterator(const OUString& path_list, sal_Unicode list_separator = FPH_CHAR_COLON) :
        m_path_list(path_list),
        m_end(m_path_list.getStr() + m_path_list.getLength() + 1),
        m_separator(list_separator)
    {
        reset();
    }

    path_list_iterator(const path_list_iterator&) = delete;
    path_list_iterator& operator=(const path_list_iterator&) = delete;

    void reset()
    {
        m_path_segment_begin = m_path_segment_end = m_path_list.getStr();
        advance();
    }

    void next()
    {
        OSL_PRECOND(!done(), "path_list_iterator: Already done!");

        m_path_segment_begin = ++m_path_segment_end;
        advance();
    }

    bool done() const
    {
        return (m_path_segment_end >= m_end);
    }

    OUString get_current_item() const
    {
        return OUString(
            m_path_segment_begin,
            (m_path_segment_end - m_path_segment_begin));
    }

private:
    /* move m_path_end to the next separator or
       to the end of the string
     */
    void advance()
    {
        while (!done() && *m_path_segment_end && (*m_path_segment_end != m_separator))
            ++m_path_segment_end;

        OSL_ASSERT(m_path_segment_end <= m_end);
    }

private:
    OUString const m_path_list;
    const sal_Unicode*  m_end;
    const sal_Unicode   m_separator;
    const sal_Unicode*  m_path_segment_begin;
    const sal_Unicode*  m_path_segment_end;
};

bool osl_searchPath(
    const rtl_uString* pustrFilePath,
    const rtl_uString* pustrSearchPathList,
    rtl_uString**      ppustrPathFound)
{
    OSL_PRECOND(pustrFilePath && pustrSearchPathList && ppustrPathFound, "osl_searchPath: Invalid parameter");

    bool               bfound = false;
    OUString      fp(const_cast<rtl_uString*>(pustrFilePath));
    OUString      pl(const_cast<rtl_uString*>(pustrSearchPathList));
    path_list_iterator pli(pl);

    while (!pli.done())
    {
        OUString p = pli.get_current_item();
        systemPathEnsureSeparator(&p);
        p += fp;

        if (osl::access(osl::OUStringToOString(p), F_OK) > -1)
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
