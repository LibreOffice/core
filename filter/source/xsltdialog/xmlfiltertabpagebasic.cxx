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

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltertabpagebasic.hxx"
#include "xmlfiltersettingsdialog.hxx"

XMLFilterTabPageBasic::XMLFilterTabPageBasic(Window* pParent)
    : TabPage(pParent, "XmlFilterTabPageGeneral", "filter/ui/xmlfiltertabpagegeneral.ui")
{
    get(m_pEDFilterName,   "filtername");
    get(m_pCBApplication,  "application");
    get(m_pEDInterfaceName,"interfacename");
    get(m_pEDExtension,    "extension");
    get(m_pEDDescription,  "description");
    m_pEDDescription->set_height_request(m_pEDDescription->GetTextHeight() * 4);

    std::vector< application_info_impl* >& rInfos = getApplicationInfos();
    std::vector< application_info_impl* >::iterator aIter( rInfos.begin() );
    while( aIter != rInfos.end() )
    {
        OUString aEntry( (*aIter++)->maDocumentUIName );
        m_pCBApplication->InsertEntry( aEntry );
    }
}

XMLFilterTabPageBasic::~XMLFilterTabPageBasic()
{
}
static OUString checkExtensions( const OUString& rExtensions )
{
    const sal_Unicode* pSource = rExtensions.getStr();
    sal_Int32 nCount = rExtensions.getLength();

    OUString aRet;
    while( nCount-- )
    {
        switch(*pSource)
        {
        case sal_Unicode(','):
            aRet += ";";
            break;
        case sal_Unicode('.'):
        case sal_Unicode('*'):
            break;
        default:
            aRet += OUString( *pSource );
        }

        pSource++;
    }

    return aRet;
}

bool XMLFilterTabPageBasic::FillInfo( filter_info_impl* pInfo )
{
    if( pInfo )
    {
        if( !m_pEDFilterName->GetText().isEmpty() )
            pInfo->maFilterName = m_pEDFilterName->GetText();

        if( !m_pCBApplication->GetText().isEmpty() )
            pInfo->maDocumentService = m_pCBApplication->GetText();

        if( !m_pEDInterfaceName->GetText().isEmpty() )
            pInfo->maInterfaceName = m_pEDInterfaceName->GetText();

        if( !m_pEDExtension->GetText().isEmpty() )
            pInfo->maExtension = checkExtensions( m_pEDExtension->GetText() );

        pInfo->maComment = string_encode( m_pEDDescription->GetText() );

        if( !pInfo->maDocumentService.isEmpty() )
        {
            std::vector< application_info_impl* >& rInfos = getApplicationInfos();
            std::vector< application_info_impl* >::iterator aIter( rInfos.begin() );
            while( aIter != rInfos.end() )
            {
                if( pInfo->maDocumentService == (*aIter)->maDocumentUIName )
                {
                    pInfo->maDocumentService = (*aIter)->maDocumentService;
                    pInfo->maExportService = (*aIter)->maXMLExporter;
                    pInfo->maImportService = (*aIter)->maXMLImporter;
                    break;
                }
                ++aIter;
            }
        }
    }

    return true;
}

void XMLFilterTabPageBasic::SetInfo(const filter_info_impl* pInfo)
{
    if( pInfo )
    {
        m_pEDFilterName->SetText( string_decode(pInfo->maFilterName) );
        /*
        if( pInfo->maDocumentService.getLength() )
            maCBApplication.SetText( getApplicationUIName( pInfo->maDocumentService ) );
        */
        if( !pInfo->maExportService.isEmpty() )
            m_pCBApplication->SetText( getApplicationUIName( pInfo->maExportService ) );
        else
            m_pCBApplication->SetText( getApplicationUIName( pInfo->maImportService ) );
        m_pEDInterfaceName->SetText( string_decode(pInfo->maInterfaceName) );
        m_pEDExtension->SetText( pInfo->maExtension );
        m_pEDDescription->SetText( string_decode( pInfo->maComment ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
