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

#include "system.h"

#include "file_url.hxx"
#include "path_helper.hxx"

#include <string.h>
#include <osl/diagnose.h>
#include <osl/profile.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <rtl/alloc.h>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <algorithm>
#include <vector>
using std::min;
static void copy_ustr_n( void *dest, const void *source, size_t length ) { memcpy(dest, source, length*sizeof(sal_Unicode)); }

#define LINES_INI       32
#define LINES_ADD       10
#define SECTIONS_INI    5
#define SECTIONS_ADD    3
#define ENTRIES_INI     5
#define ENTRIES_ADD     3

#define STR_INI_EXTENSION   L".ini"
#define STR_INI_METAHOME    "?~"
#define STR_INI_METASYS     "?$"
#define STR_INI_METACFG     "?^"
#define STR_INI_METAINS     "?#"

#define STR_INI_BOOLYES     "yes"
#define STR_INI_BOOLON      "on"
#define STR_INI_BOOLONE     "1"
#define STR_INI_BOOLNO      "no"
#define STR_INI_BOOLOFF     "off"
#define STR_INI_BOOLZERO    "0"

#define FLG_USER            0x00FF
#define FLG_AUTOOPEN        0x0100
#define FLG_MODIFIED        0x0200

#define SVERSION_LOCATION   STR_INI_METACFG
#define SVERSION_FALLBACK   STR_INI_METASYS
#define SVERSION_NAME       "sversion"
#define SVERSION_SECTION    "Versions"
#define SVERSION_SOFFICE    "StarOffice"
#define SVERSION_PROFILE    "soffice.ini"
#define SVERSION_DIRS       { "bin", "program" }
#define SVERSION_USER       "user"

/*#define DEBUG_OSL_PROFILE 1*/

typedef FILETIME osl_TStamp;

enum osl_TLockMode
{
    un_lock, read_lock, write_lock
};

struct osl_TFile
{
    HANDLE  m_Handle;
    sal_Char*   m_pReadPtr;
    sal_Char    m_ReadBuf[512];
    sal_Char*   m_pWriteBuf;
    sal_uInt32  m_nWriteBufLen;
    sal_uInt32  m_nWriteBufFree;
};

struct osl_TProfileEntry
{
    sal_uInt32      m_Line;
    sal_uInt32      m_Offset;
    sal_uInt32      m_Len;
};

struct osl_TProfileSection
{
    sal_uInt32          m_Line;
    sal_uInt32          m_Offset;
    sal_uInt32          m_Len;
    sal_uInt32          m_NoEntries;
    sal_uInt32          m_MaxEntries;
    osl_TProfileEntry*  m_Entries;
};

/*
    Profile-data structure hidden behind oslProfile:
*/
struct osl_TProfileImpl
{
    sal_uInt32  m_Flags;
    osl_TFile*  m_pFile;
    osl_TStamp  m_Stamp;
    sal_uInt32  m_NoLines;
    sal_uInt32  m_MaxLines;
    sal_uInt32  m_NoSections;
    sal_uInt32  m_MaxSections;
    sal_Char**  m_Lines;
    rtl_uString *m_strFileName;
    osl_TProfileSection* m_Sections;
};

static osl_TFile*           openFileImpl(rtl_uString * strFileName, oslProfileOption ProfileFlags  );
static osl_TStamp           closeFileImpl(osl_TFile* pFile);
static bool                 lockFile(const osl_TFile* pFile, osl_TLockMode eMode);
static bool                 rewindFile(osl_TFile* pFile, bool bTruncate);
static osl_TStamp           getFileStamp(osl_TFile* pFile);

static bool                 getLine(osl_TFile* pFile, sal_Char *pszLine, int MaxLen);
static bool                 putLine(osl_TFile* pFile, const sal_Char *pszLine);
static const sal_Char*      stripBlanks(const sal_Char* String, sal_uInt32* pLen);
static const sal_Char*      addLine(osl_TProfileImpl* pProfile, const sal_Char* Line);
static const sal_Char*      insertLine(osl_TProfileImpl* pProfile, const sal_Char* Line, sal_uInt32 LineNo);
static void                 removeLine(osl_TProfileImpl* pProfile, sal_uInt32 LineNo);
static void                 setEntry(osl_TProfileImpl* pProfile, osl_TProfileSection* pSection,
                                     sal_uInt32 NoEntry, sal_uInt32 Line,
                                     const sal_Char* Entry, sal_uInt32 Len);
static bool                 addEntry(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection,
                                     int Line, const sal_Char* Entry, sal_uInt32 Len);
static void                 removeEntry(osl_TProfileSection *pSection, sal_uInt32 NoEntry);
static bool                 addSection(osl_TProfileImpl* pProfile, int Line, const sal_Char* Section, sal_uInt32 Len);
static void                 removeSection(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection);
static osl_TProfileSection* findEntry(osl_TProfileImpl* pProfile, const sal_Char* Section,
                                      const sal_Char* Entry, sal_uInt32 *pNoEntry);
static bool                 loadProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile);
static bool                 storeProfile(osl_TProfileImpl* pProfile, bool bCleanup);
static osl_TProfileImpl*    acquireProfile(oslProfile Profile, bool bWriteable);
static bool                 releaseProfile(osl_TProfileImpl* pProfile);
static bool                 lookupProfile(const sal_Unicode *strPath, const sal_Unicode *strFile, sal_Unicode *strProfile);

static bool writeProfileImpl (osl_TFile* pFile);
static osl_TFile* osl_openTmpProfileImpl(osl_TProfileImpl*);
static bool osl_ProfileSwapProfileNames(osl_TProfileImpl*);
static rtl_uString* osl_ProfileGenerateExtension(rtl_uString* ustrFileName, rtl_uString* ustrExtension);

static bool osl_getProfileName(rtl_uString* strPath, rtl_uString* strName, rtl_uString** strProfileName);

oslProfile SAL_CALL osl_openProfile(rtl_uString *strProfileName, sal_uInt32 Flags)
{
    osl_TFile*        pFile = nullptr;
    osl_TProfileImpl* pProfile;
    rtl_uString       *FileName=nullptr;

    OSL_VERIFY(strProfileName);

    if (rtl_uString_getLength(strProfileName) == 0 )
    {
        OSL_VERIFY(osl_getProfileName(nullptr, nullptr, &FileName));
    }
    else
    {
        rtl_uString_assign(&FileName, strProfileName);
    }

    osl_getSystemPathFromFileURL(FileName, &FileName);

#ifdef DEBUG_OSL_PROFILE
    Flags=osl_Profile_FLUSHWRITE;

    if ( Flags == osl_Profile_DEFAULT )
    {
        SAL_INFO("sal.osl", "with osl_Profile_DEFAULT");
    }
    if ( Flags & osl_Profile_SYSTEM )
    {
        SAL_INFO("sal.osl", "with osl_Profile_SYSTEM");
    }
    if ( Flags & osl_Profile_READLOCK )
    {
        SAL_INFO("sal.osl", "with osl_Profile_READLOCK");
    }
    if ( Flags & osl_Profile_WRITELOCK )
    {
        SAL_INFO("sal.osl", "with osl_Profile_WRITELOCK");
    }
    if ( Flags & osl_Profile_FLUSHWRITE )
    {
        SAL_INFO("sal.osl", "with osl_Profile_FLUSHWRITE");
    }
#endif

    if ( (! (Flags & osl_Profile_SYSTEM)) && ( (pFile = openFileImpl(FileName, Flags) ) == nullptr ) )
    {
        if( FileName)
            rtl_uString_release( FileName);

        return nullptr;
    }

    pProfile = static_cast<osl_TProfileImpl*>(calloc(1, sizeof(osl_TProfileImpl)));
    if (!pProfile)
        return nullptr;

    pProfile->m_Flags = Flags & FLG_USER;
    osl_getSystemPathFromFileURL(strProfileName, &pProfile->m_strFileName);

    if (Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE ))
        pProfile->m_pFile = pFile;

    pProfile->m_Stamp = getFileStamp(pFile);

    loadProfile(pFile, pProfile);

    if (pProfile->m_pFile == nullptr)
        closeFileImpl(pFile);

    if( FileName)
        rtl_uString_release( FileName);

    return pProfile;
}

sal_Bool SAL_CALL osl_closeProfile(oslProfile Profile)
{
    osl_TProfileImpl* pProfile = static_cast<osl_TProfileImpl*>(Profile);


    if ( Profile == nullptr )
    {
        return false;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        pProfile = acquireProfile(Profile,true);

        if ( pProfile != nullptr )
        {
            if ( !( pProfile->m_Flags & osl_Profile_READLOCK )  && ( pProfile->m_Flags & FLG_MODIFIED ) )
            {
                storeProfile(pProfile, false);
            }
        }
        else
        {
            pProfile = acquireProfile(Profile,false);
        }

        if ( pProfile == nullptr )
        {
            return false;
        }

        if (pProfile->m_pFile != nullptr)
            closeFileImpl(pProfile->m_pFile);
    }

    pProfile->m_pFile = nullptr;
    rtl_uString_release(pProfile->m_strFileName);
    pProfile->m_strFileName = nullptr;

    /* release whole profile data types memory */
    if ( pProfile->m_NoLines > 0)
    {
        unsigned int index=0;
        if ( pProfile->m_Lines != nullptr )
        {
            for ( index = 0 ; index < pProfile->m_NoLines ; ++index)
            {
                if ( pProfile->m_Lines[index] != nullptr )
                {
                    free(pProfile->m_Lines[index]);
                }
            }
            free(pProfile->m_Lines);
        }
        if ( pProfile->m_Sections != nullptr )
        {
            for ( index = 0 ; index < pProfile->m_NoSections ; ++index )
            {
                if ( pProfile->m_Sections[index].m_Entries != nullptr )
                    free(pProfile->m_Sections[index].m_Entries);
            }
            free(pProfile->m_Sections);
        }

    }
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

    pFile = pProfile->m_pFile;
    if ( pFile == nullptr || pFile->m_Handle == INVALID_HANDLE_VALUE )
    {
        return false;
    }

    if ( pProfile->m_Flags & FLG_MODIFIED )
    {
#ifdef DEBUG_OSL_PROFILE
        SAL_INFO("sal.osl", "swapping to storeprofile");
#endif
        bRet = storeProfile(pProfile,false);
    }

    return bRet;
}

static bool writeProfileImpl(osl_TFile* pFile)
{
    DWORD BytesWritten=0;
    BOOL bRet;

    if ( pFile == nullptr || pFile->m_Handle == INVALID_HANDLE_VALUE || ( pFile->m_pWriteBuf == nullptr ) )
    {
        return false;
    }

    bRet=WriteFile(pFile->m_Handle, pFile->m_pWriteBuf, pFile->m_nWriteBufLen - pFile->m_nWriteBufFree,&BytesWritten,nullptr);

    if ( bRet == 0 || BytesWritten == 0 )
    {
        OSL_ENSURE(bRet,"WriteFile failed!!!");
        SAL_WARN("sal.osl", "write failed " << strerror(errno));

        return false;
    }

    free(pFile->m_pWriteBuf);
    pFile->m_pWriteBuf=nullptr;
    pFile->m_nWriteBufLen=0;
    pFile->m_nWriteBufFree=0;

    return true;
}

namespace {
// Use Unicode version of GetPrivateProfileString, to work with Multi-language paths
DWORD GetPrivateProfileStringWrapper(const osl_TProfileImpl* pProfile,
    const sal_Char* pszSection, const sal_Char* pszEntry,
    sal_Char* pszString, sal_uInt32 MaxLen,
    const sal_Char* pszDefault)
{
    OSL_ASSERT(pProfile && (!MaxLen || pszString));

    rtl_uString *pSection = nullptr, *pEntry = nullptr, *pDefault = nullptr;
    if (pszSection)
    {
        rtl_string2UString(&pSection, pszSection, strlen(pszSection), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);
        OSL_ASSERT(pSection);
    }
    if (pszEntry)
    {
        rtl_string2UString(&pEntry, pszEntry, strlen(pszEntry), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);
        OSL_ASSERT(pEntry);
    }
    if (pszDefault)
    {
        rtl_string2UString(&pDefault, pszDefault, strlen(pszDefault), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);
        OSL_ASSERT(pDefault);
    }

    LPCWSTR pWSection = (pSection ? o3tl::toW(rtl_uString_getStr(pSection)) : nullptr),
            pWEntry   = (pEntry   ? o3tl::toW(rtl_uString_getStr(pEntry))   : nullptr),
            pWDefault = (pDefault ? o3tl::toW(rtl_uString_getStr(pDefault)) : nullptr);

    std::vector<wchar_t> aBuf(MaxLen + 1);
    GetPrivateProfileStringW(pWSection, pWEntry, pWDefault, &aBuf[0], MaxLen, o3tl::toW(rtl_uString_getStr(pProfile->m_strFileName)));

    if (pDefault)
        rtl_uString_release(pDefault);
    if (pEntry)
        rtl_uString_release(pEntry);
    if (pSection)
        rtl_uString_release(pSection);

    return WideCharToMultiByte(CP_ACP, 0, &aBuf[0], -1, pszString, MaxLen, nullptr, nullptr);
}

// Use Unicode version of WritePrivateProfileString, to work with Multi-language paths
BOOL WritePrivateProfileStringWrapper(const osl_TProfileImpl* pProfile,
    const sal_Char* pszSection, const sal_Char* pszEntry,
    const sal_Char* pszString)
{
    OSL_ASSERT(pProfile && pszSection);
    rtl_uString *pSection, *pEntry = nullptr, *pString = nullptr;
    rtl_string2UString(&pSection, pszSection, strlen(pszSection), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);
    OSL_ASSERT(pSection);
    if (pszEntry)
    {
        rtl_string2UString(&pEntry, pszEntry, strlen(pszEntry), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);
        OSL_ASSERT(pEntry);
    }
    if (pszString)
    {
        rtl_string2UString(&pString, pszString, strlen(pszString), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);
        OSL_ASSERT(pString);
    }

    LPCWSTR pWSection = o3tl::toW(pSection->buffer),
            pWEntry   = (pEntry   ? o3tl::toW(rtl_uString_getStr(pEntry))   : nullptr),
            pWString  = (pString  ? o3tl::toW(rtl_uString_getStr(pString))  : nullptr);

    BOOL bResult = WritePrivateProfileStringW(pWSection, pWEntry, pWString, o3tl::toW(rtl_uString_getStr(pProfile->m_strFileName)));

    if (pString)
        rtl_uString_release(pString);
    if (pEntry)
        rtl_uString_release(pEntry);
    rtl_uString_release(pSection);

    return bResult;
}
}

sal_Bool SAL_CALL osl_readProfileString(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_Char* pszString, sal_uInt32 MaxLen,
                              const sal_Char* pszDefault)
{
    sal_uInt32    NoEntry;
    const sal_Char* pStr = nullptr;
    osl_TProfileImpl*    pProfile = nullptr;

    pProfile = acquireProfile(Profile, false);

    if (pProfile == nullptr)
    {
        return false;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        osl_TProfileSection* pSec;
        if (((pSec = findEntry(pProfile, pszSection, pszEntry, &NoEntry)) != nullptr) &&
            (NoEntry < pSec->m_NoEntries) &&
            ((pStr = strchr(pProfile->m_Lines[pSec->m_Entries[NoEntry].m_Line],
                            '=')) != nullptr))
            pStr++;
        else
            pStr = pszDefault;

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
    {
        if (GetPrivateProfileStringWrapper(pProfile, pszSection, pszEntry, pszString, MaxLen, pszDefault) > 0)
            pStr = pszString; // required to return true below
    }

    releaseProfile(pProfile);

    if ( pStr == nullptr )
    {
        return false;
    }

    return true;
}

sal_Bool SAL_CALL osl_readProfileBool(oslProfile Profile,
                            const sal_Char* pszSection, const sal_Char* pszEntry,
                            sal_Bool Default)
{
    sal_Char Line[32];

    if (osl_readProfileString(Profile, pszSection, pszEntry, Line, sizeof(Line), ""))
    {
        if ((stricmp(Line, STR_INI_BOOLYES) == 0) ||
            (stricmp(Line, STR_INI_BOOLON)  == 0) ||
            (stricmp(Line, STR_INI_BOOLONE) == 0))
            Default = true;
        else
            if ((stricmp(Line, STR_INI_BOOLNO)   == 0) ||
                (stricmp(Line, STR_INI_BOOLOFF)  == 0) ||
                (stricmp(Line, STR_INI_BOOLZERO) == 0))
                Default = false;
    }

    return Default;
}

sal_uInt32 SAL_CALL osl_readProfileIdent(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Default)
{
    sal_uInt32    i;
    sal_Char        Line[256];

    if (osl_readProfileString(Profile, pszSection, pszEntry, Line, sizeof(Line), ""))
    {
        i = 0;
        while (Strings[i] != nullptr)
        {
            if (stricmp(Line, Strings[i]) == 0)
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
                               const sal_Char* pszSection, const sal_Char* pszEntry,
                               const sal_Char* pszString)
{
    sal_uInt32    i;
    bool bRet = false;
    sal_uInt32    NoEntry;
    const sal_Char* pStr;
    sal_Char        Line[4096];
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = nullptr;

    pProfile = acquireProfile(Profile, true);

    if (pProfile == nullptr)
    {
        return false;
    }

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
                releaseProfile(pProfile);
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
                releaseProfile(pProfile);
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
    else
    {
        WritePrivateProfileStringWrapper(pProfile, pszSection, pszEntry, pszString);
    }

    bRet = releaseProfile(pProfile);
    return bRet;
}

sal_Bool SAL_CALL osl_writeProfileBool(oslProfile Profile,
                             const sal_Char* pszSection, const sal_Char* pszEntry,
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
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Value)
{
    int i, n;
    bool bRet = false;

    for (n = 0; Strings[n] != nullptr; n++);

    if ((i = Value - FirstId) >= n)
        bRet=false;
    else
        bRet=osl_writeProfileString(Profile, pszSection, pszEntry, Strings[i]);

    return bRet;
}

sal_Bool SAL_CALL osl_removeProfileEntry(oslProfile Profile,
                               const sal_Char *pszSection, const sal_Char *pszEntry)
{
    sal_uInt32    NoEntry;
    osl_TProfileImpl*    pProfile = nullptr;
    bool bRet = false;

    pProfile = acquireProfile(Profile, true);

    if (pProfile == nullptr)
        return false;

    if (!(pProfile->m_Flags & osl_Profile_SYSTEM))
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
    {
        WritePrivateProfileStringWrapper(pProfile, pszSection, pszEntry, nullptr);
    }

    bRet = releaseProfile(pProfile);
    return bRet;
}

sal_uInt32 SAL_CALL osl_getProfileSectionEntries(oslProfile Profile, const sal_Char *pszSection,
                                       sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_uInt32 i, n = 0;
    sal_uInt32 NoEntry;
    osl_TProfileImpl* pProfile = nullptr;

    pProfile = acquireProfile(Profile, false);

    if (pProfile == nullptr)
        return 0;

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
                    {
                        break;
                    }

                }

                pszBuffer[n++] = '\0';
            }
            else
            {
                for (i = 0; i < pSec->m_NoEntries; i++)
                {
                    n += pSec->m_Entries[i].m_Len + 1;
                }

                n += 1;
            }
        }
        else
        {
            n = 0;
        }
    }
    else
    {
        n = GetPrivateProfileStringWrapper(pProfile, pszSection, nullptr, pszBuffer, MaxLen, nullptr);
    }

    releaseProfile(pProfile);

    return n;
}

bool osl_getProfileName(rtl_uString* strPath, rtl_uString* strName, rtl_uString** strProfileName)
{
    bool bFailed;
    ::osl::LongPathBuffer< sal_Unicode > aFile( MAX_LONG_PATH );
    ::osl::LongPathBuffer< sal_Unicode > aPath( MAX_LONG_PATH );
    sal_uInt32  nFileLen = 0;
    sal_uInt32  nPathLen = 0;

    rtl_uString * strTmp = nullptr;
    oslFileError nError;

    /* build file name */
    if (strName && strName->length)
    {
        if( ::sal::static_int_cast< sal_uInt32 >( strName->length ) >= aFile.getBufSizeInSymbols() )
            return false;

        copy_ustr_n( aFile, strName->buffer, strName->length+1);
        nFileLen = strName->length;

        if (rtl_ustr_indexOfChar( aFile, L'.' ) == -1)
        {
            if (nFileLen + wcslen(STR_INI_EXTENSION) >= aFile.getBufSizeInSymbols())
                return false;

            /* add default extension */
            copy_ustr_n( aFile + nFileLen, STR_INI_EXTENSION, wcslen(STR_INI_EXTENSION)+1 );
            nFileLen += wcslen(STR_INI_EXTENSION);
        }
    }
    else
    {
        rtl_uString *strProgName = nullptr;
        sal_Unicode *pProgName;
        sal_Int32 nOffset = 0;
        sal_Int32 nLen;
        sal_Int32 nPos;

        if (osl_getExecutableFile(&strProgName) != osl_Process_E_None)
            return false;

        /* remove path and extension from filename */
        pProgName = strProgName->buffer;
        nLen = strProgName->length ;

        if ((nPos = rtl_ustr_lastIndexOfChar( pProgName, L'/' )) != -1)
            nOffset = nPos + 1;
        else if ((nPos = rtl_ustr_lastIndexOfChar( pProgName, L':' )) != -1)
            nOffset = nPos + 1;

        if ((nPos = rtl_ustr_lastIndexOfChar( pProgName, L'.' )) != -1 )
            nLen -= 4;

        if ((nFileLen = nLen - nOffset) >= aFile.getBufSizeInSymbols())
            return false;

        copy_ustr_n(aFile, pProgName + nOffset, nFileLen);

        if (nFileLen + wcslen(STR_INI_EXTENSION) >= aFile.getBufSizeInSymbols())
            return false;

        /* add default extension */
        copy_ustr_n(aFile + nFileLen, STR_INI_EXTENSION, wcslen(STR_INI_EXTENSION)+1);
        nFileLen += wcslen(STR_INI_EXTENSION);

        rtl_uString_release( strProgName );
    }

    if (aFile[0] == 0)
        return false;

    /* build directory path */
    if (strPath && strPath->length)
    {
        sal_Unicode *pPath = rtl_uString_getStr(strPath);
        sal_Int32 nLen = rtl_uString_getLength(strPath);

        if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METAHOME) , STR_INI_METAHOME) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METAHOME)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METAHOME)] == '/')))
        {
            rtl_uString * strHome = nullptr;
            oslSecurity security = osl_getCurrentSecurity();

            bFailed = ! osl_getHomeDir(security, &strHome);
            osl_freeSecurityHandle(security);

            if (bFailed) return false;

            if ( ::sal::static_int_cast< sal_uInt32 >( strHome->length ) >= aPath.getBufSizeInSymbols())
                return false;

            copy_ustr_n( aPath, strHome->buffer, strHome->length+1);
            nPathLen = strHome->length;

            if (nLen > RTL_CONSTASCII_LENGTH(STR_INI_METAHOME))
            {
                pPath += RTL_CONSTASCII_LENGTH(STR_INI_METAHOME);
                nLen -= RTL_CONSTASCII_LENGTH(STR_INI_METAHOME);

                if (nLen + nPathLen >= aPath.getBufSizeInSymbols())
                    return false;

                copy_ustr_n(aPath + nPathLen, pPath, nLen+1);
                nPathLen += nLen;
            }

            rtl_uString_release(strHome);
        }

        else if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METACFG), STR_INI_METACFG) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METACFG)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METACFG)] == '/')))
        {
            rtl_uString * strConfig = nullptr;
            oslSecurity security = osl_getCurrentSecurity();

            bFailed = ! osl_getConfigDir(security, &strConfig);
            osl_freeSecurityHandle(security);

            if (bFailed) return false;

            if ( ::sal::static_int_cast< sal_uInt32 >( strConfig->length ) >= aPath.getBufSizeInSymbols())
                return false;

            copy_ustr_n( aPath, strConfig->buffer, strConfig->length+1 );
            nPathLen = strConfig->length;

            if (nLen > RTL_CONSTASCII_LENGTH(STR_INI_METACFG))
            {
                pPath += RTL_CONSTASCII_LENGTH(STR_INI_METACFG);
                nLen -= RTL_CONSTASCII_LENGTH(STR_INI_METACFG);

                if (nLen + nPathLen >= aPath.getBufSizeInSymbols())
                    return false;

                copy_ustr_n(aPath + nPathLen, pPath, nLen+1);
                nPathLen += nLen;
            }

            rtl_uString_release(strConfig);
        }

        else if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METASYS), STR_INI_METASYS) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METASYS)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METASYS)] == '/')))
        {
            if (((nPathLen = GetWindowsDirectoryW(o3tl::toW(aPath), aPath.getBufSizeInSymbols())) == 0) || (nPathLen >= aPath.getBufSizeInSymbols()))
                return false;

            if (nLen > RTL_CONSTASCII_LENGTH(STR_INI_METASYS))
            {
                pPath += RTL_CONSTASCII_LENGTH(STR_INI_METASYS);
                nLen -= RTL_CONSTASCII_LENGTH(STR_INI_METASYS);

                if (nLen + nPathLen >= aPath.getBufSizeInSymbols())
                    return false;

                copy_ustr_n(aPath + nPathLen, pPath, nLen+1);
                nPathLen += nLen;
            }
        }

        else if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METAINS), STR_INI_METAINS) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METAINS)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METAINS)] == '/') ||
                (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METAINS)] == '"') ) )
        {
            if (! lookupProfile(pPath + RTL_CONSTASCII_LENGTH(STR_INI_METAINS), aFile, aPath))
                return false;

            nPathLen = rtl_ustr_getLength(aPath);
        }

        else if( ::sal::static_int_cast< sal_uInt32 >( nLen ) < aPath.getBufSizeInSymbols())
        {
            copy_ustr_n(aPath, pPath, nLen+1);
            nPathLen = rtl_ustr_getLength(aPath);
        }
        else
            return false;
    }
    else
    {
        rtl_uString * strConfigDir = nullptr;
        oslSecurity security = osl_getCurrentSecurity();

        bFailed = ! osl_getConfigDir(security, &strConfigDir);
        osl_freeSecurityHandle(security);

        if (bFailed) return false;
        if ( ::sal::static_int_cast< sal_uInt32 >( strConfigDir->length ) >= aPath.getBufSizeInSymbols() )
            return false;

        copy_ustr_n(aPath, strConfigDir->buffer, strConfigDir->length+1);
        nPathLen = strConfigDir->length;
    }

    if (nPathLen && (aPath[nPathLen - 1] != L'/') && (aPath[nPathLen - 1] != L'\\'))
    {
        aPath[nPathLen++] = L'\\';
        aPath[nPathLen] = 0;
    }

    if (nPathLen + nFileLen >= aPath.getBufSizeInSymbols())
        return false;

    /* append file name */
    copy_ustr_n(aPath + nPathLen, aFile, nFileLen+1);
    nPathLen += nFileLen;

    /* copy filename */
    rtl_uString_newFromStr_WithLength(&strTmp, aPath, nPathLen);
    nError = osl_getFileURLFromSystemPath(strTmp, strProfileName);
    rtl_uString_release(strTmp);

    return nError == osl_File_E_None;
}

sal_uInt32 SAL_CALL osl_getProfileSections(oslProfile Profile, sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_uInt32    i, n = 0;
    osl_TProfileImpl*    pProfile = acquireProfile(Profile, false);

    if (pProfile == nullptr)
        return 0;

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
    {
        std::vector<wchar_t> aBuf(MaxLen + 1);
        GetPrivateProfileSectionNamesW(&aBuf[0], MaxLen, o3tl::toW(rtl_uString_getStr(pProfile->m_strFileName)));

        n = WideCharToMultiByte(CP_ACP, 0, &aBuf[0], -1, pszBuffer, MaxLen, nullptr, nullptr);
    }

    releaseProfile(pProfile);

    return n;
}

static osl_TStamp getFileStamp(osl_TFile* pFile)
{
    FILETIME FileTime;

    if ((pFile->m_Handle == INVALID_HANDLE_VALUE) ||
        (! GetFileTime(pFile->m_Handle, nullptr, nullptr, &FileTime)))
        memset(&FileTime, 0, sizeof(FileTime));

    return FileTime;
}

static bool lockFile(const osl_TFile* pFile, osl_TLockMode eMode)
{
    bool     status = false;
    OVERLAPPED  Overlapped;

    if (pFile->m_Handle == INVALID_HANDLE_VALUE)
        return false;

    memset(&Overlapped, 0, sizeof(Overlapped));

    switch (eMode)
    {
        case un_lock:
            status = UnlockFileEx(
                pFile->m_Handle, 0, 0xFFFFFFFF, 0, &Overlapped);
            break;

        case read_lock:
            status = LockFileEx(
                pFile->m_Handle, 0, 0, 0xFFFFFFFF, 0, &Overlapped);
            break;

        case write_lock:
            status = LockFileEx(
                pFile->m_Handle, LOCKFILE_EXCLUSIVE_LOCK, 0, 0xFFFFFFFF, 0,
                &Overlapped);
            break;
    }

    return status;
}

static osl_TFile* openFileImpl(rtl_uString * strFileName, oslProfileOption ProfileFlags )
{
    osl_TFile* pFile = static_cast< osl_TFile*>( calloc( 1, sizeof(osl_TFile) ) );
    if (!pFile)
        return nullptr;
    bool bWriteable = false;

    if ( ProfileFlags & ( osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE ) )
    {
#ifdef DEBUG_OSL_PROFILE
        SAL_INFO("sal.osl", "setting bWriteable to TRUE");
#endif
        bWriteable=true;
    }

    if (! bWriteable)
    {
        pFile->m_Handle = CreateFileW( o3tl::toW(rtl_uString_getStr( strFileName )), GENERIC_READ,
                                          FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        /* mfe: argghh!!! do not check if the file could be opened */
        /*      default mode expects it that way!!!                */
    }
    else
    {
#ifdef DEBUG_OSL_PROFILE
        SAL_INFO("sal.osl", "opening read/write " << pszFilename);
#endif

        if ((pFile->m_Handle = CreateFileW( o3tl::toW(rtl_uString_getStr( strFileName )), GENERIC_READ | GENERIC_WRITE,
                                               FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr))
            == INVALID_HANDLE_VALUE)
        {
            free(pFile);
            return nullptr;
        }
    }

    pFile->m_pWriteBuf=nullptr;
    pFile->m_nWriteBufFree=0;
    pFile->m_nWriteBufLen=0;

    if ( ProfileFlags & (osl_Profile_WRITELOCK | osl_Profile_READLOCK ) )
    {
#ifdef DEBUG_OSL_PROFILE
        SAL_INFO("sal.osl", "locking file " << pszFilename);
#endif

        lockFile(pFile, bWriteable ? write_lock : read_lock);
    }

    return pFile;
}

static osl_TStamp closeFileImpl(osl_TFile* pFile)
{
    osl_TStamp stamp = {0, 0};

    if ( pFile == nullptr )
    {
        return stamp;
    }

    if (pFile->m_Handle != INVALID_HANDLE_VALUE)
    {
        stamp = getFileStamp(pFile);

        lockFile(pFile, un_lock);

        CloseHandle(pFile->m_Handle);
        pFile->m_Handle = INVALID_HANDLE_VALUE;
    }

    if ( pFile->m_pWriteBuf != nullptr )
    {
        free(pFile->m_pWriteBuf);
    }

    free(pFile);

    return stamp;
}

static bool rewindFile(osl_TFile* pFile, bool bTruncate)
{
    if (pFile->m_Handle != INVALID_HANDLE_VALUE)
    {
        pFile->m_pReadPtr = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);

        SetFilePointer(pFile->m_Handle, 0, nullptr, FILE_BEGIN);

        if (bTruncate)
            SetEndOfFile(pFile->m_Handle);
    }

    return true;
}

static bool getLine(osl_TFile* pFile, sal_Char *pszLine, int MaxLen)
{
    DWORD Max;
    size_t Free;
    sal_Char* pChr;
    sal_Char* pLine = pszLine;

    if (pFile->m_Handle == INVALID_HANDLE_VALUE)
        return false;

    MaxLen -= 1;

    do
    {
        size_t Bytes = sizeof(pFile->m_ReadBuf) - (pFile->m_pReadPtr - pFile->m_ReadBuf);

        if (Bytes <= 1)
        {
            /* refill buffer */
            memcpy(pFile->m_ReadBuf, pFile->m_pReadPtr, Bytes);
            pFile->m_pReadPtr = pFile->m_ReadBuf;

            Free = sizeof(pFile->m_ReadBuf) - Bytes;

            if (! ReadFile(pFile->m_Handle, &pFile->m_ReadBuf[Bytes], Free, &Max, nullptr))
            {
                *pLine = '\0';
                return false;
            }

            if (Max < Free)
            {
                if ((Max == 0) && (pLine == pszLine))
                {
                    *pLine = '\0';
                    return false;
                }

                pFile->m_ReadBuf[Bytes + Max] = '\0';
            }
        }

        for (pChr = pFile->m_pReadPtr;
             (*pChr != '\n') && (*pChr != '\r') && (*pChr != '\0') &&
             (pChr < (pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf) - 1));
             pChr++);

        Max = min(static_cast<int>(pChr - pFile->m_pReadPtr), MaxLen);
        memcpy(pLine, pFile->m_pReadPtr, Max);
        MaxLen -= Max;
        pLine  += Max;

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

            *pLine = '\0';

            /* setting MaxLen to -1 indicates terminating read loop */
            MaxLen = -1;
        }

        pFile->m_pReadPtr = pChr;
    }
    while (MaxLen > 0);

    return true;
}

static bool putLine(osl_TFile* pFile, const sal_Char *pszLine)
{
    unsigned int Len = strlen(pszLine);

    if ( pFile == nullptr || pFile->m_Handle == INVALID_HANDLE_VALUE )
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

    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len]='\r';
    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len + 1]='\n';
    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len + 2]='\0';

    pFile->m_nWriteBufFree-=Len+2;

    return true;
}

/* platform specific end */

static const sal_Char* stripBlanks(const sal_Char* String, sal_uInt32* pLen)
{
    if ( (pLen != nullptr) && ( *pLen != 0 ) )
    {
        while ((String[*pLen - 1] == ' ') || (String[*pLen - 1] == '\t'))
            (*pLen)--;

        while ((*String == ' ') || (*String == '\t'))
        {
            String++;
            (*pLen)--;
        }
    }
    else
        while ((*String == ' ') || (*String == '\t'))
            String++;

    return String;
}

static const sal_Char* addLine(osl_TProfileImpl* pProfile, const sal_Char* Line)
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
            unsigned int index=0;
            unsigned int oldmax=pProfile->m_MaxLines;

            pProfile->m_MaxLines += LINES_ADD;
            if (auto p = static_cast<sal_Char **>(realloc(pProfile->m_Lines, pProfile->m_MaxLines * sizeof(sal_Char *))))
            {
                pProfile->m_Lines = p;

                for ( index = oldmax ; index < pProfile->m_MaxLines ; ++index )
                {
                    pProfile->m_Lines[index]=nullptr;
                }
            }
            else
            {
                free(pProfile->m_Lines);
                pProfile->m_Lines = nullptr;
            }
        }

        if (pProfile->m_Lines == nullptr)
        {
            pProfile->m_NoLines  = 0;
            pProfile->m_MaxLines = 0;
            return nullptr;
        }

    }

    if ( pProfile->m_Lines != nullptr && pProfile->m_Lines[pProfile->m_NoLines] != nullptr )
    {
            free(pProfile->m_Lines[pProfile->m_NoLines]);
    }
    pProfile->m_Lines[pProfile->m_NoLines++] = strdup(Line);

    return pProfile->m_Lines[pProfile->m_NoLines - 1];
}

static const sal_Char* insertLine(osl_TProfileImpl* pProfile, const sal_Char* Line, sal_uInt32 LineNo)
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
            if (auto p = static_cast<sal_Char**>(
                    realloc(pProfile->m_Lines, pProfile->m_MaxLines * sizeof(sal_Char*))))
            {
                pProfile->m_Lines = p;

                memset(&pProfile->m_Lines[pProfile->m_NoLines], 0,
                       (pProfile->m_MaxLines - pProfile->m_NoLines - 1) * sizeof(sal_Char*));
            }
            else
            {
                free(pProfile->m_Lines);
                pProfile->m_Lines = nullptr;
            }
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

    return;
}

static void setEntry(osl_TProfileImpl* pProfile, osl_TProfileSection* pSection,
                     sal_uInt32 NoEntry, sal_uInt32 Line,
                     const sal_Char* Entry, sal_uInt32 Len)
{
    Entry = stripBlanks(Entry, &Len);
    pSection->m_Entries[NoEntry].m_Line   = Line;
    pSection->m_Entries[NoEntry].m_Offset = Entry - pProfile->m_Lines[Line];
    pSection->m_Entries[NoEntry].m_Len    = Len;

    return;
}

static bool addEntry(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection,
                        int Line, const sal_Char* Entry, sal_uInt32 Len)
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
                if (auto p = static_cast<osl_TProfileEntry*>(realloc(
                        pSection->m_Entries, pSection->m_MaxEntries * sizeof(osl_TProfileEntry))))
                    pSection->m_Entries = p;
                else
                {
                    free(pSection->m_Entries);
                    pSection->m_Entries = nullptr;
                }
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

    return;
}

static bool addSection(osl_TProfileImpl* pProfile, int Line, const sal_Char* Section, sal_uInt32 Len)
{
    if (pProfile->m_NoSections >= pProfile->m_MaxSections)
    {
        if (pProfile->m_Sections == nullptr)
        {
            pProfile->m_MaxSections = SECTIONS_INI;
            pProfile->m_Sections = static_cast<osl_TProfileSection*>(calloc(pProfile->m_MaxSections, sizeof(osl_TProfileSection)));
        }
        else
        {
            unsigned int index=0;
            unsigned int oldmax=pProfile->m_MaxSections;

            pProfile->m_MaxSections += SECTIONS_ADD;
            if (auto p = static_cast<osl_TProfileSection*>(realloc(
                    pProfile->m_Sections, pProfile->m_MaxSections * sizeof(osl_TProfileSection))))
            {
                pProfile->m_Sections = p;
                for ( index = oldmax ; index < pProfile->m_MaxSections ; ++index )
                {
                    pProfile->m_Sections[index].m_Entries=nullptr;
                }
            }
            else
            {
                free(pProfile->m_Sections);
                pProfile->m_Sections = nullptr;
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

    Section = stripBlanks(Section, &Len);
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

    return;
}

static osl_TProfileSection* findEntry(osl_TProfileImpl* pProfile, const sal_Char* Section,
                                      const sal_Char* Entry, sal_uInt32 *pNoEntry)
{
    static  sal_uInt32    Sect = 0;
    sal_uInt32    i, n;
    sal_uInt32    Len;
    osl_TProfileSection* pSec = nullptr;

    Len = strlen(Section);
    Section = stripBlanks(Section, &Len);

    n = Sect;

    for (i = 0; i < pProfile->m_NoSections; i++)
    {
        n %= pProfile->m_NoSections;
        pSec = &pProfile->m_Sections[n];
        if ((Len == pSec->m_Len) &&
            (strnicmp(Section, &pProfile->m_Lines[pSec->m_Line][pSec->m_Offset], pSec->m_Len)
             == 0))
            break;
        n++;
    }

    Sect = n;

    if (i < pProfile->m_NoSections)
    {
        Len = strlen(Entry);
        Entry = stripBlanks(Entry, &Len);

        *pNoEntry = pSec->m_NoEntries;

        for (i = 0; i < pSec->m_NoEntries; i++)
        {
            const sal_Char* pStr = &pProfile->m_Lines[pSec->m_Entries[i].m_Line]
                                     [pSec->m_Entries[i].m_Offset];
            if ((Len == pSec->m_Entries[i].m_Len) &&
                (strnicmp(Entry, pStr, pSec->m_Entries[i].m_Len)
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
    sal_uInt32    i;
    sal_Char const * pStr;
    sal_Char const * pChar;
    sal_Char        Line[4096];

    pProfile->m_NoLines    = 0;
    pProfile->m_NoSections = 0;

    OSL_VERIFY(rewindFile(pFile, false));

    while (getLine(pFile, Line, sizeof(Line)))
    {
        if (! addLine(pProfile, Line))
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
                return false;
        }
        else
        {
            /* new section */
            if (! addSection(pProfile, i, pStr + 1, pChar - pStr - 1))
                return false;
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

            OSL_VERIFY(rewindFile(pTmpFile, true));

            for (i = 0; i < pProfile->m_NoLines; i++)
            {
                OSL_VERIFY(putLine(pTmpFile, pProfile->m_Lines[i]));
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

                closeFileImpl(pTmpFile);

                return false;
            }

            pProfile->m_Flags &= ~FLG_MODIFIED;

            closeFileImpl(pProfile->m_pFile);
            closeFileImpl(pTmpFile);

            osl_ProfileSwapProfileNames(pProfile);

            pProfile->m_pFile = openFileImpl(pProfile->m_strFileName,pProfile->m_Flags);

        }

        if (bCleanup)
        {
            while (pProfile->m_NoLines > 0)
                removeLine(pProfile, pProfile->m_NoLines - 1);

            free(pProfile->m_Lines);
            pProfile->m_Lines = nullptr;
            pProfile->m_MaxLines = 0;

            while (pProfile->m_NoSections > 0)
                removeSection(pProfile, &pProfile->m_Sections[pProfile->m_NoSections - 1]);

            free(pProfile->m_Sections);
            pProfile->m_Sections = nullptr;
            pProfile->m_MaxSections = 0;
        }
    }

    return true;
}

static osl_TFile* osl_openTmpProfileImpl(osl_TProfileImpl* pProfile)
{
    osl_TFile* pFile=nullptr;
    rtl_uString* ustrExtension=nullptr;
    rtl_uString* ustrTmpName=nullptr;
    oslProfileOption PFlags=0;

    rtl_uString_newFromAscii(&ustrExtension,"tmp");

    /* generate tmp profilename */
    ustrTmpName=osl_ProfileGenerateExtension(pProfile->m_strFileName,ustrExtension);
    rtl_uString_release(ustrExtension);

    if (ustrTmpName == nullptr)
        return nullptr;

    if (!(pProfile->m_Flags & osl_Profile_READLOCK))
        PFlags |= osl_Profile_WRITELOCK;

    /* open this file */
    pFile = openFileImpl(ustrTmpName,pProfile->m_Flags | PFlags);

    /* return new pFile */
    return pFile;
}

static bool osl_ProfileSwapProfileNames(osl_TProfileImpl* pProfile)
{
    rtl_uString* ustrBakFile=nullptr;
    rtl_uString* ustrTmpFile=nullptr;
    rtl_uString* ustrIniFile=nullptr;
    rtl_uString* ustrExtension=nullptr;

    rtl_uString_newFromAscii(&ustrExtension,"bak");

    ustrBakFile=osl_ProfileGenerateExtension(pProfile->m_strFileName,ustrExtension);
    rtl_uString_release(ustrExtension);
    ustrExtension=nullptr;

    rtl_uString_newFromAscii(&ustrExtension,"ini");

    ustrIniFile=osl_ProfileGenerateExtension(pProfile->m_strFileName,ustrExtension);
    rtl_uString_release(ustrExtension);
    ustrExtension=nullptr;

    rtl_uString_newFromAscii(&ustrExtension,"tmp");

    ustrTmpFile=osl_ProfileGenerateExtension(pProfile->m_strFileName,ustrExtension);
    rtl_uString_release(ustrExtension);
    ustrExtension=nullptr;

    /* unlink bak */
    DeleteFileW( o3tl::toW(rtl_uString_getStr( ustrBakFile )) );

    /* rename ini bak */
    MoveFileExW( o3tl::toW(rtl_uString_getStr( ustrIniFile )), o3tl::toW(rtl_uString_getStr( ustrBakFile )), MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH );

    /* rename tmp ini */
    MoveFileExW( o3tl::toW(rtl_uString_getStr( ustrTmpFile )), o3tl::toW(rtl_uString_getStr( ustrIniFile )), MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH );

    return false;
}

static rtl_uString* osl_ProfileGenerateExtension(rtl_uString* ustrFileName, rtl_uString* ustrExtension)
{
    rtl_uString* ustrNewFileName = nullptr;
    rtl_uString* ustrOldExtension = nullptr;

    sal_Unicode* pFileNameBuf = rtl_uString_getStr(ustrFileName);

    rtl_uString_newFromAscii(&ustrOldExtension, ".");

    sal_Unicode* pExtensionBuf = rtl_uString_getStr(ustrOldExtension);

    sal_Int32 nIndex = rtl_ustr_lastIndexOfChar(pFileNameBuf, *pExtensionBuf);

    rtl_uString_newReplaceStrAt(&ustrNewFileName,
                                ustrFileName,
                                nIndex+1,
                                3,
                                ustrExtension);

    return ustrNewFileName;
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
#ifdef DEBUG_OSL_PROFILE
        SAL_INFO("sal.osl", "AUTOOPEN MODE");
#endif

        if ( ( pProfile = static_cast<osl_TProfileImpl*>(osl_openProfile( nullptr, PFlags )) ) != nullptr )
        {
            pProfile->m_Flags |= FLG_AUTOOPEN;
        }
    }
    else
    {
#ifdef DEBUG_OSL_PROFILE
        SAL_INFO("sal.osl", "try to acquire");
#endif

        if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
        {
            if (! (pProfile->m_Flags & (osl_Profile_READLOCK |
                                        osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE)))
            {
                osl_TStamp Stamp;
#ifdef DEBUG_OSL_PROFILE
                SAL_INFO("sal.osl", "DEFAULT MODE");
#endif
                pProfile->m_pFile = openFileImpl(
                    pProfile->m_strFileName, pProfile->m_Flags | PFlags);
                if (!pProfile->m_pFile)
                    return nullptr;

                Stamp = getFileStamp(pProfile->m_pFile);

                if (memcmp(&Stamp, &(pProfile->m_Stamp), sizeof(osl_TStamp)))
                {
                    pProfile->m_Stamp = Stamp;

                    loadProfile(pProfile->m_pFile, pProfile);
                }
            }
            else
            {
#ifdef DEBUG_OSL_PROFILE
                SAL_INFO("sal.osl", "READ/WRITELOCK MODE");
#endif

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

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if (pProfile->m_Flags & FLG_AUTOOPEN)
        {
            return osl_closeProfile(static_cast<oslProfile>(pProfile));
        }
        else
        {
#ifdef DEBUG_OSL_PROFILE
        SAL_INFO("sal.osl", "DEFAULT MODE");
#endif
        if (! (pProfile->m_Flags & (osl_Profile_READLOCK |
                                    osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE)))
            {
                if (pProfile->m_Flags & FLG_MODIFIED)
                    storeProfile(pProfile, false);

                closeFileImpl(pProfile->m_pFile);
                pProfile->m_pFile = nullptr;
            }
        }
    }

    return true;
}

static bool lookupProfile(const sal_Unicode *strPath, const sal_Unicode *strFile, sal_Unicode *strProfile)
{
    sal_Char *pChr;
    sal_Char Buffer[4096] = "";
    sal_Char Product[132] = "";

    ::osl::LongPathBuffer< sal_Unicode > aPath( MAX_LONG_PATH );
    aPath[0] = 0;

    if (*strPath == L'"')
    {
        int i = 0;

        strPath++;

        while ((strPath[i] != L'"') && (strPath[i] != L'\0'))
            i++;

        WideCharToMultiByte(CP_ACP,0, o3tl::toW(strPath), i, Product, sizeof(Product), nullptr, nullptr);
        Product[i] = '\0';
        strPath += i;

        if (*strPath == L'"')
            strPath++;

        if ( (*strPath == L'/') || (*strPath == L'\\') )
        {
            strPath++;
        }
    }

    else
    {
        /* if we have not product identification, do a special handling for soffice.ini */
        if (rtl_ustr_ascii_compare(strFile, SVERSION_PROFILE) == 0)
        {
            rtl_uString * strSVProfile  = nullptr;
            rtl_uString * strSVFallback = nullptr;
            rtl_uString * strSVLocation = nullptr;
            rtl_uString * strSVName     = nullptr;
            ::osl::LongPathBuffer< sal_Char > aDir( MAX_LONG_PATH );
            oslProfile hProfile;

            rtl_uString_newFromAscii(&strSVFallback, SVERSION_FALLBACK);
            rtl_uString_newFromAscii(&strSVLocation, SVERSION_LOCATION);
            rtl_uString_newFromAscii(&strSVName, SVERSION_NAME);

            /* open sversion.ini in the system directory, and try to locate the entry
               with the highest version for StarOffice */
            if (osl_getProfileName( strSVFallback, strSVName, &strSVProfile))
            {
                hProfile = osl_openProfile(strSVProfile, osl_Profile_READLOCK);
                if (hProfile)
                {
                    osl_getProfileSectionEntries(
                        hProfile, SVERSION_SECTION, Buffer, sizeof(Buffer));

                    for (pChr = Buffer; *pChr != '\0'; pChr += strlen(pChr) + 1)
                    {
                        if ((strnicmp(
                                 pChr, SVERSION_SOFFICE,
                                 sizeof(SVERSION_SOFFICE) - 1)
                             == 0)
                            && (stricmp(Product, pChr) < 0))
                        {
                            osl_readProfileString(
                                hProfile, SVERSION_SECTION, pChr, aDir,
                                aDir.getBufSizeInSymbols(), "");

                            /* check for existence of path */
                            if (access(aDir, 0) >= 0)
                                strcpy(Product, pChr);
                        }
                    }

                    osl_closeProfile(hProfile);
                }
                rtl_uString_release(strSVProfile);
                strSVProfile = nullptr;
            }

            /* open sversion.ini in the users directory, and try to locate the entry
               with the highest version for StarOffice */
            if ( osl_getProfileName(strSVLocation, strSVName, &strSVProfile) )
            {
                hProfile = osl_openProfile(strSVProfile, osl_Profile_READLOCK);
                if (hProfile)
                {
                    osl_getProfileSectionEntries(
                        hProfile, SVERSION_SECTION, Buffer, sizeof(Buffer));

                    for (pChr = Buffer; *pChr != '\0'; pChr += strlen(pChr) + 1)
                    {
                        if ((strnicmp(
                                 pChr, SVERSION_SOFFICE,
                                 sizeof(SVERSION_SOFFICE) - 1)
                             == 0)
                            && (stricmp(Product, pChr) < 0))
                        {
                            osl_readProfileString(
                                hProfile, SVERSION_SECTION, pChr, aDir,
                                aDir.getBufSizeInSymbols(), "");

                            /* check for existence of path */
                            if (access(aDir, 0) >= 0)
                                strcpy(Product, pChr);
                        }
                    }

                    osl_closeProfile(hProfile);
                }
                rtl_uString_release(strSVProfile);
            }

            rtl_uString_release(strSVFallback);
            rtl_uString_release(strSVLocation);
            rtl_uString_release(strSVName);

            /* remove any trailing build number */
            if ((pChr = strrchr(Product, '/')) != nullptr)
                *pChr = '\0';
        }
    }

    rtl_uString * strExecutable = nullptr;
    rtl_uString * strTmp = nullptr;
    sal_Int32 nPos;

    /* try to find the file in the directory of the executable */
    if (osl_getExecutableFile(&strTmp) != osl_Process_E_None)
        return false;

    /* convert to native path */
    if (osl_getSystemPathFromFileURL(strTmp, &strExecutable) != osl_File_E_None)
    {
        rtl_uString_release(strTmp);
        return false;
    }

    rtl_uString_release(strTmp);

    DWORD dwPathLen = 0;

    /* separate path from filename */
    if ((nPos = rtl_ustr_lastIndexOfChar(strExecutable->buffer, L'\\')) == -1)
    {
        if ((nPos = rtl_ustr_lastIndexOfChar(strExecutable->buffer, L':')) == -1)
        {
            rtl_uString_release(strExecutable);
            return false;
        }
        else
        {
            copy_ustr_n(aPath, strExecutable->buffer, nPos);
            aPath[nPos] = 0;
            dwPathLen = nPos;
        }
    }
    else
    {
        copy_ustr_n(aPath, strExecutable->buffer, nPos);
        dwPathLen = nPos;
        aPath[dwPathLen] = 0;
    }

    /* if we have no product identification use the executable file name */
    if (*Product == 0)
    {
        WideCharToMultiByte(CP_ACP,0, o3tl::toW(strExecutable->buffer + nPos + 1), -1, Product, sizeof(Product), nullptr, nullptr);

        /* remove extension */
        if ((pChr = strrchr(Product, '.')) != nullptr)
            *pChr = '\0';
    }

    rtl_uString_release(strExecutable);

    /* remember last subdir */
    nPos = rtl_ustr_lastIndexOfChar(aPath, L'\\');

    copy_ustr_n(aPath + dwPathLen++, L"\\", 2);

    if (*strPath)
    {
        copy_ustr_n(aPath + dwPathLen, strPath, rtl_ustr_getLength(strPath)+1);
        dwPathLen += rtl_ustr_getLength(strPath);
    }

    {
        ::osl::LongPathBuffer< sal_Char > aTmpPath( MAX_LONG_PATH );

        WideCharToMultiByte(CP_ACP,0, o3tl::toW(aPath), -1, aTmpPath, aTmpPath.getBufSizeInSymbols(), nullptr, nullptr);

        /* if file not exists, remove any specified subdirectories
           like "bin" or "program" */

        if (((access(aTmpPath, 0) < 0) && (nPos != -1)) || (*strPath == 0))
        {
            static const sal_Char *SubDirs[] = SVERSION_DIRS;

            unsigned i = 0;
            sal_Char *pStr = aTmpPath + nPos;

            for (i = 0; i < SAL_N_ELEMENTS(SubDirs); i++)
                if (strnicmp(pStr + 1, SubDirs[i], strlen(SubDirs[i])) == 0)
                {
                    if ( *strPath == 0)
                    {
                        strcpy(pStr + 1,SVERSION_USER);
                        if ( access(aTmpPath, 0) < 0 )
                        {
                            *(pStr+1)='\0';
                        }
                        else
                        {
                            dwPathLen = nPos + MultiByteToWideChar( CP_ACP, 0, SVERSION_USER, -1, o3tl::toW(aPath + nPos + 1), aPath.getBufSizeInSymbols() - (nPos + 1) );
                        }
                    }
                    else
                    {
                        copy_ustr_n(aPath + nPos + 1, strPath, rtl_ustr_getLength(strPath)+1);
                        dwPathLen = nPos + 1 + rtl_ustr_getLength(strPath);
                    }

                    break;
                }
        }
    }

    if ((aPath[dwPathLen - 1] != L'/') && (aPath[dwPathLen - 1] != L'\\'))
    {
        aPath[dwPathLen++] = L'\\';
        aPath[dwPathLen] = 0;
    }

    copy_ustr_n(aPath + dwPathLen, strFile, rtl_ustr_getLength(strFile)+1);

    {
        ::osl::LongPathBuffer< sal_Char > aTmpPath( MAX_LONG_PATH );

        WideCharToMultiByte(CP_ACP,0, o3tl::toW(aPath), -1, aTmpPath, aTmpPath.getBufSizeInSymbols(), nullptr, nullptr);

        if ((access(aTmpPath, 0) < 0) && (Product[0] != '\0'))
        {
            rtl_uString * strSVFallback = nullptr;
            rtl_uString * strSVProfile  = nullptr;
            rtl_uString * strSVLocation = nullptr;
            rtl_uString * strSVName     = nullptr;
            oslProfile hProfile;

            rtl_uString_newFromAscii(&strSVFallback, SVERSION_FALLBACK);
            rtl_uString_newFromAscii(&strSVLocation, SVERSION_LOCATION);
            rtl_uString_newFromAscii(&strSVName, SVERSION_NAME);

            /* open sversion.ini in the system directory, and try to locate the entry
               with the highest version for StarOffice */
            if (osl_getProfileName(strSVLocation, strSVName, &strSVProfile))
            {
                hProfile = osl_openProfile(
                    strSVProfile, osl_Profile_READLOCK);
                if (hProfile)
                {
                    osl_readProfileString(
                        hProfile, SVERSION_SECTION, Product, Buffer,
                        sizeof(Buffer), "");
                    osl_closeProfile(hProfile);

                    /* if not found, try the fallback */
                    if (Buffer[0] == '\0')
                    {
                        if (osl_getProfileName(
                                strSVFallback, strSVName, &strSVProfile))
                        {
                            hProfile = osl_openProfile(
                                strSVProfile, osl_Profile_READLOCK);
                            if (hProfile)
                            {
                                osl_readProfileString(
                                    hProfile, SVERSION_SECTION, Product,
                                    Buffer, sizeof(Buffer), "");
                            }
                        }

                        osl_closeProfile(hProfile);
                    }

                    if (Buffer[0] != '\0')
                    {
                        dwPathLen = MultiByteToWideChar(
                            CP_ACP, 0, Buffer, -1, o3tl::toW(aPath), aPath.getBufSizeInSymbols() );
                        dwPathLen -=1;

                        /* build full path */
                        if ((aPath[dwPathLen - 1] != L'/')
                            && (aPath[dwPathLen - 1] != L'\\'))
                        {
                            copy_ustr_n(aPath + dwPathLen++, L"\\", 2);
                        }

                        if (*strPath)
                        {
                            copy_ustr_n(aPath + dwPathLen, strPath, rtl_ustr_getLength(strPath)+1);
                            dwPathLen += rtl_ustr_getLength(strPath);
                        }
                        else
                        {
                            ::osl::LongPathBuffer< sal_Char > aTmpPath2( MAX_LONG_PATH );
                            int n;

                            if ((n = WideCharToMultiByte(
                                     CP_ACP,0, o3tl::toW(aPath), -1, aTmpPath2,
                                     aTmpPath2.getBufSizeInSymbols(), nullptr, nullptr))
                                > 0)
                            {
                                strcpy(aTmpPath2 + n, SVERSION_USER);
                                if (access(aTmpPath2, 0) >= 0)
                                {
                                    dwPathLen += MultiByteToWideChar(
                                        CP_ACP, 0, SVERSION_USER, -1,
                                        o3tl::toW(aPath + dwPathLen),
                                        aPath.getBufSizeInSymbols() - dwPathLen );
                                }
                            }
                        }
                    }
                }

                rtl_uString_release(strSVProfile);
            }

            rtl_uString_release(strSVFallback);
            rtl_uString_release(strSVLocation);
            rtl_uString_release(strSVName);
        }
    }

    aPath[dwPathLen] = 0;

    /* copy filename */
    copy_ustr_n(strProfile, aPath, dwPathLen+1);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
