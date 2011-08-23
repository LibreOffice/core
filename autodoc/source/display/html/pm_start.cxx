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
    fTitle( CurOut(), Env().RepositoryTitle(), "" );

    CurOut()
        >> *new Paragraph
            << new html::StyleAttr("font-size:14pt;")
            << "This is a reference documentation for the C++ source code."
            << new LineBreak
            << new LineBreak
            << "Points to start:";

    html::SimpleList &
        rList = *new html::SimpleList;
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
