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

#include <com/sun/star/frame/XConfigManager.hpp>

#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <sfx2/filedlghelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <osl/file.hxx>
#include <svl/urihelper.hxx>

#include "xmlfiltertabpagexslt.hxx"
#include "xmlfiltertabpagexslt.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfilterhelpids.hrc"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

XMLFilterTabPageXSLT::XMLFilterTabPageXSLT( Window* pParent, ResMgr& rResMgr, const css::uno::Reference< XMultiServiceFactory >& rxMSF ) :
    TabPage( pParent, ResId( RID_XML_FILTER_TABPAGE_XSLT, rResMgr ) ),

    maFTDocType( this, ResId( FT_XML_DOCTYPE, rResMgr ) ),
    maEDDocType( this, ResId( ED_XML_DOCTYPE, rResMgr ) ),

    maFTExportXSLT( this, ResId( FT_XML_EXPORT_XSLT, rResMgr ) ),
    maEDExportXSLT( this, ResId( ED_XML_EXPORT_XSLT, rResMgr ), INET_PROT_FILE ),
    maPBExprotXSLT( this, ResId( PB_XML_EXPORT_XSLT_BROWSE, rResMgr ) ),

    maFTImportXSLT( this, ResId( FT_XML_IMPORT_XSLT, rResMgr ) ),
    maEDImportXSLT( this, ResId( ED_XML_IMPORT_XSLT, rResMgr ), INET_PROT_FILE ),
    maPBImportXSLT( this, ResId( PB_XML_IMPORT_XSLT_BROWSE, rResMgr ) ),

    maFTImportTemplate( this, ResId( FT_XML_IMPORT_TEMPLATE, rResMgr ) ),
    maEDImportTemplate( this, ResId( ED_XML_IMPORT_TEMPLATE, rResMgr ), INET_PROT_FILE ),
    maPBImportTemplate( this, ResId( PB_XML_IMPORT_TEMPLATE_BROWSE, rResMgr ) ),

    maFTNeedsXSLT2( this, ResId (FT_XML_NEEDS_XSLT2, rResMgr ) ),
    maCBNeedsXSLT2( this, ResId (CB_XML_NEEDS_XSLT2, rResMgr ) ),

    sHTTPSchema( "http://" ),
    sSHTTPSchema( "https://" ),
    sFILESchema( "file://" ),
    sFTPSchema( "ftp://" ),
    sInstPath( "$(prog)/" )
{
    FreeResource();

    try
    {
        css::uno::Reference< XConfigManager > xCfgMgr( rxMSF->createInstance( "com.sun.star.config.SpecialConfigManager" ), UNO_QUERY );
        if( xCfgMgr.is() )
            sInstPath = xCfgMgr->substituteVariables( sInstPath );
    }
    catch(const Exception&)
    {
        OSL_FAIL( "XMLFilterTabPageXSLT::XMLFilterTabPageXSLT exception catched!" );
    }

    maPBExprotXSLT.SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
    maPBImportXSLT.SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
    maPBImportTemplate.SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );

    maEDExportXSLT.SetHelpId( HID_XML_FILTER_EXPORT_XSLT );
    maEDImportXSLT.SetHelpId( HID_XML_FILTER_IMPORT_XSLT );
    maEDImportTemplate.SetHelpId( HID_XML_FILTER_IMPORT_TEMPLATE );
    maCBNeedsXSLT2.SetHelpId( HID_XML_FILTER_NEEDS_XSLT2 );
}

XMLFilterTabPageXSLT::~XMLFilterTabPageXSLT()
{
}

bool XMLFilterTabPageXSLT::FillInfo( filter_info_impl* pInfo )
{
    if( pInfo )
    {
        pInfo->maDocType = maEDDocType.GetText();
        pInfo->maExportXSLT = GetURL( maEDExportXSLT );
        pInfo->maImportXSLT = GetURL( maEDImportXSLT );
        pInfo->maImportTemplate = GetURL( maEDImportTemplate );
        pInfo->mbNeedsXSLT2 = maCBNeedsXSLT2.IsChecked();
    }

    return true;
}

void XMLFilterTabPageXSLT::SetInfo(const filter_info_impl* pInfo)
{
    if( pInfo )
    {
        maEDDocType.SetText( pInfo->maDocType );

        SetURL( maEDExportXSLT, pInfo->maExportXSLT );
        SetURL( maEDImportXSLT, pInfo->maImportXSLT );
        SetURL( maEDImportTemplate, pInfo->maImportTemplate );
        maCBNeedsXSLT2.Check( pInfo->mbNeedsXSLT2 );
    }
}

void XMLFilterTabPageXSLT::SetURL( SvtURLBox& rURLBox, const OUString& rURL )
{
    OUString aPath;

    if( rURL.matchIgnoreAsciiCase( "file://" ) )
    {
        osl::FileBase::getSystemPathFromFileURL( rURL, aPath );

        rURLBox.SetBaseURL( rURL );
        rURLBox.SetText( aPath );
    }
    else if( rURL.matchIgnoreAsciiCase( "http://" ) ||
             rURL.matchIgnoreAsciiCase( "https://" ) ||
             rURL.matchIgnoreAsciiCase( "ftp://" ) )
    {
        rURLBox.SetBaseURL( rURL );
        rURLBox.SetText( rURL );
    }
    else if( !rURL.isEmpty() )
    {
        OUString aURL( rURL );
        aURL = URIHelper::SmartRel2Abs( sInstPath, aURL, Link(), false );
        osl::FileBase::getSystemPathFromFileURL( aURL, aPath );

        rURLBox.SetBaseURL( aURL );
        rURLBox.SetText( aPath );
    }
    else
    {
        rURLBox.SetBaseURL( sInstPath );
        OUString aEmpty;
        rURLBox.SetText( aEmpty );
    }
}

OUString XMLFilterTabPageXSLT::GetURL( SvtURLBox& rURLBox )
{
    OUString aURL;
    OUString aStrPath ( rURLBox.GetText() );
    if( aStrPath.matchIgnoreAsciiCase( "http://" ) ||
        aStrPath.matchIgnoreAsciiCase( "https://" ) ||
        aStrPath.matchIgnoreAsciiCase( "ftp://" ) )
    {
        return aStrPath;
    }
    else
    {
        osl::FileBase::getFileURLFromSystemPath( aStrPath, aURL );
    }

    return aURL;
}

IMPL_LINK ( XMLFilterTabPageXSLT, ClickBrowseHdl_Impl, PushButton *, pButton )
{
    SvtURLBox* pURLBox;

    if( pButton == &maPBExprotXSLT )
    {
        pURLBox = &maEDExportXSLT;
    }
    else if( pButton == &maPBImportXSLT )
    {
        pURLBox = &maEDImportXSLT;
    }
    else
    {
        pURLBox = &maEDImportTemplate;
    }

    // Open Fileopen-Dialog
       ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );

    aDlg.SetDisplayDirectory( GetURL( *pURLBox ) );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        OUString aURL( aDlg.GetPath() );

        SetURL( *pURLBox, aURL );
    }

    return( 0L );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
