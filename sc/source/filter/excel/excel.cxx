/*************************************************************************
 *
 *  $RCSfile: excel.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:50:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// ============================================================================

#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif

#ifndef SC_FILTER_HXX
#include "filter.hxx"
#endif

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif

#include "scerrors.hxx"
#include "root.hxx"
#include "imp_op.hxx"
#include "excimp8.hxx"
#include "exp_op.hxx"

void InitFuncData( BOOL bBiff8 );
void DeInitFuncData();


FltError ScImportExcel( SfxMedium& r, ScDocument* p )
{
    return ScImportExcel( r, p, EIF_AUTO );
}


FltError ScImportExcel( SfxMedium& rMedium, ScDocument* pDocument, const EXCIMPFORMAT eFormat )
{
    FltError eRet = eERR_OK;
    SvStorage* pStorage = rMedium.GetStorage();

    // OLE2 compound file
    if( pStorage )
    {
        // *** look for contained streams ***

        const String aStreamName5( EXC_STREAM_BOOK );
        sal_Bool bHasBook = pStorage->IsContained( aStreamName5 ) && pStorage->IsStream( aStreamName5 );

        const String aStreamName8( EXC_STREAM_WORKBOOK );
        sal_Bool bHasWorkbook = pStorage->IsContained( aStreamName8 ) && pStorage->IsStream( aStreamName8 );

        // *** handle user-defined filter selection ***

        // comparing the stream names, regardless of the stream contents
        switch( eFormat )
        {
            case EIF_AUTO:
                // nothing to do
            break;
            case EIF_BIFF5:
                bHasWorkbook = sal_False;
            break;
            case EIF_BIFF8:
                bHasBook = sal_False;
            break;
            case EIF_BIFF_LE4:
// keep auto-detection for import of external cells (file type detection returns Excel4.0)
//                eRet = eERR_FORMAT;             //!! correct error code?
            break;
            default:
                eRet = eERR_FORMAT;             //!! correct error code?
                DBG_ERRORFILE( "ScImportExcel - wrong file format specification" );
        }

        // *** find BIFF version and stream name ***

        XclBiff eDetectedBiff = xlBiffUnknown;
        const String* pStreamName = NULL;

        if( eRet == eERR_OK )
        {
            // BIFF8 is first class
            if( bHasWorkbook )
            {
                eDetectedBiff = xlBiff8;
                pStreamName = &aStreamName8;
            }
            else if( bHasBook )
            {
                eDetectedBiff = xlBiff5;
                pStreamName = &aStreamName5;
            }
            else
                eRet = eERR_UNKN_BIFF;
        }

        if( (eRet == eERR_OK) && pStreamName )
        {
            SvStorageStreamRef xStream = ScfTools::OpenStorageStreamRead( pStorage, *pStreamName );
            DBG_ASSERT( xStream.Is(), "ScImportExcel - missing stream" );
            xStream->SetBufferSize( 32768 );

            // *** special handling for wrong BIFF versions in stream ***

            XclBiff eBiff = XclImpStream::DetectBiffVersion( *xStream );

            // look for BIFF5/7 stream in "Workbook"
            if( bHasWorkbook )
            {
                if( (eBiff == xlBiff5) || (eBiff == xlBiff7) )
                    eDetectedBiff = xlBiff5;
                else if( eBiff != xlBiff8 )
                    eDetectedBiff = xlBiffUnknown;
            }
            // look for BIFF8 stream in "Book"
            else if( bHasBook )
            {
                if( eBiff == xlBiff8 )
                    eDetectedBiff = xlBiff8;
                else if( (eBiff != xlBiff5) && (eBiff != xlBiff7) )
                    eDetectedBiff = xlBiffUnknown;
            }

            // *** and Go! ***

            if( eRet == eERR_OK )
            {
                ImportExcel* pFilter = NULL;

                if( eDetectedBiff == xlBiff5 )
                    pFilter = new ImportExcel( rMedium, *xStream, eBiff, pDocument );
                else if( eDetectedBiff == xlBiff8 )
                    pFilter = new ImportExcel8( rMedium, *xStream, eBiff, pDocument );

                if( pFilter )
                    eRet = pFilter->Read();
                else
                {
                    DBG_ERRORFILE( "ScImportExcel - not imported, unknown error" );
                    eRet = eERR_UNKN_BIFF;
                }
                delete pFilter;
            }

            xStream->SetBufferSize( 0 );
        }
    }

    // no OLE2 storage - simple stream
    else if( (eFormat == EIF_AUTO) || (eFormat == EIF_BIFF_LE4) )
    {
        SvStream* pStream = rMedium.GetInStream();
        if( pStream )
        {
            pStream->Seek( 0UL );
            pStream->SetBufferSize( 32768 );

            XclBiff eBiff = XclImpStream::DetectBiffVersion( *pStream );

            ImportExcel aFilter( rMedium, *pStream, eBiff, pDocument );
            eRet = aFilter.Read();

            pStream->SetBufferSize( 0 );
        }
        else
            eRet = eERR_OPEN;
    }

    // else invalid input
    else
        eRet = eERR_FORMAT;             //!! correct error code?

    return eRet;
}




FltError ScExportExcel234( SvStream &aStream, ScDocument *pDoc,
    ExportFormatExcel eFormat, CharSet eNach )
{
    FltError                eRet = eERR_NI;
    return eRet;
}


FltError ScExportExcel5( SfxMedium &rOutMedium, ScDocument *pDocument,
    const BOOL bBiff8, CharSet eNach )
{
    String                      aWrkBook;
    const sal_Char*             pClipboard;
    const sal_Char*             pClassName;

    if( bBiff8 )
    {
        aWrkBook = EXC_STREAM_WORKBOOK;
        pClipboard = "Biff8";
        pClassName = "Microsoft Excel 97-Tabelle";
    }
    else
    {
        aWrkBook = EXC_STREAM_BOOK;
        pClipboard = "Biff5";
        pClassName = "Microsoft Excel 5.0-Tabelle";
    }

    FltError                eRet = eERR_NI;

    if( &rOutMedium != NULL )
    {
        SvStorage* pStorage = rOutMedium.GetStorage();
        if( pStorage )
        {// OLE2-Datei
            SvStorageStreamRef xStStream = ScfTools::OpenStorageStreamWrite( pStorage, aWrkBook );

            xStStream->SetBufferSize( 32768 );

            InitFuncData( bBiff8 );

            SvtSaveOptions aSaveOpt;
            bool bRelUrl = TRUE == (rOutMedium.IsRemote() ? aSaveOpt.IsSaveRelINet() : aSaveOpt.IsSaveRelFSys());

            if ( bBiff8 )
            {
                ExportBiff8 aFilter( rOutMedium, *xStStream, xlBiff8, pDocument, eNach, bRelUrl );
                eRet = aFilter.Write();
            }
            else
            {
                ExportBiff5 aFilter( rOutMedium, *xStStream, xlBiff5, pDocument, eNach, bRelUrl );
                eRet = aFilter.Write();
            }


            if( eRet == eERR_RNGOVRFLW )
                eRet = SCWARN_EXPORT_MAXROW;

            DeInitFuncData();

            xStStream->SetBufferSize( 0 );

            // CompObj schreiben
            SvGlobalName        aName( 0x00020810, 0x0000, 0x0000, 0xc0, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x46 );
            UINT32              nClip = SotExchange::RegisterFormatName( _STRING( pClipboard ) );
            pStorage->SetClass( aName, nClip, _STRING( pClassName ) );
            xStStream->Commit();
        }
        else
            eRet = eERR_OPEN;
    }
    else
        eRet = eERR_OPEN;

    return eRet;
}



