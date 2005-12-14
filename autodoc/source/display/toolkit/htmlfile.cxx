/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlfile.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-12-14 15:35:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <precomp.h>
#include <toolkit/htmlfile.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/file.hxx>
#include <udm/html/htmlitem.hxx>

namespace
{
bool            bUse_OOoFrameDiv = true;
const String    C_sOOoFrameDiv_IdlId("adc-idlref");
}

using namespace csi;
using csi::xml::AnAttribute;

DocuFile_Html::DocuFile_Html()
    :   sFilePath(),
        sTitle(),
        sLocation(),
        sStyle(),
        sCssFile(),
        sCopyright(),
        aBodyData(),
        aBuffer(60000) // Grows dynamically, when necessary.
{
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
DocuFile_Html::SetInlineStyle( const char * i_sStyle )
{
    sStyle = i_sStyle;
}

void
DocuFile_Html::SetRelativeCssPath( const char * i_sCssFile_relativePath )
{
    sCssFile = i_sCssFile_relativePath;
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

     if (bUse_OOoFrameDiv)
     {
        // Insert <div> tag to allow better formatting for OOo.
        aBodyData
            << new xml::XmlCode("<div id=\"")
            << new xml::XmlCode(C_sOOoFrameDiv_IdlId)
            << new xml::XmlCode("\">\n\n");
     }

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
    aBuffer.reset();

    static const char s1[] =
        "<html>\n<head>\n<title>";
    static const char s2[] =
        "</title>\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n";

    aBuffer.write( s1 );
    aBuffer.write( sTitle );
    aBuffer.write( s2 );


    if (NOT sCssFile.empty())
    {
        static const char s3[] =
            "<link rel=\"stylesheet\" type=\"text/css\" href=\"";
        static const char s4[] =
            "\">\n";

        aBuffer.write(s3);
        aBuffer.write(sCssFile);
        aBuffer.write(s4);
    }

    if (NOT sStyle.empty())
    {
        static const char s5[] =
            "<style>";
        static const char s6[] =
            "</style>\n";

        aBuffer.write(s5);
        aBuffer.write(sStyle);
        aBuffer.write(s6);
    }

    static const char s7[] =
        "</head>\n";
    aBuffer.write(s7);

    io_aFile.write(aBuffer.c_str(), aBuffer.size());
}

void
DocuFile_Html::WriteBody( csv::File & io_aFile )
{
    aBuffer.reset();

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

     if (bUse_OOoFrameDiv)
    {
        // Insert <div> tag to allow better formatting for OOo.
        aBodyData
            << new xml::XmlCode("\n</div> <!-- id=\"")
            << new xml::XmlCode(C_sOOoFrameDiv_IdlId)
            << new xml::XmlCode("\" -->\n");
    }

    aBodyData.WriteOut(aBuffer);
    io_aFile.write(aBuffer.c_str(), aBuffer.size());
}







