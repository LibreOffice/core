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


#include <syshelp.hxx>


// NOT FULLY DEFINED SERVICES
#include <string.h>
#include "sistr.hxx"
#include "list.hxx"

#ifdef WNT
#include <io.h>
#elif defined(UNX) || defined(OS2)
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#define stricmp strcasecmp
#else
#error Must run under unix or windows, please define UNX or WNT.
#endif


char C_sSpaceInName[] = "&nbsp;&nbsp;&nbsp;";

void
WriteName( std::ostream &       o_rFile,
           const Simstr &   i_rIdlDocuBaseDir,
           const Simstr &   i_rName,
           E_LinkType       i_eLinkType )
{
    if (i_rName.l() == 0)
        return;


    const char * pNameEnd = strstr( i_rName.str(), " in " );

    // No link:
    if ( i_eLinkType == lt_nolink )
    {
        if ( pNameEnd != 0 )
        {
            const char * pStart = i_rName.str();
            o_rFile.write( pStart, pNameEnd - pStart );
            WriteStr( o_rFile, C_sSpaceInName );
            WriteStr( o_rFile, pNameEnd );
        }
        else
        {
            WriteStr( o_rFile, i_rName );
        }
        return;
    }

    if ( i_eLinkType == lt_idl )
    {
        Simstr sPath(i_rName);
        sPath.replace_all('.','/');
        int nNameEnd = sPath.pos_first(' ');
        int nPathStart = sPath.pos_last(' ');
        WriteStr( o_rFile, "<A HREF=\"" );

        if ( nNameEnd > -1 )
        {
            WriteStr( o_rFile, "file:///" );
            WriteStr( o_rFile, i_rIdlDocuBaseDir );
            WriteStr( o_rFile, "/" );
            WriteStr( o_rFile, sPath.str() + 1 + nPathStart );
            WriteStr( o_rFile, "/" );
            o_rFile.write( sPath.str(), nNameEnd );
            WriteStr( o_rFile, ".html\">" );
        }
        else
        {   // Should not be reached:
            WriteStr(o_rFile, i_rName);
            return;
        }
    }
    else if ( i_eLinkType == lt_html )
    {
        int nKomma = i_rName.pos_first(',');
        int nEnd = i_rName.pos_first(' ');
        if ( nKomma > -1 )
        {
            o_rFile.write( i_rName.str(), nKomma );
            WriteStr( o_rFile, ": " );

            WriteStr( o_rFile, "<A HREF=\"" );

            o_rFile.write( i_rName.str(), nKomma );
            WriteStr( o_rFile, ".html#" );
            if ( nEnd > -1 )
                o_rFile.write( i_rName.str() + nKomma + 1, nEnd - nKomma );
            else
                WriteStr( o_rFile, i_rName.str() + nKomma + 1 );
            WriteStr( o_rFile, "\">" );

            o_rFile.write( i_rName.str() + nKomma + 1, nEnd - nKomma );
        }
        else
        {
            WriteStr( o_rFile, "<A HREF=\"" );
            WriteStr( o_rFile, i_rName );
            WriteStr( o_rFile, ".html\">" );

            WriteStr( o_rFile, i_rName );
        }
        WriteStr( o_rFile, "</A>" );
        return;
    }

    if ( pNameEnd != 0 )
    {
        const char * pStart = i_rName.str();
        if ( pNameEnd > pStart )
            o_rFile.write( pStart, pNameEnd - pStart );
        WriteStr( o_rFile, "</A>" );

        WriteStr( o_rFile, C_sSpaceInName );
        WriteStr( o_rFile, pNameEnd );
    }
    else
    {
        WriteStr( o_rFile, i_rName );
        WriteStr( o_rFile, "</A>" );
    }
}


void
WriteStr( std::ostream &    o_rFile,
          const char *      i_sStr )
{
    o_rFile.write( i_sStr, (int) strlen(i_sStr) );
}

void
WriteStr( std::ostream &      o_rFile,
          const Simstr &      i_sStr )
{
    o_rFile.write( i_sStr.str(), i_sStr.l() );
}


const char C_sXML_END[] = "\\*.xml";

void
GatherFileNames( List<Simstr> &     o_sFiles,
                 const char *       i_sSrcDirectory )
{
    static int   nAliveCounter = 0;

    char *       sNextDir = 0;
    Simstr       sNew = 0;

#ifdef WNT
    struct _finddata_t aEntry;
    long hFile = 0;
    int bFindMore = 0;
    char * sFilter = new char[ strlen(i_sSrcDirectory) + sizeof C_sXML_END ];

    // Stayingalive sign
    if (++nAliveCounter % 100 == 1)
        std::cout << "." << std::flush;

    strcpy(sFilter, i_sSrcDirectory);       // STRCPY SAFE HERE
    strcat(sFilter,C_sXML_END);             // STRCAT SAFE HERE

    hFile = _findfirst( sFilter, &aEntry );
    for ( bFindMore = hFile == -1;
          bFindMore == 0;
          bFindMore = _findnext( hFile, &aEntry ) )
    {
        sNew = i_sSrcDirectory;
        sNew += "\\";
        sNew += aEntry.name;
        o_sFiles.push_back(sNew);
    }   // end for

    _findclose(hFile);
    delete [] sFilter;
#elif defined(UNX) || defined(OS2)
    DIR * pDir = opendir( i_sSrcDirectory );
    dirent * pEntry = 0;
    char * sEnding;

    // Stayingalive sign
    if (++nAliveCounter % 100 == 1)
        std::cout << "." << std::flush;

    while ( (pEntry = readdir(pDir)) != 0 )
    {
        sEnding = strrchr(pEntry->d_name,'.');
        if (sEnding != 0 ? stricmp(sEnding,".xml") == 0 : 0 )
        {
            sNew = i_sSrcDirectory;
            sNew += "/";
            sNew += pEntry->d_name;
            o_sFiles.push_back(sNew);
        }
    }   // end while

    closedir( pDir );
#else
#error Must run on unix or windows, please define UNX or WNT.
#endif

    //  gathering from subdirectories:
    List<Simstr> aSubDirectories;
    GatherSubDirectories( aSubDirectories, i_sSrcDirectory );

    unsigned d_max = aSubDirectories.size();
    for ( unsigned d = 0; d < d_max; ++d )
    {
        sNextDir = new char[ strlen(i_sSrcDirectory) + 2 + aSubDirectories[d].l() ];

        strcpy(sNextDir, i_sSrcDirectory);
        strcat(sNextDir, C_sSLASH);
        strcat(sNextDir, aSubDirectories[d].str());
        GatherFileNames(o_sFiles, sNextDir);

        delete [] sNextDir;
    }
}


const char * C_sANYDIR = "\\*.*";

void
GatherSubDirectories( List<Simstr> &    o_sSubDirectories,
                      const char *      i_sParentdDirectory )
{
    Simstr sNew;

#ifdef WNT
    struct _finddata_t aEntry;
    long hFile = 0;
    int bFindMore = 0;
    char * sFilter = new char[strlen(i_sParentdDirectory) + sizeof C_sANYDIR];

    strcpy(sFilter, i_sParentdDirectory);
    strcat(sFilter,C_sANYDIR);

    hFile = _findfirst( sFilter, &aEntry );
    for ( bFindMore = hFile == -1;
          bFindMore == 0;
          bFindMore = _findnext( hFile, &aEntry ) )
    {
        if (aEntry.attrib == _A_SUBDIR)
        {
            // Do not gather . .. and outputtree directories
            if ( strchr(aEntry.name,'.') == 0
                 && strncmp(aEntry.name, "wnt", 3) != 0
                 && strncmp(aEntry.name, "unx", 3) != 0 )
            {
                sNew = aEntry.name;
                o_sSubDirectories.push_back(sNew);
            }
        }   // endif (aEntry.attrib == _A_SUBDIR)
    }   // end for
    _findclose(hFile);
    delete [] sFilter;

#elif defined(UNX) || defined(OS2)
    DIR * pDir = opendir( i_sParentdDirectory );
    dirent * pEntry = 0;
    struct stat     aEntryStatus;

    while ( ( pEntry = readdir(pDir) ) != 0 )
    {
        stat(pEntry->d_name, &aEntryStatus);
        if ( ( aEntryStatus.st_mode & S_IFDIR ) == S_IFDIR )
        {
            // Do not gather . .. and outputtree directories
            if ( strchr(pEntry->d_name,'.') == 0
                 && strncmp(pEntry->d_name, "wnt", 3) != 0
                 && strncmp(pEntry->d_name, "unx", 3) != 0 )
            {
                sNew = pEntry->d_name;
                o_sSubDirectories.push_back(sNew);
            }
        }   // endif (aEntry.attrib == _A_SUBDIR)
    }   // end while
    closedir( pDir );
#else
#error Must run on unix or windows, please define UNX or WNT.
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
