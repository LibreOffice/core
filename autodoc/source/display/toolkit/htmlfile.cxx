/*************************************************************************
 *
 *  $RCSfile: htmlfile.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-02-20 09:41:53 $
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

#include <precomp.h>
#include <toolkit/htmlfile.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/file.hxx>
#include <udm/html/htmlitem.hxx>


using namespace csi;
using csi::xml::AnAttribute;

DocuFile_Html::DocuFile_Html()
    :   sFilePath(),
        sTitle(),
        sLocation(),
        sStyle(),
        sCopyright(),
        aBodyData()
{
    SetBodyAttr( "bgcolor", "#ffffff" );
}

void
DocuFile_Html::SetLocation( const csv::ploc::Path & i_rFilePath )
{
    StreamLock sPath(1000);
    i_rFilePath.Get( sPath() );

    sFilePath = sPath().c_str();
}

void
DocuFile_Html::SetTitle( const char * i_sTitle )
{
    sTitle = i_sTitle;
}

void
DocuFile_Html::SetStyle( const char * i_sStyle )
{
    sStyle = i_sStyle;
}

void
DocuFile_Html::SetBodyAttr( const char *        i_sAttrName,
                            const char *        i_sAttrValue )
{
    aBodyData << new AnAttribute( i_sAttrName, i_sAttrValue );
}

void
DocuFile_Html::SetCopyright( const char * i_sCopyright )
{
    sCopyright = i_sCopyright;
}

void
DocuFile_Html::EmptyBody()
{
    aBodyData.SetContent(0);
    aBodyData
        >> *new html::Label( "_top_" )
        << " ";

}

bool
DocuFile_Html::CreateFile()
{
    csv::File aFile(sFilePath, csv::CFM_CREATE);
    if (NOT aFile.open())
    {
        Cerr() << "Can't create file " << sFilePath << "." << Endl();
        return false;
    }

    WriteHeader(aFile);
    WriteBody(aFile);

    // Write end
    static const char sCompletion[] = "\n</html>\n";
    aFile.write( sCompletion );

    aFile.close();
    Cout() << '.' << Flush();
    return true;
}


void
DocuFile_Html::WriteHeader( csv::File & io_aFile )
{
    static const char s1[] =
        "<html>\n<head>\n<title>";
    static const char s2[] =
        "</title>\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
        "<style>";
    static const char s3[] =
        "</style>\n</head>\n";

    io_aFile.write( s1 );
    io_aFile.write( sTitle );
    io_aFile.write( s2 );
    io_aFile.write( sStyle );
    io_aFile.write( s3 );
}

void
DocuFile_Html::WriteBody( csv::File & io_aFile )
{
    aBodyData
        >> *new html::Link( "#_top_" )
                << "Top of Page";

    if ( sCopyright.length() > 0 )
    {
        aBodyData
            << new xml::XmlCode("<hr size=\"3\">");

        aBodyData
            >> *new html::Paragraph
                    << new html::ClassAttr( "copyright" )
                    << new xml::AnAttribute( "align", "center" )
                    << new xml::XmlCode(sCopyright);
    }
    aBodyData.WriteOut(io_aFile);
}







