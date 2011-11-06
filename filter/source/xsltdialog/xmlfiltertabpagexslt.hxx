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


#ifndef _XMLFILTERTABPAGEXSLT_HXX_
#define _XMLFILTERTABPAGEXSLT_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <svtools/inettbc.hxx>

class Window;
class ResMgr;
class filter_info_impl;

class XMLFilterTabPageXSLT : public TabPage
{
public:
    XMLFilterTabPageXSLT( Window* pParent, ResMgr& rResMgr, const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    virtual ~XMLFilterTabPageXSLT();

    bool FillInfo( filter_info_impl* pInfo );
    void SetInfo(const filter_info_impl* pInfo);

    DECL_LINK( ClickBrowseHdl_Impl, PushButton * );

    FixedText       maFTDocType;
    Edit            maEDDocType;

    FixedText       maFTDTDSchema;
    SvtURLBox       maEDDTDSchema;
    PushButton      maPBDTDSchemaBrowse;

    FixedText       maFTExportXSLT;
    SvtURLBox       maEDExportXSLT;
    PushButton      maPBExprotXSLT;

    FixedText       maFTImportXSLT;
    SvtURLBox       maEDImportXSLT;
    PushButton      maPBImportXSLT;

    FixedText       maFTImportTemplate;
    SvtURLBox       maEDImportTemplate;
    PushButton      maPBImportTemplate;

private:
    void SetURL( SvtURLBox& rURLBox, const rtl::OUString& rURL );
    rtl::OUString GetURL( SvtURLBox& rURLBox );

    ::rtl::OUString sHTTPSchema;
    ::rtl::OUString sSHTTPSchema;
    ::rtl::OUString sFILESchema;
    ::rtl::OUString sFTPSchema;
    ::rtl::OUString sInstPath;
};

#endif
