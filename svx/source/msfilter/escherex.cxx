/*************************************************************************
 *
 *  $RCSfile: escherex.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:21 $
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

#ifndef _ESCHESDO_HXX
#include "eschesdo.hxx"
#endif
#ifndef _SVX_ESCHEREX_HXX
#include "escherex.hxx"
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _ZCODEC_HXX
#include <tools/zcodec.hxx>
#endif
#ifndef _INC_STDLIB
#include <stdlib.h>
#endif
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENTSTYLE_HPP_
#include <com/sun/star/awt/GradientStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RASTEROPERATION_HPP_
#include <com/sun/star/awt/RasterOperation.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XGRAPHICS_HPP_
#include <com/sun/star/awt/XGraphics.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::rtl;

// ---------------------------------------------------------------------------------------------

EscherExClientRecord_Base::~EscherExClientRecord_Base()
{
}

EscherExClientAnchor_Base::~EscherExClientAnchor_Base()
{
}

// ---------------------------------------------------------------------------------------------

struct EscherPropSortStruct
{
    BYTE*       pBuf;
    UINT32      nPropSize;
    UINT32      nPropValue;
    UINT16      nPropId;
};


struct ESCHER_GDIStruct
{
    Rectangle   GDIBoundRect;
    Size        GDISize;
    UINT32      GDIUncompressedSize;
};

// ---------------------------------------------------------------------------------------------

EscherPersistTable::EscherPersistTable()
{
}

// ---------------------------------------------------------------------------------------------

EscherPersistTable::~EscherPersistTable()
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
        delete (EscherPersistEntry*)pPtr;
}

// ---------------------------------------------------------------------------------------------

BOOL EscherPersistTable::PtIsID( UINT32 nID )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
            return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

void EscherPersistTable::PtInsert( UINT32 nID, UINT32 nOfs )
{
    maPersistTable.Insert( new EscherPersistEntry( nID, nOfs ) );
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherPersistTable::PtDelete( UINT32 nID )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
        {
            UINT32 nRetValue = ((EscherPersistEntry*)pPtr)->mnOffset;
            delete (EscherPersistEntry*) maPersistTable.Remove();
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherPersistTable::PtGetOffsetByID( UINT32 nID )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
            return ((EscherPersistEntry*)pPtr)->mnOffset;
    }
    return 0;
};

// ---------------------------------------------------------------------------------------------

UINT32 EscherPersistTable::PtReplace( UINT32 nID, UINT32 nOfs )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
        {
            UINT32 nRetValue = ((EscherPersistEntry*)pPtr)->mnOffset;
            ((EscherPersistEntry*)pPtr)->mnOffset = nOfs;
            return nRetValue;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherPersistTable::PtReplaceOrInsert( UINT32 nID, UINT32 nOfs )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((EscherPersistEntry*)pPtr)->mnID == nID )
        {
            UINT32 nRetValue = ((EscherPersistEntry*)pPtr)->mnOffset;
            ((EscherPersistEntry*)pPtr)->mnOffset = nOfs;
            return nRetValue;
        }
    }
    PtInsert( nID, nOfs );
    return 0;
}

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

EscherBlibEntry::EscherBlibEntry( SvMemoryStream& rStream, ESCHER_BlibType eBlibType, UINT32 nPictureOffset )
{
    UINT32* pPtr = &mnIdentifier[0];
    UINT32 nLenght, nType, nPos, nCRC, nOldPos = rStream.Tell();
    mnSizeExtra = 0;
    mnRefCount = 1;
    rStream.Seek( STREAM_SEEK_TO_END );
    mnSize = rStream.Tell();
    rStream.Seek( STREAM_SEEK_TO_BEGIN );
    mnPictureOffset = nPictureOffset;
    meBlibType = eBlibType;
    if ( eBlibType == PNG )         // Bitmap ID ermitteln, um keine doppelten Grafiken abzuspeichern
    {
        rStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
        rStream.SeekRel( 8 );
        do
        {
            rStream >> nLenght >> nType;                // wir ermitteln die den Identifier anhand der Checksumme des PNG IDat chunks
            nPos = rStream.Tell() + nLenght;            // naechste chunk position
            if ( nPos >= mnSize )                       // kein IDAT -> break;
                break;
            rStream.Seek( nPos );
            rStream >> nCRC;
        }
        while ( nType != 0x49444154 );                  // IDAT chunk suchen
    }
    else
    {
        if ( mnSize > 8 )
        {
            rStream.SeekRel( mnSize - 8 );
            rStream >> nPos >> nCRC;                    // ( Komprimiertes UINT32 + Checksumme des ZCodec ) ergeben einen Teil der UID
            nLenght = mnSize;
        }
    }
    *pPtr++ = nCRC;                                     // LitteEndian / BigEndian ist fuer die Checksumme egal
    *pPtr++ = nLenght;
    *pPtr++ = nPos;
    *pPtr = 0;
    rStream.Seek( nOldPos );
};

// ---------------------------------------------------------------------------------------------

EscherBlibEntry::~EscherBlibEntry()
{
};

// ---------------------------------------------------------------------------------------------

BOOL EscherBlibEntry::operator==( const EscherBlibEntry& rEscherBlibEntry ) const
{
    if ( meBlibType != rEscherBlibEntry.meBlibType )
        return FALSE;
    for ( int i = 0; i < 3; i++ )
    {
        if ( mnIdentifier[ i ] != rEscherBlibEntry.mnIdentifier[ i ] )
            return FALSE;
    }
    return TRUE;
}

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

EscherEx::EscherEx( SvStream& rOutStrm, UINT32 nDrawings ) :
    mpOutStrm               ( &rOutStrm ),
    mpPicOutStrm            ( NULL ),
    mpOffsets               ( new UINT32[ 32 ] ),
    mpSizes                 ( new UINT32[ 32 ] ),
    mpRecTypes              ( new UINT16[ 32 ] ),
    mnDrawings              ( nDrawings ),
    mnLevel                 ( 0 ),
    mbEscherSpgr            ( FALSE ),
    mbEscherDgg             ( FALSE ),                                      // TRUE, wenn jemals ein ESCHER_Dgg angelegt wurde, dieser wird dann im Dest. aktualisiert
    mbEscherDg              ( FALSE ),
    mbOleEmf                ( FALSE ),
    mpSortStruct            ( NULL ),
    mpBlibEntrys            ( NULL ),
    mnBlibBufSize           ( 0 ),
    mnBlibEntrys            ( 0 ),
    mnGroupLevel            ( 0 ),
    mnHellLayerId           ( USHRT_MAX )
{
    mpSizes[ 0 ] = 0;
    mnStrmStartOfs = mpOutStrm->Tell();
    mpImplEscherExSdr = new ImplEscherExSdr( *this );
}

// ---------------------------------------------------------------------------------------------

void EscherEx::Flush( SvStream* pPicStreamMergeBSE /* = NULL */ )
{
    if ( mbEscherDgg )                                                      // ESCHER_Dgg anpassen
    {
        PtReplaceOrInsert( ESCHER_Persist_CurrentPosition, mpOutStrm->Tell() );
        if ( ImplSeek( ESCHER_Persist_Dgg ) )
        {
            *mpOutStrm << mnCurrentShapeID << (UINT32)( mnFIDCLs + 1 ) << mnTotalShapesDgg << mnDrawings;
        }
        if ( mnBlibEntrys )
        {
            if ( ImplSeek( ESCHER_Persist_BlibStoreContainer ) )            // ESCHER_BlibStoreContainer schreiben
            {
                UINT32 nAddBytes = 44 * mnBlibEntrys;
                if ( pPicStreamMergeBSE )
                {
                    for ( UINT32 i = 0; i < mnBlibEntrys; i++ )
                    {
                        nAddBytes += mpBlibEntrys[ i ]->mnSize + mpBlibEntrys[ i ]->mnSizeExtra;
                    }
                }
                InsertAtCurrentPos( 8 + nAddBytes, TRUE );          // platz schaffen fuer Blib Container samt seinen Blib Atomen
                OpenContainer( ESCHER_BstoreContainer, 1 );
                if ( pPicStreamMergeBSE )
                {
                    ULONG nOldPos = pPicStreamMergeBSE->Tell();
                    const nBuf = 0x40000;   // 256KB buffer
                    BYTE* pBuf = new BYTE[ nBuf ];
                    for ( UINT32 i = 0; i < mnBlibEntrys; i++ )
                    {
                        EscherBlibEntry* pBlibEntry = mpBlibEntrys[ i ];
                        BYTE nBlibType = pBlibEntry->meBlibType;
                        UINT32 nBlipSize = pBlibEntry->mnSize + pBlibEntry->mnSizeExtra;
                        AddAtom( 36 + nBlipSize, ESCHER_BSE, 2, nBlibType );
                        // FBSE
                        *mpOutStrm << nBlibType;
                        switch ( nBlibType )
                        {
                            case EMF :
                            case WMF :  // EMF/WMF auf OS2 zu Pict Konvertieren
                                *mpOutStrm << BYTE(PICT);
                            break;
                            default:
                                *mpOutStrm << nBlibType;
                        }
                        mpOutStrm->Write( &pBlibEntry->mnIdentifier[ 0 ], 16 );
                        *mpOutStrm
                            << (UINT16)0
                            << nBlipSize
                            << pBlibEntry->mnRefCount
                            << (UINT32)0    // yes, the offset is always zero
                            << (UINT32)0;
                        // BLIP
                        pPicStreamMergeBSE->Seek( pBlibEntry->mnPictureOffset );
                        UINT16 n16;
                        // record version and instance
                        *pPicStreamMergeBSE >> n16;
                        *mpOutStrm << n16;
                        // record type
                        *pPicStreamMergeBSE >> n16;
                        *mpOutStrm << UINT16( ESCHER_BlipFirst + nBlibType );
                        DBG_ASSERT( n16 == ESCHER_BlipFirst + nBlibType , "EscherEx::Flush: BLIP record types differ" );
                        UINT32 n32;
                        // record size
                        *pPicStreamMergeBSE >> n32;
                        nBlipSize -= 8;
                        *mpOutStrm << nBlipSize;
                        DBG_ASSERT( nBlipSize == n32, "EscherEx::Flush: BLIP sizes differ" );
                        // record
                        while ( nBlipSize )
                        {
                            UINT32 nBytes = ( nBlipSize > nBuf ? nBuf : nBlipSize );
                            pPicStreamMergeBSE->Read( pBuf, nBytes );
                            mpOutStrm->Write( pBuf, nBytes );
                            nBlipSize -= nBytes;
                        }
                    }
                    delete [] pBuf;
                    pPicStreamMergeBSE->Seek( nOldPos );
                }
                else
                {
                    for ( UINT32 i = 0; i < mnBlibEntrys; i++ )
                    {
                        EscherBlibEntry* pBlibEntry = mpBlibEntrys[ i ];
                        BYTE nBlibType = pBlibEntry->meBlibType;
                        AddAtom( 36, ESCHER_BSE, 2, nBlibType );
                        *mpOutStrm << nBlibType;
                        if ( nBlibType == 3 )
                            *mpOutStrm << (BYTE)4;  // WMF auf OS2 zu Pict Konvertieren
                        else
                            *mpOutStrm << nBlibType;
                        mpOutStrm->Write( &pBlibEntry->mnIdentifier[ 0 ], 16 );
                        *mpOutStrm
                            << (UINT16)0
                            << UINT32( pBlibEntry->mnSize + pBlibEntry->mnSizeExtra )
                            << pBlibEntry->mnRefCount
                            << pBlibEntry->mnPictureOffset
                            << (UINT32)0;
                    }
                }
                CloseContainer();   // ESCHER_BlibStoreContainer
            }
        }
        mpOutStrm->Seek( PtGetOffsetByID( ESCHER_Persist_CurrentPosition ) );
    }
}

// ---------------------------------------------------------------------------------------------

EscherEx::~EscherEx()
{
    for ( UINT32 i = 0; i < mnBlibEntrys; delete mpBlibEntrys[ i++ ] );
    delete mpBlibEntrys;
    delete[] mpSortStruct;
    delete[] mpRecTypes;
    delete[] mpSizes;
    delete[] mpOffsets;
    delete mpImplEscherExSdr;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::ImplInsertBlib( EscherBlibEntry* pEscherBlibEntry )
{
    if ( mnBlibBufSize == mnBlibEntrys )
    {
        mnBlibBufSize += 64;
        EscherBlibEntry** pTemp = new EscherBlibEntry*[ mnBlibBufSize ];
        for ( UINT32 i = 0; i < mnBlibEntrys; i++ )
        {
            pTemp[ i ] = mpBlibEntrys[ i ];
        }
        delete mpBlibEntrys;
        mpBlibEntrys = pTemp;
    }
    mpBlibEntrys[ mnBlibEntrys++ ] = pEscherBlibEntry;
    return mnBlibEntrys;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::ImplGetBlibID( SvMemoryStream& rSource, ESCHER_BlibType eBlibType, const ESCHER_GDIStruct* pGDI )
{
    EscherBlibEntry* pEscherBlibEntry = new EscherBlibEntry( rSource, eBlibType, mpPicOutStrm->Tell() );
    for ( UINT32 i = 0; i < mnBlibEntrys; i++ )
    {
        if ( *( mpBlibEntrys[ i ] ) == *pEscherBlibEntry )
        {
            mpBlibEntrys[ i ]->mnRefCount++;
            delete pEscherBlibEntry;
            return i + 1;
        }
    }
    switch ( eBlibType )
    {
        case PNG :
        {
            const UINT32 nExtra = 17;
            pEscherBlibEntry->mnSizeExtra += 8 + nExtra;
            *mpPicOutStrm << (UINT32)0xf01e6e00 << (UINT32)( pEscherBlibEntry->mnSize + nExtra );
            mpPicOutStrm->Write( pEscherBlibEntry->mnIdentifier, 16 );
            *mpPicOutStrm << (BYTE)0xff;
            mpPicOutStrm->Write( rSource.GetData(), pEscherBlibEntry->mnSize );
        }
        break;
        case WMF :
        case EMF :
        {
            const UINT32 nExtra = (eBlibType == WMF ? 0x42 : 0x32);
            const UINT32 nHeaderID = (eBlibType == WMF ? 0xf01b2170 : 0xf01a3d40);
            pEscherBlibEntry->mnSizeExtra += 8 + nExtra;
            *mpPicOutStrm << nHeaderID << (UINT32)( pEscherBlibEntry->mnSize + nExtra );
            if ( eBlibType == WMF )
                mpPicOutStrm->Write( pEscherBlibEntry->mnIdentifier, 16 );
            mpPicOutStrm->Write( pEscherBlibEntry->mnIdentifier, 16 );


            UINT32 nWidth = pGDI->GDIBoundRect.GetWidth() * 360;
            UINT32 nHeight = pGDI->GDIBoundRect.GetHeight() * 360;
            double fWidth = (double)pGDI->GDIBoundRect.GetWidth() / 10000.0 * 1027.0;
            double fHeight = (double)pGDI->GDIBoundRect.GetHeight() / 10000.0 * 1027.0;

            *mpPicOutStrm   << (UINT32)( pGDI->GDIUncompressedSize )// WMFSize ohne FileHeader
                            << (INT32)0         // da die Originalgroesse des WMF's (ohne FileHeader)
                            << (INT32)0         // nicht mehr feststellbar ist, schreiben wir 10cm / x
                            << (INT32)fWidth
                            << (INT32)fHeight
                            << nWidth
                            << nHeight

/*
            double fQuo = (double)pGDI->GDIBoundRect.GetHeight() / pGDI->GDIBoundRect.GetWidth();
            *mpPicOutStrm   << (UINT32)( pGDI->GDIUncompressedSize )// WMFSize ohne FileHeader
                            << (INT32)0         // da die Originalgroesse des WMF's (ohne FileHeader)
                            << (INT32)0         // nicht mehr feststellbar ist, schreiben wir 10cm / x
                            << (INT32)1027
                            << (INT32)( 1027 * fQuo )
                            << (UINT32)0x36ee80
                            << (UINT32)( 0x36ee80 * fQuo )
*/
                            << (UINT32)( pEscherBlibEntry->mnSize )
                            << (UINT16)0xfe00;                      // compression Flags
            mpPicOutStrm->Write( rSource.GetData(), pEscherBlibEntry->mnSize );
        }
        break;
        default:
            DBG_ERRORFILE( "EscherEx::ImplGetBlibID: BlibType not handled" );
    }
    return ImplInsertBlib( pEscherBlibEntry );
}

// ---------------------------------------------------------------------------------------------

void EscherEx::InsertAtCurrentPos( UINT32 nBytes, BOOL bContainer )
{
    UINT32  i, nSize, nType, nSource, nBufSize, nToCopy, nCurPos = mpOutStrm->Tell();
    BYTE*   pBuf;

    // Persist table anpassen
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        UINT32 nOfs = ((EscherPersistEntry*)pPtr)->mnOffset;
        if ( nOfs >= nCurPos )
            ((EscherPersistEntry*)pPtr)->mnOffset += nBytes;
    }

    // container und atom sizes anpassen
    mpOutStrm->Seek( mnStrmStartOfs );
    while ( mpOutStrm->Tell() < nCurPos )
    {
        *mpOutStrm >> nType >> nSize;
        if ( ( mpOutStrm->Tell() + nSize ) >= ( ( bContainer ) ? nCurPos + 1 : nCurPos ) )
        {
            mpOutStrm->SeekRel( -4 );
            *mpOutStrm << (UINT32)( nSize + nBytes );
            if ( ( nType & 0xf ) != 0xf )
                mpOutStrm->SeekRel( nSize );
        }
        else
            mpOutStrm->SeekRel( nSize );
    }

    // Container Offsets verschieben
    for ( i = 1; i <= (UINT32)mnLevel; i++ )
    {
        if ( mpOffsets[ i ] > nCurPos )
            mpOffsets[ i ] += nBytes;
    }
    mpOutStrm->Seek( STREAM_SEEK_TO_END );
    nSource = mpOutStrm->Tell();
    nToCopy = nSource - nCurPos;                        // Stream um nBytes vergroessern
    pBuf = new BYTE[ 0x40000 ];                         // 256KB Buffer
    while ( nToCopy )
    {
        nBufSize = ( nToCopy >= 0x40000 ) ? 0x40000 : nToCopy;
        nToCopy -= nBufSize;
        nSource -= nBufSize;
        mpOutStrm->Seek( nSource );
        mpOutStrm->Read( pBuf, nBufSize );
        mpOutStrm->Seek( nSource + nBytes );
        mpOutStrm->Write( pBuf, nBufSize );
    }
    delete pBuf;
    mpOutStrm->Seek( nCurPos );
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::SeekBehindRecHeader( UINT16 nRecType )
{
    UINT32  nOldPos, nStreamEnd, nType, nSize;

    nOldPos = mpOutStrm->Tell();
    nStreamEnd = mpOutStrm->Seek( STREAM_SEEK_TO_END );
    mpOutStrm->Seek( nOldPos );
    while ( mpOutStrm->Tell() < nStreamEnd )
    {
        *mpOutStrm >> nType >> nSize;
        if ( ( nType >> 16 ) == nRecType )
            return TRUE;
        if ( ( nType & 0xf ) != 0xf )
            mpOutStrm->SeekRel( nSize );
    }
    mpOutStrm->Seek( nOldPos );
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::InsertPersistOffset( UINT32 nKey, UINT32 nOffset )
{
    PtInsert( ESCHER_Persist_PrivateEntry | nKey, nOffset );
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::ImplSeek( UINT32 nKey )
{
    UINT32 nPos = PtGetOffsetByID( nKey );
    if ( nPos )
        mpOutStrm->Seek( nPos );
    else
    {
        if (! PtIsID( nKey ) )
            return FALSE;
        mpOutStrm->Seek( 0 );
    }
    return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::SeekToPersistOffset( UINT32 nKey )
{
    return ImplSeek( ESCHER_Persist_PrivateEntry | nKey );
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::InsertAtPersistOffset( UINT32 nKey, UINT32 nValue )
{
    UINT32  nOldPos = mpOutStrm->Tell();
    BOOL    bRetValue = SeekToPersistOffset( nKey );
    if ( bRetValue )
    {
        *mpOutStrm << nValue;
        mpOutStrm->Seek( nOldPos );
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::OpenContainer( UINT16 nEscherContainer, int nRecInstance )
{
    if ( mnLevel == 0 )
        mpSizes[ 0 ] = 0;

    mpSizes[ mnLevel++ ] += 8;
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | 0xf  ) << nEscherContainer << (UINT32)0;
    mpOffsets[ mnLevel ] = mpOutStrm->Tell() - 4;
    mpSizes[ mnLevel ] = 0;
    mpRecTypes[ mnLevel ] = nEscherContainer;

    switch( nEscherContainer )
    {
        case ESCHER_DggContainer :
        {
            mbEscherDgg = TRUE;
            mnFIDCLs = mnDrawings;
            mnCurrentDg = 0;
            mnCurrentShapeID = 0;
            mnTotalShapesDgg = 0;
            mnCurrentShapeMaximumID = 0;
            AddAtom( 16 + ( mnDrawings << 3 ), ESCHER_Dgg );                // an FDGG and several FIDCLs
            PtReplaceOrInsert( ESCHER_Persist_Dgg, mpOutStrm->Tell() );
            *mpOutStrm << (UINT32)0                                         // the current maximum shape ID
                       << (UINT32)0                                         // the number of ID clusters + 1
                       << (UINT32)0                                         // the number of total shapes saved
                       << (UINT32)0;                                        // the total number of drawings saved
            PtReplaceOrInsert( ESCHER_Persist_Dgg_FIDCL, mpOutStrm->Tell() );
            for ( UINT32 i = 0; i < mnFIDCLs; i++ )                         // Dummy FIDCLs einfuegen
            {
                *mpOutStrm << (UINT32)0 << (UINT32)0;                       // Drawing Nummer, Anzahl der Shapes in diesem IDCL
            }
            PtReplaceOrInsert( ESCHER_Persist_BlibStoreContainer, mpOutStrm->Tell() );
        }
        break;

        case ESCHER_DgContainer :
        {
            if ( mbEscherDgg )
            {
                if ( !mbEscherDg )
                {
                    mbEscherDg = TRUE;
                    mnCurrentDg++;
                    mnTotalShapesDg = 0;
                    mnTotalShapeIdUsedDg = 0;
                    mnCurrentShapeID = ( mnCurrentShapeMaximumID &~0x3ff ) + 0x400; // eine neue Seite bekommt immer eine neue ShapeId die ein vielfaches von 1024 ist,
                                                                                    // damit ist erste aktuelle Shape ID 0x400
                    AddAtom( 8, ESCHER_Dg, 0, mnCurrentDg );
                    PtReplaceOrInsert( ESCHER_Persist_Dg | mnCurrentDg, mpOutStrm->Tell() );
                    *mpOutStrm << (UINT32)0     // The number of shapes in this drawing
                               << (UINT32)0;    // The last MSOSPID given to an SP in this DG
                }
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherSpgr = TRUE;
            }
        }
        break;

        case ESCHER_SpContainer :
        {
        }
        break;

        default:
        break;
    }
}

// ---------------------------------------------------------------------------------------------

void EscherEx::CloseContainer()
{
    UINT32 nOldPos = mpOutStrm->Tell();
    mpOutStrm->Seek( mpOffsets[ mnLevel ] );
    UINT32 nSize = mpSizes[ mnLevel ];
    *mpOutStrm << nSize;
    switch( mpRecTypes[ mnLevel ] )
    {
        case ESCHER_DgContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherDg = FALSE;
                if ( ImplSeek( ESCHER_Persist_Dg | mnCurrentDg ) )
                {
                    // shapeanzahl des drawings setzen
                    mnTotalShapesDgg += mnTotalShapesDg;
                    *mpOutStrm << mnTotalShapesDg << mnCurrentShapeMaximumID;
                    if ( ImplSeek( ESCHER_Persist_Dgg_FIDCL ) )
                    {
                        if ( mnTotalShapesDg == 0 )
                        {
                            mpOutStrm->SeekRel( 8 );
                        }
                        else
                        {
                            if ( mnTotalShapeIdUsedDg )
                            {
                                // die benutzten Shape Ids des drawings in die fidcls setzen
                                UINT32 i, nFIDCL = ( ( mnTotalShapeIdUsedDg - 1 ) / 0x400 );
                                if ( nFIDCL )
                                {
                                    if ( nOldPos > mpOutStrm->Tell() )
                                        nOldPos += ( nFIDCL << 3 );

                                    mnFIDCLs += nFIDCL;
                                    InsertAtCurrentPos( nFIDCL << 3 );          // platz schaffen fuer weitere FIDCL's
                                }
                                for ( i = 0; i <= nFIDCL; i++ )
                                {
                                    *mpOutStrm << mnCurrentDg;
                                    if ( i < nFIDCL )
                                        *mpOutStrm << (UINT32)0x400;
                                    else
                                    {
                                        UINT32 nShapesLeft = mnTotalShapeIdUsedDg % 0x400;
                                        if ( !nShapesLeft )
                                            nShapesLeft = 0x400;
                                        *mpOutStrm << (UINT32)nShapesLeft;
                                    }
                                }
                            }
                        }
                        PtReplaceOrInsert( ESCHER_Persist_Dgg_FIDCL, mpOutStrm->Tell() );   // neuen FIDCL Offset fuer naechste Seite
                    }
                }
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherSpgr )
            {
                mbEscherSpgr = FALSE;

            }
        }
        break;

        default:
        break;
    }
    mpSizes[ --mnLevel ] += nSize;
    mpOutStrm->Seek( nOldPos );
}

// ---------------------------------------------------------------------------------------------

void EscherEx::BeginAtom()
{
    mnCountOfs = mpOutStrm->Tell();
    *mpOutStrm << (UINT32)0 << (UINT32)0;       // record header wird spaeter geschrieben
}

// ---------------------------------------------------------------------------------------------

void EscherEx::EndAtom( UINT16 nRecType, int nRecVersion, int nRecInstance )
{
    UINT32  nOldPos = mpOutStrm->Tell();
    mpOutStrm->Seek( mnCountOfs );
    mnCountSize = nOldPos - mnCountOfs;
    mpSizes[ mnLevel ] += mnCountSize;
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ) << nRecType << (UINT32)( mnCountSize - 8 );
    mpOutStrm->Seek( nOldPos );
}

// ---------------------------------------------------------------------------------------------

void EscherEx::AddAtom( UINT32 nAtomSize, UINT16 nRecType, int nRecVersion, int nRecInstance )
{
    mpSizes[ mnLevel ] += nAtomSize + 8;
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ) << nRecType << nAtomSize;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::AddClientAnchor( const Rectangle& rRect )
{
    AddAtom( 8, ESCHER_ClientAnchor );
    *mpOutStrm << (INT16)rRect.Top() << (INT16)rRect.Left() << (INT16)rRect.Right() << (INT16)rRect.Bottom();
}

// ---------------------------------------------------------------------------------------------

EscherExHostAppData* EscherEx::EnterAdditionalTextGroup()
{
    return NULL;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::EnterGroup( const Rectangle* pBoundRect )
{
    Rectangle aRect;
    if( pBoundRect )
        aRect = *pBoundRect;

    OpenContainer( ESCHER_SpgrContainer );
    OpenContainer( ESCHER_SpContainer );
    AddAtom( 16, ESCHER_Spgr, 1 );
    PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel,
                        mpOutStrm->Tell() );
    *mpOutStrm  << (INT32)aRect.Left()  // Bounding box fuer die Gruppierten shapes an die sie attached werden
                << (INT32)aRect.Top()
                << (INT32)aRect.Right()
                << (INT32)aRect.Bottom();

    UINT32 nShapeId = GetShapeID();
    if ( !mnGroupLevel )
    {
        AddShape( ESCHER_ShpInst_Min, 5, nShapeId );                    // Flags: Group | Patriarch
        CloseContainer();                                               // ESCHER_SpContainer
    }
    else
    {
        AddShape( ESCHER_ShpInst_Min, 0x201, nShapeId );                // Flags: Group | HaveAnchor

#if EES_WRITE_EPP
        AddAtom( 8, ESCHER_ClientAnchor );
        PtReplaceOrInsert( ESCHER_Persist_Grouping_Logic | mnGroupLevel,
                            mpOutStrm->Tell() );
        *mpOutStrm << (INT16)aRect.Top() << (INT16)aRect.Left()
                   << (INT16)aRect.Right() << (INT16)aRect.Bottom();
#else // !EES_WRITE_EPP
        BeginCount();
        AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x00040004 );
        EndCount( ESCHER_OPT, 3 );
        if ( mnGroupLevel > 1 )
        {
            AddAtom( 16, ESCHER_ChildAnchor );
            *mpOutStrm  << (INT32)aRect.Left()
                        << (INT32)aRect.Top()
                           << (INT32)aRect.Right()
                        << (INT32)aRect.Bottom();
        }
        EscherExHostAppData* pAppData = mpImplEscherExSdr->ImplGetHostData();
        if( pAppData )
        {
            if ( mnGroupLevel <= 1 )
                pAppData->WriteClientAnchor( *this, aRect );
            pAppData->WriteClientData( *this );
        }
#endif // EES_WRITE_EPP

        CloseContainer();                                               // ESCHER_SpContainer
    }
    mnGroupLevel++;
    return nShapeId;
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::SetGroupSnapRect( UINT32 nGroupLevel, const Rectangle& rRect )
{
    BOOL bRetValue = FALSE;
    if ( nGroupLevel )
    {
        UINT32 nCurrentPos = mpOutStrm->Tell();
        if ( ImplSeek( ESCHER_Persist_Grouping_Snap | ( nGroupLevel - 1 ) ) )
        {
            *mpOutStrm  << (INT32)rRect.Left()  // Bounding box fuer die Gruppierten shapes an die sie attached werden
                        << (INT32)rRect.Top()
                        << (INT32)rRect.Right()
                        << (INT32)rRect.Bottom();
            mpOutStrm->Seek( nCurrentPos );
        }
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

BOOL EscherEx::SetGroupLogicRect( UINT32 nGroupLevel, const Rectangle& rRect )
{
    BOOL bRetValue = FALSE;
    if ( nGroupLevel )
    {
        UINT32 nCurrentPos = mpOutStrm->Tell();
        if ( ImplSeek( ESCHER_Persist_Grouping_Logic | ( nGroupLevel - 1 ) ) )
        {
            *mpOutStrm << (INT16)rRect.Top() << (INT16)rRect.Left() << (INT16)rRect.Right() << (INT16)rRect.Bottom();
            mpOutStrm->Seek( nCurrentPos );
        }
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::LeaveGroup()
{
    --mnGroupLevel;
    PtDelete( ESCHER_Persist_Grouping_Snap | mnGroupLevel );
    PtDelete( ESCHER_Persist_Grouping_Logic | mnGroupLevel );
    CloseContainer();
}

// ---------------------------------------------------------------------------------------------

void EscherEx::AddShape( UINT32 nShpInstance, UINT32 nFlags, UINT32 nShapeID )
{
    AddAtom( 8, ESCHER_Sp, 2, nShpInstance );

    if ( !nShapeID )
        nShapeID = GetShapeID();

    if ( nFlags ^ 1 )                           // is this a group shape ?
    {                                           // if not
        if ( mnGroupLevel > 1 )
            nFlags |= 2;                        // this not a topmost shape
    }
    *mpOutStrm << nShapeID << nFlags;

    if ( mbEscherSpgr )
        mnTotalShapesDg++;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::GetShapeID()
{
    mnCurrentShapeMaximumID = mnCurrentShapeID; // maximum setzen
    mnCurrentShapeID++;                         // mnCurrentShape ID auf nachste freie ID
    mnTotalShapeIdUsedDg++;
    return mnCurrentShapeMaximumID;
}

// ---------------------------------------------------------------------------------------------

extern "C" int __LOADONCALLAPI EscherPropSortFunc( const void* p1, const void* p2 )
{
    INT16   nID1 = ((EscherPropSortStruct*)p1)->nPropId &~0xc000;
    INT16   nID2 = ((EscherPropSortStruct*)p2)->nPropId &~0xc000;

    if( nID1  < nID2 )
        return -1;
    else if( nID1 > nID2 )
        return 1;
    else
        return 0;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::BeginCount()
{
    mnCountOfs = mpOutStrm->Tell();
    *mpOutStrm << (UINT32)0 << (UINT32)0;       // record header wird spaeter geschrieben
    mnSortCount = 0;                            // normale Properties werden automatisch sortiert
    mnCountCount = 0;
    mnCountSize = 8;

    if ( !mpSortStruct )
        mpSortStruct = new EscherPropSortStruct[ mnSortBufSize = 64 ];
}

// ---------------------------------------------------------------------------------------------

void EscherEx::AddOpt( UINT16 nPropID, UINT32 nPropValue, BOOL bBlib )
{
    AddOpt( nPropID, bBlib, nPropValue, NULL, 0 );
}

// ---------------------------------------------------------------------------------------------

void EscherEx::AddOpt( UINT16 nPropID, BOOL bBlib, UINT32 nPropValue, BYTE* pProp, UINT32 nPropSize )
{
    if ( bBlib )                // bBlib is only valid when fComplex = 0
        nPropID |= 0x4000;
    if ( pProp )
        nPropID |= 0x8000;      // fComplex = TRUE;

    UINT32 i;
    for( i = 0; i < mnSortCount; i++ )
    {
        if ( ( mpSortStruct[ i ].nPropId &~0xc000 ) == ( nPropID &~0xc000 ) )   // pruefen, ob Property nur ersetzt wird
        {
            mpSortStruct[ i ].nPropId = nPropID;
            if ( mpSortStruct[ i ].pBuf )
            {
                mnCountSize -= mpSortStruct[ i ].nPropSize;
                delete mpSortStruct[ i ].pBuf;
            }
            mpSortStruct[ i ].pBuf = pProp;
            mpSortStruct[ i ].nPropSize = nPropSize;
            mpSortStruct[ i ].nPropValue = nPropValue;
            if ( pProp )
                mnCountSize += nPropSize;
            return;
        }
    }
    mnCountCount++;
    mnCountSize += 6;
    if ( mnSortCount == mnSortBufSize )                                         // buffer vergroessern
    {
        mnSortBufSize <<= 1;
        EscherPropSortStruct* pTemp = new EscherPropSortStruct[ mnSortBufSize ];
        for( i = 0; i < mnSortCount; i++ )
        {
            pTemp[ i ] = mpSortStruct[ i ];
        }
        delete mpSortStruct;
        mpSortStruct = pTemp;
    }
    mpSortStruct[ mnSortCount ].nPropId = nPropID;                              // property einfuegen
    mpSortStruct[ mnSortCount ].pBuf = pProp;
    mpSortStruct[ mnSortCount ].nPropSize = nPropSize;
    mpSortStruct[ mnSortCount++ ].nPropValue = nPropValue;

    if ( pProp )
        mnCountSize += nPropSize;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::AddColor( UINT32 nColor )
{
    mnCountCount++;
    mnCountSize += 4;
    *mpOutStrm << nColor;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::EndCount( UINT16 nRecType, UINT16 nRecVersion )
{
    UINT32  nOldPos = mpOutStrm->Tell();
    mpOutStrm->Seek( mnCountOfs );
    mpSizes[ mnLevel ] += mnCountSize;
    *mpOutStrm << (UINT16)( ( mnCountCount << 4 ) | ( nRecVersion & 0xf ) ) << (UINT16)nRecType << (UINT32)( mnCountSize - 8 );
    mpOutStrm->Seek( nOldPos );
    if ( mnSortCount )
    {
        qsort( mpSortStruct, mnSortCount, sizeof( EscherPropSortStruct ), EscherPropSortFunc );
        BOOL bComplex = FALSE;
        UINT32 i;
        for ( i = 0; i < mnSortCount; i++ )
        {
            *mpOutStrm << (UINT16)mpSortStruct[ i ].nPropId
                       << (UINT32)mpSortStruct[ i ].nPropValue;
            if ( mpSortStruct[ i ].pBuf )
                bComplex = TRUE;
        }
        if ( bComplex )
        {
            for ( i = 0; i < mnSortCount; i++ )
            {
                if ( mpSortStruct[ i ].pBuf )
                {
                    mpOutStrm->Write( mpSortStruct[ i ].pBuf, mpSortStruct[ i ].nPropSize );
                    delete mpSortStruct[ i ].pBuf;
                }
            }
        }
        mnSortCount = 0;
    }
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::GetColor( const UINT32 nSOColor, BOOL bSwap )
{
    if ( bSwap )
    {
        UINT32 nColor = nSOColor & 0xff00;      // GRUEN
        nColor |= (BYTE)( nSOColor ) << 16;     // ROT
        nColor |= (BYTE)( nSOColor >> 16 );     // BLAU
        return nColor;
    }
    else
        return nSOColor & 0xffffff;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::GetColor( const Color& rSOColor, BOOL bSwap )
{
    UINT32 nColor = ( rSOColor.GetRed() << 16 );
    nColor |= ( rSOColor.GetGreen() << 8 );
    nColor |= rSOColor.GetBlue();

    if ( !bSwap )
        nColor = GetColor( nColor, TRUE );

    return nColor;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::GetGradientColor( const awt::Gradient* pVCLGradient, UINT32 nStartColor )
{
    UINT32 nIntensity;
    Color   aColor;
    if ( nStartColor & 1 )
    {
        nIntensity = pVCLGradient->StartIntensity;
        aColor = pVCLGradient->StartColor;
    }
    else
    {
        nIntensity = pVCLGradient->EndIntensity;
        aColor = pVCLGradient->EndColor;
    }
    UINT32 nRed = ( ( aColor.GetRed() * nIntensity ) / 100 );
    UINT32 nGreen = ( ( aColor.GetGreen() * nIntensity ) / 100 ) << 8;
    UINT32 nBlue = ( ( aColor.GetBlue() * nIntensity ) / 100 ) << 16;
    return nRed | nGreen | nBlue;
}

// ---------------------------------------------------------------------------------------------

void EscherEx::WriteGradient( const awt::Gradient* pVCLGradient )
{
    UINT32 nFillFocus = 0x64;
    UINT32 nFirstColor = 0;


    switch ( pVCLGradient->Style )
    {
        default:
        case GradientStyle_LINEAR :
        {
        }
        break;
        case GradientStyle_AXIAL :
        {
            nFillFocus = 0x32;
            nFirstColor = 1;
        }
        break;
        case GradientStyle_RADIAL :
        {
        }
        break;
        case GradientStyle_ELLIPTICAL :
        {
        }
        break;
        case GradientStyle_SQUARE :
        {
        }
        break;
        case GradientStyle_RECT :
        {
        }
        break;
    }
    AddOpt( ESCHER_Prop_fillType, ESCHER_FillShadeScale );
    AddOpt( ESCHER_Prop_fillAngle, ( ( -3600 + pVCLGradient->Angle ) << 16 ) / 10 );
    AddOpt( ESCHER_Prop_fillColor, GetGradientColor( pVCLGradient, nFirstColor ) );
    AddOpt( ESCHER_Prop_fillBackColor, GetGradientColor( pVCLGradient, nFirstColor ^ 1 ) );
    AddOpt( ESCHER_Prop_fillFocus, nFillFocus );
};

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::AddGraphic( SvStream& rStrm, const Graphic& rGraphic )
{
    mpPicOutStrm = &rStrm;
    UINT32 nId = 0;
    switch( rGraphic.GetType() )
    {
    case GRAPHIC_BITMAP:
        {
            SvMemoryStream aDestStrm;
            if( ERRCODE_NONE == GraphicConverter::Export( aDestStrm,
                    rGraphic, CVT_PNG ) )
                nId = ImplGetBlibID( aDestStrm, PNG );
        }
        break;

    case GRAPHIC_GDIMETAFILE:
        {
            SvMemoryStream aGrfStrm;
            WriteWindowMetafile( aGrfStrm, rGraphic.GetGDIMetaFile() );
            const BYTE* pMem = (BYTE*)aGrfStrm.GetData();
            UINT32 nLen = aGrfStrm.GetSize();

             const MapMode aMap100mm( MAP_100TH_MM );
            Size aSize( rGraphic.GetPrefSize() );
            if( MAP_PIXEL == rGraphic.GetPrefMapMode().GetMapUnit() )
                aSize = Application::GetDefaultDevice()->PixelToLogic( aSize,
                                    aMap100mm );
            else
                aSize = OutputDevice::LogicToLogic( aSize,
                                    rGraphic.GetPrefMapMode(), aMap100mm );

            Rectangle aRect( Point(0,0), aSize );

            nId = AddWMF( *mpPicOutStrm, pMem + 22, nLen - 22, aRect );
        }
        break;
    }
    return nId;
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::ImplAddMetafile( SvStream& rStrm, const BYTE* pSource,
            UINT32 nSize, const Rectangle& rRect, ESCHER_BlibType eType )
{
    ESCHER_GDIStruct aGDIStruct;
    aGDIStruct.GDIBoundRect = rRect;
    aGDIStruct.GDISize = rRect.GetSize();
    aGDIStruct.GDIUncompressedSize = nSize;
    mpPicOutStrm = &rStrm;
    ZCodec aZCodec( 0x8000, 0x8000 );
    aZCodec.BeginCompression();
    SvMemoryStream aDestStrm;
    aZCodec.Write( aDestStrm, pSource, nSize );
    aZCodec.EndCompression();
    return ImplGetBlibID( aDestStrm, eType, &aGDIStruct );
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::AddWMF( SvStream& rStrm, const BYTE* pSource, UINT32 nSize, const Rectangle& rRect )
{
    return ImplAddMetafile( rStrm, pSource, nSize, rRect, WMF );
}

// ---------------------------------------------------------------------------------------------

UINT32 EscherEx::AddEMF( SvStream& rStrm, const BYTE* pSource, UINT32 nSize, const Rectangle& rRect )
{
    return ImplAddMetafile( rStrm, pSource, nSize, rRect, EMF );
}

// ---------------------------------------------------------------------------------------------

void EscherEx::SetNewBlipStreamOffset( INT32 nOffset )
{
    for( UINT32 i = 0; i < mnBlibEntrys; i++ )
    {
        EscherBlibEntry* pBlibEntry = mpBlibEntrys[ i ];
        pBlibEntry->mnPictureOffset += nOffset;
    }
}


