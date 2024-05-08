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

#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include "xmlfiltercommon.hxx"
#include "xmlfiltertabpagebasic.hxx"

XMLFilterTabPageBasic::XMLFilterTabPageBasic(weld::Widget* pPage)
    : m_xBuilder(Application::CreateBuilder(pPage, u"filter/ui/xmlfiltertabpagegeneral.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_widget(u"XmlFilterTabPageGeneral"_ustr))
    , m_xEDFilterName(m_xBuilder->weld_entry(u"filtername"_ustr))
    , m_xCBApplication(m_xBuilder->weld_combo_box(u"application"_ustr))
    , m_xEDInterfaceName(m_xBuilder->weld_entry(u"interfacename"_ustr))
    , m_xEDExtension(m_xBuilder->weld_entry(u"extension"_ustr))
    , m_xEDDescription(m_xBuilder->weld_text_view(u"description"_ustr))
{
    m_xEDDescription->set_size_request(-1, m_xEDDescription->get_height_rows(4));

    std::vector< application_info_impl > const & rInfos = getApplicationInfos();
    for (auto const& info : rInfos)
    {
        OUString aEntry( info.maDocumentUIName );
        m_xCBApplication->append_text( aEntry );
    }
}

XMLFilterTabPageBasic::~XMLFilterTabPageBasic()
{
}

static OUString checkExtensions( const OUString& rExtensions )
{
    const sal_Unicode* pSource = rExtensions.getStr();
    sal_Int32 nCount = rExtensions.getLength();

    OUStringBuffer aRet;
    while( nCount-- )
    {
        switch(*pSource)
        {
        case u',':
            aRet.append(";");
            break;
        case u'.':
        case u'*':
            break;
        default:
            aRet.append( *pSource );
        }

        pSource++;
    }

    return aRet.makeStringAndClear();
}

void XMLFilterTabPageBasic::FillInfo( filter_info_impl* pInfo )
{
    if( !pInfo )
        return;

    if( !m_xEDFilterName->get_text().isEmpty() )
        pInfo->maFilterName = m_xEDFilterName->get_text();

    if( !m_xCBApplication->get_active_text().isEmpty() )
        pInfo->maDocumentService = m_xCBApplication->get_active_text();

    if( !m_xEDInterfaceName->get_text().isEmpty() )
        pInfo->maInterfaceName = m_xEDInterfaceName->get_text();

    if( !m_xEDExtension->get_text().isEmpty() )
        pInfo->maExtension = checkExtensions( m_xEDExtension->get_text() );

    pInfo->maComment = string_encode( m_xEDDescription->get_text() );

    if( pInfo->maDocumentService.isEmpty() )
        return;

    std::vector< application_info_impl > const & rInfos = getApplicationInfos();
    for (auto const& info : rInfos)
    {
        if( pInfo->maDocumentService == info.maDocumentUIName )
        {
            pInfo->maDocumentService = info.maDocumentService;
            pInfo->maExportService = info.maXMLExporter;
            pInfo->maImportService = info.maXMLImporter;
            break;
        }
    }
}

void XMLFilterTabPageBasic::SetInfo(const filter_info_impl* pInfo)
{
    if( !pInfo )
        return;

    m_xEDFilterName->set_text( string_decode(pInfo->maFilterName) );
    /*
    if( pInfo->maDocumentService.getLength() )
        maCBApplication.set_text( getApplicationUIName( pInfo->maDocumentService ) );
    */
    if( !pInfo->maExportService.isEmpty() )
        m_xCBApplication->set_entry_text( getApplicationUIName( pInfo->maExportService ) );
    else
        m_xCBApplication->set_entry_text( getApplicationUIName( pInfo->maImportService ) );
    m_xEDInterfaceName->set_text( string_decode(pInfo->maInterfaceName) );
    m_xEDExtension->set_text( pInfo->maExtension );
    m_xEDDescription->set_text( string_decode( pInfo->maComment ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
