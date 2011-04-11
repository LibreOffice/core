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
#include "precompiled_sd.hxx"


#include "escherex.hxx"

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

PptEscherEx::PptEscherEx( SvStream& rOutStrm ) :
    EscherEx( EscherExGlobalRef( new EscherExGlobal ), rOutStrm )
{
    mnCurrentDg = 0;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PptEscherEx::DrawingGroupContainerSize()
{
    return ImplDggContainerSize() + 8;
}

void PptEscherEx::WriteDrawingGroupContainer( SvStream& rSt )
{
    sal_uInt32 nSize = DrawingGroupContainerSize();
    rSt << (sal_uInt32)( 0xf | ( 1035 << 16 ) )     // EPP_PPDrawingGroup
        << (sal_uInt32)( nSize - 8 );

    ImplWriteDggContainer( rSt );
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PptEscherEx::ImplDggContainerSize()
{
    sal_uInt32 nSize;

    nSize  = mxGlobal->GetDggAtomSize();
    nSize += mxGlobal->GetBlibStoreContainerSize();
    nSize += ImplOptAtomSize();
    nSize += ImplSplitMenuColorsAtomSize();

    return nSize + 8;
}

void PptEscherEx::ImplWriteDggContainer( SvStream& rSt )
{
    sal_uInt32 nSize = ImplDggContainerSize();
    if ( nSize )
    {
        rSt << (sal_uInt32)( 0xf | ( ESCHER_DggContainer << 16 ) )
            << (sal_uInt32)( nSize - 8 );

        mxGlobal->SetDggContainer();
        mxGlobal->WriteDggAtom( rSt );
        mxGlobal->WriteBlibStoreContainer( rSt );
        ImplWriteOptAtom( rSt );
        ImplWriteSplitMenuColorsAtom( rSt );
    }
}

// ---------------------------------------------------------------------------------------------

#define ESCHER_OPT_COUNT 6

sal_uInt32 PptEscherEx::ImplOptAtomSize()
{
    sal_uInt32 nSize = 0;
    if ( ESCHER_OPT_COUNT )
        nSize = ( ESCHER_OPT_COUNT * 6 ) + 8;
    return nSize;
}

void PptEscherEx::ImplWriteOptAtom( SvStream& rSt )
{
    sal_uInt32 nSize = ImplOptAtomSize();
    if ( nSize )
    {
        rSt << (sal_uInt32)( ( ESCHER_OPT << 16 ) | ( ESCHER_OPT_COUNT << 4 ) | 0x3 )
            << (sal_uInt32)( nSize - 8 )
            << (sal_uInt16)ESCHER_Prop_fillColor            << (sal_uInt32)0xffb800
            << (sal_uInt16)ESCHER_Prop_fillBackColor        << (sal_uInt32)0
            << (sal_uInt16)ESCHER_Prop_fNoFillHitTest       << (sal_uInt32)0x00100010
            << (sal_uInt16)ESCHER_Prop_lineColor            << (sal_uInt32)0x8000001
            << (sal_uInt16)ESCHER_Prop_fNoLineDrawDash      << (sal_uInt32)0x00080008
            << (sal_uInt16)ESCHER_Prop_shadowColor          << (sal_uInt32)0x8000002;
    }
}

// ---------------------------------------------------------------------------------------------

#define ESCHER_SPLIT_MENU_COLORS_COUNT  4

sal_uInt32 PptEscherEx::ImplSplitMenuColorsAtomSize()
{
    sal_uInt32 nSize = 0;
    if ( ESCHER_SPLIT_MENU_COLORS_COUNT )
        nSize = ( ESCHER_SPLIT_MENU_COLORS_COUNT << 2 ) + 8;
    return nSize;
}

void PptEscherEx::ImplWriteSplitMenuColorsAtom( SvStream& rSt )
{
    sal_uInt32 nSize = ImplSplitMenuColorsAtomSize();
    if ( nSize )
    {
        rSt << (sal_uInt32)( ( ESCHER_SplitMenuColors << 16 ) | ( ESCHER_SPLIT_MENU_COLORS_COUNT << 4 ) )
            << (sal_uInt32)( nSize - 8 )
            << (sal_uInt32)0x08000004
            << (sal_uInt32)0x08000001
            << (sal_uInt32)0x08000002
            << (sal_uInt32)0x100000f7;
    }

}

// ---------------------------------------------------------------------------------------------

PptEscherEx::~PptEscherEx()
{
}

// ---------------------------------------------------------------------------------------------

void PptEscherEx::OpenContainer( sal_uInt16 n_EscherContainer, int nRecInstance )
{
    *mpOutStrm << (sal_uInt16)( ( nRecInstance << 4 ) | 0xf  ) << n_EscherContainer << (sal_uInt32)0;
    mOffsets.push_back( mpOutStrm->Tell() - 4 );
    mRecTypes.push_back( n_EscherContainer );

    switch( n_EscherContainer )
    {
        case ESCHER_DgContainer :
        {
            if ( !mbEscherDg )
            {
                mbEscherDg = sal_True;
                mnCurrentDg = mxGlobal->GenerateDrawingId();
                AddAtom( 8, ESCHER_Dg, 0, mnCurrentDg );
                PtReplaceOrInsert( ESCHER_Persist_Dg | mnCurrentDg, mpOutStrm->Tell() );
                *mpOutStrm << (sal_uInt32)0     // The number of shapes in this drawing
                           << (sal_uInt32)0;    // The last MSOSPID given to an SP in this DG
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherSpgr = sal_True;
            }
        }
        break;

        default:
        break;
    }
}

// ---------------------------------------------------------------------------------------------

void PptEscherEx::CloseContainer()
{
    /* SJ: #Issue 26747#
       not creating group objects with a depth higher than 16, because then
       PPT is having a big performance problem when starting a slide show
    */
    if ( ( mRecTypes.back() != ESCHER_SpgrContainer ) || ( mnGroupLevel < 12 ) )
    {
        sal_uInt32 nSize, nPos = mpOutStrm->Tell();
        nSize = ( nPos - mOffsets.back() ) - 4;
        mpOutStrm->Seek( mOffsets.back() );
        *mpOutStrm << nSize;

        switch( mRecTypes.back() )
        {
            case ESCHER_DgContainer :
            {
                if ( mbEscherDg )
                {
                    mbEscherDg = sal_False;
                    if ( DoSeek( ESCHER_Persist_Dg | mnCurrentDg ) )
                        *mpOutStrm << mxGlobal->GetDrawingShapeCount( mnCurrentDg ) << mxGlobal->GetLastShapeId( mnCurrentDg );
                }
            }
            break;

            case ESCHER_SpgrContainer :
            {
                if ( mbEscherSpgr )
                {
                    mbEscherSpgr = sal_False;

                }
            }
            break;

            default:
            break;
        }
        mOffsets.pop_back();
        mRecTypes.pop_back();
        mpOutStrm->Seek( nPos );
    }
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PptEscherEx::EnterGroup( Rectangle* pBoundRect, SvMemoryStream* pClientData )
{
    sal_uInt32 nShapeId = 0;
    /* SJ: #Issue 26747#
       not creating group objects with a depth higher than 16, because then
       PPT is having a big performance problem when starting a slide show
    */
    if ( mnGroupLevel < 12 )
    {
        Rectangle aRect;
        if ( pBoundRect )
            aRect = *pBoundRect;

        OpenContainer( ESCHER_SpgrContainer );
        OpenContainer( ESCHER_SpContainer );
        AddAtom( 16, ESCHER_Spgr, 1 );
        PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel, mpOutStrm->Tell() );
        *mpOutStrm  << (sal_Int32)aRect.Left()  // Bounding box fuer die Gruppierten shapes an die sie attached werden
                    << (sal_Int32)aRect.Top()
                    << (sal_Int32)aRect.Right()
                    << (sal_Int32)aRect.Bottom();

        nShapeId = GenerateShapeId();
        if ( !mnGroupLevel )
            AddShape( ESCHER_ShpInst_Min, 5, nShapeId );                    // Flags: Group | Patriarch
        else
        {
            AddShape( ESCHER_ShpInst_Min, 0x201, nShapeId );                // Flags: Group | HaveAnchor
            if ( mnGroupLevel == 1 )
            {
                AddAtom( 8, ESCHER_ClientAnchor );
                PtReplaceOrInsert( ESCHER_Persist_Grouping_Logic | mnGroupLevel, mpOutStrm->Tell() );
                *mpOutStrm << (sal_Int16)aRect.Top() << (sal_Int16)aRect.Left() << (sal_Int16)aRect.Right() << (sal_Int16)aRect.Bottom();
            }
            else
            {
                AddAtom( 16, ESCHER_ChildAnchor );
                PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel, mpOutStrm->Tell() );
                *mpOutStrm  << (sal_Int32)aRect.Left()
                            << (sal_Int32)aRect.Top()
                            << (sal_Int32)aRect.Right()
                            << (sal_Int32)aRect.Bottom();
            }
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
    }
    mnGroupLevel++;
    return nShapeId;
}

// ---------------------------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
