/*************************************************************************
 *
 *  $RCSfile: excel.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-11 16:39:09 $
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

#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <svtools/saveopt.hxx>
#include <so3/svstor.hxx>
#include <vcl/exchange.hxx>
#include <tools/globname.hxx>
//#include <segmentc.hxx>

// wenn definiert, erzeugt Excel-Import nur Dumps ueber die DBG_TRACE-Funktion
//#define __DUMPER__

#include <filter.hxx>

#include "scerrors.hxx"
#include "root.hxx"
#include "imp_op.hxx"
#include "excimp8.hxx"
#include "exp_op.hxx"


void InitFuncData( BOOL bBiff8 );
void DeInitFuncData();



static const sal_Char*          pWrkbkNameExcel5 = "Book";
static const sal_Char*          pWrkbkNameExcel97 = "Workbook";
const sal_Char*                 pPivotCacheStorageName = "_SX_DB_CUR";
const sal_Char*                 pVBAStorageName = "_VBA_PROJECT_CUR";
const sal_Char*                 pVBASubStorageName = "VBA";


FltError ScImportExcel( SvStream& rStream, ScDocument* pDocument )
{
    ImportExcel             aFilter( rStream, pDocument );
    return aFilter.Read();
}


FltError ScImportExcel( SfxMedium& r, ScDocument* p )
{
    return ScImportExcel( r, p, EIF_AUTO );
}


FltError ScImportExcel( SfxMedium& rMedium, ScDocument* pDocument, const EXCIMPFORMAT eFormat )
{
    DBG_ASSERT( &rMedium != NULL,
        "--ScImportExcel(): Wer hat mich da falsch gerufen?!!!" );

    FltError                eRet;

    SvStorage* pStorage = rMedium.GetStorage();
    if( pStorage )
    {// OLE2-Datei
        enum BiffType   { BT0, BT5, BT8 };
        SvStorage*          pPivotCacheStorage = NULL;
        const String        aPvCchStrgNm( String::CreateFromAscii( pPivotCacheStorageName ) );

        BiffType            eBT = BT0;

        String              aWrkbkName( String::CreateFromAscii( pWrkbkNameExcel97 ) ); // -> Biff8 hoeherwertiger!
        const BOOL          bContBiff8 = pStorage->IsContained( aWrkbkName ) && pStorage->IsStream( aWrkbkName );

        aWrkbkName.AssignAscii( pWrkbkNameExcel5 );
        const BOOL          bContBiff5 = pStorage->IsContained( aWrkbkName ) && pStorage->IsStream( aWrkbkName );

        if( !bContBiff8 && !bContBiff5 )
            return eERR_UNKN_BIFF;

        switch( eFormat )
        {
            case EIF_AUTO:
                if( bContBiff8 )
                {
                    eBT = BT8;
                    aWrkbkName.AssignAscii( pWrkbkNameExcel97 );
                }
                else
                {
                    eBT = BT5;
                    DBG_ASSERT( bContBiff5, "*ScImportExcel(): Falscher Zustand" );
                }
                break;
            case EIF_BIFF5:
                if( bContBiff5 )
                    eBT = BT5;
                else
                    return eERR_FORMAT;         // Error-Code richtig?
                break;
            case EIF_BIFF8:
                if( bContBiff8 )
                {
                    eBT = BT8;
                    aWrkbkName.AssignAscii( pWrkbkNameExcel97 );
                    pPivotCacheStorage = pStorage->OpenStorage( aPvCchStrgNm, STREAM_STD_READ );
                }
                else
                    return eERR_FORMAT;         // Error-Code richtig?
                break;
            case EIF_BIFF_LE4:
                return eERR_FORMAT;             // Error-Code richtig?
                break;
#ifdef DEBUG
            default:
                DBG_ERROR( "*ScImportExcel(): Format vergessen!" );
#endif
        }

        SvStorageStreamRef  xStStream = pStorage->OpenStream( aWrkbkName,
                                    STREAM_READ | STREAM_SHARE_DENYALL );

        xStStream->SetBufferSize( 32768 );

        if( eBT == BT8 )
        { // Tuerk-Test
            SvStream& r = *xStStream;
            r.SeekRel( 4 );
            UINT16  nVersion;
            r >> nVersion;
            if( nVersion == 0x0500 )
                eBT = BT5;      // getuerkt!!!
            r.Seek( 0 );
        }

        ImportExcel*        pFilter;

        if( eBT == BT5 )
            pFilter = new ImportExcel( *xStStream, pDocument );
        else
            pFilter = new ImportExcel8( pStorage, *xStStream, pDocument, pPivotCacheStorage );

        eRet = pFilter->Read();

        xStStream->SetBufferSize( 0 );

        delete pFilter;
    }
    else if( eFormat == EIF_AUTO || eFormat == EIF_BIFF_LE4 )
    {// normale Datei
        SvStream*           pStream = rMedium.GetInStream();

        if( pStream == NULL )
            return eERR_OPEN;

        pStream->Seek( 0UL );

        pStream->SetBufferSize( 32768 );

        ImportExcel         aFilter( *pStream, pDocument );
        eRet = aFilter.Read();

        pStream->SetBufferSize( 0 );
    }
    else
        return eERR_FORMAT;                     // Error-Code richtig?

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
    const sal_Char*             pWrkBook;
    const sal_Char*             pClipboard;
    const sal_Char*             pClassName;

    if( bBiff8 )
    {
        pWrkBook = pWrkbkNameExcel97;
        pClipboard = "Biff8";
        pClassName = "Microsoft Excel 97-Tabelle";
    }
    else
    {
        pWrkBook = pWrkbkNameExcel5;
        pClipboard = "Biff5";
        pClassName = "Microsoft Excel 5.0-Tabelle";
    }

    FltError                eRet = eERR_NI;

    if( &rOutMedium == NULL )
        return eERR_OPEN;

    SvStorage* pStorage = rOutMedium.GetStorage();
    if( pStorage )
    {// OLE2-Datei
        SvStorageStreamRef  xStStream =
            pStorage->OpenStream( _STRING( pWrkBook ), STREAM_READWRITE | STREAM_TRUNC );

        xStStream->SetBufferSize( 32768 );

        InitFuncData( bBiff8 );

        if ( bBiff8 )
        {
            SvtSaveOptions aSaveOpt;
            BOOL            bStoreRel = rOutMedium.IsRemote()?
                                            aSaveOpt.IsSaveRelINet() :
                                            aSaveOpt.IsSaveRelFSys();
            ExportBiff8     aFilter( *pStorage, *xStStream, pDocument, eNach, bStoreRel );
            eRet = aFilter.Write();
        }
        else
        {
            ExportBiff5     aFilter( *pStorage, *xStStream, pDocument, eNach );
            eRet = aFilter.Write();
        }


        if( eRet == eERR_RNGOVRFLW )
            eRet = SCWARN_EXPORT_MAXROW;

        DeInitFuncData();

        xStStream->SetBufferSize( 0 );

        // CompObj schreiben
        SvGlobalName        aName( 0x00020810, 0x0000, 0x0000, 0xc0, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x46 );
        UINT32              nClip = Exchange::RegisterFormatName( _STRING( pClipboard ) );
        pStorage->SetClass( aName, nClip, _STRING( pClassName ) );
        xStStream->Commit();
    }
    else
        eRet = eERR_OPEN;

    return eRet;
}



