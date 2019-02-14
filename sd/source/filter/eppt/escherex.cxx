/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "escherex.hxx"

PptEscherEx::PptEscherEx( SvStream& rOutStrm, const OUString& rBaseURI ) :
    EscherEx( std::make_shared<EscherExGlobal>( ), &rOutStrm )
{
    mxGlobal->SetBaseURI( rBaseURI );
    mnCurrentDg = 0;
}

sal_uInt32 PptEscherEx::DrawingGroupContainerSize()
{
    return ImplDggContainerSize() + 8;
}

void PptEscherEx::WriteDrawingGroupContainer( SvStream& rSt )
{
    sal_uInt32 nSize = DrawingGroupContainerSize();
    rSt.WriteUInt32( 0xf | ( 1035 << 16 ) )     // EPP_PPDrawingGroup
       .WriteUInt32( nSize - 8 );

    ImplWriteDggContainer( rSt );
}

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
        rSt.WriteUInt32( 0xf | ( ESCHER_DggContainer << 16 ) )
           .WriteUInt32( nSize - 8 );

        mxGlobal->SetDggContainer();
        mxGlobal->WriteDggAtom( rSt );
        mxGlobal->WriteBlibStoreContainer( rSt );
        ImplWriteOptAtom( rSt );
        ImplWriteSplitMenuColorsAtom( rSt );
    }
}

#define ESCHER_OPT_COUNT 6

sal_uInt32 PptEscherEx::ImplOptAtomSize()
{
    sal_uInt32 nSize = 0;
    if ( ESCHER_OPT_COUNT != 0 )
        nSize = ( ESCHER_OPT_COUNT * 6 ) + 8;
    return nSize;
}

void PptEscherEx::ImplWriteOptAtom( SvStream& rSt )
{
    sal_uInt32 nSize = ImplOptAtomSize();
    if ( nSize )
    {
        rSt.WriteUInt32( ( ESCHER_OPT << 16 ) | ( ESCHER_OPT_COUNT << 4 ) | 0x3 )
           .WriteUInt32( nSize - 8 )
           .WriteUInt16( ESCHER_Prop_fillColor )           .WriteUInt32( 0xffb800 )
           .WriteUInt16( ESCHER_Prop_fillBackColor )       .WriteUInt32( 0 )
           .WriteUInt16( ESCHER_Prop_fNoFillHitTest )      .WriteUInt32( 0x00100010 )
           .WriteUInt16( ESCHER_Prop_lineColor )           .WriteUInt32( 0x8000001 )
           .WriteUInt16( ESCHER_Prop_fNoLineDrawDash )     .WriteUInt32( 0x00080008 )
           .WriteUInt16( ESCHER_Prop_shadowColor )         .WriteUInt32( 0x8000002 );
    }
}

#define ESCHER_SPLIT_MENU_COLORS_COUNT  4

sal_uInt32 PptEscherEx::ImplSplitMenuColorsAtomSize()
{
    sal_uInt32 nSize = 0;
    if ( ESCHER_SPLIT_MENU_COLORS_COUNT != 0 )
        nSize = ( ESCHER_SPLIT_MENU_COLORS_COUNT << 2 ) + 8;
    return nSize;
}

void PptEscherEx::ImplWriteSplitMenuColorsAtom( SvStream& rSt )
{
    sal_uInt32 nSize = ImplSplitMenuColorsAtomSize();
    if ( nSize )
    {
        rSt.WriteUInt32( ( ESCHER_SplitMenuColors << 16 ) | ( ESCHER_SPLIT_MENU_COLORS_COUNT << 4 ) )
           .WriteUInt32( nSize - 8 )
           .WriteUInt32( 0x08000004 )
           .WriteUInt32( 0x08000001 )
           .WriteUInt32( 0x08000002 )
           .WriteUInt32( 0x100000f7 );
    }

}

PptEscherEx::~PptEscherEx()
{
}

void PptEscherEx::OpenContainer( sal_uInt16 n_EscherContainer, int nRecInstance )
{
    mpOutStrm->WriteUInt16( ( nRecInstance << 4 ) | 0xf ).WriteUInt16( n_EscherContainer ).WriteUInt32( 0 );
    mOffsets.push_back( mpOutStrm->Tell() - 4 );
    mRecTypes.push_back( n_EscherContainer );

    switch( n_EscherContainer )
    {
        case ESCHER_DgContainer :
        {
            if ( !mbEscherDg )
            {
                mbEscherDg = true;
                mnCurrentDg = mxGlobal->GenerateDrawingId();
                AddAtom( 8, ESCHER_Dg, 0, mnCurrentDg );
                PtReplaceOrInsert( ESCHER_Persist_Dg | mnCurrentDg, mpOutStrm->Tell() );
                mpOutStrm->WriteUInt32( 0 )     // The number of shapes in this drawing
                          .WriteUInt32( 0 );    // The last MSOSPID given to an SP in this DG
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherSpgr = true;
            }
        }
        break;

        default:
        break;
    }
}

void PptEscherEx::CloseContainer()
{
    /* SJ: #Issue 26747#
       not creating group objects with a depth higher than 16, because then
       PPT is having a big performance problem when starting a slide show
    */
    if ( ( mRecTypes.back() == ESCHER_SpgrContainer ) && ( mnGroupLevel >= 12 ) )
        return;

    sal_uInt32 nSize, nPos = mpOutStrm->Tell();
    nSize = ( nPos - mOffsets.back() ) - 4;
    mpOutStrm->Seek( mOffsets.back() );
    mpOutStrm->WriteUInt32( nSize );

    switch( mRecTypes.back() )
    {
        case ESCHER_DgContainer :
        {
            if ( mbEscherDg )
            {
                mbEscherDg = false;
                if ( DoSeek( ESCHER_Persist_Dg | mnCurrentDg ) )
                    mpOutStrm->WriteUInt32( mxGlobal->GetDrawingShapeCount( mnCurrentDg ) ).WriteUInt32( mxGlobal->GetLastShapeId( mnCurrentDg ) );
            }
        }
        break;

        case ESCHER_SpgrContainer :
        {
            if ( mbEscherSpgr )
            {
                mbEscherSpgr = false;
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

sal_uInt32 PptEscherEx::EnterGroup( ::tools::Rectangle const * pBoundRect, SvMemoryStream* pClientData )
{
    sal_uInt32 nShapeId = 0;
    /* SJ: #Issue 26747#
       not creating group objects with a depth higher than 16, because then
       PPT is having a big performance problem when starting a slide show
    */
    if ( mnGroupLevel < 12 )
    {
        ::tools::Rectangle aRect;
        if ( pBoundRect )
            aRect = *pBoundRect;

        OpenContainer( ESCHER_SpgrContainer );
        OpenContainer( ESCHER_SpContainer );
        AddAtom( 16, ESCHER_Spgr, 1 );
        PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel, mpOutStrm->Tell() );
        mpOutStrm ->WriteInt32( aRect.Left() )  // bounding box for the grouped shapes to which they are attached
                   .WriteInt32( aRect.Top() )
                   .WriteInt32( aRect.Right() )
                   .WriteInt32( aRect.Bottom() );

        nShapeId = GenerateShapeId();
        if ( !mnGroupLevel )
            AddShape( ESCHER_ShpInst_Min, ShapeFlag::Group | ShapeFlag::Patriarch, nShapeId );
        else
        {
            AddShape( ESCHER_ShpInst_Min, ShapeFlag::HaveAnchor | ShapeFlag::Group, nShapeId );
            if ( mnGroupLevel == 1 )
            {
                AddAtom( 8, ESCHER_ClientAnchor );
                PtReplaceOrInsert( ESCHER_Persist_Grouping_Logic | mnGroupLevel, mpOutStrm->Tell() );
                mpOutStrm->WriteInt16( aRect.Top() ).WriteInt16( aRect.Left() ).WriteInt16( aRect.Right() ).WriteInt16( aRect.Bottom() );
            }
            else
            {
                AddAtom( 16, ESCHER_ChildAnchor );
                PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel, mpOutStrm->Tell() );
                mpOutStrm ->WriteInt32( aRect.Left() )
                           .WriteInt32( aRect.Top() )
                           .WriteInt32( aRect.Right() )
                           .WriteInt32( aRect.Bottom() );
            }
        }
        if ( pClientData )
        {
            sal_uInt32 nSize = pClientData->TellEnd();
            if ( nSize )
            {
                mpOutStrm->WriteUInt32( ( ESCHER_ClientData << 16 ) | 0xf )
                       .WriteUInt32( nSize );
                mpOutStrm->WriteBytes(pClientData->GetData(), nSize);
            }
        }
        CloseContainer();                                               // ESCHER_SpContainer
    }
    mnGroupLevel++;
    return nShapeId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
