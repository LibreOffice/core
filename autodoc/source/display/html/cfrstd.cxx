/*************************************************************************
 *
 *  $RCSfile: cfrstd.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:23:09 $
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
#include <cfrstd.hxx>


// NOT FULLY DEFINED SERVICES


/*                      CSS Styles
                        ----------


Colors:
-   light background color              #eeeeff
-   dark background color               #ccccff
-   self in navibar background color    #2222ad


Fonts:
-   page title              20, bold, Arial
-   navibar main            12, bold, Arial
-   navibar sub              8, Arial, kapitälchen
-   attrtable title line     8, bold, Arial, kapitälchen
-   attrtable value line     8, Arial kapitälchen

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
const char * const C_sStdStyle =
    "h3 { font-size:13pt; font-weight:bold; margin-top:3pt; margin-bottom:1pt; }"CRLF
    "p, dt, dd, pre  { font-size:11pt; margin-top:3pt; margin-bottom:1pt; }"CRLF

    "table.lightbg { background-color:#eeeeff; }"CRLF
    "table.subtitle { margin-top:6pt; margin-bottom:6pt; }"CRLF

    "td { font-size:11pt; }"CRLF
    "td.title { font-family: Arial; font-size:19pt; font-weight:bold; text-align:center; background-color:#ccccff; line-height:30pt; }"CRLF
    "td.subtitle { font-family: Arial; font-size:13pt; background-color:#ccccff; line-height:20pt; }"CRLF
    "td.crosstitle { font-size:12pt; font-weight:bold; background-color:#eeeeff; line-height:15pt; }"CRLF
    "td.imdetail { width:100%; background-color:#eeeeff; }"CRLF
    "a.membertitle { font-size:12pt; font-weight:bold; line-height:18pt; }"CRLF

    "td.imsum_left { width:30%;  }"CRLF
    "td.imsum_right { width:70%;  }"CRLF

    "td.navimain, a.navimain { text-align:center; font-family: Arial; font-size:12pt; font-weight:bold; }"CRLF
    "td.navimainself { text-align:center; font-family: Arial; font-size:12pt; font-weight:bold; color:#ffffff; background-color:#2222ad; }"CRLF
    "td.navimainnone { text-align:center; font-family: Arial; font-size:12pt; }"CRLF
    "td.attrtitle { font-weight:bold; background-color:#eeeeff; }"CRLF
    "td.navisub, a.navisub, td.attrtitle, td.attrvalue { text-align:center; font-family: Arial; font-size:9pt; font-variant:small-caps; }"CRLF
    "td.navimain, td.navisub { padding-left:7pt; padding-right:7pt; }"CRLF

    "p.raise  { font-size:11pt; margin-top:0pt; text-align:right; padding-right:5pt; }"CRLF

    "a.navimain, a.navisub  { color:#000000; }"CRLF
    ".dt     { font-weight:bold; }"CRLF
    ".namechain  { font-size:13pt; font-weight:bold; margin-top:3pt; margin-bottom:6pt; }"CRLF
    ".tpl        { font-size:13pt; margin-top:3pt; margin-bottom:6pt; }"CRLF
    ;
}   // anonymous namespace


StdFrame::StdFrame()
    :   sDevelopersGuideHtmlRoot(),
        bSimpleLinks(false)
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

const char *
StdFrame::CopyrightText() const
{
//  return "Copyright &copy; 2002 Sun Microsystems, Inc., 901 San Antonio Road, Palo Alto, CA 94303 USA.";
    return "Copyright &copy; 2003 Sun Microsystems, Inc.";
//  return "Copyright 2001 OpenOffice.org Foundation. All Rights Reserved.";

}

const char *
StdFrame::CssStyle() const
{
    return C_sStdStyle;
}

const char *
StdFrame::DevelopersGuideHtmlRoot() const
{
    return sDevelopersGuideHtmlRoot;
}

bool
StdFrame::SimpleLinks() const
{
    return bSimpleLinks;
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

void
StdFrame::Set_SimpleLinks()
{
    bSimpleLinks = true;
}
