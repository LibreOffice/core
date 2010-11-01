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

#include <fstream>
#include "cr_html.hxx"
#include "xmltree.hxx"
#include "../support/syshelp.hxx"




char C_sHtmlFileHeader1[] =
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n"
    "<HTML>\n"
    "<HEAD>\n"
    "   <TITLE>";

char C_sHtmlFileHeader2[] =
        "</TITLE>\n"
    "       <META NAME=\"GENERATOR\" CONTENT=\"xml2cmp\">\n"
    "</HEAD>\n"
    "<BODY BGCOLOR=\"#ffffff\">\n<P><BR></P>";


char C_sHtmlFileFoot[] = "</BODY>\n</HTML>\n";


HtmlCreator::HtmlCreator( const char *              i_pOutputFileName,
                          const XmlElement &        i_rDocument,
                          const Simstr &            i_sIDL_BaseDirectory )
    :   aFile(i_pOutputFileName, std::ios::out
#if defined(WNT) || defined(OS2)
                                               | std::ios::binary
#endif
                                                             ),
        rDocument(i_rDocument),
        sIdl_BaseDirectory(i_sIDL_BaseDirectory)
{
    if ( !aFile )
    {
        std::cerr << "Error: " << i_pOutputFileName << " could not be created." << std::endl;
        exit(0);
    }
}

HtmlCreator::~HtmlCreator()
{
    aFile.close();
}

void
HtmlCreator::Run()
{
    WriteStr( C_sHtmlFileHeader1 );
    WriteStr( "ModuleDescription" );
    WriteStr( C_sHtmlFileHeader2 );

    rDocument.Write2Html(*this);

    WriteStr( "<P><BR><BR></P>\n" );
    WriteStr( C_sHtmlFileFoot );
}

void
HtmlCreator::StartTable()
{
    WriteStr( "<P><BR></P>\n" );
    WriteStr(
            "<TABLE WIDTH=95% BORDER=1 CELLSPACING=0 CELLPADDING=4>\n"
            "   <TBODY>\n"  );
}

void
HtmlCreator::FinishTable()
{
    WriteStr( " </TBODY>\n"
              "</TABLE>\n\n" );
}

void
HtmlCreator::StartBigCell( const char * i_sTitle )
{
    WriteStr( "<TR><TD COLSPAN=2>\n"
              "<H4><BR>" );
    WriteStr( i_sTitle );
    WriteStr( "</H4>\n" );

}

void
HtmlCreator::FinishBigCell()
{
    WriteStr( "</TD><TR>\n" );
}

void
HtmlCreator::Write_SglTextElement( const SglTextElement &  i_rElement,
                                   bool                    i_bStrong )
{
    StartRow();

    WriteElementName( i_rElement.Name(), i_bStrong );

    StartCell( "77%");
    if (i_bStrong)
    {
        WriteStr( "<H4><A NAME=\"" );
        unsigned nLen = strlen(i_rElement.Data());
        if ( i_rElement.IsReversedName())
        {
            const char * pEnd = strchr(i_rElement.Data(), ' ');
            nLen = (unsigned)( pEnd - i_rElement.Data() );
        }
        aFile.write( i_rElement.Data(), (int) nLen );
        WriteStr( "\">" );
    }

    WriteName( aFile, sIdl_BaseDirectory, i_rElement.Data(),
               i_bStrong ? lt_nolink : i_rElement.LinkType() );

    if (i_bStrong)
        WriteStr( "</A></H4>" );
    FinishCell();

    FinishRow();
}

void
HtmlCreator::Write_MultiTextElement( const MultipleTextElement &    i_rElement )
{
    StartRow();

    WriteElementName( i_rElement.Name(), false );

    StartCell( "77%");
    unsigned i_max = i_rElement.Size();
    for ( unsigned i = 0; i < i_max; ++i )
    {
        if (i > 0)
            WriteStr( "<BR>\n" );
        WriteName( aFile, sIdl_BaseDirectory, i_rElement.Data(i), i_rElement.LinkType() );
    }   // end for
    FinishCell();

    FinishRow();
}

void
HtmlCreator::Write_SglText( const Simstr &      i_sName,
                            const Simstr &      i_sValue )
{
    StartRow();

    WriteElementName( i_sName, false );

    StartCell( "77%");
    WriteStr( i_sValue );
    FinishCell();

    FinishRow();
}

void
HtmlCreator::Write_ReferenceDocu( const Simstr &      i_sName,
                                  const Simstr &      i_sRef,
                                  const Simstr &      i_sRole,
                                  const Simstr &      i_sTitle )
{
    StartRow();

    StartCell( "23%" );
    WriteStr(i_sName);
    FinishCell();

    StartCell( "77%" );
    if ( !i_sRef.is_empty() )
    {
        WriteStr("<A href=\"");
        WriteStr(i_sRef);
        WriteStr("\">");
        if ( !i_sTitle.is_empty() )
            WriteStr( i_sTitle );
        else
            WriteStr(i_sRef);
        WriteStr("</A><BR>\n");
    }
    else if ( !i_sTitle.is_empty() )
    {
        WriteStr("Title: ");
        WriteStr( i_sTitle );
        WriteStr("<BR>\n");
    }
    if ( !i_sRole.is_empty() )
    {
        WriteStr("Role: ");
        WriteStr( i_sRole );
    }
    FinishCell();

    FinishRow();
}


void
HtmlCreator::StartRow()
{
    WriteStr( "     <TR VALIGN=TOP>\n" );
}

void
HtmlCreator::FinishRow()
{
    WriteStr( "     </TR>\n" );
}

void
HtmlCreator::StartCell( const char * i_pWidth)
{
    WriteStr( "         <TD WIDTH=" );
    WriteStr( i_pWidth );
    WriteStr( ">\n              <P>" );
}

void
HtmlCreator::FinishCell()
{
    WriteStr( "</P>\n           </TD>\n" );
}

void
HtmlCreator::WriteElementName( const Simstr & i_sName,
                               bool           i_bStrong )
{
    StartCell( "23%" );
    if (i_bStrong)
        WriteStr( "<H4>" );
    WriteStr(i_sName);
    if (i_bStrong)
        WriteStr( "</H4>" );
    FinishCell();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
