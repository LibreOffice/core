/*************************************************************************
 *
 *  $RCSfile: escherex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2000-11-03 18:01:54 $
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
#ifndef _INC_STDLIB
#include <stdlib.h>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif

// ---------------------------------------------------------------------------------------------

_EscherPersistTable::_EscherPersistTable()
{
}

// ---------------------------------------------------------------------------------------------

_EscherPersistTable::~_EscherPersistTable()
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
        delete (_EscherPersistEntry*)pPtr;
}

// ---------------------------------------------------------------------------------------------

BOOL _EscherPersistTable::PtIsID( UINT32 nID )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((_EscherPersistEntry*)pPtr)->mnID == nID )
            return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

void _EscherPersistTable::PtInsert( UINT32 nID, UINT32 nOfs )
{
    maPersistTable.Insert( new _EscherPersistEntry( nID, nOfs ) );
}

// ---------------------------------------------------------------------------------------------

UINT32 _EscherPersistTable::PtDelete( UINT32 nID )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((_EscherPersistEntry*)pPtr)->mnID == nID )
        {
            UINT32 nRetValue = ((_EscherPersistEntry*)pPtr)->mnOffset;
            delete (_EscherPersistEntry*)maPersistTable.Remove();
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------

UINT32 _EscherPersistTable::PtGetOffsetByID( UINT32 nID )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((_EscherPersistEntry*)pPtr)->mnID == nID )
            return ((_EscherPersistEntry*)pPtr)->mnOffset;
    }
    return 0;
};

// ---------------------------------------------------------------------------------------------

UINT32 _EscherPersistTable::PtReplace( UINT32 nID, UINT32 nOfs )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((_EscherPersistEntry*)pPtr)->mnID == nID )
        {
            UINT32 nRetValue = ((_EscherPersistEntry*)pPtr)->mnOffset;
            ((_EscherPersistEntry*)pPtr)->mnOffset = nOfs;
            return nRetValue;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------

UINT32 _EscherPersistTable::PtReplaceOrInsert( UINT32 nID, UINT32 nOfs )
{
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        if ( ((_EscherPersistEntry*)pPtr)->mnID == nID )
        {
            UINT32 nRetValue = ((_EscherPersistEntry*)pPtr)->mnOffset;
            ((_EscherPersistEntry*)pPtr)->mnOffset = nOfs;
            return nRetValue;
        }
    }
    PtInsert( nID, nOfs );
    return 0;
}

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

_EscherBlibEntry::_EscherBlibEntry( sal_uInt32 nPictureOffset, const GraphicObject& rObject, const ByteString& rId,
                                        const GraphicAttr* pGraphicAttr ) :
    mbIsEmpty       ( TRUE ),
    mnPictureOffset ( nPictureOffset )
{
    mbIsNativeGraphicPossible = ( pGraphicAttr == NULL );
    meBlibType = UNKNOWN;
    mnSize = 0;

    sal_uInt32      nLen = rId.Len();
    const sal_Char* pData = rId.GetBuffer();
    GraphicType     eType( rObject.GetType() );
    if ( nLen && pData && ( eType != GRAPHIC_NONE ) )
    {
        mnIdentifier[ 0 ] = rtl_crc32( 0,pData, nLen );

        mnIdentifier[ 1 ] = 0;

        if ( pGraphicAttr )
        {
            if ( pGraphicAttr->IsSpecialDrawMode()
                    || pGraphicAttr->IsMirrored()
                         || pGraphicAttr->IsCropped()
                            || pGraphicAttr->IsRotated()
                                || pGraphicAttr->IsTransparent()
                                    || pGraphicAttr->IsAdjusted() )
            {
                SvMemoryStream aSt( sizeof( GraphicAttr ) );
                aSt << (sal_uInt16)pGraphicAttr->GetDrawMode()
                    << pGraphicAttr->GetMirrorFlags()
                    << pGraphicAttr->GetLeftCrop()
                    << pGraphicAttr->GetTopCrop()
                    << pGraphicAttr->GetRightCrop()
                    << pGraphicAttr->GetBottomCrop()
                    << pGraphicAttr->GetRotation()
                    << pGraphicAttr->GetLuminance()
                    << pGraphicAttr->GetContrast()
                    << pGraphicAttr->GetChannelR()
                    << pGraphicAttr->GetChannelG()
                    << pGraphicAttr->GetChannelB()
                    << pGraphicAttr->GetGamma()
                    << (BOOL)( pGraphicAttr->IsInvert() == TRUE )
                    << pGraphicAttr->GetTransparency();
                mnIdentifier[ 1 ] = rtl_crc32( 0, aSt.GetData(), aSt.Tell() );
            }
            else
                mbIsNativeGraphicPossible = TRUE;
        }
        sal_uInt32 i, nTmp, n1, n2;
        n1 = n2 = 0;
        for ( i = 0; i < nLen; i++ )
        {
            nTmp = n2 >> 28;    // rotating 4 bit
            n2 <<= 4;
            n2 |= n1 >> 28;
            n1 <<= 4;
            n1 |= nTmp;
            n1 ^= *pData++ - '0';
        }
        mnIdentifier[ 2 ] = n1;
        mnIdentifier[ 3 ] = n2;
        mbIsEmpty = FALSE;
    }
};

// ---------------------------------------------------------------------------------------------

_EscherBlibEntry::~_EscherBlibEntry()
{
};

// ---------------------------------------------------------------------------------------------

BOOL _EscherBlibEntry::operator==( const _EscherBlibEntry& r_EscherBlibEntry ) const
{
    for ( int i = 0; i < 3; i++ )
    {
        if ( mnIdentifier[ i ] != r_EscherBlibEntry.mnIdentifier[ i ] )
            return FALSE;
    }
    return TRUE;
}

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
        rSt << (UINT32)( 0xf | ( _Escher_DggContainer << 16 ) )
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
        rSt << (UINT32)( _Escher_Dgg << 16 )
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
        rSt << (UINT32)( ( _Escher_BstoreContainer << 16 ) | 0x1f )
            << (UINT32)( nSize - 8 );

        for ( UINT32 i = 0; i < mpGraphicProvider->mnBlibEntrys; i++ )
        {
            _EscherBlibEntry* pBlibEntry = mpGraphicProvider->mpBlibEntrys[ i ];
            BYTE nBlibType = pBlibEntry->meBlibType;
            AddAtom( 36, _Escher_BSE, 2, nBlibType );
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
        rSt << (UINT32)( ( _Escher_OPT << 16 ) | ( ESCHER_OPT_COUNT << 4 ) | 0x3 )
            << (UINT32)( nSize - 8 )
            << (UINT16)_Escher_Prop_fillColor           << (UINT32)0xffb800
            << (UINT16)_Escher_Prop_fillBackColor       << (UINT32)0
            << (UINT16)_Escher_Prop_fNoFillHitTest      << (UINT32)0x00100010
            << (UINT16)_Escher_Prop_lineColor           << (UINT32)0x8000001
            << (UINT16)_Escher_Prop_fNoLineDrawDash     << (UINT32)0x00080008
            << (UINT16)_Escher_Prop_shadowColor         << (UINT32)0x8000002;
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
        rSt << (UINT32)( ( _Escher_SplitMenuColors << 16 ) | ( ESCHER_SPLIT_MENU_COLORS_COUNT << 4 ) )
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

_EscherGraphicProvider::_EscherGraphicProvider( SvStream& rStream, UINT32 nFlags ) :
    mrPicOutStrm    ( rStream ),
    mnFlags         ( nFlags ),
    mpBlibEntrys    ( NULL ),
    mnBlibBufSize   ( 0 ),
    mnBlibEntrys    ( 0 )
{
}

_EscherGraphicProvider::~_EscherGraphicProvider()
{
    for ( UINT32 i = 0; i < mnBlibEntrys; delete mpBlibEntrys[ i++ ] );
    delete mpBlibEntrys;
}

UINT32 _EscherGraphicProvider::ImplInsertBlib( _EscherBlibEntry* p_EscherBlibEntry )
{
    if ( mnBlibBufSize == mnBlibEntrys )
    {
        mnBlibBufSize += 64;
        _EscherBlibEntry** pTemp = new _EscherBlibEntry*[ mnBlibBufSize ];
        for ( UINT32 i = 0; i < mnBlibEntrys; i++ )
        {
            pTemp[ i ] = mpBlibEntrys[ i ];
        }
        delete mpBlibEntrys;
        mpBlibEntrys = pTemp;
    }
    mpBlibEntrys[ mnBlibEntrys++ ] = p_EscherBlibEntry;
    return mnBlibEntrys;
}

sal_uInt32 _EscherGraphicProvider::ImplGetBlibID( const ByteString& rId, const Rectangle& rBoundRect,
                                                    const GraphicAttr* pGraphicAttr )
{
    sal_uInt32          nBlibId = 0;
    GraphicAttr*        pAttr = NULL;
    const GraphicAttr*  pAttrUsed = pGraphicAttr;
    GraphicObject       aGraphicObject( rId );

    if ( pAttrUsed
            && pAttrUsed->GetRotation()
                && ( aGraphicObject.GetType() == GRAPHIC_GDIMETAFILE )
                    && ( mnFlags & _E_GRAPH_PROV_DO_NOT_ROTATE_METAFILES ) )
    {
        pAttr = new GraphicAttr;
        *pAttr = *pAttrUsed;
        pAttr->SetRotation( 0 );
        pAttrUsed = pAttr;
    }
    _EscherBlibEntry* p_EscherBlibEntry = new _EscherBlibEntry( mrPicOutStrm.Tell(), aGraphicObject, rId, pAttrUsed );
    if ( !p_EscherBlibEntry->IsEmpty() )
    {
        for ( UINT32 i = 0; i < mnBlibEntrys; i++ )
        {
            if ( *( mpBlibEntrys[ i ] ) == *p_EscherBlibEntry )
            {
                delete p_EscherBlibEntry;
                return i + 1;
            }
        }

        sal_Bool            bUseNativeGraphic( FALSE );

        Graphic             aGraphic( aGraphicObject.GetTransformedGraphic( pAttrUsed ) );
        GfxLink             aGraphicLink;
        SvMemoryStream      aStream;

        const sal_uInt8*    pGraphicAry = NULL;

        if ( p_EscherBlibEntry->mbIsNativeGraphicPossible && aGraphic.IsLink() )
        {
            aGraphicLink = aGraphic.GetLink();

            p_EscherBlibEntry->mnSize = aGraphicLink.GetDataSize();
            pGraphicAry = aGraphicLink.GetData();

            if ( p_EscherBlibEntry->mnSize && pGraphicAry )
            {
                switch ( aGraphicLink.GetType() )
                {
                    case GFX_LINK_TYPE_NATIVE_JPG : p_EscherBlibEntry->meBlibType = PEG; break;
                    case GFX_LINK_TYPE_NATIVE_PNG : p_EscherBlibEntry->meBlibType = PNG; break;
                    case GFX_LINK_TYPE_NATIVE_WMF :
                    {
                        if ( pGraphicAry && ( p_EscherBlibEntry->mnSize > 0x2c ) )
                        {
                            if ( ( pGraphicAry[ 0x28 ] == 0x20 ) && ( pGraphicAry[ 0x29 ] == 0x45 )     // check the magic
                                && ( pGraphicAry[ 0x2a ] == 0x4d ) && ( pGraphicAry[ 0x2b ] == 0x46 ) ) // number ( emf detection )
                            {
                                p_EscherBlibEntry->meBlibType = EMF;
                            }
                            else
                            {
                                p_EscherBlibEntry->meBlibType = WMF;
                                if ( ( pGraphicAry[ 0 ] == 0xd7 ) && ( pGraphicAry[ 1 ] == 0xcd )
                                    && ( pGraphicAry[ 2 ] == 0xc6 ) && ( pGraphicAry[ 3 ] == 0x9a ) )
                                {   // we have to get rid of the metafileheader
                                    pGraphicAry += 22;
                                    p_EscherBlibEntry->mnSize -= 22;
                                }
                            }
                        }
                    }
                    break;
                }
                if ( p_EscherBlibEntry->meBlibType != UNKNOWN )
                    bUseNativeGraphic = TRUE;
            }
        }
        if ( !bUseNativeGraphic )
        {
            GraphicType eGraphicType = aGraphic.GetType();
            if ( ( eGraphicType == GRAPHIC_BITMAP ) || ( eGraphicType == GRAPHIC_GDIMETAFILE ) )
            {
                sal_uInt32 nErrCode = GraphicConverter::Export( aStream, aGraphic, ( eGraphicType == GRAPHIC_BITMAP ) ? CVT_PNG  : CVT_WMF );
                if ( nErrCode == ERRCODE_NONE )
                {
                    p_EscherBlibEntry->meBlibType = ( eGraphicType == GRAPHIC_BITMAP ) ? PNG : WMF;
                    aStream.Seek( STREAM_SEEK_TO_END );
                    p_EscherBlibEntry->mnSize = aStream.Tell();
                    pGraphicAry = (sal_uInt8*)aStream.GetData();

                    if ( p_EscherBlibEntry->meBlibType == WMF )     // the fileheader is not used
                    {
                        p_EscherBlibEntry->mnSize -= 22;
                        pGraphicAry += 22;
                    }
                }
            }
        }
        if ( p_EscherBlibEntry->mnSize && pGraphicAry && ( p_EscherBlibEntry->meBlibType != UNKNOWN ) )
        {
            sal_uInt32 nAtomSize = 0;
            sal_uInt32 nInstance, nUncompressedSize = p_EscherBlibEntry->mnSize;

            if ( mnFlags & _E_GRAPH_PROV_USE_INSTANCES )
            {
                mrPicOutStrm << (UINT32)( 0x7f90000 | (UINT16)( mnBlibEntrys << 4 ) )
                             << (UINT32)0;
                nAtomSize = mrPicOutStrm.Tell();
                 if ( p_EscherBlibEntry->meBlibType == PNG )
                    mrPicOutStrm << (UINT16)0x0606;
                else if ( p_EscherBlibEntry->meBlibType == WMF )
                    mrPicOutStrm << (UINT16)0x0403;
            }
            if ( ( p_EscherBlibEntry->meBlibType == PEG ) || ( p_EscherBlibEntry->meBlibType == PNG ) )
            {
                nInstance = ( p_EscherBlibEntry->meBlibType == PNG ) ? 0xf01e6e00 : 0xf01d46a0;
                mrPicOutStrm << nInstance << (sal_uInt32)( p_EscherBlibEntry->mnSize + 17 );
                mrPicOutStrm.Write( p_EscherBlibEntry->mnIdentifier, 16 );
                mrPicOutStrm << (BYTE)0xff;
                mrPicOutStrm.Write( pGraphicAry, p_EscherBlibEntry->mnSize );
            }
            else
            {
                ZCodec aZCodec( 0x8000, 0x8000 );
                aZCodec.BeginCompression();
                SvMemoryStream aDestStrm;
                aZCodec.Write( aDestStrm, pGraphicAry, p_EscherBlibEntry->mnSize );
                aZCodec.EndCompression();
                aDestStrm.Seek( STREAM_SEEK_TO_END );
                p_EscherBlibEntry->mnSize = aDestStrm.Tell();
                pGraphicAry = (sal_uInt8*)aDestStrm.GetData();
                if ( p_EscherBlibEntry->mnSize && pGraphicAry )
                {
                    nInstance = ( p_EscherBlibEntry->meBlibType == WMF ) ? 0xf01b2170 : 0xf01a3d50;
                    mrPicOutStrm << nInstance << (sal_uInt32)( p_EscherBlibEntry->mnSize + 0x42 );
                    mrPicOutStrm.Write( p_EscherBlibEntry->mnIdentifier, 16 );
                    mrPicOutStrm.Write( p_EscherBlibEntry->mnIdentifier, 16 );
                    UINT32 nWidth = rBoundRect.GetWidth() * 360;
                    UINT32 nHeight = rBoundRect.GetHeight() * 360;
                    double fWidth = (double)rBoundRect.GetWidth() / 10000.0 * 1027.0;
                    double fHeight = (double)rBoundRect.GetHeight() / 10000.0 * 1027.0;
                    mrPicOutStrm    << nUncompressedSize    // WMFSize ohne FileHeader
                                    << (sal_Int32)0         // da die Originalgroesse des WMF's (ohne FileHeader)
                                    << (sal_Int32)0         // nicht mehr feststellbar ist, schreiben wir 10cm / x
                                    << (sal_Int32)fWidth
                                    << (sal_Int32)fHeight
                                    << nWidth
                                    << nHeight
                                    << p_EscherBlibEntry->mnSize
                                    << (sal_uInt16)0xfe00;  // compression Flags
                    mrPicOutStrm.Write( pGraphicAry, p_EscherBlibEntry->mnSize );
                }
            }
            if ( nAtomSize )
            {
                sal_uInt32  nPos = mrPicOutStrm.Tell();
                mrPicOutStrm.Seek( nAtomSize - 4 );
                mrPicOutStrm << (sal_uInt32)( nPos - nAtomSize );
                mrPicOutStrm.Seek( nPos );
            }
            nBlibId = ImplInsertBlib( p_EscherBlibEntry ), p_EscherBlibEntry = NULL;
        }
    }
    if ( p_EscherBlibEntry )
        delete p_EscherBlibEntry;
    if ( pAttr )
        delete pAttr;
    return nBlibId;
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::InsertAtCurrentPos( UINT32 nBytes, BOOL bContainer )
{
    UINT32  i, nSize, nType, nSource, nBufSize, nToCopy, nCurPos = mpOutStrm->Tell();
    BYTE*   pBuf;

    // Persist table anpassen
    for ( void* pPtr = maPersistTable.First(); pPtr; pPtr = maPersistTable.Next() )
    {
        UINT32 nOfs = ((_EscherPersistEntry*)pPtr)->mnOffset;
        if ( nOfs >= nCurPos )
            ((_EscherPersistEntry*)pPtr)->mnOffset += nBytes;
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
    PtInsert( _Escher_Persist_PrivateEntry | nKey, nOffset );
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
    return ImplSeek( _Escher_Persist_PrivateEntry | nKey );
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
        case _Escher_DgContainer :
        {
            if ( !mb_EscherDg )
            {
                mb_EscherDg = TRUE;
                mnCurrentDg++;
                mnTotalShapesDg = 0;
                mnTotalShapeIdUsedDg = 0;
                mnCurrentShapeID = ( mnCurrentShapeMaximumID &~0x3ff ) + 0x400; // eine neue Seite bekommt immer eine neue ShapeId die ein vielfaches von 1024 ist,
                                                                                // damit ist erste aktuelle Shape ID 0x400
                AddAtom( 8, _Escher_Dg, 0, mnCurrentDg );
                PtReplaceOrInsert( _Escher_Persist_Dg | mnCurrentDg, mpOutStrm->Tell() );
                *mpOutStrm << (UINT32)0     // The number of shapes in this drawing
                           << (UINT32)0;    // The last MSOSPID given to an SP in this DG
            }
        }
        break;

        case _Escher_SpgrContainer :
        {
            if ( mb_EscherDg )
            {
                mb_EscherSpgr = TRUE;
            }
        }
        break;

        case _Escher_SpContainer :
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
        case _Escher_DgContainer :
        {
            if ( mb_EscherDg )
            {
                mb_EscherDg = FALSE;
                if ( ImplSeek( _Escher_Persist_Dg | mnCurrentDg ) )
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

        case _Escher_SpgrContainer :
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
    AddAtom( 8, _Escher_ClientAnchor );
    *mpOutStrm << (INT16)rRect.Top() << (INT16)rRect.Left() << (INT16)rRect.Right() << (INT16)rRect.Bottom();
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::EnterGroup( Rectangle* pBoundRect )
{
    Rectangle aRect;
    if ( pBoundRect )
        aRect = *pBoundRect;

    OpenContainer( _Escher_SpgrContainer );
    OpenContainer( _Escher_SpContainer );
    AddAtom( 16, _Escher_Spgr, 1 );
    PtReplaceOrInsert( _Escher_Persist_Grouping_Snap | mnGroupLevel, mpOutStrm->Tell() );
    *mpOutStrm  << (INT32)aRect.Left()  // Bounding box fuer die Gruppierten shapes an die sie attached werden
                << (INT32)aRect.Top()
                << (INT32)aRect.Right()
                << (INT32)aRect.Bottom();

    if ( !mnGroupLevel )
    {
        AddShape( _Escher_ShpInst_Min, 5 );                             // Flags: Group | Patriarch
        CloseContainer();                                               // _Escher_SpContainer
    }
    else
    {
        AddShape( _Escher_ShpInst_Min, 0x201 );                         // Flags: Group | HaveAnchor
        AddAtom( 8, _Escher_ClientAnchor );
        PtReplaceOrInsert( _Escher_Persist_Grouping_Logic | mnGroupLevel, mpOutStrm->Tell() );
        *mpOutStrm << (INT16)aRect.Top() << (INT16)aRect.Left() << (INT16)aRect.Right() << (INT16)aRect.Bottom();
        CloseContainer();                                               // _Escher_SpContainer
    }
    mnGroupLevel++;
}

// ---------------------------------------------------------------------------------------------

BOOL _EscherEx::SetGroupSnapRect( UINT32 nGroupLevel, const Rectangle& rRect )
{
    BOOL bRetValue = FALSE;
    if ( nGroupLevel )
    {
        UINT32 nCurrentPos = mpOutStrm->Tell();
        if ( ImplSeek( _Escher_Persist_Grouping_Snap | ( nGroupLevel - 1 ) ) )
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
        if ( ImplSeek( _Escher_Persist_Grouping_Logic | ( nGroupLevel - 1 ) ) )
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
    PtDelete( _Escher_Persist_Grouping_Snap | mnGroupLevel );
    PtDelete( _Escher_Persist_Grouping_Logic | mnGroupLevel );
    CloseContainer();
}

// ---------------------------------------------------------------------------------------------

void _EscherEx::AddShape( UINT32 nShpInstance, UINT32 nFlags, UINT32 nShapeID )
{
    AddAtom( 8, _Escher_Sp, 2, nShpInstance );

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
    AddOpt( _Escher_Prop_fillType, _Escher_FillShadeScale );
    AddOpt( _Escher_Prop_fillAngle, ( ( -3600 + pGradient->Angle ) << 16 ) / 10 );
    AddOpt( _Escher_Prop_fillColor, GetGradientColor( pGradient, nFirstColor ) );
    AddOpt( _Escher_Prop_fillBackColor, GetGradientColor( pGradient, nFirstColor ^ 1 ) );
    AddOpt( _Escher_Prop_fillFocus, nFillFocus );
};

// ---------------------------------------------------------------------------------------------

UINT32 _EscherEx::AddGraphic( SvStorageStream& rStrm, const ByteString& rUniqueId,
                                const Rectangle& rBoundRect, const GraphicAttr* pGraphicAttr )
{
    sal_uInt32 nBlibId = 0;
    if ( rUniqueId.Len() )
    {
        if ( !mpGraphicProvider )
            mpGraphicProvider = new _EscherGraphicProvider( rStrm );

        nBlibId = mpGraphicProvider->ImplGetBlibID( rUniqueId, rBoundRect, pGraphicAttr );
    }
    return nBlibId;
}
