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

XMLFilterTabPageXSLT::XMLFilterTabPageXSLT( Window* pParent, ResMgr& rResMgr, const Reference< XMultiServiceFactory >& rxMSF ) :
    TabPage( pParent, ResId( RID_XML_FILTER_TABPAGE_XSLT, rResMgr ) ),
    
    maFTDocType( this, ResId( FT_XML_DOCTYPE, rResMgr ) ),
    maEDDocType( this, ResId( ED_XML_DOCTYPE, rResMgr ) ),

    maFTDTDSchema( this, ResId( FT_XML_DTD_SCHEMA, rResMgr ) ),
    maEDDTDSchema( this, ResId( ED_XML_DTD_SCHEMA, rResMgr ), INET_PROT_FILE ),
    maPBDTDSchemaBrowse( this, ResId( ED_XML_DTD_SCHEMA_BROWSE, rResMgr ) ),

    maFTExportXSLT( this, ResId( FT_XML_EXPORT_XSLT, rResMgr ) ),
    maEDExportXSLT( this, ResId( ED_XML_EXPORT_XSLT, rResMgr ), INET_PROT_FILE ),
    maPBExprotXSLT( this, ResId( PB_XML_EXPORT_XSLT_BROWSE, rResMgr ) ),

    maFTImportXSLT( this, ResId( FT_XML_IMPORT_XSLT, rResMgr ) ),
    maEDImportXSLT( this, ResId( ED_XML_IMPORT_XSLT, rResMgr ), INET_PROT_FILE ),
    maPBImportXSLT( this, ResId( PB_XML_IMPORT_XSLT_BROWSE, rResMgr ) ),

    maFTImportTemplate( this, ResId( FT_XML_IMPORT_TEMPLATE, rResMgr ) ),
    maEDImportTemplate( this, ResId( ED_XML_IMPORT_TEMPLATE, rResMgr ), INET_PROT_FILE ),
    maPBImportTemplate( this, ResId( PB_XML_IMPORT_TEMPLATE_BROWSE, rResMgr ) ),

    sHTTPSchema( RTL_CONSTASCII_USTRINGPARAM( "http://" ) ),
    sSHTTPSchema( RTL_CONSTASCII_USTRINGPARAM( "shttp://" ) ),
    sFILESchema( RTL_CONSTASCII_USTRINGPARAM( "file://" ) ),
    sFTPSchema( RTL_CONSTASCII_USTRINGPARAM( "ftp://" ) ),
    sInstPath( RTL_CONSTASCII_USTRINGPARAM( "$(prog)/" ) )
{
    FreeResource();

    try
    {
        Reference< XConfigManager > xCfgMgr( rxMSF->createInstance(OUString::createFromAscii("com.sun.star.config.SpecialConfigManager")), UNO_QUERY );
        if( xCfgMgr.is() )
            sInstPath = xCfgMgr->substituteVariables( sInstPath );
    }
    catch(Exception&)
    {
        DBG_ERROR( "XMLFilterTabPageXSLT::XMLFilterTabPageXSLT exception catched!" );
    }

    maPBDTDSchemaBrowse.SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
    maPBExprotXSLT.SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
    maPBImportXSLT.SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );
    maPBImportTemplate.SetClickHdl( LINK ( this, XMLFilterTabPageXSLT, ClickBrowseHdl_Impl ) );

    maEDDTDSchema.SetHelpId( HID_XML_FILTER_DTD );
    maEDExportXSLT.SetHelpId( HID_XML_FILTER_EXPORT_XSLT );
    maEDImportXSLT.SetHelpId( HID_XML_FILTER_IMPORT_XSLT );
    maEDImportTemplate.SetHelpId( HID_XML_FILTER_IMPORT_TEMPLATE );
}

XMLFilterTabPageXSLT::~XMLFilterTabPageXSLT()
{
}

bool XMLFilterTabPageXSLT::FillInfo( filter_info_impl* pInfo )
{
    if( pInfo )
    {
        pInfo->maDocType = maEDDocType.GetText();
        pInfo->maDTD = GetURL( maEDDTDSchema );
        pInfo->maExportXSLT = GetURL( maEDExportXSLT );
        pInfo->maImportXSLT = GetURL( maEDImportXSLT );
        pInfo->maImportTemplate = GetURL( maEDImportTemplate );
    }

    return true;
}

void XMLFilterTabPageXSLT::SetInfo(const filter_info_impl* pInfo)
{
    if( pInfo )
    {
        maEDDocType.SetText( pInfo->maDocType );

        SetURL( maEDDTDSchema, pInfo->maDTD );
        SetURL( maEDExportXSLT, pInfo->maExportXSLT );
        SetURL( maEDImportXSLT, pInfo->maImportXSLT );
        SetURL( maEDImportTemplate, pInfo->maImportTemplate );
    }
}

void XMLFilterTabPageXSLT::SetURL( SvtURLBox& rURLBox, const OUString& rURL )
{
    OUString aPath;

    if( rURL.matchIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM( "file://" ) ) ) )
    {
        osl::FileBase::getSystemPathFromFileURL( rURL, aPath );

        rURLBox.SetBaseURL( rURL );
        rURLBox.SetText( aPath );
    }
    else if( rURL.matchIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM( "http://" ) ) ) ||
             rURL.matchIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM( "shttp://" ) ) ) ||
             rURL.matchIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM( "ftp://" ) ) ) )
    {
        rURLBox.SetBaseURL( rURL );
        rURLBox.SetText( rURL );
    }
    else if( rURL.getLength() )
    {
        rtl::OUString aURL( rURL );
        aURL = URIHelper::SmartRel2Abs( sInstPath, aURL, Link(), false );
        osl::FileBase::getSystemPathFromFileURL( aURL, aPath );

        rURLBox.SetBaseURL( aURL );
        rURLBox.SetText( aPath );
    }
    else
    {
        rURLBox.SetBaseURL( sInstPath );
        String aEmpty;
        rURLBox.SetText( aEmpty );
    }
}

OUString XMLFilterTabPageXSLT::GetURL( SvtURLBox& rURLBox )
{
    OUString aURL;
    OUString aStrPath ( rURLBox.GetText() );
    if( aStrPath.matchIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM( "http://" ) ) ) ||
        aStrPath.matchIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM( "shttp://" ) ) ) ||
        aStrPath.matchIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM( "ftp://" ) ) ) )
    {
        return aStrPath;
    }
    else
    {
        const String aBaseURL ( rURLBox.GetBaseURL() );
        osl::FileBase::getFileURLFromSystemPath( aStrPath, aURL );
    }

    return aURL;
}

IMPL_LINK ( XMLFilterTabPageXSLT, ClickBrowseHdl_Impl, PushButton *, pButton )
{
    SvtURLBox* pURLBox;

    if( pButton == &maPBDTDSchemaBrowse )
    {
        pURLBox = &maEDDTDSchema;
    }
    else if( pButton == &maPBExprotXSLT )
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

