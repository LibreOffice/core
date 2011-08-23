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

#include <precomp.h>
#include "outfile.hxx"

// NOT FULLY DECLARED SERVICES
#include <cosv/file.hxx>
#include <udm/html/htmlitem.hxx>
#include <toolkit/out_position.hxx>
#include "strconst.hxx"


namespace
{
bool            bUse_OOoFrameDiv = true;
const String    C_sOOoFrameDiv_CppId("adc-cppref");
}


using namespace csi;
using csi::xml::AnAttribute;



#define CRLF "\n"

const char * const
    C_sStdStyle =
    "body   { background-color:#ffffff; }"CRLF
    "h1     { font-size:20pt; margin-top:3pt; margin-bottom:7pt; }"CRLF
    "h2     { font-family:Arial; font-size:16pt; margin-top:3pt; margin-bottom:5pt; }"CRLF
    "h3     { font-size:13pt; margin-top:2pt; margin-bottom:3pt; }"CRLF
    "h4     { font-size:10pt; font-weight:bold; margin-top:2pt; margin-bottom:1pt; }"CRLF
    "dl     { margin-top:1pt; margin-bottom:1pt; }"CRLF
    "dl.member  { margin-top:1pt; margin-bottom:1pt; background-color:#eeeeff; }"CRLF
    "dt     { font-size:10pt; font-weight:bold; margin-top:2pt; margin-bottom:1pt; }"CRLF
    "dt.member  { font-size:13pt; font-weight:bold; margin-top:2pt; margin-bottom:1pt; }"CRLF
    "dt.simple  { font-size:10pt; font-weight:normal; margin-top:2pt; margin-bottom:1pt; }"CRLF
    "dd     { font-size:10pt; margin-top:1pt; margin-bottom:1pt; }"CRLF
    "dd.member  { font-size:10pt; margin-top:1pt; margin-bottom:1pt; background-color:#ffffff; }"CRLF
    "p      { font-size:10pt; margin-top:3pt; margin-bottom:1pt; }"CRLF
    "pre    { font-family:monospace; font-size:10pt; margin-top:1pt; margin-bottom:1pt; }"CRLF
    "tr     { font-size:10pt; }"CRLF
    "td     { font-size:10pt; }"CRLF
    CRLF
    "dt.attention   { color:#dd0000; }"CRLF
    CRLF
    "div.title      { text-align:center;  line-height:26pt; background-color:#ccccff; }"CRLF
    ".subtitle      { background-color:#ccccff; }"CRLF
    CRLF
    "td.flagname    { background-color:#eeeeff; font-family:Arial; font-size:8pt; font-weight:bold; }"CRLF
    "td.flagyes     { font-family:Arial; font-size:8pt; font-weight:bold; }"CRLF
    "td.flagno      { font-family:Arial; font-size:8pt; }"CRLF
    "td.flagtext    { font-family:Arial; font-size:8pt; font-weight:bold; }"CRLF
    CRLF
    "td.navimain, td.navimain a"CRLF
    "               { background-color:#eeeeff; color:#000000;"CRLF
    "                 font-family:Arial; font-size:12pt; font-weight:bold; }"CRLF
    "td.navimainself"CRLF
    "               { background-color:#2222ad; color:#ffffff;"CRLF
    "                 font-family:Arial; font-size:12pt; font-weight:bold; }"CRLF
    "td.navimainnone"CRLF
    "               { background-color:#eeeeff; color:#000000;"CRLF
    "                 font-family:Arial; font-size:12pt; }"CRLF
    CRLF
    "div.define     { font-family:Arial; background-color:#ccccff; }"CRLF
    CRLF
    ".nqclass       { color:#008800; }"CRLF
    CRLF
    "h3.help        { background-color:#eeeeff; margin-top:12pt; }"CRLF
    CRLF
    ".btpubl        { color:#33ff33; }"CRLF
    ".btprot        { color:#cc9933; }"CRLF
    ".btpriv        { color:#ff6666; }"CRLF
    ".btvpubl       { color:#33ff33; font-style:italic; }"CRLF
    ".btvprot       { color:#cc9933; font-style:italic; }"CRLF
    ".btvpriv       { color:#ff6666; font-style:italic; }"CRLF
    ".btself        { font-weight:bold; }"CRLF
    ;


const char * const
    C_sCssExplanations =
    "/* Explanation of CSS classes:"CRLF
    CRLF
    "dl.member       provides coloured frame for function descriptions."CRLF
    "dd.member       makes the content of this frame white"CRLF
    CRLF
    "dt.attention    special colour for @attention remarks"CRLF
    CRLF
    "div.title       HTML page headline"CRLF
    ".subtitle       headline of lists of members and similar"CRLF
    CRLF
    "                These are for the flagtables in classes:"CRLF
    "td.flagname     Flag name."CRLF
    "td.flagyes      flag value \"yes\""CRLF
    "td.flagno       flag value \"no\""CRLF
    "td.flagtext     other flag value"CRLF
    CRLF
    CRLF
    "                These are for the main navigationbar:"CRLF
    "td.navimain, td.navimain a"CRLF
    "                Links in navibar."CRLF
    "td.navimainself Text in navibar which refers to current page."CRLF
    "td.navimainnone Text which links to nothing."CRLF
    CRLF
    CRLF
    "div.define      Subtitles on the #define/macro descriptions page"CRLF
    CRLF
    ".nqclass        special color for classes in the qualification"CRLF
    "                on top of type pages like in:"CRLF
    "                ::nsp1::nsp2::_ClassXY_::"CRLF
    CRLF
    "h3.help         Subtitles on the help page"CRLF
    CRLF
    "                These are for the base class tree on class pages:"CRLF
    ".btpubl         public base class"CRLF
    ".btprot         protected"CRLF
    ".btpriv         private"CRLF
    ".btvpubl        virtual public"CRLF
    ".btvprot        virtual protected"CRLF
    ".btvpriv        virtual private"CRLF
    ".btself         placeholder for currently displayed class"CRLF
    CRLF
    "*/"CRLF
    ;


const char * const
    C_sStdStyle_withDivFrame =
    "body   { background-color:#ffffff; }"CRLF
    "#adc-cppref h1     { font-size:20pt; margin-top:3pt; margin-bottom:7pt; }"CRLF
    "#adc-cppref h2     { font-family:Arial; font-size:16pt; margin-top:3pt; margin-bottom:5pt; }"CRLF
    "#adc-cppref h3     { font-size:13pt; margin-top:2pt; margin-bottom:3pt; }"CRLF
    "#adc-cppref h4     { font-size:10pt; font-weight:bold; margin-top:2pt; margin-bottom:1pt; }"CRLF
    "#adc-cppref dl     { margin-top:1pt; margin-bottom:1pt; }"CRLF
    "#adc-cppref dl.member  { margin-top:1pt; margin-bottom:1pt; background-color:#eeeeff; }"CRLF
    "#adc-cppref dt     { font-size:10pt; font-weight:bold; margin-top:2pt; margin-bottom:1pt; }"CRLF
    "#adc-cppref dt.member  { font-size:13pt; font-weight:bold; margin-top:2pt; margin-bottom:1pt; }"CRLF
    "#adc-cppref dt.simple  { font-size:10pt; font-weight:normal; margin-top:2pt; margin-bottom:1pt; }"CRLF
    "#adc-cppref dd     { font-size:10pt; margin-top:1pt; margin-bottom:1pt; }"CRLF
    "#adc-cppref dd.member  { font-size:10pt; margin-top:1pt; margin-bottom:1pt; background-color:#ffffff; }"CRLF
    "#adc-cppref p      { font-size:10pt; margin-top:3pt; margin-bottom:1pt; }"CRLF
    "#adc-cppref pre    { font-family:monospace; font-size:10pt; margin-top:1pt; margin-bottom:1pt; }"CRLF
    "#adc-cppref tr     { font-size:10pt; }"CRLF
    "#adc-cppref td     { font-size:10pt; }"CRLF
    CRLF
    "#adc-cppref dt.attention   { color:#dd0000; }"CRLF
    CRLF
    "#adc-cppref div.title      { text-align:center;  line-height:26pt; background-color:#ccccff; }"CRLF
    "#adc-cppref .subtitle      { background-color:#ccccff; }"CRLF
    CRLF
    "#adc-cppref td.flagname    { background-color:#eeeeff; font-family:Arial; font-size:8pt; font-weight:bold; }"CRLF
    "#adc-cppref td.flagyes     { font-family:Arial; font-size:8pt; font-weight:bold; }"CRLF
    "#adc-cppref td.flagno      { font-family:Arial; font-size:8pt; }"CRLF
    "#adc-cppref td.flagtext    { font-family:Arial; font-size:8pt; font-weight:bold; }"CRLF
    CRLF
    "#adc-cppref td.navimain, #adc-cppref td.navimain a"CRLF
    "               { background-color:#eeeeff; color:#000000;"CRLF
    "                 font-family:Arial; font-size:12pt; font-weight:bold; }"CRLF
    "#adc-cppref td.navimainself"CRLF
    "               { background-color:#2222ad; color:#ffffff;"CRLF
    "                 font-family:Arial; font-size:12pt; font-weight:bold; }"CRLF
    "#adc-cppref td.navimainnone"CRLF
    "               { background-color:#eeeeff; color:#000000;"CRLF
    "                 font-family:Arial; font-size:12pt; }"CRLF
    CRLF
    "#adc-cppref div.define     { font-family:Arial; background-color:#ccccff; }"CRLF
    CRLF
    "#adc-cppref .nqclass       { color:#008800; }"CRLF
    CRLF
    "#adc-cppref h3.help        { background-color:#eeeeff; margin-top:12pt; }"CRLF
    CRLF
    "#adc-cppref .btpubl        { color:#33ff33; }"CRLF
    "#adc-cppref .btprot        { color:#cc9933; }"CRLF
    "#adc-cppref .btpriv        { color:#ff6666; }"CRLF
    "#adc-cppref .btvpubl       { color:#33ff33; font-style:italic; }"CRLF
    "#adc-cppref .btvprot       { color:#cc9933; font-style:italic; }"CRLF
    "#adc-cppref .btvpriv       { color:#ff6666; font-style:italic; }"CRLF
    "#adc-cppref .btself        { font-weight:bold; }"CRLF
    ""CRLF
    "#adc-cppref table { empty-cells:show; }"CRLF
    ""CRLF
    "#adc-cppref .childlist td, "CRLF
    "#adc-cppref .commentedlinks td, "CRLF
    "#adc-cppref .memberlist td, "CRLF
    "#adc-cppref .subtitle td, "CRLF
    "#adc-cppref .crosstitle td  { border: .1pt solid #000000; }"CRLF
    ""CRLF
    "#adc-cppref .flag-table td { border: .1pt solid #cccccc; } "CRLF
    ""CRLF
    "#adc-cppref .title-table td, "CRLF
    "#adc-cppref .table-in-method td, "CRLF
    "#adc-cppref .table-in-data td, "CRLF
    "#adc-cppref .navimain td, "CRLF
    "#adc-cppref .navisub td, "CRLF
    "#adc-cppref .expl-table td, "CRLF
    "#adc-cppref .param-table td  { border: none; }"CRLF
    ;



HtmlDocuFile::HtmlDocuFile()
    :	sFilePath(),
        sTitle(),
        sLocation(),
        sCopyright(),
        nDepthInOutputTree(0),
        aBodyData(),
        aBuffer(60000)  // Grows dynamically when necessary.
{
}

void
HtmlDocuFile::SetLocation( const csv::ploc::Path &  i_rFilePath,
                           uintt                    i_depthInOutputTree )
{
    static StreamStr sPath_(1000);
    sPath_.seekp(0);
    i_rFilePath.Get( sPath_ );

    sFilePath = sPath_.c_str();
    nDepthInOutputTree = i_depthInOutputTree;
}

void
HtmlDocuFile::SetTitle( const char * i_sTitle )
{
    sTitle = i_sTitle;
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

     if (bUse_OOoFrameDiv)
     {
        // Insert <div> tag to allow better formatting for OOo.
        aBodyData
            << new xml::XmlCode("<div id=\"")
            << new xml::XmlCode(C_sOOoFrameDiv_CppId)
            << new xml::XmlCode("\">\n\n");
     }

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
HtmlDocuFile::WriteCssFile(	const csv::ploc::Path & i_rFilePath )
{
    Cout() << "\nCreate css file ..." << Endl();

    csv::File
        aCssFile(i_rFilePath, csv::CFM_CREATE);
    csv::OpenCloseGuard
        aOpenGuard(aCssFile);
    if (NOT aOpenGuard)
    {
        Cerr() << "Can't create file " << "cpp.css" << "." << Endl();
        return;
    }

    aCssFile.write("/*      Autodoc css file for C++ documentation      */\n\n\n");

    if (bUse_OOoFrameDiv)
        aCssFile.write(C_sStdStyle_withDivFrame);
    else
        aCssFile.write(C_sStdStyle);

    aCssFile.write("\n\n\n");
    aCssFile.write(C_sCssExplanations);
}

void
HtmlDocuFile::WriteHeader( csv::File & io_aFile )
{
    aBuffer.reset();

    static const char s1[] =
        "<html>\n<head>\n"
        "<title>";
    static const char s2[] =
        "</title>\n"
        "<link rel=\"stylesheet\" type=\"text/css\" href=\"";
    static const char s3[] =
        "\">\n</head>\n";

    aBuffer.write( s1 );
    aBuffer.write( sTitle );
    aBuffer.write( s2 );
    aBuffer.write( output::get_UpLink(nDepthInOutputTree) );
    aBuffer.write( C_sHFN_Css );
    aBuffer.write( s3 );

    io_aFile.write(aBuffer.c_str(), aBuffer.size());
}

void
HtmlDocuFile::WriteBody( csv::File & io_aFile )
{
    aBuffer.reset();

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

     if (bUse_OOoFrameDiv)
    {
        // Insert <div> tag to allow better formatting for OOo.
        aBodyData
            << new xml::XmlCode("\n</div> <!-- id=\"")
            << new xml::XmlCode(C_sOOoFrameDiv_CppId)
            << new xml::XmlCode("\" -->\n");
    }

    aBodyData.WriteOut(aBuffer);
    io_aFile.write(aBuffer.c_str(), aBuffer.size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
