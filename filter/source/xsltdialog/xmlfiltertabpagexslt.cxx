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
#include <unotools/pathoptions.hxx>
#include <osl/file.hxx>
#include <svl/urihelper.hxx>

#include "xmlfiltertabpagexslt.hxx"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfilterhelpids.hrc"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

XMLFilterTabPageXSLT::XMLFilterTabPageXSLT( Window* pParent) :
    TabPage( pParent, "XmlFilterTabPageTransformation", "filter/ui/xmlfiltertabpagetransformation.ui" ),

    sHTTPSchema( "http://" ),
    sSHTTPSchema( "https://" ),
    sFILESchema( "file://" ),
    sFTPSchema( "ftp://" ),
    sInstPath( "$(prog)/" )
{
    get(m_pEDDocType,"doc");
    get(m_pEDExportXSLT,"xsltexport");
    get(m_pPBExprotXSLT,"browseexport");
    get(m_pEDImportXSLT,"xsltimport");
    get(m_pPBImportXSLT,"browseimport");
    get(m_pEDImportTemplate,"tempimport");
    get(m_pPBImportTemplate,"browsetemp");
    get(m_pCBNeedsXSLT2,"filtercb");


    SvtPathOptions aOptions;
    sInstPath = aOptions.SubstituteVariable( sInstPath );

    m_pPBExprotXSLT->SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
    m_pPBImportXSLT->SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
    m_pPBImportTemplate->SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
}

XMLFilterTabPageXSLT::~XMLFilterTabPageXSLT()
{
}

bool XMLFilterTabPageXSLT::FillInfo( filter_info_impl* pInfo )
{
    if( pInfo )
    {
        pInfo->maDocType = m_pEDDocType->GetText();
        pInfo->maExportXSLT = GetURL( m_pEDExportXSLT );
        pInfo->maImportXSLT = GetURL( m_pEDImportXSLT );
        pInfo->maImportTemplate = GetURL( m_pEDImportTemplate );
        pInfo->mbNeedsXSLT2 = m_pCBNeedsXSLT2->IsChecked();
    }

    return true;
}

void XMLFilterTabPageXSLT::SetInfo(const filter_info_impl* pInfo)
{
    if( pInfo )
    {
        m_pEDDocType->SetText( pInfo->maDocType );

        SetURL( m_pEDExportXSLT, pInfo->maExportXSLT );
        SetURL( m_pEDImportXSLT, pInfo->maImportXSLT );
        SetURL( m_pEDImportTemplate, pInfo->maImportTemplate );
        m_pCBNeedsXSLT2->Check( pInfo->mbNeedsXSLT2 );
    }
}

void XMLFilterTabPageXSLT::SetURL( SvtURLBox* rURLBox, const OUString& rURL )
{
    OUString aPath;

    if( rURL.matchIgnoreAsciiCase( "file://" ) )
    {
        osl::FileBase::getSystemPathFromFileURL( rURL, aPath );

        rURLBox->SetBaseURL( rURL );
        rURLBox->SetText( aPath );
    }
    else if( rURL.matchIgnoreAsciiCase( "http://" ) ||
             rURL.matchIgnoreAsciiCase( "https://" ) ||
             rURL.matchIgnoreAsciiCase( "ftp://" ) )
    {
        rURLBox->SetBaseURL( rURL );
        rURLBox->SetText( rURL );
    }
    else if( !rURL.isEmpty() )
    {
        OUString aURL( rURL );
        aURL = URIHelper::SmartRel2Abs( sInstPath, aURL, Link(), false );
        osl::FileBase::getSystemPathFromFileURL( aURL, aPath );

        rURLBox->SetBaseURL( aURL );
        rURLBox->SetText( aPath );
    }
    else
    {
        rURLBox->SetBaseURL( sInstPath );
        OUString aEmpty;
        rURLBox->SetText( aEmpty );
    }
}

OUString XMLFilterTabPageXSLT::GetURL( SvtURLBox* rURLBox )
{
    OUString aURL;
    OUString aStrPath ( rURLBox->GetText() );
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

    if( pButton == m_pPBExprotXSLT )
    {
        pURLBox = m_pEDExportXSLT;
    }
    else if( pButton == m_pPBImportXSLT )
    {
        pURLBox = m_pEDImportXSLT;
    }
    else
    {
        pURLBox = m_pEDImportTemplate;
    }

    // Open Fileopen-Dialog
       ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );

    aDlg.SetDisplayDirectory( GetURL( pURLBox ) );

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        OUString aURL( aDlg.GetPath() );

        SetURL( pURLBox, aURL );
    }

    return( 0L );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
