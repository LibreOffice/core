/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/request.hxx>
#include <sot/storage.hxx>
#include <sot/exchange.hxx>
#include <tools/globname.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include "scitems.hxx"
#include <svl/stritem.hxx>
#include "filter.hxx"
#include "document.hxx"
#include "xistream.hxx"

#include "scerrors.hxx"
#include "root.hxx"
#include "imp_op.hxx"
#include "excimp8.hxx"
#include "exp_op.hxx"


FltError ScFormatFilterPluginImpl::ScImportExcel( SfxMedium& rMedium, ScDocument* pDocument, const EXCIMPFORMAT eFormat )
{
    
    OSL_ENSURE( pDocument, "::ScImportExcel - no document" );
    if( !pDocument ) return eERR_INTERN;        

    /*  Import all BIFF versions regardless on eFormat, needed for import of
        external cells (file type detection returns Excel4.0). */
    if( (eFormat != EIF_AUTO) && (eFormat != EIF_BIFF_LE4) && (eFormat != EIF_BIFF5) && (eFormat != EIF_BIFF8) )
    {
        OSL_FAIL( "::ScImportExcel - wrong file format specification" );
        return eERR_FORMAT;
    }

    
    SvStream* pMedStrm = rMedium.GetInStream();
    OSL_ENSURE( pMedStrm, "::ScImportExcel - medium without input stream" );
    if( !pMedStrm ) return eERR_OPEN;           

    SvStream* pBookStrm = 0;            
    XclBiff eBiff = EXC_BIFF_UNKNOWN;   

    
    SotStorageRef xRootStrg;
    SotStorageStreamRef xStrgStrm;
    if( SotStorage::IsStorageFile( pMedStrm ) )
    {
        xRootStrg = new SotStorage( pMedStrm, false );
        if( xRootStrg->GetError() )
            xRootStrg = 0;
    }

    
    if( xRootStrg.Is() )
    {
        
        SotStorageStreamRef xBookStrm = ScfTools::OpenStorageStreamRead( xRootStrg, EXC_STREAM_BOOK );
        XclBiff eBookBiff = xBookStrm.Is() ?  XclImpStream::DetectBiffVersion( *xBookStrm ) : EXC_BIFF_UNKNOWN;

        
        SotStorageStreamRef xWorkbookStrm = ScfTools::OpenStorageStreamRead( xRootStrg, EXC_STREAM_WORKBOOK );
        XclBiff eWorkbookBiff = xWorkbookStrm.Is() ?  XclImpStream::DetectBiffVersion( *xWorkbookStrm ) : EXC_BIFF_UNKNOWN;

        
        if( (eWorkbookBiff != EXC_BIFF_UNKNOWN) && ((eBookBiff == EXC_BIFF_UNKNOWN) || (eWorkbookBiff > eBookBiff)) )
        {
            /*  Only "Workbook" stream exists; or both streams exist,
                and "Workbook" has higher BIFF version than "Book" stream. */
            xStrgStrm = xWorkbookStrm;
            eBiff = eWorkbookBiff;
        }
        else if( eBookBiff != EXC_BIFF_UNKNOWN )
        {
            /*  Only "Book" stream exists; or both streams exist,
                and "Book" has higher BIFF version than "Workbook" stream. */
            xStrgStrm = xBookStrm;
            eBiff = eBookBiff;
        }

        pBookStrm = xStrgStrm;
    }

    
    if( !pBookStrm )
    {
        eBiff = XclImpStream::DetectBiffVersion( *pMedStrm );
        if( eBiff != EXC_BIFF_UNKNOWN )
            pBookStrm = pMedStrm;
    }

    
    FltError eRet = eERR_UNKN_BIFF;
    if( pBookStrm )
    {
        pBookStrm->SetBufferSize( 0x8000 );     

        XclImpRootData aImpData( eBiff, rMedium, xRootStrg, *pDocument, RTL_TEXTENCODING_MS_1252 );
        ::std::auto_ptr< ImportExcel > xFilter;
        switch( eBiff )
        {
            case EXC_BIFF2:
            case EXC_BIFF3:
            case EXC_BIFF4:
            case EXC_BIFF5:
                xFilter.reset( new ImportExcel( aImpData, *pBookStrm ) );
            break;
            case EXC_BIFF8:
                xFilter.reset( new ImportExcel8( aImpData, *pBookStrm ) );
            break;
            default:    DBG_ERROR_BIFF();
        }

        eRet = xFilter.get() ? xFilter->Read() : eERR_INTERN;
    }

    return eRet;
}


static FltError lcl_ExportExcelBiff( SfxMedium& rMedium, ScDocument *pDocument,
        SvStream* pMedStrm, sal_Bool bBiff8, rtl_TextEncoding eNach )
{
    
    SotStorageRef xRootStrg = new SotStorage( pMedStrm, false );
    if( xRootStrg->GetError() ) return eERR_OPEN;

    
    OUString aStrmName, aClipName, aClassName;
    if( bBiff8 )
    {
        aStrmName = EXC_STREAM_WORKBOOK;
        aClipName = "Biff8";
        aClassName = "Microsoft Excel 97-Tabelle";
    }
    else
    {
        aStrmName = EXC_STREAM_BOOK;
        aClipName = "Biff5";
        aClassName = "Microsoft Excel 5.0-Tabelle";
    }

    
    SotStorageStreamRef xStrgStrm = ScfTools::OpenStorageStreamWrite( xRootStrg, aStrmName );
    if( !xStrgStrm.Is() || xStrgStrm->GetError() ) return eERR_OPEN;

    xStrgStrm->SetBufferSize( 0x8000 );     

    FltError eRet = eERR_UNKN_BIFF;
    XclExpRootData aExpData( bBiff8 ? EXC_BIFF8 : EXC_BIFF5, rMedium, xRootStrg, *pDocument, eNach );
    if ( bBiff8 )
    {
        ExportBiff8 aFilter( aExpData, *xStrgStrm );
        eRet = aFilter.Write();
    }
    else
    {
        ExportBiff5 aFilter( aExpData, *xStrgStrm );
        eRet = aFilter.Write();
    }

    if( eRet == eERR_RNGOVRFLW )
        eRet = SCWARN_EXPORT_MAXROW;

    SvGlobalName aGlobName( 0x00020810, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 );
    sal_uInt32 nClip = SotExchange::RegisterFormatName( aClipName );
    xRootStrg->SetClass( aGlobName, nClip, aClassName );

    xStrgStrm->Commit();
    xRootStrg->Commit();

    return eRet;
}

FltError ScFormatFilterPluginImpl::ScExportExcel5( SfxMedium& rMedium, ScDocument *pDocument,
    ExportFormatExcel eFormat, rtl_TextEncoding eNach )
{
    if( eFormat != ExpBiff5 && eFormat != ExpBiff8 )
        return eERR_NI;

    
    OSL_ENSURE( pDocument, "::ScImportExcel - no document" );
    if( !pDocument ) return eERR_INTERN;        

    
    SvStream* pMedStrm = rMedium.GetOutStream();
    OSL_ENSURE( pMedStrm, "::ScExportExcel5 - medium without output stream" );
    if( !pMedStrm ) return eERR_OPEN;           

    FltError eRet = eERR_UNKN_BIFF;
    if( eFormat == ExpBiff5 || eFormat == ExpBiff8 )
        eRet = lcl_ExportExcelBiff( rMedium, pDocument, pMedStrm, eFormat == ExpBiff8, eNach );

    return eRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
