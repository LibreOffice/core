/*************************************************************************
 *
 *  $RCSfile: epgm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:12 $
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

#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/config.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/solar.hrc>
#include <svtools/fltcall.hxx>
#include "strings.hrc"
#include "dlgepgm.hrc"
#include "dlgepgm.hxx"

//============================ PGMWriter ==================================

class PGMWriter {

private:

    PFilterCallback     mpCallback;
    void *              mpCallerData;

    SvStream*           mpOStm;             // Die auszugebende PGM-Datei
    USHORT              mpOStmOldModus;

    BOOL                mbStatus;
    UINT32              mnMode;
    BitmapReadAccess*   mpAcc;
    ULONG               mnWidth, mnHeight;  // Bildausmass in Pixeln

    BOOL                ImplCallback( USHORT nPercent );
    BOOL                ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

public:
                        PGMWriter();
                        ~PGMWriter();

    BOOL                WritePGM( const Graphic& rGraphic, SvStream& rPGM,
                                  PFilterCallback pCallback, void* pCallerdata,
                                  Config* pOptionsConfig );
};

//=================== Methoden von PGMWriter ==============================

PGMWriter::PGMWriter() :
    mpAcc       ( NULL ),
    mbStatus    ( TRUE )
{
}

// ------------------------------------------------------------------------

PGMWriter::~PGMWriter()
{
}

// ------------------------------------------------------------------------

BOOL PGMWriter::ImplCallback( USHORT nPercent )
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

BOOL PGMWriter::WritePGM( const Graphic& rGraphic, SvStream& rPGM,
                          PFilterCallback pCallback, void* pCallerdata,
                          Config* pOptionsConfig )
{

    mpOStm = &rPGM;
    mpCallback = pCallback;
    mpCallerData = pCallerdata;

    if ( pOptionsConfig )
    {
        mnMode = pOptionsConfig->ReadKey( "PGM-EXPORT-FORMAT", "0" ).ToInt32();
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
                if ( ( nNumb = nDat / 100 ) )
                {
                    *mpOStm << (BYTE)( nNumb + '0' );
                    nDat -= ( nNumb * 100 );
                    nNumb = nDat / 10;
                    *mpOStm << (BYTE)( nNumb + '0' );
                    nDat -= ( nNumb * 10 );
                    *mpOStm << (BYTE)( nDat + '0' );
                    nCount -= 4;
                }
                else if ( ( nNumb = nDat / 10 ) )
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

    for( sal_Int32 n = 0UL, nLen = aNum.Len(); n < nLen; n++  )
        *mpOStm << aNum.GetChar( n );

}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic,
                                               PFilterCallback pCallback, void* pCallerData,
                                               Config* pOptionsConfig, BOOL )
{
    PGMWriter aPGMWriter;

    return aPGMWriter.WritePGM( rGraphic, rStream, pCallback, pCallerData, pOptionsConfig );
}

// ------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL bRet = FALSE;

    if ( rPara.pWindow && rPara.pCfg )
    {
        ByteString  aResMgrName( "epg" );
        ResMgr* pResMgr;

        aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );

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

#pragma hdrstop

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
