/*************************************************************************
 *
 *  $RCSfile: escherex.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sj $ $Date: 2000-12-07 16:52:31 $
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


#ifndef __EscherEX_HXX
#include "escherex.hxx"
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _ZCODEC_HXX
#include <tools/zcodec.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _INC_STDLIB
#include <stdlib.h>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

_EscherEx::_EscherEx( SvStorageStream& rOutStrm, UINT32 nDrawings ) :
    mpOutStrm               ( &rOutStrm ),
    mpGraphicProvider       ( NULL ),
    mpOffsets               ( new UINT32[ 32 ] ),
    mpRecTypes              ( new UINT16[ 32 ] ),
    mnDrawings              ( nDrawings ),
    mnLevel                 ( 0 ),
    mb_EscherSpgr           ( FALSE ),
    mb_EscherDg             ( FALSE ),
    mpSortStruct            ( NULL ),
    mnGroupLevel            ( 0 ),
    mnFIDCLs                ( nDrawings ),
    mnCurrentDg             ( 0 ),
    mnCurrentShapeID        ( 0 ),
    mnTotalShapesDgg        ( 0 ),
    mnCurrentShapeMaximumID ( 0 )
{
    mnStrmStartOfs = mpOutStrm->Tell();
}

// ---------------------------------------------------------------------------------------------

UINT32 _EscherEx::DrawingGroupContainerSize()
{
    return ImplDggContainerSize() + 8;
}

void _EscherEx::WriteDrawingGroupContainer( SvStream& rSt )
{
    UINT32 nSize = DrawingGroupContainerSize();
    rSt << (UINT32)( 0xf | ( 1035 << 16 ) )     // EPP_PPDrawingGroup
        << (UINT32)( nSize - 8 );

    ImplWriteDggContainer( rSt );
}

// ---------------------------------------------------------------------------------------------

UINT32 _EscherEx::ImplDggContainerSize()
{
    UINT32 nSize;

    nSize  = ImplDggAtomSize();
    nSize += ImplBlibStoreContainerSize();
    nSize += ImplOptAtomSize();
    nSize += ImplSplitMenuColorsAtomSize();

    return nSize + 8;
}

void _EscherEx::ImplWriteDggContainer( SvStream& rSt )
{
    UINT32 nSize = ImplDggContainerSize();
    if ( nSize )
    {
        rSt << (UINT32)( 0xf | ( ESCHER_DggContainer << 16 ) )
            << (UINT32)( nSize - 8 );

        ImplWriteDggAtom( rSt );
        ImplWriteBlibStoreContainer( rSt );
        ImplWriteOptAtom( rSt );
        ImplWriteSplitMenuColorsAtom( rSt );
    }
}

// ---------------------------------------------------------------------------------------------

UINT32 _EscherEx::ImplDggAtomSize()
{
    return maFIDCLs.Tell() + 24;
}

void _EscherEx::ImplWriteDggAtom( SvStream& rSt )
{
    UINT32 nSize = ImplDggAtomSize();
    if ( nSize )
    {
        rSt << (UINT32)( ESCHER_Dgg << 16 )
            << (UINT32)( nSize - 8 )
            << mnCurrentShapeID
            << (UINT32)( mnFIDCLs + 1 )
            << mnTotalShapesDgg
            << mnDrawings;

        rSt.Write( maFIDCLs.GetData(), nSize - 24 );
    }
}

// ---------------------------------------------------------------------------------------------

UINT32 _EscherEx::ImplBlibStoreContainerSize()
{
    UINT32 nSize = 0;
    if ( mpGraphicProvider && mpGraphicProvider->mnBlibEntrys )
        nSize = 44 * mpGraphicProvider->mnBlibEntrys + 8;
    return nSize;
}

void _EscherEx::ImplWriteBlibStoreContainer( SvStream& rSt )
{
    UINT32 nSize = ImplBlibStoreContainerSize();
    if ( nSize )
    {
        rSt << (UINT32)( ( ESCHER_BstoreContainer << 16 ) | 0x1f )
            << (UINT32)( nSize - 8 );

        for ( UINT32 i = 0; i < mpGraphicProvider->mnBlibEntrys; i++ )
        {
            EscherBlibEntry* pBlibEntry = mpGraphicProvider->mpBlibEntrys[ i ];
            BYTE nBlibType = pBlibEntry->meBlibType;
            AddAtom( 36, ESCHER_BSE, 2, nBlibType );
            rSt << nBlibType;
            if ( ( nBlibType == WMF ) || ( nBlibType == EMF ) )
            {                                                       // WMF auf OS2 zu Pict Konvertieren
                rSt << (BYTE)4;
                rSt.Write( &pBlibEntry->mnIdentifier[ 0 ], 16 );
                rSt << (UINT16)0 << (UINT32)( pBlibEntry->mnSize + 0x4a ) << (UINT32)( i + 1 ) << pBlibEntry->mnPictureOffset << (UINT32)0;
            }
            else
            {
                rSt << nBlibType;
                rSt.Write( &pBlibEntry->mnIdentifier[ 0 ], 16 );
                rSt << (UINT16)0 << pBlibEntry->mnSize << (UINT32)( i + 1 ) << pBlibEntry->mnPictureOffset << (UINT32)0;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------

#define ESCHER_OPT_COUNT 6

UINT32 _EscherEx::ImplOptAtomSize()
{
    UINT32 nSize = 0;
    if ( ESCHER_OPT_COUNT )
        nSize = ( ESCHER_OPT_COUNT * 6 ) + 8;
    return nSize;
}

void _EscherEx::ImplWriteOptAtom( SvStream& rSt )
{
    UINT32 nSize = ImplOptAtomSize();
    if ( nSize )
    {
        rSt << (UINT32)( ( ESCHER_OPT << 16 ) | ( ESCHER_OPT_COUNT << 4 ) | 0x3 )
            << (UINT32)( nSize - 8 )
            << (UINT16)ESCHER_Prop_fillColor            << (UINT32)0xffb800
            << (UINT16)ESCHER_Prop_fillBackColor        << (UINT32)0
            << (UINT16)ESCHER_Prop_fNoFillHitTest       << (UINT32)0x00100010
            << (UINT16)ESCHER_Prop_lineColor            << (UINT32)0x8000001
            << (UINT16)ESCHER_Prop_fNoLineDrawDash      << (UINT32)0x00080008
            << (UINT16)ESCHER_Prop_shadowColor          << (UINT32)0x8000002;
    }

}

// ---------------------------------------------------------------------------------------------

#define ESCHER_SPLIT_MENU_COLORS_COUNT  4

UINT32 _EscherEx::ImplSplitMenuColorsAtomSize()
{
    UINT32 nSize = 0;
    if ( ESCHER_SPLIT_MENU_COLORS_COUNT )
        nSize = ( ESCHER_SPLIT_MENU_COLORS_COUNT << 2 ) + 8;
    return nSize;
}

void _EscherEx::ImplWriteSplitMenuColorsAtom( SvStream& rSt )
{
    UINT32 nSize = ImplSplitMenuColorsAtomSize();
    if ( nSize )
    {
        rSt << (UINT32)( ( ESCHER_SplitMenuColors << 16 ) | ( ESCHER_SPLIT_MENU_COLORS_COUNT << 4 ) )
            << (UINT32)( nSize - 8 )
            << (UINT32)0x08000004
            << (UINT32)0x08000001
            << (UINT32)0x08000002
            << (UINT32)0x100000f7;
    }

}

// ---------------------------------------------------------------------------------------------

_EscherEx::~_EscherEx()
{
    delete mpGraphicProvider;
    delete[] mpSortStruct;
    delete[] mpRecTypes;
    delete[] mpOffsets;
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::InsertAtCurrentPos( UINT32 nBytes, BOOL bContainer )
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

BOOL _EscherEx::SeekBehindRecHeader( UINT16 nRecType )
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

void _EscherEx::InsertPersistOffset( UINT32 nKey, UINT32 nOffset )
{
    PtInsert( ESCHER_Persist_PrivateEntry | nKey, nOffset );
}

// ---------------------------------------------------------------------------------------------

BOOL _EscherEx::ImplSeek( UINT32 nKey )
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

BOOL _EscherEx::SeekToPersistOffset( UINT32 nKey )
{
    return ImplSeek( ESCHER_Persist_PrivateEntry | nKey );
}

// ---------------------------------------------------------------------------------------------

BOOL _EscherEx::InsertAtPersistOffset( UINT32 nKey, UINT32 nValue )
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

void _EscherEx::OpenContainer( UINT16 n_EscherContainer, int nRecInstance )
{
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | 0xf  ) << n_EscherContainer << (UINT32)0;
    mpOffsets[ ++mnLevel ] = mpOutStrm->Tell() - 4;
    mpRecTypes[ mnLevel ] = n_EscherContainer;

    switch( n_EscherContainer )
    {
        case ESCHER_DgContainer :
        {
            if ( !mb_EscherDg )
            {
                mb_EscherDg = TRUE;
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
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mb_EscherDg )
            {
                mb_EscherSpgr = TRUE;
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

void _EscherEx::CloseContainer()
{
    UINT32 nSize, nPos = mpOutStrm->Tell();
    nSize = ( nPos - mpOffsets[ mnLevel ] ) - 4;
    mpOutStrm->Seek( mpOffsets[ mnLevel ] );
    *mpOutStrm << nSize;

    switch( mpRecTypes[ mnLevel ] )
    {
        case ESCHER_DgContainer :
        {
            if ( mb_EscherDg )
            {
                mb_EscherDg = FALSE;
                if ( ImplSeek( ESCHER_Persist_Dg | mnCurrentDg ) )
                {
                    // shapeanzahl des drawings setzen
                    mnTotalShapesDgg += mnTotalShapesDg;
                    *mpOutStrm << mnTotalShapesDg << mnCurrentShapeMaximumID;

                    if ( !mnTotalShapesDg )
                    {
                        maFIDCLs << (UINT32)0
                                 << (UINT32)0;
                    }
                    else
                    {
                        if ( mnTotalShapeIdUsedDg )
                        {
                            UINT32 i, nFIDCL = ( ( mnTotalShapeIdUsedDg - 1 ) / 0x400 );
                            if ( nFIDCL )
                                mnFIDCLs += nFIDCL;
                            for ( i = 0; i <= nFIDCL; i++ )
                            {
                                maFIDCLs << mnCurrentDg;            // drawing number
                                if ( i < nFIDCL )
                                    maFIDCLs << 0x400;
                                else
                                {
                                    UINT32 nShapesLeft = mnTotalShapeIdUsedDg % 0x400;
                                    if ( !nShapesLeft )
                                        nShapesLeft = 0x400;        // shape count in this IDCL
                                    maFIDCLs << (UINT32)nShapesLeft;
                                }
                            }
                        }
                    }
                }
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mb_EscherSpgr )
            {
                mb_EscherSpgr = FALSE;

            }
        }
        break;

        default:
        break;
    }
    --mnLevel;
    mpOutStrm->Seek( nPos );
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::BeginAtom()
{
    mnCountOfs = mpOutStrm->Tell();
    *mpOutStrm << (UINT32)0 << (UINT32)0;       // record header wird spaeter geschrieben
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::EndAtom( UINT16 nRecType, int nRecVersion, int nRecInstance )
{
    UINT32  nOldPos = mpOutStrm->Tell();
    mpOutStrm->Seek( mnCountOfs );
    mnCountSize = nOldPos - mnCountOfs;
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ) << nRecType << (UINT32)( mnCountSize - 8 );
    mpOutStrm->Seek( nOldPos );
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::AddAtom( UINT32 nAtomSize, UINT16 nRecType, int nRecVersion, int nRecInstance )
{
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | ( nRecVersion & 0xf ) ) << nRecType << nAtomSize;
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::AddClientAnchor( const Rectangle& rRect )
{
    AddAtom( 8, ESCHER_ClientAnchor );
    *mpOutStrm << (INT16)rRect.Top() << (INT16)rRect.Left() << (INT16)rRect.Right() << (INT16)rRect.Bottom();
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::EnterGroup( Rectangle* pBoundRect, SvMemoryStream* pClientData )
{
    Rectangle aRect;
    if ( pBoundRect )
        aRect = *pBoundRect;

    OpenContainer( ESCHER_SpgrContainer );
    OpenContainer( ESCHER_SpContainer );
    AddAtom( 16, ESCHER_Spgr, 1 );
    PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel, mpOutStrm->Tell() );
    *mpOutStrm  << (INT32)aRect.Left()  // Bounding box fuer die Gruppierten shapes an die sie attached werden
                << (INT32)aRect.Top()
                << (INT32)aRect.Right()
                << (INT32)aRect.Bottom();

    if ( !mnGroupLevel )
        AddShape( ESCHER_ShpInst_Min, 5 );                              // Flags: Group | Patriarch
    else
    {
        AddShape( ESCHER_ShpInst_Min, 0x201 );                          // Flags: Group | HaveAnchor
        AddAtom( 8, ESCHER_ClientAnchor );
        PtReplaceOrInsert( ESCHER_Persist_Grouping_Logic | mnGroupLevel, mpOutStrm->Tell() );
        *mpOutStrm << (INT16)aRect.Top() << (INT16)aRect.Left() << (INT16)aRect.Right() << (INT16)aRect.Bottom();
    }
    if ( pClientData )
    {
        pClientData->Seek( STREAM_SEEK_TO_END );
        sal_uInt32 nSize = pClientData->Tell();
        if ( nSize )
        {
            *mpOutStrm << (sal_uInt32)( ( ESCHER_ClientData << 16 ) | 0xf )
                       << nSize;
            mpOutStrm->Write( pClientData->GetData(), nSize );
        }
    }
    CloseContainer();                                               // ESCHER_SpContainer
    mnGroupLevel++;
}

// ---------------------------------------------------------------------------------------------

BOOL _EscherEx::SetGroupSnapRect( UINT32 nGroupLevel, const Rectangle& rRect )
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

BOOL _EscherEx::SetGroupLogicRect( UINT32 nGroupLevel, const Rectangle& rRect )
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

void _EscherEx::LeaveGroup()
{
    --mnGroupLevel;
    PtDelete( ESCHER_Persist_Grouping_Snap | mnGroupLevel );
    PtDelete( ESCHER_Persist_Grouping_Logic | mnGroupLevel );
    CloseContainer();
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::AddShape( UINT32 nShpInstance, UINT32 nFlags, UINT32 nShapeID )
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

    if ( mb_EscherSpgr )
        mnTotalShapesDg++;
}

// ---------------------------------------------------------------------------------------------

UINT32 _EscherEx::GetShapeID()
{
    mnCurrentShapeMaximumID = mnCurrentShapeID; // maximum setzen
    mnCurrentShapeID++;                         // mnCurrentShape ID auf nachste freie ID
    mnTotalShapeIdUsedDg++;
    return mnCurrentShapeMaximumID;
}

// ---------------------------------------------------------------------------------------------

extern "C" int __LOADONCALLAPI _EscherPropSortFunc( const void* p1, const void* p2 )
{
    INT16   nID1 = ((_EscherPropSortStruct*)p1)->nPropId &~0xc000;
    INT16   nID2 = ((_EscherPropSortStruct*)p2)->nPropId &~0xc000;

    if( nID1  < nID2 )
        return -1;
    else if( nID1 > nID2 )
        return 1;
    else
        return 0;
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::BeginCount()
{
    mnCountOfs = mpOutStrm->Tell();
    *mpOutStrm << (UINT32)0 << (UINT32)0;       // record header wird spaeter geschrieben
    mnSortCount = 0;                            // normale Properties werden automatisch sortiert
    mnCountCount = 0;
    mnCountSize = 8;

    if ( !mpSortStruct )
        mpSortStruct = new _EscherPropSortStruct[ mnSortBufSize = 64 ];
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::AddOpt( UINT16 nPropID, UINT32 nPropValue, BOOL bBlib )
{
    AddOpt( nPropID, bBlib, nPropValue, NULL, 0 );
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::AddOpt( UINT16 nPropID, BOOL bBlib, UINT32 nPropValue, BYTE* pProp, UINT32 nPropSize )
{
    if ( bBlib )                // bBlib is only valid when fComplex = 0
        nPropID |= 0x4000;
    if ( pProp )
        nPropID |= 0x8000;      // fComplex = TRUE;

    for( UINT32 i = 0; i < mnSortCount; i++ )
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
        _EscherPropSortStruct* pTemp = new _EscherPropSortStruct[ mnSortBufSize ];
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

BOOL _EscherEx::GetOpt( UINT16 nPropId, UINT32& nPropValue )
{
    BOOL bRetValue = FALSE;
    for( UINT32 i = 0; i < mnSortCount; i++ )
    {
        if ( ( mpSortStruct[ i ].nPropId &~0xc000 ) == ( nPropId &~0xc000 ) )   // pruefen, ob Property nur ersetzt wird
        {
            nPropValue = mpSortStruct[ i ].nPropValue;
            bRetValue = TRUE;
            break;
        }
    }
    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::AddColor( UINT32 nColor )
{
    mnCountCount++;
    mnCountSize += 4;
    *mpOutStrm << nColor;
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::EndCount( UINT16 nRecType, UINT16 nRecVersion )
{
    UINT32  nOldPos = mpOutStrm->Tell();
    mpOutStrm->Seek( mnCountOfs );
    *mpOutStrm << (UINT16)( ( mnCountCount << 4 ) | ( nRecVersion & 0xf ) ) << (UINT16)nRecType << (UINT32)( mnCountSize - 8 );
    mpOutStrm->Seek( nOldPos );
    if ( mnSortCount )
    {
        qsort( mpSortStruct, mnSortCount, sizeof( _EscherPropSortStruct ), _EscherPropSortFunc );
        BOOL bComplex = FALSE;
        for ( UINT32 i = 0; i < mnSortCount; i++ )
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

UINT32 _EscherEx::GetColor( const UINT32 nSOColor, BOOL bSwap )
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

UINT32 _EscherEx::GetColor( const Color& rSOColor, BOOL bSwap )
{
    UINT32 nColor = ( rSOColor.GetRed() << 16 );
    nColor |= ( rSOColor.GetGreen() << 8 );
    nColor |= rSOColor.GetBlue();

    if ( !bSwap )
        nColor = GetColor( nColor, TRUE );

    return nColor;
}

// ---------------------------------------------------------------------------------------------

UINT32 _EscherEx::GetGradientColor( const ::com::sun::star::awt::Gradient* pGradient, UINT32 nStartColor )
{
    UINT32 nIntensity;
    Color   aColor;
    if ( nStartColor & 1 )
    {
        nIntensity = pGradient->StartIntensity;
        aColor = pGradient->StartColor;
    }
    else
    {
        nIntensity = pGradient->EndIntensity;
        aColor = pGradient->EndColor;
    }
    UINT32 nRed = ( ( aColor.GetRed() * nIntensity ) / 100 );
    UINT32 nGreen = ( ( aColor.GetGreen() * nIntensity ) / 100 ) << 8;
    UINT32 nBlue = ( ( aColor.GetBlue() * nIntensity ) / 100 ) << 16;
    return nRed | nGreen | nBlue;
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::WriteGradient( const ::com::sun::star::awt::Gradient* pGradient )
{
    UINT32 nFillFocus = 0x64;
    UINT32 nFirstColor = 0;


    switch ( pGradient->Style )
    {
        default:
        case ::com::sun::star::awt::GradientStyle_LINEAR :
        {
        }
        break;
        case ::com::sun::star::awt::GradientStyle_AXIAL :
        {
            nFillFocus = 0x32;
            nFirstColor = 1;
        }
        break;
        case ::com::sun::star::awt::GradientStyle_RADIAL :
        {
        }
        break;
        case ::com::sun::star::awt::GradientStyle_ELLIPTICAL :
        {
        }
        break;
        case ::com::sun::star::awt::GradientStyle_SQUARE :
        {
        }
        break;
        case ::com::sun::star::awt::GradientStyle_RECT :
        {
        }
        break;
    }
    AddOpt( ESCHER_Prop_fillType, ESCHER_FillShadeScale );
    AddOpt( ESCHER_Prop_fillAngle, ( ( -3600 + pGradient->Angle ) << 16 ) / 10 );
    AddOpt( ESCHER_Prop_fillColor, GetGradientColor( pGradient, nFirstColor ) );
    AddOpt( ESCHER_Prop_fillBackColor, GetGradientColor( pGradient, nFirstColor ^ 1 ) );
    AddOpt( ESCHER_Prop_fillFocus, nFillFocus );
};

// ---------------------------------------------------------------------------------------------

UINT32 _EscherEx::AddGraphic( SvStorageStream& rStrm, const ByteString& rUniqueId,
                                const Rectangle& rBoundRect, const GraphicAttr* pGraphicAttr )
{
    sal_uInt32 nBlibId = 0;
    if ( rUniqueId.Len() )
    {
        if ( !mpGraphicProvider )
            mpGraphicProvider = new EscherGraphicProvider();

        nBlibId = mpGraphicProvider->GetBlibID( rStrm, rUniqueId, rBoundRect, pGraphicAttr );
    }
    return nBlibId;
}
