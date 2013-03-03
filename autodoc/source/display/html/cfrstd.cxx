/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <precomp.h>
#include <cfrstd.hxx>


// NOT FULLY DEFINED SERVICES
#include <time.h>


/*                      CSS Styles
                        ----------

Colors:
-   light background color              #eeeeff
-   dark background color               #ccccff
-   self in navibar background color    #2222ad


Fonts:
-   page title              20, bold, Arial
-   navibar main            12, bold, Arial
-   navibar sub              8, Arial, small caps
-   attrtable title line     8, bold, Arial, small caps
-   attrtable value line     8, Arial small caps

-   namespace chain         13, bold
-   table title             13, bold
-   template line           13

-   member paragraph title  12, bold

-   docu paragraph title    11, bold
-   standard text           11

-   hierarchy               11, monospace


classes:

    td.title                page title
    h3                      table title
    h4                      member paragraph title

    td.nmain                navigation main bar
    td.nsub                 navigation sub bar
    a.nmain                 links in navigation main bar
    a.nsub                  links in navigation sub bar

    td.attr1                attribute table head line
    td.attr2                attribute table value line

    p.namechain             namespace chain in head of pages
    p.tpl                   template line in head of pages

    pre.doc                 preformatted docu
    pre.hierarchy           class bases hierarchy graphic

    dl.syntax               function- or variable-declaration field
    a.syntax                link in function- or variable-declaration field

    p.dt                    docu paragraph title
    dl.dt                   docu paragraph title

    p                       standard text
    dl                      standard text
    dd                      standard text
*/


#define CRLF "\n"

namespace
{

bool            bUse_OOoFrameDiv = true;


//***************   These are used for IDL currently only!   ********************

const char * const C_sStdStyle =
    "/*See bottom of file for explanations.*/" CRLF
    CRLF
    "body { background-color:#ffffff; }" CRLF
    CRLF
    "h3             { font-size:13pt; font-weight:bold;" CRLF
    "                 margin-top:3pt; margin-bottom:1pt; }" CRLF
    "p, dt, dd, pre { font-size:11pt;" CRLF
    "                 margin-top:3pt; margin-bottom:1pt; }" CRLF
    "pre            { font-family:monospace; }" CRLF
    CRLF
    "table.navimain { background-color:#eeeeff; }" CRLF
    "table.subtitle { margin-top:6pt; margin-bottom:6pt; }" CRLF
    CRLF
    "td             {                     font-size:11pt; }" CRLF
    "td.title       { font-family: Arial; font-size:19pt; font-weight:bold;" CRLF
    "                 line-height:30pt;   background-color:#ccccff; text-align:center; }" CRLF
    "td.subtitle    { font-family: Arial; font-size:13pt;" CRLF
    "                 line-height:20pt;   background-color:#ccccff; }" CRLF
    "td.crosstitle  {                     font-size:12pt; font-weight:bold;" CRLF
    "                 line-height:15pt;   background-color:#eeeeff; }" CRLF
    "td.imdetail    { width:100%;         background-color:#eeeeff; }" CRLF
    CRLF
    "td.imsum_left  { width:30%;  }" CRLF
    "td.imsum_right { width:70%;  }" CRLF
    CRLF
    "td.navimain, a.navimain" CRLF
    "                   { text-align:center; font-family: Arial; font-size:12pt; font-weight:bold; }" CRLF
    "td.navimainself    { text-align:center; font-family: Arial; font-size:12pt; font-weight:bold;" CRLF
    "                     color:#ffffff; background-color:#2222ad; }" CRLF
    "td.navimainnone    { text-align:center; font-family: Arial; font-size:12pt; }" CRLF
    "td.navisub, a.navisub" CRLF
    "                   { text-align:center; font-family: Arial; font-size:9pt; font-variant:small-caps; }" CRLF
    "td.navimain, td.navisub" CRLF
    "                   { padding-left:7pt; padding-right:7pt; }" CRLF
    CRLF
    "a.membertitle  { font-size:12pt; font-weight:bold; line-height:18pt; }" CRLF
    "a.navimain, a.navisub  { color:#000000; }" CRLF
    ".dt            { font-weight:bold; }" CRLF
    ".namechain     { font-size:13pt; font-weight:bold;" CRLF
    "                 margin-top:3pt; margin-bottom:6pt; }" CRLF
    ".title2        { font-size:13pt; font-style:italic; font-weight:bold; text-align:left; }" CRLF
    ;


const char * const C_sCssExplanations =
    "/* Explanation of CSS classes:" CRLF
    CRLF
    ".navimain          Text in main navigation bar." CRLF
    ".navisub           Text in lower navigation bar." CRLF
    "td.navimainself    Cell in main navigation bar with \"selected\" shadow: You are here." CRLF
    "td.navimainnone    Cell in main navigation bar with no link." CRLF
    CRLF
    ".namechain         Line with current module path." CRLF
    CRLF
    "td.crosstitle      Comment box for bases (base interfaces etc.)" CRLF
    "td.imsum_left      Left part of such boxes." CRLF
    "td.imsum_right     Right part of such boxes." CRLF
    CRLF
    "td.title           Main title of the page like \"interface XYz\"" CRLF
    ".subtitle          Tables, and head cells of those, which list members" CRLF
    "                   like \"method summary\" and \"method details\"." CRLF
    CRLF
    "td.imdetail        Background table of method's detail description." CRLF
    "a.membertitle      Method name (as jump label) in method's detail" CRLF
    "                   description." CRLF
    ".title2            smaller font prefixes to page titles" CRLF
    "*/" CRLF
    ;

const char * const C_sStdStyle_withDivFrame =
    "/*See bottom of file for explanations.*/" CRLF
    CRLF
    "body { background-color:#ffffff; }" CRLF
    CRLF
    "#adc-idlref h3 { font-size:13pt; font-weight:bold;" CRLF
    "                 margin-top:3pt; margin-bottom:1pt; }" CRLF
    "#adc-idlref p, #adc-idlref dt, #adc-idlref dd, #adc-idlref pre" CRLF
    "               { font-size:11pt;" CRLF
    "                 margin-top:3pt; margin-bottom:1pt; }" CRLF
    "#adc-idlref pre    { font-family:monospace; }" CRLF
    CRLF
    "#adc-idlref table.navimain { background-color:#eeeeff; }" CRLF
    "#adc-idlref table.subtitle { margin-top:6pt; margin-bottom:6pt; }" CRLF
    CRLF
    "#adc-idlref td             { font-size:11pt; }" CRLF
    "#adc-idlref td.title       { font-family: Arial; font-size:19pt; font-weight:bold;" CRLF
    "                             line-height:30pt;   background-color:#ccccff; text-align:center; }" CRLF
    "#adc-idlref td.subtitle    { font-family: Arial; font-size:13pt;" CRLF
    "                             line-height:20pt;   background-color:#ccccff; }" CRLF
    "#adc-idlref td.crosstitle  { font-size:12pt; font-weight:bold;" CRLF
    "                             line-height:15pt;   background-color:#eeeeff; }" CRLF
    "#adc-idlref td.imdetail    { width:100%;         background-color:#eeeeff; }" CRLF
    CRLF
    "#adc-idlref td.imsum_left  { width:30%;  }" CRLF
    "#adc-idlref td.imsum_right { width:70%;  }" CRLF
    CRLF
    "#adc-idlref td.navimain, #adc-idlref a.navimain" CRLF
    "                   { text-align:center; font-family: Arial; font-size:12pt; font-weight:bold; }" CRLF
    "#adc-idlref td.navimainself    { text-align:center; font-family: Arial; font-size:12pt; font-weight:bold;" CRLF
    "                                 color:#ffffff; background-color:#2222ad; }" CRLF
    "#adc-idlref td.navimainnone    { text-align:center; font-family: Arial; font-size:12pt; }" CRLF
    "#adc-idlref td.navisub, #adc-idlref a.navisub" CRLF
    "                   { text-align:center; font-family: Arial; font-size:9pt; font-variant:small-caps; }" CRLF
    "#adc-idlref td.navimain, #adc-idlref td.navisub" CRLF
    "                   { padding-left:7pt; padding-right:7pt; }" CRLF
    CRLF
    "#adc-idlref a.membertitle  { font-size:12pt; font-weight:bold; line-height:18pt; }" CRLF
    "#adc-idlref a.navimain, #adc-idlref a.navisub  { color:#000000; }" CRLF
    "#adc-idlref .dt            { font-weight:bold; }" CRLF
    "#adc-idlref .namechain     { font-size:13pt; font-weight:bold;" CRLF
    "                             margin-top:3pt; margin-bottom:6pt; }" CRLF
    "#adc-idlref .title2        { font-size:13pt; font-style:italic; font-weight:bold; text-align:left; }" CRLF
    "" CRLF
    "#adc-idlref table { empty-cells:show; }" CRLF
    "" CRLF
    "#adc-idlref .childlist td, " CRLF
    "#adc-idlref .commentedlinks td, " CRLF
    "#adc-idlref .memberlist td, " CRLF
    "#adc-idlref .subtitle td, " CRLF
    "#adc-idlref .crosstitle td  { border: .1pt solid #000000; }" CRLF
    "" CRLF
    "#adc-idlref .flag-table td { border: .1pt solid #cccccc; } " CRLF
    "" CRLF
    "#adc-idlref .title-table td, " CRLF
    "#adc-idlref .table-in-method td, " CRLF
    "#adc-idlref .table-in-data td, " CRLF
    "#adc-idlref .navimain td, " CRLF
    "#adc-idlref .navisub td, " CRLF
    "#adc-idlref .expl-table td, " CRLF
    "#adc-idlref .param-table td  { border: none; }" CRLF
    ;


}   // anonymous namespace


StdFrame::StdFrame()
    :   sDevelopersGuideHtmlRoot()
{
}

DYN Html_Image *
StdFrame::LogoSrc() const
{
    return 0;

//    return  new Html_Image( "logodot-blu.gif",
//                            "109",
//                            "54",
//                            "RIGHT",
//                            "0",
//                            "OpenOffice" );

}

const char *
StdFrame::LogoLink() const
{
    return "";
//  return "http://www.sun.com";
//  return "http://www.openoffice.org";
}


String              MakeCopyRight();

const char *
StdFrame::CopyrightText() const
{
    static String sCopyRight_( MakeCopyRight() );
    return sCopyRight_.c_str();
}

const char *
StdFrame::CssStyle() const
{
    if (bUse_OOoFrameDiv)
        return C_sStdStyle_withDivFrame;
    else
        return C_sStdStyle;
}

const char *
StdFrame::CssStylesExplanation() const
{
    return C_sCssExplanations;
}

const char *
StdFrame::DevelopersGuideHtmlRoot() const
{
    return sDevelopersGuideHtmlRoot;
}

void
StdFrame::Set_DevelopersGuideHtmlRoot( const String & i_directory )
{
    if (NOT i_directory.empty())
    {
        if (i_directory.char_at(i_directory.length()-1) == '/')
        {
            sDevelopersGuideHtmlRoot.assign(i_directory,i_directory.length()-1);
            return;
        }
    }
    sDevelopersGuideHtmlRoot = i_directory;
}

String
MakeCopyRight()
{
    StreamStr cr(1400);
    time_t
        gt;
    time(&gt);
    tm *
        plt = localtime(&gt);
    int year = 1900 + plt->tm_year;

    cr  << "Copyright &copy; 2000, "
	<< year
	<< " LibreOffice contributors and/or their affiliates. All rights reserved.</p>"
	<< "<p class=\"copyright\" align=\"center\">LibreOffice was created by The Document Foundation, based on OpenOffice.org, which is Copyright 2000, 2010 Oracle and/or its affiliates.</p>"
	<< "<p class=\"copyright\" align=\"center\">The Document Foundation acknowledges all community members, please find more info <a href=\"http://www.libreoffice.org/about-us/credits/\" target=\"_blank\">at our website</a>."
	<< "<p>&nbsp;</p>"
	<< "<p class=\"copyright\" align=\"center\"><a href=\"http://www.documentfoundation.org/privacy\" target=\"_blank\">Privacy Policy</a> | <a href=\"http://www.documentfoundation.org/imprint\" target=\"_blank\">Impressum (Legal Info)</a> | Copyright information: The source code of LibreOffice is licensed under the GNU Lesser General Public License (<a href=\"http://www.libreoffice.org/download/license/\" target=\"_blank\">LGPLv3</a>). \"LibreOffice\" and \"The Document Foundation\" are registered trademarks of their corresponding registered owners or are in actual use as trademarks in one or more countries. Their respective logos and icons are also subject to international copyright laws. Use thereof is explained in our <a href=\"http://wiki.documentfoundation.org/TradeMark_Policy\" target=\"_blank\">trademark policy</a>.";
    return String(cr.c_str());

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
