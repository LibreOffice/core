/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#ifndef _PptEscherEx_HXX
#include "escherex.hxx"
#endif

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

PptEscherEx::PptEscherEx( SvStream& rOutStrm, const rtl::OUString& rBaseURI ) :
    EscherEx( EscherExGlobalRef( new EscherExGlobal ), rOutStrm )
{
    mxGlobal->SetBaseURI( rBaseURI );
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
                mbEscherDg = true;
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
                mbEscherSpgr = true;
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
                    mbEscherDg = false;
                    if ( DoSeek( ESCHER_Persist_Dg | mnCurrentDg ) )
                        *mpOutStrm << mxGlobal->GetDrawingShapeCount( mnCurrentDg ) << mxGlobal->GetLastShapeId( mnCurrentDg );
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
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PptEscherEx::EnterGroup(
    const basegfx::B2DRange* pObjectRange,
    SvMemoryStream* pClientData )
{
    sal_uInt32 nShapeId = 0;
    /* SJ: #Issue 26747#
       not creating group objects with a depth higher than 16, because then
       PPT is having a big performance problem when starting a slide show
    */
    if ( mnGroupLevel < 12 )
    {
        basegfx::B2DRange aObjectRange(0.0, 0.0, 1.0, 1.0);

        if(pObjectRange)
        {
            aObjectRange = *pObjectRange;
        }

        OpenContainer( ESCHER_SpgrContainer );
        OpenContainer( ESCHER_SpContainer );
        AddAtom( 16, ESCHER_Spgr, 1 );
        PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel, mpOutStrm->Tell() );
        *mpOutStrm  << (sal_Int32)basegfx::fround(aObjectRange.getMinX())   // Bounding box fuer die Gruppierten shapes an die sie attached werden
                    << (sal_Int32)basegfx::fround(aObjectRange.getMinY()) // here minx,y, maxx,y is correct
                    << (sal_Int32)basegfx::fround(aObjectRange.getMaxX())
                    << (sal_Int32)basegfx::fround(aObjectRange.getMaxY());

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
                *mpOutStrm << (sal_Int16)basegfx::fround(aObjectRange.getMinY()) // CAUTION! Here, X,Y is exchanged
                           << (sal_Int16)basegfx::fround(aObjectRange.getMinX()) // by PURPOSE, it's not an error (!)
                           << (sal_Int16)basegfx::fround(aObjectRange.getMaxX()) // TTTT: Check for mirrored if Min/Max needs to be exchanged
                           << (sal_Int16)basegfx::fround(aObjectRange.getMaxY());
            }
            else
            {
                AddAtom( 16, ESCHER_ChildAnchor );
                PtReplaceOrInsert( ESCHER_Persist_Grouping_Snap | mnGroupLevel, mpOutStrm->Tell() );
                *mpOutStrm << (sal_Int32)basegfx::fround(aObjectRange.getMinX()) // here minx,y, maxx,y is correct
                           << (sal_Int32)basegfx::fround(aObjectRange.getMinY())
                           << (sal_Int32)basegfx::fround(aObjectRange.getMaxX()) // TTTT: Check for mirrored if Min/Max needs to be exchanged
                           << (sal_Int32)basegfx::fround(aObjectRange.getMaxY());
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
