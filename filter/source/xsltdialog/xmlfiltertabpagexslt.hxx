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

    FixedText		maFTDocType;
    Edit			maEDDocType;

    FixedText		maFTDTDSchema;
    SvtURLBox		maEDDTDSchema;
    PushButton		maPBDTDSchemaBrowse;

    FixedText		maFTExportXSLT;
    SvtURLBox		maEDExportXSLT;
    PushButton		maPBExprotXSLT;

    FixedText		maFTImportXSLT;
    SvtURLBox		maEDImportXSLT;
    PushButton		maPBImportXSLT;

    FixedText		maFTImportTemplate;
    SvtURLBox		maEDImportTemplate;
    PushButton		maPBImportTemplate;

private:
    void SetURL( SvtURLBox& rURLBox, const rtl::OUString& rURL );
    rtl::OUString GetURL( SvtURLBox& rURLBox );

    ::rtl::OUString	sHTTPSchema;
    ::rtl::OUString	sSHTTPSchema;
    ::rtl::OUString	sFILESchema;
    ::rtl::OUString	sFTPSchema;
    ::rtl::OUString	sInstPath;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
