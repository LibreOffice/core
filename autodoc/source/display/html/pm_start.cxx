/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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



