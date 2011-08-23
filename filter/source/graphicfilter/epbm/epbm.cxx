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

#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/msgbox.hxx>
#include <svl/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>
#include "strings.hrc"
#include "dlgepbm.hrc"
#include "dlgepbm.hxx"

//============================ PBMWriter ==================================

class PBMWriter {

private:

    SvStream*			mpOStm; 			// Die auszugebende PBM-Datei
    USHORT				mpOStmOldModus;

    BOOL				mbStatus;
    sal_Int32			mnMode;				// 0 -> raw, 1-> ascii
    BitmapReadAccess*	mpAcc;
    ULONG				mnWidth, mnHeight;	// Bildausmass in Pixeln

    BOOL				ImplWriteHeader();
    void				ImplWriteBody();
    void				ImplWriteNumber( sal_Int32 );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:
                        PBMWriter();
                        ~PBMWriter();

    BOOL				WritePBM( const Graphic& rGraphic, SvStream& rPBM, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von PBMWriter ==============================

PBMWriter::PBMWriter() :
    mbStatus	( TRUE ),
    mpAcc		( NULL )
{
}

// ------------------------------------------------------------------------

PBMWriter::~PBMWriter()
{
}

// ------------------------------------------------------------------------

BOOL PBMWriter::WritePBM( const Graphic& rGraphic, SvStream& rPBM, FilterConfigItem* pFilterConfigItem )
{

    mpOStm = &rPBM;

    if ( pFilterConfigItem )
    {
        mnMode = pFilterConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "FileFormat" ) ), 0 );

        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            rtl::OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    BitmapEx	aBmpEx( rGraphic.GetBitmapEx() );
    Bitmap		aBmp = aBmpEx.GetBitmap();
    aBmp.Convert( BMP_CONVERSION_1BIT_THRESHOLD );

    mpOStmOldModus = mpOStm->GetNumberFormatInt();
    mpOStm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    mpAcc = aBmp.AcquireReadAccess();
    if( mpAcc )
    {
        if ( ImplWriteHeader() )
            ImplWriteBody();

        aBmp.ReleaseAccess( mpAcc );
    }
    else
        mbStatus = FALSE;

    mpOStm->SetNumberFormatInt( mpOStmOldModus );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}

// ------------------------------------------------------------------------

BOOL PBMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight )
    {
        if ( mnMode == 0 )
            *mpOStm << "P4\x0a";
        else
            *mpOStm << "P1\x0a";

        ImplWriteNumber( mnWidth );
        *mpOStm << (BYTE)32;
        ImplWriteNumber( mnHeight );
        *mpOStm << (BYTE)10;
    }
    else mbStatus = FALSE;
    return mbStatus;
}

// ------------------------------------------------------------------------

void PBMWriter::ImplWriteBody()
{
    if ( mnMode == 0 )
    {
        BYTE	nBYTE = 0;
        for ( ULONG y = 0; y < mnHeight; y++ )
        {
            ULONG x;
            for ( x = 0; x < mnWidth; x++ )
            {
                nBYTE <<= 1;
                if (!(mpAcc->GetPixel( y, x ) & 1 ) )
                    nBYTE++;
                if ( ( x & 7 ) == 7 )
                    *mpOStm << nBYTE;
            }
            if ( ( x & 7 ) != 0 )
                *mpOStm << (BYTE)( nBYTE << ( ( x ^ 7 ) + 1 ) );
        }
    }
    else
    {
        int	nxCount;
        for ( ULONG y = 0; y < mnHeight; y++ )
        {
            nxCount = 70;
            for ( ULONG x = 0; x < mnWidth; x++ )
            {
                if (!( --nxCount ) )
                {
                    nxCount = 69;
                    *mpOStm << (BYTE)10;
                }
                *mpOStm << (BYTE)( ( mpAcc->GetPixel( y, x ) ^ 1 ) + '0' ) ;
            }
            *mpOStm << (BYTE)10;
        }
    }
}

// ------------------------------------------------------------------------
// eine Dezimalzahl im ASCII format wird in den Stream geschrieben

void PBMWriter::ImplWriteNumber( sal_Int32 nNumber )
{
    const ByteString aNum( ByteString::CreateFromInt32( nNumber ) );

    for( sal_Int16 n = 0, nLen = aNum.Len(); n < nLen; n++ )
        *mpOStm << aNum.GetChar( n );

}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, BOOL )
{
    PBMWriter aPBMWriter;

    return aPBMWriter.WritePBM( rGraphic, rStream, pFilterConfigItem );
}

// ------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL bRet = FALSE;

    if ( rPara.pWindow )
    {
        ByteString	aResMgrName( "epb" );
        ResMgr*	pResMgr;

        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

        if( pResMgr )
        {
            rPara.pResMgr = pResMgr;
            bRet = ( DlgExportEPBM( rPara ).Execute() == RET_OK );
            delete pResMgr;
        }
        else
            bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------
#ifndef GCC
#endif

// ---------------
// - Win16 trash -
// ---------------

#ifdef WIN

static HINSTANCE hDLLInst = 0;

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
    if ( nHeap )
        UnlockData( 0 );

    hDLLInst = hDLL;

    return TRUE;
}

// ------------------------------------------------------------------------

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif
