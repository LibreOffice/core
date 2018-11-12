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

#include "system.hxx"
#include "readwrite_helper.hxx"
#include "file_url.hxx"
#include "unixerrnostring.hxx"

#include <osl/diagnose.h>
#include <osl/profile.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/alloc.h>
#include <sal/log.hxx>

#define LINES_INI       32
#define LINES_ADD       10
#define SECTIONS_INI    5
#define SECTIONS_ADD    3
#define ENTRIES_INI     5
#define ENTRIES_ADD     3

#define STR_INI_BOOLYES     "yes"
#define STR_INI_BOOLON      "on"
#define STR_INI_BOOLONE     "1"
#define STR_INI_BOOLNO      "no"
#define STR_INI_BOOLOFF     "off"
#define STR_INI_BOOLZERO    "0"

#define FLG_USER            0x00FF
#define FLG_AUTOOPEN        0x0100
#define FLG_MODIFIED        0x0200

#define DEFAULT_PMODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

typedef time_t  osl_TStamp;

enum osl_TLockMode
{
    un_lock, read_lock, write_lock
};

struct osl_TFile
{
    int     m_Handle;
    sal_Char*   m_pReadPtr;
    sal_Char    m_ReadBuf[512];
    sal_Char*   m_pWriteBuf;
    sal_uInt32  m_nWriteBufLen;
    sal_uInt32  m_nWriteBufFree;
};

struct osl_TProfileEntry
{
    sal_uInt32  m_Line;
    sal_uInt32  m_Offset;
    sal_uInt32  m_Len;
};

struct osl_TProfileSection
{
    sal_uInt32  m_Line;
    sal_uInt32  m_Offset;
    sal_uInt32  m_Len;
    sal_uInt32  m_NoEntries;
    sal_uInt32  m_MaxEntries;
    osl_TProfileEntry*  m_Entries;
};

/* Profile-data structure hidden behind oslProfile: */
struct osl_TProfileImpl
{
    sal_uInt32  m_Flags;
    osl_TFile*  m_pFile;
    osl_TStamp  m_Stamp;
    sal_Char    m_FileName[PATH_MAX + 1];
    sal_uInt32  m_NoLines;
    sal_uInt32  m_MaxLines;
    sal_uInt32  m_NoSections;
    sal_uInt32  m_MaxSections;
    sal_Char**  m_Lines;
    osl_TProfileSection* m_Sections;
    pthread_mutex_t m_AccessLock;
    bool        m_bIsValid;
};

static osl_TFile* openFileImpl(const sal_Char* pszFilename, oslProfileOption ProfileFlags);
static osl_TStamp closeFileImpl(osl_TFile* pFile, oslProfileOption Flags);
static bool   OslProfile_lockFile(const osl_TFile* pFile, osl_TLockMode eMode);
static bool   OslProfile_rewindFile(osl_TFile* pFile, bool bTruncate);
static osl_TStamp OslProfile_getFileStamp(osl_TFile* pFile);

static sal_Char*   OslProfile_getLine(osl_TFile* pFile);
static bool   OslProfile_putLine(osl_TFile* pFile, const sal_Char *pszLine);
static sal_Char* stripBlanks(sal_Char* String, sal_uInt32* pLen);
static sal_Char* addLine(osl_TProfileImpl* pProfile, const sal_Char* Line);
static sal_Char* insertLine(osl_TProfileImpl* pProfile, const sal_Char* Line, sal_uInt32 LineNo);
static void removeLine(osl_TProfileImpl* pProfile, sal_uInt32 LineNo);
static void setEntry(osl_TProfileImpl* pProfile, osl_TProfileSection* pSection,
                     sal_uInt32 NoEntry, sal_uInt32 Line,
                     sal_Char* Entry, sal_uInt32 Len);
static bool addEntry(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection,
                     int Line, sal_Char* Entry, sal_uInt32 Len);
static void removeEntry(osl_TProfileSection *pSection, sal_uInt32 NoEntry);
static bool addSection(osl_TProfileImpl* pProfile, int Line, const sal_Char* Section, sal_uInt32 Len);
static void removeSection(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection);
static osl_TProfileSection* findEntry(osl_TProfileImpl* pProfile, const sal_Char* Section,
                                      const sal_Char* Entry, sal_uInt32 *pNoEntry);
static bool loadProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile);
static bool storeProfile(osl_TProfileImpl* pProfile, bool bCleanup);
static osl_TProfileImpl* acquireProfile(oslProfile Profile, bool bWriteable);
static bool releaseProfile(osl_TProfileImpl* pProfile);

static bool writeProfileImpl (osl_TFile* pFile);
static osl_TFile* osl_openTmpProfileImpl(osl_TProfileImpl*);
static bool osl_ProfileSwapProfileNames(osl_TProfileImpl*);
static void osl_ProfileGenerateExtension(const sal_Char* pszFileName, const sal_Char* pszExtension, sal_Char* pszTmpName, int BufferMaxLen);
static oslProfile osl_psz_openProfile(const sal_Char *pszProfileName, oslProfileOption Flags);

oslProfile SAL_CALL osl_openProfile(rtl_uString *ustrProfileName, oslProfileOption Options)
{
    char profilePath[PATH_MAX] = "";
    return
        (ustrProfileName == nullptr
         || ustrProfileName->buffer[0] == 0
         || (FileURLToPath(profilePath, PATH_MAX, ustrProfileName)
             == osl_File_E_None))
        ? osl_psz_openProfile(profilePath, Options)
        : nullptr;
}

static oslProfile osl_psz_openProfile(const sal_Char *pszProfileName, oslProfileOption Flags)
{
    osl_TFile*        pFile;
    osl_TProfileImpl* pProfile;
    bool bRet = false;

    if ( ( pFile = openFileImpl(pszProfileName, Flags ) ) == nullptr )
    {
        return nullptr;
    }

    pProfile = static_cast<osl_TProfileImpl*>(calloc(1, sizeof(osl_TProfileImpl)));

    if ( pProfile == nullptr )
    {
        closeFileImpl(pFile, Flags);
        return nullptr;
    }

    pProfile->m_Flags = Flags & FLG_USER;

    if ( Flags & ( osl_Profile_READLOCK | osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE ) )
    {
        pProfile->m_pFile = pFile;
    }

    pthread_mutex_init(&(pProfile->m_AccessLock),PTHREAD_MUTEXATTR_DEFAULT);
    pProfile->m_bIsValid = true;

    pProfile->m_Stamp = OslProfile_getFileStamp(pFile);
    bRet=loadProfile(pFile, pProfile);
    bRet &= realpath(pszProfileName, pProfile->m_FileName) != nullptr;
    SAL_WARN_IF(!bRet, "sal.osl", "realpath(pszProfileName, pProfile->m_FileName) != NULL ==> false");

    if (pProfile->m_pFile == nullptr)
        closeFileImpl(pFile,pProfile->m_Flags);

    // coverity[leaked_storage] - pFile is not leaked
    return pProfile;
}

sal_Bool SAL_CALL osl_closeProfile(oslProfile Profile)
{
    osl_TProfileImpl* pProfile = static_cast<osl_TProfileImpl*>(Profile);
    osl_TProfileImpl* pTmpProfile;

    if ( Profile == nullptr )
    {
        return false;
    }

    pthread_mutex_lock(&(pProfile->m_AccessLock));

    if ( !pProfile->m_bIsValid )
    {
        SAL_WARN("sal.osl", "!pProfile->m_bIsValid");
        pthread_mutex_unlock(&(pProfile->m_AccessLock));

        return false;
    }

    pProfile->m_bIsValid = false;

    if ( ! ( pProfile->m_Flags & osl_Profile_READLOCK ) && ( pProfile->m_Flags & FLG_MODIFIED ) )
    {
        pTmpProfile = acquireProfile(Profile, true);

        if ( pTmpProfile != nullptr )
        {
            bool bRet = storeProfile(pTmpProfile, true);
            SAL_WARN_IF(!bRet, "sal.osl", "storeProfile(pTmpProfile, true) ==> false");
        }
    }
    else
    {
        pTmpProfile = acquireProfile(Profile, false);
    }

    if ( pTmpProfile == nullptr )
    {
        pthread_mutex_unlock(&(pProfile->m_AccessLock));

        SAL_INFO("sal.osl", "Out osl_closeProfile [pProfile==0]");
        return false;
    }

    pProfile = pTmpProfile;

    if (pProfile->m_pFile != nullptr)
        closeFileImpl(pProfile->m_pFile,pProfile->m_Flags);

    pProfile->m_pFile = nullptr;
    pProfile->m_FileName[0] = '\0';

    /* release whole profile data types memory */
    if ( pProfile->m_NoLines > 0)
    {
        unsigned int idx=0;
        if ( pProfile->m_Lines != nullptr )
        {
            for ( idx = 0 ; idx < pProfile->m_NoLines ; ++idx)
            {
                if ( pProfile->m_Lines[idx] != nullptr )
                {
                    free(pProfile->m_Lines[idx]);
                    pProfile->m_Lines[idx]=nullptr;
                }
            }
            free(pProfile->m_Lines);
            pProfile->m_Lines=nullptr;
        }
        if ( pProfile->m_Sections != nullptr )
        {
            /*osl_TProfileSection* pSections=pProfile->m_Sections;*/
            for ( idx = 0 ; idx < pProfile->m_NoSections ; ++idx )
            {
                if ( pProfile->m_Sections[idx].m_Entries != nullptr )
                {
                    free(pProfile->m_Sections[idx].m_Entries);
                    pProfile->m_Sections[idx].m_Entries=nullptr;
                }
            }
            free(pProfile->m_Sections);
            pProfile->m_Sections=nullptr;
        }
    }

    pthread_mutex_unlock(&(pProfile->m_AccessLock));

    pthread_mutex_destroy(&(pProfile->m_AccessLock));

    free(pProfile);

    return true;
}

sal_Bool SAL_CALL osl_flushProfile(oslProfile Profile)
{
    osl_TProfileImpl* pProfile = static_cast<osl_TProfileImpl*>(Profile);
    osl_TFile* pFile;
    bool bRet = false;

    if ( pProfile == nullptr )
    {
        return false;
    }

    pthread_mutex_lock(&(pProfile->m_AccessLock));

    if ( !pProfile->m_bIsValid )
    {
        SAL_WARN_IF(!pProfile->m_bIsValid, "sal.osl", "!pProfile->m_bIsValid");
        pthread_mutex_unlock(&(pProfile->m_AccessLock));
        return false;
    }

    pFile = pProfile->m_pFile;
    if ( !( pFile != nullptr && pFile->m_Handle >= 0 ) )
    {
        pthread_mutex_unlock(&(pProfile->m_AccessLock));

        return false;
    }

    if ( pProfile->m_Flags & FLG_MODIFIED )
    {
        bRet = storeProfile(pProfile, false);
        SAL_WARN_IF(!bRet, "sal.osl", "storeProfile(pProfile, false) ==> false");
    }

    pthread_mutex_unlock(&(pProfile->m_AccessLock));
    return bRet;
}

static bool writeProfileImpl(osl_TFile* pFile)
{
    if ( !( pFile != nullptr && pFile->m_Handle >= 0 ) || ( pFile->m_pWriteBuf == nullptr ) )
    {
        return false;
    }

    SAL_WARN_IF(
        (strlen(pFile->m_pWriteBuf)
         != pFile->m_nWriteBufLen - pFile->m_nWriteBufFree),
        "sal.osl",
        strlen(pFile->m_pWriteBuf) << " != "
            << (pFile->m_nWriteBufLen - pFile->m_nWriteBufFree));

    if ( !safeWrite(pFile->m_Handle, pFile->m_pWriteBuf, pFile->m_nWriteBufLen - pFile->m_nWriteBufFree) )
    {
        SAL_INFO("sal.osl", "write failed: " << UnixErrnoString(errno));
        return false;
    }

    free(pFile->m_pWriteBuf);
    pFile->m_pWriteBuf=nullptr;
    pFile->m_nWriteBufLen=0;
    pFile->m_nWriteBufFree=0;

    return true;
}

sal_Bool SAL_CALL osl_readProfileString(oslProfile Profile,
                                        const sal_Char* pszSection,
                                        const sal_Char* pszEntry,
                                        sal_Char* pszString,
                                        sal_uInt32 MaxLen,
                                        const sal_Char* pszDefault)
{
    sal_uInt32    NoEntry;
    sal_Char* pStr=nullptr;
    osl_TProfileImpl*    pProfile=nullptr;
    osl_TProfileImpl*    pTmpProfile=nullptr;
    bool bRet = false;

    pTmpProfile = static_cast<osl_TProfileImpl*>(Profile);

    if ( pTmpProfile == nullptr )
    {
        return false;
    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( !pTmpProfile->m_bIsValid )
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return false;
    }

    pProfile = acquireProfile(Profile, false);

    if ( pProfile == nullptr )
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return false;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        osl_TProfileSection* pSec;
        if (((pSec = findEntry(pProfile, pszSection, pszEntry, &NoEntry)) != nullptr) &&
            (NoEntry < pSec->m_NoEntries) &&
            ((pStr = strchr(pProfile->m_Lines[pSec->m_Entries[NoEntry].m_Line],
                            '=')) != nullptr))
        {
            pStr++;
        }
        else
        {
            pStr=const_cast<sal_Char*>(pszDefault);
        }

        if ( pStr != nullptr )
        {
            pStr = stripBlanks(pStr, nullptr);
            MaxLen = (MaxLen - 1 < strlen(pStr)) ? (MaxLen - 1) : strlen(pStr);
            pStr = stripBlanks(pStr, &MaxLen);
            strncpy(pszString, pStr, MaxLen);
            pszString[MaxLen] = '\0';
        }
    }
    else
    { /* not implemented */ }

    bRet=releaseProfile(pProfile);
    SAL_WARN_IF(!bRet, "sal.osl", "releaseProfile(pProfile) ==> false");

    if ( pStr == nullptr )
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return false;
    }

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

    return true;
}

sal_Bool SAL_CALL osl_readProfileBool(oslProfile Profile,
                                      const sal_Char* pszSection,
                                      const sal_Char* pszEntry,
                                      sal_Bool Default)
{
    sal_Char Line[32];
    Line[0] = '\0';

    if (osl_readProfileString(Profile, pszSection, pszEntry, Line, sizeof(Line), ""))
    {
        if ((strcasecmp(Line, STR_INI_BOOLYES) == 0) ||
            (strcasecmp(Line, STR_INI_BOOLON)  == 0) ||
            (strcasecmp(Line, STR_INI_BOOLONE) == 0))
            Default = true;
        else
            if ((strcasecmp(Line, STR_INI_BOOLNO)   == 0) ||
                (strcasecmp(Line, STR_INI_BOOLOFF)  == 0) ||
                (strcasecmp(Line, STR_INI_BOOLZERO) == 0))
                Default = false;
    }

    return Default;
}

sal_uInt32 SAL_CALL osl_readProfileIdent(oslProfile Profile,
                                         const sal_Char* pszSection,
                                         const sal_Char* pszEntry,
                                         sal_uInt32 FirstId,
                                         const sal_Char* Strings[],
                                         sal_uInt32 Default)
{
    sal_uInt32  i;
    sal_Char    Line[256];
    Line[0] = '\0';

    if (osl_readProfileString(Profile, pszSection, pszEntry, Line, sizeof(Line), ""))
    {
        i = 0;
        while (Strings[i] != nullptr)
        {
            if (strcasecmp(Line, Strings[i]) == 0)
            {
                Default = i + FirstId;
                break;
            }
            i++;
        }
    }

    return Default;
}

sal_Bool SAL_CALL osl_writeProfileString(oslProfile Profile,
                                         const sal_Char* pszSection,
                                         const sal_Char* pszEntry,
                                         const sal_Char* pszString)
{
    sal_uInt32  i;
    bool bRet = false;
    sal_uInt32    NoEntry;
    sal_Char* pStr;
    sal_Char*       Line = nullptr;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = nullptr;
    osl_TProfileImpl*    pTmpProfile = nullptr;

    pTmpProfile = static_cast<osl_TProfileImpl*>(Profile);

    if ( pTmpProfile == nullptr )
    {
        return false;
    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( !pTmpProfile->m_bIsValid )
    {
        SAL_WARN_IF(!pTmpProfile->m_bIsValid, "sal.osl", "!pTmpProfile->m_bIsValid");
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return false;
    }

    pProfile=acquireProfile(Profile, true);

    if (pProfile == nullptr)
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return false;
    }

    Line = static_cast<sal_Char*>(malloc(strlen(pszEntry)+strlen(pszString)+48));

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if ((pSec = findEntry(pProfile, pszSection, pszEntry, &NoEntry)) == nullptr)
        {
            Line[0] = '\0';
            addLine(pProfile, Line);

            Line[0] = '[';
            strcpy(&Line[1], pszSection);
            Line[1 + strlen(pszSection)] = ']';
            Line[2 + strlen(pszSection)] = '\0';

            if (((pStr = addLine(pProfile, Line)) == nullptr) ||
                (! addSection(pProfile, pProfile->m_NoLines - 1, &pStr[1], strlen(pszSection))))
            {
                bRet=releaseProfile(pProfile);
                SAL_WARN_IF(!bRet, "sal.osl", "releaseProfile(pProfile) ==> false");

                pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

                free(Line);
                return false;
            }

            pSec = &pProfile->m_Sections[pProfile->m_NoSections - 1];
            NoEntry = pSec->m_NoEntries;
        }

        Line[0] = '\0';
        strcpy(&Line[0], pszEntry);
        Line[0 + strlen(pszEntry)] = '=';
        strcpy(&Line[1 + strlen(pszEntry)], pszString);

        if (NoEntry >= pSec->m_NoEntries)
        {
            if (pSec->m_NoEntries > 0)
                i = pSec->m_Entries[pSec->m_NoEntries - 1].m_Line + 1;
            else
                i = pSec->m_Line + 1;

            if (((pStr = insertLine(pProfile, Line, i)) == nullptr) ||
                (! addEntry(pProfile, pSec, i, pStr, strlen(pszEntry))))
            {
                bRet=releaseProfile(pProfile);
                SAL_WARN_IF(!bRet, "sal.osl", "releaseProfile(pProfile) ==> false");

                pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
                free(Line);

                return false;
            }

            pProfile->m_Flags |= FLG_MODIFIED;
        }
        else
        {
            i = pSec->m_Entries[NoEntry].m_Line;
            free(pProfile->m_Lines[i]);
            pProfile->m_Lines[i] = strdup(Line);
            setEntry(pProfile, pSec, NoEntry, i, pProfile->m_Lines[i], strlen(pszEntry));

            pProfile->m_Flags |= FLG_MODIFIED;
        }
    }
    else {
        /* not implemented */
    }

    bRet = releaseProfile(pProfile);
    SAL_WARN_IF(!bRet, "sal.osl", "releaseProfile(pProfile) ==> false");

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
    if ( Line!= nullptr )
    {
        free(Line);
    }

    return bRet;
}

sal_Bool SAL_CALL osl_writeProfileBool(oslProfile Profile,
                                       const sal_Char* pszSection,
                                       const sal_Char* pszEntry,
                                       sal_Bool Value)
{
    bool bRet = false;

    if (Value)
        bRet=osl_writeProfileString(Profile, pszSection, pszEntry, STR_INI_BOOLONE);
    else
        bRet=osl_writeProfileString(Profile, pszSection, pszEntry, STR_INI_BOOLZERO);

    return bRet;
}

sal_Bool SAL_CALL osl_writeProfileIdent(oslProfile Profile,
                                        const sal_Char* pszSection,
                                        const sal_Char* pszEntry,
                                        sal_uInt32 FirstId,
                                        const sal_Char* Strings[],
                                        sal_uInt32 Value)
{
    int i, n = 0;
    bool bRet = false;

    while (Strings[n] != nullptr)
        ++n;

    if ((i = Value - FirstId) >= n)
        bRet = false;
    else
        bRet = osl_writeProfileString(Profile, pszSection, pszEntry, Strings[i]);

    return bRet;
}

sal_Bool SAL_CALL osl_removeProfileEntry(oslProfile Profile,
                                         const sal_Char *pszSection,
                                         const sal_Char *pszEntry)
{
    sal_uInt32    NoEntry;
    osl_TProfileImpl*    pProfile = nullptr;
    osl_TProfileImpl*    pTmpProfile = nullptr;
    bool bRet = false;

    pTmpProfile = static_cast<osl_TProfileImpl*>(Profile);

    if ( pTmpProfile == nullptr )
    {
        return false;
    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( !pTmpProfile->m_bIsValid )
    {
        SAL_WARN_IF(!pTmpProfile->m_bIsValid, "sal.osl", "!pTmpProfile->m_bIsValid");
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
        return false;
    }

    pProfile = acquireProfile(Profile, true);

    if (pProfile == nullptr)
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return false;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        osl_TProfileSection* pSec;
        if (((pSec = findEntry(pProfile, pszSection, pszEntry, &NoEntry)) != nullptr) &&
            (NoEntry < pSec->m_NoEntries))
        {
            removeLine(pProfile, pSec->m_Entries[NoEntry].m_Line);
            removeEntry(pSec, NoEntry);
            if (pSec->m_NoEntries == 0)
            {
                removeLine(pProfile, pSec->m_Line);

                /* remove any empty separation line */
                if ((pSec->m_Line > 0) && (pProfile->m_Lines[pSec->m_Line - 1][0] == '\0'))
                    removeLine(pProfile, pSec->m_Line - 1);

                removeSection(pProfile, pSec);
            }

            pProfile->m_Flags |= FLG_MODIFIED;
        }
    }
    else
    { /* not implemented */ }

    bRet = releaseProfile(pProfile);
    SAL_WARN_IF(!bRet, "sal.osl", "releaseProfile(pProfile) ==> false");

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

    return bRet;
}

sal_uInt32 SAL_CALL osl_getProfileSectionEntries(oslProfile Profile,
                                                 const sal_Char *pszSection,
                                                 sal_Char* pszBuffer,
                                                 sal_uInt32 MaxLen)
{
    sal_uInt32    i, n = 0;
    sal_uInt32    NoEntry;
    osl_TProfileImpl*    pProfile = nullptr;
    osl_TProfileImpl*    pTmpProfile = nullptr;
    bool bRet = false;

    pTmpProfile = static_cast<osl_TProfileImpl*>(Profile);

    if ( pTmpProfile == nullptr )
    {
        return 0;

    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( !pTmpProfile->m_bIsValid )
    {
        SAL_WARN_IF(!pTmpProfile->m_bIsValid, "sal.osl", "!pTmpProfile->m_bIsValid"); 

        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return 0;
    }

    pProfile = acquireProfile(Profile, false);

    if (pProfile == nullptr)
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return 0;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        osl_TProfileSection* pSec;
        if ((pSec = findEntry(pProfile, pszSection, "", &NoEntry)) != nullptr)
        {
            if (MaxLen != 0)
            {
                for (i = 0; i < pSec->m_NoEntries; i++)
                {
                    if ((n + pSec->m_Entries[i].m_Len + 1) < MaxLen)
                    {
                        strncpy(&pszBuffer[n], &pProfile->m_Lines[pSec->m_Entries[i].m_Line]
                                [pSec->m_Entries[i].m_Offset], pSec->m_Entries[i].m_Len);
                        n += pSec->m_Entries[i].m_Len;
                        pszBuffer[n++] = '\0';
                    }
                    else
                        break;

                }

                pszBuffer[n++] = '\0';
            }
            else
            {
                for (i = 0; i < pSec->m_NoEntries; i++)
                    n += pSec->m_Entries[i].m_Len + 1;

                n += 1;
            }
        }
        else
            n = 0;
    }
    else {
        /* not implemented */
    }

    bRet=releaseProfile(pProfile);
    SAL_WARN_IF(!bRet, "sal.osl", "releaseProfile(pProfile) ==> false");

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

    return n;
}

sal_uInt32 SAL_CALL osl_getProfileSections(oslProfile Profile,
                                           sal_Char* pszBuffer,
                                           sal_uInt32 MaxLen)
{
    sal_uInt32    i, n = 0;
    osl_TProfileImpl*    pProfile = nullptr;
    osl_TProfileImpl*    pTmpProfile = nullptr;
    bool bRet = false;

    pTmpProfile = static_cast<osl_TProfileImpl*>(Profile);

    if ( pTmpProfile == nullptr )
    {
        return 0;
    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( !pTmpProfile->m_bIsValid )
    {
        SAL_WARN_IF(!pTmpProfile->m_bIsValid, "sal.osl", "!pTmpProfile->m_bIsValid"); 
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return 0;
    }

    pProfile = acquireProfile(Profile, false);

    if (pProfile == nullptr)
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

        return 0;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if (MaxLen != 0)
        {
            for (i = 0; i < pProfile->m_NoSections; i++)
            {
                osl_TProfileSection* pSec = &pProfile->m_Sections[i];

                if ((n + pSec->m_Len + 1) < MaxLen)
                {
                    strncpy(&pszBuffer[n], &pProfile->m_Lines[pSec->m_Line][pSec->m_Offset],
                            pSec->m_Len);
                    n += pSec->m_Len;
                    pszBuffer[n++] = '\0';
                }
                else
                    break;
            }

            pszBuffer[n++] = '\0';
        }
        else
        {
            for (i = 0; i < pProfile->m_NoSections; i++)
                n += pProfile->m_Sections[i].m_Len + 1;

            n += 1;
        }
    }
    else
    { /* not implemented */ }

    bRet=releaseProfile(pProfile);
    SAL_WARN_IF(!bRet, "sal.osl", "releaseProfile(pProfile) ==> false");

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

    return n;
}

static osl_TStamp OslProfile_getFileStamp(osl_TFile* pFile)
{
    struct stat status;

    if ( (pFile->m_Handle < 0) || (fstat(pFile->m_Handle, &status) < 0) )
    {
        return 0;
    }

    return status.st_mtime;
}

static bool OslProfile_lockFile(const osl_TFile* pFile, osl_TLockMode eMode)
{
    struct flock lock;
    static bool const bLockingDisabled = getenv( "STAR_PROFILE_LOCKING_DISABLED" ) != nullptr;

    if (pFile->m_Handle < 0)
    {
        return false;
    }

    if ( bLockingDisabled )
    {
        return true;
    }

    lock.l_start  = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len    = 0;

    switch (eMode)
    {
        case un_lock:
            lock.l_type = F_UNLCK;
            break;

        case read_lock:
            lock.l_type = F_RDLCK;
            break;

        case write_lock:
            lock.l_type = F_WRLCK;
            break;
    }

#ifndef MACOSX
    if ( fcntl(pFile->m_Handle, F_SETLKW, &lock) == -1 )
#else
    /* Mac OSX will return ENOTSUP for webdav drives so we should ignore it */
    if ( fcntl(pFile->m_Handle, F_SETLKW, &lock) == -1 && errno != ENOTSUP )
#endif
    {
        SAL_INFO("sal.osl", "fcntl failed: " << UnixErrnoString(errno));
        return false;
    }

    return true;
}

static osl_TFile* openFileImpl(const sal_Char* pszFilename, oslProfileOption ProfileFlags )
{
    int        Flags;
    osl_TFile* pFile = static_cast<osl_TFile*>(calloc(1, sizeof(osl_TFile)));
    bool       bWriteable = false;

    if ( ProfileFlags & ( osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE ) )
    {
        bWriteable = true;
    }

    if (! bWriteable)
    {
        pFile->m_Handle = open(pszFilename, O_RDONLY);

        if (pFile->m_Handle == -1)
        {
            int e = errno;
            SAL_INFO("sal.file", "open(" << pszFilename << ",O_RDONLY): " << UnixErrnoString(e));
        }
        else
            SAL_INFO("sal.file", "open(" << pszFilename << ",O_RDONLY) => " << pFile->m_Handle);

        /* mfe: argghh!!! do not check if the file could be opened */
        /*      default mode expects it that way!!!                 */
    }
    else
    {
        if (((pFile->m_Handle = open(pszFilename, O_RDWR | O_CREAT | O_EXCL, DEFAULT_PMODE)) < 0) &&
            ((pFile->m_Handle = open(pszFilename, O_RDWR)) < 0))
        {
            int e = errno;
            SAL_INFO("sal.file", "open(" << pszFilename << ",...): " << UnixErrnoString(e));
            free(pFile);
            return nullptr;
        }
        else
            SAL_INFO("sal.file", "open(" << pszFilename << ",...) => " << pFile->m_Handle);
    }

    /* set close-on-exec flag */
    if ((Flags = fcntl(pFile->m_Handle, F_GETFD, 0)) != -1)
    {
        Flags |= FD_CLOEXEC;
        int e = fcntl(pFile->m_Handle, F_SETFD, Flags);
        SAL_INFO_IF(
            e != 0, "sal.osl",
            "fcntl to set FD_CLOEXEC failed for " << pszFilename);
    }

    pFile->m_pWriteBuf=nullptr;
    pFile->m_nWriteBufFree=0;
    pFile->m_nWriteBufLen=0;

    if ( ProfileFlags & (osl_Profile_WRITELOCK | osl_Profile_READLOCK ) )
    {
        OslProfile_lockFile(pFile, bWriteable ? write_lock : read_lock);
    }

    return pFile;
}

static osl_TStamp closeFileImpl(osl_TFile* pFile, oslProfileOption Flags)
{
    osl_TStamp stamp = 0;

    if ( pFile == nullptr )
    {
        return stamp;
    }

    if ( pFile->m_Handle >= 0 )
    {
        stamp = OslProfile_getFileStamp(pFile);

        if ( Flags & (osl_Profile_WRITELOCK | osl_Profile_READLOCK ) )
        {
            OslProfile_lockFile(pFile, un_lock);
        }

        close(pFile->m_Handle);
        SAL_INFO("sal.file", "close(" << pFile->m_Handle << ")");
        pFile->m_Handle = -1;
    }

    if ( pFile->m_pWriteBuf )
    {
        free(pFile->m_pWriteBuf);
    }

    free(pFile);

    return stamp;
}

static bool OslProfile_rewindFile(osl_TFile* pFile, bool bTruncate)
{
    bool bRet = true;

    if (pFile->m_Handle >= 0)
    {
        pFile->m_pReadPtr  = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);

        bRet = (lseek(pFile->m_Handle, SEEK_SET, 0L) == 0);

        if (bTruncate)
        {
            bRet &= (ftruncate(pFile->m_Handle, 0L) == 0);
        }

    }

    return bRet;
}

static sal_Char* OslProfile_getLine(osl_TFile* pFile)
{
    int   Max, Free, nLineBytes = 0;
    sal_Char* pChr;
    sal_Char* pLine = nullptr;
    sal_Char* pNewLine;

    if ( pFile == nullptr )
    {
        return nullptr;
    }

    if (pFile->m_Handle < 0)
        return nullptr;

    do
    {
        int Bytes = sizeof(pFile->m_ReadBuf) - (pFile->m_pReadPtr - pFile->m_ReadBuf);

        if (Bytes <= 1)
        {
            /* refill buffer */
            memcpy(pFile->m_ReadBuf, pFile->m_pReadPtr, Bytes);
            pFile->m_pReadPtr = pFile->m_ReadBuf;

            Free = sizeof(pFile->m_ReadBuf) - Bytes;

            if ((Max = read(pFile->m_Handle, &pFile->m_ReadBuf[Bytes], Free)) < 0)
            {
                SAL_INFO("sal.osl", "read failed: " << UnixErrnoString(errno));

                if( pLine )
                    free( pLine );
                pLine = nullptr;
                break;
            }

            if (Max < Free)
            {
                 if ((Max == 0) && ! pLine)
                     break;

                 pFile->m_ReadBuf[Bytes + Max] = '\0';
            }
        }

        for (pChr = pFile->m_pReadPtr;
             (*pChr != '\n') && (*pChr != '\r') && (*pChr != '\0') &&
             (pChr < (pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf) - 1));
             pChr++);

        Max = pChr - pFile->m_pReadPtr;
        pNewLine = static_cast<sal_Char*>(malloc( nLineBytes + Max + 1 ));
        if( pLine )
        {
            memcpy( pNewLine, pLine, nLineBytes );
            free( pLine );
        }
        memcpy(pNewLine+nLineBytes, pFile->m_pReadPtr, Max);
        nLineBytes += Max;
        pNewLine[ nLineBytes ] = 0;
        pLine = pNewLine;

        if (pChr < (pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf) - 1))
        {
            if (*pChr != '\0')
            {
                if ((pChr[0] == '\r') && (pChr[1] == '\n'))
                    pChr += 2;
                else
                    pChr += 1;
            }

            if ((pChr < (pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf))) &&
                (*pChr == '\0'))
                pChr = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);

            /* setting Max to -1 indicates terminating read loop */
            Max = -1;
        }

        pFile->m_pReadPtr = pChr;
    }
    while (Max > 0) ;

    return pLine;
}

static bool OslProfile_putLine(osl_TFile* pFile, const sal_Char *pszLine)
{
    unsigned int Len = strlen(pszLine);

    if ( pFile == nullptr || pFile->m_Handle < 0 )
    {
        return false;
    }

    if ( pFile->m_pWriteBuf == nullptr )
    {
        pFile->m_pWriteBuf = static_cast<sal_Char*>(malloc(Len+3));
        pFile->m_nWriteBufLen = Len+3;
        pFile->m_nWriteBufFree = Len+3;
    }
    else
    {
        if ( pFile->m_nWriteBufFree <= Len + 3 )
        {
            sal_Char* pTmp;

            pTmp=static_cast<sal_Char*>(realloc(pFile->m_pWriteBuf,( ( pFile->m_nWriteBufLen + Len ) * 2) ));
            if ( pTmp == nullptr )
            {
                return false;
            }
            pFile->m_pWriteBuf = pTmp;
            pFile->m_nWriteBufFree = pFile->m_nWriteBufFree + pFile->m_nWriteBufLen + ( 2 * Len );
            pFile->m_nWriteBufLen = ( pFile->m_nWriteBufLen + Len ) * 2;
            memset( (pFile->m_pWriteBuf) + ( pFile->m_nWriteBufLen - pFile->m_nWriteBufFree ), 0, pFile->m_nWriteBufFree);
        }
    }

    memcpy(pFile->m_pWriteBuf + ( pFile->m_nWriteBufLen - pFile->m_nWriteBufFree ),pszLine,Len+1);
    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len]='\n';
    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len + 1]='\0';

    pFile->m_nWriteBufFree-=Len+1;

    return true;
}

static sal_Char* stripBlanks(sal_Char* String, sal_uInt32* pLen)
{
    if ( ( pLen != nullptr ) && ( *pLen != 0 ) )
    {
        while ((String[*pLen - 1] == ' ') || (String[*pLen - 1] == '\t'))
            (*pLen)--;

        while ( (*String == ' ') || (*String == '\t') )
        {
            String++;
            (*pLen)--;
        }
    }
    else
        while ( (*String == ' ') || (*String == '\t') )
            String++;

    return String;
}

static sal_Char* addLine(osl_TProfileImpl* pProfile, const sal_Char* Line)
{
    if (pProfile->m_NoLines >= pProfile->m_MaxLines)
    {
        if (pProfile->m_Lines == nullptr)
        {
            pProfile->m_MaxLines = LINES_INI;
            pProfile->m_Lines = static_cast<sal_Char **>(calloc(pProfile->m_MaxLines, sizeof(sal_Char *)));
        }
        else
        {
            unsigned int idx=0;
            unsigned int oldmax=pProfile->m_MaxLines;

            pProfile->m_MaxLines += LINES_ADD;
            pProfile->m_Lines = static_cast<sal_Char **>(realloc(pProfile->m_Lines,
                                                 pProfile->m_MaxLines * sizeof(sal_Char *)));
            for ( idx = oldmax ; idx < pProfile->m_MaxLines ; ++idx )
            {
                pProfile->m_Lines[idx]=nullptr;
            }
        }
    }
    if (pProfile->m_Lines == nullptr)
    {
        pProfile->m_NoLines  = 0;
        pProfile->m_MaxLines = 0;
        return nullptr;
    }

    if ( pProfile->m_Lines[pProfile->m_NoLines] != nullptr )
    {
        free(pProfile->m_Lines[pProfile->m_NoLines]);
    }
    pProfile->m_Lines[pProfile->m_NoLines++] = strdup(Line);

    return pProfile->m_Lines[pProfile->m_NoLines - 1];
}

static sal_Char* insertLine(osl_TProfileImpl* pProfile, const sal_Char* Line, sal_uInt32 LineNo)
{
    if (pProfile->m_NoLines >= pProfile->m_MaxLines)
    {
        if (pProfile->m_Lines == nullptr)
        {
            pProfile->m_MaxLines = LINES_INI;
            pProfile->m_Lines = static_cast<sal_Char **>(calloc(pProfile->m_MaxLines, sizeof(sal_Char *)));
        }
        else
        {
            pProfile->m_MaxLines += LINES_ADD;
            pProfile->m_Lines = static_cast<sal_Char **>(realloc(pProfile->m_Lines,
                                                 pProfile->m_MaxLines * sizeof(sal_Char *)));

            memset(&pProfile->m_Lines[pProfile->m_NoLines],
                0,
                (pProfile->m_MaxLines - pProfile->m_NoLines - 1) * sizeof(sal_Char*));
        }

        if (pProfile->m_Lines == nullptr)
        {
            pProfile->m_NoLines  = 0;
            pProfile->m_MaxLines = 0;
            return nullptr;
        }
    }

    LineNo = std::min(LineNo, pProfile->m_NoLines);

    if (LineNo < pProfile->m_NoLines)
    {
        sal_uInt32 i, n;

        memmove(&pProfile->m_Lines[LineNo + 1], &pProfile->m_Lines[LineNo],
                (pProfile->m_NoLines - LineNo) * sizeof(sal_Char *));

        /* adjust line references */
        for (i = 0; i < pProfile->m_NoSections; i++)
        {
            osl_TProfileSection* pSec = &pProfile->m_Sections[i];

            if (pSec->m_Line >= LineNo)
                pSec->m_Line++;

            for (n = 0; n < pSec->m_NoEntries; n++)
                if (pSec->m_Entries[n].m_Line >= LineNo)
                    pSec->m_Entries[n].m_Line++;
        }
    }

    pProfile->m_NoLines++;

    pProfile->m_Lines[LineNo] = strdup(Line);

    return pProfile->m_Lines[LineNo];
}

static void removeLine(osl_TProfileImpl* pProfile, sal_uInt32 LineNo)
{
    if (LineNo < pProfile->m_NoLines)
    {
        free(pProfile->m_Lines[LineNo]);
        pProfile->m_Lines[LineNo]=nullptr;
        if (pProfile->m_NoLines - LineNo > 1)
        {
            sal_uInt32 i, n;

            memmove(&pProfile->m_Lines[LineNo], &pProfile->m_Lines[LineNo + 1],
                    (pProfile->m_NoLines - LineNo - 1) * sizeof(sal_Char *));

            memset(&pProfile->m_Lines[pProfile->m_NoLines - 1],
                0,
                (pProfile->m_MaxLines - pProfile->m_NoLines) * sizeof(sal_Char*));

            /* adjust line references */
            for (i = 0; i < pProfile->m_NoSections; i++)
            {
                osl_TProfileSection* pSec = &pProfile->m_Sections[i];

                if (pSec->m_Line > LineNo)
                    pSec->m_Line--;

                for (n = 0; n < pSec->m_NoEntries; n++)
                    if (pSec->m_Entries[n].m_Line > LineNo)
                        pSec->m_Entries[n].m_Line--;
            }
        }
        else
        {
            pProfile->m_Lines[LineNo] = nullptr;
        }

        pProfile->m_NoLines--;
    }
}

static void setEntry(osl_TProfileImpl* pProfile, osl_TProfileSection* pSection,
                     sal_uInt32 NoEntry, sal_uInt32 Line,
                     sal_Char* Entry, sal_uInt32 Len)
{
    Entry = stripBlanks(Entry, &Len);
    pSection->m_Entries[NoEntry].m_Line   = Line;
    pSection->m_Entries[NoEntry].m_Offset = Entry - pProfile->m_Lines[Line];
    pSection->m_Entries[NoEntry].m_Len    = Len;
}

static bool addEntry(osl_TProfileImpl* pProfile,
                     osl_TProfileSection *pSection,
                     int Line, sal_Char* Entry,
                     sal_uInt32 Len)
{
    if (pSection != nullptr)
    {
        if (pSection->m_NoEntries >= pSection->m_MaxEntries)
        {
            if (pSection->m_Entries == nullptr)
            {
                pSection->m_MaxEntries = ENTRIES_INI;
                pSection->m_Entries = static_cast<osl_TProfileEntry *>(malloc(
                                pSection->m_MaxEntries * sizeof(osl_TProfileEntry)));
            }
            else
            {
                pSection->m_MaxEntries += ENTRIES_ADD;
                pSection->m_Entries = static_cast<osl_TProfileEntry *>(realloc(pSection->m_Entries,
                                pSection->m_MaxEntries * sizeof(osl_TProfileEntry)));
            }

            if (pSection->m_Entries == nullptr)
            {
                pSection->m_NoEntries  = 0;
                pSection->m_MaxEntries = 0;
                return false;
            }
        }

        pSection->m_NoEntries++;

        Entry = stripBlanks(Entry, &Len);
        setEntry(pProfile, pSection, pSection->m_NoEntries - 1, Line,
                 Entry, Len);

        return true;
    }

    return false;
}

static void removeEntry(osl_TProfileSection *pSection, sal_uInt32 NoEntry)
{
    if (NoEntry < pSection->m_NoEntries)
    {
        if (pSection->m_NoEntries - NoEntry > 1)
        {
            memmove(&pSection->m_Entries[NoEntry],
                    &pSection->m_Entries[NoEntry + 1],
                    (pSection->m_NoEntries - NoEntry - 1) * sizeof(osl_TProfileEntry));
            pSection->m_Entries[pSection->m_NoEntries - 1].m_Line=0;
            pSection->m_Entries[pSection->m_NoEntries - 1].m_Offset=0;
            pSection->m_Entries[pSection->m_NoEntries - 1].m_Len=0;
        }

        pSection->m_NoEntries--;
    }

}

static bool addSection(osl_TProfileImpl* pProfile, int Line, const sal_Char* Section, sal_uInt32 Len)
{
    if (pProfile->m_NoSections >= pProfile->m_MaxSections)
    {
        if (pProfile->m_Sections == nullptr)
        {
            pProfile->m_MaxSections = SECTIONS_INI;
            pProfile->m_Sections = static_cast<osl_TProfileSection *>(calloc(pProfile->m_MaxSections, sizeof(osl_TProfileSection)));
        }
        else
        {
            unsigned int idx=0;
            unsigned int oldmax=pProfile->m_MaxSections;

            pProfile->m_MaxSections += SECTIONS_ADD;
            pProfile->m_Sections = static_cast<osl_TProfileSection *>(realloc(pProfile->m_Sections,
                                          pProfile->m_MaxSections * sizeof(osl_TProfileSection)));
            for ( idx = oldmax ; idx < pProfile->m_MaxSections ; ++idx )
            {
                pProfile->m_Sections[idx].m_Entries=nullptr;
            }
        }

        if (pProfile->m_Sections == nullptr)
        {
            pProfile->m_NoSections = 0;
            pProfile->m_MaxSections = 0;
            return false;
        }
    }

    pProfile->m_NoSections++;

    if ( pProfile->m_Sections[(pProfile->m_NoSections) - 1].m_Entries != nullptr )
    {
         free(pProfile->m_Sections[(pProfile->m_NoSections) - 1].m_Entries);
    }
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Entries    = nullptr;
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_NoEntries  = 0;
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_MaxEntries = 0;

    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Line = Line;
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Offset = Section - pProfile->m_Lines[Line];
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Len = Len;

    return true;
}

static void removeSection(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection)
{
    sal_uInt32 Section;

    if ((Section = pSection - pProfile->m_Sections) < pProfile->m_NoSections)
    {
        free (pSection->m_Entries);
        pSection->m_Entries=nullptr;
        if (pProfile->m_NoSections - Section > 1)
        {
            memmove(&pProfile->m_Sections[Section], &pProfile->m_Sections[Section + 1],
                    (pProfile->m_NoSections - Section - 1) * sizeof(osl_TProfileSection));

            memset(&pProfile->m_Sections[pProfile->m_NoSections - 1],
                   0,
                   (pProfile->m_MaxSections - pProfile->m_NoSections) * sizeof(osl_TProfileSection));
            pProfile->m_Sections[pProfile->m_NoSections - 1].m_Entries = nullptr;
        }
        else
        {
            pSection->m_Entries = nullptr;
        }

        pProfile->m_NoSections--;
    }
}

static osl_TProfileSection* findEntry(osl_TProfileImpl* pProfile,
                                      const sal_Char* Section,
                                      const sal_Char* Entry,
                                      sal_uInt32 *pNoEntry)
{
    static  sal_uInt32    Sect = 0;
    sal_uInt32    i, n;
    sal_uInt32  Len;
    osl_TProfileSection* pSec=nullptr;

    Len = strlen(Section);

    n = Sect;

    for (i = 0; i < pProfile->m_NoSections; i++)
    {
        n %= pProfile->m_NoSections;
        pSec = &pProfile->m_Sections[n];
        if ((Len == pSec->m_Len) &&
            (strncasecmp(Section, &pProfile->m_Lines[pSec->m_Line][pSec->m_Offset], pSec->m_Len)
             == 0))
            break;
        n++;
    }

    Sect = n;

    if (i < pProfile->m_NoSections)
    {
        Len = strlen(Entry);

        *pNoEntry = pSec->m_NoEntries;

        for (i = 0; i < pSec->m_NoEntries; i++)
        {
            const sal_Char* pStr = &pProfile->m_Lines[pSec->m_Entries[i].m_Line]
                                     [pSec->m_Entries[i].m_Offset];
            if ((Len == pSec->m_Entries[i].m_Len) &&
                (strncasecmp(Entry, pStr, pSec->m_Entries[i].m_Len)
                 == 0))
            {
                *pNoEntry = i;
                break;
            }
        }
    }
    else
        pSec = nullptr;

    return pSec;
}

static bool loadProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile)
{
    sal_uInt32  i;
    sal_Char*   pStr;
    sal_Char*   pChar;

    sal_Char* pLine;

    if ( !pFile )
    {
        return false;
    }

    if ( !pProfile )
    {
        return false;
    }

    pProfile->m_NoLines    = 0;
    pProfile->m_NoSections = 0;

    OSL_VERIFY(OslProfile_rewindFile(pFile, false));

    while ( ( pLine=OslProfile_getLine(pFile) ) != nullptr )
    {
        sal_Char* bWasAdded = addLine( pProfile, pLine );
        free( pLine );
        SAL_WARN_IF(!bWasAdded, "sal.osl", "addLine( pProfile, pLine ) ==> false");
        if ( ! bWasAdded )
            return false;
    }

    for (i = 0; i < pProfile->m_NoLines; i++)
    {
        pStr = stripBlanks(pProfile->m_Lines[i], nullptr);

        if ((*pStr == '\0') || (*pStr == ';'))
            continue;

        if ((*pStr != '[') || ((pChar = strrchr(pStr, ']')) == nullptr) ||
            ((pChar - pStr) <= 2))
        {
            /* insert entry */

            if (pProfile->m_NoSections < 1)
                continue;

            if ((pChar = strchr(pStr, '=')) == nullptr)
                pChar = pStr + strlen(pStr);

            if (! addEntry(pProfile, &pProfile->m_Sections[pProfile->m_NoSections - 1],
                           i, pStr, pChar - pStr))
            {
                SAL_WARN("sal.osl", "Adding entry => false");
                continue;
            }

        }
        else
        {
            /* new section */

            if (! addSection(pProfile, i, pStr + 1, pChar - pStr - 1))
            {
                SAL_WARN("sal.osl", "Adding section => false");
                continue;
            }

        }
    }

    return true;
}

static bool storeProfile(osl_TProfileImpl* pProfile, bool bCleanup)
{
    if (pProfile->m_Lines != nullptr)
    {
        if (pProfile->m_Flags & FLG_MODIFIED)
        {
            sal_uInt32 i;

            osl_TFile* pTmpFile = osl_openTmpProfileImpl(pProfile);

            if ( pTmpFile == nullptr )
            {
                return false;
            }

            OSL_VERIFY(OslProfile_rewindFile(pTmpFile, true));

            for ( i = 0 ; i < pProfile->m_NoLines ; i++ )
            {
                OSL_VERIFY(OslProfile_putLine(pTmpFile, pProfile->m_Lines[i]));
            }

            if ( ! writeProfileImpl(pTmpFile) )
            {
                if ( pTmpFile->m_pWriteBuf != nullptr )
                {
                    free(pTmpFile->m_pWriteBuf);
                }

                pTmpFile->m_pWriteBuf=nullptr;
                pTmpFile->m_nWriteBufLen=0;
                pTmpFile->m_nWriteBufFree=0;

                closeFileImpl(pTmpFile,pProfile->m_Flags);

                return false;
            }

            pProfile->m_Flags &= ~FLG_MODIFIED;

            closeFileImpl(pProfile->m_pFile,pProfile->m_Flags);
            closeFileImpl(pTmpFile,pProfile->m_Flags);

            osl_ProfileSwapProfileNames(pProfile);

            pProfile->m_pFile = openFileImpl(pProfile->m_FileName,pProfile->m_Flags);

        }

        if (bCleanup)
        {
            while (pProfile->m_NoLines > 0)
                removeLine(pProfile, pProfile->m_NoLines - 1);

            free(pProfile->m_Lines);
            pProfile->m_Lines = nullptr;
            pProfile->m_NoLines = 0;
            pProfile->m_MaxLines = 0;

            while (pProfile->m_NoSections > 0)
                removeSection(pProfile, &pProfile->m_Sections[pProfile->m_NoSections - 1]);

            free(pProfile->m_Sections);
            pProfile->m_Sections = nullptr;
            pProfile->m_NoSections = 0;
            pProfile->m_MaxSections = 0;
        }
    }

    return true;
}

static osl_TFile* osl_openTmpProfileImpl(osl_TProfileImpl* pProfile)
{
    osl_TFile* pFile=nullptr;
    sal_Char const * const pszExtension = "tmp";
    sal_Char pszTmpName[PATH_MAX];
    oslProfileOption PFlags=0;

    pszTmpName[0] = '\0';

    /* generate tmp profilename */
    osl_ProfileGenerateExtension(pProfile->m_FileName, pszExtension, pszTmpName, PATH_MAX);

    if ( pszTmpName[0] == 0 )
    {
        return nullptr;
    }

    if ( ! ( pProfile->m_Flags & osl_Profile_READLOCK ) )
    {
        PFlags |= osl_Profile_WRITELOCK;
    }

    /* open this file */
    pFile = openFileImpl(pszTmpName,pProfile->m_Flags | PFlags);

    /* return new pFile */
    return pFile;
}

static bool osl_ProfileSwapProfileNames(osl_TProfileImpl* pProfile)
{
    sal_Char pszBakFile[PATH_MAX];
    sal_Char pszTmpFile[PATH_MAX];

    pszBakFile[0] = '\0';
    pszTmpFile[0] = '\0';

    osl_ProfileGenerateExtension(pProfile->m_FileName, "bak", pszBakFile, PATH_MAX);
    osl_ProfileGenerateExtension(pProfile->m_FileName, "tmp", pszTmpFile, PATH_MAX);

    /* unlink bak */
    unlink( pszBakFile );

    // Rename ini -> bak, then tmp -> ini:
    bool result = rename( pProfile->m_FileName, pszBakFile ) == 0;
    if (!result)
    {
        int e = errno;
        SAL_INFO("sal.file", "rename(" << pProfile->m_FileName << "," << pszBakFile << "): " << UnixErrnoString(e));
    }
    else
    {
        SAL_INFO("sal.file", "rename(" << pProfile->m_FileName << "," << pszBakFile << "): OK");
        result = rename( pszTmpFile, pProfile->m_FileName ) == 0;
        if (!result)
        {
            int e = errno;
            SAL_INFO("sal.file", "rename(" << pszTmpFile << "," << pProfile->m_FileName << "): " << UnixErrnoString(e));
        }
        else
        {
            SAL_INFO("sal.file", "rename(" << pszTmpFile << "," << pProfile->m_FileName << "): OK");
        }
    }
    return result;
}

static void osl_ProfileGenerateExtension(const sal_Char* pszFileName, const sal_Char* pszExtension, sal_Char* pszTmpName, int BufferMaxLen)
{
    sal_Char* cursor = pszTmpName;
    int len;

    /* concatenate filename + "." + extension, limited to the size of the
     * output buffer; in case of overrun, data is truncated at the end...
     * and the result is always 0-terminated.
     */
    len = strlen(pszFileName);
    if(len < BufferMaxLen)
    {
        memcpy(cursor, pszFileName, len);
        cursor += len;
        BufferMaxLen -= len;
    }
    else
    {
        memcpy(cursor, pszFileName, BufferMaxLen - 1);
        cursor += BufferMaxLen - 1;
        BufferMaxLen = 1;
    }
    if(BufferMaxLen > 1)
    {
        *cursor++ = '.';
        BufferMaxLen -= 1;
    }
    len = strlen(pszExtension);
    if(len < BufferMaxLen)
    {
        memcpy(cursor, pszExtension, len);
        cursor += len;
    }
    else
    {
        memcpy(cursor, pszExtension, BufferMaxLen - 1);
        cursor += BufferMaxLen - 1;
    }
    *cursor = 0;
}

static osl_TProfileImpl* acquireProfile(oslProfile Profile, bool bWriteable)
{
    osl_TProfileImpl* pProfile = static_cast<osl_TProfileImpl*>(Profile);
    oslProfileOption PFlags=0;

    if ( bWriteable )
    {
        PFlags = osl_Profile_DEFAULT | osl_Profile_WRITELOCK;
    }
    else
    {
        PFlags = osl_Profile_DEFAULT;
    }

    if (pProfile == nullptr)
    {
        if ( ( pProfile = static_cast<osl_TProfileImpl*>(osl_openProfile(nullptr, PFlags )) ) != nullptr )
        {
            pProfile->m_Flags |= FLG_AUTOOPEN;
        }
    }
    else
    {
        if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
        {
            if (! (pProfile->m_Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE )))
            {
                osl_TStamp Stamp;

                if (! (pProfile->m_pFile = openFileImpl(pProfile->m_FileName, pProfile->m_Flags | PFlags )))
                    return nullptr;

                Stamp = OslProfile_getFileStamp(pProfile->m_pFile);

                if (memcmp(&Stamp, &(pProfile->m_Stamp), sizeof(osl_TStamp)))
                {
                    pProfile->m_Stamp = Stamp;
                    bool bRet = loadProfile(pProfile->m_pFile, pProfile);
                    SAL_WARN_IF(!bRet, "sal.osl", "loadProfile(pProfile->m_pFile, pProfile) ==> false");
                }
            }
            else
            {
                /* A readlock file could not be written */
                if ((pProfile->m_Flags & osl_Profile_READLOCK) && bWriteable)
                {
                    return nullptr;
                }
            }
        }
    }

    return pProfile;
}

static bool releaseProfile(osl_TProfileImpl* pProfile)
{
    if ( pProfile == nullptr )
    {
        return false;
    }

    if (pProfile->m_Flags & FLG_AUTOOPEN)
    {
        return osl_closeProfile(static_cast<oslProfile>(pProfile));
    }

    if (! (pProfile->m_Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE )))
    {
        if (pProfile->m_Flags & FLG_MODIFIED)
        {
            bool bRet = storeProfile(pProfile, false);
            SAL_WARN_IF(!bRet, "sal.osl", "storeProfile(pProfile, false) ==> false");
        }

        closeFileImpl(pProfile->m_pFile,pProfile->m_Flags);
        pProfile->m_pFile = nullptr;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
