/*************************************************************************
 *
 *  $RCSfile: cr_html.cxx,v $
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

#include <fstream>
#include "cr_html.hxx"
#include "xmltree.hxx"
#include "../support/syshelp.hxx"

using std::cerr;
using std::ofstream;




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
                          const ModuleDescription & i_rDescr,
                          const Simstr &            i_sIDL_BaseDirectory )
    :   aFile(i_pOutputFileName, ios::out
#ifdef WNT
                                               | ios::binary
#endif
                                                             ),
        rDescr(i_rDescr),
        sIdl_BaseDirectory(i_sIDL_BaseDirectory)
{
    if ( !aFile )
    {
        cerr << "Error: " << i_pOutputFileName << " could not be created." << std::endl;
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

    StartTable();

    const ParentElement::ChildList & rModuleElements = rDescr.Children();
    unsigned i_max = rModuleElements.size();

    for (unsigned i = 0; i < i_max; i++)
    {
        Write_TextElement( *rModuleElements[i], lt_nolink );        // Name,
                                                                    // ProjectBuildDependency,
                                                                    // RuntimeModuleDependency
    }  // end for

    WriteStr( "<TR><TD COLSPAN=2>\n"
              "<H4><BR>Components</H4>\n" );

    const ModuleDescription::CD_List & rCDs = rDescr.Components();
    unsigned d_max = rCDs.size();
    for (unsigned d = 0; d < d_max; d++)
    {
        WriteStr( "<P><BR><BR></P>\n" );

        Write_Description( *rCDs[d] );
    }  // end for

    WriteStr( "</TD><TR>\n" );
    FinishTable();

    WriteStr( "<P><BR><BR></P>\n" );
    WriteStr( C_sHtmlFileFoot );
}


void
HtmlCreator::Write_Description( const ComponentDescription & i_rDescr )
{
    StartTable();

    const ParentElement::ChildList & rDescrElements = i_rDescr.Children();

    unsigned i_max = rDescrElements.size();
    Write_TextElement( *rDescrElements[0], lt_nolink );           // Name
    Write_TextElement( *rDescrElements[1], lt_nolink );           // Author
    Write_TextElement( *rDescrElements[2], lt_nolink );           // Description
    Write_TextElement( *rDescrElements[3], lt_idl );              // Loader Name
    Write_TextElement( *rDescrElements[4], lt_nolink );           // Language

    Write_Status(i_rDescr.Status());                              // Status

    for (unsigned i = 5; i < i_max; i++)
    {
        Write_TextElement( *rDescrElements[i], lt_idl );          // Supported Service,
                                                                  // Service Dependency,
                                                                  // Type

    }  // end for

    const ComponentDescription::Docu_List & rDocuRefs = i_rDescr.DocuRefs();
    unsigned r_max = rDocuRefs.size();
    for (unsigned r = 0; r < r_max; r++)
    {
        Write_ReferenceDocu(*rDocuRefs[r]);
    }  // end for

    FinishTable();
}

void
HtmlCreator::Write_TextElement( TextElement & i_rElement,
                                E_LinkType    i_eLinkType )
{
    StartRow();

    WriteElementName( i_rElement );
    WriteElementData( i_rElement, i_eLinkType );

    FinishRow();
}

void
HtmlCreator::Write_ReferenceDocu( const ReferenceDocuElement & i_rRefDocu )
{
    StartRow();

    StartCell( "23%" );
    WriteStr("ReferenceDocu");
    FinishCell();

    StartCell( "77%" );
    if ( i_rRefDocu.sAttr_href.l() > 0)
    {
        WriteStr("<A href=\"http://");
        WriteStr(i_rRefDocu.sAttr_href.str());
        WriteStr("\">");
        WriteStr(i_rRefDocu.sAttr_href.str());
        WriteStr("</A><BR>\n");
    }
    if ( i_rRefDocu.sAttr_title.l() > 0)
    {
        WriteStr("Title: ");
        WriteStr(i_rRefDocu.sAttr_title.str());
        WriteStr("<BR>\n");
    }
    if ( i_rRefDocu.sAttr_role.l() > 0)
    {
        WriteStr("Role: ");
        WriteStr(i_rRefDocu.sAttr_role.str());
    }
    FinishCell();

    FinishRow();
}

void
HtmlCreator::Write_Status(const char * i_sStatus)
{
    StartRow();

    StartCell( "23%" );
    WriteStr("status");
    FinishCell();

    StartCell( "77%");
    WriteName( aFile, sIdl_BaseDirectory, i_sStatus, lt_nolink );
    FinishCell();

    FinishRow();
}

void
HtmlCreator::PrintH1( char * i_pText)
{
    static char sH1a[] = "<H1 ALIGN=CENTER>";
    static char sH1e[] = "</H1>";
    WriteStr(sH1a);
    WriteStr(i_pText);
    WriteStr(sH1e);
}

void
HtmlCreator::StartTable()
{
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
HtmlCreator::StartCell( char * i_pWidth)
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
HtmlCreator::WriteElementName( TextElement & i_rElement )
{
    StartCell( "23%" );
    const char * pName = i_rElement.Name();
    bool bStrong = strcmp(pName,"name") == 0 || strcmp(pName,"module-name") == 0;

    if (bStrong)
        WriteStr( "<H4>" );
    WriteStr(pName);
    if (bStrong)
        WriteStr( "</H4>" );
    FinishCell();
}

void
HtmlCreator::WriteElementData( TextElement & i_rElement,
                               E_LinkType    i_eLinkType )
{
    StartCell( "77%");

    bool bStrong = strcmp(i_rElement.Name(),"name") == 0 || strcmp(i_rElement.Name(),"module-name") == 0;

    if (bStrong)
    {
        WriteStr( "<H4><A NAME=\"" );
        int nLen = strlen(i_rElement.Data());
        if ( *i_rElement.Name() == 'n')
        {
            const char * pEnd = strchr(i_rElement.Data(), ' ');
            if (pEnd)
            {
                nLen = pEnd - i_rElement.Data();
            }
        }
        aFile.write( i_rElement.Data(), nLen );
        WriteStr( "\">" );
    }

    WriteName( aFile, sIdl_BaseDirectory, i_rElement.Data(), i_eLinkType );

    if (bStrong)
        WriteStr( "</A></H4>" );


    unsigned i_max = i_rElement.Size();
    if ( i_max > 1 )
    {
        for ( unsigned i = 1; i < i_max; ++i )
        {
            WriteStr( "<BR>\n" );
            WriteName( aFile, sIdl_BaseDirectory, i_rElement.Data(i), i_eLinkType );
        }   // end for
    }   // end if

    FinishCell();
}



