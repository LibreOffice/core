/*************************************************************************
 *
 *  $RCSfile: epbm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-08 10:03:27 $
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
#include <vcl/msgbox.hxx>
#include <svtools/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>
#include "strings.hrc"
#include "dlgepbm.hrc"
#include "dlgepbm.hxx"

//============================ PBMWriter ==================================

class PBMWriter {

private:

    PFilterCallback     mpCallback;
    void *              mpCallerData;

    SvStream*           mpOStm;             // Die auszugebende PBM-Datei
    USHORT              mpOStmOldModus;

    BOOL                mbStatus;
    sal_Int32           mnMode;             // 0 -> raw, 1-> ascii
    BitmapReadAccess*   mpAcc;
    ULONG               mnWidth, mnHeight;  // Bildausmass in Pixeln

    BOOL                ImplCallback( USHORT nPercent );
    BOOL                ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

public:
                        PBMWriter();
                        ~PBMWriter();

    BOOL                WritePBM( const Graphic& rGraphic, SvStream& rPBM,
                                  PFilterCallback pCallback, void* pCallerdata,
                                  FilterConfigItem* pConfigItem );
};

//=================== Methoden von PBMWriter ==============================

PBMWriter::PBMWriter() :
    mpAcc       ( NULL ),
    mbStatus    ( TRUE )
{
}

// ------------------------------------------------------------------------

PBMWriter::~PBMWriter()
{
}

// ------------------------------------------------------------------------

BOOL PBMWriter::ImplCallback( USHORT nPercent )
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

BOOL PBMWriter::WritePBM( const Graphic& rGraphic, SvStream& rPBM,
                          PFilterCallback pCallback, void* pCallerdata,
                          FilterConfigItem* pConfigItem )
{

    mpOStm = &rPBM;
    mpCallback = pCallback;
    mpCallerData = pCallerdata;

    if ( pConfigItem )
        mnMode = pConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "FileFPBM-EXPORT-FORMAT" ) ), 0 );

    BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
    Bitmap      aBmp = aBmpEx.GetBitmap();
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
        BYTE    nBYTE;
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
        int nxCount;
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

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic,
                                               PFilterCallback pCallback, void* pCallerData,
                                               FilterConfigItem* pConfigItem, BOOL )
{
    PBMWriter aPBMWriter;

    return aPBMWriter.WritePBM( rGraphic, rStream, pCallback, pCallerData, pConfigItem );
}

// ------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL bRet = FALSE;

    if ( rPara.pWindow )
    {
        ByteString  aResMgrName( "epb" );
        ResMgr* pResMgr;

        aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );

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
