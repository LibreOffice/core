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

#include <iostream>
#include <fstream>
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



int                     Do_IndexCommandLine(
                            const CommandLine &     i_rCommandLine );
int                     Do_SingleFileCommandLine(
                            const CommandLine &     i_rCommandLine );
void                    Create_TypeInfo(
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
        std::cerr <<  aCommandLine.ErrorText() << std::endl ;
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
        std::cerr << "Error: File " << i_rCommandLine.XmlSrcFile() << " could not be loaded." << std::endl;
        return 1;
    }

    if ( strlen(i_rCommandLine.FuncFile()) > 0 )
    {
        Create_AccessMethod( i_rCommandLine.FuncFile(),
                             aParser.PureText() );

        std::cout << "File "
             << i_rCommandLine.FuncFile()
             << " with component_getDescriptionFunc() is created now."
             << std::endl;
    }

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
        Create_TypeInfo( i_rCommandLine.TypeInfoFile(),
                         aDescr );
    }

    return 0;
};

int
Do_IndexCommandLine(const CommandLine & i_rCommandLine)
{
    // Parse files:
    List<Simstr>    aFiles;
    Index           aIndex( i_rCommandLine.OutputDirectory(),
                            i_rCommandLine.IdlRootPath(),
                            i_rCommandLine.IndexedTags() );

    std::cout << "Gathering xml-files ..." << std::endl;
    GatherFileNames( aFiles, i_rCommandLine.XmlSrcDirectory() );

    std::cout << "Creating output ..." << std::endl;
    aIndex.GatherData(aFiles);
    aIndex.WriteOutput( i_rCommandLine.IndexOutputFile() );

    std::cout << "... done." << std::endl;

    return 0;
};



//********************      Creating of typeinfo       ********************//


void                    Put2StdOut_TypeInfo(
                            ModuleDescription &     i_rData );
void                    Put2File_TypeInfo(
                            const char *            i_sOutputFile,
                            ModuleDescription &     i_rData );
void                    StreamOut_TypeInfo(
                            std::ostream &               o_rOut,
                            ModuleDescription &     i_rData,
                            const char *            i_sSeparator );




void
Create_TypeInfo( const char *           o_sOutputFile,
                 ModuleDescription &    i_rData )
{
    if ( strcmp(o_sOutputFile, "stdout") == 0 )
        Put2StdOut_TypeInfo(i_rData);
    else
        Put2File_TypeInfo(o_sOutputFile,i_rData);
}

void
Put2StdOut_TypeInfo( ModuleDescription &    i_rData )
{
    StreamOut_TypeInfo(std::cout, i_rData, " ");
}

void
Put2File_TypeInfo( const char *            i_sOutputFile,
                   ModuleDescription &     i_rData )
{
    std::ofstream aOut(i_sOutputFile, std::ios::out
#if defined(WNT) || defined(OS2)
                                               | std::ios::binary
#endif
    );
    if ( !aOut )
    {
        std::cerr << "Error: " << i_sOutputFile << " could not be created." << std::endl;
        return;
    }

    Simstr  sLibPrefix = i_rData.ModuleName();
    WriteStr( aOut, sLibPrefix );
    WriteStr( aOut, "_XML2CMPTYPES= ");

    StreamOut_TypeInfo(aOut, i_rData, "\t\\\n\t\t");

    aOut.close();
}

void
StreamOut_TypeInfo( std::ostream &               o_rOut,
                    ModuleDescription &     i_rData,
                    const char *            i_sSeparator )
{
    Heap    aTypesHeap(12);

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
    HeapItem * pLastHeapTop = 0;
    for ( HeapItem * pHeapTop = aTypesHeap.ReleaseTop(); pHeapTop != 0; pHeapTop = aTypesHeap.ReleaseTop() )
    {
        if (pLastHeapTop != 0)
        {
            if ( 0 == strcmp(pHeapTop->Key(), pLastHeapTop->Key()) )
                continue;
            delete pLastHeapTop;
        }
        pLastHeapTop = pHeapTop;

        WriteStr( o_rOut, i_sSeparator );

        const char * sEnd = strchr( pHeapTop->Key(), ' ' );
        if (sEnd != 0)
        {
            const char * sQuali = strrchr( pHeapTop->Key(), ' ' ) + 1;
            WriteStr( o_rOut, sQuali );
            WriteStr( o_rOut, "." );
            o_rOut.write( pHeapTop->Key(), sEnd - pHeapTop->Key() );
        }
        else
            WriteStr( o_rOut, pHeapTop->Key() );
    }   // end for

    if (pLastHeapTop != 0)
    {
        delete pLastHeapTop;
        pLastHeapTop = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
