/*************************************************************************
 *
 *  $RCSfile: gdimtf.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ka $ $Date: 2002-08-01 09:58:51 $
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

#define _SV_GDIMTF_CXX

#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_CVTSVM_HXX
#include <cvtsvm.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#include <gdimtf.hxx>

// -----------
// - Defines -
// -----------

#define GAMMA( _def_cVal, _def_InvGamma )   ((BYTE)MinMax(FRound(pow( _def_cVal/255.0,_def_InvGamma)*255.0),0L,255L))

// --------------------------
// - Color exchange structs -
// --------------------------

struct ImplColAdjustParam
{
    BYTE*   pMapR;
    BYTE*   pMapG;
    BYTE*   pMapB;
};

struct ImplBmpAdjustParam
{
    short   nLuminancePercent;
    short   nContrastPercent;
    short   nChannelRPercent;
    short   nChannelGPercent;
    short   nChannelBPercent;
    double  fGamma;
    BOOL    bInvert;
};

// -----------------------------------------------------------------------------

struct ImplColConvertParam
{
    MtfConversion   eConversion;
};

struct ImplBmpConvertParam
{
    BmpConversion   eConversion;
};

// -----------------------------------------------------------------------------

struct ImplColMonoParam
{
    Color aColor;
};

struct ImplBmpMonoParam
{
    Color aColor;
};

// -----------------------------------------------------------------------------

struct ImplColReplaceParam
{
    ULONG*          pMinR;
    ULONG*          pMaxR;
    ULONG*          pMinG;
    ULONG*          pMaxG;
    ULONG*          pMinB;
    ULONG*          pMaxB;
    const Color*    pDstCols;
    ULONG           nCount;
};

struct ImplBmpReplaceParam
{
    const Color*    pSrcCols;
    const Color*    pDstCols;
    ULONG           nCount;
    const ULONG*    pTols;
};


// ---------
// - Label -
// ---------

struct ImpLabel
{
    String  aLabelName;
    ULONG   nActionPos;

            ImpLabel( const String& rLabelName, ULONG _nActionPos ) :
                aLabelName( rLabelName ),
                nActionPos( _nActionPos ) {}
};

// -------------
// - LabelList -
// -------------

class ImpLabelList : private List
{
public:

                ImpLabelList() : List( 8, 4, 4 ) {}
                ImpLabelList( const ImpLabelList& rList );
                ~ImpLabelList();

    void        ImplInsert( ImpLabel* p ) { Insert( p, LIST_APPEND ); }
    ImpLabel*   ImplRemove( ULONG nPos ) { return (ImpLabel*) Remove( nPos ); }
    void        ImplReplace( ImpLabel* p ) { Replace( (void*)p ); }
    ImpLabel*   ImplFirst() { return (ImpLabel*) First(); }
    ImpLabel*   ImplNext() { return (ImpLabel*) Next(); }
    ImpLabel*   ImplGetLabel( ULONG nPos ) const { return (ImpLabel*) GetObject( nPos ); }
    ULONG       ImplGetLabelPos( const String& rLabelName );
    ULONG       ImplCount() const { return Count(); }
};

// ------------------------------------------------------------------------

ImpLabelList::ImpLabelList( const ImpLabelList& rList ) :
        List( rList )
{
    for( ImpLabel* pLabel = ImplFirst(); pLabel; pLabel = ImplNext() )
        ImplReplace( new ImpLabel( *pLabel ) );
}

// ------------------------------------------------------------------------

ImpLabelList::~ImpLabelList()
{
    for( ImpLabel* pLabel = ImplFirst(); pLabel; pLabel = ImplNext() )
        delete pLabel;
}

// ------------------------------------------------------------------------

ULONG ImpLabelList::ImplGetLabelPos( const String& rLabelName )
{
    ULONG nLabelPos = METAFILE_LABEL_NOTFOUND;

    for( ImpLabel* pLabel = ImplFirst(); pLabel; pLabel = ImplNext() )
    {
        if ( rLabelName == pLabel->aLabelName )
        {
            nLabelPos = GetCurPos();
            break;
        }
    }

    return nLabelPos;
}

// ---------------
// - GDIMetaFile -
// ---------------

GDIMetaFile::GDIMetaFile() :
    List        ( 0x3EFF, 64, 64 ),
    aPrefSize   ( 1, 1 ),
    pPrev       ( NULL ),
    pNext       ( NULL ),
    pOutDev     ( NULL ),
    pLabelList  ( NULL ),
    bPause      ( FALSE ),
    bRecord     ( FALSE )
{
}

// ------------------------------------------------------------------------

GDIMetaFile::GDIMetaFile( const GDIMetaFile& rMtf ) :
    List            ( rMtf ),
    aPrefMapMode    ( rMtf.aPrefMapMode ),
    aPrefSize       ( rMtf.aPrefSize ),
    aHookHdlLink    ( rMtf.aHookHdlLink ),
    pPrev           ( rMtf.pPrev ),
    pNext           ( rMtf.pNext ),
    pOutDev         ( NULL ),
    bPause          ( FALSE ),
    bRecord         ( FALSE )
{
    // RefCount der MetaActions erhoehen
    for( void* pAct = First(); pAct; pAct = Next() )
        ( (MetaAction*) pAct )->Duplicate();

    if( rMtf.pLabelList )
        pLabelList = new ImpLabelList( *rMtf.pLabelList );
    else
        pLabelList = NULL;

    if( rMtf.bRecord )
    {
        Record( rMtf.pOutDev );

        if ( rMtf.bPause )
            Pause( TRUE );
    }
}

// ------------------------------------------------------------------------

GDIMetaFile::~GDIMetaFile()
{
    Clear();
}

// ------------------------------------------------------------------------

GDIMetaFile& GDIMetaFile::operator=( const GDIMetaFile& rMtf )
{
    if( this != &rMtf )
    {
        Clear();

        List::operator=( rMtf );

        // RefCount der MetaActions erhoehen
        for( void* pAct = First(); pAct; pAct = Next() )
            ( (MetaAction*) pAct )->Duplicate();

        if( rMtf.pLabelList )
            pLabelList = new ImpLabelList( *rMtf.pLabelList );
        else
           pLabelList = NULL;

        aPrefMapMode = rMtf.aPrefMapMode;
        aPrefSize = rMtf.aPrefSize;
        aHookHdlLink = rMtf.aHookHdlLink;
        pPrev = rMtf.pPrev;
        pNext = rMtf.pNext;
        pOutDev = NULL;
        bPause = FALSE;
        bRecord = FALSE;

        if( rMtf.bRecord )
        {
            Record( rMtf.pOutDev );

            if( rMtf.bPause )
                Pause( TRUE );
        }
    }

    return *this;
}

// ------------------------------------------------------------------------

BOOL GDIMetaFile::operator==( const GDIMetaFile& rMtf ) const
{
    const ULONG nCount = Count();
    BOOL        bRet = FALSE;

    if( this == &rMtf )
        bRet = TRUE;
    else if( rMtf.GetActionCount() == nCount &&
             rMtf.GetPrefSize() == aPrefSize &&
             rMtf.GetPrefMapMode() == aPrefMapMode )
    {
        bRet = TRUE;

        for( ULONG n = 0UL; n < nCount; n++ )
        {
            if( GetObject( n ) != rMtf.GetObject( n ) )
            {
                bRet = FALSE;
                break;
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::Clear()
{
    if( bRecord )
        Stop();

    for( void* pAct = First(); pAct; pAct = Next() )
        ( (MetaAction*) pAct )->Delete();

    List::Clear();

    delete pLabelList;
    pLabelList = NULL;
}

// ------------------------------------------------------------------------

void GDIMetaFile::Linker( OutputDevice* pOut, BOOL bLink )
{
    if( bLink )
    {
        pNext = NULL;
        pPrev = pOut->GetConnectMetaFile();
        pOut->SetConnectMetaFile( this );

        if( pPrev )
            pPrev->pNext = this;
    }
    else
    {
        if( pNext )
        {
            pNext->pPrev = pPrev;

            if( pPrev )
                pPrev->pNext = pNext;
        }
        else
        {
            if( pPrev )
                pPrev->pNext = NULL;

            pOut->SetConnectMetaFile( pPrev );
        }

        pPrev = NULL;
        pNext = NULL;
    }
}

// ------------------------------------------------------------------------

long GDIMetaFile::Hook()
{
    return aHookHdlLink.Call( this );
}

// ------------------------------------------------------------------------

void GDIMetaFile::Record( OutputDevice* pOut )
{
    if( bRecord )
        Stop();

    Last();
    pOutDev = pOut;
    bRecord = TRUE;
    Linker( pOut, TRUE );
}

// ------------------------------------------------------------------------

void GDIMetaFile::Play( GDIMetaFile& rMtf, ULONG nPos )
{
    if ( !bRecord && !rMtf.bRecord )
    {
        MetaAction* pAction = GetCurAction();
        const ULONG nCount = Count();

        if( nPos > nCount )
            nPos = nCount;

        for( ULONG nCurPos = GetCurPos(); nCurPos < nPos; nCurPos++ )
        {
            if( !Hook() )
            {
                pAction->Duplicate();
                rMtf.AddAction( pAction );
            }

            pAction = (MetaAction*) Next();
        }
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Play( OutputDevice* pOut, ULONG nPos )
{
    if( !bRecord )
    {
        MetaAction* pAction = GetCurAction();
        const ULONG nCount = Count();
        ULONG       i  = 0, nSyncCount = ( pOut->GetOutDevType() == OUTDEV_WINDOW ) ? 0x000000ff : 0xffffffff;

        if( nPos > nCount )
            nPos = nCount;

        for( ULONG nCurPos = GetCurPos(); nCurPos < nPos; nCurPos++ )
        {
            if( !Hook() )
            {
                pAction->Execute( pOut );

                // flush output from time to time
                if( i++ > nSyncCount )
                    ( (Window*) pOut )->Flush(), i = 0;
            }

            pAction = (MetaAction*) Next();
        }
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Play( OutputDevice* pOut, const Point& rPos,
                        const Size& rSize, ULONG nPos )
{
    Region  aDrawClipRegion;
    MapMode aDrawMap( GetPrefMapMode() );
    Size    aDestSize( pOut->LogicToPixel( rSize ) );

    if( aDestSize.Width() && aDestSize.Height() )
    {
        Size            aTmpPrefSize( pOut->LogicToPixel( GetPrefSize(), aDrawMap ) );
        GDIMetaFile*    pMtf = pOut->GetConnectMetaFile();

        if( !aTmpPrefSize.Width() )
            aTmpPrefSize.Width() = aDestSize.Width();

        if( !aTmpPrefSize.Height() )
            aTmpPrefSize.Height() = aDestSize.Height();

        Fraction aScaleX( aDestSize.Width(), aTmpPrefSize.Width() );
        Fraction aScaleY( aDestSize.Height(), aTmpPrefSize.Height() );

        aScaleX *= aDrawMap.GetScaleX(); aDrawMap.SetScaleX( aScaleX );
        aScaleY *= aDrawMap.GetScaleY(); aDrawMap.SetScaleY( aScaleY );

        aDrawMap.SetOrigin( pOut->PixelToLogic( pOut->LogicToPixel( rPos ), aDrawMap ) );

        pOut->Push();

        if ( pMtf && pMtf->IsRecord() && ( pOut->GetOutDevType() != OUTDEV_PRINTER ) )
            pOut->SetRelativeMapMode( aDrawMap );
        else
            pOut->SetMapMode( aDrawMap );

        Play( pOut, nPos );

        pOut->Pop();
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Pause( BOOL _bPause )
{
    if( bRecord )
    {
        if( _bPause )
        {
            if( !bPause )
                Linker( pOutDev, FALSE );
        }
        else
        {
            if( bPause )
                Linker( pOutDev, TRUE );
        }

        bPause = _bPause;
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Stop()
{
    if( bRecord )
    {
        bRecord = FALSE;

        if( !bPause )
            Linker( pOutDev, FALSE );
        else
            bPause = FALSE;
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::WindStart()
{
    if( !bRecord )
        First();
}

// ------------------------------------------------------------------------

void GDIMetaFile::WindEnd()
{
    if( !bRecord )
        Last();
}

// ------------------------------------------------------------------------

void GDIMetaFile::Wind( ULONG nActionPos )
{
    if( !bRecord )
        Seek( nActionPos );
}

// ------------------------------------------------------------------------

void GDIMetaFile::WindPrev()
{
    if( !bRecord )
        Prev();
}

// ------------------------------------------------------------------------

void GDIMetaFile::WindNext()
{
    if( !bRecord )
        Next();
}

// ------------------------------------------------------------------------

void GDIMetaFile::AddAction( MetaAction* pAction )
{
    Insert( pAction, LIST_APPEND );

    if( pPrev )
    {
        pAction->Duplicate();
        pPrev->AddAction( pAction );
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::AddAction( MetaAction* pAction, ULONG nPos )
{
    Insert( pAction, nPos );

    if( pPrev )
    {
        pAction->Duplicate();
        pPrev->AddAction( pAction, nPos );
    }
}

// ------------------------------------------------------------------------

MetaAction* GDIMetaFile::CopyAction( ULONG nPos ) const
{
    return ( (MetaAction*) GetObject( nPos ) )->Clone();
}

// ------------------------------------------------------------------------

ULONG GDIMetaFile::GetActionPos( const String& rLabel )
{
    ImpLabel* pLabel = NULL;

    if( pLabelList )
        pLabel = pLabelList->ImplGetLabel( pLabelList->ImplGetLabelPos( rLabel ) );
    else
        pLabel = NULL;

    return( pLabel ? pLabel->nActionPos : METAFILE_LABEL_NOTFOUND );
}

// ------------------------------------------------------------------------

BOOL GDIMetaFile::InsertLabel( const String& rLabel, ULONG nActionPos )
{
    BOOL bRet = FALSE;

    if( !pLabelList )
        pLabelList = new ImpLabelList;

    if( pLabelList->ImplGetLabelPos( rLabel ) == METAFILE_LABEL_NOTFOUND )
    {
        pLabelList->ImplInsert( new ImpLabel( rLabel, nActionPos ) );
        bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::RemoveLabel( const String& rLabel )
{
    if( pLabelList )
    {
        const ULONG nLabelPos = pLabelList->ImplGetLabelPos( rLabel );

        if( nLabelPos != METAFILE_LABEL_NOTFOUND )
            delete pLabelList->ImplRemove( nLabelPos );
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::RenameLabel( const String& rLabel, const String& rNewLabel )
{
    if( pLabelList )
    {
        const ULONG nLabelPos = pLabelList->ImplGetLabelPos( rLabel );

        if ( nLabelPos != METAFILE_LABEL_NOTFOUND )
            pLabelList->ImplGetLabel( nLabelPos )->aLabelName = rNewLabel;
    }
}

// ------------------------------------------------------------------------

ULONG GDIMetaFile::GetLabelCount() const
{
    return( pLabelList ? pLabelList->ImplCount() : 0UL );
}

// ------------------------------------------------------------------------

String GDIMetaFile::GetLabel( ULONG nLabel )
{
    String aString;

    if( pLabelList )
    {
        const ImpLabel* pLabel = pLabelList->ImplGetLabel( nLabel );

        if( pLabel )
            aString = pLabel->aLabelName;
    }

    return aString;
}

// ------------------------------------------------------------------------

BOOL GDIMetaFile::SaveStatus()
{
    if ( bRecord )
    {
        if ( bPause )
            Linker( pOutDev, TRUE );

        AddAction( new MetaLineColorAction( pOutDev->GetLineColor(),
                                            pOutDev->IsLineColor() ) );
        AddAction( new MetaFillColorAction( pOutDev->GetFillColor(),
                                            pOutDev->IsFillColor() ) );
        AddAction( new MetaFontAction( pOutDev->GetFont() ) );
        AddAction( new MetaTextColorAction( pOutDev->GetTextColor() ) );
        AddAction( new MetaTextFillColorAction( pOutDev->GetTextFillColor(),
                                                pOutDev->IsTextFillColor() ) );
        AddAction( new MetaTextLineColorAction( pOutDev->GetTextLineColor(),
                                                pOutDev->IsTextLineColor() ) );
        AddAction( new MetaTextAlignAction( pOutDev->GetTextAlign() ) );
        AddAction( new MetaRasterOpAction( pOutDev->GetRasterOp() ) );
        AddAction( new MetaMapModeAction( pOutDev->GetMapMode() ) );
        AddAction( new MetaClipRegionAction( pOutDev->GetClipRegion(),
                                             pOutDev->IsClipRegion() ) );

        if ( bPause )
            Linker( pOutDev, FALSE );

        return TRUE;
    }
    else
        return FALSE;
}

// ------------------------------------------------------------------------

BOOL GDIMetaFile::Mirror( ULONG nMirrorFlags )
{
    const Size  aOldPrefSize( GetPrefSize() );
    long        nMoveX, nMoveY;
    double      fScaleX, fScaleY;
    BOOL        bRet;

    if( nMirrorFlags & MTF_MIRROR_HORZ )
        nMoveX = VOS_ABS( aOldPrefSize.Width() ) - 1, fScaleX = -1.0;
    else
        nMoveX = 0, fScaleX = 1.0;

    if( nMirrorFlags & MTF_MIRROR_VERT )
        nMoveY = VOS_ABS( aOldPrefSize.Height() ) - 1, fScaleY = -1.0;
    else
        nMoveY = 0, fScaleY = 1.0;

    if( ( fScaleX != 1.0 ) || ( fScaleY != 1.0 ) )
    {
        Scale( fScaleX, fScaleY );
        Move( nMoveX, nMoveY );
        SetPrefSize( aOldPrefSize );
        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::Move( long nX, long nY )
{
    const Size      aBaseOffset( nX, nY );
    Size            aOffset( aBaseOffset );
    VirtualDevice   aMapVDev;

    aMapVDev.EnableOutput( FALSE );
    aMapVDev.SetMapMode( GetPrefMapMode() );

    for( MetaAction* pAct = (MetaAction*) First(); pAct; pAct = (MetaAction*) Next() )
    {
        const long  nType = pAct->GetType();
        MetaAction* pModAct;

        if( pAct->GetRefCount() > 1 )
        {
            Replace( pModAct = pAct->Clone(), GetCurPos() );
            pAct->Delete();
        }
        else
            pModAct = pAct;

        if( ( META_MAPMODE_ACTION == nType ) ||
            ( META_PUSH_ACTION == nType ) ||
            ( META_POP_ACTION == nType ) )
        {
            pModAct->Execute( &aMapVDev );
            aOffset = aMapVDev.LogicToLogic( aBaseOffset, GetPrefMapMode(), aMapVDev.GetMapMode() );
        }

        pModAct->Move( aOffset.Width(), aOffset.Height() );
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Scale( double fScaleX, double fScaleY )
{
    for( MetaAction* pAct = (MetaAction*) First(); pAct; pAct = (MetaAction*) Next() )
    {
        MetaAction* pModAct;

        if( pAct->GetRefCount() > 1 )
        {
            Replace( pModAct = pAct->Clone(), GetCurPos() );
            pAct->Delete();
        }
        else
            pModAct = pAct;

        pModAct->Scale( fScaleX, fScaleY );
    }

    aPrefSize.Width() = FRound( aPrefSize.Width() * fScaleX );
    aPrefSize.Height() = FRound( aPrefSize.Height() * fScaleY );
}

// ------------------------------------------------------------------------

void GDIMetaFile::Scale( const Fraction& rScaleX, const Fraction& rScaleY )
{
    Scale( (double) rScaleX, (double) rScaleY );
}

// ------------------------------------------------------------------------

Point GDIMetaFile::ImplGetRotatedPoint( const Point& rPt, const Point& rRotatePt,
                                        const Size& rOffset, double fSin, double fCos )
{
    const long nX = rPt.X() - rRotatePt.X();
    const long nY = rPt.Y() - rRotatePt.Y();

    return Point( FRound( fCos * nX + fSin * nY ) + rRotatePt.X() + rOffset.Width(),
                  -FRound( fSin * nX - fCos * nY ) + rRotatePt.Y() + rOffset.Height() );
}

// ------------------------------------------------------------------------

Polygon GDIMetaFile::ImplGetRotatedPolygon( const Polygon& rPoly, const Point& rRotatePt,
                                            const Size& rOffset, double fSin, double fCos )
{
    Polygon aRet( rPoly );

    aRet.Rotate( rRotatePt, fSin, fCos );
    aRet.Move( rOffset.Width(), rOffset.Height() );

    return aRet;
}

// ------------------------------------------------------------------------

PolyPolygon GDIMetaFile::ImplGetRotatedPolyPolygon( const PolyPolygon& rPolyPoly, const Point& rRotatePt,
                                                    const Size& rOffset, double fSin, double fCos )
{
    PolyPolygon aRet( rPolyPoly );

    aRet.Rotate( rRotatePt, fSin, fCos );
    aRet.Move( rOffset.Width(), rOffset.Height() );

    return aRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::Rotate( long nAngle10 )
{
    nAngle10 %= 3600L;
    nAngle10 = ( nAngle10 < 0L ) ? ( 3599L + nAngle10 ) : nAngle10;

    if( nAngle10 )
    {
        GDIMetaFile     aMtf;
        VirtualDevice   aMapVDev;
        const double    fAngle = F_PI1800 * nAngle10;
        const double    fSin = sin( fAngle );
        const double    fCos = cos( fAngle );
        Rectangle       aRect=Rectangle( Point(), GetPrefSize() );
        Polygon         aPoly( aRect );

        aPoly.Rotate( Point(), fSin, fCos );

        aMapVDev.EnableOutput( FALSE );
        aMapVDev.SetMapMode( GetPrefMapMode() );

        const Rectangle aNewBound( aPoly.GetBoundRect() );

        const Point aOrigin( GetPrefMapMode().GetOrigin().X(), GetPrefMapMode().GetOrigin().Y() );
        const Size  aOffset( -aNewBound.Left(), -aNewBound.Top() );

        Point     aRotAnchor( aOrigin );
        Size      aRotOffset( aOffset );

        for( MetaAction* pAction = (MetaAction*) First(); pAction; pAction = (MetaAction*) Next() )
        {
            const USHORT nType = pAction->GetType();

            switch( nType )
            {
                case( META_PIXEL_ACTION ):
                {
                    MetaPixelAction* pAct = (MetaPixelAction*) pAction;
                    aMtf.AddAction( new MetaPixelAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                              pAct->GetColor() ) );
                }
                break;

                case( META_POINT_ACTION ):
                {
                    MetaPointAction* pAct = (MetaPointAction*) pAction;
                    aMtf.AddAction( new MetaPointAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_LINE_ACTION ):
                {
                    MetaLineAction* pAct = (MetaLineAction*) pAction;
                    aMtf.AddAction( new MetaLineAction( ImplGetRotatedPoint( pAct->GetStartPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                        ImplGetRotatedPoint( pAct->GetEndPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                        pAct->GetLineInfo() ) );
                }
                break;

                case( META_RECT_ACTION ):
                {
                    MetaRectAction* pAct = (MetaRectAction*) pAction;
                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( pAct->GetRect(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_ROUNDRECT_ACTION ):
                {
                    MetaRoundRectAction*    pAct = (MetaRoundRectAction*) pAction;
                    const Polygon           aRoundRectPoly( pAct->GetRect(), pAct->GetHorzRound(), pAct->GetVertRound() );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aRoundRectPoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_ELLIPSE_ACTION ):
                {
                    MetaEllipseAction*      pAct = (MetaEllipseAction*) pAction;
                    const Polygon           aEllipsePoly( pAct->GetRect().Center(), pAct->GetRect().GetWidth() >> 1, pAct->GetRect().GetHeight() >> 1 );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aEllipsePoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_ARC_ACTION ):
                {
                    MetaArcAction*  pAct = (MetaArcAction*) pAction;
                    const Polygon   aArcPoly( pAct->GetRect(), pAct->GetStartPoint(), pAct->GetEndPoint(), POLY_ARC );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aArcPoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_PIE_ACTION ):
                {
                    MetaPieAction*  pAct = (MetaPieAction*) pAction;
                    const Polygon   aPiePoly( pAct->GetRect(), pAct->GetStartPoint(), pAct->GetEndPoint(), POLY_PIE );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aPiePoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_CHORD_ACTION ):
                {
                    MetaChordAction*    pAct = (MetaChordAction*) pAction;
                    const Polygon       aChordPoly( pAct->GetRect(), pAct->GetStartPoint(), pAct->GetEndPoint(), POLY_CHORD );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aChordPoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_POLYLINE_ACTION ):
                {
                    MetaPolyLineAction* pAct = (MetaPolyLineAction*) pAction;
                    aMtf.AddAction( new MetaPolyLineAction( ImplGetRotatedPolygon( pAct->GetPolygon(), aRotAnchor, aRotOffset, fSin, fCos ), pAct->GetLineInfo() ) );
                }
                break;

                case( META_POLYGON_ACTION ):
                {
                    MetaPolygonAction* pAct = (MetaPolygonAction*) pAction;
                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( pAct->GetPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_POLYPOLYGON_ACTION ):
                {
                    MetaPolyPolygonAction* pAct = (MetaPolyPolygonAction*) pAction;
                    aMtf.AddAction( new MetaPolyPolygonAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_TEXT_ACTION ):
                {
                    MetaTextAction* pAct = (MetaTextAction*) pAction;
                    aMtf.AddAction( new MetaTextAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                             pAct->GetText(), pAct->GetIndex(), pAct->GetLen() ) );
                }
                break;

                case( META_TEXTARRAY_ACTION ):
                {
                    MetaTextArrayAction* pAct = (MetaTextArrayAction*) pAction;
                    aMtf.AddAction( new MetaTextArrayAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                  pAct->GetText(), pAct->GetDXArray(), pAct->GetIndex(), pAct->GetLen() ) );
                }
                break;

                case( META_STRETCHTEXT_ACTION ):
                {
                    MetaStretchTextAction* pAct = (MetaStretchTextAction*) pAction;
                    aMtf.AddAction( new MetaStretchTextAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                    pAct->GetWidth(), pAct->GetText(), pAct->GetIndex(), pAct->GetLen() ) );
                }
                break;

                case( META_TEXTLINE_ACTION ):
                {
                    MetaTextLineAction* pAct = (MetaTextLineAction*) pAction;
                    aMtf.AddAction( new MetaTextLineAction( ImplGetRotatedPoint( pAct->GetStartPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                 pAct->GetWidth(), pAct->GetStrikeout(), pAct->GetUnderline() ) );
                }
                break;

                case( META_BMPSCALE_ACTION ):
                {
                    MetaBmpScaleAction* pAct = (MetaBmpScaleAction*) pAction;
                    Polygon             aBmpPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle           aBmpRect( aBmpPoly.GetBoundRect() );
                    BitmapEx            aBmpEx( pAct->GetBitmap() );

                    aBmpEx.Rotate( nAngle10, Color( COL_TRANSPARENT ) );
                    aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(),
                                                              aBmpEx ) );
                }
                break;

                case( META_BMPSCALEPART_ACTION ):
                {
                    MetaBmpScalePartAction* pAct = (MetaBmpScalePartAction*) pAction;
                    Polygon                 aBmpPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetDestPoint(), pAct->GetDestSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle               aBmpRect( aBmpPoly.GetBoundRect() );
                    BitmapEx                aBmpEx( pAct->GetBitmap() );

                    aBmpEx.Crop( Rectangle( pAct->GetSrcPoint(), pAct->GetSrcSize() ) );
                    aBmpEx.Rotate( nAngle10, Color( COL_TRANSPARENT ) );

                    aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(), aBmpEx ) );
                }
                break;

                case( META_BMPEXSCALE_ACTION ):
                {
                    MetaBmpExScaleAction*   pAct = (MetaBmpExScaleAction*) pAction;
                    Polygon                 aBmpPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle               aBmpRect( aBmpPoly.GetBoundRect() );
                    BitmapEx                aBmpEx( pAct->GetBitmapEx() );

                    aBmpEx.Rotate( nAngle10, Color( COL_TRANSPARENT ) );

                    aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(), aBmpEx ) );
                }
                break;

                case( META_BMPEXSCALEPART_ACTION ):
                {
                    MetaBmpExScalePartAction*   pAct = (MetaBmpExScalePartAction*) pAction;
                    Polygon                     aBmpPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetDestPoint(), pAct->GetDestSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle                   aBmpRect( aBmpPoly.GetBoundRect() );
                    BitmapEx                    aBmpEx( pAct->GetBitmapEx() );

                    aBmpEx.Crop( Rectangle( pAct->GetSrcPoint(), pAct->GetSrcSize() ) );
                    aBmpEx.Rotate( nAngle10, Color( COL_TRANSPARENT ) );

                    aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(), aBmpEx ) );
                }
                break;

                case( META_GRADIENT_ACTION ):
                {
                    MetaGradientAction* pAct = (MetaGradientAction*) pAction;
                    aMtf.AddAction( new MetaGradientAction( ImplGetRotatedPolygon( pAct->GetRect(), aRotAnchor, aRotOffset, fSin, fCos ).GetBoundRect(),
                                                                                   pAct->GetGradient() ) );
                }
                break;

                case( META_GRADIENTEX_ACTION ):
                {
                    MetaGradientExAction* pAct = (MetaGradientExAction*) pAction;
                    Gradient              aGradient(  );

                    aMtf.AddAction( new MetaGradientExAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                         pAct->GetGradient() ) );
                }
                break;

                case( META_HATCH_ACTION ):
                {
                    MetaHatchAction*    pAct = (MetaHatchAction*) pAction;
                    Hatch               aHatch( pAct->GetHatch() );

                    aHatch.SetAngle( aHatch.GetAngle() + (USHORT) nAngle10 );
                    aMtf.AddAction( new MetaHatchAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                    aHatch ) );
                }
                break;

                case( META_TRANSPARENT_ACTION ):
                {
                    MetaTransparentAction* pAct = (MetaTransparentAction*) pAction;
                    aMtf.AddAction( new MetaTransparentAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                          pAct->GetTransparence() ) );
                }
                break;

                case( META_FLOATTRANSPARENT_ACTION ):
                {
                    MetaFloatTransparentAction* pAct = (MetaFloatTransparentAction*) pAction;
                    GDIMetaFile                 aTransMtf( pAct->GetGDIMetaFile() );
                    Polygon                     aMtfPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle                   aMtfRect( aMtfPoly.GetBoundRect() );

                    aTransMtf.Rotate( nAngle10 );
                    aMtf.AddAction( new MetaFloatTransparentAction( aTransMtf, aMtfRect.TopLeft(), aMtfRect.GetSize(),
                                                                    pAct->GetGradient() ) );
                }
                break;

                case( META_EPS_ACTION ):
                {
                    MetaEPSAction*  pAct = (MetaEPSAction*) pAction;
                    GDIMetaFile     aEPSMtf( pAct->GetSubstitute() );
                    Polygon         aEPSPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle       aEPSRect( aEPSPoly.GetBoundRect() );

                    aEPSMtf.Rotate( nAngle10 );
                    aMtf.AddAction( new MetaEPSAction( aEPSRect.TopLeft(), aEPSRect.GetSize(),
                                                       pAct->GetLink(), aEPSMtf ) );
                }
                break;

                case( META_CLIPREGION_ACTION ):
                {
                    MetaClipRegionAction* pAct = (MetaClipRegionAction*) pAction;

                    if( pAct->IsClipping() && pAct->GetRegion().HasPolyPolygon() )
                        aMtf.AddAction( new MetaClipRegionAction( Region( ImplGetRotatedPolyPolygon( pAct->GetRegion().GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ), TRUE ) );
                    else
                    {
                        pAction->Duplicate();
                        aMtf.AddAction( pAction );
                    }
                }
                break;

                case( META_ISECTRECTCLIPREGION_ACTION ):
                {
                    MetaISectRectClipRegionAction*  pAct = (MetaISectRectClipRegionAction*) pAction;
                    aMtf.AddAction( new MetaISectRegionClipRegionAction( ImplGetRotatedPolygon( pAct->GetRect(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_ISECTREGIONCLIPREGION_ACTION ):
                {
                    MetaISectRegionClipRegionAction*    pAct = (MetaISectRegionClipRegionAction*) pAction;
                    const Region&                       rRegion = pAct->GetRegion();

                    if( rRegion.HasPolyPolygon() )
                        aMtf.AddAction( new MetaISectRegionClipRegionAction( Region( ImplGetRotatedPolyPolygon( rRegion.GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ) ) );
                    else
                    {
                        pAction->Duplicate();
                        aMtf.AddAction( pAction );
                    }
                }
                break;

                case( META_REFPOINT_ACTION ):
                {
                    MetaRefPointAction* pAct = (MetaRefPointAction*) pAction;
                    aMtf.AddAction( new MetaRefPointAction( ImplGetRotatedPoint( pAct->GetRefPoint(), aRotAnchor, aRotOffset, fSin, fCos ), pAct->IsSetting() ) );
                }
                break;

                case( META_FONT_ACTION ):
                {
                    MetaFontAction* pAct = (MetaFontAction*) pAction;
                    Font            aFont( pAct->GetFont() );

                    aFont.SetOrientation( aFont.GetOrientation() + (USHORT) nAngle10 );
                    aMtf.AddAction( new MetaFontAction( aFont ) );
                }
                break;

                case( META_BMP_ACTION ):
                case( META_BMPEX_ACTION ):
                case( META_MASK_ACTION ):
                case( META_MASKSCALE_ACTION ):
                case( META_MASKSCALEPART_ACTION ):
                case( META_WALLPAPER_ACTION ):
                case( META_TEXTRECT_ACTION ):
                case( META_MOVECLIPREGION_ACTION ):
                {
                    DBG_ERROR( "GDIMetaFile::Rotate(): unsupported action" );
                }
                break;

                default:
                {
                    pAction->Execute( &aMapVDev );
                    pAction->Duplicate();
                    aMtf.AddAction( pAction );

                    // update rotation point and offset, if necessary
                    if( ( META_MAPMODE_ACTION == nType ) ||
                        ( META_PUSH_ACTION == nType ) ||
                        ( META_POP_ACTION == nType ) )
                    {
                        aRotAnchor = aMapVDev.LogicToLogic( aOrigin, aPrefMapMode, aMapVDev.GetMapMode() );
                        aRotOffset = aMapVDev.LogicToLogic( aOffset, aPrefMapMode, aMapVDev.GetMapMode() );
                    }
                }
                break;
            }
        }

        aMtf.aPrefMapMode = aPrefMapMode;
        aMtf.aPrefSize = aNewBound.GetSize();

        *this = aMtf;
    }
}

// ------------------------------------------------------------------------

Color GDIMetaFile::ImplColAdjustFnc( const Color& rColor, const void* pColParam )
{
    return Color( rColor.GetTransparency(),
                  ( (const ImplColAdjustParam*) pColParam )->pMapR[ rColor.GetRed() ],
                  ( (const ImplColAdjustParam*) pColParam )->pMapG[ rColor.GetGreen() ],
                  ( (const ImplColAdjustParam*) pColParam )->pMapB[ rColor.GetBlue() ] );

}

// ------------------------------------------------------------------------

BitmapEx GDIMetaFile::ImplBmpAdjustFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    const ImplBmpAdjustParam*   p = (const ImplBmpAdjustParam*) pBmpParam;
    BitmapEx                    aRet( rBmpEx );

    aRet.Adjust( p->nLuminancePercent, p->nContrastPercent,
                 p->nChannelRPercent, p->nChannelGPercent, p->nChannelBPercent,
                 p->fGamma, p->bInvert );

    return aRet;
}

// ------------------------------------------------------------------------

Color GDIMetaFile::ImplColConvertFnc( const Color& rColor, const void* pColParam )
{
    BYTE cLum = rColor.GetLuminance();

    if( MTF_CONVERSION_1BIT_THRESHOLD == ( (const ImplColConvertParam*) pColParam )->eConversion )
        cLum = ( cLum < 128 ) ? 0 : 255;

    return Color( rColor.GetTransparency(), cLum, cLum, cLum );
}

// ------------------------------------------------------------------------

BitmapEx GDIMetaFile::ImplBmpConvertFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    BitmapEx aRet( rBmpEx );

    aRet.Convert( ( (const ImplBmpConvertParam*) pBmpParam )->eConversion );

    return aRet;
}

// ------------------------------------------------------------------------

Color GDIMetaFile::ImplColMonoFnc( const Color& rColor, const void* pColParam )
{
    return( ( (const ImplColMonoParam*) pColParam )->aColor );
}

// ------------------------------------------------------------------------

BitmapEx GDIMetaFile::ImplBmpMonoFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    BitmapPalette aPal( 3 );

    aPal[ 0 ] = Color( COL_BLACK );
    aPal[ 1 ] = Color( COL_WHITE );
    aPal[ 2 ] = ( (const ImplBmpMonoParam*) pBmpParam )->aColor;

    Bitmap aBmp( rBmpEx.GetSizePixel(), 4, &aPal );
    aBmp.Erase( ( (const ImplBmpMonoParam*) pBmpParam )->aColor );

    if( rBmpEx.IsAlpha() )
        return BitmapEx( aBmp, rBmpEx.GetAlpha() );
    else if( rBmpEx.IsTransparent() )
        return BitmapEx( aBmp, rBmpEx.GetMask() );
    else
        return aBmp;
}

// ------------------------------------------------------------------------

Color GDIMetaFile::ImplColReplaceFnc( const Color& rColor, const void* pColParam )
{
    const ULONG nR = rColor.GetRed(), nG = rColor.GetGreen(), nB = rColor.GetBlue();

    for( ULONG i = 0; i < ( (const ImplColReplaceParam*) pColParam )->nCount; i++ )
    {
        if( ( ( (const ImplColReplaceParam*) pColParam )->pMinR[ i ] <= nR ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMaxR[ i ] >= nR ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMinG[ i ] <= nG ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMaxG[ i ] >= nG ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMinB[ i ] <= nB ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMaxB[ i ] >= nB ) )
        {
            return( ( (const ImplColReplaceParam*) pColParam )->pDstCols[ i ] );
        }
    }

    return rColor;
}

// ------------------------------------------------------------------------

BitmapEx GDIMetaFile::ImplBmpReplaceFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    const ImplBmpReplaceParam*  p = (const ImplBmpReplaceParam*) pBmpParam;
    BitmapEx                    aRet( rBmpEx );

    aRet.Replace( p->pSrcCols, p->pDstCols, p->nCount, p->pTols );

    return aRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::ImplExchangeColors( ColorExchangeFnc pFncCol, const void* pColParam,
                                      BmpExchangeFnc pFncBmp, const void* pBmpParam )
{
    GDIMetaFile aMtf;

    aMtf.aPrefSize = aPrefSize;
    aMtf.aPrefMapMode = aPrefMapMode;

    for( MetaAction* pAction = (MetaAction*) First(); pAction; pAction = (MetaAction*) Next() )
    {
        const USHORT nType = pAction->GetType();

        switch( nType )
        {
            case( META_PIXEL_ACTION ):
            {
                MetaPixelAction* pAct = (MetaPixelAction*) pAction;
                aMtf.Insert( new MetaPixelAction( pAct->GetPoint(), pFncCol( pAct->GetColor(), pColParam ) ), LIST_APPEND );
            }
            break;

            case( META_LINECOLOR_ACTION ):
            {
                MetaLineColorAction* pAct = (MetaLineColorAction*) pAction;

                if( !pAct->IsSetting() )
                    pAct->Duplicate();
                else
                    pAct = new MetaLineColorAction( pFncCol( pAct->GetColor(), pColParam ), TRUE );

                aMtf.Insert( pAct, LIST_APPEND );
            }
            break;

            case( META_FILLCOLOR_ACTION ):
            {
                MetaFillColorAction* pAct = (MetaFillColorAction*) pAction;

                if( !pAct->IsSetting() )
                    pAct->Duplicate();
                else
                    pAct = new MetaFillColorAction( pFncCol( pAct->GetColor(), pColParam ), TRUE );

                aMtf.Insert( pAct, LIST_APPEND );
            }
            break;

            case( META_TEXTCOLOR_ACTION ):
            {
                MetaTextColorAction* pAct = (MetaTextColorAction*) pAction;
                aMtf.Insert( new MetaTextColorAction( pFncCol( pAct->GetColor(), pColParam ) ), LIST_APPEND );
            }
            break;

            case( META_TEXTFILLCOLOR_ACTION ):
            {
                MetaTextFillColorAction* pAct = (MetaTextFillColorAction*) pAction;

                if( !pAct->IsSetting() )
                    pAct->Duplicate();
                else
                    pAct = new MetaTextFillColorAction( pFncCol( pAct->GetColor(), pColParam ), TRUE );

                aMtf.Insert( pAct, LIST_APPEND );
            }
            break;

            case( META_TEXTLINECOLOR_ACTION ):
            {
                MetaTextLineColorAction* pAct = (MetaTextLineColorAction*) pAction;

                if( !pAct->IsSetting() )
                    pAct->Duplicate();
                else
                    pAct = new MetaTextLineColorAction( pFncCol( pAct->GetColor(), pColParam ), TRUE );

                aMtf.Insert( pAct, LIST_APPEND );
            }
            break;

            case( META_FONT_ACTION ):
            {
                MetaFontAction* pAct = (MetaFontAction*) pAction;
                Font            aFont( pAct->GetFont() );

                aFont.SetColor( pFncCol( aFont.GetColor(), pColParam ) );
                aFont.SetFillColor( pFncCol( aFont.GetFillColor(), pColParam ) );
                aMtf.Insert( new MetaFontAction( aFont ), LIST_APPEND );
            }
            break;

            case( META_WALLPAPER_ACTION ):
            {
                MetaWallpaperAction*    pAct = (MetaWallpaperAction*) pAction;
                Wallpaper               aWall( pAct->GetWallpaper() );
                const Rectangle&        rRect = pAct->GetRect();

                aWall.SetColor( pFncCol( aWall.GetColor(), pColParam ) );

                if( aWall.IsBitmap() )
                    aWall.SetBitmap( pFncBmp( aWall.GetBitmap(), pBmpParam ) );

                if( aWall.IsGradient() )
                {
                    Gradient aGradient( aWall.GetGradient() );

                    aGradient.SetStartColor( pFncCol( aGradient.GetStartColor(), pColParam ) );
                    aGradient.SetEndColor( pFncCol( aGradient.GetEndColor(), pColParam ) );
                    aWall.SetGradient( aGradient );
                }

                aMtf.Insert( new MetaWallpaperAction( rRect, aWall ), LIST_APPEND );
            }
            break;

            case( META_BMP_ACTION ):
            case( META_BMPEX_ACTION ):
            case( META_MASK_ACTION ):
            {
                DBG_ERROR( "Don't use bitmap actions of this type in metafiles!" );
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                MetaBmpScaleAction* pAct = (MetaBmpScaleAction*) pAction;
                aMtf.Insert( new MetaBmpScaleAction( pAct->GetPoint(), pAct->GetSize(),
                                                     pFncBmp( pAct->GetBitmap(), pBmpParam ).GetBitmap() ),
                                                     LIST_APPEND );
            }
            break;

            case( META_BMPSCALEPART_ACTION ):
            {
                MetaBmpScalePartAction* pAct = (MetaBmpScalePartAction*) pAction;
                aMtf.Insert( new MetaBmpScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                         pAct->GetSrcPoint(), pAct->GetSrcSize(),
                                                         pFncBmp( pAct->GetBitmap(), pBmpParam ).GetBitmap() ),
                                                         LIST_APPEND );
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                MetaBmpExScaleAction* pAct = (MetaBmpExScaleAction*) pAction;
                aMtf.Insert( new MetaBmpExScaleAction( pAct->GetPoint(), pAct->GetSize(),
                                                       pFncBmp( pAct->GetBitmapEx(), pBmpParam ) ),
                                                       LIST_APPEND );
            }
            break;

            case( META_BMPEXSCALEPART_ACTION ):
            {
                MetaBmpExScalePartAction* pAct = (MetaBmpExScalePartAction*) pAction;
                aMtf.Insert( new MetaBmpExScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                           pAct->GetSrcPoint(), pAct->GetSrcSize(),
                                                           pFncBmp( pAct->GetBitmapEx(), pBmpParam ) ),
                                                           LIST_APPEND );
            }
            break;

            case( META_MASKSCALE_ACTION ):
            {
                MetaMaskScaleAction* pAct = (MetaMaskScaleAction*) pAction;
                aMtf.Insert( new MetaMaskScaleAction( pAct->GetPoint(), pAct->GetSize(),
                                                      pAct->GetBitmap(),
                                                      pFncCol( pAct->GetColor(), pColParam ) ),
                                                      LIST_APPEND );
            }
            break;

            case( META_MASKSCALEPART_ACTION ):
            {
                MetaMaskScalePartAction* pAct = (MetaMaskScalePartAction*) pAction;
                aMtf.Insert( new MetaMaskScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                          pAct->GetSrcPoint(), pAct->GetSrcSize(),
                                                          pAct->GetBitmap(),
                                                          pFncCol( pAct->GetColor(), pColParam ) ),
                                                          LIST_APPEND );
            }
            break;

            case( META_GRADIENT_ACTION ):
            {
                MetaGradientAction* pAct = (MetaGradientAction*) pAction;
                Gradient            aGradient( pAct->GetGradient() );

                aGradient.SetStartColor( pFncCol( aGradient.GetStartColor(), pColParam ) );
                aGradient.SetEndColor( pFncCol( aGradient.GetEndColor(), pColParam ) );
                aMtf.Insert( new MetaGradientAction( pAct->GetRect(), aGradient ), LIST_APPEND );
            }
            break;

            case( META_GRADIENTEX_ACTION ):
            {
                MetaGradientExAction* pAct = (MetaGradientExAction*) pAction;
                Gradient              aGradient( pAct->GetGradient() );

                aGradient.SetStartColor( pFncCol( aGradient.GetStartColor(), pColParam ) );
                aGradient.SetEndColor( pFncCol( aGradient.GetEndColor(), pColParam ) );
                aMtf.Insert( new MetaGradientExAction( pAct->GetPolyPolygon(), aGradient ), LIST_APPEND );
            }
            break;

            case( META_HATCH_ACTION ):
            {
                MetaHatchAction*    pAct = (MetaHatchAction*) pAction;
                Hatch               aHatch( pAct->GetHatch() );

                aHatch.SetColor( pFncCol( aHatch.GetColor(), pColParam ) );
                aMtf.Insert( new MetaHatchAction( pAct->GetPolyPolygon(), aHatch ), LIST_APPEND );
            }
            break;

            case( META_FLOATTRANSPARENT_ACTION ):
            {
                MetaFloatTransparentAction* pAct = (MetaFloatTransparentAction*) pAction;
                GDIMetaFile                 aTransMtf( pAct->GetGDIMetaFile() );

                aTransMtf.ImplExchangeColors( pFncCol, pColParam, pFncBmp, pBmpParam );
                aMtf.Insert( new MetaFloatTransparentAction( aTransMtf,
                                                             pAct->GetPoint(), pAct->GetSize(),
                                                             pAct->GetGradient() ),
                                                             LIST_APPEND );
            }
            break;

            case( META_EPS_ACTION ):
            {
                MetaEPSAction*  pAct = (MetaEPSAction*) pAction;
                GDIMetaFile     aSubst( pAct->GetSubstitute() );

                aSubst.ImplExchangeColors( pFncCol, pColParam, pFncBmp, pBmpParam );
                aMtf.Insert( new MetaEPSAction( pAct->GetPoint(), pAct->GetSize(),
                                                pAct->GetLink(), aSubst ),
                                                LIST_APPEND );
            }
            break;

            default:
            {
                pAction->Duplicate();
                aMtf.Insert( pAction, LIST_APPEND );
            }
            break;
        }
    }

    *this = aMtf;
}

// ------------------------------------------------------------------------

void GDIMetaFile::Adjust( short nLuminancePercent, short nContrastPercent,
                          short nChannelRPercent, short nChannelGPercent,
                          short nChannelBPercent, double fGamma, BOOL bInvert )
{
    // nothing to do? => return quickly
    if( nLuminancePercent || nContrastPercent ||
        nChannelRPercent || nChannelGPercent || nChannelBPercent ||
        ( fGamma != 1.0 ) || bInvert )
    {
        double              fM, fROff, fGOff, fBOff, fOff;
        ImplColAdjustParam  aColParam;
        ImplBmpAdjustParam  aBmpParam;

        aColParam.pMapR = new BYTE[ 256 ];
        aColParam.pMapG = new BYTE[ 256 ];
        aColParam.pMapB = new BYTE[ 256 ];

        // calculate slope
        if( nContrastPercent >= 0 )
            fM = 128.0 / ( 128.0 - 1.27 * MinMax( nContrastPercent, 0L, 100L ) );
        else
            fM = ( 128.0 + 1.27 * MinMax( nContrastPercent, -100L, 0L ) ) / 128.0;

        // total offset = luminance offset + contrast offset
        fOff = MinMax( nLuminancePercent, -100L, 100L ) * 2.55 + 128.0 - fM * 128.0;

        // channel offset = channel offset  + total offset
        fROff = nChannelRPercent * 2.55 + fOff;
        fGOff = nChannelGPercent * 2.55 + fOff;
        fBOff = nChannelBPercent * 2.55 + fOff;

        // calculate gamma value
        fGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );
        const BOOL bGamma = ( fGamma != 1.0 );

        // create mapping table
        for( long nX = 0L; nX < 256L; nX++ )
        {
            aColParam.pMapR[ nX ] = (BYTE) MinMax( FRound( nX * fM + fROff ), 0L, 255L );
            aColParam.pMapG[ nX ] = (BYTE) MinMax( FRound( nX * fM + fGOff ), 0L, 255L );
            aColParam.pMapB[ nX ] = (BYTE) MinMax( FRound( nX * fM + fBOff ), 0L, 255L );

            if( bGamma )
            {
                aColParam.pMapR[ nX ] = GAMMA( aColParam.pMapR[ nX ], fGamma );
                aColParam.pMapG[ nX ] = GAMMA( aColParam.pMapG[ nX ], fGamma );
                aColParam.pMapB[ nX ] = GAMMA( aColParam.pMapB[ nX ], fGamma );
            }

            if( bInvert )
            {
                aColParam.pMapR[ nX ] = ~aColParam.pMapR[ nX ];
                aColParam.pMapG[ nX ] = ~aColParam.pMapG[ nX ];
                aColParam.pMapB[ nX ] = ~aColParam.pMapB[ nX ];
            }
        }

        aBmpParam.nLuminancePercent = nLuminancePercent;
        aBmpParam.nContrastPercent = nContrastPercent;
        aBmpParam.nChannelRPercent = nChannelRPercent;
        aBmpParam.nChannelGPercent = nChannelGPercent;
        aBmpParam.nChannelBPercent = nChannelBPercent;
        aBmpParam.fGamma = fGamma;
        aBmpParam.bInvert = bInvert;

        // do color adjustment
        ImplExchangeColors( ImplColAdjustFnc, &aColParam, ImplBmpAdjustFnc, &aBmpParam );

        delete[] aColParam.pMapR;
        delete[] aColParam.pMapG;
        delete[] aColParam.pMapB;
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Convert( MtfConversion eConversion )
{
    // nothing to do? => return quickly
    if( eConversion != MTF_CONVERSION_NONE )
    {
        ImplColConvertParam aColParam;
        ImplBmpConvertParam aBmpParam;

        aColParam.eConversion = eConversion;
        aBmpParam.eConversion = ( MTF_CONVERSION_1BIT_THRESHOLD == eConversion ) ? BMP_CONVERSION_1BIT_THRESHOLD : BMP_CONVERSION_8BIT_GREYS;

        ImplExchangeColors( ImplColConvertFnc, &aColParam, ImplBmpConvertFnc, &aBmpParam );
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::ReplaceColors( const Color& rSearchColor, const Color& rReplaceColor, ULONG nTol )
{
    ReplaceColors( &rSearchColor, &rReplaceColor, 1, &nTol );
}

// ------------------------------------------------------------------------

void GDIMetaFile::ReplaceColors( const Color* pSearchColors, const Color* pReplaceColors, ULONG nColorCount, ULONG* pTols )
{
    ImplColReplaceParam aColParam;
    ImplBmpReplaceParam aBmpParam;

    aColParam.pMinR = new ULONG[ nColorCount ];
    aColParam.pMaxR = new ULONG[ nColorCount ];
    aColParam.pMinG = new ULONG[ nColorCount ];
    aColParam.pMaxG = new ULONG[ nColorCount ];
    aColParam.pMinB = new ULONG[ nColorCount ];
    aColParam.pMaxB = new ULONG[ nColorCount ];

    for( ULONG i = 0; i < nColorCount; i++ )
    {
        const long  nTol = pTols ? ( pTols[ i ] * 255 ) / 100 : 0;
        long        nVal;

        nVal = pSearchColors[ i ].GetRed();
        aColParam.pMinR[ i ] = (ULONG) Max( nVal - nTol, 0L );
        aColParam.pMaxR[ i ] = (ULONG) Min( nVal + nTol, 255L );

        nVal = pSearchColors[ i ].GetGreen();
        aColParam.pMinG[ i ] = (ULONG) Max( nVal - nTol, 0L );
        aColParam.pMaxG[ i ] = (ULONG) Min( nVal + nTol, 255L );

        nVal = pSearchColors[ i ].GetBlue();
        aColParam.pMinB[ i ] = (ULONG) Max( nVal - nTol, 0L );
        aColParam.pMaxB[ i ] = (ULONG) Min( nVal + nTol, 255L );
    }

    aColParam.pDstCols = pReplaceColors;
    aColParam.nCount = nColorCount;

    aBmpParam.pSrcCols = pSearchColors;
    aBmpParam.pDstCols = pReplaceColors;
    aBmpParam.nCount = nColorCount;
    aBmpParam.pTols = pTols;

    ImplExchangeColors( ImplColReplaceFnc, &aColParam, ImplBmpReplaceFnc, &aBmpParam );

    delete[] aColParam.pMinR;
    delete[] aColParam.pMaxR;
    delete[] aColParam.pMinG;
    delete[] aColParam.pMaxG;
    delete[] aColParam.pMinB;
    delete[] aColParam.pMaxB;
};

// ------------------------------------------------------------------------

GDIMetaFile GDIMetaFile::GetMonochromeMtf( const Color& rColor ) const
{
    GDIMetaFile aRet( *this );

    ImplColMonoParam    aColParam;
    ImplBmpMonoParam    aBmpParam;

    aColParam.aColor = rColor;
    aBmpParam.aColor = rColor;

    aRet.ImplExchangeColors( ImplColMonoFnc, &aColParam, ImplBmpMonoFnc, &aBmpParam );

    return aRet;
}

// ------------------------------------------------------------------------

ULONG GDIMetaFile::GetChecksum() const
{
    GDIMetaFile         aMtf;
    SvMemoryStream      aMemStm( 65535, 65535 );
    ImplMetaWriteData   aWriteData; aWriteData.meActualCharSet = aMemStm.GetStreamCharSet();
    SVBT16              aBT16;
    SVBT32              aBT32;
    ULONG               nCrc = 0;

    for( ULONG i = 0, nCount = GetActionCount(); i < nCount; i++ )
    {
        MetaAction* pAction = GetAction( i );

        switch( pAction->GetType() )
        {
            case( META_BMP_ACTION ):
            {
                MetaBmpAction* pAct = (MetaBmpAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                LongToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                MetaBmpScaleAction* pAct = (MetaBmpScaleAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                LongToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPSCALEPART_ACTION ):
            {
                MetaBmpScalePartAction* pAct = (MetaBmpScalePartAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                LongToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPEX_ACTION ):
            {
                MetaBmpExAction* pAct = (MetaBmpExAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                LongToSVBT32( pAct->GetBitmapEx().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                MetaBmpExScaleAction* pAct = (MetaBmpExScaleAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                LongToSVBT32( pAct->GetBitmapEx().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPEXSCALEPART_ACTION ):
            {
                MetaBmpExScalePartAction* pAct = (MetaBmpExScalePartAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                LongToSVBT32( pAct->GetBitmapEx().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_MASK_ACTION ):
            {
                MetaMaskAction* pAct = (MetaMaskAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                LongToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetColor().GetColor(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_MASKSCALE_ACTION ):
            {
                MetaMaskScaleAction* pAct = (MetaMaskScaleAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                LongToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetColor().GetColor(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_MASKSCALEPART_ACTION ):
            {
                MetaMaskScalePartAction* pAct = (MetaMaskScalePartAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                LongToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetColor().GetColor(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetDestSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                LongToSVBT32( pAct->GetSrcSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            default:
            {
                pAction->Write( aMemStm, &aWriteData );
                nCrc = rtl_crc32( nCrc, aMemStm.GetData(), aMemStm.Tell() );
                aMemStm.Seek( 0 );
            }
            break;
        }
    }

    return nCrc;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, GDIMetaFile& rGDIMetaFile )
{
    if( !rIStm.GetError() )
    {
        char    aId[ 7 ];
        ULONG   nStmPos = rIStm.Tell();
        USHORT  nOldFormat = rIStm.GetNumberFormatInt();
        BOOL    bError = FALSE;

        rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

        rIStm.Read( aId, 6 );
        aId[ 6 ] = 0;

        if ( !strcmp( aId, "VCLMTF" ) )
        {
            // new format
            VersionCompat*  pCompat;
            MetaAction*     pAction;
            UINT32          nStmCompressMode;
            UINT32          nCount;

            pCompat = new VersionCompat( rIStm, STREAM_READ );

            rIStm >> nStmCompressMode;
            rIStm >> rGDIMetaFile.aPrefMapMode;
            rIStm >> rGDIMetaFile.aPrefSize;
            rIStm >> nCount;

            delete pCompat;

            ImplMetaReadData aReadData;
            aReadData.meActualCharSet = rIStm.GetStreamCharSet();

            for( UINT32 nAction = 0UL; ( nAction < nCount ) && !rIStm.IsEof(); nAction++ )
            {
                pAction = MetaAction::ReadMetaAction( rIStm, &aReadData );

                if( pAction )
                    rGDIMetaFile.AddAction( pAction );
            }
        }
        else
        {
            // to avoid possible compiler optimizations => new/delete
            rIStm.Seek( nStmPos );
            delete( new SVMConverter( rIStm, rGDIMetaFile, CONVERT_FROM_SVM1 ) );
        }

        // check for errors
        if( rIStm.GetError() )
        {
            rGDIMetaFile.Clear();
            rIStm.Seek( nStmPos );
        }

        rIStm.SetNumberFormatInt( nOldFormat );
    }

    return rIStm;
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const GDIMetaFile& rGDIMetaFile )
{
    if( !rOStm.GetError() )
    {
        if( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_50 )
            ((GDIMetaFile&) rGDIMetaFile ).Write( rOStm );
        else
            delete( new SVMConverter( rOStm, (GDIMetaFile&) rGDIMetaFile, CONVERT_TO_SVM1 ) );
    }

    return rOStm;
}

// ------------------------------------------------------------------------

SvStream& GDIMetaFile::Read( SvStream& rIStm )
{
    Clear();
    rIStm >> *this;

    return rIStm;
}

// ------------------------------------------------------------------------

SvStream& GDIMetaFile::Write( SvStream& rOStm )
{
    VersionCompat*  pCompat;
    const UINT32    nStmCompressMode = rOStm.GetCompressMode();
    USHORT          nOldFormat = rOStm.GetNumberFormatInt();

    rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rOStm.Write( "VCLMTF", 6 );

    pCompat = new VersionCompat( rOStm, STREAM_WRITE, 1 );

    rOStm << nStmCompressMode;
    rOStm << aPrefMapMode;
    rOStm << aPrefSize;
    rOStm << (UINT32) GetActionCount();

    delete pCompat;

    ImplMetaWriteData aWriteData;
    aWriteData.meActualCharSet = rOStm.GetStreamCharSet();

    MetaAction* pAct = (MetaAction*)First();
    while ( pAct )
    {
        pAct->Write( rOStm, &aWriteData );
        pAct = (MetaAction*)Next();
    }

    rOStm.SetNumberFormatInt( nOldFormat );

    return rOStm;
}
