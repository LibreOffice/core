/*************************************************************************
 *
 *  $RCSfile: cr_index.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2001-03-12 19:24:52 $
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


#include "cr_index.hxx"

#include <string.h>
#include <fstream>
#include "../support/syshelp.hxx"
#include "xmltree.hxx"
#include "parse.hxx"
#include "cr_html.hxx"

using std::cerr;
using std::ofstream;


char C_sLineEnd[] = "\n";

char C_sFileBegin[]     = "<HTML><HEAD></HEAD><BODY bgcolor=\"#ffffff\">\n";
char C_sFileEnd[]       = "</BODY></HTML>\n";
char C_sTableBegin[]    = "<TABLE WIDTH=100% BORDER=1 CELLPADDING=4 CELLSPACING=0><TBODY>\n";
char C_sTableEnd[]      = "</TBODY></TABLE>\n";
char C_sService[]       = "SupportedService";
char C_sModule[]        = "ModuleName";
char C_sComponentname[] = "ComponentName";


int NrOfTagName(const char * i_sName);


Simstr sIdlRootPath;

void                WriteTableFromHeap(
                        ofstream &          o_rOut,
                        Heap &              i_rHeap,
                        const char *        i_sIndexKey,
                        const char *        i_sIndexReference,
                        E_LinkType          i_eLinkType );
void                WriteHeap(
                        ofstream &          o_rOut,
                        Heap &              i_rHeap,
                        E_LinkType          i_eLinkType );


Index::Index( const char *          i_sOutputDirectory,
              const char *          i_sIdlRootPath,
              const List<Simstr> &  i_rTagList )
    :   aService2Module(20),
        aModule2Service(20),
        aTagIndices(i_rTagList),
        sOutputDirectory(i_sOutputDirectory),
        sIdlRootPath(i_sIdlRootPath)
{
    ::sIdlRootPath = i_sIdlRootPath;
}

Index::IndexedTags::IndexedTags( const List<Simstr> & i_rTagList )
    :   nSize(i_rTagList.size())
{
    aTagHeaps.reserve(nSize);
    aTagNames.reserve(nSize);

    for (int h = 0; h < C_nNrOfTagNames; ++h )
    {
        aHeapsPerIndex[h] = 0;
    }

    for ( unsigned i = 0; i < nSize; ++i )
    {
        int nNr = NrOfTagName(i_rTagList[i]);
        if ( nNr > -1 )
        {
            Heap * pNew = new Heap(20);
            aTagHeaps.push_back( pNew );
            aTagNames.push_back( i_rTagList[i] );
            aHeapsPerIndex[nNr] = pNew;
        }
        else
        {
            cerr << "Warning: It is not possible to create an index for the tag "
                 << i_rTagList[i]
                 << "."
                 << std::endl;
        }
    }   // end for
}

Index::~Index()
{
}

Index::IndexedTags::~IndexedTags()
{
}

Heap *
Index::IndexedTags::HeapFor( const char * i_sTagName )
{
    int nNr = NrOfTagName(i_sTagName);
    if (nNr > -1 )
        return aHeapsPerIndex[nNr];
    return 0;
}

void
Index::GatherData(  const List<Simstr> & i_rInputFileList )
{
    for ( unsigned i = 0; i < i_rInputFileList.size(); ++i )
    {
        ReadFile( i_rInputFileList[i].str() );
    }
}

void
Index::WriteOutput( const char * i_sOuputFile )
{
    ofstream aOut( i_sOuputFile, ios::out );
    if (! aOut)
    {
        cerr << "Error: Indexfile \""
             << i_sOuputFile
             << "\" could not be created."
             << std::endl;
        return;
    }

    WriteStr(aOut, C_sFileBegin);

    WriteStr(aOut, "<H2>Module Descriptions Index</H2>");
    WriteStr(aOut, C_sLineEnd );


    WriteTableFromHeap( aOut, aService2Module, C_sService, C_sModule, lt_html );
    WriteTableFromHeap( aOut, aModule2Service, C_sModule, C_sService, lt_idl );

    for ( unsigned i = 0; i < aTagIndices.nSize; ++i )
    {
        WriteTableFromHeap( aOut, *aTagIndices.aTagHeaps[i],
                            aTagIndices.aTagNames[i], C_sComponentname, lt_html );
    }

    WriteStr( aOut, C_sFileEnd );
    aOut.close();
}

void
Index::ReadFile(  const char * i_sFilename )
{
    static char             sOutputHtml[1020];

    ModuleDescription   aModule;
    X2CParser           aParser(aModule);

    if ( ! aParser.Parse(i_sFilename) )
    {
        cerr << "Error: File \""
             << i_sFilename
             << "\" could not be parsed."
             << std::endl;
        return;
    }

    // Create Html:
    CreateHtmlFileName( sOutputHtml, aModule );
    HtmlCreator     aHtmlCreator( sOutputHtml, aModule, sIdlRootPath );
    aHtmlCreator.Run();

    // GetResults:
    Simstr sModuleName = aModule.Name();

        // Module Tags
    const ModuleDescription::ChildList &
            rModuleElements = aModule.Children();
    for ( unsigned mi = 1; mi < rModuleElements.size(); ++mi )
    {
        Heap * pHeap = aTagIndices.HeapFor(rModuleElements[mi]->Name());
        if (pHeap != 0)
        {
            unsigned nDatSize = rModuleElements[mi]->Size();
            for ( unsigned d = 0; d < nDatSize; ++d )
            {
                pHeap->InsertValue( rModuleElements[mi]->Data(d), sModuleName.str() );
            }
        }
    }   // end for

        // Component Tags
    const ModuleDescription::CD_List &
                    rCDs = aModule.Components();
    static char     sCD_inModule[1020];
    strcpy( sCD_inModule, sModuleName.str() );
    strcat( sCD_inModule, "," );
    char * pCut = strchr(sCD_inModule,',') + 1;

    for ( unsigned cd = 0; cd < rCDs.size(); ++cd )
    {
        const ComponentDescription::ChildList &
                rCD_Elements = rCDs[cd]->Children();
        strcpy( pCut, rCDs[cd]->Name() );
        for ( unsigned ci = 1; ci < rCD_Elements.size(); ++ci )
        {
            if ( ci == 5 )
            {
                TextElement * pElem = rCD_Elements[ci];
                unsigned nDatSize = pElem->Size();
                for ( unsigned d = 0; d < nDatSize; ++d )
                {
                    aService2Module.InsertValue(
                            pElem->Data(d),
                            sModuleName );
                    aModule2Service.InsertValue(
                            sModuleName,
                            pElem->Data(d) );
                }
            }
            else
            {
                Heap * pHeap = aTagIndices.HeapFor(rCD_Elements[ci]->Name());
                if (pHeap != 0)
                {
                    unsigned nDatSize = rCD_Elements[ci]->Size();
                    for ( unsigned d = 0; d < nDatSize; ++d )
                    {
                        pHeap->InsertValue( rCD_Elements[ci]->Data(d), sCD_inModule );
                    }
                }
            }   // end if (ci == 6) else
        }   // end for
    }   // for
}

void
Index::CreateHtmlFileName(  char *                      o_sOutputHtml,
                            const ModuleDescription &   i_rModule )
{
    strcpy( o_sOutputHtml, sOutputDirectory.str() );
#ifdef WNT
    strcat(o_sOutputHtml, "\\");
#elif defined(UNX)
    strcat(o_sOutputHtml, "/");
#else
#error  WNT or UNX have to be defined.
#endif
    strcat( o_sOutputHtml, i_rModule.Name() );
    strcat( o_sOutputHtml, ".html" );
}


void
WriteTableFromHeap( ofstream &      o_rOut,
                    Heap &          i_rHeap,
                    const char *    i_sIndexValue,
                    const char *    i_sIndexReference,
                    E_LinkType      i_eLinkType )
{
    WriteStr(o_rOut, "<H3><BR>");
    WriteStr(o_rOut, i_sIndexValue );
    WriteStr(o_rOut, " -> ");
    WriteStr(o_rOut, i_sIndexReference );
    WriteStr(o_rOut, "</H3>\n");

    WriteStr(o_rOut, C_sTableBegin);
    WriteHeap( o_rOut, i_rHeap, i_eLinkType );
    WriteStr(o_rOut, C_sTableEnd);
}


void
WriteHeap( ofstream &   o_rOut,
           Heap &       i_rHeap,
           E_LinkType   i_eLinkType )
{
    static Simstr S_sKey;
    static char C_sSpaceInName[] = "&nbsp;&nbsp;&nbsp;";
    S_sKey = "";


    WriteStr( o_rOut, "<TR><TD width=33% valign=\"top\">" );

    for ( HeapItem * pHeapTop = i_rHeap.ReleaseTop();
          pHeapTop != 0;
          pHeapTop = i_rHeap.ReleaseTop() )
    {
        if ( S_sKey != pHeapTop->Key() )
        {
            const char * pStart = pHeapTop->Key().str();
            const char * pBreak = strstr( pStart, " in ");

            if (S_sKey.l()>0)
            {
                WriteStr( o_rOut, "</TD></TR>\n" );
                WriteStr( o_rOut, "<TR><TD width=33% valign=\"top\">" );
            }

            if ( pBreak == 0 )
                WriteStr( o_rOut, pStart );
            else
            {
                o_rOut.write( pStart, pBreak - pStart );
                WriteStr( o_rOut, C_sSpaceInName );
                WriteStr( o_rOut, pBreak );
            }
            WriteStr( o_rOut, "</TD><TD width=66%>" );
            S_sKey = pHeapTop->Key();
        }
        else
        {
            WriteStr( o_rOut, "<BR>" );
        }
        WriteName( o_rOut, sIdlRootPath, pHeapTop->Value(), i_eLinkType );
        delete pHeapTop;
    }

    WriteStr( o_rOut, "</TD></TR>\n" );
}

int
NrOfTagName( const char * i_sName )
{
    static const char * aTagNames[C_nNrOfTagNames] =
                            {
//                              "module-name",                  // 1
                                    "author",                   // 1        0
//                                  "name",                     // 1
//                                  "description",              // 1
                                    "loader-name",              // 1        1
                                    "language",                 // 1        2
//                                  "status",
                                    "supported-service",        // +        3
//                                  "reference-docu",
                                    "service-dependency",       // *        4
                                    "type",                     // *        5
                                "project-build-dependency",     // *        6
                                "runtime-module-dependency",    // *        7
                            };
    if (! i_sName)
        return -1;

    switch (*i_sName)
    {
        case 'a':   if ( strcmp( aTagNames[0], i_sName ) == 0 )
                        return 0;
                    break;
        case 'l':   if ( strcmp( aTagNames[1], i_sName ) == 0 )
                        return 1;
                    else if ( strcmp( aTagNames[2], i_sName ) == 0 )
                        return 1;
                    break;
        case 's':   if ( strcmp( aTagNames[3], i_sName ) == 0 )
                        return 3;
                    else if ( strcmp( aTagNames[4], i_sName ) == 0 )
                        return 4;
                    break;
        case 't':   if ( strcmp( aTagNames[5], i_sName ) == 0 )
                        return 5;
                    break;
        case 'p':   if ( strcmp( aTagNames[6], i_sName ) == 0 )
                        return 6;
                    break;
        case 'r':   if ( strcmp( aTagNames[7], i_sName ) == 0 )
                        return 7;
                    break;
    }
    return -1;
}





/** Übersicht der Struktur

MODULEDESCRIPTION
{
    ModuleName,
    COMPONENTDESCRIPTION
    {
        Author,
        Name,
        Description,
        LoaderName,
        Language,
        Status,
        SupportedService+,
        ReferenceDocu*
        ServiceDependency*
        Type*
    }
    ProjectBuildDependency*
    RuntimeModuleDependency*
}


*/






