/*************************************************************************
 *
 *  $RCSfile: outfile.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-15 18:45:13 $
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
#include "outfile.hxx"

// NOT FULLY DECLARED SERVICES
#include <cosv/file.hxx>
#include <udm/html/htmlitem.hxx>


using namespace csi;
using csi::xml::AnAttribute;



#define CRLF "\n"
    static const char s_StdStyle[] =
//          "\ta { color: #444488; }" CRLF
/*
            "\ta.syntax { font-weight:bold; color: #444488; }" CRLF
            "\ta.memberlink { font-weight:bold; color: #444488; }" CRLF
            "\ta.objchapter { font-size:16pt; font-weight:bold }" CRLF
            "\ta.inverse { font-size:12pt; color: #ffffff; }" CRLF
            "\ta.inversebig { font-size:20pt; color: #ffffff; }" CRLF
            "\th2 { font-size:18pt; }" CRLF
            "\th2.inverse { font-size:18pt; color: #ffffff; }" CRLF
            "\th3 { font-size:15pt;  }" CRLF
            "\th3.inverse { font-size:15pt; color: #ffffff; }" CRLF
            "\th4 { font-size:14pt; }" CRLF
            "\th4.inverse { font-size:14pt; color: #ffffff; }" CRLF
            "\tp { font-size:12pt; margin-top:2pt; margin-bottom:1pt; }" CRLF
            "\tp.inverse { font-size:12pt; color: #ffffff; }" CRLF
            "\tp.tagtitle { font-size:14pt; font-weight:bold; margin-top:7pt; margin-bottom:3pt; }" CRLF
            "\tp.copyright {font-size: 10pt; font-style: italic; text-align: center}" CRLF
            "\tpre.inverse { color: #ffffff; }" CRLF
            "\tcode { color: #444488; }" CRLF
*/
            "\th1 { font-size:20pt; margin-top:3pt; margin-bottom:7pt; }" CRLF
            "\th2 { font-family:\"Arial\"; font-size:16pt; margin-top:3pt; margin-bottom:5pt; }" CRLF
            "\th3 { font-size:13pt; margin-top:2pt; margin-bottom:3pt; }" CRLF
            "\th4 { font-size:10pt; font-weight:bold; margin-top:2pt; margin-bottom:1pt; }" CRLF
            "\tdl { margin-top:1pt; margin-bottom:1pt; }" CRLF
            "\tdl.member { margin-top:1pt; margin-bottom:1pt; background-color:#eeeeff; }" CRLF
            "\tdt { font-size:10pt; font-weight:bold; margin-top:2pt; margin-bottom:1pt; }" CRLF
            "\tdt.member { font-size:13pt; font-weight:bold; margin-top:2pt; margin-bottom:1pt; }" CRLF
            "\tdt.simple { font-size:10pt; font-weight:normal; margin-top:2pt; margin-bottom:1pt; }" CRLF
            "\tdd { font-size:10pt; margin-top:1pt; margin-bottom:1pt; }" CRLF
            "\tdd.member { font-size:10pt; margin-top:1pt; margin-bottom:1pt; background-color:#ffffff; }" CRLF
            "\tp  { font-size:10pt; margin-top:3pt; margin-bottom:1pt; }" CRLF
            "\tpre { font-family: Times, serif; font-size:10pt; margin-top:1pt; margin-bottom:1pt; }" CRLF
            "\ttr { font-size:10pt; }" CRLF
            "\ttd { font-size:10pt; }" CRLF

//          "\t.TableHeadingColor     { background: #CCCCFF } /* Dark mauve */" CRLF
//          "\t.TableSubHeadingColor  { background: #EEEEFF } /* Light mauve */" CRLF
//          "\t.TableRowColor         { background: #FFFFFF } /* White */" CRLF
//          "\t.FrameTitleFont   { font-size: normal; font-family: normal }" CRLF
//          "\t.FrameHeadingFont { font-size: normal; font-family: normal }" CRLF
//          "\t.FrameItemFont    { font-size: normal; font-family: normal }" CRLF
//          "\ttd.NavBarCell1    { background-color:#EEEEFF;}/* Light mauve */" CRLF
//          "\ttd.NavBarCell1Rev { background-color:#00008B;}/* Dark Blue */" CRLF
//          "\tfont.NavBarFont1    { font-family: Arial, Helvetica, sans-serif; color:#000000;}" CRLF
//          "\tfont.NavBarFont1Rev { font-family: Arial, Helvetica, sans-serif; color:#FFFFFF;}" CRLF
//          "\t.NavBarCell2    { font-family: Arial, Helvetica, sans-serif; background-color:#FFFFFF;}" CRLF
//          "\t.NavBarCell3    { font-family: Arial, Helvetica, sans-serif; background-color:#FFFFFF;}" CRLF
            ;


HtmlDocuFile::HtmlDocuFile()
//  :   // aBodyData
{
    aBodyData
//      << new AnAttribute( "link", "#0000cc" )
//      << new AnAttribute( "vlink", "#9999cc" )
        << new AnAttribute( "bgcolor", "#ffffff" );

    SetStyle(s_StdStyle);
}

void
HtmlDocuFile::SetLocation( const csv::ploc::Path & i_rFilePath )
{
    static StreamStr sPath_(1000);
    sPath_.seekp(0);
    i_rFilePath.Get( sPath_ );

    sFilePath = sPath_.c_str();
}

void
HtmlDocuFile::SetTitle( const char * i_sTitle )
{
    sTitle = i_sTitle;
}

void
HtmlDocuFile::SetStyle( const char * i_sStyle )
{
    sStyle = i_sStyle;
}

void
HtmlDocuFile::SetBodyAttr( const char *        i_sAttrName,
                            const char *        i_sAttrValue )
{
    aBodyData << new AnAttribute( i_sAttrName, i_sAttrValue );
}

void
HtmlDocuFile::SetCopyright( const char * i_sCopyright )
{
    sCopyright = i_sCopyright;
}

void
HtmlDocuFile::EmptyBody()
{
     aBodyData.SetContent(0);
    aBodyData
        >> *new html::Label( "_top_" )
        << " ";

}

bool
HtmlDocuFile::CreateFile()
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
HtmlDocuFile::WriteHeader( csv::File & io_aFile )
{
    static const char s1[] =
        "<html>\n<head>\n<title>";
    static const char s2[] =
        "</title>\n<style>";
    static const char s3[] =
        "</style>\n</head>\n";

    io_aFile.write( s1 );
    io_aFile.write( sTitle );
    io_aFile.write( s2 );
    io_aFile.write( sStyle );
    io_aFile.write( s3 );
}

void
HtmlDocuFile::WriteBody( csv::File & io_aFile )
{
    aBodyData
        >> *new html::Link( "#_top_" )
                << new html::ClassAttr( "objchapter" )
                << "Top of Page";

    if ( sCopyright.length() > 0 )
    {
        aBodyData
#ifndef COMPATIBLE_NETSCAPE_47
            >> *new html::HorizontalLine
                    <<  new html::SizeAttr( "3" );
#else
            << new xml::XmlCode("<hr size=\"3\">");
#endif

        aBodyData
            >> *new html::Paragraph
                    << new html::ClassAttr( "copyright" )
                    << new xml::AnAttribute( "align", "center" )
                    << new xml::XmlCode(sCopyright);
    }
    aBodyData.WriteOut(io_aFile);
}







#if 0


#define CRLF "\n"
    static const char sStyle[] =
            "\ta { color: #444488; }" CRLF
            "\ta.syntax { font-weight:bold; color: #444488; }" CRLF
            "\ta.memberlink { font-weight:bold; color: #444488; }" CRLF
            "\ta.objchapter { font-size:16pt; font-weight:bold }" CRLF
            "\ta.inverse { font-size:12pt; color: #ffffff; }" CRLF
            "\ta.inversebig { font-size:20pt; color: #ffffff; }" CRLF
            "\th2 { font-size:18pt; }" CRLF
            "\th2.inverse { font-size:18pt; color: #ffffff; }" CRLF
            "\th3 { font-size:15pt;  }" CRLF
            "\th3.inverse { font-size:15pt; color: #ffffff; }" CRLF
            "\th4 { font-size:14pt; }" CRLF
            "\th4.inverse { font-size:14pt; color: #ffffff; }" CRLF
            "\tdt { margin-top:3pt; margin-bottom:1pt; }" CRLF
            "\tdd { margin-top:1pt; margin-bottom:1pt; }" CRLF
            "\tp { font-size:12pt; margin-top:2pt; margin-bottom:1pt; }" CRLF
            "\tp.inverse { font-size:12pt; color: #ffffff; }" CRLF
            "\tp.tagtitle { font-size:14pt; font-weight:bold; margin-top:7pt; margin-bottom:3pt; }" CRLF
            "\tp.copyright {font-size: 10pt; font-style: italic; text-align: center}" CRLF
            "\tpre.inverse { color: #ffffff; }" CRLF
            "\tcode { color: #444488; }" CRLF
            ".TableHeadingColor     { background: #CCCCFF } /* Dark mauve */" CRLF
            ".TableSubHeadingColor  { background: #EEEEFF } /* Light mauve */" CRLF
            ".TableRowColor         { background: #FFFFFF } /* White */" CRLF
            ".FrameTitleFont   { font-size: normal; font-family: normal }" CRLF
            ".FrameHeadingFont { font-size: normal; font-family: normal }" CRLF
            ".FrameItemFont    { font-size: normal; font-family: normal }" CRLF
            ".NavBarCell1    { background-color:#EEEEFF;}/* Light mauve */" CRLF
            ".NavBarCell1Rev { background-color:#00008B;}/* Dark Blue */" CRLF
            ".NavBarFont1    { font-family: Arial, Helvetica, sans-serif; color:#000000;}" CRLF
            ".NavBarFont1Rev { font-family: Arial, Helvetica, sans-serif; color:#FFFFFF;}" CRLF
            ".NavBarCell2    { font-family: Arial, Helvetica, sans-serif; background-color:#FFFFFF;}" CRLF
            ".NavBarCell3    { font-family: Arial, Helvetica, sans-serif; background-color:#FFFFFF;}" CRLF;
    static const char sBodyAttrs[] =
            "link=\"#444488\" vlink=\"#444488\" bgcolor=\"#ffffff\"";



#endif // 0

