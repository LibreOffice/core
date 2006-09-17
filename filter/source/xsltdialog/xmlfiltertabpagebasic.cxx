/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfiltertabpagebasic.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:49:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltertabpagebasic.hxx"
#include "xmlfiltertabpagebasic.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfilterhelpids.hrc"

using namespace rtl;

XMLFilterTabPageBasic::XMLFilterTabPageBasic( Window* pParent, ResMgr& rResMgr ) :
    TabPage( pParent, ResId( RID_XML_FILTER_TABPAGE_BASIC, &rResMgr ) ),
    maFTFilterName( this, ResId( FT_XML_FILTER_NAME ) ),
    maEDFilterName( this, ResId( ED_XML_FILTER_NAME ) ),
    maFTApplication( this, ResId( FT_XML_APPLICATION ) ),
    maCBApplication( this, ResId( CB_XML_APPLICATION ) ),
    maFTInterfaceName( this, ResId( FT_XML_INTERFACE_NAME ) ),
    maEDInterfaceName( this, ResId( ED_XML_INTERFACE_NAME ) ),
    maFTExtension( this, ResId( FT_XML_EXTENSION ) ),
    maEDExtension( this, ResId( ED_XML_EXTENSION ) ),
    maFTDescription( this, ResId( FT_XML_DESCRIPTION ) ),
    maEDDescription( this, ResId( ED_XML_DESCRIPTION ) )
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
            maCBApplication.SetText( getApplicationUIName( pInfo->maImportService ) );
        else
            maCBApplication.SetText( getApplicationUIName( pInfo->maExportService ) );
        maEDInterfaceName.SetText( string_decode(pInfo->maInterfaceName) );
        maEDExtension.SetText( pInfo->maExtension );
        maEDDescription.SetText( string_decode( pInfo->maComment ) );
    }
}
