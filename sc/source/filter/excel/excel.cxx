/*************************************************************************
 *
 *  $RCSfile: excel.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-09 15:00:11 $
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
#include <sot/storage.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
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


FltError ScImportExcel( SfxMedium& r, ScDocument* p )
{
    return ScImportExcel( r, p, EIF_AUTO );
}


FltError ScImportExcel( SfxMedium& rMedium, ScDocument* pDocument, const EXCIMPFORMAT eFormat )
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

    SvStream* pBookStrm = 0;            // The "Book"/"Workbook" stream containing main data.
    XclBiff eBiff = xlBiffUnknown;      // The BIFF version of the main stream.

    // try to open an OLE storage
    SotStorageRef xRootStrg;
    SotStorageStreamRef xStrgStrm;
    if( SotStorage::IsStorageFile( pMedStrm ) )
    {
        xRootStrg = new SotStorage( pMedStrm, FALSE );
        if( xRootStrg->GetError() )
            xRootStrg = 0;
    }

    // try to open "Book" or "Workbook" stream in OLE storage
    if( xRootStrg.Is() )
    {
        // try to open the "Book" stream
        SotStorageStreamRef xBookStrm5 = ScfTools::OpenStorageStreamRead( xRootStrg, EXC_STREAM_BOOK );
        XclBiff eBookStrm5Biff = xBookStrm5.Is() ?  XclImpStream::DetectBiffVersion( *xBookStrm5 ) : xlBiffUnknown;

        // try to open the "Workbook" stream
        SotStorageStreamRef xBookStrm8 = ScfTools::OpenStorageStreamRead( xRootStrg, EXC_STREAM_WORKBOOK );
        XclBiff eBookStrm8Biff = xBookStrm8.Is() ?  XclImpStream::DetectBiffVersion( *xBookStrm8 ) : xlBiffUnknown;

        // decide which stream to use
        if( (eBookStrm8Biff != xlBiffUnknown) && ((eBookStrm5Biff == xlBiffUnknown) || (eBookStrm8Biff > eBookStrm5Biff)) )
        {
            /*  Only "Workbook" stream exists; or both streams exist,
                and "Workbook" has higher BIFF version than "Book" stream. */
            xStrgStrm = xBookStrm8;
            eBiff = eBookStrm8Biff;
        }
        else if( eBookStrm5Biff != xlBiffUnknown )
        {
            /*  Only "Book" stream exists; or both streams exist,
                and "Book" has higher BIFF version than "Workbook" stream. */
            xStrgStrm = xBookStrm5;
            eBiff = eBookStrm5Biff;
        }

        pBookStrm = xStrgStrm;
    }

    // no "Book" or "Workbook" stream found, try plain input stream from medium (even for BIFF5+)
    if( !pBookStrm )
    {
        eBiff = XclImpStream::DetectBiffVersion( *pMedStrm );
        if( eBiff != xlBiffUnknown )
            pBookStrm = pMedStrm;
    }

    // try to import the file
    FltError eRet = eERR_UNKN_BIFF;
    if( pBookStrm )
    {
        pBookStrm->SetBufferSize( 0x8000 );     // still needed?

        XclImpRootData aImpData( eBiff, rMedium, xRootStrg, *pBookStrm, *pDocument, RTL_TEXTENCODING_MS_1252 );
        ::std::auto_ptr< ImportExcel > xFilter;
        switch( eBiff )
        {
            case xlBiff2:
            case xlBiff3:
            case xlBiff4:
            case xlBiff5:
            case xlBiff7:
                xFilter.reset( new ImportExcel( aImpData ) );
            break;
            case xlBiff8:
                xFilter.reset( new ImportExcel8( aImpData ) );
            break;
            default:    DBG_ERROR_BIFF();
        }

        eRet = xFilter.get() ? xFilter->Read() : eERR_INTERN;
    }

    return eRet;
}




FltError ScExportExcel234( SvStream &aStream, ScDocument *pDoc,
    ExportFormatExcel eFormat, CharSet eNach )
{
    FltError                eRet = eERR_NI;
    return eRet;
}


FltError ScExportExcel5( SfxMedium& rMedium, ScDocument *pDocument,
    const BOOL bBiff8, CharSet eNach )
{
    // check the passed Calc document
    DBG_ASSERT( pDocument, "::ScImportExcel - no document" );
    if( !pDocument ) return eERR_INTERN;        // should not happen

    // check the output stream from medium
    SvStream* pMedStrm = rMedium.GetOutStream();
    DBG_ASSERT( pMedStrm, "::ScExportExcel5 - medium without output stream" );
    if( !pMedStrm ) return eERR_OPEN;           // should not happen

    // try to open an OLE storage
    SotStorageRef xRootStrg = new SotStorage( pMedStrm, FALSE );
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
    XclExpRootData aExpData( bBiff8 ? xlBiff8 : xlBiff5, rMedium, xRootStrg, *xStrgStrm, *pDocument, eNach );
    if ( bBiff8 )
    {
        ExportBiff8 aFilter( aExpData );
        eRet = aFilter.Write();
    }
    else
    {
        ExportBiff5 aFilter( aExpData );
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



