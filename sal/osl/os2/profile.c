/*************************************************************************
 *
 *  $RCSfile: profile.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:20 $
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

#include "system.h"

#define min(a,b) (((a) < (b)) ? (a) : (b))

#include <osl/security.h>
#include <osl/diagnose.h>
#include <osl/profile.h>
#include <osl/process.h>
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
    sal_Char*   m_pReadPtr;
    sal_Char    m_ReadBuf[512];
    sal_Char*   m_pWritePtr;
    sal_Char    m_WriteBuf[512];
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
    sal_Char    m_Filename[_MAX_PATH + 1];
    sal_uInt32  m_NoLines;
    sal_uInt32  m_MaxLines;
    sal_uInt32  m_NoSections;
    sal_uInt32  m_MaxSections;
    sal_Char**  m_Lines;
    osl_TProfileSection*    m_Sections;
    HINI                    m_hIni;
} osl_TProfileImpl;


/*****************************************************************************/
/* Static Module Function Declarations */
/*****************************************************************************/

static osl_TFile* openFile(const sal_Char* pszFilename, sal_Bool bWriteable);
static osl_TStamp closeFile(osl_TFile* pFile);
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




/*****************************************************************************/
/* Exported Module Functions */
/*****************************************************************************/

oslProfile SAL_CALL osl_openProfile(const sal_Char *pszProfileName, sal_uInt32 Flags)
{
    osl_TFile*        pFile;
    osl_TProfileImpl* pProfile;
    sal_Char              Filename[_MAX_PATH];

    if (pszProfileName == NULL)
    {
        OSL_VERIFY(osl_getProfileName(NULL, NULL, Filename, sizeof(Filename)));
        pszProfileName = Filename;
    }

    if ((! (Flags & osl_Profile_SYSTEM)) &&
        ((pFile = openFile(pszProfileName, (Flags & osl_Profile_WRITELOCK) ? sal_True : sal_False)) == NULL))
            return NULL;

    pProfile = (osl_TProfileImpl*)calloc(1, sizeof(osl_TProfileImpl));

    pProfile->m_Flags = Flags & FLG_USER;
    osl_getFullPath(pszProfileName, pProfile->m_Filename, sizeof(pProfile->m_Filename));

    if (Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK))
        pProfile->m_pFile = pFile;

    pProfile->m_Stamp = getFileStamp(pFile);

    loadProfile(pFile, pProfile);

    if (pProfile->m_pFile == NULL)
        closeFile(pFile);

    return pProfile;
}

sal_Bool SAL_CALL osl_closeProfile(oslProfile Profile)
{
    osl_TProfileImpl* pProfile = (osl_TProfileImpl*)Profile;

    if ( pProfile == 0 )
    {
        return sal_False;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if (pProfile->m_Flags & FLG_MODIFIED)
        {
            if (pProfile->m_pFile == NULL)
                pProfile->m_pFile = openFile(pProfile->m_Filename, sal_True);

            storeProfile(pProfile->m_pFile, pProfile, sal_True);
        }

        if (pProfile->m_pFile != NULL)
            closeFile(pProfile->m_pFile);
    }

    pProfile->m_pFile = NULL;
    pProfile->m_Filename[0] = '\0';

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

    return sal_True;
}

sal_Bool SAL_CALL osl_readProfileString(oslProfile Profile,
                                        const sal_Char* pszSection, const sal_Char* pszEntry,
                                        sal_Char* pszString, sal_uInt32 MaxLen,
                                        const sal_Char* pszDefault)
{
    sal_uInt32    NoEntry;
    sal_Char* pStr=0;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
        return sal_False;

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
        PrfQueryProfileString(pProfile->m_hIni, (sal_Char*)pszSection,
                              (sal_Char*)pszEntry, (sal_Char*)pszDefault,
                              pszString, MaxLen);

    releaseProfile(pProfile);

    if ( pStr == 0 )
        return sal_False ;

    return sal_True;
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
            Default = sal_True;
        else
            if ((stricmp(Line, STR_INI_BOOLNO)   == 0) ||
                (stricmp(Line, STR_INI_BOOLOFF)  == 0) ||
                (stricmp(Line, STR_INI_BOOLZERO) == 0))
                Default = sal_False;
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

    return Default;
}


sal_Bool SAL_CALL osl_writeProfileString(oslProfile Profile,
                               const sal_Char* pszSection, const sal_Char* pszEntry,
                               const sal_Char* pszString)
{
    sal_uInt32    i;
    sal_uInt32    NoEntry;
    const sal_Char* pStr;
    sal_Char        Line[1024];
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = acquireProfile(Profile, sal_True);

    if (pProfile == NULL)
        return sal_False;

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if ((pSec = findEntry(pProfile, pszSection, pszEntry, &NoEntry)) == NULL)
        {
            Line[0] = '\0';
            addLine(pProfile, Line);

            Line[0] = '[';
            strcpy(&Line[0], pszSection);
            Line[0 + strlen(pszSection)] = ']';
            Line[1 + strlen(pszSection)] = '\0';

            if (((pStr = addLine(pProfile, Line)) == NULL) ||
                (! addSection(pProfile, pProfile->m_NoLines - 1, &pStr[1], strlen(pszSection))))
            {
                releaseProfile(pProfile);
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

            if ( ( (pStr = insertLine(pProfile, Line, i) ) == NULL ) ||
                 ( ! addEntry(pProfile, pSec, i, pStr, strlen(pszEntry)) ) )
            {
                releaseProfile(pProfile);
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
        PrfWriteProfileString(pProfile->m_hIni, (sal_Char*)pszSection,
                              (sal_Char*)pszEntry, (sal_Char*)pszString);

    return (releaseProfile(pProfile));
}


sal_Bool SAL_CALL osl_writeProfileBool(oslProfile Profile,
                             const sal_Char* pszSection, const sal_Char* pszEntry,
                             sal_Bool Value)
{
    if (Value)
        return (osl_writeProfileString(Profile, pszSection, pszEntry, STR_INI_BOOLONE));
    else
        return (osl_writeProfileString(Profile, pszSection, pszEntry, STR_INI_BOOLZERO));
}


sal_Bool SAL_CALL osl_writeProfileIdent(oslProfile Profile,
                              const sal_Char* pszSection, const sal_Char* pszEntry,
                              sal_uInt32 FirstId, const sal_Char* Strings[],
                              sal_uInt32 Value)
{
    int i, n;

    for (n = 0; Strings[n] != NULL; n++);

    if ((i = Value - FirstId) >= n)
        return (sal_False);
    else
        return (osl_writeProfileString(Profile, pszSection, pszEntry, Strings[i]));
}


sal_Bool SAL_CALL osl_removeProfileEntry(oslProfile Profile,
                               const sal_Char *pszSection, const sal_Char *pszEntry)
{
    sal_uInt32    NoEntry;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = acquireProfile(Profile, sal_True);

    if (pProfile == NULL)
        return (sal_False);

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
        PrfWriteProfileString(pProfile->m_hIni, (sal_Char*)pszSection, (sal_Char*)pszEntry, NULL);

    return (releaseProfile(pProfile));
}


sal_uInt32 SAL_CALL osl_getProfileSectionEntries(oslProfile Profile, const sal_Char *pszSection,
                                      sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_uInt32    i, n = 0;
    sal_uInt32    NoEntry;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
        return (0);

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
        n = PrfQueryProfileString(pProfile->m_hIni, (sal_Char*)pszSection, NULL, NULL,
                                    pszBuffer, MaxLen );

    releaseProfile(pProfile);

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

sal_Bool SAL_CALL osl_getProfileName(const sal_Char* pszPath, const sal_Char* pszName,
                                     sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_Bool    bFailed;
    sal_Char*   pStr;
    sal_Char    File[_MAX_PATH];
    sal_Char    Path[_MAX_PATH];

    Path[0]='\0';

    /* build file name */
    if (pszName)
    {
        strcpy(File, pszName);

        /* add default extensiom */
        if (strchr(pszName, '.') == NULL)
            strcat(File, STR_INI_EXTENSION);
    }
    else
    {
        sal_Char *pChr;
        sal_Char ProgName[_MAX_PATH];

        if (osl_getExecutableFile(ProgName, sizeof(ProgName)) != osl_Process_E_None)
            return (sal_False);

        /* remove path from filename */
        if ((pStr = strrchr(ProgName, '\\')) == NULL)
            if ((pStr = strrchr(ProgName, ':')) == NULL)
                pStr = ProgName;
            else
                pStr++;
        else
            pStr++;

        if ((pChr = strrchr(pStr, '.')) != NULL)
            *pChr = '\0';

        strcpy(File, pStr);
        strcat(File, STR_INI_EXTENSION);
    }

    /* build directory path */
    if (pszPath)
    {
        if ((strncmp(pszPath, STR_INI_METAHOME, sizeof(STR_INI_METAHOME) - 1) == 0) &&
            ((pszPath[sizeof(STR_INI_METAHOME) - 1] == '\0') ||
             (pszPath[sizeof(STR_INI_METAHOME) - 1] == '\\') ||
             (pszPath[sizeof(STR_INI_METAHOME) - 1] == '/')))
        {
            oslSecurity security = osl_getCurrentSecurity();
            bFailed = ! osl_getHomeDir(security, Path, sizeof(Path));
            osl_freeSecurityHandle(security);

            if (bFailed) return (sal_False);

            if ((pszPath[sizeof(STR_INI_METAHOME) - 1] == '/') ||
                (pszPath[sizeof(STR_INI_METAHOME) - 1] == '\\'))
            {
                if ((Path[strlen(Path) - 1] != '/') && (Path[strlen(Path) - 1] != '\\'))
                    strcat(Path, "\\");
                strcat(Path, pszPath + sizeof(STR_INI_METAHOME));
            }
        }
        else if ((strncmp(pszPath, STR_INI_METACFG, sizeof(STR_INI_METACFG) - 1) == 0) &&
                 ((pszPath[sizeof(STR_INI_METACFG) - 1] == '\0') ||
                   (pszPath[sizeof(STR_INI_METACFG) - 1] == '\\') ||
                  (pszPath[sizeof(STR_INI_METACFG) - 1] == '/')))
        {
            oslSecurity security = osl_getCurrentSecurity();
            bFailed = ! osl_getConfigDir(security, Path, sizeof(Path));
            osl_freeSecurityHandle(security);

            if (bFailed) return (sal_False);

            if ((pszPath[sizeof(STR_INI_METACFG) - 1] == '/') ||
                (pszPath[sizeof(STR_INI_METACFG) - 1] == '\\'))
            {
                if ((Path[strlen(Path) - 1] != '/') && (Path[strlen(Path) - 1] != '\\'))
                    strcat(Path, "\\");
                strcat(Path, pszPath + sizeof(STR_INI_METACFG));
            }
        }
        else if ((strncmp(pszPath, STR_INI_METASYS, sizeof(STR_INI_METASYS) - 1) == 0) &&
                 ((pszPath[sizeof(STR_INI_METASYS) - 1] == '\0') ||
                  (pszPath[sizeof(STR_INI_METASYS) - 1] == '\\') ||
                  (pszPath[sizeof(STR_INI_METASYS) - 1] == '/')))
        {
            if (!osl_getSystemDir(Path, sizeof(Path)))
                return (sal_False);

            if ((pszPath[sizeof(STR_INI_METASYS) - 1] == '/') ||
                (pszPath[sizeof(STR_INI_METASYS) - 1] == '\\'))
            {
                if ((Path[strlen(Path) - 1] != '/') && (Path[strlen(Path) - 1] != '\\'))
                    strcat(Path, "\\");
                strcat(Path, pszPath + sizeof(STR_INI_METASYS));
            }
        }
        else if ((strncmp(pszPath, STR_INI_METAINS, sizeof(STR_INI_METAINS) - 1) == 0) &&
                 ((pszPath[sizeof(STR_INI_METAINS) - 1] == '\0') ||
                  (pszPath[sizeof(STR_INI_METAINS) - 1] == '\\') ||
                  (pszPath[sizeof(STR_INI_METAINS) - 1] == '/') ||
                  (pszPath[sizeof(STR_INI_METAINS) - 1] == '"')))
        {
            if (! lookupProfile(pszPath + sizeof(STR_INI_METAINS) - 1, File, Path))
                return (sal_False);
        }
        else
            strcpy(Path, pszPath);

        OSL_ASSERT(strlen(Path) <= _MAX_DIR);
    }
    else
    {
        oslSecurity security = osl_getCurrentSecurity();
        bFailed = ! osl_getConfigDir(security, Path, sizeof(Path));
        osl_freeSecurityHandle(security);

        if (bFailed) return (sal_False);
    }


    if ( Path[0]!= 0 && (Path[strlen(Path) - 1] != '/') && (Path[strlen(Path) - 1] != '\\'))
    {
        OSL_ASSERT(strlen(Path) > 0);
        strcat(Path, "\\");
    }

    /* append file name */
    strcat(Path, File);

    /* copy filename */
    strncpy(pszBuffer, Path, MaxLen);
    pszBuffer[MaxLen - 1] = '\0';

    return (strlen(Path) < MaxLen);
}


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

static osl_TFile* openFile(const sal_Char* pszFilename, sal_Bool bWriteable)
{
    sal_uInt32  action;
    APIRET      rc;
    osl_TFile*  pFile = calloc(1, sizeof(osl_TFile));

        ULONG attributes;
        ULONG flags;
        ULONG mode;

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

        if (rc = DosOpen((sal_Char*)pszFilename, &pFile->m_Handle, &action, 0, flags, attributes, mode, NULL))
        {
            if (rc == ERROR_TOO_MANY_OPEN_FILES)
            {
                LONG fhToAdd = 10;
                ULONG fhOld = 0;
                rc = DosSetRelMaxFH(&fhToAdd, &fhOld);
                rc = DosOpen((sal_Char*)pszFilename, &pFile->m_Handle, &action, 0, flags, attributes, mode, NULL);
            }
    }

        if ( (rc != NO_ERROR) && bWriteable)
        {
            free(pFile);
              return (NULL);
        }
        else
        {
            lockFile(pFile, bWriteable ? write_lock : read_lock);

            pFile->m_pWritePtr = pFile->m_WriteBuf;
            pFile->m_pReadPtr  = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);

            return (pFile);
        }
}

static osl_TStamp closeFile(osl_TFile* pFile)
{
    osl_TStamp stamp = {0, 0};

    if ( pFile == 0 )
    {
        return stamp;
    }

    if (pFile->m_Handle)
    {
        if (pFile->m_pWritePtr > pFile->m_WriteBuf)
        {
            sal_uInt32 Bytes;

            DosWrite(pFile->m_Handle, pFile->m_WriteBuf,
                     pFile->m_pWritePtr - pFile->m_WriteBuf,
                     &Bytes);
        }

        stamp = getFileStamp(pFile);

        lockFile(pFile, un_lock);

        DosClose(pFile->m_Handle);
    }

    free(pFile);

    return(stamp);
}

static sal_Bool rewindFile(osl_TFile* pFile, sal_Bool bTruncate)
{
    if (pFile->m_Handle)
    {
        sal_uInt32 Position;

        if (pFile->m_pWritePtr > pFile->m_WriteBuf)
        {
            sal_uInt32 Bytes;

            DosWrite(pFile->m_Handle, pFile->m_WriteBuf,
                     pFile->m_pWritePtr - pFile->m_WriteBuf,
                     &Bytes);

            pFile->m_pWritePtr = pFile->m_WriteBuf;
        }

        pFile->m_pReadPtr = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);

        DosSetFilePtr(pFile->m_Handle, 0, FILE_BEGIN, &Position);

        if (bTruncate)
            DosSetFileSize(pFile->m_Handle, 0);
    }

    return (sal_True);
}

static sal_Bool getLine(osl_TFile* pFile, const sal_Char *pszLine, int MaxLen)
{
    int         Free, Bytes;
    sal_Char*   pChr;
    sal_Char*   pLine = (sal_Char *)pszLine;
    sal_uInt32  Max;

    if (!pFile->m_Handle)
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

        Max = min(pChr - pFile->m_pReadPtr, MaxLen);
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
    int   Len   = strlen(pszLine);
    int   Bytes = pFile->m_pWritePtr - pFile->m_WriteBuf;
    int   Free  = sizeof(pFile->m_WriteBuf) - Bytes;
    int   Max   = min(Len, Free);
    sal_uInt32 Written;

    if (!pFile->m_Handle)
        return (sal_False);

    if ((Len + 2 - Max) > sizeof(pFile->m_WriteBuf))
    {
        /* Length of Line could not be splittet into the buffer,
         * so flush the buffer and write the line directly
         */
        if (DosWrite(pFile->m_Handle, pFile->m_WriteBuf, Bytes, &Written))
            return (sal_False);
        pFile->m_pWritePtr = pFile->m_WriteBuf;
        if (DosWrite(pFile->m_Handle, (sal_Char*)pszLine, Len, &Written))
            return (sal_False);
    }
    else
    {
        /* Copy maximum count of characters to the buffer */
        memcpy(pFile->m_pWritePtr, pszLine, Max);
        pFile->m_pWritePtr += Max;

        if ((Free -= Max) <= 1)
        {
            if (DosWrite(pFile->m_Handle, pFile->m_WriteBuf, sizeof(pFile->m_WriteBuf) - Free, &Written))
                return (sal_False);
            pFile->m_pWritePtr = pFile->m_WriteBuf;
        }

        if ((Len -= Max) > 0)
        {
            /* Line does not fit into the free space of the buffer.
             * Flush the buffer an copy the remaining part into the
             * the buffer.
             */

            memcpy(pFile->m_pWritePtr, pszLine + Max, Len);
            pFile->m_pWritePtr += Len;
        }
    }
    *(pFile->m_pWritePtr)++ = '\r';
    *(pFile->m_pWritePtr)++ = '\n';
    return sal_True;
}

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

    if (pProfile == NULL)
    {
        if ((pProfile = osl_openProfile(NULL, bWriteable ?
                                            osl_Profile_DEFAULT : (osl_Profile_DEFAULT | osl_Profile_WRITELOCK)))
            != NULL)
            pProfile->m_Flags |= FLG_AUTOOPEN;
    }
    else
    {
        if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
        {
            if (! (pProfile->m_Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK)))
            {
                osl_TStamp Stamp;

                if (! (pProfile->m_pFile = openFile(pProfile->m_Filename, bWriteable)))
                    return NULL;

                Stamp = getFileStamp(pProfile->m_pFile);

                if (memcmp(&Stamp, &(pProfile->m_Stamp), sizeof(osl_TStamp)))
                {
                    pProfile->m_Stamp = Stamp;

                    loadProfile(pProfile->m_pFile, pProfile);
                }
            }
            else
                /* A readlock file could not be written */
                if ((pProfile->m_Flags & osl_Profile_READLOCK) && bWriteable)
                    return (NULL);
        }
        else
        {
            /* hack: usualy you have a specific HAB, but NULL works here... */
            pProfile->m_hIni = PrfOpenProfile(NULL, pProfile->m_Filename);
            if (! pProfile->m_hIni)
                return (NULL);
        }

    }

    return (pProfile);
}

static sal_Bool releaseProfile(osl_TProfileImpl* pProfile)
{
    if ( pProfile == 0 )
    {
        return sal_False;
    }

    if (! (pProfile->m_Flags & osl_Profile_SYSTEM))
    {
        if (pProfile->m_Flags & FLG_AUTOOPEN)
            return (osl_closeProfile((oslProfile)pProfile));
        else
        {
            if (! (pProfile->m_Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK)))
            {
                if (pProfile->m_Flags & FLG_MODIFIED)
                    storeProfile(pProfile->m_pFile, pProfile, sal_False);

                closeFile(pProfile->m_pFile);
                pProfile->m_pFile = NULL;
            }
        }
    }
    else
        PrfCloseProfile(pProfile->m_hIni);

    return (sal_True);
}

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

            for (i = 0; i < (sizeof(SubDirs) / sizeof(SubDirs[0])); i++)
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






