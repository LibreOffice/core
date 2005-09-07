/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_start.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:37:23 $
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
#include "pm_start.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/ary.hxx>
#include "hd_chlst.hxx"
#include "hd_docu.hxx"
#include "navibar.hxx"
#include "html_kit.hxx"
#include "opageenv.hxx"
#include "pagemake.hxx"
#include "strconst.hxx"


using namespace csi;
using csi::html::Paragraph;
using csi::html::HorizontalLine;
using csi::html::AlignAttr;
using csi::html::Bold;
using csi::html::Link;
using csi::html::Sbr;
using csi::html::LineBreak;



PageMaker_Overview::PageMaker_Overview( PageDisplay & io_rPage )
    :   SpecializedPageMaker(io_rPage),
        pNavi(0)
{
}

PageMaker_Overview::~PageMaker_Overview()
{
}

void
PageMaker_Overview::MakePage()
{
    pNavi = new NavigationBar( Env(), NavigationBar::LOC_Overview );
    Write_NavBar();

    Write_TopArea();
    Write_DocuArea();
}

void
PageMaker_Overview::Write_NavBar()
{
    pNavi->Write( CurOut() );
    CurOut() << new HorizontalLine;
}

void
PageMaker_Overview::Write_TopArea()
{
    adcdisp::PageTitle_Std fTitle;
    fTitle( CurOut(), ary::Repository::The_().Name().c_str(), "" );

    CurOut()
        >> *new Paragraph
            << new html::StyleAttr("font-size:14pt;")
            << "This is a reference documentation for the C++ source code."
            << new LineBreak
            << new LineBreak
            << "Points to start:";

    html::SimpleList & rList = *new html::SimpleList;
    CurOut() >> rList;

    html::ListItem & rNamedObjsItem =
        rList.AddItem();

    StreamLock sNspDir(50);
    rNamedObjsItem
            << new html::StyleAttr("font-size:14pt;")
            >> *new Link( sNspDir() << C_sDIR_NamespacesCpp
                                    << "/"
                                    << C_sHFN_Namespace
                                    << c_str )
                    >> *new Bold
                        << "Named Objects";
    rNamedObjsItem << " (classes, functions, namespaces, etc.)"
                   << new html::LineBreak;
    rList.AddItem()
            << new html::StyleAttr("font-size:14pt;")
            >> *new Link( "def-all.html" )
                    >> *new Bold
                        << "Defines and Macros"
                        << new html::LineBreak;
    StreamLock sIndexDir(50);
    rList.AddItem()
            << new html::StyleAttr("font-size:14pt;")
            >> *new Link( sIndexDir() << C_sDIR_Index
                                      << "/index-1.html"
                                      << c_str )
                    >> *new Bold
                        << "Global Index"
                        << new html::LineBreak;
}

void
PageMaker_Overview::Write_DocuArea()
{
    CurOut() << new HorizontalLine;
}



