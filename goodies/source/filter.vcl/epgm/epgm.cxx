/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: epgm.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 17:16:51 $
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
#include "dlgepgm.hrc"
#include "dlgepgm.hxx"

//============================ PGMWriter ==================================

class PGMWriter {

private:

    SvStream*           mpOStm;             // Die auszugebende PGM-Datei
    USHORT              mpOStmOldModus;

    BOOL                mbStatus;
    UINT32              mnMode;
    BitmapReadAccess*   mpAcc;
    ULONG               mnWidth, mnHeight;  // Bildausmass in Pixeln

    void                ImplCallback( USHORT nPercent );
    BOOL                ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:
                        PGMWriter();
                        ~PGMWriter();

    BOOL                WritePGM( const Graphic& rGraphic, SvStream& rPGM, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von PGMWriter ==============================

PGMWriter::PGMWriter() :
    mbStatus    ( TRUE ),
    mpAcc       ( NULL )
{
}

// ------------------------------------------------------------------------

PGMWriter::~PGMWriter()
{
}

// ------------------------------------------------------------------------

void PGMWriter::ImplCallback( USHORT nPercent )
{
    if ( xStatusIndicator.is() && ( nPercent <= 100 ) )
        xStatusIndicator->setValue( nPercent );
}

// ------------------------------------------------------------------------

BOOL PGMWriter::WritePGM( const Graphic& rGraphic, SvStream& rPGM, FilterConfigItem* pFilterConfigItem )
{

    mpOStm = &rPGM;

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

    BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
    Bitmap      aBmp = aBmpEx.GetBitmap();
    aBmp.Convert( BMP_CONVERSION_8BIT_GREYS );

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

BOOL PGMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight )
    {
        if ( mnMode == 0 )
            *mpOStm << "P5\x0a";
        else
            *mpOStm << "P2\x0a";

        ImplWriteNumber( mnWidth );
        *mpOStm << (BYTE)32;
        ImplWriteNumber( mnHeight );
        *mpOStm << (BYTE)32;
        ImplWriteNumber( 255 );         // max. gray value
        *mpOStm << (BYTE)10;
    }
    else
        mbStatus = FALSE;

    return mbStatus;
}

// ------------------------------------------------------------------------

void PGMWriter::ImplWriteBody()
{
    if ( mnMode == 0 )
    {
        for ( ULONG y = 0; y < mnHeight; y++ )
        {
            for ( ULONG x = 0; x < mnWidth; x++ )
            {
                *mpOStm << (BYTE)( mpAcc->GetPixel( y, x ) );
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
                BYTE nDat, nNumb;
                if ( nCount < 0 )
                {
                    nCount = 69;
                    *mpOStm << (BYTE)10;
                }
                nDat = (BYTE)mpAcc->GetPixel( y, x );
                nNumb = nDat / 100;
                if ( nNumb )
                {
                    *mpOStm << (BYTE)( nNumb + '0' );
                    nDat -= ( nNumb * 100 );
                    nNumb = nDat / 10;
                    *mpOStm << (BYTE)( nNumb + '0' );
                    nDat -= ( nNumb * 10 );
                    *mpOStm << (BYTE)( nDat + '0' );
                    nCount -= 4;
                }
                else
                {
                    nNumb = nDat / 10;
                    if ( nNumb )
                    {
                        *mpOStm << (BYTE)( nNumb + '0' );
                        nDat -= ( nNumb * 10 );
                        *mpOStm << (BYTE)( nDat + '0' );
                        nCount -= 3;
                    }
                    else
                    {
                        *mpOStm << (BYTE)( nDat + '0' );
                        nCount -= 2;
                    }
                }
                *mpOStm << (BYTE)' ';
            }
            *mpOStm << (BYTE)10;
        }
    }
}

// ------------------------------------------------------------------------
// eine Dezimalzahl im ASCII format wird in den Stream geschrieben

void PGMWriter::ImplWriteNumber( sal_Int32 nNumber )
{
    const ByteString aNum( ByteString::CreateFromInt32( nNumber ) );

    for( sal_Int16 n = 0UL, nLen = aNum.Len(); n < nLen; n++  )
        *mpOStm << aNum.GetChar( n );

}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, BOOL )
{
    PGMWriter aPGMWriter;

    return aPGMWriter.WritePGM( rGraphic, rStream, pFilterConfigItem );
}

// ------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL bRet = FALSE;

    if ( rPara.pWindow )
    {
        ByteString  aResMgrName( "epg" );
        ResMgr* pResMgr;

        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

        if( pResMgr )
        {
            rPara.pResMgr = pResMgr;
            bRet = ( DlgExportEPGM( rPara ).Execute() == RET_OK );
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
