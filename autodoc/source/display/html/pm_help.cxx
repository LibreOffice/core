/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_help.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:32:13 $
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
#include "pm_help.hxx"


// NOT FULLY DEFINED SERVICES
#include "navibar.hxx"
#include "html_kit.hxx"

using namespace csi;
using csi::html::Paragraph;
using csi::html::HorizontalLine;
using csi::html::Headline;
using csi::html::AlignAttr;
using csi::html::Bold;
using csi::html::Link;
using csi::html::Sbr;
using csi::html::LineBreak;
using csi::xml::Element;



const String  C_sHelpText(
"<div style=\"font-size:10pt;\">\n"
"<h3 class=\"help\">The Main Navigationbar</h3>\n"
"<p> On top of every page, there is a main navigationbar on a lightly coloured\n"
"background with the following items:<br>\n"
"</p>\n"
"<ul>\n"
"  <li>Overview - the start page for this document,</li>\n"
"  <li>Namespace - the lowest/deepest namespace of the language objects, described\n"
"on the current page,</li>\n"
"  <li>Class - the class, struct or union, which owns the methods or data,\n"
"described on the current page,</li>\n"
"  <li>Index - the global alphabetical index,</li>\n"
"  <li>Help - this page.</li>\n"
"</ul>\n"
" Each item in this&nbsp; bar can be in three different states:<br>\n"
"<ul>\n"
"  <li>Link - the item is valid and you can get there,</li>\n"
"  <li>Simple - the item does not apply (if this page described a namespace,\n"
"there would be no owning class),</li>\n"
"  <li>Reversed (white text on dark background) - this is the current page.</li>\n"
"</ul>\n"
"<h3 class=\"help\">Lower Navigationbars</h3>\n"
" Just below the main navigation bar, there may be zero to three lower navigationbars\n"
"on white background.<br>\n"
"<br>\n"
"Their items are dependent of the context, but they always link to paragraphs\n"
"on the same, current page.<br>\n"
"Available items appear as links. Unavailable items appear as simple text.<br>\n"
"\n"
"\n"
"<h3 class=\"help\">Namespace Descriptions</h3>\n"
"\n"
"<dl>\n"
"<dt class=\"simple\">Parent namespaces</dt>\n"
"  </dl>\n"
"  <dl>\n"
"  <dd>In front of the namespace title, there is a linked list of the parent\n"
"namespaces. The global namespace is linked with the first \"::\",</dd>\n"
"    <dd>the namespaces between the global and the current one are linked\n"
"by their names.<br>\n"
"      </dd>\n"
"      <dd> </dd>\n"
"        </dl>\n"
"After the title, the documentation of the namespace follows (which is often\n"
"missing, because the namespace name may be self-explaining).<br>\n"
"        <br>\n"
"Below are the lists of nested namspaces and of the classes, functions and\n"
"other program objects, that belong within this namespace.<br>\n"
"Each of this lists is accessible by the lower navigationbar on top of the\n"
"page.<br>\n"
"\n"
"<h3 class=\"help\">Class Descriptions</h3>\n"
"\n"
"        <dl>\n"
"        <dt class=\"simple\">Parent namespaces and classes</dt>\n"
"          </dl>\n"
"          <dl>\n"
"          <dd>In front of the class title, there is a linked list of the\n"
"parent namespaces or classes. The global namespace is linked with the first\n"
"\"::\",</dd>\n"
"            <dd>the namespaces between the global and the current one are\n"
"linked by their names. Enclosing classes are linked as well, but appear in\n"
"<span class=\"nqclass\">green</font> color.&nbsp;</dd>\n"
"              <dd>So you see on the first glance, that this is a parent class,\n"
"no namespace.<br>\n"
"               </dd>\n"
"                <dd><br>\n"
"                  </dd>\n"
"                  </dl>\n"
"After the title, the bases and derivations of the class follow. <br>\n"
"Base classes are displayed as a graph. The text around base classes can appear\n"
"in different styles and colours:<br>\n"
"<ul>\n"
"  <li><span class=\"btpubl\">Green</span> - public inherited,</li>\n"
"  <li><span class=\"btprot\">Orange</span> - protected inherited,</li>\n"
"  <li><span class=\"btpriv\">Red</span> - private inherited,</li>\n"
"  <li><span class=\"btvpubl\">italic</span> - a (public inherited) virtual base class.</li>\n"
"  <li><span class=\"btself\">Bold and black</span> without a link - the placeholder\n"
"for the currently described class.<br>\n"
"  </li>\n"
"</ul>\n"
"There may be many derivations of a class, but only the known ones, which\n"
"are described within this document also, are listed.<br>\n"
"<br>\n"
"Below the derivations is a little table with some properties of the class:<br>\n"
"<ul>\n"
"  <li>virtual - the class owns at least one virtual method,</li>\n"
"  <li>abstract - the class owns at least one abstract method,</li>\n"
"  <li>interface - the class may or may be not abstract,\n"
"but it is intended by its author to be used only as an interface and never\n"
"to be instantiated,</li>\n"
"  <li>template - the class is a template class.<br>\n"
"  </li>\n"
"</ul>\n"
"Next comes further documentation of the class itself.<br>\n"
"<br>\n"
"Lastly, there are listed all members of the class. Public members come first,\n"
"then protected, at last the private ones.<br>\n"
"All member lists are accessible by the lower navigationbars on top of the\n"
"page.<br>\n"
"\n"
"<h3 class=\"help\">Macros and Defines</h3>\n"
"In C++ and C, there are also program constructs, which do not fit into the\n"
"name tree, because they are #define'd: macros and definitions.<br>\n"
"These may be documented, too. Those comments you find <a href=\"def-all.html\">\n"
"here</a>\n"
" or from the \"Overview\" start page.\n"
"<h3 class=\"help\">Links to IDL-Documentation</h3>\n"
"Some types, which appear as links, may refer to classes, enums or other\n"
"entities, which are direct mappings of UNO-IDL entities.<br>\n"
"In those cases the link doesn't lead to the C++ class, enum or whatever,\n"
"but to the description of the IDL entity.\n"
"<h3 class=\"help\">How to Link From Extern Documents</h3>\n"
"If you wish to write an extern html document, which links to types within\n"
"this C++ reference, you can do so, if your links have the following format:<br>\n"
"<br>\n"
"&lt;RootDirectory-of-this-Document&gt;/names/&lt;Namespace-A&gt;/&lt;Namespace-XY&gt;/EnclosingClass-nn&gt;/&lt;TypePreFix&gt;-&lt;MyTypeName&gt;.html<br>\n"
"<br>\n"
"&lt;TypePreFix&gt; can have the following values:<br>\n"
"<ul>\n"
"<li>c - class, struct or union</li>\n"
"<li>e - enum</li>\n"
"<li>t - typedef</li>\n"
"</ul>\n"
"If this document would be located in directory &nbsp;\"/doc/cpp/ref\", examples\n"
"would look like this:<br>\n"
"<br>\n"
"&lt;a href=\"/doc/cpp/ref/names/osl/c-File.html\"&gt;class File&lt;/a&gt;<br>\n"
"&lt;a href=\"/doc/cpp/ref/names/osl/FileBase/e-RC.html\"&gt;enum FileBase::RC&lt;/a&gt;<br>\n"
"&lt;a href=\"/doc/cpp/ref/names/t-oslMutex.html\"&gt;typedef oslMutex&lt;/a&gt;<br>\n"
"<br>\n"
"Namespaces are described in the index.html file within their directory:<br>\n"
"<br>\n"
"&lt;a href=\"/doc/cpp/ref/names/cppu/index.html\"&gt;namespace cppu&lt;/a&gt;<br>\n"
"</div>" );




PageMaker_Help::PageMaker_Help( PageDisplay & io_rPage )
    :   SpecializedPageMaker(io_rPage),
        pNavi(0)
{
}

PageMaker_Help::~PageMaker_Help()
{
}

void
PageMaker_Help::MakePage()
{
    pNavi = new NavigationBar( Env(), NavigationBar::LOC_Help );
    Write_NavBar();

    Write_TopArea();
    Write_DocuArea();
}

void
PageMaker_Help::Write_NavBar()
{
    pNavi->Write( CurOut() );
    CurOut() << new HorizontalLine;
}

void
PageMaker_Help::Write_TopArea()
{
    adcdisp::PageTitle_Std fTitle;
    fTitle( CurOut(), "How to Use", "this Reference Document" );

    CurOut() << new xml::XmlCode(C_sHelpText);
}

void
PageMaker_Help::Write_DocuArea()
{
    CurOut() << new HorizontalLine;
}



