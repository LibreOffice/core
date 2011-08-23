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
#include "precompiled_filter.hxx"

#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/msgbox.hxx>
#include <svl/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>
#include "strings.hrc"
#include "dlgeppm.hrc"
#include "dlgeppm.hxx"

//============================ PPMWriter ==================================

class PPMWriter {

private:

    SvStream*			mpOStm; 			// Die auszugebende PPM-Datei
    USHORT				mpOStmOldModus;

    BOOL				mbStatus;
    sal_Int32			mnMode;
    BitmapReadAccess*	mpAcc;
    ULONG				mnWidth, mnHeight;	// Bildausmass in Pixeln

    BOOL				ImplWriteHeader();
    void				ImplWriteBody();
    void				ImplWriteNumber( sal_Int32 );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:
                        PPMWriter();
                        ~PPMWriter();

    BOOL				WritePPM( const Graphic& rGraphic, SvStream& rPPM, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von PPMWriter ==============================

PPMWriter::PPMWriter() :
    mbStatus	( TRUE ),
    mpAcc		( NULL )
{
}

// ------------------------------------------------------------------------

PPMWriter::~PPMWriter()
{
}

// ------------------------------------------------------------------------

BOOL PPMWriter::WritePPM( const Graphic& rGraphic, SvStream& rPPM, FilterConfigItem* pFilterConfigItem )
{

    mpOStm = &rPPM;

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
    aBmp.Convert( BMP_CONVERSION_24BIT );

    mpOStmOldModus = mpOStm->GetNumberFormatInt();
    mpOStm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    mpAcc = aBmp.AcquireReadAccess();
    if( mpAcc )
    {
        if ( ImplWriteHeader() )
        {
            ImplWriteBody();
        }
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

BOOL PPMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight )
    {
        if ( mnMode == 0 )
            *mpOStm << "P6\x0a";
        else
            *mpOStm << "P3\x0a";

        ImplWriteNumber( mnWidth );
        *mpOStm << (BYTE)32;
        ImplWriteNumber( mnHeight );
        *mpOStm << (BYTE)32;
        ImplWriteNumber( 255 ); 		// max. col.
        *mpOStm << (BYTE)10;
    }
    else
        mbStatus = FALSE;

    return mbStatus;
}

// ------------------------------------------------------------------------

void PPMWriter::ImplWriteBody()
{
    if ( mnMode == 0 )
    {
        for ( ULONG y = 0; y < mnHeight; y++ )
        {
            for ( ULONG x = 0; x < mnWidth; x++ )
            {
                const BitmapColor& rColor = mpAcc->GetPixel( y, x );
                *mpOStm << rColor.GetRed();
                *mpOStm << rColor.GetGreen();
                *mpOStm << rColor.GetBlue();
            }
        }
    }
    else
    {
        for ( ULONG y = 0; y < mnHeight; y++ )
        {
            int nCount = 70;
            for ( ULONG x = 0; x < mnWidth; x++ )
            {
                BYTE i, nDat[3], nNumb;
                if ( nCount < 0 )
                {
                    nCount = 69;
                    *mpOStm << (BYTE)10;
                }
                nDat[0] = mpAcc->GetPixel( y, x ).GetRed();
                nDat[1] = mpAcc->GetPixel( y, x ).GetGreen();
                nDat[2] = mpAcc->GetPixel( y, x ).GetBlue();
                for ( i = 0; i < 3; i++ )
                {
                    nNumb = nDat[ i ] / 100;
                    if ( nNumb )
                    {
                        *mpOStm << (BYTE)( nNumb + '0' );
                        nDat[ i ] -= ( nNumb * 100 );
                        nNumb = nDat[ i ] / 10;
                        *mpOStm << (BYTE)( nNumb + '0' );
                        nDat[ i ] -= ( nNumb * 10 );
                        *mpOStm << (BYTE)( nDat[ i ] + '0' );
                        nCount -= 4;
                    }
                    else
                    {
                        nNumb = nDat[ i ] / 10;
                        if ( nNumb )
                        {
                            *mpOStm << (BYTE)( nNumb + '0' );
                            nDat[ i ] -= ( nNumb * 10 );
                            *mpOStm << (BYTE)( nDat[ i ] + '0' );
                            nCount -= 3;
                        }
                        else
                        {
                            *mpOStm << (BYTE)( nDat[ i ] + '0' );
                            nCount -= 2;
                        }
                    }
                    *mpOStm << (BYTE)' ';
                }
            }
            *mpOStm << (BYTE)10;
        }
    }
}

// ------------------------------------------------------------------------
// eine Dezimalzahl im ASCII format wird in den Stream geschrieben

void PPMWriter::ImplWriteNumber( sal_Int32 nNumber )
{
    const ByteString aNum( ByteString::CreateFromInt32( nNumber ) );

    for( sal_Int16 n = 0, nLen = aNum.Len(); n < nLen; n++  )
        *mpOStm << aNum.GetChar( n );

}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, BOOL )
{
    PPMWriter aPPMWriter;
    return aPPMWriter.WritePPM( rGraphic, rStream, pFilterConfigItem );
}

// ------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL bRet = FALSE;

    if ( rPara.pWindow )
    {
        ByteString	aResMgrName( "epp" );
        ResMgr*	pResMgr;

        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

        if( pResMgr )
        {
            rPara.pResMgr = pResMgr;
            bRet = ( DlgExportEPPM( rPara ).Execute() == RET_OK );
            delete pResMgr;
        }
        else
            bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
