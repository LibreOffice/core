/*************************************************************************
 *
 *  $RCSfile: escherex.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:49:44 $
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


#ifndef _PptEscherEx_HXX
#include "escherex.hxx"
#endif

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

PptEscherEx::PptEscherEx( SvStream& rOutStrm, UINT32 nDrawings ) :
    EscherEx                ( rOutStrm, nDrawings )
{
    mnFIDCLs = nDrawings;
    mnCurrentDg = 0;
    mnCurrentShapeID = 0;
    mnTotalShapesDgg = 0;
    mnCurrentShapeMaximumID = 0;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PptEscherEx::DrawingGroupContainerSize()
{
    return ImplDggContainerSize() + 8;
}

void PptEscherEx::WriteDrawingGroupContainer( SvStream& rSt )
{
    UINT32 nSize = DrawingGroupContainerSize();
    rSt << (sal_uInt32)( 0xf | ( 1035 << 16 ) )     // EPP_PPDrawingGroup
        << (sal_uInt32)( nSize - 8 );

    ImplWriteDggContainer( rSt );
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PptEscherEx::ImplDggContainerSize()
{
    UINT32 nSize;

    nSize  = ImplDggAtomSize();
    nSize += GetBlibStoreContainerSize();
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

        ImplWriteDggAtom( rSt );
        WriteBlibStoreContainer( rSt );
        ImplWriteOptAtom( rSt );
        ImplWriteSplitMenuColorsAtom( rSt );
    }
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PptEscherEx::ImplDggAtomSize()
{
    return maFIDCLs.Tell() + 24;
}

void PptEscherEx::ImplWriteDggAtom( SvStream& rSt )
{
    sal_uInt32 nSize = ImplDggAtomSize();
    if ( nSize )
    {
        rSt << (sal_uInt32)( ESCHER_Dgg << 16 )
            << (sal_uInt32)( nSize - 8 )
            << mnCurrentShapeID
            << (sal_uInt32)( mnFIDCLs + 1 )
            << mnTotalShapesDgg
            << mnDrawings;

        rSt.Write( maFIDCLs.GetData(), nSize - 24 );
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
    UINT32 nSize = ImplSplitMenuColorsAtomSize();
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

void PptEscherEx::OpenContainer( UINT16 n_EscherContainer, int nRecInstance )
{
    *mpOutStrm << (UINT16)( ( nRecInstance << 4 ) | 0xf  ) << n_EscherContainer << (UINT32)0;
    mOffsets.push_back( mpOutStrm->Tell() - 4 );
    mRecTypes.push_back( n_EscherContainer );

    switch( n_EscherContainer )
    {
        case ESCHER_DgContainer :
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

void PptEscherEx::CloseContainer()
{
    /* SJ: #Issue 26747#
       not creating group objects with a depth higher than 16, because then
       PPT is having a big performance problem when starting a slide show
    */
    if ( ( mRecTypes.back() != ESCHER_SpgrContainer ) || ( mnGroupLevel < 12 ) )
    {
        UINT32 nSize, nPos = mpOutStrm->Tell();
        nSize = ( nPos - mOffsets.back() ) - 4;
        mpOutStrm->Seek( mOffsets.back() );
        *mpOutStrm << nSize;

        switch( mRecTypes.back() )
        {
            case ESCHER_DgContainer :
            {
                if ( mbEscherDg )
                {
                    mbEscherDg = FALSE;
                    if ( DoSeek( ESCHER_Persist_Dg | mnCurrentDg ) )
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
                if ( mbEscherSpgr )
                {
                    mbEscherSpgr = FALSE;

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
        *mpOutStrm  << (INT32)aRect.Left()  // Bounding box fuer die Gruppierten shapes an die sie attached werden
                    << (INT32)aRect.Top()
                    << (INT32)aRect.Right()
                    << (INT32)aRect.Bottom();

        nShapeId = GetShapeID();
        if ( !mnGroupLevel )
            AddShape( ESCHER_ShpInst_Min, 5, nShapeId );                    // Flags: Group | Patriarch
        else
        {
            AddShape( ESCHER_ShpInst_Min, 0x201, nShapeId );                // Flags: Group | HaveAnchor
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
    }
    mnGroupLevel++;
    return nShapeId;
}

// ---------------------------------------------------------------------------------------------
