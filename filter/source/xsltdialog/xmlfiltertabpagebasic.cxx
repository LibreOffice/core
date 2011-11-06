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
