/*************************************************************************
 *
 *  $RCSfile: cr_index.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: np $ $Date: 2001-03-23 13:39:36 $
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
              const List<Simstr> &  i_rTagList )
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
    strcpy( o_sOutputHtml, sOutputDirectory.str() );
#ifdef WNT
    strcat(o_sOutputHtml, "\\");
#elif defined(UNX)
    strcat(o_sOutputHtml, "/");
#else
#error  WNT or UNX have to be defined.
#endif
    strcat( o_sOutputHtml, i_rModule.ModuleName() );
    strcat( o_sOutputHtml, ".html" );
}


void
Index::WriteTableFromHeap( ofstream &   o_rOut,
                           Heap &           i_rHeap,
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
Index::WriteHeap( ofstream &    o_rOut,
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






