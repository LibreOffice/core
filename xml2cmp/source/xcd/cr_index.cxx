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


#include "cr_index.hxx"

#include <string.h>
#include <fstream>
#include "../support/syshelp.hxx"
#include "xmltree.hxx"
#include "parse.hxx"
#include "cr_html.hxx"


extern unsigned C_nSupportedServicesIndex;

char C_sLineEnd[] = "\n";

char C_sFileBegin[]     = "<HTML><HEAD></HEAD><BODY bgcolor=\"#ffffff\">\n";
char C_sFileEnd[]       = "</BODY></HTML>\n";
char C_sTableBegin[]    = "<TABLE WIDTH=100% BORDER=1 CELLPADDING=4 CELLSPACING=0><TBODY>\n";
char C_sTableEnd[]      = "</TBODY></TABLE>\n";
char C_sService[]       = "SupportedService";
char C_sModule[]        = "ModuleName";
char C_sComponentname[] = "ComponentName";



Simstr sIdlRootPath;


Index::Index( const char *          i_sOutputDirectory,
              const char *          i_sIdlRootPath,
              const List<Simstr> &   )
    :   aService2Module(20),
        aModule2Service(20),
        sOutputDirectory(i_sOutputDirectory),
        sIdlRootPath(i_sIdlRootPath)
        // sCurModule
{
    ::sIdlRootPath = i_sIdlRootPath;
}

Index::~Index()
{
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
    std::ofstream aOut( i_sOuputFile, std::ios::out );
    if (! aOut)
    {
        std::cerr << "Error: Indexfile \""
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

    WriteStr( aOut, C_sFileEnd );
    aOut.close();
}

void
Index::InsertSupportedService( const Simstr &       i_sService )
{
    aService2Module.InsertValue( i_sService, sCurModule );
    aModule2Service.InsertValue( sCurModule, i_sService );
}

void
Index::ReadFile(  const char * i_sFilename )
{
    static char             sOutputHtml[1020];

    ModuleDescription   aModule;
    X2CParser           aParser(aModule);

    // Parse
    bool bResult = aParser.Parse(i_sFilename);
    if (! bResult)
    {
        std::cerr << "Error: File \""
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
    sCurModule = aModule.ModuleName();

    List< const MultipleTextElement* > aSupportedServices;
    aModule.Get_SupportedServices(aSupportedServices);

    for ( unsigned s = 0; s < aSupportedServices.size(); ++s )
    {
        aSupportedServices[s]->Insert2Index(*this);
    }
}

void
Index::CreateHtmlFileName(  char *                      o_sOutputHtml,
                            const ModuleDescription &   i_rModule )
{
    if ( strlen(sOutputDirectory.str()) + strlen(i_rModule.ModuleName()) > 1000 )
    {
         strcpy( o_sOutputHtml, "too-long-filename.html");   // STRCPY SAFE HERE
        return;
    }

    strcpy( o_sOutputHtml, sOutputDirectory.str() );        // STRCPY SAFE HERE
#if defined(WNT) || defined(OS2)
    strcat(o_sOutputHtml, "\\");                            // STRCAT SAFE HERE
#elif defined(UNX)
    strcat(o_sOutputHtml, "/");                             // STRCAT SAFE HERE
#else
#error  WNT or UNX have to be defined.
#endif
    strcat( o_sOutputHtml, i_rModule.ModuleName() );        // STRCAT SAFE HERE
    strcat( o_sOutputHtml, ".html" );                       // STRCAT SAFE HERE
}


void
Index::WriteTableFromHeap( std::ostream &   o_rOut,
                           Heap &       i_rHeap,
                           const char * i_sIndexValue,
                           const char * i_sIndexReference,
                           E_LinkType       i_eLinkType )
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
Index::WriteHeap( std::ostream &    o_rOut,
                  Heap &        i_rHeap,
                  E_LinkType    i_eLinkType )
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
    ReferenceDocu*
    ServiceDependency*
    Type*
}


*/






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
