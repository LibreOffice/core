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

#include <idlc/idlc.hxx>
#include <idlc/astmodule.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>

#if defined(SAL_W32)
#include <io.h>
#include <direct.h>
#include <errno.h>
#endif

#ifdef SAL_UNX
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#endif

#include <string.h>

using namespace ::osl;

StringList* pCreatedDirectories = nullptr;

static bool checkOutputPath(const OString& completeName)
{
    OString sysPathName = convertToAbsoluteSystemPath(completeName);
    OStringBuffer buffer(sysPathName.getLength());

    if ( sysPathName.indexOf( SEPARATOR ) == -1 )
        return true;

    sal_Int32 nIndex = 0;
    OString token(sysPathName.getToken(0, SEPARATOR, nIndex));
    const sal_Char* p = token.getStr();
    if (strcmp(p, "..") == 0
        || *(p+1) == ':'
        || strcmp(p, ".") == 0)
    {
        buffer.append(token);
        buffer.append(SEPARATOR);
    }
    else
        nIndex = 0;

    do
    {
        buffer.append(sysPathName.getToken(0, SEPARATOR, nIndex));

        if ( !buffer.isEmpty() && nIndex != -1 )
        {
#if defined(SAL_UNX)
            if (mkdir(buffer.getStr(), 0777) == -1)
#else
            if (mkdir((char*)buffer.getStr()) == -1)
#endif
            {
                if (errno == ENOENT)
                {
                    fprintf(stderr, "%s: cannot create directory '%s'\n",
                            idlc()->getOptions()->getProgramName().getStr(), buffer.getStr());
                    return false;
                }
            } else
            {
                if ( !pCreatedDirectories )
                    pCreatedDirectories = new StringList();
                pCreatedDirectories->push_front(buffer.getStr());
            }
        }
        buffer.append(SEPARATOR);
    } while( nIndex != -1 );
    return true;
}

static bool cleanPath()
{
    if ( pCreatedDirectories )
    {
        StringList::iterator iter = pCreatedDirectories->begin();
        StringList::iterator end = pCreatedDirectories->end();
        while ( iter != end )
        {
//#ifdef SAL_UNX
//          if (rmdir((char*)(*iter).getStr(), 0777) == -1)
//#else
            if (rmdir((*iter).getStr()) == -1)
//#endif
            {
                fprintf(stderr, "%s: cannot remove directory '%s'\n",
                        idlc()->getOptions()->getProgramName().getStr(), (*iter).getStr());
                return false;
            }
            ++iter;
        }
        delete pCreatedDirectories;
    }
    return true;
}

void removeIfExists(const OString& pathname)
{
    osl::File::remove(OStringToOUString(pathname, RTL_TEXTENCODING_UTF8));
}

sal_Int32 SAL_CALL
produceFile(const OString& regFileName, sPair_t const*const pDepFile)
{
    Options* pOptions = idlc()->getOptions();

    OString regTmpName = regFileName.replaceAt(regFileName.getLength() -3, 3, "_idlc_");

    if ( !checkOutputPath(regFileName) )
    {
        fprintf(stderr, "%s: could not create path of registry file '%s'.\n",
                pOptions->getProgramName().getStr(), regFileName.getStr());
        return 1;
    }

    OString depTmpName;
    if (pDepFile)
    {
        depTmpName = pDepFile->first.replaceAt(
                        regFileName.getLength() -3, 3, "_idlc_");
        if ( !checkOutputPath(depTmpName) )
        {
            fprintf(stderr, "%s: could not create path of dep file '%s'.\n",
                pOptions->getProgramName().getStr(), pDepFile->first.getStr());
            return 1;
        }
        removeIfExists(depTmpName);
    }

    removeIfExists(regTmpName);
    OString urlRegTmpName = convertToFileUrl(regTmpName);

    Registry regFile;
    if ( regFile.create(OStringToOUString(urlRegTmpName, RTL_TEXTENCODING_UTF8)) != RegError::NO_ERROR )
    {
        fprintf(stderr, "%s: could not create registry file '%s'\n",
                pOptions->getProgramName().getStr(), regTmpName.getStr());
        removeIfExists(regTmpName);
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    RegistryKey rootKey;
    if ( regFile.openRootKey(rootKey) != RegError::NO_ERROR )
    {
        fprintf(stderr, "%s: could not open root of registry file '%s'\n",
                pOptions->getProgramName().getStr(), regFileName.getStr());
        removeIfExists(regTmpName);
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    // produce registry file
    if ( !idlc()->getRoot()->dump(rootKey) )
    {
        rootKey.releaseKey();
        if (regFile.close() != RegError::NO_ERROR)
        {
            fprintf(stderr, "%s: could not close registry file '%s'\n",
                    pOptions->getProgramName().getStr(), regFileName.getStr());
        }
        regFile.destroy(OStringToOUString(regFileName, RTL_TEXTENCODING_UTF8));
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    rootKey.releaseKey();
    if ( regFile.close() != RegError::NO_ERROR )
    {
        fprintf(stderr, "%s: could not close registry file '%s'\n",
                pOptions->getProgramName().getStr(), regFileName.getStr());
        removeIfExists(regTmpName);
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    if (pDepFile && !idlc()->dumpDeps(depTmpName, pDepFile->second))
    {
        fprintf(stderr, "%s: could not write dep file '%s'\n",
                pOptions->getProgramName().getStr(), pDepFile->first.getStr());
        removeIfExists(depTmpName);
        removeIfExists(pDepFile->first);
        removeIfExists(regTmpName);
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    removeIfExists(regFileName);

    if ( File::move(OStringToOUString(regTmpName, osl_getThreadTextEncoding()),
                    OStringToOUString(regFileName, osl_getThreadTextEncoding())) != FileBase::E_None ) {
        fprintf(stderr, "%s: cannot rename temporary registry '%s' to '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                regTmpName.getStr(), regFileName.getStr());
        removeIfExists(regTmpName);
        cleanPath();
        return 1;
    }
    removeIfExists(regTmpName);

    if (pDepFile)
    {
        removeIfExists(pDepFile->first);
        if ( File::move(OStringToOUString(depTmpName, osl_getThreadTextEncoding()),
                        OStringToOUString(pDepFile->first, osl_getThreadTextEncoding())) != FileBase::E_None ) {
            fprintf(stderr, "%s: cannot rename dep file '%s' to '%s'\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    depTmpName.getStr(), pDepFile->first.getStr());
            removeIfExists(depTmpName);
            removeIfExists(pDepFile->first);
            removeIfExists(regFileName);
            cleanPath();
            return 1;
        }
        removeIfExists(depTmpName);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
