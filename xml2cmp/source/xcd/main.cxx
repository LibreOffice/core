/*************************************************************************
 *
 *  $RCSfile: main.cxx,v $
 *
 *  $Revision: 1.5 $
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

#include <iostream>
#include <stdio.h>


#include <string.h>
#include "../support/cmdline.hxx"
#include "cr_metho.hxx"
#include "cr_html.hxx"
#include "cr_index.hxx"
#include "xmlelem.hxx"
#include "xmltree.hxx"
#include "parse.hxx"
#include "../support/syshelp.hxx"
#include "../support/heap.hxx"




using namespace std;


int                     Do_IndexCommandLine(
                            const CommandLine &     i_rCommandLine );
int                     Do_SingleFileCommandLine(
                            const CommandLine &     i_rCommandLine );
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

    // Load file and create Function-file
    bool bLoadResult = aParser.LoadFile(i_rCommandLine.XmlSrcFile());
    if (! bLoadResult)
    {
        cerr << "Error: File %s could not be loaded." << i_rCommandLine.XmlSrcFile() << endl;
        return 1;
    }

    if ( strlen(i_rCommandLine.FuncFile()) > 0 )
    {
        Create_AccessMethod( i_rCommandLine.FuncFile(),
                             aParser.PureText() );
    }

    cout << "File "
         << i_rCommandLine.FuncFile()
         << " with component_getDescriptionFunc() is created now."
         << endl;


    // Parse
    aParser.Parse();

    // Produce output
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

    std::cout << "Gather xml-files ..." << endl;
    GatherFileNames( aFiles, i_rCommandLine.XmlSrcDirectory() );

    std::cout << "Create output ..." << endl;
    aIndex.GatherData(aFiles);
    aIndex.WriteOutput( i_rCommandLine.IndexOutputFile() );

    std::cout << "... done." << endl;

    return 0;
};



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
    Simstr  sLibPrefix = i_rData.ModuleName();

    // Gather types:
    List< const MultipleTextElement * > aTypes;
    i_rData.Get_Types(aTypes);

    for ( unsigned t = 0; t < aTypes.size(); ++t )
    {
        unsigned i_max = aTypes[t]->Size();
        for ( unsigned  i = 0; i < i_max; ++i )
        {
            aTypesHeap.InsertValue( aTypes[t]->Data(i), "" );
        }  // end for
    }

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
        {
            const char * sQuali = strrchr( pHeapTop->Key(), ' ' )+1;
            WriteStr( aOut, sQuali );
            WriteStr( aOut, "." );
            aOut.write( pHeapTop->Key(), sEnd - pHeapTop->Key() );
        }
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

