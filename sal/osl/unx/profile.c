/*************************************************************************
 *
 *  $RCSfile: profile.c,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mfe $ $Date: 2001-02-26 16:17:49 $
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

#include <osl/diagnose.h>
#include <osl/profile.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/alloc.h>
#include <osl/util.h>

#define LINES_INI       32
#define LINES_ADD       10
#define SECTIONS_INI    5
#define SECTIONS_ADD    3
#define ENTRIES_INI     5
#define ENTRIES_ADD     3


#define STR_INI_EXTENSION   "rc"
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
#define SVERSION_PROFILE    "sofficerc"
#define SVERSION_OPTION     "userid:"
#define SVERSION_DIRS       { "bin", "program" }
#define SVERSION_USER       "user"

#define DEFAULT_PMODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

#define _BUILD_STR_(n)  # n
#define BUILD_STR(n)    _BUILD_STR_(n)


/*#define DEBUG_OSL_PROFILE*/
/*#define TRACE_OSL_PROFILE*/

/*****************************************************************************/
/* Data Type Definition */
/*****************************************************************************/

typedef time_t  osl_TStamp;

typedef enum _osl_TLockMode
{
    un_lock, read_lock, write_lock
} osl_TLockMode;

typedef struct _osl_TFile
{
    int     m_Handle;
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
    sal_uInt32  m_Line;
    sal_uInt32  m_Offset;
    sal_uInt32  m_Len;
} osl_TProfileEntry;

typedef struct _osl_TProfileSection
{
    sal_uInt32  m_Line;
    sal_uInt32  m_Offset;
    sal_uInt32  m_Len;
    sal_uInt32  m_NoEntries;
    sal_uInt32  m_MaxEntries;
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
    sal_Char    m_FileName[PATH_MAX + 1];
    sal_uInt32  m_NoLines;
    sal_uInt32  m_MaxLines;
    sal_uInt32  m_NoSections;
    sal_uInt32  m_MaxSections;
    sal_Char**  m_Lines;
    osl_TProfileSection* m_Sections;
    pthread_mutex_t m_AccessLock;
    sal_Bool    m_bIsValid;
    sal_Bool    m_bIsSoffice;
} osl_TProfileImpl;


/*****************************************************************************/
/* Static Module Function Declarations */
/*****************************************************************************/

static osl_TFile* openFileImpl(const sal_Char* pszFilename, oslProfileOption ProfileFlags);
static osl_TStamp closeFileImpl(osl_TFile* pFile, oslProfileOption Flags);
static sal_Bool   OslProfile_lockFile(const osl_TFile* pFile, osl_TLockMode eMode);
static sal_Bool   OslProfile_rewindFile(osl_TFile* pFile, sal_Bool bTruncate);
static osl_TStamp OslProfile_getFileStamp(osl_TFile* pFile);

static sal_Char*   OslProfile_getLine(osl_TFile* pFile);
static sal_Bool   OslProfile_putLine(osl_TFile* pFile, const sal_Char *pszLine);
static sal_Char* stripBlanks(sal_Char* String, sal_uInt32* pLen);
static sal_Char* addLine(osl_TProfileImpl* pProfile, const sal_Char* Line);
static sal_Char* insertLine(osl_TProfileImpl* pProfile, const sal_Char* Line, sal_uInt32 LineNo);
static void removeLine(osl_TProfileImpl* pProfile, sal_uInt32 LineNo);
static void setEntry(osl_TProfileImpl* pProfile, osl_TProfileSection* pSection,
                     sal_uInt32 NoEntry, sal_uInt32 Line,
                     sal_Char* Entry, sal_uInt32 Len);
static sal_Bool addEntry(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection,
                         int Line, sal_Char* Entry, sal_uInt32 Len);
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

static sal_Bool writeProfileImpl (osl_TFile* pFile);
static osl_TFile* osl_openTmpProfileImpl(osl_TProfileImpl*);
static sal_Bool osl_ProfileSwapProfileNames(osl_TProfileImpl*);
static void osl_ProfileGenerateExtension(sal_Char* pszFileName, sal_Char* pszExtension, sal_Char* pszTmpName);

extern sal_Bool SAL_CALL osl_psz_getHomeDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);
extern sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, rtl_uString **ustrDirectory);
extern oslProcessError SAL_CALL osl_psz_getExecutableFile(sal_Char* pszBuffer, sal_uInt32 Max);
oslProfile SAL_CALL osl_psz_openProfile(const sal_Char *pszProfileName, oslProfileOption Flags);
sal_Bool SAL_CALL osl_psz_getProfileName(const sal_Char* pszPath, const sal_Char* pszName,
                                         sal_Char* pszBuffer, sal_uInt32 MaxLen);
sal_Bool osl_getFullPath(const sal_Char* pszFilename, sal_Char* pszPath, sal_uInt32 MaxLen);
sal_Bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);
oslProcessError SAL_CALL osl_getCommandArgs(sal_Char* pszBuffer, sal_uInt32 Max);


static sal_Bool bGlobalSofficeFlag=sal_False;


/*****************************************************************************/
/* Exported Module Functions */
/*****************************************************************************/
oslProfile SAL_CALL osl_openProfile(rtl_uString *ustrProfileName, oslProfileOption Options)
{
    oslProfile pProfile=0;
    rtl_String* strProfileName=0;
    sal_Char* pszProfileName=0;


    if ( ustrProfileName != 0  && ustrProfileName->buffer[0] != 0 )
    {

        rtl_uString2String( &strProfileName,
                            rtl_uString_getStr(ustrProfileName),
                            rtl_uString_getLength(ustrProfileName),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszProfileName = rtl_string_getStr(strProfileName);

        if ( strncmp(pszProfileName,"//./",4) != 0 )
        {
            rtl_string_release(strProfileName);
            return 0;
        }
        pszProfileName+=3;
    }


    pProfile = osl_psz_openProfile(pszProfileName,Options);

    if ( strProfileName != 0 )
    {
        rtl_string_release(strProfileName);
    }


    return pProfile;
}


oslProfile SAL_CALL osl_psz_openProfile(const sal_Char *pszProfileName, oslProfileOption Flags)
{
    osl_TFile*        pFile;
    osl_TProfileImpl* pProfile;
    sal_Char          Filename[PATH_MAX];
    sal_Bool bRet = sal_False;

    Filename[0] = '\0';

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_openProfile\n");
#endif

    if (pszProfileName == NULL)
    {
        OSL_VERIFY(osl_psz_getProfileName(NULL, NULL, Filename, sizeof(Filename)));
        pszProfileName = Filename;
    }


#ifdef DEBUG_OSL_PROFILE
    Flags=osl_Profile_FLUSHWRITE;

    OSL_TRACE("opening '%s'\n",pszProfileName);
    if ( Flags == osl_Profile_DEFAULT )
    {
        OSL_TRACE("with osl_Profile_DEFAULT\n");
    }
    if ( Flags & osl_Profile_SYSTEM )
    {
        OSL_TRACE("with osl_Profile_SYSTEM\n");
    }
    if ( Flags & osl_Profile_READLOCK )
    {
        OSL_TRACE("with osl_Profile_READLOCK\n");
    }
    if ( Flags & osl_Profile_WRITELOCK )
    {
        OSL_TRACE("with osl_Profile_WRITELOCK\n");
    }
/*      if ( Flags & osl_Profile_READWRITE ) */
/*      { */
/*          OSL_TRACE("with osl_Profile_READWRITE\n"); */
/*      } */
    if ( Flags & osl_Profile_FLUSHWRITE )
    {
        OSL_TRACE("with osl_Profile_FLUSHWRITE\n");
    }
#endif


    if ( ( pFile = openFileImpl(pszProfileName, Flags ) ) == NULL )
    {
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_openProfile [not opened]\n");
#endif
        return (NULL);
    }


    pProfile = (osl_TProfileImpl*)calloc(1, sizeof(osl_TProfileImpl));

    if ( pProfile == 0 )
    {
        return 0;
    }

    pProfile->m_bIsSoffice = sal_False;

    if ( strstr(pszProfileName,"sofficerc") != 0 )
    {
        if ( bGlobalSofficeFlag == sal_True )
        {
            OSL_ENSURE(0,"Attempt to open sofficerc more than once\n");
        }
        else
        {
            bGlobalSofficeFlag=sal_True;
        }
        pProfile->m_bIsSoffice=sal_True;
    }

    pProfile->m_Flags = Flags & FLG_USER;

    if ( Flags & ( osl_Profile_READLOCK | osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE ) )
    {
        pProfile->m_pFile = pFile;
    }

    pthread_mutex_init(&(pProfile->m_AccessLock),PTHREAD_MUTEXATTR_DEFAULT);
    pProfile->m_bIsValid=sal_True;

    pProfile->m_Stamp = OslProfile_getFileStamp(pFile);
    bRet=loadProfile(pFile, pProfile);
    OSL_ASSERT(bRet);


    OSL_VERIFY(osl_getFullPath(pszProfileName, pProfile->m_FileName, sizeof(pProfile->m_FileName)));


    if (pProfile->m_pFile == NULL)
        closeFileImpl(pFile,pProfile->m_Flags);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_openProfile [ok]\n");
#endif
    return (pProfile);
}

sal_Bool SAL_CALL osl_closeProfile(oslProfile Profile)
{
    osl_TProfileImpl* pProfile = (osl_TProfileImpl*)Profile;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_closeProfile\n");
#endif

    if ( Profile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_closeProfile [profile==0]\n");
#endif
        return sal_False;
    }

    pthread_mutex_lock(&(pProfile->m_AccessLock));

    if ( pProfile->m_bIsValid == sal_False )
    {
        OSL_ASSERT(pProfile->m_bIsValid);
        pthread_mutex_unlock(&(pProfile->m_AccessLock));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_closeProfile [not valid]\n");
#endif
        return sal_False;
    }

    pProfile->m_bIsValid=sal_False;

/*      if (! (pProfile->m_Flags & osl_Profile_SYSTEM)) */
/*      { */
    if ( ! ( pProfile->m_Flags & osl_Profile_READLOCK ) && ( pProfile->m_Flags & FLG_MODIFIED ) )
    {
        pProfile = acquireProfile(Profile,sal_True);

        if ( pProfile != 0 )
        {
/*      if (pProfile->m_pFile == NULL) */
/*          pProfile->m_pFile = openFileImpl(pProfile->m_FileName, pProfile->m_Flags); */

            bRet=storeProfile(pProfile->m_pFile, pProfile, sal_True);
            OSL_ASSERT(bRet);
        }
    }
    else
    {
        pProfile = acquireProfile(Profile,sal_False);
    }


    if ( pProfile == 0 )
    {
        pthread_mutex_unlock(&(pProfile->m_AccessLock));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_closeProfile [pProfile==0]\n");
#endif
        return sal_False;
    }

    if (pProfile->m_pFile != NULL)
        closeFileImpl(pProfile->m_pFile,pProfile->m_Flags);
/*      } */

    pProfile->m_pFile = NULL;
    pProfile->m_FileName[0] = '\0';

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
                    pProfile->m_Lines[index]=0;
                }
            }
            free(pProfile->m_Lines);
            pProfile->m_Lines=0;
        }
        if ( pProfile->m_Sections != 0 )
        {
            /*osl_TProfileSection* pSections=pProfile->m_Sections;*/
            for ( index = 0 ; index < pProfile->m_NoSections ; ++index )
            {
                if ( pProfile->m_Sections[index].m_Entries != 0 )
                {
                    free(pProfile->m_Sections[index].m_Entries);
                    pProfile->m_Sections[index].m_Entries=0;
                }
            }
            free(pProfile->m_Sections);
            pProfile->m_Sections=0;
        }
    }

    pthread_mutex_unlock(&(pProfile->m_AccessLock));

    pthread_mutex_destroy(&(pProfile->m_AccessLock));

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

    pthread_mutex_lock(&(pProfile->m_AccessLock));

    if ( pProfile->m_bIsValid == sal_False )
    {
        OSL_ASSERT(pProfile->m_bIsValid);
        pthread_mutex_unlock(&(pProfile->m_AccessLock));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_flushProfile [not valid]\n");
#endif
        return sal_False;
    }

    pFile = pProfile->m_pFile;
    if ( !( pFile != 0 && pFile->m_Handle >= 0 ) )
    {
        pthread_mutex_unlock(&(pProfile->m_AccessLock));
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
        OSL_ASSERT(bRet);
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_flushProfile() [ok]\n");
#endif
    pthread_mutex_unlock(&(pProfile->m_AccessLock));
    return bRet;
}

static sal_Bool writeProfileImpl(osl_TFile* pFile)
{
    int BytesWritten=0;
#ifdef DEBUG
    unsigned int nLen=0;
#endif

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_writeProfileImpl()\n");
#endif

    if ( !( pFile != 0 && pFile->m_Handle >= 0 ) || ( pFile->m_pWriteBuf == 0 ) )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_writeProfileImpl() [invalid args]\n");
#endif
        return sal_False;
    }

#ifdef DEBUG_OSL_PROFILE
/*    OSL_TRACE("File Buffer in writeProfileImpl '%s' size == '%i' '%i'(%i)\n",
      pFile->m_pWriteBuf,pFile->m_nWriteBufLen,strlen(pFile->m_pWriteBuf),pFile->m_nWriteBufLen - pFile->m_nWriteBufFree);*/
#endif

#ifdef DEBUG
    nLen=strlen(pFile->m_pWriteBuf);
    OSL_ASSERT(nLen == (pFile->m_nWriteBufLen - pFile->m_nWriteBufFree));
#endif

    BytesWritten = write(pFile->m_Handle, pFile->m_pWriteBuf, pFile->m_nWriteBufLen - pFile->m_nWriteBufFree);

    if ( BytesWritten <= 0 )
    {
        OSL_TRACE("write failed '%s'\n",strerror(errno));
/*        OSL_TRACE("Out osl_writeProfileImpl() [write '%s']\n",strerror(errno));*/
        return (sal_False);
    }

#ifdef DEBUG
    OSL_ASSERT(BytesWritten == nLen);
#endif

    free(pFile->m_pWriteBuf);
    pFile->m_pWriteBuf=0;
    pFile->m_nWriteBufLen=0;
    pFile->m_nWriteBufFree=0;
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_writeProfileImpl() [ok]\n");
#endif
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
    osl_TProfileImpl*    pProfile=0;
    osl_TProfileImpl*    pTmpProfile=0;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_readProfileString\n");
#endif

    pTmpProfile = (osl_TProfileImpl*) Profile;

    if ( pTmpProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_readProfileString [pTmpProfile==0]\n");
#endif
        return sal_False;
    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( pTmpProfile->m_bIsValid == sal_False )
    {
        OSL_ASSERT(pProfile->m_bIsValid);
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_readProfileString [not valid]\n");
#endif
        return sal_False;
    }

    pProfile = acquireProfile(Profile, sal_False);

    if ( pProfile == NULL )
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
        {
            pStr++;
        }
        else
        {
            pStr=(sal_Char*)pszDefault;
        }

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
    { /* not implemented */ }


    bRet=releaseProfile(pProfile);
    OSL_ASSERT(bRet);

    if ( pStr == 0 )
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_readProfileString [pStr==0]\n");
#endif
        return sal_False;
    }

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

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
    Line[0] = '\0';

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_readProfileBool\n");
#endif

    if (osl_readProfileString(Profile, pszSection, pszEntry, Line, sizeof(Line), ""))
    {
        if ((strcasecmp(Line, STR_INI_BOOLYES) == 0) ||
            (strcasecmp(Line, STR_INI_BOOLON)  == 0) ||
            (strcasecmp(Line, STR_INI_BOOLONE) == 0))
            Default = sal_True;
        else
            if ((strcasecmp(Line, STR_INI_BOOLNO)   == 0) ||
                (strcasecmp(Line, STR_INI_BOOLOFF)  == 0) ||
                (strcasecmp(Line, STR_INI_BOOLZERO) == 0))
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
    sal_uInt32  i;
    sal_Char    Line[256];
    Line[0] = '\0';

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_readProfileIdent\n");
#endif

    if (osl_readProfileString(Profile, pszSection, pszEntry, Line, sizeof(Line), ""))
    {
        i = 0;
        while (Strings[i] != NULL)
        {
            if (strcasecmp(Line, Strings[i]) == 0)
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
    sal_uInt32  i;
    sal_Bool bRet = sal_False;
    sal_uInt32    NoEntry;
    sal_Char* pStr;
    sal_Char*       Line = 0;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = 0;
    osl_TProfileImpl*    pTmpProfile = 0;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_writeProfileString\n");
#endif

    pTmpProfile = (osl_TProfileImpl*) Profile;

    if ( pTmpProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_writeProfileString [pTmpProfile==0]\n");
#endif
        return sal_False;
    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( pTmpProfile->m_bIsValid == sal_False )
    {
        OSL_ASSERT(pTmpProfile->m_bIsValid);
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_writeProfileString [not valid]\n");
#endif
        return sal_False;
    }

    pProfile=acquireProfile(Profile, sal_True);

    if (pProfile == NULL)
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_writeProfileString [pProfile==0]\n");
#endif
        return (sal_False);
    }

    Line = (sal_Char*) malloc(strlen(pszEntry)+strlen(pszString)+48);

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
                bRet=releaseProfile(pProfile);
                OSL_ASSERT(bRet);

                pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

                free(Line);

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
                bRet=releaseProfile(pProfile);
                OSL_ASSERT(bRet);

                pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
                free(Line);

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
    else {
        /* not implemented */
    }

    bRet = releaseProfile(pProfile);
    OSL_ASSERT(bRet);

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
    if ( Line!= 0 )
    {
        free(Line);
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_writeProfileString [ok]\n");
#endif

    return bRet;
}


sal_Bool SAL_CALL osl_writeProfileBool(oslProfile Profile,
                             const sal_Char* pszSection, const sal_Char* pszEntry,
                             sal_Bool Value)
{
    sal_Bool bRet=sal_False;

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
    sal_Bool bRet=sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_writeProfileIdent\n");
#endif

    for (n = 0; Strings[n] != NULL; n++);

    if ((i = Value - FirstId) >= n)
        bRet=sal_False;
    else
        bRet = osl_writeProfileString(Profile, pszSection, pszEntry, Strings[i]);

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
    osl_TProfileImpl*    pTmpProfile = 0;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_removeProfileEntry\n");
#endif

    pTmpProfile = (osl_TProfileImpl*) Profile;

    if ( pTmpProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_removeProfileEntry [pProfile==0]\n");
#endif
        return sal_False;
    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( pTmpProfile->m_bIsValid == sal_False )
    {
        OSL_ASSERT(pTmpProfile->m_bIsValid);
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_removeProfileEntry [not valid]\n");
#endif
        return sal_False;
    }


    pProfile = acquireProfile(Profile, sal_True);

    if (pProfile == NULL)
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
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
    { /* not implemented */ }


    bRet = releaseProfile(pProfile);
    OSL_ASSERT(bRet);

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

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
    osl_TProfileImpl*    pTmpProfile = 0;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_getProfileSectionEntries\n");
#endif

    pTmpProfile = (osl_TProfileImpl*) Profile;

    if ( pTmpProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_getProfileSectionEntries [pTmpProfile==0]\n");
#endif
        return sal_False;

    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( pTmpProfile->m_bIsValid == sal_False )
    {
        OSL_ASSERT(pTmpProfile->m_bIsValid);

        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_getProfileSectionEntries [not valid]\n");
#endif

        return sal_False;
    }

    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

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
    else {
        /* not implemented */
    }

    bRet=releaseProfile(pProfile);
    OSL_ASSERT(bRet);

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_getProfileSectionEntries [ok]\n");
#endif

    return (n);
}

sal_uInt32 SAL_CALL osl_getProfileSections(oslProfile Profile, sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_uInt32    i, n = 0;
    osl_TProfileSection* pSec;
    osl_TProfileImpl*    pProfile = 0;
    osl_TProfileImpl*    pTmpProfile = 0;
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_getProfileSections\n");
#endif

    pTmpProfile = (osl_TProfileImpl*) Profile;

    if ( pTmpProfile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_getProfileSections [pTmpProfile==0]\n");
#endif
        return sal_False;
    }

    pthread_mutex_lock(&(pTmpProfile->m_AccessLock));

    if ( pTmpProfile->m_bIsValid == sal_False )
    {
        OSL_ASSERT(pTmpProfile->m_bIsValid);
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_getProfileSections [not valid]\n");
#endif
        return sal_False;
    }

    pProfile = acquireProfile(Profile, sal_False);

    if (pProfile == NULL)
    {
        pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_getProfileSections [pProfile==0]\n");
#endif
        return (0);
    }

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
    { /* not implemented */ }


    bRet=releaseProfile(pProfile);
    OSL_ASSERT(bRet);

    pthread_mutex_unlock(&(pTmpProfile->m_AccessLock));

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_getProfileSections [ok]\n");
#endif

    return (n);
}

sal_Bool SAL_CALL osl_getProfileName(rtl_uString* ustrPath, rtl_uString* ustrName, rtl_uString** strProfileName)
{
    sal_Bool bRet=sal_False;
    sal_Char pszBuffer[PATH_MAX];
    rtl_uString* strNativeName=0;
    rtl_String* strPath=0;
    rtl_String* strName=0;
    sal_Char* pszPath=0;
    sal_Char* pszName=0;

    pszBuffer[0] = '\0';


    if ( ustrPath != 0 && ustrPath->buffer[0] != 0 )
    {
        rtl_uString2String( &strPath,
                            rtl_uString_getStr(ustrPath),
                            rtl_uString_getLength(ustrPath),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszPath = rtl_string_getStr(strPath);

        /* Don't skip UNC prefix if metatag is used */

        if ( strncmp(pszPath,"//./",4) == 0 )
        {
            pszPath+=3;
        }
        else if ( strncmp(pszPath, "?", 1) != 0 )
        {
            rtl_string_release(strPath);
            return 0;
        }
    }

    if ( ustrName != 0 && ustrName->buffer[0] != 0 )
    {
        rtl_uString2String( &strName,
                            rtl_uString_getStr(ustrName),
                            rtl_uString_getLength(ustrName),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszName = rtl_string_getStr(strName);
    }



    bRet=osl_psz_getProfileName(pszPath,pszName,pszBuffer,sizeof(pszBuffer));


    if ( strPath != 0 )
    {
        rtl_string_release(strPath);
    }

    if ( strName != 0 )
    {
        rtl_string_release(strName);
    }

    /* Convert result to UNC notation */

    rtl_string2UString( &strNativeName, pszBuffer, rtl_str_getLength( pszBuffer ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );

    osl_normalizePath( strNativeName, strProfileName );
    rtl_uString_release( strNativeName );


    return bRet;
}


sal_Bool SAL_CALL osl_psz_getProfileName(const sal_Char* pszPath, const sal_Char* pszName,
                           sal_Char* pszBuffer, sal_uInt32 MaxLen)
{
    sal_Char *pChr=0;
    const sal_Char *pStr=0;
    sal_Char  Home[PATH_MAX];
    sal_Char  Config[PATH_MAX];
    sal_Char  Path[PATH_MAX];
    sal_Char  File[PATH_MAX];
    sal_Bool  bFailed;
    sal_Bool  bHidden = sal_False;
    oslSecurity security;

    Home[0] = '\0';
    Config[0] = '\0';
    Path[0] = '\0';
    File[0] = '\0';

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_getProfileName\n");
#endif

    security = osl_getCurrentSecurity();
    bFailed = ! (osl_psz_getHomeDir(security, Home, sizeof(Home)) &&
                 osl_psz_getConfigDir(security, Config, sizeof(Config)));
    osl_freeSecurityHandle(security);

    if (bFailed)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out osl_getProfileName [get directory]\n");
#endif
        return (sal_False);
    }


    if (Home[strlen(Home) - 1] != '/')
    {
        strcat(Home, "/");
    }

    if (Config[strlen(Config) - 1] != '/')
    {
        strcat(Config, "/");
    }


    /* build file name */
    if (pszName == NULL)
    {
        if (osl_psz_getExecutableFile(Path, sizeof(Path)) != osl_Process_E_None)
        {
#ifdef TRACE_OSL_PROFILE
            OSL_TRACE("Out osl_getProfileName [get exe file name]\n");
#endif
            return (sal_False);
        }


        /* remove path from filename */
        if ((pStr = strrchr(Path, '/')) == NULL)
            pStr = Path;
        else
            pStr++;

        if (((pChr = strrchr(pStr, '.')) != NULL) && (pChr != pStr))
            *pChr = '\0';
    }
    else
    {
        /* FIXME: mfe: assignment of 'const char' to 'char' */
        pStr = pszName;
    }

    strcpy(File, pStr);

    /* add default extensiom */
    if (strchr(File + 1, '.') == NULL)
        strcat(File, STR_INI_EXTENSION);

    /* build directory path */
    if (pszPath)
    {
        sal_Char  Dir[PATH_MAX];
        sal_Char  Loc[PATH_MAX];

        Dir[0] = '\0';
        Loc[0] = '\0';

        if ((strncmp(pszPath, STR_INI_METAHOME, sizeof(STR_INI_METAHOME) - 1) == 0) &&
            ((pszPath[sizeof(STR_INI_METAHOME) - 1] == '\0') ||
             (pszPath[sizeof(STR_INI_METAHOME) - 1] == '/')))
        {
            strcpy(Dir, Home);
            if (pszPath[sizeof(STR_INI_METAHOME) - 1] == '/')
                strcat(Dir, pszPath + sizeof(STR_INI_METAHOME));
            pszPath = Dir;
        }
        else if ((strncmp(pszPath, STR_INI_METACFG, sizeof(STR_INI_METACFG) - 1) == 0) &&
                 ((pszPath[sizeof(STR_INI_METACFG) - 1] == '\0') ||
                 (pszPath[sizeof(STR_INI_METACFG) - 1] == '/')))
        {
            strcpy(Dir, Config);
            if (pszPath[sizeof(STR_INI_METACFG) - 1] == '/')
                strcat(Dir, pszPath + sizeof(STR_INI_METACFG));
            pszPath = Dir;
        }
        else if ((strncmp(pszPath, STR_INI_METASYS, sizeof(STR_INI_METASYS) - 1) == 0) &&
                 ((pszPath[sizeof(STR_INI_METASYS) - 1] == '\0') ||
                 (pszPath[sizeof(STR_INI_METASYS) - 1] == '/')))
        {
            strcpy(Dir, "/etc/");
            if (pszPath[sizeof(STR_INI_METASYS) - 1] == '/')
                strcat(Dir, pszPath + sizeof(STR_INI_METASYS));
            pszPath = Dir;
        }
        else if ((strncmp(pszPath, STR_INI_METAINS, sizeof(STR_INI_METAINS) - 1) == 0) &&
                 ((pszPath[sizeof(STR_INI_METAINS) - 1] == '\0') ||
                  (pszPath[sizeof(STR_INI_METAINS) - 1] == '/') ||
                  (pszPath[sizeof(STR_INI_METAINS) - 1] == '"')))
        {
            if (! lookupProfile(pszPath + sizeof(STR_INI_METAINS) - 1, File, Dir))
            {
#ifdef TRACE_OSL_PROFILE
                OSL_TRACE("Out osl_getProfileName [lookup profile]\n");
#endif
                return (sal_False);
            }

            pszPath = Dir;
        }

        OSL_VERIFY(osl_getFullPath(Home, Loc, sizeof(Loc)));

        /* when file is in the home directory it should be hidden */
        if (osl_getFullPath(pszPath, Path, sizeof(Path)))
        {
            if (Path[strlen(Path) - 1] != '/') strcat(Path, "/");

            if (strcmp(Loc, Path) == 0)
                bHidden = sal_True;
        }

        OSL_ASSERT(strlen(pszPath) <= sizeof(Path));
        strcpy(Path, pszPath);
        if (Path[strlen(Path) - 1] != '/') strcat(Path, "/");
    }
    else
    {
        /* config files should be hidden if in the home directory */
        if (strcmp(Home, Config) == 0)
            bHidden = sal_True;

        strcpy(Path, Config);
    }

    if (bHidden && (Path[0] != '.'))
        strcat(Path, ".");

    strcat(Path, File);

    /* copy filename */
    strncpy(pszBuffer, Path, MaxLen);
    pszBuffer[MaxLen - 1] = '\0';

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_getProfileName [ok]\n");
#endif

    return (strlen(File) < MaxLen);
}

/*****************************************************************************/
/* Static Module Functions */
/*****************************************************************************/

static osl_TStamp OslProfile_getFileStamp(osl_TFile* pFile)
{
    struct stat status;

    if ( (pFile->m_Handle < 0) || (fstat(pFile->m_Handle, &status) < 0) )
    {
        return (0);
    }


    return (status.st_mtime);
}

static sal_Bool OslProfile_lockFile(const osl_TFile* pFile, osl_TLockMode eMode)
{
    struct flock lock;
    /* boring hack, but initializers for static vars must be constant */
    static sal_Bool bIsInitialized = sal_False;
    static sal_Bool bLockingDisabled;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  OslProfile_lockFile\n");
#endif

    if ( !bIsInitialized )
    {
        sal_Char* pEnvValue;
        pEnvValue = getenv( "STAR_PROFILE_LOCKING_DISABLED" );

        if ( pEnvValue == 0 )
        {
            bLockingDisabled = sal_False;

        }
        else
        {
            bLockingDisabled = sal_True;
        }

        bIsInitialized = sal_True;
    }

    if (pFile->m_Handle < 0)
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out OslProfile_lockFile [invalid file handle]\n");
#endif
        return (sal_False);
    }


    if ( bLockingDisabled )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out OslProfile_lockFile [locking disabled]\n");
#endif
        return (sal_True);
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

    if ( fcntl(pFile->m_Handle, F_SETLKW, &lock) == -1 )
    {
        OSL_TRACE("fcntl returned -1 (%s)\n",strerror(errno));
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out OslProfile_lockFile [fcntl F_SETLKW]\n");
#endif
        return sal_False;
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out OslProfile_lockFile [ok]\n");
#endif
    return sal_True;
}

static osl_TFile* openFileImpl(const sal_Char* pszFilename, oslProfileOption ProfileFlags )
{
    int        Flags;
    osl_TFile* pFile = (osl_TFile*) calloc(1, sizeof(osl_TFile));
    sal_Bool bWriteable = sal_False;

/*    if ( ProfileFlags & ( osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE | osl_Profile_READWRITE ) )*/
    if ( ProfileFlags & ( osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE ) )
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("setting bWriteable to TRUE\n");
#endif
        bWriteable=sal_True;
    }

    if (! bWriteable)
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("opening '%s' read only\n",pszFilename);
#endif

        pFile->m_Handle = open(pszFilename, O_RDONLY);
        /* mfe: argghh!!! do not check if the file could be openend */
        /*      default mode expects it that way!!!                 */
    }
    else
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("opening '%s' read/write\n",pszFilename);
#endif
        if (((pFile->m_Handle = open(pszFilename, O_RDWR | O_CREAT | O_EXCL, DEFAULT_PMODE)) < 0) &&
            ((pFile->m_Handle = open(pszFilename, O_RDWR)) < 0))
        {
            free(pFile);
#ifdef TRACE_OSL_PROFILE
            OSL_TRACE("Out openFileImpl [open read/write]\n");
#endif
            return (NULL);
        }
    }

    /* set close-on-exec flag */
    if ((Flags = fcntl(pFile->m_Handle, F_GETFD, 0)) != -1)
    {
        Flags |= FD_CLOEXEC;
        fcntl(pFile->m_Handle, F_SETFD, Flags);
    }

    pFile->m_pWriteBuf=0;
    pFile->m_nWriteBufFree=0;
    pFile->m_nWriteBufLen=0;

    if ( ProfileFlags & (osl_Profile_WRITELOCK | osl_Profile_READLOCK ) )
    {
#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("locking '%s' file\n",pszFilename);
#endif
        OslProfile_lockFile(pFile, bWriteable ? write_lock : read_lock);
    }

    /* mfe: new WriteBuf obsolete */
/*  pFile->m_pWritePtr = pFile->m_WriteBuf;*/
/*  pFile->m_pReadPtr  = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);*/

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out openFileImpl [ok]\n");
#endif
    return (pFile);
}

static osl_TStamp closeFileImpl(osl_TFile* pFile, oslProfileOption Flags)
{
    osl_TStamp stamp = 0;

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  closeFileImpl\n");
#endif

    if ( pFile == 0 )
    {
#ifdef TRACE_OSL_PROFILE
        OSL_TRACE("Out closeFileImpl [pFile == 0]\n");
#endif
        return stamp;
    }

    if ( pFile->m_Handle >= 0 )
    {
        /* mfe: new WriteBuf obsolete */
        /* we just closing the file here, DO NOT write, it has to be handled in higher levels */
/*          if (pFile->m_pWritePtr > pFile->m_WriteBuf)  */
/*              write(pFile->m_Handle, pFile->m_WriteBuf,  */
/*                    pFile->m_pWritePtr - pFile->m_WriteBuf); */

        stamp = OslProfile_getFileStamp(pFile);

        if ( Flags & (osl_Profile_WRITELOCK | osl_Profile_WRITELOCK ) )
        {
            OslProfile_lockFile(pFile, un_lock);
        }

        close(pFile->m_Handle);
        pFile->m_Handle = -1;
    }


    if ( pFile->m_pWriteBuf )
    {
        free(pFile->m_pWriteBuf);
    }

    free(pFile);

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out closeFileImpl [ok]\n");
#endif

    return(stamp);
}

static sal_Bool OslProfile_rewindFile(osl_TFile* pFile, sal_Bool bTruncate)
{
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  osl_OslProfile_rewindFile\n");
#endif

    if (pFile->m_Handle >= 0)
    {
        /* mfe: new WriteBuf obsolete */
        /* we just closing the file here, DO NOT write, it has to be handled in higher levels */
/*          if (pFile->m_pWritePtr > pFile->m_WriteBuf)  */
/*          { */
/*              write(pFile->m_Handle, pFile->m_WriteBuf,  */
/*                    pFile->m_pWritePtr - pFile->m_WriteBuf); */

/*              pFile->m_pWritePtr = pFile->m_WriteBuf; */
/*          } */

        pFile->m_pReadPtr  = pFile->m_ReadBuf + sizeof(pFile->m_ReadBuf);

#ifdef DEBUG_OSL_PROFILE
        OSL_TRACE("rewinding\n");
#endif
        lseek(pFile->m_Handle, SEEK_SET, 0L);

        if (bTruncate)
        {
#ifdef DEBUG_OSL_PROFILE
            OSL_TRACE("truncating\n");
#endif
            ftruncate(pFile->m_Handle, 0L);
        }

    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out osl_OslProfile_rewindFile [ok]\n");
#endif
    return (sal_True);
}


static sal_Char* OslProfile_getLine(osl_TFile* pFile)
{
    int   Max, Free, Bytes, nLineBytes = 0;
    sal_Char* pChr;
    sal_Char* pLine = NULL;
    sal_Char* pNewLine;

    if ( pFile == 0 )
    {
        return 0;
    }

    if (pFile->m_Handle < 0)
        return NULL;

    do
    {
        Bytes = sizeof(pFile->m_ReadBuf) - (pFile->m_pReadPtr - pFile->m_ReadBuf);

        if (Bytes <= 1)
        {
            /* refill buffer */
            memcpy(pFile->m_ReadBuf, pFile->m_pReadPtr, Bytes);
            pFile->m_pReadPtr = pFile->m_ReadBuf;

            Free = sizeof(pFile->m_ReadBuf) - Bytes;

            if ((Max = read(pFile->m_Handle, &pFile->m_ReadBuf[Bytes], Free)) < 0)
            {
                OSL_TRACE("read failed '%s'\n",strerror(errno));

                if( pLine )
                    rtl_freeMemory( pLine );
                pLine = NULL;
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
        pNewLine = (sal_Char*) rtl_allocateMemory( nLineBytes + Max + 1 );
        if( pLine )
        {
            memcpy( pNewLine, pLine, nLineBytes );
            rtl_freeMemory( pLine );
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
    while (Max > 0);

#ifdef DEBUG_OSL_PROFILE
/*  OSL_TRACE( "read line: \"%s\" (%d bytes)\n", pLine ? pLine : "<nil>", nLineBytes );*/
#endif

    return pLine;
}

static sal_Bool OslProfile_putLine(osl_TFile* pFile, const sal_Char *pszLine)
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
    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len]='\n';
    pFile->m_pWriteBuf[pFile->m_nWriteBufLen - pFile->m_nWriteBufFree + Len + 1]='\0';

    pFile->m_nWriteBufFree-=Len+1;

#ifdef DEBUG_OSL_PROFILE
/*    OSL_TRACE("File Buffer in _putLine '%s' size == '%i' '%i'(%i)\n",
      pFile->m_pBuf,pFile->m_BufLen,strlen(pFile->m_pBuf),pFile->m_BufLen - pFile->m_BufFree);*/
#endif

    return sal_True;
}

/* platform specific end */

static sal_Char* stripBlanks(sal_Char* String, sal_uInt32* pLen)
{
    if ( ( pLen != NULL ) && ( *pLen != 0 ) )
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

    return (String);
}

static sal_Char* addLine(osl_TProfileImpl* pProfile, const sal_Char* Line)
{
    if (pProfile->m_NoLines >= pProfile->m_MaxLines)
    {
        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_MaxLines = LINES_INI;
            pProfile->m_Lines = (sal_Char **)malloc(pProfile->m_MaxLines * sizeof(sal_Char *));
            memset(pProfile->m_Lines,0,pProfile->m_MaxLines * sizeof(sal_Char *));
        }
        else
        {
            unsigned int index=0;
            unsigned int oldmax=pProfile->m_MaxLines;

            pProfile->m_MaxLines += LINES_ADD;
            pProfile->m_Lines = (sal_Char **)realloc(pProfile->m_Lines,
                                                 pProfile->m_MaxLines * sizeof(sal_Char *));
            for ( index = oldmax ; index < pProfile->m_MaxLines ; ++index )
            {
                pProfile->m_Lines[index]=0;
            }
        }

        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_NoLines  = 0;
            pProfile->m_MaxLines = 0;
            return (NULL);
        }

    }

    if ( pProfile->m_Lines != 0 && pProfile->m_Lines[pProfile->m_NoLines] != 0 )
    {
        free(pProfile->m_Lines[pProfile->m_NoLines]);
    }
    pProfile->m_Lines[pProfile->m_NoLines++] = strdup(Line);

    return (pProfile->m_Lines[pProfile->m_NoLines - 1]);
}

static sal_Char* insertLine(osl_TProfileImpl* pProfile, const sal_Char* Line, sal_uInt32 LineNo)
{
    if (pProfile->m_NoLines >= pProfile->m_MaxLines)
    {
        if (pProfile->m_Lines == NULL)
        {
            pProfile->m_MaxLines = LINES_INI;
            pProfile->m_Lines = (sal_Char **)malloc(pProfile->m_MaxLines * sizeof(sal_Char *));
            memset(pProfile->m_Lines,0,pProfile->m_MaxLines * sizeof(sal_Char *));
        }
        else
        {
            pProfile->m_MaxLines += LINES_ADD;
            pProfile->m_Lines = (sal_Char **)realloc(pProfile->m_Lines,
                                                 pProfile->m_MaxLines * sizeof(sal_Char *));

            memset(&pProfile->m_Lines[pProfile->m_NoLines],
                0,
                (pProfile->m_MaxLines - pProfile->m_NoLines - 1) * sizeof(sal_Char*));
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
        pProfile->m_Lines[LineNo]=0;
        if (pProfile->m_NoLines - LineNo > 1)
        {
            sal_uInt32 i, n;
            osl_TProfileSection* pSec;

            memmove(&pProfile->m_Lines[LineNo], &pProfile->m_Lines[LineNo + 1],
                    (pProfile->m_NoLines - LineNo - 1) * sizeof(sal_Char *));

            memset(&pProfile->m_Lines[pProfile->m_NoLines - 1],
                0,
                (pProfile->m_MaxLines - pProfile->m_NoLines) * sizeof(sal_Char*));

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
                     sal_Char* Entry, sal_uInt32 Len)
{
    Entry = stripBlanks(Entry, &Len);
    pSection->m_Entries[NoEntry].m_Line   = Line;
    pSection->m_Entries[NoEntry].m_Offset = Entry - pProfile->m_Lines[Line];
    pSection->m_Entries[NoEntry].m_Len    = Len;

    return;
}

static sal_Bool addEntry(osl_TProfileImpl* pProfile, osl_TProfileSection *pSection,
                         int Line, sal_Char* Entry, sal_uInt32 Len)
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
            pProfile->m_Sections = (osl_TProfileSection *)malloc(pProfile->m_MaxSections * sizeof(osl_TProfileSection));
            memset(pProfile->m_Sections,0,pProfile->m_MaxSections * sizeof(osl_TProfileSection));
        }
        else
        {
            unsigned int index=0;
            unsigned int oldmax=pProfile->m_MaxSections;

            pProfile->m_MaxSections += SECTIONS_ADD;
            pProfile->m_Sections = (osl_TProfileSection *)realloc(pProfile->m_Sections,
                                          pProfile->m_MaxSections * sizeof(osl_TProfileSection));
            for ( index = oldmax ; index < pProfile->m_MaxSections ; ++index )
            {
                pProfile->m_Sections[index].m_Entries=0;
            }
        }

        if (pProfile->m_Sections == NULL)
        {
            pProfile->m_NoSections = 0;
            pProfile->m_MaxSections = 0;
            return (sal_False);
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
        sal_uInt32  Len;
        const sal_Char* pStr;
        osl_TProfileSection* pSec=0;

    Len = strlen(Section);
/*    Section = (sal_Char *)stripBlanks(Section, &Len);*/

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
/*        Entry = stripBlanks(Entry, &Len);*/

        *pNoEntry = pSec->m_NoEntries;

        for (i = 0; i < pSec->m_NoEntries; i++)
        {
            pStr = &pProfile->m_Lines[pSec->m_Entries[i].m_Line]
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
        pSec = NULL;

    return (pSec);
}

static sal_Bool loadProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile)
{
    sal_uInt32  i;
    sal_Char*       pStr;
    sal_Char*       pChar;

    sal_Char* pLine;
    sal_Char* bWasAdded = NULL;

    pProfile->m_NoLines    = 0;
    pProfile->m_NoSections = 0;

    if ( pFile == 0 )
    {
        return sal_False;
    }

    if ( pProfile == 0 )
    {
        return sal_False;
    }

    OSL_VERIFY(OslProfile_rewindFile(pFile, sal_False));

    while ( ( pLine=OslProfile_getLine(pFile) ) != 0 )
    {
        bWasAdded = addLine( pProfile, pLine );
        rtl_freeMemory( pLine );
        OSL_ASSERT(bWasAdded);
        if ( ! bWasAdded )
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
            {
                OSL_ASSERT(0);
                continue;
/*                return (sal_False);*/
            }

        }
        else
        {
            /* new section */

            if (! addSection(pProfile, i, pStr + 1, pChar - pStr - 1))
            {
                OSL_ASSERT(0);
                continue;
/*                return (sal_False);*/
            }

        }
    }

    return (sal_True);
}

static sal_Bool storeProfile(osl_TFile* pFile, osl_TProfileImpl* pProfile, sal_Bool bCleanup)
{
#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("In  storeProfile\n");
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

            OSL_VERIFY(OslProfile_rewindFile(pTmpFile, sal_True));

            for ( i = 0 ; i < pProfile->m_NoLines ; i++ )
            {
                OSL_VERIFY(OslProfile_putLine(pTmpFile, pProfile->m_Lines[i]));
            }

            if ( pProfile->m_bIsSoffice == sal_True )
            {
/*                  OSL_ENSURE( ( ( pTmpFile->m_nWriteBufLen - pTmpFile->m_nWriteBufFree ) > 5000 ), "Profile to write is less than 5000 bytes\n"); */
/*                  OSL_ENSURE( ( pProfile->m_NoSections > 15 ), "Profile to write has less than 15 Sections\n"); */
/*                  OSL_ENSURE( ( pProfile->m_NoLines > 170 ), "Profile to write has less than 170 lines\n"); */
/*                  OSL_ENSURE( ( i > 170 ), "less than 170 lines to be written in Profile\n"); */
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
                OSL_TRACE("Out storeProfile [not flushed]\n");
#endif
                closeFileImpl(pTmpFile,pProfile->m_Flags);

                return sal_False;
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
            pProfile->m_Lines = NULL;
            pProfile->m_NoLines = 0;
            pProfile->m_MaxLines = 0;

            while (pProfile->m_NoSections > 0)
                removeSection(pProfile, &pProfile->m_Sections[pProfile->m_NoSections - 1]);

            free(pProfile->m_Sections);
            pProfile->m_Sections = NULL;
            pProfile->m_NoSections = 0;
            pProfile->m_MaxSections = 0;
        }
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out storeProfile [ok]\n");
#endif
    return (sal_True);
}


static osl_TFile* osl_openTmpProfileImpl(osl_TProfileImpl* pProfile)
{
    osl_TFile* pFile=0;
    sal_Char* pszExtension = "tmp";
    sal_Char pszTmpName[PATH_MAX];
    oslProfileOption PFlags=0;

    pszTmpName[0] = '\0';

    /* generate tmp profilename */
    osl_ProfileGenerateExtension(pProfile->m_FileName,pszExtension,pszTmpName);

    if ( pszTmpName[0] == 0 )
    {
        return 0;
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

static sal_Bool osl_ProfileSwapProfileNames(osl_TProfileImpl* pProfile)
{
      sal_Bool bRet = sal_False;

      sal_Char pszBakFile[PATH_MAX];
      sal_Char pszTmpFile[PATH_MAX];
      sal_Char pszIniFile[PATH_MAX];

    pszBakFile[0] = '\0';
    pszTmpFile[0] = '\0';
    pszIniFile[0] = '\0';

      osl_ProfileGenerateExtension(pProfile->m_FileName,"bak",pszBakFile);

    strcpy(pszIniFile,pProfile->m_FileName);

    osl_ProfileGenerateExtension(pProfile->m_FileName,"tmp",pszTmpFile);

      /* unlink bak */
      unlink( pszBakFile );

      /* rename ini bak */
      rename( pszIniFile, pszBakFile );

      /* rename tmp ini */
      rename( pszTmpFile, pszIniFile );

    return bRet;
}


static void osl_ProfileGenerateExtension(sal_Char* pszFileName, sal_Char* pszExtension, sal_Char* pszTmpName)
{

    strcpy(pszTmpName,pszFileName);
    strcat(pszTmpName,".");
    strcat(pszTmpName,pszExtension);

    return;
}


static osl_TProfileImpl* acquireProfile(oslProfile Profile, sal_Bool bWriteable)
{
    osl_TProfileImpl* pProfile = (osl_TProfileImpl*)Profile;
    oslProfileOption PFlags=0;
    sal_Bool bRet=sal_False;

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

        if ( ( pProfile = (osl_TProfileImpl*) osl_openProfile(0, PFlags ) ) != NULL )
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
            if (! (pProfile->m_Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE )))
            {
                osl_TStamp Stamp;

#ifdef DEBUG_OSL_PROFILE
                OSL_TRACE("Profile acquire DEFAULT MODE\n");
#endif
                if (! (pProfile->m_pFile = openFileImpl(pProfile->m_FileName, pProfile->m_Flags | PFlags )))
                    return NULL;

                Stamp = OslProfile_getFileStamp(pProfile->m_pFile);

                if (memcmp(&Stamp, &(pProfile->m_Stamp), sizeof(osl_TStamp)))
                {
                    pProfile->m_Stamp = Stamp;

                    bRet=loadProfile(pProfile->m_pFile, pProfile);
                    OSL_ASSERT(bRet);
                }
            }
            else
            {
#ifdef DEBUG_OSL_PROFILE
                OSL_TRACE("Profile acquire READ/WRITELOCK MODE\n");
#endif
                /* A readlock file could not be written */
                if ((pProfile->m_Flags & osl_Profile_READLOCK) && bWriteable)
                {
                    return (NULL);
                }
            }
        }
    }

    return (pProfile);
}

static sal_Bool releaseProfile(osl_TProfileImpl* pProfile)
{
    sal_Bool bRet=sal_False;

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
        if (! (pProfile->m_Flags & (osl_Profile_READLOCK | osl_Profile_WRITELOCK | osl_Profile_FLUSHWRITE )))
        {
            if (pProfile->m_Flags & FLG_MODIFIED)
            {
                bRet=storeProfile(pProfile->m_pFile, pProfile, sal_False);
                OSL_ASSERT(bRet);
            }


            closeFileImpl(pProfile->m_pFile,pProfile->m_Flags);
            pProfile->m_pFile = NULL;
        }
    }

#ifdef TRACE_OSL_PROFILE
    OSL_TRACE("Out releaseProfile [ok]\n");
#endif
    return (sal_True);
}

static sal_Bool lookupProfile(const sal_Char *pszPath, const sal_Char *pszFile, sal_Char *pPath)
{
    sal_Char *pChr, *pStr;
    sal_Char Path[PATH_MAX];
    sal_Char Product[132];
    sal_Char Buffer[1024];

    Path[0] = '\0';
    Product[0] = '\0';
    Buffer[0] = '\0';

    if (*pszPath == '"')
    {
        int i = 0;

        pszPath++;

        while ((*pszPath != '"') && (*pszPath != '\0'))
            Product[i++] = *pszPath++;

        Product[i] = '\0';

        if (*pszPath == '"')
            pszPath++;

        if ( *pszPath == '/' )
        {
            pszPath++;
        }
    }
    else
    {
        /* if we have no product identification, do a special handling for soffice.ini */
        if (strcasecmp(SVERSION_PROFILE, pszFile) == 0)
        {
            sal_Char   Profile[PATH_MAX];
            sal_Char   Dir[PATH_MAX];
            oslProfile hProfile;

            Profile[0] = '\0';
            Dir[0] = '\0';

            /* open sversionrc in the system directory, and try to locate the entry
               with the highest version for StarOffice */
            if ((osl_psz_getProfileName(SVERSION_FALLBACK, SVERSION_NAME, Profile, sizeof(Profile))) &&
                (hProfile = osl_psz_openProfile(Profile, osl_Profile_READLOCK)))
            {
                  osl_getProfileSectionEntries(hProfile, SVERSION_SECTION,
                                                Buffer, sizeof(Buffer));

                for (pChr = Buffer; *pChr != '\0'; pChr += strlen(pChr) + 1)
                {
                    if ((strncasecmp(pChr, SVERSION_SOFFICE, sizeof(SVERSION_SOFFICE) - 1) == 0) &&
                        (strcasecmp(Product, pChr) < 0))
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

            /* open sversionrc in the users directory, and try to locate the entry
               with the highest version for StarOffice */
            if ((strcmp(SVERSION_LOCATION, SVERSION_FALLBACK) != 0) &&
                (osl_psz_getProfileName(SVERSION_LOCATION, SVERSION_NAME, Profile, sizeof(Profile))) &&
                (hProfile = osl_psz_openProfile(Profile, osl_Profile_READLOCK)))
            {
                  osl_getProfileSectionEntries(hProfile, SVERSION_SECTION,
                                                Buffer, sizeof(Buffer));

                for (pChr = Buffer; *pChr != '\0'; pChr += strlen(pChr) + 1)
                {
                    if ((strncasecmp(pChr, SVERSION_SOFFICE, sizeof(SVERSION_SOFFICE) - 1) == 0) &&
                        (strcasecmp(Product, pChr) < 0))
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
                (strncasecmp(pChr + 1, SVERSION_OPTION, sizeof(SVERSION_OPTION) - 1) == 0))
            {
                if (((pStart = strchr(pChr + sizeof(SVERSION_OPTION), '[')) != NULL) &&
                    ((pEnd = strchr(pStart + 1, ']')) != NULL))
                {
                    strncpy(Path, pStart + 1, pEnd - (pStart + 1));
                    Path[pEnd - (pStart + 1)] = '\0';

                    /* build full path */
                    if (Path[strlen(Path) - 1] != '/')
                    {
                        strcat(Path, "/");
                    }

                    pChr = &Path[strlen(Path)];
                    if ( strlen(pszPath) <= 0 )
                    {
                        strcat(Path,SVERSION_USER);

                        if ( access ( Path, 0 ) < 0 )
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
        if (osl_psz_getExecutableFile(Path, sizeof(Path)) != osl_Process_E_None)
            return (sal_False);

        /* seperate path from filename */
        if ((pChr = strrchr(Path, '/')) == NULL)
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
        pStr = strrchr(Path, '/');

        strcat(Path, "/");

        if (strlen(pszPath) <= 0 )
        {
            strcat(Path,SVERSION_USER);
        }
        else
        {
            strcat(Path, pszPath);
        }

        /* if the directory given as a parameter does not exist, remove any specified subdirectories
           like "bin" or "program" */
        if (((access(Path, 0) < 0) && (pStr != NULL)) || (strlen(pszPath) <= 0))
        {
            static sal_Char *SubDirs[] = SVERSION_DIRS;

            unsigned int i = 0;

            for (i = 0; i < (sizeof(SubDirs) / sizeof(SubDirs[0])); i++)
                if (strncasecmp(pStr + 1, SubDirs[i], strlen(SubDirs[i])) == 0)
                {
                    if ( strlen(pszPath) <= 0 )
                    {
                        strcpy(pStr + 1, SVERSION_USER);
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
        if (Path[strlen(Path) - 1] != '/')
            strcat(Path, "/");
        strcat(Path, pszFile);

        if ((access(Path, 0) < 0) && (strlen(Product) > 0))
        {
            sal_Char   Profile[PATH_MAX];
            oslProfile hProfile;

            Profile[0] = '\0';

            /* remove appended filename */
            *pChr = '\0';

            /* open sversion.ini in the system directory, and try to locate the entry
               with the highest version for StarOffice */
            if ((osl_psz_getProfileName(SVERSION_LOCATION, SVERSION_NAME, Profile, sizeof(Profile))) &&
                (hProfile = osl_psz_openProfile(Profile, osl_Profile_READLOCK)))
            {
                osl_readProfileString(hProfile, SVERSION_SECTION, Product, Buffer, sizeof(Buffer), "");
                osl_closeProfile(hProfile);

                /* if not found, try the fallback */
                if ((strlen(Buffer) <= 0) && (strcmp(SVERSION_LOCATION, SVERSION_FALLBACK) != 0))
                {
                    if ((osl_psz_getProfileName(SVERSION_FALLBACK, SVERSION_NAME, Profile, sizeof(Profile))) &&
                        (hProfile = osl_psz_openProfile(Profile, osl_Profile_READLOCK)))
                    {
                        osl_readProfileString(hProfile, SVERSION_SECTION, Product, Buffer, sizeof(Buffer), "");
                    }

                    osl_closeProfile(hProfile);
                }

                if (strlen(Buffer) > 0)
                {
                    strcpy(Path, Buffer);

                    /* build full path */
                    if (Path[strlen(Path) - 1] != '/')
                    {

                        strcat(Path, "/");
                    }
                    pChr=&Path[strlen(Path)];
                    if (strlen(pszPath) > 0 )
                    {
                        strcat(Path, pszPath);
                    }
                    else
                    {
                        strcat(Path,SVERSION_USER);
                        if ( access (Path,0) < 0 )
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



