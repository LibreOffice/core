/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/request.hxx>
#include <sot/storage.hxx>
#include <sot/exchange.hxx>
#include <tools/globname.hxx>
#include <comphelper/mediadescriptor.hxx>
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
    // check the passed Calc document
    DBG_ASSERT( pDocument, "::ScImportExcel - no document" );
    if( !pDocument ) return eERR_INTERN;        // should not happen

    /*  Import all BIFF versions regardless on eFormat, needed for import of
        external cells (file type detection returns Excel4.0). */
    if( (eFormat != EIF_AUTO) && (eFormat != EIF_BIFF_LE4) && (eFormat != EIF_BIFF5) && (eFormat != EIF_BIFF8) )
    {
        DBG_ERRORFILE( "::ScImportExcel - wrong file format specification" );
        return eERR_FORMAT;
    }

    // check the input stream from medium
    SvStream* pMedStrm = rMedium.GetInStream();
    DBG_ASSERT( pMedStrm, "::ScImportExcel - medium without input stream" );
    if( !pMedStrm ) return eERR_OPEN;           // should not happen

#if OSL_DEBUG_LEVEL > 0
    using namespace ::com::sun::star;
    using namespace ::comphelper;

    /*  Environment variable "OOO_OOXBIFFFILTER":
        - "1" = use new OOX filter for import;
        - undef/other = use old sc filter for import (OOX only as file dumper). */
    const sal_Char* pcFileName = ::getenv( "OOO_OOXBIFFFILTER" );
    bool bUseOoxFilter = pcFileName && (*pcFileName == '1') && (*(pcFileName + 1) == 0);
    if( SfxObjectShell* pDocShell = pDocument->GetDocumentShell() ) try
    {
        uno::Reference< lang::XComponent > xComponent( pDocShell->GetModel(), uno::UNO_QUERY_THROW );

        uno::Sequence< beans::NamedValue > aArgSeq( 1 );
        aArgSeq[ 0 ].Name = CREATE_OUSTRING( "UseBiffFilter" );
        aArgSeq[ 0 ].Value <<= bUseOoxFilter;

        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[ 0 ] <<= getProcessServiceFactory();
        aArgs[ 1 ] <<= aArgSeq;
        uno::Reference< document::XImporter > xImporter( ScfApiHelper::CreateInstanceWithArgs(
            CREATE_OUSTRING( "com.sun.star.comp.oox.xls.ExcelBiffFilter" ), aArgs ), uno::UNO_QUERY_THROW );
        xImporter->setTargetDocument( xComponent );

        MediaDescriptor aMediaDesc;
        SfxItemSet* pItemSet = rMedium.GetItemSet();
        if( pItemSet )
        {
            SFX_ITEMSET_ARG( pItemSet, pFileNameItem, SfxStringItem, SID_FILE_NAME, false);
            if( pFileNameItem )
                aMediaDesc[ MediaDescriptor::PROP_URL() ] <<= ::rtl::OUString( pFileNameItem->GetValue() );

            SFX_ITEMSET_ARG( pItemSet, pPasswordItem, SfxStringItem, SID_PASSWORD, false);
            if( pPasswordItem )
                aMediaDesc[ MediaDescriptor::PROP_PASSWORD() ] <<= ::rtl::OUString( pPasswordItem->GetValue() );

            SFX_ITEMSET_ARG( pItemSet, pEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA, false);
            if( pEncryptionDataItem )
                aMediaDesc[ MediaDescriptor::PROP_ENCRYPTIONDATA() ] = pEncryptionDataItem->GetValue();
        }
        aMediaDesc[ MediaDescriptor::PROP_INPUTSTREAM() ] <<= rMedium.GetInputStream();
        aMediaDesc[ MediaDescriptor::PROP_INTERACTIONHANDLER() ] <<= rMedium.GetInteractionHandler();

        // call the filter
        uno::Reference< document::XFilter > xFilter( xImporter, uno::UNO_QUERY_THROW );
        bool bResult = xFilter->filter( aMediaDesc.getAsConstPropertyValueList() );

        // if filter returns false, document is invalid, or dumper has disabled import -> exit here
        if( !bResult )
            return ERRCODE_ABORT;

        // if OOX filter has been used, exit with OK code
        if( bUseOoxFilter )
            return eERR_OK;
    }
    catch( uno::Exception& )
    {
        if( bUseOoxFilter )
            return ERRCODE_ABORT;
        // else ignore exception and import the document with this filter
    }
#endif

    SvStream* pBookStrm = 0;            // The "Book"/"Workbook" stream containing main data.
    XclBiff eBiff = EXC_BIFF_UNKNOWN;   // The BIFF version of the main stream.

    // try to open an OLE storage
    SotStorageRef xRootStrg;
    SotStorageStreamRef xStrgStrm;
    if( SotStorage::IsStorageFile( pMedStrm ) )
    {
        xRootStrg = new SotStorage( pMedStrm, false );
        if( xRootStrg->GetError() )
            xRootStrg = 0;
    }

    // try to open "Book" or "Workbook" stream in OLE storage
    if( xRootStrg.Is() )
    {
        // try to open the "Book" stream
        SotStorageStreamRef xBookStrm = ScfTools::OpenStorageStreamRead( xRootStrg, EXC_STREAM_BOOK );
        XclBiff eBookBiff = xBookStrm.Is() ?  XclImpStream::DetectBiffVersion( *xBookStrm ) : EXC_BIFF_UNKNOWN;

        // try to open the "Workbook" stream
        SotStorageStreamRef xWorkbookStrm = ScfTools::OpenStorageStreamRead( xRootStrg, EXC_STREAM_WORKBOOK );
        XclBiff eWorkbookBiff = xWorkbookStrm.Is() ?  XclImpStream::DetectBiffVersion( *xWorkbookStrm ) : EXC_BIFF_UNKNOWN;

        // decide which stream to use
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

    // no "Book" or "Workbook" stream found, try plain input stream from medium (even for BIFF5+)
    if( !pBookStrm )
    {
        eBiff = XclImpStream::DetectBiffVersion( *pMedStrm );
        if( eBiff != EXC_BIFF_UNKNOWN )
            pBookStrm = pMedStrm;
    }

    // try to import the file
    FltError eRet = eERR_UNKN_BIFF;
    if( pBookStrm )
    {
        pBookStrm->SetBufferSize( 0x8000 );     // still needed?

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
        SvStream* pMedStrm, sal_Bool bBiff8, CharSet eNach )
{
    // try to open an OLE storage
    SotStorageRef xRootStrg = new SotStorage( pMedStrm, false );
    if( xRootStrg->GetError() ) return eERR_OPEN;

    // create BIFF dependent strings
    String aStrmName, aClipName, aClassName;
    if( bBiff8 )
    {
        aStrmName = EXC_STREAM_WORKBOOK;
        aClipName = CREATE_STRING( "Biff8" );
        aClassName = CREATE_STRING( "Microsoft Excel 97-Tabelle" );
    }
    else
    {
        aStrmName = EXC_STREAM_BOOK;
        aClipName = CREATE_STRING( "Biff5" );
        aClassName = CREATE_STRING( "Microsoft Excel 5.0-Tabelle" );
    }

    // open the "Book"/"Workbook" stream
    SotStorageStreamRef xStrgStrm = ScfTools::OpenStorageStreamWrite( xRootStrg, aStrmName );
    if( !xStrgStrm.Is() || xStrgStrm->GetError() ) return eERR_OPEN;

    xStrgStrm->SetBufferSize( 0x8000 );     // still needed?

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
    ExportFormatExcel eFormat, CharSet eNach )
{
    if( eFormat != ExpBiff5 && eFormat != ExpBiff8 )
        return eERR_NI;

    // check the passed Calc document
    DBG_ASSERT( pDocument, "::ScImportExcel - no document" );
    if( !pDocument ) return eERR_INTERN;        // should not happen

    // check the output stream from medium
    SvStream* pMedStrm = rMedium.GetOutStream();
    DBG_ASSERT( pMedStrm, "::ScExportExcel5 - medium without output stream" );
    if( !pMedStrm ) return eERR_OPEN;           // should not happen

    FltError eRet = eERR_UNKN_BIFF;
    if( eFormat == ExpBiff5 || eFormat == ExpBiff8 )
        eRet = lcl_ExportExcelBiff( rMedium, pDocument, pMedStrm, eFormat == ExpBiff8, eNach );

    return eRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
