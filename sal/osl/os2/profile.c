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

#include "system.h"
#include <sal/macros.h>

#include <osl/security.h>
#include <osl/diagnose.h>
#include <osl/profile.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <osl/file.h>

#define LINES_INI       32
#define LINES_ADD       10
#define SECTIONS_INI    5
#define SECTIONS_ADD    3
#define ENTRIES_INI     5
#define ENTRIES_ADD     3


#define STR_INI_EXTENSION   ".ini"
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

#define _BUILD_STR_(n)  # n
#define BUILD_STR(n)    _BUILD_STR_(n)

/* implemented in file.c */
extern oslFileError FileURLToPath( char *, size_t, rtl_uString* );

/*****************************************************************************/
/* Data Type Definition */
/*****************************************************************************/

typedef struct _osl_TStamp
{
    FDATE  m_Date;
    FTIME  m_Time;
} osl_TStamp;

typedef enum _osl_TLockMode
{
    un_lock, read_lock, write_lock
} osl_TLockMode;

typedef struct _osl_TFile
{
    HFILE   m_Handle;
/*
    sal_Char*   m_pReadPtr;
    sal_Char    m_ReadBuf[512];
    sal_Char*   m_pWritePtr;
    sal_Char    m_WriteBuf[512];
*/
    sal_Char*   m_pReadPtr;
    sal_Char    m_ReadBuf[512];
/*      sal_Char*   m_pWritePtr; */
/*      sal_Char    m_WriteBuf[512]; */
    sal_Char*   m_pWriteBuf;
    sal_uInt32  m_nWriteBufLen;
    sal_uInt32  m_nWriteBufFree;
} osl_TFile;

typedef struct _osl_TProfileEntry
{
    sal_uInt32    m_Line;
    sal_uInt32    m_Offset;
    sal_uInt32    m_Len;
} osl_TProfileEntry;

typedef struct _osl_TProfileSection
{
    sal_uInt32            m_Line;
    sal_uInt32            m_Offset;
    sal_uInt32            m_Len;
    sal_uInt32            m_NoEntries;
    sal_uInt32            m_MaxEntries;
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
    //sal_Char    m_Filename[_MAX_PATH + 1];
    sal_uInt32  m_NoLines;
    sal_uInt32  m_MaxLines;
    sal_uInt32  m_NoSections;
    sal_uInt32  m_MaxSections;
    sal_Char**  m_Lines;
    rtl_uString *m_strFileName;
    osl_TProfileSection*    m_Sections;
    HINI                    m_hIni;
} osl_TProfileImpl;


/*****************************************************************************/
/* Static Module Function Declarations */
/*****************************************************************************/

//static osl_TFile* openFile(rtl_uString* pszFilename, sal_Bool bWriteable);
//static osl_TStamp closeFile(osl_TFile* pFile);
static osl_TFile*           openFileImpl(rtl_uString * strFileName, oslProfileOption ProfileFlags  );
static osl_TStamp           closeFileImpl(osl_TFile* pFile);
static sal_Bool   lockFile(const osl_TFile* pFile, osl_TLockMode eMode);
static sal_Bool   rewindFile(osl_TFile* pFile, sal_Bool bTruncate);
static osl_TStamp getFileStamp(osl_TFile* pFile);

static sal_Bool getLine(osl_TFile* pFile, const sal_Char *pszLine, int MaxLen);
static sal_Bool putLine(osl_TFile* pFile, const sal_Char *pszLine);
static const sal_Char* stripBlanks(const sal_Char* String, sal_uInt32* pLen);
static const sal_Char* addLine(osl_TProfileImpl* pProfile, const sal_Char* Line);
static const sal_Char* insertLine(osl_TProfileImpl* pProfile, const sal_Char* Line, sal_uInt32 LineNo);
static void removeLine(osl_TProfileImpl* pProfile, sal_uInt32 LineNo);
static void setEntry(osl_TProfileImpl* pProfile, osl_TProfileSection* pSection,
                     sal_uInt32 NoEntry, sal_uInt32 Line,
                     const sal_Char* Entry, sal_uInt32 Len);
static sal_Bool addEntry(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection,
                         int Line, const sal_Char* Entry, sal_uInt32 Len);
static void removeEntry(osl_TProfileSection *pSection, sal_uInt32 NoEntry);
static sal_Bool addSection(osl_TProfileImpl* pProfile, int Line, const sal_Char* Section, sal_uInt32 Len);
static void removeSection(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection);
static osl_TProfileSection* findEntry(osl_TProfileImpl* pProfile, const sal_Char* Section,
                                      const sal_Char* Entry, sal_uInt32 *pNoEntry);
static sal_Bool loadProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile);
static sal_Bool storeProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile, sal_Bool bCleanup);
static osl_TProfileImpl* acquireProfile(oslProfile Profile, sal_Bool bWriteable);
static sal_Bool releaseProfile(osl_TProfileImpl* pProfile);
static sal_Bool lookupProfile(const sal_Char *pszPath, const sal_Char *pszFile, sal_Char *pPath);


static sal_Bool SAL_CALL osl_getProfileName(rtl_uString* strPath, rtl_uString* strName, rtl_uString** strProfileName);

sal_Bool SAL_CALL osl_getFullPath(rtl_uString* pszFilename, sal_Char* pszPath, sal_uInt32 MaxLen)
{
    return NO_ERROR == DosQueryPathInfo( (PCSZ)pszFilename, FIL_QUERYFULLNAME, pszPath, MaxLen);
}



/*****************************************************************************/
/* Exported Module Functions */
/*****************************************************************************/

oslProfile SAL_CALL osl_openProfile(rtl_uString *strProfileName, sal_uInt32 Flags)
{
    osl_TFile*        pFile;
    osl_TProfileImpl* pProfile;
    rtl_uString       *FileName=NULL;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_openProfile\n");
#endif
    OSL_VERIFY(strProfileName);

/*  if (rtl_uString_getLength(strProfileName) == 0 )
    {
        OSL_VERIFY(osl_getProfileName(NULL, NULL, &FileName));
    }
    else
*/
    {
        rtl_uString_assign(&FileName, strProfileName);
    }

    osl_getSystemPathFromFileURL(FileName, &FileName);

#ifdef DEBUG_OSL_PROFILE
    Flags=osl_Profile_FLUSHWRITE;

    // OSL_TRACE("opening '%s'\n",FileName);
    if ( Flags == osl_Profile_DEFAULT )
    {
        OSL_TRACE("with osl_Profile_DEFAULT \n");
    }
    if ( Flags & osl_Profile_SYSTEM )
    {
        OSL_TRACE("with osl_Profile_SYSTEM \n");
    }
    if ( Flags & osl_Profile_READLOCK )
    {
        OSL_TRACE("with osl_Profile_READLOCK \n");
    }
    if ( Flags & osl_Profile_WRITELOCK )
    {
        OSL_TRACE("with osl_Profile_WRITELOCK \n");
    }
/*      if ( Flags & osl_Profile_READWRITE ) */
/*      { */
/*          OSL_TRACE("with osl_Profile_READWRITE \n"); */
/*      } */
    if ( Flags & osl_Profile_FLUSHWRITE )
    {
        OSL_TRACE("with osl_Profile_FLUSHWRITE \n");
    }
#endif

    if ((! (Flags & osl_Profile_SYSTEM)) &&
        ((pFile = openFileImpl(FileName, (Flags & osl_Profile_WRITELOCK) ? sal_True : sal_False)) == NULL))
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_openProfile [not opened]\n");
#endif
        if( FileName)
            rtl_uString_release( FileName);

        return (NULL);
    }

    pProfile = (osl_TProfileImpl*)calloc(1, sizeof(osl_TProfileImpl));

    pProfile->m_Flags = Flags & FLG_USER;
    osl_getSystemPathFromFileURL(strProfileName, &pProfile->m_strFileName);
//  rtl_uString_assign(&pProfile->m_strFileName, strProfileName);

    if (Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK))
        pProfile->m_pFile = pFile;

    pProfile->m_Stamp = getFileStamp(pFile);

    loadProfile(pFile, pProfile);

    if (pProfile->m_pFile == NULL)
        closeFileImpl(pFile);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_openProfile [ok]\n");
#endif
    if( FileName)
        rtl_uString_release( FileName);

    return pProfile;
}

sal_Bool SAL_CALL osl_closeProfile(oslProfile Profile)
{
    osl_TProfileImpl* pProfile = (osl_TProfileImpl*)Profile;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_closeProfile\n");
#endif

    if ( pProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_closeProfile [profile==0]\n");
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
/*                  if (pProfile->m_pFile == NULL) */
/*                      pProfile->m_pFile = openFileImpl(pProfile->m_Filename, sal_True); */

                storeProfile(pProfile->m_pFile, pProfile, sal_False);
            }
        }
        else
        {
            pProfile = acquireProfile(Profile,sal_False);
        }

        if ( pProfile == 0 )
        {
#ifdef TRACE_OSL_PROFILE
            OSL_TRACE("Out osl_closeProfile [pProfile==0]\n");
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
            /*osl_TProfileSection* pSections=pProfile->m_Sections;*/
            for ( index = 0 ; index < pProfile->m_NoSections ; ++index )
            {
                if ( pProfile->m_Sections[index].m_Entries != 0 )
                {
                    free(pProfile->m_Sections[index].m_Entries);
                }
            }
            free(pProfile->m_Sections);
        }

    }
    free(pProfile);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_closeProfile [ok]\n");
#endif
    return (sal_True);
}

sal_Bool SAL_CALL osl_flushProfile(oslProfile Profile)
{
    osl_TProfileImpl* pProfile = (osl_TProfileImpl*) Profile;
    osl_TFile* pFile;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_flushProfile()\n");
#endif

    if ( pProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_flushProfile() [pProfile == 0]\n");
#endif
        return sal_False;
    }

    pFile = pProfile->m_pFile;
    if ( !( pFile != 0 && pFile->m_Handle >= 0 ) )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_flushProfile() [invalid file]\n");
#endif
        return sal_False;
    }

    if ( pProfile->m_Flags & FLG_MODIFIED )
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("swapping to storeprofile\n");
#endif
        bRet = storeProfile(pFile,pProfile,sal_False);
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_flushProfile() [ok]\n");
#endif
    return bRet;
}

sal_Bool SAL_CALL osl_readProfileString(oslProfile Profile,
                                        const sal_Char* pszSection, const sal_Char* pszEntry,
                                        sal_Char* pszString, sal_uInt32 MaxLen,
                                        const sal_Char* pszDefault)
{
    sal_uInt32    NoEntry;
    const sal_Char* pStr = 0;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = 0;


#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_readProfileString\n");
#endif

    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_readProfileString [pProfile==0]\n");
#endif


        return (sal_False);
    }


    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
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
        PrfQueryProfileString(pProfile->m_hIni, (PCSZ)pszSection,
                              (PCSZ)pszEntry, (PCSZ)pszDefault,
                              pszString, MaxLen);

    releaseProfile(pProfile);

    if ( pStr == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_readProfileString [pStr==0]\n");
#endif


        return (sal_False);
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_readProfileString [ok]\n");
#endif

    return (sal_True);
}


sal_Bool SAL_CALL osl_readProfileBool(oslProfile Profile,
                            const sal_Char* pszSection, const sal_Char* pszEntry,
                            sal_Bool Default)
{
    sal_Char Line[32];

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_readProfileBool\n");
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
    OSL_TRACE("Out osl_readProfileBool [ok]\n");
#endif

    return (Default);
}


sal_uInt32 SAL_CALL osl_readProfileIdent(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Default)
{
    sal_uInt32    i;
    sal_Char        Line[256];

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_readProfileIdent\n");
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
    OSL_TRACE("Out osl_readProfileIdent [ok]\n");
#endif
    return (Default);
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
    OSL_TRACE("In  osl_writeProfileString\n");
#endif

    pProfile = acquireProfile(Profile, sal_True);

    if (pProfile == NULL)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_writeProfileString [pProfile==0]\n");
#endif
        return (sal_False);
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
                OSL_TRACE("Out osl_writeProfileString [not added]\n");
#endif
                return (sal_False);
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
                OSL_TRACE("Out osl_writeProfileString [not inserted]\n");
#endif
                return (sal_False);
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
        PrfWriteProfileString(pProfile->m_hIni, (PCSZ)pszSection,
                              (PCSZ)pszEntry, (PCSZ)pszString);

    bRet = releaseProfile(pProfile);
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_writeProfileString [ok]\n");
#endif
    return bRet;
}


sal_Bool SAL_CALL osl_writeProfileBool(oslProfile Profile,
                             const sal_Char* pszSection, const sal_Char* pszEntry,
                             sal_Bool Value)
{
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_writeProfileBool\n");
#endif

    if (Value)
        bRet=osl_writeProfileString(Profile, pszSection, pszEntry, STR_INI_BOOLONE);
    else
        bRet=osl_writeProfileString(Profile, pszSection, pszEntry, STR_INI_BOOLZERO);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_writeProfileBool [ok]\n");
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
    OSL_TRACE("In  osl_writeProfileIdent\n");
#endif

    for (n = 0; Strings[n] != NULL; n++);

    if ((i = Value - FirstId) >= n)
        bRet=sal_False;
    else
        bRet=osl_writeProfileString(Profile, pszSection, pszEntry, Strings[i]);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_writeProfileIdent\n");
#endif
    return bRet;
}


sal_Bool SAL_CALL osl_removeProfileEntry(oslProfile Profile,
                               const sal_Char *pszSection, const sal_Char *pszEntry)
{
    sal_uInt32    NoEntry;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = 0;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_removeProfileEntry\n");
#endif

    pProfile = acquireProfile(Profile, sal_True);

    if (pProfile == NULL)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_removeProfileEntry [pProfile==0]\n");
#endif


        return (sal_False);
    }


    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
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
        PrfWriteProfileString(pProfile->m_hIni, (PCSZ)pszSection, (PCSZ)pszEntry, NULL);

    bRet = releaseProfile(pProfile);
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_removeProfileEntry [ok]\n");
#endif
    return bRet;
}


sal_uInt32 SAL_CALL osl_getProfileSectionEntries(oslProfile Profile, const sal_Char *pszSection,
                                      sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_uInt32    i, n = 0;
    sal_uInt32    NoEntry;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = 0;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_getProfileSectionEntries\n");
#endif

    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_getProfileSectionEntries [pProfile=0]\n");
#endif


        return (0);
    }


    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
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
        n = PrfQueryProfileString(pProfile->m_hIni, (PCSZ)pszSection, NULL, NULL,
                                    pszBuffer, MaxLen );

    releaseProfile(pProfile);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_getProfileSectionEntries [ok]\n");
#endif

    return (n);
}

sal_uInt32 SAL_CALL osl_getProfileSections(oslProfile Profile, sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_uInt32    i, n = 0;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
        return (0);

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if (MaxLen != 0)
        {
             for (i = 0; i < pProfile->m_NoSections; i++)
            {
                pSec = &pProfile->m_Sections[i];

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
        n = PrfQueryProfileString(pProfile->m_hIni, NULL, NULL, NULL,
                                    pszBuffer, MaxLen );

    releaseProfile(pProfile);

    return (n);
}

#if 0 // YD
sal_Bool SAL_CALL osl_getProfileName(rtl_uString* strPath, rtl_uString* strName, rtl_uString** strProfileName)
{
    sal_Bool bFailed;
    sal_Char File[_MAX_PATH];
    sal_Char Path[_MAX_PATH];
    sal_uInt32  nFileLen;
    sal_uInt32  nPathLen = 0;

    rtl_uString * strTmp = NULL;
    oslFileError nError;

    /* build file name */
    if (strName && strName->length)
    {
        if(strName->length >= _MAX_PATH)
            return sal_False;

        strcpy(File, (char*)strName->buffer);
        nFileLen = strName->length;

        if (rtl_ustr_indexOfChar( File, L'.' ) == -1)
        {
            if (nFileLen + strlen(STR_INI_EXTENSION) >= _MAX_PATH)
                return sal_False;

            /* add default extension */
            strcpy(File + nFileLen, STR_INI_EXTENSION);
            nFileLen += strlen(STR_INI_EXTENSION);
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

        if ((nFileLen = nLen - nOffset) >= _MAX_PATH)
            return sal_False;

        strncpy(File, pProgName + nOffset, nFileLen);

        if (nFileLen + strlen(STR_INI_EXTENSION) >= _MAX_PATH)
            return sal_False;

        /* add default extension */
        strcpy(File + nFileLen, STR_INI_EXTENSION);
        nFileLen += strlen(STR_INI_EXTENSION);

        rtl_uString_release( strProgName );
    }

    if (File[0] == 0)
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

            if (bFailed) return (sal_False);

            if (strHome->length >= _MAX_PATH)
                return sal_False;

            strcpy( Path, strHome->buffer);
            nPathLen = strHome->length;

            if (nLen > RTL_CONSTASCII_LENGTH(STR_INI_METAHOME))
            {
                pPath += RTL_CONSTASCII_LENGTH(STR_INI_METAHOME);
                nLen -= RTL_CONSTASCII_LENGTH(STR_INI_METAHOME);

                if (nLen + nPathLen >= _MAX_PATH)
                    return sal_False;

                strcpy(Path + nPathLen, pPath);
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

            if (bFailed) return (sal_False);

            if (strConfig->length >= _MAX_PATH)
                return sal_False;

            strcpy( Path, strConfig->buffer);
            nPathLen = strConfig->length;

            if (nLen > RTL_CONSTASCII_LENGTH(STR_INI_METACFG))
            {
                pPath += RTL_CONSTASCII_LENGTH(STR_INI_METACFG);
                nLen -= RTL_CONSTASCII_LENGTH(STR_INI_METACFG);

                if (nLen + nPathLen >= _MAX_PATH)
                    return sal_False;

                strcpy(Path + nPathLen, pPath);
                nPathLen += nLen;
            }

            rtl_uString_release(strConfig);
        }

        else if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METASYS), STR_INI_METASYS) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METASYS)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METASYS)] == '/')))
        {
            if (((nPathLen = GetWindowsDirectoryW(Path, _MAX_PATH)) == 0) || (nPathLen >= _MAX_PATH))
                return (sal_False);

            if (nLen > RTL_CONSTASCII_LENGTH(STR_INI_METASYS))
            {
                pPath += RTL_CONSTASCII_LENGTH(STR_INI_METASYS);
                nLen -= RTL_CONSTASCII_LENGTH(STR_INI_METASYS);

                if (nLen + nPathLen >= MAX_PATH)
                    return sal_False;

                strcpy(Path + nPathLen, pPath);
                nPathLen += nLen;
            }
        }

        else if ((rtl_ustr_ascii_compare_WithLength(pPath, RTL_CONSTASCII_LENGTH(STR_INI_METAINS), STR_INI_METAINS) == 0) &&
            ((nLen == RTL_CONSTASCII_LENGTH(STR_INI_METAINS)) || (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METAINS)] == '/') ||
                (pPath[RTL_CONSTASCII_LENGTH(STR_INI_METAINS)] == '"') ) )
        {
            if (! lookupProfile(pPath + RTL_CONSTASCII_LENGTH(STR_INI_METAINS), File, Path))
                return (sal_False);

            nPathLen = strlen(Path);
        }

        else if(nLen < MAX_PATH)
        {
            strcpy(Path, pPath);
            nPathLen = strlen(Path);
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

        if (bFailed) return (sal_False);
        if (strConfigDir->length >= MAX_PATH)
            return sal_False;

        strcpy(Path, strConfigDir->buffer);
        nPathLen = strConfigDir->length;
    }

    if (nPathLen && (Path[nPathLen - 1] != L'/') && (Path[nPathLen - 1] != L'\\'))
    {
        Path[nPathLen++] = L'\\';
        Path[nPathLen] = 0;
    }

    if (nPathLen + nFileLen >= MAX_PATH)
        return sal_False;

    /* append file name */
    strcpy(Path + nPathLen, File);
    nPathLen += nFileLen;

    /* copy filename */
    rtl_uString_newFromStr_WithLength(&strTmp, Path, nPathLen);
    nError = osl_getFileURLFromSystemPath(strTmp, strProfileName);
    rtl_uString_release(strTmp);

    return nError == osl_File_E_None;
}
#endif // 0 // YD


/*****************************************************************************/
/* Static Module Functions */
/*****************************************************************************/

static osl_TStamp getFileStamp(osl_TFile* pFile)
{
    osl_TStamp  FileTime;
    FILESTATUS3 FileStatus;
    sal_uInt32  Bytes;

    Bytes = sizeof( FILESTATUS3 );
    if ( (!pFile->m_Handle) ||
        DosQueryFileInfo(pFile->m_Handle, FIL_STANDARD, &FileStatus, Bytes))
        memset(&FileTime, 0, sizeof(FileTime));
    else
    {
        FileTime.m_Date = FileStatus.fdateLastWrite;
        FileTime.m_Time = FileStatus.ftimeLastWrite;
    }

    return (FileTime);
}

static sal_Bool lockFile(const osl_TFile* pFile, osl_TLockMode eMode)
{
    sal_uInt32  status = 1;
    FILELOCK    Lock;

    if (!pFile->m_Handle)
        return (sal_False);

    Lock.lOffset = 0;
    Lock.lRange  = 0xFFFFFFFF;

    switch (eMode)
    {
        case un_lock:
            status = DosSetFileLocks(pFile->m_Handle, &Lock, NULL, 1000, 0);
            break;

        case read_lock:
            status = DosSetFileLocks(pFile->m_Handle, NULL, &Lock, 1000, 1);
            break;

        case write_lock:
            status = DosSetFileLocks(pFile->m_Handle, NULL, &Lock, 1000, 0);
            break;
    }

    return (status == 0);
}

//static osl_TFile* openFile(rtl_uString* pszFilename, sal_Bool bWriteable)
static osl_TFile* openFileImpl(rtl_uString *ustrFileName, oslProfileOption ProfileFlags )
{
    sal_uInt32  action;
    APIRET      rc;
    osl_TFile*  pFile = (osl_TFile*)calloc(1, sizeof(osl_TFile));

        ULONG attributes;
        ULONG flags;
        ULONG mode;
    sal_Bool bWriteable = sal_False;
    rtl_String* strFileName=0;
    sal_Char* pszFileName=0;

    /* check parameters */
    OSL_ASSERT( ustrFileName );

    rtl_uString2String( &strFileName,
                            rtl_uString_getStr(ustrFileName),
                            rtl_uString_getLength(ustrFileName),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );
    pszFileName = rtl_string_getStr(strFileName);

/*    if ( ProfileFlags & ( osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE | osl_Profile_READWRITE ) )*/
    if ( ProfileFlags & ( osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE ) )
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("setting bWriteable to TRUE\n");
#endif
        bWriteable=sal_True;
    }

        if (bWriteable)
        {
            flags = FILE_NORMAL | FILE_ARCHIVED;
            attributes = OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
            mode = OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE;
        }
        else
        {
            flags = FILE_NORMAL;
            attributes = OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
               mode = OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY;
        }

        if (rc = DosOpen((PCSZ)pszFileName, &pFile->m_Handle, &action, 0, flags, attributes, mode, NULL))
        {
            if (rc == ERROR_TOO_MANY_OPEN_FILES)
            {
                LONG fhToAdd = 10;
                ULONG fhOld = 0;
                rc = DosSetRelMaxFH(&fhToAdd, &fhOld);
                rc = DosOpen((PCSZ)pszFileName, &pFile->m_Handle, &action, 0, flags, attributes, mode, NULL);
            }
        }

        if ( (rc != NO_ERROR) && bWriteable)
        {
            free(pFile);
            rtl_string_release(strFileName);
              return (NULL);
        }

    rtl_string_release(strFileName);

    pFile->m_pWriteBuf=0;
    pFile->m_nWriteBufFree=0;
    pFile->m_nWriteBufLen=0;

    if ( ProfileFlags & (osl_Profile_WRITELOCK | osl_Profile_READLOCK ) )
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("locking '%s' file\n",pszFilename);
#endif

        lockFile(pFile, bWriteable ? write_lock : read_lock);
    }

    /* mfe: new WriteBuf obsolete */
/*  pFile->m_pWritePtr = pFile->m_Buf;*/
/*  pFile->m_pReadPtr  = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);*/

    return (pFile);
}

//static osl_TStamp closeFile(osl_TFile* pFile)
static osl_TStamp closeFileImpl(osl_TFile* pFile)
{
    osl_TStamp stamp = {0, 0};

    if ( pFile == 0 )
    {
        return stamp;
    }

    if (pFile->m_Handle)
    {
        /* mfe: new WriteBuf obsolete */
        /* we just closing the file here, DO NOT write, it has to be handled in higher levels */
        //if (pFile->m_pWritePtr > pFile->m_WriteBuf)
        //{
        //  sal_uInt32 Bytes;

        //  DosWrite(pFile->m_Handle, pFile->m_WriteBuf,
        //           pFile->m_pWritePtr - pFile->m_WriteBuf,
        //           &Bytes);
        //}

        stamp = getFileStamp(pFile);

        lockFile(pFile, un_lock);

        DosClose(pFile->m_Handle);
    }

    if ( pFile->m_pWriteBuf != 0 )
    {
        free(pFile->m_pWriteBuf);
    }

    free(pFile);

    return(stamp);
}

static sal_Bool rewindFile(osl_TFile* pFile, sal_Bool bTruncate)
{
    if (pFile->m_Handle)
    {
        sal_uInt32 Position;

        /* mfe: new WriteBuf obsolete */
        /* we just closing the file here, DO NOT write, it has to be handled in higher levels */
        /* if (pFile->m_pWritePtr > pFile->m_WriteBuf)
        {
            sal_uInt32 Bytes;

            DosWrite(pFile->m_Handle, pFile->m_WriteBuf,
                     pFile->m_pWritePtr - pFile->m_WriteBuf,
                     &Bytes);

            pFile->m_pWritePtr = pFile->m_WriteBuf;
        } */

        pFile->m_pReadPtr = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);

        DosSetFilePtr(pFile->m_Handle, 0, FILE_BEGIN, &Position);

        if (bTruncate)
            DosSetFileSize(pFile->m_Handle, 0);
    }

    return (sal_True);
}

static sal_Bool getLine(osl_TFile* pFile, const sal_Char *pszLine, int MaxLen)
{
    int   Free, Bytes;
    sal_Char* pChr;
    sal_Char* pLine = (sal_Char *)pszLine;
    sal_uInt32  Max;

    if (pFile->m_Handle == 0)
        return (sal_False);

    MaxLen -= 1;

    do
    {
        Bytes = sizeof(pFile->m_ReadBuf) - (pFile->m_pReadPtr - pFile->m_ReadBuf);

        if (Bytes <= 1)
        {
            /* refill buffer */
            memcpy(pFile->m_ReadBuf, pFile->m_pReadPtr, Bytes);
            pFile->m_pReadPtr = pFile->m_ReadBuf;

            Free = sizeof(pFile->m_ReadBuf) - Bytes;

            if (DosRead(pFile->m_Handle, &pFile->m_ReadBuf[Bytes], Free, &Max))
            {
                *pLine = '\0';
                return (sal_False);
            }

            if (Max < Free)
            {
                if ((Max == 0) && (pLine == pszLine))
                {
                    *pLine = '\0';
                    return (sal_False);
                }

                pFile->m_ReadBuf[Bytes + Max] = '\0';
            }
        }

        for (pChr = pFile->m_pReadPtr;
             (*pChr != '\n') && (*pChr != '\r') && (*pChr != '\0') &&
             (pChr < (pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf) - 1));
             pChr++);

        Max = SAL_MIN(pChr - pFile->m_pReadPtr, MaxLen);
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

    return (sal_True);
}

static sal_Bool putLine(osl_TFile* pFile, const sal_Char *pszLine)
{
    unsigned int Len = strlen(pszLine);

#ifdef DEBUG_OSL_PROFILE
    int strLen=0;
#endif

    if ( pFile == 0 || pFile->m_Handle < 0 )
    {
        return (sal_False);
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
#ifdef DEBUG_OSL_PROFILE
    strLen = strlen(pFile->m_pWriteBuf);
#endif
    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len]='\r';
    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len + 1]='\n';
    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len + 2]='\0';

    pFile->m_nWriteBufFree-=Len+2;

#ifdef DEBUG_OSL_PROFILE
/*    OSL_TRACE("File Buffer in _putLine '%s' '%i'(%i)\n",pFile->m_pWriteBuf,strlen(pFile->m_pWriteBuf),pFile->m_nWriteBufLen - pFile->m_nWriteBufFree);*/
#endif

    return (sal_True);
}

/* platform specific end */

static const sal_Char* stripBlanks(const sal_Char* String, sal_uInt32* pLen)
{
    if  ( (pLen != NULL) && ( *pLen != 0 ) )
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

    return (String);
}

static const sal_Char* addLine(osl_TProfileImpl* pProfile, const sal_Char* Line)
{
    if (pProfile->m_NoLines >= pProfile->m_MaxLines)
    {
        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_MaxLines = LINES_INI;
            pProfile->m_Lines = (sal_Char **)malloc(pProfile->m_MaxLines * sizeof(sal_Char *));
        }
        else
        {
            pProfile->m_MaxLines += LINES_ADD;
            pProfile->m_Lines = (sal_Char **)realloc(pProfile->m_Lines,
                                                 pProfile->m_MaxLines * sizeof(sal_Char *));
        }

        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_NoLines  = 0;
            pProfile->m_MaxLines = 0;
            return (NULL);
        }

    }

    pProfile->m_Lines[pProfile->m_NoLines++] = strdup(Line);

    return (pProfile->m_Lines[pProfile->m_NoLines - 1]);
}

static const sal_Char* insertLine(osl_TProfileImpl* pProfile, const sal_Char* Line, sal_uInt32 LineNo)
{
    if (pProfile->m_NoLines >= pProfile->m_MaxLines)
    {
        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_MaxLines = LINES_INI;
            pProfile->m_Lines = (sal_Char **)malloc(pProfile->m_MaxLines * sizeof(sal_Char *));
        }
        else
        {
            pProfile->m_MaxLines += LINES_ADD;
            pProfile->m_Lines = (sal_Char **)realloc(pProfile->m_Lines,
                                                 pProfile->m_MaxLines * sizeof(sal_Char *));
        }

        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_NoLines  = 0;
            pProfile->m_MaxLines = 0;
            return (NULL);
        }

    }

    LineNo = LineNo > pProfile->m_NoLines ? pProfile->m_NoLines : LineNo;

    if (LineNo < pProfile->m_NoLines)
    {
        sal_uInt32 i, n;
        osl_TProfileSection* pSec;

        memmove(&pProfile->m_Lines[LineNo + 1], &pProfile->m_Lines[LineNo],
                (pProfile->m_NoLines - LineNo) * sizeof(sal_Char *));

        /* adjust line references */
        for (i = 0; i < pProfile->m_NoSections; i++)
        {
            pSec = &pProfile->m_Sections[i];

            if (pSec->m_Line >= LineNo)
                pSec->m_Line++;

            for (n = 0; n < pSec->m_NoEntries; n++)
                if (pSec->m_Entries[n].m_Line >= LineNo)
                    pSec->m_Entries[n].m_Line++;
        }
    }

    pProfile->m_NoLines++;

    pProfile->m_Lines[LineNo] = strdup(Line);

    return (pProfile->m_Lines[LineNo]);
}

static void removeLine(osl_TProfileImpl* pProfile, sal_uInt32 LineNo)
{
    if (LineNo < pProfile->m_NoLines)
    {
        free(pProfile->m_Lines[LineNo]);
        if (pProfile->m_NoLines - LineNo > 1)
        {
            sal_uInt32 i, n;
            osl_TProfileSection* pSec;

            memmove(&pProfile->m_Lines[LineNo], &pProfile->m_Lines[LineNo + 1],
                    (pProfile->m_NoLines - LineNo - 1) * sizeof(sal_Char *));

            /* adjust line references */
            for (i = 0; i < pProfile->m_NoSections; i++)
            {
                pSec = &pProfile->m_Sections[i];

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
                return (sal_False);
            }
        }

        pSection->m_NoEntries++;

        Entry = stripBlanks(Entry, &Len);
        setEntry(pProfile, pSection, pSection->m_NoEntries - 1, Line,
                 Entry, Len);

        return (sal_True);
    }

    return (sal_False);
}

static void removeEntry(osl_TProfileSection *pSection, sal_uInt32 NoEntry)
{
    if (NoEntry < pSection->m_NoEntries)
    {
        if (pSection->m_NoEntries - NoEntry > 1)
            memmove(&pSection->m_Entries[NoEntry],
                    &pSection->m_Entries[NoEntry + 1],
                    (pSection->m_NoEntries - NoEntry - 1) * sizeof(osl_TProfileEntry));
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
            pProfile->m_Sections = (osl_TProfileSection *)malloc(pProfile->m_MaxSections * sizeof(osl_TProfileSection));
        }
        else
        {
            pProfile->m_MaxSections += SECTIONS_ADD;
            pProfile->m_Sections = (osl_TProfileSection *)realloc(pProfile->m_Sections,
                                          pProfile->m_MaxSections * sizeof(osl_TProfileSection));
        }

        if (pProfile->m_Sections == NULL)
        {
            pProfile->m_NoSections = 0;
            pProfile->m_MaxSections = 0;
            return (sal_False);
        }
    }

    pProfile->m_NoSections++;

    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Entries    = NULL;
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_NoEntries  = 0;
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_MaxEntries = 0;

    Section = (sal_Char *)stripBlanks(Section, &Len);
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Line = Line;
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Offset = Section - pProfile->m_Lines[Line];
    pProfile->m_Sections[pProfile->m_NoSections - 1].m_Len = Len;

    return (sal_True);
}

static void removeSection(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection)
{
    sal_uInt32 Section;

    if ((Section = pSection - pProfile->m_Sections) < pProfile->m_NoSections)
    {
        free (pSection->m_Entries);
        if (pProfile->m_NoSections - Section > 1)
        {
            memmove(&pProfile->m_Sections[Section], &pProfile->m_Sections[Section + 1],
                    (pProfile->m_NoSections - Section - 1) * sizeof(osl_TProfileSection));
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
        const sal_Char* pStr;
        osl_TProfileSection* pSec;

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
            pStr = &pProfile->m_Lines[pSec->m_Entries[i].m_Line]
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

    return (pSec);
}

static sal_Bool loadProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile)
{
    sal_uInt32    i;
    sal_Char*       pStr;
    sal_Char*       pChar;
    sal_Char        Line[1024];

    pProfile->m_NoLines    = 0;
    pProfile->m_NoSections = 0;

    OSL_VERIFY(rewindFile(pFile, sal_False));

    while (getLine(pFile, Line, sizeof(Line)))
    {
        if (! addLine(pProfile, Line))
            return (sal_False);
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
                return (sal_False);
        }
        else
        {
            /* new section */

            if (! addSection(pProfile, i, pStr + 1, pChar - pStr - 1))
                return (sal_False);
        }
    }

    return (sal_True);
}

static sal_Bool storeProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile, sal_Bool bCleanup)
{
    if (pProfile->m_Lines != NULL)
    {
        if (pProfile->m_Flags & FLG_MODIFIED)
        {
            sal_uInt32 i;

            OSL_VERIFY(rewindFile(pFile, sal_True));

            for (i = 0; i < pProfile->m_NoLines; i++)
                OSL_VERIFY(putLine(pFile, pProfile->m_Lines[i]));

            pProfile->m_Flags &= ~FLG_MODIFIED;
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

    return (sal_True);
}

static osl_TProfileImpl* acquireProfile(oslProfile Profile, sal_Bool bWriteable)
{
    osl_TProfileImpl* pProfile = (osl_TProfileImpl*)Profile;
    oslProfileOption PFlags=0;


    if ( bWriteable )
    {
/*          PFlags = osl_Profile_DEFAULT | osl_Profile_READWRITE; */
        PFlags = osl_Profile_DEFAULT | osl_Profile_WRITELOCK;
    }
    else
    {
        PFlags = osl_Profile_DEFAULT;
    }


    if (pProfile == NULL)
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("AUTOOPEN MODE\n");
#endif

        if ((pProfile = (osl_TProfileImpl*)osl_openProfile(NULL, PFlags)) != NULL )
        {
            pProfile->m_Flags |= FLG_AUTOOPEN;
        }
    }
    else
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("try to acquire\n");
#endif



        if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
        {
            if (! (pProfile->m_Flags & (osl_Profile_READLOCK |
                                        osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE)))
            {
                osl_TStamp Stamp;
#ifdef DEBUG_OSL_PROFILE
                OSL_TRACE("DEFAULT MODE\n");
#endif
                if (! (pProfile->m_pFile = openFileImpl(pProfile->m_strFileName, pProfile->m_Flags | PFlags)))
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
                OSL_TRACE("READ/WRITELOCK MODE\n");
#endif


                /* A readlock file could not be written */
                if ((pProfile->m_Flags & osl_Profile_READLOCK) && bWriteable)
                {
                    return (NULL);
                }
            }
        }
        else
        {
            sal_Bool bWriteable = sal_False;
            char pszFilename[PATH_MAX] = "";

            if ( pProfile->m_strFileName != 0  && pProfile->m_strFileName->buffer[0] != 0 )
                FileURLToPath( pszFilename, PATH_MAX, pProfile->m_strFileName );
            /* hack: usualy you have a specific HAB, but NULL works here... */
            pProfile->m_hIni = PrfOpenProfile(NULL, (PCSZ)pszFilename);
            if (! pProfile->m_hIni)
                return (NULL);
        }
    }

    return (pProfile);
}

static sal_Bool releaseProfile(osl_TProfileImpl* pProfile)
{
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  releaseProfile\n");
#endif

    if ( pProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out releaseProfile [profile==0]\n");
#endif
        return sal_False;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if (pProfile->m_Flags & FLG_AUTOOPEN)
        {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out releaseProfile [AUTOOPEN]\n");
#endif
            return (osl_closeProfile((oslProfile)pProfile));
        }
        else
        {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("DEFAULT MODE\n");
#endif
        if (! (pProfile->m_Flags & (osl_Profile_READLOCK |
                                    osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE)))
            {
                if (pProfile->m_Flags & FLG_MODIFIED)
                    storeProfile(pProfile->m_pFile, pProfile, sal_False);

                closeFileImpl(pProfile->m_pFile);
                pProfile->m_pFile = NULL;
            }
        }
    }
    else
        PrfCloseProfile(pProfile->m_hIni);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out releaseProfile [ok]\n");
#endif
    return (sal_True);
}

#if 0 // YD
static sal_Bool lookupProfile(const sal_Char *pszPath, const sal_Char *pszFile, sal_Char *pPath)
{
    sal_Char *pChr, *pStr;
    sal_Char Path[_MAX_PATH] = "";
    sal_Char Product[132] = "";
    sal_Char Buffer[1024];

    if (*pszPath == '"')
    {
        int i = 0;

        pszPath++;

        while ((*pszPath != '"') && (*pszPath != '\0'))
            Product[i++] = *pszPath++;

        Product[i] = '\0';

        if (*pszPath == '"')
            pszPath++;

        if ( (*pszPath == '/') || (*pszPath == '\\') )
        {
            pszPath++;
        }
    }
    else
    {
        /* if we have not product identfication, do a special handling for soffice.ini */
        if (stricmp(SVERSION_PROFILE, pszFile) == 0)
        {
            sal_Char   Profile[_MAX_PATH];
            sal_Char   Dir[_MAX_PATH];
            oslProfile hProfile;

            /* open sversion.ini in the system directory, and try to locate the entry
               with the highest version for StarOffice */
            if ((osl_getProfileName(SVERSION_FALLBACK, SVERSION_NAME, Profile, sizeof(Profile))) &&
                (hProfile = osl_openProfile(Profile, osl_Profile_READLOCK)))
            {
                  osl_getProfileSectionEntries(hProfile, SVERSION_SECTION,
                                                Buffer, sizeof(Buffer));

                for (pChr = Buffer; *pChr != '\0'; pChr += strlen(pChr) + 1)
                {
                    if ((strnicmp(pChr, SVERSION_SOFFICE, sizeof(SVERSION_SOFFICE) - 1) == 0) &&
                        (stricmp(Product, pChr) < 0))
                    {
                        osl_readProfileString(hProfile, SVERSION_SECTION, pChr,
                                              Dir, sizeof(Dir), "");

                        /* check for existence of path */
                        if (access(Dir, 0) >= 0)
                            strcpy(Product, pChr);
                    }
                }

                osl_closeProfile(hProfile);
            }

            /* open sversion.ini in the users directory, and try to locate the entry
               with the highest version for StarOffice */
            if ((strcmp(SVERSION_LOCATION, SVERSION_FALLBACK) != 0) &&
                (osl_getProfileName(SVERSION_LOCATION, SVERSION_NAME, Profile, sizeof(Profile))) &&
                (hProfile = osl_openProfile(Profile, osl_Profile_READLOCK)))
            {
                  osl_getProfileSectionEntries(hProfile, SVERSION_SECTION,
                                                Buffer, sizeof(Buffer));

                for (pChr = Buffer; *pChr != '\0'; pChr += strlen(pChr) + 1)
                {
                    if ((strnicmp(pChr, SVERSION_SOFFICE, sizeof(SVERSION_SOFFICE) - 1) == 0) &&
                        (stricmp(Product, pChr) < 0))
                    {
                        osl_readProfileString(hProfile, SVERSION_SECTION, pChr,
                                              Dir, sizeof(Dir), "");

                        /* check for existence of path */
                        if (access(Dir, 0) >= 0)
                            strcpy(Product, pChr);
                    }
                }

                osl_closeProfile(hProfile);
            }

            /* remove any trailing build number */
            if ((pChr = strrchr(Product, '/')) != NULL)
                *pChr = '\0';
        }
    }


    /* if we have an userid option eg. "-userid:rh[/usr/home/rh/staroffice]",
       this will supercede all other locations */
    if (osl_getCommandArgs(Buffer, sizeof(Buffer)) == osl_Process_E_None)
    {
        sal_Char *pStart, *pEnd;

        for (pChr = Buffer; *pChr != '\0'; pChr += strlen(pChr) + 1)
            if (((*pChr == '-') || (*pChr == '+')) &&
                (strnicmp(pChr + 1, SVERSION_OPTION, sizeof(SVERSION_OPTION) - 1) == 0))
            {
                if (((pStart = strchr(pChr + sizeof(SVERSION_OPTION), '[')) != NULL) &&
                    ((pEnd = strchr(pStart + 1, ']')) != NULL))
                {
                    strncpy(Path, pStart + 1, pEnd - (pStart + 1));
                    Path[pEnd - (pStart + 1)] = '\0';

                    /* build full path */
                    if ((Path[strlen(Path) - 1] != '/') && (Path[strlen(Path) - 1] != '\\'))
                    {
                        strcat(Path, "\\");
                    }

                    pChr =&Path[strlen(Path)];
                    if ( strlen(pszPath) <= 0 )
                    {
                        strcat(Path,SVERSION_USER);

                        if ( access(Path, 0) < 0 )
                        {
                            *pChr='\0';
                        }
                    }
                    else
                    {
                        strcat(Path, pszPath);
                    }

                    break;
                }
            }
    }

    if (strlen(Path) <= 0)
    {
        /* try to find the file in the directory of the executbale */
        if (osl_getExecutableFile(Path, sizeof(Path)) != osl_Process_E_None)
            return (sal_False);

        /* seperate path from filename */
        if ((pChr = strrchr(Path, '\\')) == NULL)
            if ((pChr = strrchr(Path, ':')) == NULL)
                return (sal_False);
            else
                *pChr = '\0';
        else
            *pChr = '\0';

        /* if we have no product identification use the executable file name */
        if (strlen(Product) <= 0)
        {
            strcpy(Product, pChr + 1);

            /* remove extension */
            if ((pChr = strrchr(Product, '.')) != NULL)
                *pChr = '\0';
        }

        /* remember last subdir */
        pStr = strrchr(Path, '\\');

        strcat(Path, "\\");

        if ( strlen(pszPath) <= 0 )
        {
            strcat(Path, pszPath);
        }
        else
        {
            strcat(Path,pszPath);
        }

        /* if file not exists, remove any specified subdirectories
           like "bin" or "program" */
        if (((access(Path, 0) < 0) && (pStr != NULL)) || (strlen(pszPath) <= 0))
        {
            static sal_Char *SubDirs[] = SVERSION_DIRS;

            int i = 0;

            for (i = 0; i < SAL_N_ELEMENTS(SubDirs); i++)
                if (strnicmp(pStr + 1, SubDirs[i], strlen(SubDirs[i])) == 0)
                {
                    if ( strlen(pszPath) <= 0)
                    {
                        strcpy(pStr + 1,SVERSION_USER);
                        if ( access(Path, 0) < 0 )
                        {
                            *(pStr+1)='\0';
                        }
                    }
                    else
                    {
                        strcpy(pStr + 1, pszPath);
                    }

                    break;
                }
        }

        pChr = &Path[strlen(Path)];
        if ((Path[strlen(Path) - 1] != '/') && (Path[strlen(Path) - 1] != '\\'))
            strcat(Path, "\\");
        strcat(Path, pszFile);

        if ((access(Path, 0) < 0) && (strlen(Product) > 0))
        {
            sal_Char   Profile[_MAX_PATH];
            oslProfile hProfile;

            /* remove appended filename */
            *pChr = '\0';

            /* open sversion.ini in the system directory, and try to locate the entry
               with the highest version for StarOffice */
            if ((osl_getProfileName(SVERSION_LOCATION, SVERSION_NAME, Profile, sizeof(Profile))) &&
                (hProfile = osl_openProfile(Profile, osl_Profile_READLOCK)))
            {
                pChr = &Product[strlen(Product)];

                /* append build number */
                strcat(Product, "/");
                strcat(Product, BUILD_STR(SUPD));

                osl_readProfileString(hProfile, SVERSION_SECTION, Product,
                                      Buffer, sizeof(Buffer), "");

                /* if not found, try it without build number */
                if (strlen(Buffer) <= 0)
                {
                    *pChr = '\0';

                    osl_readProfileString(hProfile, SVERSION_SECTION, Product,
                                          Buffer, sizeof(Buffer), "");

                    osl_closeProfile(hProfile);

                    /* if not found, try the fallback */
                    if ((strlen(Buffer) <= 0) && (strcmp(SVERSION_LOCATION, SVERSION_FALLBACK) != 0))
                    {
                        if ((osl_getProfileName(SVERSION_FALLBACK, SVERSION_NAME, Profile, sizeof(Profile))) &&
                            (hProfile = osl_openProfile(Profile, osl_Profile_READLOCK)))
                        {
                            /* prepare build number */
                            *pChr = '/';

                            osl_readProfileString(hProfile, SVERSION_SECTION, Product,
                                                  Buffer, sizeof(Buffer), "");

                            /* if not found, try it without build number */
                            if (strlen(Buffer) <= 0)
                            {
                                *pChr = '\0';

                                osl_readProfileString(hProfile, SVERSION_SECTION, Product,
                                                      Buffer, sizeof(Buffer), "");
                            }

                            osl_closeProfile(hProfile);
                        }
                    }
                }
                else
                    osl_closeProfile(hProfile);

                if (strlen(Buffer) > 0)
                {
                    strcpy(Path, Buffer);

                    /* build full path */
                    if ((Path[strlen(Path) - 1] != '/') && (Path[strlen(Path) - 1] != '\\'))
                    {
                        if ((*pszPath != '/') && (*pszPath != '\\'))
                            strcat(Path, "\\");
                    }

                    pChr=&Path[strlen(pszPath)];
                    if ( strlen(pszPath) > 0 )
                    {
                        strcat(Path, pszPath);
                    }
                    else
                    {
                        strcat(Path,SVERSION_USER);
                        if ( access(Path, 0) < 0 )
                        {
                            *pChr='\0';
                        }
                    }
                }
            }
        }
        else
            /* remove appended filename */
            *pChr = '\0';
    }

    strcpy(pPath, Path);

    return (sal_True);
}

#endif // 0 // YD

