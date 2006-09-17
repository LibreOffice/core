/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eppm.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:43:54 $
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
#include "precompiled_goodies.hxx"

#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>
#include "strings.hrc"
#include "dlgeppm.hrc"
#include "dlgeppm.hxx"

//============================ PPMWriter ==================================

class PPMWriter {

private:

    PFilterCallback     mpCallback;
    void *              mpCallerData;

    SvStream*           mpOStm;             // Die auszugebende PPM-Datei
    USHORT              mpOStmOldModus;

    BOOL                mbStatus;
    sal_Int32           mnMode;
    BitmapReadAccess*   mpAcc;
    ULONG               mnWidth, mnHeight;  // Bildausmass in Pixeln

    BOOL                ImplCallback( USHORT nPercent );
    BOOL                ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

public:
                        PPMWriter();
                        ~PPMWriter();

    BOOL                WritePPM( const Graphic& rGraphic, SvStream& rPPM,
                                  PFilterCallback pCallback, void* pCallerdata,
                                  FilterConfigItem* pConfigItem );
};

//=================== Methoden von PPMWriter ==============================

PPMWriter::PPMWriter() :
    mbStatus    ( TRUE ),
    mpAcc       ( NULL )
{
}

// ------------------------------------------------------------------------

PPMWriter::~PPMWriter()
{
}

// ------------------------------------------------------------------------

BOOL PPMWriter::ImplCallback( USHORT nPercent )
{
    if ( mpCallback != NULL )
    {
        if ( ( (*mpCallback)( mpCallerData, nPercent ) ) == TRUE )
        {
            mpOStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
            return TRUE;
        }
    }
    return FALSE;
}

// ------------------------------------------------------------------------

BOOL PPMWriter::WritePPM( const Graphic& rGraphic, SvStream& rPPM,
                            PFilterCallback pCallback, void* pCallerdata,
                                FilterConfigItem* pConfigItem )
{

    mpOStm = &rPPM;
    mpCallback = pCallback;
    mpCallerData = pCallerdata;

    if ( pConfigItem )
        mnMode = pConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "FileFormat" ) ), 0 );

    BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
    Bitmap      aBmp = aBmpEx.GetBitmap();
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
        ImplWriteNumber( 255 );         // max. col.
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
                    if ( ( nNumb = nDat[ i ] / 100 ) )
                    {
                        *mpOStm << (BYTE)( nNumb + '0' );
                        nDat[ i ] -= ( nNumb * 100 );
                        nNumb = nDat[ i ] / 10;
                        *mpOStm << (BYTE)( nNumb + '0' );
                        nDat[ i ] -= ( nNumb * 10 );
                        *mpOStm << (BYTE)( nDat[ i ] + '0' );
                        nCount -= 4;
                    }
                    else if ( ( nNumb = nDat[ i ] / 10 ) )
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

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic,
                                               PFilterCallback pCallback, void* pCallerData,
                                               FilterConfigItem* pConfigItem, BOOL )
{
    PPMWriter aPPMWriter;
    return aPPMWriter.WritePPM( rGraphic, rStream, pCallback, pCallerData, pConfigItem );
}

// ------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL bRet = FALSE;

    if ( rPara.pWindow )
    {
        ByteString  aResMgrName( "epp" );
        ResMgr* pResMgr;

        aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
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

#ifndef GCC
#endif

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
