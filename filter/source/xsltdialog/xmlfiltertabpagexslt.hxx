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

    FixedText       maFTTransformationService;
    RadioButton		maRBTransformationServiceLibXSLT;
    RadioButton		maRBTransformationServiceSaxonJ;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
