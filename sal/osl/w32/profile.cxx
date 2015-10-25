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

#include "file_url.h"
#include "path_helper.hxx"

#include <string.h>
#include <osl/diagnose.h>
#include <osl/profile.h>
#include <osl/process.h>
#include <osl/file.h>
#include <rtl/alloc.h>
#include <sal/macros.h>
#include <algorithm>
using std::min;
static inline void copy_ustr_n( void *dest, const void *source, size_t length ) { memcpy(dest, source, length*sizeof(sal_Unicode)); }

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
#define SVERSION_OPTION     "userid:"
#define SVERSION_DIRS       { "bin", "program" }
#define SVERSION_USER       "user"

/*#define DEBUG_OSL_PROFILE 1*/
/*#define TRACE_OSL_PROFILE 1*/

/*****************************************************************************/
/* Data Type Definition */
/*****************************************************************************/

typedef FILETIME osl_TStamp;

typedef enum _osl_TLockMode
{
    un_lock, read_lock, write_lock
} osl_TLockMode;

typedef struct _osl_TFile
{
    HANDLE  m_Handle;
    sal_Char*   m_pReadPtr;
    sal_Char    m_ReadBuf[512];
    sal_Char*   m_pWriteBuf;
    sal_uInt32  m_nWriteBufLen;
    sal_uInt32  m_nWriteBufFree;
} osl_TFile;

typedef struct _osl_TProfileEntry
{
    sal_uInt32      m_Line;
    sal_uInt32      m_Offset;
    sal_uInt32      m_Len;
} osl_TProfileEntry;

typedef struct _osl_TProfileSection
{
    sal_uInt32          m_Line;
    sal_uInt32          m_Offset;
    sal_uInt32          m_Len;
    sal_uInt32          m_NoEntries;
    sal_uInt32          m_MaxEntries;
    osl_TProfileEntry*  m_Entries;
} osl_TProfileSection;

/*
    Profile-data structure hidden behind oslProfile:
*/
typedef struct _osl_TProfileImpl
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
} osl_TProfileImpl;

/*****************************************************************************/
/* Static Module Function Declarations */
/*****************************************************************************/

static osl_TFile*           openFileImpl(rtl_uString * strFileName, oslProfileOption ProfileFlags  );
static osl_TStamp           closeFileImpl(osl_TFile* pFile);
static sal_Bool             lockFile(const osl_TFile* pFile, osl_TLockMode eMode);
static sal_Bool             rewindFile(osl_TFile* pFile, sal_Bool bTruncate);
static osl_TStamp           getFileStamp(osl_TFile* pFile);

static sal_Bool             getLine(osl_TFile* pFile, const sal_Char *pszLine, int MaxLen);
static sal_Bool             putLine(osl_TFile* pFile, const sal_Char *pszLine);
static const sal_Char*      stripBlanks(const sal_Char* String, sal_uInt32* pLen);
static const sal_Char*      addLine(osl_TProfileImpl* pProfile, const sal_Char* Line);
static const sal_Char*      insertLine(osl_TProfileImpl* pProfile, const sal_Char* Line, sal_uInt32 LineNo);
static void                 removeLine(osl_TProfileImpl* pProfile, sal_uInt32 LineNo);
static void                 setEntry(osl_TProfileImpl* pProfile, osl_TProfileSection* pSection,
                                     sal_uInt32 NoEntry, sal_uInt32 Line,
                                     const sal_Char* Entry, sal_uInt32 Len);
static sal_Bool             addEntry(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection,
                                     int Line, const sal_Char* Entry, sal_uInt32 Len);
static void                 removeEntry(osl_TProfileSection *pSection, sal_uInt32 NoEntry);
static sal_Bool             addSection(osl_TProfileImpl* pProfile, int Line, const sal_Char* Section, sal_uInt32 Len);
static void                 removeSection(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection);
static osl_TProfileSection* findEntry(osl_TProfileImpl* pProfile, const sal_Char* Section,
                                      const sal_Char* Entry, sal_uInt32 *pNoEntry);
static sal_Bool             loadProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile);
static sal_Bool             storeProfile(osl_TProfileImpl* pProfile, sal_Bool bCleanup);
static osl_TProfileImpl*    acquireProfile(oslProfile Profile, sal_Bool bWriteable);
static sal_Bool             releaseProfile(osl_TProfileImpl* pProfile);
static sal_Bool             lookupProfile(const sal_Unicode *strPath, const sal_Unicode *strFile, sal_Unicode *strProfile);

static sal_Bool writeProfileImpl (osl_TFile* pFile);
static osl_TFile* osl_openTmpProfileImpl(osl_TProfileImpl*);
static sal_Bool osl_ProfileSwapProfileNames(osl_TProfileImpl*);
static rtl_uString* osl_ProfileGenerateExtension(rtl_uString* ustrFileName, rtl_uString* ustrExtension);

static sal_Bool SAL_CALL osl_getProfileName(rtl_uString* strPath, rtl_uString* strName, rtl_uString** strProfileName);

/*****************************************************************************/
/* Exported Module Functions */
/*****************************************************************************/

oslProfile SAL_CALL osl_openProfile(rtl_uString *strProfileName, sal_uInt32 Flags)
{
    osl_TFile*        pFile = NULL;
    osl_TProfileImpl* pProfile;
    rtl_uString       *FileName=NULL;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_openProfile");
#endif
    OSL_VERIFY(strProfileName);

    if (rtl_uString_getLength(strProfileName) == 0 )
    {
        OSL_VERIFY(osl_getProfileName(NULL, NULL, &FileName));
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
        OSL_TRACE("with osl_Profile_DEFAULT");
    }
    if ( Flags & osl_Profile_SYSTEM )
    {
        OSL_TRACE("with osl_Profile_SYSTEM");
    }
    if ( Flags & osl_Profile_READLOCK )
    {
        OSL_TRACE("with osl_Profile_READLOCK");
    }
    if ( Flags & osl_Profile_WRITELOCK )
    {
        OSL_TRACE("with osl_Profile_WRITELOCK");
    }
    if ( Flags & osl_Profile_FLUSHWRITE )
    {
        OSL_TRACE("with osl_Profile_FLUSHWRITE");
    }
#endif

    if ( (! (Flags & osl_Profile_SYSTEM)) && ( (pFile = openFileImpl(FileName, Flags) ) == NULL ) )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_openProfile [not opened]");
#endif
        if( FileName)
            rtl_uString_release( FileName);

        return NULL;
    }

    pProfile = (osl_TProfileImpl*)calloc(1, sizeof(osl_TProfileImpl));

    pProfile->m_Flags = Flags & FLG_USER;
    osl_getSystemPathFromFileURL(strProfileName, &pProfile->m_strFileName);

    if (Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE ))
        pProfile->m_pFile = pFile;

    pProfile->m_Stamp = getFileStamp(pFile);

    loadProfile(pFile, pProfile);

    if (pProfile->m_pFile == NULL)
        closeFileImpl(pFile);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_openProfile [ok]");
#endif
    if( FileName)
        rtl_uString_release( FileName);

    return pProfile;
}

sal_Bool SAL_CALL osl_closeProfile(oslProfile Profile)
{
    osl_TProfileImpl* pProfile = (osl_TProfileImpl*)Profile;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_closeProfile");
#endif

    if ( Profile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_closeProfile [profile==0]");
#endif
        return sal_False;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        pProfile = acquireProfile(Profile,sal_True);

        if ( pProfile != 0 )
        {
            if ( !( pProfile->m_Flags & osl_Profile_READLOCK )  && ( pProfile->m_Flags & FLG_MODIFIED ) )
            {
                storeProfile(pProfile, sal_False);
            }
        }
        else
        {
            pProfile = acquireProfile(Profile,sal_False);
        }

        if ( pProfile == 0 )
        {
#ifdef TRACE_OSL_PROFILE
            OSL_TRACE("Out osl_closeProfile [pProfile==0]");
#endif
            return sal_False;
        }

        if (pProfile->m_pFile != NULL)
            closeFileImpl(pProfile->m_pFile);
    }

    pProfile->m_pFile = NULL;
    rtl_uString_release(pProfile->m_strFileName);
    pProfile->m_strFileName = NULL;

    /* release whole profile data types memory */
    if ( pProfile->m_NoLines > 0)
    {
        unsigned int index=0;
        if ( pProfile->m_Lines != 0 )
        {
            for ( index = 0 ; index < pProfile->m_NoLines ; ++index)
            {
                if ( pProfile->m_Lines[index] != 0 )
                {
                    free(pProfile->m_Lines[index]);
                }
            }
            free(pProfile->m_Lines);
        }
        if ( pProfile->m_Sections != 0 )
        {
            for ( index = 0 ; index < pProfile->m_NoSections ; ++index )
            {
                if ( pProfile->m_Sections[index].m_Entries != 0 )
                    free(pProfile->m_Sections[index].m_Entries);
            }
            free(pProfile->m_Sections);
        }

    }
    free(pProfile);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_closeProfile [ok]");
#endif
    return sal_True;
}

sal_Bool SAL_CALL osl_flushProfile(oslProfile Profile)
{
    osl_TProfileImpl* pProfile = (osl_TProfileImpl*) Profile;
    osl_TFile* pFile;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_flushProfile()");
#endif

    if ( pProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_flushProfile() [pProfile == 0]");
#endif
        return sal_False;
    }

    pFile = pProfile->m_pFile;
    if ( pFile == 0 || pFile->m_Handle == INVALID_HANDLE_VALUE )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_flushProfile() [invalid file]");
#endif
        return sal_False;
    }

    if ( pProfile->m_Flags & FLG_MODIFIED )
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("swapping to storeprofile");
#endif
        bRet = storeProfile(pProfile,sal_False);
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_flushProfile() [ok]");
#endif
    return bRet;
}

static sal_Bool writeProfileImpl(osl_TFile* pFile)
{
    DWORD BytesWritten=0;
    BOOL bRet;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_writeProfileImpl()");
#endif

    if ( !( pFile != 0 && pFile->m_Handle != INVALID_HANDLE_VALUE ) || ( pFile->m_pWriteBuf == 0 ) )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_writeProfileImpl() [invalid args]");
#endif
        return sal_False;
    }

    bRet=WriteFile(pFile->m_Handle, pFile->m_pWriteBuf, pFile->m_nWriteBufLen - pFile->m_nWriteBufFree,&BytesWritten,NULL);

    if ( bRet == 0 || BytesWritten == 0 )
    {
        OSL_ENSURE(bRet,"WriteFile failed!!!");
        OSL_TRACE("write failed '%s'",strerror(errno));

        return sal_False;
    }

    free(pFile->m_pWriteBuf);
    pFile->m_pWriteBuf=0;
    pFile->m_nWriteBufLen=0;
    pFile->m_nWriteBufFree=0;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_writeProfileImpl() [ok]");
#endif
    return sal_True;
}

sal_Bool SAL_CALL osl_readProfileString(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_Char* pszString, sal_uInt32 MaxLen,
                              const sal_Char* pszDefault)
{
    sal_uInt32    NoEntry;
    const sal_Char* pStr = 0;
    osl_TProfileImpl*    pProfile = 0;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_readProfileString");
#endif

    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_readProfileString [pProfile==0]");
#endif

        return sal_False;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        osl_TProfileSection* pSec;
        if (((pSec = findEntry(pProfile, pszSection, pszEntry, &NoEntry)) != NULL) &&
            (NoEntry < pSec->m_NoEntries) &&
            ((pStr = strchr(pProfile->m_Lines[pSec->m_Entries[NoEntry].m_Line],
                            '=')) != NULL))
            pStr++;
        else
            pStr = pszDefault;

        if ( pStr != 0 )
        {
            pStr = stripBlanks(pStr, NULL);
            MaxLen = (MaxLen - 1 < strlen(pStr)) ? (MaxLen - 1) : strlen(pStr);
            pStr = stripBlanks(pStr, &MaxLen);
            strncpy(pszString, pStr, MaxLen);
            pszString[MaxLen] = '\0';
        }
    }
    else
    {
        ::osl::LongPathBuffer< sal_Char > aFileName( MAX_LONG_PATH );

        WideCharToMultiByte(CP_ACP,0, reinterpret_cast<LPCWSTR>(pProfile->m_strFileName->buffer), -1, aFileName, aFileName.getBufSizeInSymbols(), NULL, NULL);
        GetPrivateProfileString(pszSection, pszEntry, pszDefault, pszString, MaxLen, aFileName);
    }

    releaseProfile(pProfile);

    if ( pStr == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_readProfileString [pStr==0]");
#endif

        return sal_False;
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_readProfileString [ok]");
#endif

    return sal_True;
}

sal_Bool SAL_CALL osl_readProfileBool(oslProfile Profile,
                            const sal_Char* pszSection, const sal_Char* pszEntry,
                            sal_Bool Default)
{
    sal_Char Line[32];

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_readProfileBool");
#endif

    if (osl_readProfileString(Profile, pszSection, pszEntry, Line, sizeof(Line), ""))
    {
        if ((stricmp(Line, STR_INI_BOOLYES) == 0) ||
            (stricmp(Line, STR_INI_BOOLON)  == 0) ||
            (stricmp(Line, STR_INI_BOOLONE) == 0))
            Default = sal_True;
        else
            if ((stricmp(Line, STR_INI_BOOLNO)   == 0) ||
                (stricmp(Line, STR_INI_BOOLOFF)  == 0) ||
                (stricmp(Line, STR_INI_BOOLZERO) == 0))
                Default = sal_False;
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_readProfileBool [ok]");
#endif

    return Default;
}

sal_uInt32 SAL_CALL osl_readProfileIdent(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Default)
{
    sal_uInt32    i;
    sal_Char        Line[256];

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_readProfileIdent");
#endif

    if (osl_readProfileString(Profile, pszSection, pszEntry, Line, sizeof(Line), ""))
    {
        i = 0;
        while (Strings[i] != NULL)
        {
            if (stricmp(Line, Strings[i]) == 0)
            {
                Default = i + FirstId;
                break;
            }
            i++;
        }
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_readProfileIdent [ok]");
#endif
    return Default;
}

sal_Bool SAL_CALL osl_writeProfileString(oslProfile Profile,
                               const sal_Char* pszSection, const sal_Char* pszEntry,
                               const sal_Char* pszString)
{
    sal_uInt32    i;
    sal_Bool bRet = sal_False;
    sal_uInt32    NoEntry;
    const sal_Char* pStr;
    sal_Char        Line[4096];
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = 0;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_writeProfileString");
#endif

    pProfile = acquireProfile(Profile, sal_True);

    if (pProfile == NULL)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_writeProfileString [pProfile==0]");
#endif
        return sal_False;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if ((pSec = findEntry(pProfile, pszSection, pszEntry, &NoEntry)) == NULL)
        {
            Line[0] = '\0';
            addLine(pProfile, Line);

            Line[0] = '[';
            strcpy(&Line[1], pszSection);
            Line[1 + strlen(pszSection)] = ']';
            Line[2 + strlen(pszSection)] = '\0';

            if (((pStr = addLine(pProfile, Line)) == NULL) ||
                (! addSection(pProfile, pProfile->m_NoLines - 1, &pStr[1], strlen(pszSection))))
            {
                releaseProfile(pProfile);
#ifdef TRACE_OSL_PROFILE
                OSL_TRACE("Out osl_writeProfileString [not added]");
#endif
                return sal_False;
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

            if (((pStr = insertLine(pProfile, Line, i)) == NULL) ||
                (! addEntry(pProfile, pSec, i, pStr, strlen(pszEntry))))
            {
                releaseProfile(pProfile);
#ifdef TRACE_OSL_PROFILE
                OSL_TRACE("Out osl_writeProfileString [not inserted]");
#endif
                return sal_False;
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
        ::osl::LongPathBuffer< sal_Char > aFileName( MAX_LONG_PATH );

        WideCharToMultiByte(CP_ACP,0, reinterpret_cast<LPCWSTR>(pProfile->m_strFileName->buffer), -1, aFileName, aFileName.getBufSizeInSymbols(), NULL, NULL);
        WritePrivateProfileString(pszSection, pszEntry, pszString, aFileName);
    }

    bRet = releaseProfile(pProfile);
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_writeProfileString [ok]");
#endif
    return bRet;
}

sal_Bool SAL_CALL osl_writeProfileBool(oslProfile Profile,
                             const sal_Char* pszSection, const sal_Char* pszEntry,
                             sal_Bool Value)
{
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_writeProfileBool");
#endif

    if (Value)
        bRet=osl_writeProfileString(Profile, pszSection, pszEntry, STR_INI_BOOLONE);
    else
        bRet=osl_writeProfileString(Profile, pszSection, pszEntry, STR_INI_BOOLZERO);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_writeProfileBool [ok]");
#endif

    return bRet;
}

sal_Bool SAL_CALL osl_writeProfileIdent(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Value)
{
    int i, n;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_writeProfileIdent");
#endif

    for (n = 0; Strings[n] != NULL; n++);

    if ((i = Value - FirstId) >= n)
        bRet=sal_False;
    else
        bRet=osl_writeProfileString(Profile, pszSection, pszEntry, Strings[i]);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_writeProfileIdent");
#endif
    return bRet;
}

sal_Bool SAL_CALL osl_removeProfileEntry(oslProfile Profile,
                               const sal_Char *pszSection, const sal_Char *pszEntry)
{
    sal_uInt32    NoEntry;
    osl_TProfileImpl*    pProfile = 0;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_removeProfileEntry");
#endif

    pProfile = acquireProfile(Profile, sal_True);

    if (pProfile == NULL)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_removeProfileEntry [pProfile==0]");
#endif

        return sal_False;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        osl_TProfileSection* pSec;
        if (((pSec = findEntry(pProfile, pszSection, pszEntry, &NoEntry)) != NULL) &&
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
        ::osl::LongPathBuffer< sal_Char > aFileName( MAX_LONG_PATH );

        WideCharToMultiByte(CP_ACP,0, reinterpret_cast<LPCWSTR>(pProfile->m_strFileName->buffer), -1, aFileName, aFileName.getBufSizeInSymbols(), NULL, NULL);
        WritePrivateProfileString(pszSection, pszEntry, NULL, aFileName);
    }

    bRet = releaseProfile(pProfile);
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_removeProfileEntry [ok]");
#endif
    return bRet;
}

sal_uInt32 SAL_CALL osl_getProfileSectionEntries(oslProfile Profile, const sal_Char *pszSection,
                                       sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_uInt32    i, n = 0;
    sal_uInt32    NoEntry;
    osl_TProfileImpl*    pProfile = 0;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_getProfileSectionEntries");
#endif

    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_getProfileSectionEntries [pProfile=0]");
#endif

        return 0;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        osl_TProfileSection* pSec;
        if ((pSec = findEntry(pProfile, pszSection, "", &NoEntry)) != NULL)
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
    else
    {
        ::osl::LongPathBuffer< sal_Char > aFileName( MAX_LONG_PATH );

        WideCharToMultiByte(CP_ACP,0, reinterpret_cast<LPCWSTR>(pProfile->m_strFileName->buffer), -1, aFileName, aFileName.getBufSizeInSymbols(), NULL, NULL);
        n = GetPrivateProfileString(pszSection, NULL, NULL, pszBuffer, MaxLen, aFileName);
    }

    releaseProfile(pProfile);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_getProfileSectionEntries [ok]");
#endif

    return n;
}

sal_Bool SAL_CALL osl_getProfileName(rtl_uString* strPath, rtl_uString* strName, rtl_uString** strProfileName)
{
    sal_Bool bFailed;
    ::osl::LongPathBuffer< sal_Unicode > aFile( MAX_LONG_PATH );
    ::osl::LongPathBuffer< sal_Unicode > aPath( MAX_LONG_PATH );
    sal_uInt32  nFileLen = 0;
    sal_uInt32  nPathLen = 0;

    rtl_uString * strTmp = NULL;
    oslFileError nError;

    /* build file name */
    if (strName && strName->length)
    {
        if( ::sal::static_int_cast< sal_uInt32 >( strName->length ) >= aFile.getBufSizeInSymbols() )
            return sal_False;

        copy_ustr_n( aFile, strName->buffer, strName->length+1);
        nFileLen = strName->length;

        if (rtl_ustr_indexOfChar( aFile, L'.' ) == -1)
        {
            if (nFileLen + wcslen(STR_INI_EXTENSION) >= aFile.getBufSizeInSymbols())
                return sal_False;

            /* add default extension */
            copy_ustr_n( aFile + nFileLen, STR_INI_EXTENSION, wcslen(STR_INI_EXTENSION)+1 );
            nFileLen += wcslen(STR_INI_EXTENSION);
        }
    }
    else
    {
        rtl_uString *strProgName = NULL;
        sal_Unicode *pProgName;
        sal_Int32 nOffset = 0;
        sal_Int32 nLen;
        sal_Int32 nPos;

        if (osl_getExecutableFile(&strProgName) != osl_Process_E_None)
            return sal_False;

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
            return sal_False;

        copy_ustr_n(aFile, pProgName + nOffset, nFileLen);

        if (nFileLen + wcslen(STR_INI_EXTENSION) >= aFile.getBufSizeInSymbols())
            return sal_False;

        /* add default extension */
        copy_ustr_n(aFile + nFileLen, STR_INI_EXTENSION, wcslen(STR_INI_EXTENSION)+1);
        nFileLen += wcslen(STR_INI_EXTENSION);

        rtl_uString_release( strProgName );
    }

    if (aFile[0] == 0)
        return sal_False;

    /* build directory path */
    if (strPath && strPath->length)
    {
        sal_Unicode *pPath = rtl_uString_getStr(strPath);
        sal_Int32 nLen = rtl_uString_getLength(strPath);

        if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METAHOME) , STR_INI_METAHOME) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METAHOME)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METAHOME)] == '/')))
        {
            rtl_uString * strHome = NULL;
            oslSecurity security = osl_getCurrentSecurity();

            bFailed = ! osl_getHomeDir(security, &strHome);
            osl_freeSecurityHandle(security);

            if (bFailed) return sal_False;

            if ( ::sal::static_int_cast< sal_uInt32 >( strHome->length ) >= aPath.getBufSizeInSymbols())
                return sal_False;

            copy_ustr_n( aPath, strHome->buffer, strHome->length+1);
            nPathLen = strHome->length;

            if (nLen > RTL_CONSTASCII_LENGTH(STR_INI_METAHOME))
            {
                pPath += RTL_CONSTASCII_LENGTH(STR_INI_METAHOME);
                nLen -= RTL_CONSTASCII_LENGTH(STR_INI_METAHOME);

                if (nLen + nPathLen >= aPath.getBufSizeInSymbols())
                    return sal_False;

                copy_ustr_n(aPath + nPathLen, pPath, nLen+1);
                nPathLen += nLen;
            }

            rtl_uString_release(strHome);
        }

        else if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METACFG), STR_INI_METACFG) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METACFG)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METACFG)] == '/')))
        {
            rtl_uString * strConfig = NULL;
            oslSecurity security = osl_getCurrentSecurity();

            bFailed = ! osl_getConfigDir(security, &strConfig);
            osl_freeSecurityHandle(security);

            if (bFailed) return sal_False;

            if ( ::sal::static_int_cast< sal_uInt32 >( strConfig->length ) >= aPath.getBufSizeInSymbols())
                return sal_False;

            copy_ustr_n( aPath, strConfig->buffer, strConfig->length+1 );
            nPathLen = strConfig->length;

            if (nLen > RTL_CONSTASCII_LENGTH(STR_INI_METACFG))
            {
                pPath += RTL_CONSTASCII_LENGTH(STR_INI_METACFG);
                nLen -= RTL_CONSTASCII_LENGTH(STR_INI_METACFG);

                if (nLen + nPathLen >= aPath.getBufSizeInSymbols())
                    return sal_False;

                copy_ustr_n(aPath + nPathLen, pPath, nLen+1);
                nPathLen += nLen;
            }

            rtl_uString_release(strConfig);
        }

        else if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METASYS), STR_INI_METASYS) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METASYS)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METASYS)] == '/')))
        {
            if (((nPathLen = GetWindowsDirectoryW(::osl::mingw_reinterpret_cast<LPWSTR>(aPath), aPath.getBufSizeInSymbols())) == 0) || (nPathLen >= aPath.getBufSizeInSymbols()))
                return sal_False;

            if (nLen > RTL_CONSTASCII_LENGTH(STR_INI_METASYS))
            {
                pPath += RTL_CONSTASCII_LENGTH(STR_INI_METASYS);
                nLen -= RTL_CONSTASCII_LENGTH(STR_INI_METASYS);

                if (nLen + nPathLen >= aPath.getBufSizeInSymbols())
                    return sal_False;

                copy_ustr_n(aPath + nPathLen, pPath, nLen+1);
                nPathLen += nLen;
            }
        }

        else if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METAINS), STR_INI_METAINS) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METAINS)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METAINS)] == '/') ||
                (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METAINS)] == '"') ) )
        {
            if (! lookupProfile(pPath + RTL_CONSTASCII_LENGTH(STR_INI_METAINS), aFile, aPath))
                return sal_False;

            nPathLen = rtl_ustr_getLength(aPath);
        }

        else if( ::sal::static_int_cast< sal_uInt32 >( nLen ) < aPath.getBufSizeInSymbols())
        {
            copy_ustr_n(aPath, pPath, nLen+1);
            nPathLen = rtl_ustr_getLength(aPath);
        }
        else
            return sal_False;
    }
    else
    {
        rtl_uString * strConfigDir = NULL;
        oslSecurity security = osl_getCurrentSecurity();

        bFailed = ! osl_getConfigDir(security, &strConfigDir);
        osl_freeSecurityHandle(security);

        if (bFailed) return sal_False;
        if ( ::sal::static_int_cast< sal_uInt32 >( strConfigDir->length ) >= aPath.getBufSizeInSymbols() )
            return sal_False;

        copy_ustr_n(aPath, strConfigDir->buffer, strConfigDir->length+1);
        nPathLen = strConfigDir->length;
    }

    if (nPathLen && (aPath[nPathLen - 1] != L'/') && (aPath[nPathLen - 1] != L'\\'))
    {
        aPath[nPathLen++] = L'\\';
        aPath[nPathLen] = 0;
    }

    if (nPathLen + nFileLen >= aPath.getBufSizeInSymbols())
        return sal_False;

    /* append file name */
    copy_ustr_n(aPath + nPathLen, aFile, nFileLen+1);
    nPathLen += nFileLen;

    /* copy filename */
    rtl_uString_newFromStr_WithLength(&strTmp, aPath, nPathLen);
    nError = osl_getFileURLFromSystemPath(strTmp, strProfileName);
    rtl_uString_release(strTmp);

    return (sal_Bool) (nError == osl_File_E_None);
}

sal_uInt32 SAL_CALL osl_getProfileSections(oslProfile Profile, sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_uInt32    i, n = 0;
    osl_TProfileImpl*    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
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
        ::osl::LongPathBuffer< sal_Char > aFileName( MAX_LONG_PATH );

        WideCharToMultiByte(CP_ACP,0, reinterpret_cast<LPCWSTR>(pProfile->m_strFileName->buffer), -1, aFileName, aFileName.getBufSizeInSymbols(), NULL, NULL);
        n = GetPrivateProfileSectionNames(pszBuffer, MaxLen, aFileName);
    }

    releaseProfile(pProfile);

    return n;
}

/*****************************************************************************/
/* Static Module Functions */
/*****************************************************************************/

static osl_TStamp getFileStamp(osl_TFile* pFile)
{
    FILETIME FileTime;

    if ((pFile->m_Handle == INVALID_HANDLE_VALUE) ||
        (! GetFileTime(pFile->m_Handle, NULL, NULL, &FileTime)))
        memset(&FileTime, 0, sizeof(FileTime));

    return FileTime;
}

static sal_Bool lockFile(const osl_TFile* pFile, osl_TLockMode eMode)
{
    sal_Bool     status = sal_False;
    OVERLAPPED  Overlapped;

    if (pFile->m_Handle == INVALID_HANDLE_VALUE)
        return sal_False;

    memset(&Overlapped, 0, sizeof(Overlapped));

    switch (eMode)
    {
        case un_lock:
            status = (sal_Bool) UnlockFileEx(
                pFile->m_Handle, 0, 0xFFFFFFFF, 0, &Overlapped);
            break;

        case read_lock:
            status = (sal_Bool) LockFileEx(
                pFile->m_Handle, 0, 0, 0xFFFFFFFF, 0, &Overlapped);
            break;

        case write_lock:
            status = (sal_Bool) LockFileEx(
                pFile->m_Handle, LOCKFILE_EXCLUSIVE_LOCK, 0, 0xFFFFFFFF, 0,
                &Overlapped);
            break;
    }

    return status;
}

static osl_TFile* openFileImpl(rtl_uString * strFileName, oslProfileOption ProfileFlags )
{
    osl_TFile* pFile = reinterpret_cast< osl_TFile*>( calloc( 1, sizeof(osl_TFile) ) );
    sal_Bool bWriteable = sal_False;

    if ( ProfileFlags & ( osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE ) )
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("setting bWriteable to TRUE");
#endif
        bWriteable=sal_True;
    }

    if (! bWriteable)
    {
        pFile->m_Handle = CreateFileW( reinterpret_cast<LPCWSTR>(rtl_uString_getStr( strFileName )), GENERIC_READ,
                                          FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        /* mfe: argghh!!! do not check if the file could be openend */
        /*      default mode expects it that way!!!                 */
    }
    else
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("opening '%s' read/write",pszFilename);
#endif

        if ((pFile->m_Handle = CreateFileW( reinterpret_cast<LPCWSTR>(rtl_uString_getStr( strFileName )), GENERIC_READ | GENERIC_WRITE,
                                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL))
            == INVALID_HANDLE_VALUE)
        {
            free(pFile);
            return NULL;
        }
    }

    pFile->m_pWriteBuf=0;
    pFile->m_nWriteBufFree=0;
    pFile->m_nWriteBufLen=0;

    if ( ProfileFlags & (osl_Profile_WRITELOCK | osl_Profile_READLOCK ) )
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("locking '%s' file",pszFilename);
#endif

        lockFile(pFile, bWriteable ? write_lock : read_lock);
    }

    return pFile;
}

static osl_TStamp closeFileImpl(osl_TFile* pFile)
{
    osl_TStamp stamp = {0, 0};

    if ( pFile == 0 )
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

    if ( pFile->m_pWriteBuf != 0 )
    {
        free(pFile->m_pWriteBuf);
    }

    free(pFile);

    return stamp;
}

static sal_Bool rewindFile(osl_TFile* pFile, sal_Bool bTruncate)
{
    if (pFile->m_Handle != INVALID_HANDLE_VALUE)
    {
        pFile->m_pReadPtr = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);

        SetFilePointer(pFile->m_Handle, 0, NULL, FILE_BEGIN);

        if (bTruncate)
            SetEndOfFile(pFile->m_Handle);
    }

    return sal_True;
}

static sal_Bool getLine(osl_TFile* pFile, const sal_Char *pszLine, int MaxLen)
{
    DWORD Max;
    size_t Free;
    sal_Char* pChr;
    sal_Char* pLine = (sal_Char *)pszLine;

    if (pFile->m_Handle == INVALID_HANDLE_VALUE)
        return sal_False;

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

            if (! ReadFile(pFile->m_Handle, &pFile->m_ReadBuf[Bytes], Free, &Max, NULL))
            {
                *pLine = '\0';
                return sal_False;
            }

            if (Max < Free)
            {
                if ((Max == 0) && (pLine == pszLine))
                {
                    *pLine = '\0';
                    return sal_False;
                }

                pFile->m_ReadBuf[Bytes + Max] = '\0';
            }
        }

        for (pChr = pFile->m_pReadPtr;
             (*pChr != '\n') && (*pChr != '\r') && (*pChr != '\0') &&
             (pChr < (pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf) - 1));
             pChr++);

        Max = min((int) (pChr - pFile->m_pReadPtr), MaxLen);
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

    return sal_True;
}

static sal_Bool putLine(osl_TFile* pFile, const sal_Char *pszLine)
{
    unsigned int Len = strlen(pszLine);

    if ( pFile == 0 || pFile->m_Handle == INVALID_HANDLE_VALUE )
    {
        return sal_False;
    }

    if ( pFile->m_pWriteBuf == 0 )
    {
        pFile->m_pWriteBuf = (sal_Char*) malloc(Len+3);
        pFile->m_nWriteBufLen = Len+3;
        pFile->m_nWriteBufFree = Len+3;
    }
    else
    {
        if ( pFile->m_nWriteBufFree <= Len + 3 )
        {
            sal_Char* pTmp;

            pTmp=(sal_Char*) realloc(pFile->m_pWriteBuf,( ( pFile->m_nWriteBufLen + Len ) * 2) );
            if ( pTmp == 0 )
            {
                return sal_False;
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

    return sal_True;
}

/* platform specific end */

static const sal_Char* stripBlanks(const sal_Char* String, sal_uInt32* pLen)
{
    if ( (pLen != NULL) && ( *pLen != 0 ) )
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
        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_MaxLines = LINES_INI;
            pProfile->m_Lines = static_cast<sal_Char **>(calloc(pProfile->m_MaxLines, sizeof(sal_Char *)));
        }
        else
        {
            unsigned int index=0;
            unsigned int oldmax=pProfile->m_MaxLines;

            pProfile->m_MaxLines += LINES_ADD;
            pProfile->m_Lines = static_cast<sal_Char **>(realloc(pProfile->m_Lines, pProfile->m_MaxLines * sizeof(sal_Char *)));

            for ( index = oldmax ; index < pProfile->m_MaxLines ; ++index )
            {
                pProfile->m_Lines[index]=0;
            }
        }

        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_NoLines  = 0;
            pProfile->m_MaxLines = 0;
            return NULL;
        }

    }

    if ( pProfile->m_Lines != 0 && pProfile->m_Lines[pProfile->m_NoLines] != 0 )
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
        if (pProfile->m_Lines == NULL)
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

        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_NoLines  = 0;
            pProfile->m_MaxLines = 0;
            return NULL;
        }
    }

    LineNo = LineNo > pProfile->m_NoLines ? pProfile->m_NoLines : LineNo;

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
        pProfile->m_Lines[LineNo]=0;
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
            pProfile->m_Lines[LineNo] = 0;
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

static sal_Bool addEntry(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection,
                        int Line, const sal_Char* Entry, sal_uInt32 Len)
{
    if (pSection != NULL)
    {
        if (pSection->m_NoEntries >= pSection->m_MaxEntries)
        {
            if (pSection->m_Entries == NULL)
            {
                pSection->m_MaxEntries = ENTRIES_INI;
                pSection->m_Entries = (osl_TProfileEntry *)malloc(
                                pSection->m_MaxEntries * sizeof(osl_TProfileEntry));
            }
            else
            {
                pSection->m_MaxEntries += ENTRIES_ADD;
                pSection->m_Entries = (osl_TProfileEntry *)realloc(pSection->m_Entries,
                                pSection->m_MaxEntries * sizeof(osl_TProfileEntry));
            }

            if (pSection->m_Entries == NULL)
            {
                pSection->m_NoEntries  = 0;
                pSection->m_MaxEntries = 0;
                return sal_False;
            }
        }

        pSection->m_NoEntries++;

        Entry = stripBlanks(Entry, &Len);
        setEntry(pProfile, pSection, pSection->m_NoEntries - 1, Line,
                 Entry, Len);

        return sal_True;
    }

    return sal_False;
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

static sal_Bool addSection(osl_TProfileImpl* pProfile, int Line, const sal_Char* Section, sal_uInt32 Len)
{
    if (pProfile->m_NoSections >= pProfile->m_MaxSections)
    {
        if (pProfile->m_Sections == NULL)
        {
            pProfile->m_MaxSections = SECTIONS_INI;
            pProfile->m_Sections = static_cast<osl_TProfileSection*>(calloc(pProfile->m_MaxSections, sizeof(osl_TProfileSection)));
        }
        else
        {
            unsigned int index=0;
            unsigned int oldmax=pProfile->m_MaxSections;

            pProfile->m_MaxSections += SECTIONS_ADD;
            pProfile->m_Sections = static_cast<osl_TProfileSection*>(realloc(pProfile->m_Sections,
                                          pProfile->m_MaxSections * sizeof(osl_TProfileSection)));
            for ( index = oldmax ; index < pProfile->m_MaxSections ; ++index )
            {
                pProfile->m_Sections[index].m_Entries=0;
            }
        }

        if (pProfile->m_Sections == NULL)
        {
            pProfile->m_NoSections = 0;
            pProfile->m_MaxSections = 0;
            return sal_False;
        }
    }

    pProfile->m_NoSections++;

    if ( pProfile->m_Sections[(pProfile->m_NoSections) - 1].m_Entries != 0 )
    {
        free(pProfile->m_Sections[(pProfile->m_NoSections) - 1].m_Entries);
    }
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Entries    = NULL;
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_NoEntries  = 0;
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_MaxEntries = 0;

    Section = (sal_Char *)stripBlanks(Section, &Len);
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Line = Line;
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Offset = Section - pProfile->m_Lines[Line];
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Len = Len;

    return sal_True;
}

static void removeSection(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection)
{
    sal_uInt32 Section;

    if ((Section = pSection - pProfile->m_Sections) < pProfile->m_NoSections)
    {
        free (pSection->m_Entries);
        pSection->m_Entries=0;
        if (pProfile->m_NoSections - Section > 1)
        {
            memmove(&pProfile->m_Sections[Section], &pProfile->m_Sections[Section + 1],
                    (pProfile->m_NoSections - Section - 1) * sizeof(osl_TProfileSection));

            memset(&pProfile->m_Sections[pProfile->m_NoSections - 1],
                0,
                (pProfile->m_MaxSections - pProfile->m_NoSections) * sizeof(osl_TProfileSection));
            pProfile->m_Sections[pProfile->m_NoSections - 1].m_Entries = 0;
        }
        else
        {
            pSection->m_Entries = 0;
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
        osl_TProfileSection* pSec = NULL;

    Len = strlen(Section);
    Section = (sal_Char *)stripBlanks(Section, &Len);

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
        pSec = NULL;

    return pSec;
}

static sal_Bool loadProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile)
{
    sal_uInt32    i;
    sal_Char*       pStr;
    sal_Char*       pChar;
    sal_Char        Line[4096];

    pProfile->m_NoLines    = 0;
    pProfile->m_NoSections = 0;

    OSL_VERIFY(rewindFile(pFile, sal_False));

    while (getLine(pFile, Line, sizeof(Line)))
    {
        if (! addLine(pProfile, Line))
            return sal_False;
    }

    for (i = 0; i < pProfile->m_NoLines; i++)
    {
        pStr = (sal_Char *)stripBlanks(pProfile->m_Lines[i], NULL);

        if ((*pStr == '\0') || (*pStr == ';'))
            continue;

        if ((*pStr != '[') || ((pChar = strrchr(pStr, ']')) == NULL) ||
            ((pChar - pStr) <= 2))
        {
            /* insert entry */

            if (pProfile->m_NoSections < 1)
                continue;

            if ((pChar = strchr(pStr, '=')) == NULL)
                pChar = pStr + strlen(pStr);

            if (! addEntry(pProfile, &pProfile->m_Sections[pProfile->m_NoSections - 1],
                           i, pStr, pChar - pStr))
                return sal_False;
        }
        else
        {
            /* new section */
            if (! addSection(pProfile, i, pStr + 1, pChar - pStr - 1))
                return sal_False;
        }
    }

    return sal_True;
}

static sal_Bool storeProfile(osl_TProfileImpl* pProfile, sal_Bool bCleanup)
{
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  storeProfile");
#endif

    if (pProfile->m_Lines != NULL)
    {
        if (pProfile->m_Flags & FLG_MODIFIED)
        {
            sal_uInt32 i;

            osl_TFile* pTmpFile = osl_openTmpProfileImpl(pProfile);

            if ( pTmpFile == 0 )
            {
                return sal_False;
            }

            OSL_VERIFY(rewindFile(pTmpFile, sal_True));

            for (i = 0; i < pProfile->m_NoLines; i++)
            {
                OSL_VERIFY(putLine(pTmpFile, pProfile->m_Lines[i]));
            }

            if ( ! writeProfileImpl(pTmpFile) )
            {
                if ( pTmpFile->m_pWriteBuf != 0 )
                {
                    free(pTmpFile->m_pWriteBuf);
                }

                pTmpFile->m_pWriteBuf=0;
                pTmpFile->m_nWriteBufLen=0;
                pTmpFile->m_nWriteBufFree=0;

#ifdef TRACE_OSL_PROFILE
                OSL_TRACE("Out storeProfile [not flushed]");
#endif
                closeFileImpl(pTmpFile);

                return sal_False;
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
            pProfile->m_Lines = NULL;
            pProfile->m_MaxLines = 0;

            while (pProfile->m_NoSections > 0)
                removeSection(pProfile, &pProfile->m_Sections[pProfile->m_NoSections - 1]);

            free(pProfile->m_Sections);
            pProfile->m_Sections = NULL;
            pProfile->m_MaxSections = 0;
        }
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out storeProfile [ok]");
#endif
    return sal_True;
}

static osl_TFile* osl_openTmpProfileImpl(osl_TProfileImpl* pProfile)
{
    osl_TFile* pFile=0;
    rtl_uString* ustrExtension=0;
    rtl_uString* ustrTmpName=0;
    oslProfileOption PFlags=0;

    rtl_uString_newFromAscii(&ustrExtension,"tmp");

    /* generate tmp profilename */
    ustrTmpName=osl_ProfileGenerateExtension(pProfile->m_strFileName,ustrExtension);
    rtl_uString_release(ustrExtension);

    if ( ustrTmpName == 0 )
    {
        return 0;
    }

    if ( ! ( pProfile->m_Flags & osl_Profile_READLOCK ) )
    {
        PFlags |= osl_Profile_WRITELOCK;
    }

    /* open this file */
    pFile = openFileImpl(ustrTmpName,pProfile->m_Flags | PFlags);

    /* return new pFile */
    return pFile;
}

static sal_Bool osl_ProfileSwapProfileNames(osl_TProfileImpl* pProfile)
{
    sal_Bool bRet = sal_False;

    rtl_uString* ustrBakFile=0;
    rtl_uString* ustrTmpFile=0;
    rtl_uString* ustrIniFile=0;
    rtl_uString* ustrExtension=0;

    rtl_uString_newFromAscii(&ustrExtension,"bak");

    ustrBakFile=osl_ProfileGenerateExtension(pProfile->m_strFileName,ustrExtension);
    rtl_uString_release(ustrExtension);
    ustrExtension=0;

    rtl_uString_newFromAscii(&ustrExtension,"ini");

    ustrIniFile=osl_ProfileGenerateExtension(pProfile->m_strFileName,ustrExtension);
    rtl_uString_release(ustrExtension);
    ustrExtension=0;

    rtl_uString_newFromAscii(&ustrExtension,"tmp");

    ustrTmpFile=osl_ProfileGenerateExtension(pProfile->m_strFileName,ustrExtension);
    rtl_uString_release(ustrExtension);
    ustrExtension=0;

    /* unlink bak */
    DeleteFileW( reinterpret_cast<LPCWSTR>(rtl_uString_getStr( ustrBakFile )) );

    /* rename ini bak */
    MoveFileExW( reinterpret_cast<LPCWSTR>(rtl_uString_getStr( ustrIniFile )), reinterpret_cast<LPCWSTR>(rtl_uString_getStr( ustrBakFile )), MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH );

    /* rename tmp ini */
    MoveFileExW( reinterpret_cast<LPCWSTR>(rtl_uString_getStr( ustrTmpFile )), reinterpret_cast<LPCWSTR>(rtl_uString_getStr( ustrIniFile )), MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH );

    return bRet;
}

static rtl_uString* osl_ProfileGenerateExtension(rtl_uString* ustrFileName, rtl_uString* ustrExtension)
{
    rtl_uString* ustrNewFileName = 0;
    rtl_uString* ustrOldExtension = 0;

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

static osl_TProfileImpl* acquireProfile(oslProfile Profile, sal_Bool bWriteable)
{
    osl_TProfileImpl* pProfile = (osl_TProfileImpl*)Profile;
    oslProfileOption PFlags=0;

    if ( bWriteable )
    {
        PFlags = osl_Profile_DEFAULT | osl_Profile_WRITELOCK;
    }
    else
    {
        PFlags = osl_Profile_DEFAULT;
    }

    if (pProfile == NULL)
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("AUTOOPEN MODE");
#endif

        if ( ( pProfile = (osl_TProfileImpl*)osl_openProfile( NULL, PFlags ) ) != NULL )
        {
            pProfile->m_Flags |= FLG_AUTOOPEN;
        }
    }
    else
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("try to acquire");
#endif

        if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
        {
            if (! (pProfile->m_Flags & (osl_Profile_READLOCK |
                                        osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE)))
            {
                osl_TStamp Stamp;
#ifdef DEBUG_OSL_PROFILE
                OSL_TRACE("DEFAULT MODE");
#endif
                pProfile->m_pFile = openFileImpl(
                    pProfile->m_strFileName, pProfile->m_Flags | PFlags);
                if (!pProfile->m_pFile)
                    return NULL;

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
                OSL_TRACE("READ/WRITELOCK MODE");
#endif

                /* A readlock file could not be written */
                if ((pProfile->m_Flags & osl_Profile_READLOCK) && bWriteable)
                {
                    return NULL;
                }
            }
        }
    }

    return pProfile;
}

static sal_Bool releaseProfile(osl_TProfileImpl* pProfile)
{
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  releaseProfile");
#endif

    if ( pProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out releaseProfile [profile==0]");
#endif
        return sal_False;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if (pProfile->m_Flags & FLG_AUTOOPEN)
        {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out releaseProfile [AUTOOPEN]");
#endif
            return osl_closeProfile((oslProfile)pProfile);
        }
        else
        {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("DEFAULT MODE");
#endif
        if (! (pProfile->m_Flags & (osl_Profile_READLOCK |
                                    osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE)))
            {
                if (pProfile->m_Flags & FLG_MODIFIED)
                    storeProfile(pProfile, sal_False);

                closeFileImpl(pProfile->m_pFile);
                pProfile->m_pFile = NULL;
            }
        }
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out releaseProfile [ok]");
#endif
    return sal_True;
}

static sal_Bool lookupProfile(const sal_Unicode *strPath, const sal_Unicode *strFile, sal_Unicode *strProfile)
{
    sal_Char *pChr;
    sal_Char Buffer[4096] = "";
    sal_Char Product[132] = "";

    ::osl::LongPathBuffer< sal_Unicode > aPath( MAX_LONG_PATH );
    aPath[0] = 0;
    DWORD dwPathLen = 0;

    if (*strPath == L'"')
    {
        int i = 0;

        strPath++;

        while ((strPath[i] != L'"') && (strPath[i] != L'\0'))
            i++;

        WideCharToMultiByte(CP_ACP,0, reinterpret_cast<LPCWSTR>(strPath), i, Product, sizeof(Product), NULL, NULL);
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
        /* if we have not product identfication, do a special handling for soffice.ini */
        if (rtl_ustr_ascii_compare(strFile, SVERSION_PROFILE) == 0)
        {
            rtl_uString * strSVProfile  = NULL;
            rtl_uString * strSVFallback = NULL;
            rtl_uString * strSVLocation = NULL;
            rtl_uString * strSVName     = NULL;
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
                strSVProfile = NULL;
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
            if ((pChr = strrchr(Product, '/')) != NULL)
                *pChr = '\0';
        }
    }

    /* if we have an userid option eg. "-userid:rh[/usr/home/rh/staroffice]",
       this will supersede all other locations */
    {
        sal_uInt32 n, nArgs = osl_getCommandArgCount();

        for (n = 0; n < nArgs; n++)
        {
            rtl_uString * strCommandArg = NULL;
            osl_getCommandArg( n, &strCommandArg );
            if (((strCommandArg->buffer[0] == L'-') || (strCommandArg->buffer[0] == L'+')) &&
                (rtl_ustr_ascii_compare_WithLength(strCommandArg->buffer, RTL_CONSTASCII_LENGTH(SVERSION_OPTION), SVERSION_OPTION)))
            {
                sal_Unicode *pCommandArg = strCommandArg->buffer + RTL_CONSTASCII_LENGTH(SVERSION_OPTION);
                sal_Int32 nStart, nEnd;

                if (((nStart = rtl_ustr_indexOfChar(pCommandArg, L'[')) != -1) &&
                    ((nEnd = rtl_ustr_indexOfChar(pCommandArg + nStart + 1, L']')) != -1))
                {
                    dwPathLen = nEnd;
                    copy_ustr_n(aPath, pCommandArg + nStart + 1, dwPathLen);
                    aPath[dwPathLen] = 0;

                    /* build full path */
                    if ((aPath[dwPathLen - 1] != L'/') && (aPath[dwPathLen - 1] != L'\\'))
                    {
                        copy_ustr_n(aPath + dwPathLen++, L"/", 2);
                    }

                    if (*strPath)
                    {
                        copy_ustr_n(aPath + dwPathLen, strPath, rtl_ustr_getLength(strPath)+1);
                        dwPathLen += rtl_ustr_getLength(strPath);
                    }
                    else
                    {
                        ::osl::LongPathBuffer< sal_Char > aTmpPath( MAX_LONG_PATH );
                        int nLen = 0;

                        if ((nLen = WideCharToMultiByte(CP_ACP,0, ::osl::mingw_reinterpret_cast<LPCWSTR>(aPath), -1, aTmpPath, aTmpPath.getBufSizeInSymbols(), NULL, NULL)) > 0)
                        {
                            strcpy(aTmpPath + nLen, SVERSION_USER);
                            if (access(aTmpPath, 0) >= 0)
                            {
                                dwPathLen += MultiByteToWideChar( CP_ACP, 0, SVERSION_USER, -1, reinterpret_cast<LPWSTR>(aPath + dwPathLen), aPath.getBufSizeInSymbols() - dwPathLen );
                            }
                        }
                    }

                    break;
                }
            }
        }
    }

    if (dwPathLen == 0)
    {
        rtl_uString * strExecutable = NULL;
        rtl_uString * strTmp = NULL;
        sal_Int32 nPos;

        /* try to find the file in the directory of the executbale */
        if (osl_getExecutableFile(&strTmp) != osl_Process_E_None)
            return sal_False;

        /* convert to native path */
        if (osl_getSystemPathFromFileURL(strTmp, &strExecutable) != osl_File_E_None)
        {
            rtl_uString_release(strTmp);
            return sal_False;
        }

        rtl_uString_release(strTmp);

        /* separate path from filename */
        if ((nPos = rtl_ustr_lastIndexOfChar(strExecutable->buffer, L'\\')) == -1)
        {
            if ((nPos = rtl_ustr_lastIndexOfChar(strExecutable->buffer, L':')) == -1)
            {
                return sal_False;
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
            WideCharToMultiByte(CP_ACP,0, reinterpret_cast<LPCWSTR>(strExecutable->buffer + nPos + 1), -1, Product, sizeof(Product), NULL, NULL);

            /* remove extension */
            if ((pChr = strrchr(Product, '.')) != NULL)
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

            WideCharToMultiByte(CP_ACP,0, ::osl::mingw_reinterpret_cast<LPCWSTR>(aPath), -1, aTmpPath, aTmpPath.getBufSizeInSymbols(), NULL, NULL);

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
                                dwPathLen = nPos + MultiByteToWideChar( CP_ACP, 0, SVERSION_USER, -1, reinterpret_cast<LPWSTR>(aPath + nPos + 1), aPath.getBufSizeInSymbols() - (nPos + 1) );
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

            WideCharToMultiByte(CP_ACP,0, ::osl::mingw_reinterpret_cast<LPCWSTR>(aPath), -1, aTmpPath, aTmpPath.getBufSizeInSymbols(), NULL, NULL);

            if ((access(aTmpPath, 0) < 0) && (Product[0] != '\0'))
            {
                rtl_uString * strSVFallback = NULL;
                rtl_uString * strSVProfile  = NULL;
                rtl_uString * strSVLocation = NULL;
                rtl_uString * strSVName     = NULL;
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
                                CP_ACP, 0, Buffer, -1, ::osl::mingw_reinterpret_cast<LPWSTR>(aPath), aPath.getBufSizeInSymbols() );
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
                                         CP_ACP,0, ::osl::mingw_reinterpret_cast<LPCWSTR>(aPath), -1, aTmpPath2,
                                         aTmpPath2.getBufSizeInSymbols(), NULL, NULL))
                                    > 0)
                                {
                                    strcpy(aTmpPath2 + n, SVERSION_USER);
                                    if (access(aTmpPath2, 0) >= 0)
                                    {
                                        dwPathLen += MultiByteToWideChar(
                                            CP_ACP, 0, SVERSION_USER, -1,
                                            reinterpret_cast<LPWSTR>(aPath + dwPathLen),
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
    }

    /* copy filename */
    copy_ustr_n(strProfile, aPath, dwPathLen+1);

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
