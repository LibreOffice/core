/*************************************************************************
 *
 *  $RCSfile: main.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2001-03-09 15:23:00 $
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

#include <stdio.h>

#include <string.h>
#include "../support/cmdline.hxx"
#include "cr_metho.hxx"
#include "cr_html.hxx"
#include "cr_index.hxx"
#include "xmltree.hxx"
#include "parse.hxx"
#include "../support/syshelp.hxx"
#include "../support/heap.hxx"


#ifdef WNT
#include <io.h>
#elif defined(UNX)
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#else
#error Must run under unix or windows, please define UNX or WNT.
#endif




int                     Do_IndexCommandLine(
                            const CommandLine &     i_rCommandLine );
int                     Do_SingleFileCommandLine(
                            const CommandLine &     i_rCommandLine );
void                    GatherFileNames(
                            List<Simstr> &          o_sFiles,
                            const char *            i_sSrcDirectory );
void                    GatherSubDirectories(
                            List<Simstr> &          o_sSubDirectories,
                            const char *            i_sParentdDirectory );
void                    Create_TypeInfoFile(
                            const char *            o_sOutputFile,
                            ModuleDescription &     i_rData );


int
#ifdef WNT
_cdecl
#endif
main( int       argc,
      char *    argv[] )
{
    // Variables
    CommandLine             aCommandLine(argc, argv);
    int ret = 0;

    if (! aCommandLine.IsOk())
    {
        cerr <<  aCommandLine.ErrorText() << endl;
        return 1;
    }

    if ( aCommandLine.IsIndexCommand() )
        ret = Do_IndexCommandLine(aCommandLine);
    else
        ret = Do_SingleFileCommandLine(aCommandLine);

    return ret;
}


int
Do_SingleFileCommandLine(const CommandLine & i_rCommandLine)
{
    ModuleDescription   aDescr;
    X2CParser           aParser(aDescr);

    // Parse
    bool bParseResult = aParser.Parse(i_rCommandLine.XmlSrcFile());
    if (! bParseResult)
    {
        cerr << "Error: File %s could not be parsed." << i_rCommandLine.XmlSrcFile() << endl;
        return 1;
    }

    // Produce output
    if ( strlen(i_rCommandLine.FuncFile()) > 0 )
    {
        Create_AccessMethod( i_rCommandLine.FuncFile(),
                             aParser.PureText() );
    }
    if ( strlen(i_rCommandLine.HtmlFile()) > 0 )
    {
        HtmlCreator aHtmlCreator( i_rCommandLine.HtmlFile(),
                                  aDescr,
                                  i_rCommandLine.IdlRootPath() );
        aHtmlCreator.Run();
    }

    if (strlen(i_rCommandLine.TypeInfoFile()) > 0)
    {
        Create_TypeInfoFile( i_rCommandLine.TypeInfoFile() ,
                             aDescr );
    }

    return 0;
};

int
Do_IndexCommandLine(const CommandLine & i_rCommandLine)
{
    // Parsen files:
    List<Simstr>    aFiles;
    Index           aIndex( i_rCommandLine.OutputDirectory(),
                            i_rCommandLine.IdlRootPath(),
                            i_rCommandLine.IndexedTags() );

    cout << "Gather xml-files ..." << endl;
    GatherFileNames( aFiles, i_rCommandLine.XmlSrcDirectory() );

    cout << "Create output ..." << endl;
    aIndex.GatherData(aFiles);
    aIndex.WriteOutput( i_rCommandLine.IndexOutputFile() );

    cout << "... done." << endl;

    return 0;
};


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
        fprintf(stdout,".");

    strcpy(sFilter, i_sSrcDirectory);
    strcat(sFilter,C_sXML_END);

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
#elif defined(UNX)
    struct DIR * pDir = opendir( i_sSrcDirectory );
    struct dirent * pEntry = 0;
    char * sEnding;

    while ( pEntry = readdir(pDir) != 0 )
    {
        sEnding = strrchr(pEntry->d_name,'.');
        if (sEnding != 0 ? stricmp(sEnding,".xml") == 0 : False )
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

#elif defined(UNX)
    struct DIR * pDir = opendir( i_sParentdDirectory );
    struct dirent * pEntry = 0;
    struct stat     aEntryStatus;
    char * sEnding;

    while ( pEntry = readdir(pDir) != 0 )
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
                o_sSubDirectories.push_back(pNew);
            }
        }   // endif (aEntry.attrib == _A_SUBDIR)
    }   // end while
    closedir( pDir );
#else
#error Must run on unix or windows, please define UNX or WNT.
#endif
}



void
Create_TypeInfoFile( const char *           o_sOutputFile,
                     ModuleDescription &    i_rData )
{
    ofstream aOut(o_sOutputFile, ios::out
#ifdef WNT
                                               | ios::binary
#endif
    );
    if ( !aOut )
    {
        cerr << "Error: " << o_sOutputFile << " could not be created." << endl;
        return;
    }

    Heap    aTypesHeap(12);
    Simstr  sLibPrefix = i_rData.Children()[0]->Data();

    // Gather types:
    const ModuleDescription::CD_List & rListDescriptions = i_rData.Components();

    // Descriptions:
    for ( unsigned nDescrIter = 0; nDescrIter < rListDescriptions.size(); ++nDescrIter )
    {
        // Find Type-Tag:
        TextElement * pTypes = 0;
        ComponentDescription::ChildList & rTags = rListDescriptions[nDescrIter]->Children();
        for ( unsigned nTagIter = 0; nTagIter < rTags.size(); ++nTagIter )
        {
            if ( strcmp( rTags[nTagIter]->Name(),"type") == 0 )
            {
                pTypes = rTags[nTagIter];
                break;
            }
        }   // end for (aTagIter)

        if ( pTypes != 0)
        {
            for ( unsigned nTypeIter = 0; nTypeIter < pTypes->Size(); ++nTypeIter )
            {   // Loop Services
                aTypesHeap.InsertValue( pTypes->Data(nTypeIter), "" );
            }   // end for aDataIter
        }
    }   // end for (aDescrIter)

    // Write types:
    WriteStr( aOut, sLibPrefix );
    WriteStr( aOut, "_XML2CMPTYPES= ");

    HeapItem * pLastHeapTop = 0;
    for ( HeapItem * pHeapTop = aTypesHeap.ReleaseTop(); pHeapTop != 0; pHeapTop = aTypesHeap.ReleaseTop() )
    {
        if (pLastHeapTop != 0)
        {
            if ( 0 == strcmp(pHeapTop->Key(), pLastHeapTop->Key()) )
                continue;
            delete pLastHeapTop;
            // pLastHeapTop = 0;
        }
        pLastHeapTop = pHeapTop;

        WriteStr( aOut, "\t\\\n\t\t" );

        const char * sEnd = strchr( pHeapTop->Key(), ' ' );
        if (sEnd != 0)
            aOut.write( pHeapTop->Key(), sEnd - pHeapTop->Key() );
        else
            WriteStr( aOut, pHeapTop->Key() );
    }   // end for

    if (pLastHeapTop != 0)
    {
        delete pLastHeapTop;
        pLastHeapTop = 0;
    }

    aOut.close();
}

