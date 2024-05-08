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

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <sfx2/filedlghelper.hxx>
#include <unotools/pathoptions.hxx>
#include <osl/file.hxx>
#include <svl/urihelper.hxx>
#include <vcl/svapp.hxx>

#include "xmlfiltercommon.hxx"
#include "xmlfiltertabpagexslt.hxx"

XMLFilterTabPageXSLT::XMLFilterTabPageXSLT(weld::Widget* pPage, weld::Dialog* pDialog)
    : sInstPath( u"$(prog)/"_ustr )
    , m_pDialog(pDialog)
    , m_xBuilder(Application::CreateBuilder(pPage, u"filter/ui/xmlfiltertabpagetransformation.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_widget(u"XmlFilterTabPageTransformation"_ustr))
    , m_xEDDocType(m_xBuilder->weld_entry(u"doc"_ustr))
    , m_xEDExportXSLT(new SvtURLBox(m_xBuilder->weld_combo_box(u"xsltexport"_ustr)))
    , m_xPBExprotXSLT(m_xBuilder->weld_button(u"browseexport"_ustr))
    , m_xEDImportXSLT(new SvtURLBox(m_xBuilder->weld_combo_box(u"xsltimport"_ustr)))
    , m_xPBImportXSLT(m_xBuilder->weld_button(u"browseimport"_ustr))
    , m_xEDImportTemplate(new SvtURLBox(m_xBuilder->weld_combo_box(u"tempimport"_ustr)))
    , m_xPBImportTemplate(m_xBuilder->weld_button(u"browsetemp"_ustr))
    , m_xCBNeedsXSLT2(m_xBuilder->weld_check_button(u"filtercb"_ustr))
{
    SvtPathOptions aOptions;
    sInstPath = aOptions.SubstituteVariable( sInstPath );

    m_xPBExprotXSLT->connect_clicked( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
    m_xPBImportXSLT->connect_clicked( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
    m_xPBImportTemplate->connect_clicked( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
}

XMLFilterTabPageXSLT::~XMLFilterTabPageXSLT()
{
}

void XMLFilterTabPageXSLT::FillInfo( filter_info_impl* pInfo )
{
    if( pInfo )
    {
        pInfo->maDocType = m_xEDDocType->get_text();
        pInfo->maExportXSLT = GetURL(*m_xEDExportXSLT);
        pInfo->maImportXSLT = GetURL(*m_xEDImportXSLT);
        pInfo->maImportTemplate = GetURL(*m_xEDImportTemplate);
        pInfo->mbNeedsXSLT2 = m_xCBNeedsXSLT2->get_active();
    }
}

void XMLFilterTabPageXSLT::SetInfo(const filter_info_impl* pInfo)
{
    if( pInfo )
    {
        m_xEDDocType->set_text( pInfo->maDocType );

        SetURL( *m_xEDExportXSLT, pInfo->maExportXSLT );
        SetURL( *m_xEDImportXSLT, pInfo->maImportXSLT );
        SetURL( *m_xEDImportTemplate, pInfo->maImportTemplate );
        m_xCBNeedsXSLT2->set_active(pInfo->mbNeedsXSLT2);
    }
}

void XMLFilterTabPageXSLT::SetURL( SvtURLBox& rURLBox, const OUString& rURL )
{
    OUString aPath;

    if( rURL.matchIgnoreAsciiCase( "file://" ) )
    {
        osl::FileBase::getSystemPathFromFileURL( rURL, aPath );

        rURLBox.SetBaseURL( rURL );
        rURLBox.set_entry_text( aPath );
    }
    else if( rURL.matchIgnoreAsciiCase( "http://" ) ||
             rURL.matchIgnoreAsciiCase( "https://" ) ||
             rURL.matchIgnoreAsciiCase( "ftp://" ) )
    {
        rURLBox.SetBaseURL( rURL );
        rURLBox.set_entry_text( rURL );
    }
    else if( !rURL.isEmpty() )
    {
        OUString aURL = URIHelper::SmartRel2Abs( INetURLObject(sInstPath), rURL, Link<OUString *, bool>(), false );
        osl::FileBase::getSystemPathFromFileURL( aURL, aPath );

        rURLBox.SetBaseURL( aURL );
        rURLBox.set_entry_text( aPath );
    }
    else
    {
        rURLBox.SetBaseURL( sInstPath );
        rURLBox.set_entry_text( u""_ustr );
    }
}

OUString XMLFilterTabPageXSLT::GetURL(const SvtURLBox& rURLBox)
{
    OUString aURL;
    OUString aStrPath(rURLBox.get_active_text());
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

IMPL_LINK ( XMLFilterTabPageXSLT, ClickBrowseHdl_Impl, weld::Button&, rButton, void )
{
    SvtURLBox* pURLBox;

    if( &rButton == m_xPBExprotXSLT.get() )
    {
        pURLBox = m_xEDExportXSLT.get();
    }
    else if( &rButton == m_xPBImportXSLT.get() )
    {
        pURLBox = m_xEDImportXSLT.get();
    }
    else
    {
        pURLBox = m_xEDImportTemplate.get();
    }

    // Open Fileopen-Dialog
    ::sfx2::FileDialogHelper aDlg(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                  FileDialogFlags::NONE, m_pDialog);

    aDlg.SetDisplayDirectory(GetURL(*pURLBox));

    if (aDlg.Execute() == ERRCODE_NONE)
    {
        OUString aURL(aDlg.GetPath());
        SetURL(*pURLBox, aURL);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
