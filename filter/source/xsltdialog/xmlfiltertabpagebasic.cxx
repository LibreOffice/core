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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"
#include <rtl/ustrbuf.hxx>

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltertabpagebasic.hxx"
#include "xmlfiltertabpagebasic.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfilterhelpids.hrc"

using namespace rtl;

XMLFilterTabPageBasic::XMLFilterTabPageBasic( Window* pParent, ResMgr& rResMgr ) :
    TabPage( pParent, ResId( RID_XML_FILTER_TABPAGE_BASIC, rResMgr ) ),
    maFTFilterName( this, ResId( FT_XML_FILTER_NAME, rResMgr ) ),
    maEDFilterName( this, ResId( ED_XML_FILTER_NAME, rResMgr ) ),
    maFTApplication( this, ResId( FT_XML_APPLICATION, rResMgr ) ),
    maCBApplication( this, ResId( CB_XML_APPLICATION, rResMgr ) ),
    maFTInterfaceName( this, ResId( FT_XML_INTERFACE_NAME, rResMgr ) ),
    maEDInterfaceName( this, ResId( ED_XML_INTERFACE_NAME, rResMgr ) ),
    maFTExtension( this, ResId( FT_XML_EXTENSION, rResMgr ) ),
    maEDExtension( this, ResId( ED_XML_EXTENSION, rResMgr ) ),
    maFTDescription( this, ResId( FT_XML_DESCRIPTION, rResMgr ) ),
    maEDDescription( this, ResId( ED_XML_DESCRIPTION, rResMgr ) )
{
    maCBApplication.SetHelpId( HID_XML_FILTER_APPLICATION );
    maEDDescription.SetHelpId( HID_XML_FILTER_DESCRIPTION );

    FreeResource();

    std::vector< application_info_impl* >& rInfos = getApplicationInfos();
    std::vector< application_info_impl* >::iterator aIter( rInfos.begin() );
    while( aIter != rInfos.end() )
    {
        XubString aEntry( (*aIter++)->maDocumentUIName );
        maCBApplication.InsertEntry( aEntry );
    }
}

XMLFilterTabPageBasic::~XMLFilterTabPageBasic()
{
}

static OUString checkExtensions( const String& rExtensions )
{
    const sal_Unicode* pSource = rExtensions.GetBuffer();
    sal_Int32 nCount = rExtensions.Len();

    String aRet;
    while( nCount-- )
    {
        switch(*pSource)
        {
        case sal_Unicode(','):
            aRet += sal_Unicode(';');
            break;
        case sal_Unicode('.'):
        case sal_Unicode('*'):
            break;
        default:
            aRet += *pSource;
        }

        pSource++;
    }

    return aRet;
}

bool XMLFilterTabPageBasic::FillInfo( filter_info_impl* pInfo )
{
    if( pInfo )
    {
        if( maEDFilterName.GetText().Len() )
            pInfo->maFilterName = maEDFilterName.GetText();

        if( maCBApplication.GetText().Len() )
            pInfo->maDocumentService = maCBApplication.GetText();

        if( maEDInterfaceName.GetText().Len() )
            pInfo->maInterfaceName = maEDInterfaceName.GetText();

        if( maEDExtension.GetText().Len() )
            pInfo->maExtension = checkExtensions( maEDExtension.GetText() );

        pInfo->maComment = string_encode( maEDDescription.GetText() );

        if( pInfo->maDocumentService.getLength() )
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
                aIter++;
            }
        }
    }

    return true;
}

void XMLFilterTabPageBasic::SetInfo(const filter_info_impl* pInfo)
{
    if( pInfo )
    {
        maEDFilterName.SetText( string_decode(pInfo->maFilterName) );
        /*
        if( pInfo->maDocumentService.getLength() )
            maCBApplication.SetText( getApplicationUIName( pInfo->maDocumentService ) );
        */
        if( pInfo->maExportService.getLength() )
            maCBApplication.SetText( getApplicationUIName( pInfo->maExportService ) );
        else
            maCBApplication.SetText( getApplicationUIName( pInfo->maImportService ) );
        maEDInterfaceName.SetText( string_decode(pInfo->maInterfaceName) );
        maEDExtension.SetText( pInfo->maExtension );
        maEDDescription.SetText( string_decode( pInfo->maComment ) );
    }
}
