/*************************************************************************
 *
 *  $RCSfile: impgraph.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
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

#define _SV_IMPGRAPH_CXX

#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _NEW_HXX
#include <tools/new.hxx>
#endif
#include <impgraph.hxx>
#ifndef _GFXLINK_HXX
#include <gfxlink.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <cvtgrf.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <graph.hxx>
#endif

// -----------
// - Defines -
// -----------

#define GRAPHIC_MAXPARTLEN          256000L
#define GRAPHIC_MTFTOBMP_MAXEXT     2048
#define GRAPHIC_STREAMBUFSIZE       8192UL

#define SYS_WINMETAFILE         0x00000003UL
#define SYS_WNTMETAFILE         0x00000004UL
#define SYS_OS2METAFILE         0x00000005UL
#define SYS_MACMETAFILE         0x00000006UL

#define GRAPHIC_FORMAT_50       COMPAT_FORMAT( 'G', 'R', 'F', '5' )
#define NATIVE_FORMAT_50        COMPAT_FORMAT( 'N', 'A', 'T', '5' )

// ---------------
// - ImpSwapFile -
// ---------------

struct ImpSwapFile
{
    String  aSwapFileName;
    USHORT  nRefCount;
};

// -----------------
// - Graphicreader -
// -----------------

GraphicReader::~GraphicReader()
{
}

// --------------
// - ImpGraphic -
// --------------

ImpGraphic::ImpGraphic() :
        mpAnimation     ( NULL ),
        mpContext       ( NULL ),
        mpSwapFile      ( NULL ),
        mpGfxLink       ( NULL ),
        meType          ( GRAPHIC_NONE ),
        mnDocFilePos    ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( FALSE ),
        mbSwapUnderway  ( FALSE )
{
}

// ------------------------------------------------------------------------

ImpGraphic::ImpGraphic( const ImpGraphic& rImpGraphic ) :
        maEx            ( rImpGraphic.maEx ),
        maMetaFile      ( rImpGraphic.maMetaFile ),
        mpContext       ( NULL ),
        mpSwapFile      ( rImpGraphic.mpSwapFile ),
        meType          ( rImpGraphic.meType ),
        maDocFileName   ( rImpGraphic.maDocFileName ),
        mnDocFilePos    ( rImpGraphic.mnDocFilePos ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( rImpGraphic.mbSwapOut ),
        mbSwapUnderway  ( FALSE )
{
    if( mpSwapFile )
        mpSwapFile->nRefCount++;

    if( rImpGraphic.mpGfxLink )
        mpGfxLink = new GfxLink( *rImpGraphic.mpGfxLink );
    else
        mpGfxLink = NULL;

    if( rImpGraphic.mpAnimation )
    {
        mpAnimation = new Animation( *rImpGraphic.mpAnimation );
        maEx = mpAnimation->GetBitmapEx();
    }
    else
        mpAnimation = NULL;
}

// ------------------------------------------------------------------------

ImpGraphic::ImpGraphic( const Bitmap& rBitmap ) :
        maEx            ( rBitmap ),
        mpAnimation     ( NULL ),
        mpContext       ( NULL ),
        mpSwapFile      ( NULL ),
        mpGfxLink       ( NULL ),
        meType          ( !rBitmap ? GRAPHIC_NONE : GRAPHIC_BITMAP ),
        mnDocFilePos    ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( FALSE ),
        mbSwapUnderway  ( FALSE )
{
}

// ------------------------------------------------------------------------

ImpGraphic::ImpGraphic( const BitmapEx& rBitmapEx ) :
        maEx            ( rBitmapEx ),
        mpAnimation     ( NULL ),
        mpContext       ( NULL ),
        mpSwapFile      ( NULL ),
        mpGfxLink       ( NULL ),
        meType          ( !rBitmapEx ? GRAPHIC_NONE : GRAPHIC_BITMAP ),
        mnDocFilePos    ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( FALSE ),
        mbSwapUnderway  ( FALSE )
{
}

// ------------------------------------------------------------------------

ImpGraphic::ImpGraphic( const Animation& rAnimation ) :
        maEx            ( rAnimation.GetBitmapEx() ),
        mpAnimation     ( new Animation( rAnimation ) ),
        mpContext       ( NULL ),
        mpSwapFile      ( NULL ),
        mpGfxLink       ( NULL ),
        meType          ( GRAPHIC_BITMAP ),
        mnDocFilePos    ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( FALSE ),
        mbSwapUnderway  ( FALSE )
{
}

// ------------------------------------------------------------------------

ImpGraphic::ImpGraphic( const GDIMetaFile& rMtf ) :
        maMetaFile      ( rMtf ),
        mpAnimation     ( NULL ),
        mpContext       ( NULL ),
        mpSwapFile      ( NULL ),
        mpGfxLink       ( NULL ),
        meType          ( GRAPHIC_GDIMETAFILE ),
        mnDocFilePos    ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( FALSE ),
        mbSwapUnderway  ( FALSE )
{
}

// ------------------------------------------------------------------------

ImpGraphic::~ImpGraphic()
{
    ImplClear();

    if( (ULONG) mpContext > 1UL )
        delete mpContext;
}

// ------------------------------------------------------------------------

ImpGraphic& ImpGraphic::operator=( const ImpGraphic& rImpGraphic )
{
    if( &rImpGraphic != this )
    {
        if( !mbSwapUnderway )
            ImplClear();

        maMetaFile = rImpGraphic.maMetaFile;
        meType = rImpGraphic.meType;

        delete mpAnimation;

        if ( rImpGraphic.mpAnimation )
        {
            mpAnimation = new Animation( *rImpGraphic.mpAnimation );
            maEx = mpAnimation->GetBitmapEx();
        }
        else
        {
            mpAnimation = NULL;
            maEx = rImpGraphic.maEx;
        }

        if( !mbSwapUnderway )
        {
            maDocFileName = rImpGraphic.maDocFileName;
            mnDocFilePos = rImpGraphic.mnDocFilePos;
            mbSwapOut = rImpGraphic.mbSwapOut;
            mpSwapFile = rImpGraphic.mpSwapFile;

            if( mpSwapFile )
                mpSwapFile->nRefCount++;
        }

        delete mpGfxLink;

        if( rImpGraphic.mpGfxLink )
            mpGfxLink = new GfxLink( *rImpGraphic.mpGfxLink );
        else
            mpGfxLink = NULL;
    }

    return *this;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::operator==( const ImpGraphic& rImpGraphic ) const
{
    BOOL bRet = FALSE;

    if( this == &rImpGraphic )
        bRet = TRUE;
    else if( !ImplIsSwapOut() && ( rImpGraphic.meType == meType ) )
    {
        switch( meType )
        {
            case( GRAPHIC_NONE ):
                bRet = TRUE;
            break;

            case( GRAPHIC_GDIMETAFILE ):
            {
                if( rImpGraphic.maMetaFile == maMetaFile )
                    bRet = TRUE;
            }
            break;

            case( GRAPHIC_BITMAP ):
            {
                if( mpAnimation )
                {
                    if( rImpGraphic.mpAnimation && ( *rImpGraphic.mpAnimation == *mpAnimation ) )
                        bRet = TRUE;
                }
                else if( !rImpGraphic.mpAnimation && ( rImpGraphic.maEx == maEx ) )
                    bRet = TRUE;
            }
            break;

            default:
            break;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplClearGraphics( BOOL bCreateSwapInfo )
{
    if( bCreateSwapInfo && !ImplIsSwapOut() )
    {
        maSwapInfo.maPrefMapMode = ImplGetPrefMapMode();
        maSwapInfo.maPrefSize = ImplGetPrefSize();
    }

    maEx.Clear();
    maMetaFile.Clear();

    if( mpAnimation )
    {
        mpAnimation->Clear();
        delete mpAnimation;
        mpAnimation = NULL;
    }

    if( mpGfxLink )
    {
        delete mpGfxLink;
        mpGfxLink = NULL;
    }
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplClear()
{
    if( mpSwapFile )
    {
        if( mpSwapFile->nRefCount > 1 )
            mpSwapFile->nRefCount--;
        else
        {
            try
            {
                ::ucb::Content aCnt( INetURLObject( mpSwapFile->aSwapFileName, INET_PROT_FILE ).GetMainURL(),
                                     ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

                aCnt.executeCommand( ::rtl::OUString::createFromAscii( "delete" ),
                                     ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
            }
            catch( ::com::sun::star::ucb::CommandAbortedException& )
            {
                DBG_ERRORFILE( "CommandAbortedException" );
            }
            catch( ... )
            {
                DBG_ERRORFILE( "Any other exception" );
            }

            delete mpSwapFile;
        }

        mpSwapFile = NULL;
    }

    mbSwapOut = FALSE;
    mnDocFilePos = 0UL;
    maDocFileName.Erase();

    // cleanup
    ImplClearGraphics( FALSE );
    meType = GRAPHIC_NONE;
}

// ------------------------------------------------------------------------

GraphicType ImpGraphic::ImplGetType() const
{
    return meType;
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplSetDefaultType()
{
    ImplClear();
    meType = GRAPHIC_DEFAULT;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplIsSupportedGraphic() const
{
    return( meType != GRAPHIC_NONE );
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplIsTransparent() const
{
    BOOL bRet;

    if( meType == GRAPHIC_BITMAP )
        bRet = ( mpAnimation ? mpAnimation->IsTransparent() : maEx.IsTransparent() );
    else
        bRet = TRUE;

    return bRet;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplIsAlpha() const
{
    BOOL bRet;

    if( meType == GRAPHIC_BITMAP )
        bRet = ( NULL == mpAnimation ) && maEx.IsAlpha();
    else
        bRet = FALSE;

    return bRet;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplIsAnimated() const
{
    return( mpAnimation != NULL );
}

// ------------------------------------------------------------------------

Bitmap ImpGraphic::ImplGetBitmap() const
{
    Bitmap aRetBmp;

    if( meType == GRAPHIC_BITMAP )
    {
        const BitmapEx& rRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maEx );
        const Color     aReplaceColor( COL_WHITE );

        aRetBmp = rRetBmpEx.GetBitmap( &aReplaceColor );
    }
    else if( ( meType != GRAPHIC_DEFAULT ) && ImplIsSupportedGraphic() )
    {
        VirtualDevice   aVDev;
        Size            aSizePix( aVDev.LogicToPixel( maMetaFile.GetPrefSize(),
                                                      maMetaFile.GetPrefMapMode() ) );

        if( aSizePix.Width() && aSizePix.Height() &&
            ( aSizePix.Width() > GRAPHIC_MTFTOBMP_MAXEXT || aSizePix.Height() > GRAPHIC_MTFTOBMP_MAXEXT ) )
        {
            double fWH = (double) aSizePix.Width() / aSizePix.Height();

            if( fWH <= 1.0 )
            {
                aSizePix.Width() = FRound( fWH * GRAPHIC_MTFTOBMP_MAXEXT );
                aSizePix.Height() = GRAPHIC_MTFTOBMP_MAXEXT;
            }
            else
            {
                aSizePix.Width() = GRAPHIC_MTFTOBMP_MAXEXT;
                aSizePix.Height() = FRound( GRAPHIC_MTFTOBMP_MAXEXT / fWH );
            }
        }

        if( aVDev.SetOutputSizePixel( aSizePix ) )
        {
            const Point aPt;
            ImplDraw( &aVDev, aPt, aSizePix );
            aRetBmp =  aVDev.GetBitmap( aPt, aSizePix );
        }
    }

    if( !!aRetBmp )
    {
        aRetBmp.SetPrefMapMode( ImplGetPrefMapMode() );
        aRetBmp.SetPrefSize( ImplGetPrefSize() );
    }

    return aRetBmp;
}

// ------------------------------------------------------------------------

BitmapEx ImpGraphic::ImplGetBitmapEx() const
{
    BitmapEx aRetBmpEx;

    if( meType == GRAPHIC_BITMAP )
        aRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maEx );
    else if( ( meType != GRAPHIC_DEFAULT ) && ImplIsSupportedGraphic() )
    {
        const ImpGraphic aMonoMask( maMetaFile.GetMonochromeMtf( COL_BLACK ) );
        aRetBmpEx = BitmapEx( ImplGetBitmap(), aMonoMask.ImplGetBitmap() );
    }

    return aRetBmpEx;
}

// ------------------------------------------------------------------------

Animation ImpGraphic::ImplGetAnimation() const
{
    Animation aAnimation;

    if( mpAnimation )
        aAnimation = *mpAnimation;

    return aAnimation;
}

// ------------------------------------------------------------------------

const GDIMetaFile& ImpGraphic::ImplGetGDIMetaFile() const
{
    return maMetaFile;
}

// ------------------------------------------------------------------------

Size ImpGraphic::ImplGetPrefSize() const
{
    Size aSize;

    if( ImplIsSwapOut() )
        aSize = maSwapInfo.maPrefSize;
    else
    {
        switch( meType )
        {
            case( GRAPHIC_NONE ):
            case( GRAPHIC_DEFAULT ):
            break;

            case( GRAPHIC_BITMAP ):
            {
                aSize = maEx.GetPrefSize();

                if( !aSize.Width() || !aSize.Height() )
                    aSize = maEx.GetSizePixel();
            }
            break;

            default:
            {
                if( ImplIsSupportedGraphic() )
                  aSize = maMetaFile.GetPrefSize();
            }
            break;
        }
    }

    return aSize;
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplSetPrefSize( const Size& rPrefSize )
{
    switch( meType )
    {
        case( GRAPHIC_NONE ):
        case( GRAPHIC_DEFAULT ):
        break;

        case( GRAPHIC_BITMAP ):
            maEx.SetPrefSize( rPrefSize );
        break;

        default:
        {
            if( ImplIsSupportedGraphic() )
                maMetaFile.SetPrefSize( rPrefSize );
        }
        break;
    }
}

// ------------------------------------------------------------------------

MapMode ImpGraphic::ImplGetPrefMapMode() const
{
    MapMode aMapMode;

    if( ImplIsSwapOut() )
        aMapMode = maSwapInfo.maPrefMapMode;
    else
    {
        switch( meType )
        {
            case( GRAPHIC_NONE ):
            case( GRAPHIC_DEFAULT ):
            break;

            case( GRAPHIC_BITMAP ):
            {
                const Size aSize( maEx.GetPrefSize() );

                if ( aSize.Width() && aSize.Height() )
                    aMapMode = maEx.GetPrefMapMode();
            }
            break;

            default:
            {
                if( ImplIsSupportedGraphic() )
                    return maMetaFile.GetPrefMapMode();
            }
            break;
        }
    }

    return aMapMode;
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplSetPrefMapMode( const MapMode& rPrefMapMode )
{
    switch( meType )
    {
        case( GRAPHIC_NONE ):
        case( GRAPHIC_DEFAULT ):
        break;

        case( GRAPHIC_BITMAP ):
            maEx.SetPrefMapMode( rPrefMapMode );
        break;

        default:
        {
            if( ImplIsSupportedGraphic() )
                maMetaFile.SetPrefMapMode( rPrefMapMode );
        }
        break;
    }
}

// ------------------------------------------------------------------------

ULONG ImpGraphic::ImplGetSizeBytes() const
{
    ULONG nSizeBytes;

    if( meType == GRAPHIC_BITMAP )
    {
        if( mpAnimation )
            nSizeBytes = mpAnimation->GetSizeBytes();
        else
            nSizeBytes = maEx.GetSizeBytes();
    }
    else
        nSizeBytes = 0UL;

    return nSizeBytes;
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplDraw( OutputDevice* pOutDev, const Point& rDestPt ) const
{
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() )
    {
        switch( meType )
        {
            case( GRAPHIC_DEFAULT ):
            break;

            case( GRAPHIC_BITMAP ):
            {
                if ( mpAnimation )
                    mpAnimation->Draw( pOutDev, rDestPt );
                else
                    maEx.Draw( pOutDev, rDestPt );
            }
            break;

            default:
                ImplDraw( pOutDev, rDestPt, maMetaFile.GetPrefSize() );
            break;
        }
    }
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplDraw( OutputDevice* pOutDev,
                           const Point& rDestPt, const Size& rDestSize ) const
{
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() )
    {
        switch( meType )
        {
            case( GRAPHIC_DEFAULT ):
            break;

            case( GRAPHIC_BITMAP ):
            {
                if( mpAnimation )
                    mpAnimation->Draw( pOutDev, rDestPt, rDestSize );
                else
                    maEx.Draw( pOutDev, rDestPt, rDestSize );
            }
            break;

            default:
            {
                ( (ImpGraphic*) this )->maMetaFile.WindStart();
                ( (ImpGraphic*) this )->maMetaFile.Play( pOutDev, rDestPt, rDestSize );
                ( (ImpGraphic*) this )->maMetaFile.WindStart();
            }
            break;
        }
    }
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplStartAnimation( OutputDevice* pOutDev,
                                     const Point& rDestPt,
                                     long nExtraData,
                                     OutputDevice* pFirstFrameOutDev )
{
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() && mpAnimation )
        mpAnimation->Start( pOutDev, rDestPt, nExtraData, pFirstFrameOutDev );
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplStartAnimation( OutputDevice* pOutDev, const Point& rDestPt,
                                     const Size& rDestSize, long nExtraData,
                                     OutputDevice* pFirstFrameOutDev )
{
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() && mpAnimation )
        mpAnimation->Start( pOutDev, rDestPt, rDestSize, nExtraData, pFirstFrameOutDev );
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplStopAnimation( OutputDevice* pOutDev, long nExtraData )
{
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() && mpAnimation )
        mpAnimation->Stop( pOutDev, nExtraData );
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplSetAnimationNotifyHdl( const Link& rLink )
{
    if( mpAnimation )
        mpAnimation->SetNotifyHdl( rLink );
}

// ------------------------------------------------------------------------

Link ImpGraphic::ImplGetAnimationNotifyHdl() const
{
    Link aLink;

    if( mpAnimation )
        aLink = mpAnimation->GetNotifyHdl();

    return aLink;
}

// ------------------------------------------------------------------------

ULONG ImpGraphic::ImplGetAnimationLoopCount() const
{
    return( mpAnimation ? mpAnimation->GetLoopCount() : 0UL );
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplResetAnimationLoopCount()
{
    if( mpAnimation )
        mpAnimation->ResetLoopCount();
}

// ------------------------------------------------------------------------

List* ImpGraphic::ImplGetAnimationInfoList() const
{
    return( mpAnimation ? mpAnimation->GetAInfoList() : NULL );
}

// ------------------------------------------------------------------------

GraphicReader* ImpGraphic::ImplGetContext()
{
    return mpContext;
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplSetContext( GraphicReader* pReader )
{
    mpContext = pReader;
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplSetDocFileName( const String& rName, ULONG nFilePos )
{
    maDocFileName = rName;
    mnDocFilePos = nFilePos;
}

// ------------------------------------------------------------------------

const String& ImpGraphic::ImplGetDocFileName() const
{
    return maDocFileName;
}

// ------------------------------------------------------------------------

ULONG ImpGraphic::ImplGetDocFilePos() const
{
    return mnDocFilePos;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplReadEmbedded( SvStream& rIStm, BOOL bSwap )
{
    MapMode         aMapMode;
    Size            aSize;
    const ULONG     nStartPos = rIStm.Tell();
    ULONG           nId;
    ULONG           nHeaderLen;
    long            nType;
    long            nLen;
    const USHORT    nOldFormat = rIStm.GetNumberFormatInt();
    BOOL            bRet = FALSE;

    if( !mbSwapUnderway )
    {
        const String aTempName( maDocFileName );
        const ULONG  nTempPos = mnDocFilePos;

        ImplClear();

        maDocFileName = aTempName;
        mnDocFilePos = nTempPos;
    }

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rIStm >> nId;

    // check version
    if( GRAPHIC_FORMAT_50 == nId )
    {
        // read new style header
        VersionCompat* pCompat = new VersionCompat( rIStm, STREAM_READ );

        rIStm >> nType;
        rIStm >> nLen;
        rIStm >> aSize;
        rIStm >> aMapMode;

        delete pCompat;
    }
    else
    {
        // read old style header
        long nWidth, nHeight;
        long nMapMode, nScaleNumX, nScaleDenomX;
        long nScaleNumY, nScaleDenomY, nOffsX, nOffsY;

        rIStm.SeekRel( -4L );

        rIStm >> nType >> nLen >> nWidth >> nHeight;
        rIStm >> nMapMode >> nScaleNumX >> nScaleDenomX >> nScaleNumY;
        rIStm >> nScaleDenomY >> nOffsX >> nOffsY;

        // swapped
        if( nType > 100L )
        {
            nType = SWAPLONG( nType );
            nLen = SWAPLONG( nLen );
            nWidth = SWAPLONG( nWidth );
            nHeight = SWAPLONG( nHeight );
            nMapMode = SWAPLONG( nMapMode );
            nScaleNumX = SWAPLONG( nScaleNumX );
            nScaleDenomX = SWAPLONG( nScaleDenomX );
            nScaleNumY = SWAPLONG( nScaleNumY );
            nScaleDenomY = SWAPLONG( nScaleDenomY );
            nOffsX = SWAPLONG( nOffsX );
            nOffsY = SWAPLONG( nOffsY );
        }

        aSize = Size( nWidth, nHeight );
        aMapMode = MapMode( (MapUnit) nMapMode, Point( nOffsX, nOffsY ),
                            Fraction( nScaleNumX, nScaleDenomX ),
                            Fraction( nScaleNumY, nScaleDenomY ) );
    }

    nHeaderLen = rIStm.Tell() - nStartPos;
    meType = (GraphicType) nType;

    if( meType )
    {
        if( meType == GRAPHIC_BITMAP )
        {
            maEx.aBitmapSize = aSize;

            if( aMapMode != MapMode() )
            {
                maEx.SetPrefMapMode( aMapMode );
                maEx.SetPrefSize( aSize );
            }
        }
        else
        {
            maMetaFile.SetPrefMapMode( aMapMode );
            maMetaFile.SetPrefSize( aSize );
        }

        if( bSwap )
        {
            if( maDocFileName.Len() )
            {
                rIStm.Seek( nStartPos + nHeaderLen + nLen );
                bRet = mbSwapOut = TRUE;
            }
            else
            {
                const String aTmpName( TempFile::CreateTempName() );

                if( aTmpName.Len() )
                {
                    SvFileStream aOStm( aTmpName, STREAM_WRITE | STREAM_SHARE_DENYWRITE );

                      aOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

                    if( !aOStm.GetError() )
                    {
                        ULONG   nFullLen = nHeaderLen + nLen;
                        ULONG   nPartLen = Min( nFullLen, (ULONG) GRAPHIC_MAXPARTLEN );
                        BYTE*   pBuffer = (BYTE*) SvMemAlloc( nPartLen );

                        if( pBuffer )
                        {
                            rIStm.Seek( nStartPos );

                            while( nFullLen )
                            {
                                rIStm.Read( (char*) pBuffer, nPartLen );
                                aOStm.Write( (char*) pBuffer, nPartLen );

                                nFullLen -= nPartLen;

                                if( nFullLen < GRAPHIC_MAXPARTLEN )
                                    nPartLen = nFullLen;
                            }

                            SvMemFree( pBuffer );

                            ULONG nReadErr = rIStm.GetError();
                            ULONG nWriteErr = aOStm.GetError();

                            aOStm.Close();

                            if( !nReadErr && !nWriteErr )
                            {
                                bRet = mbSwapOut = TRUE;
                                mpSwapFile = new ImpSwapFile;
                                mpSwapFile->nRefCount = 1;
                                mpSwapFile->aSwapFileName = aTmpName;
                            }
                            else
                            {
                                try
                                {
                                    ::ucb::Content aCnt( INetURLObject( aTmpName, INET_PROT_FILE ).GetMainURL(),
                                                         ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

                                    aCnt.executeCommand( ::rtl::OUString::createFromAscii( "delete" ),
                                                         ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
                                }
                                catch( ::com::sun::star::ucb::CommandAbortedException& )
                                {
                                    DBG_ERRORFILE( "CommandAbortedException" );
                                }
                                catch( ... )
                                {
                                    DBG_ERRORFILE( "Any other exception" );
                                }
                            }
                        }
                    }
                }
            }
        }
        else if( meType == GRAPHIC_BITMAP || meType == GRAPHIC_GDIMETAFILE )
        {
            rIStm >> *this;
            bRet = ( rIStm.GetError() == 0UL );
        }
        else if( meType >= SYS_WINMETAFILE && meType <= SYS_MACMETAFILE )
        {
            Graphic aSysGraphic;
            ULONG   nCvtType;

            switch( (ULONG) meType )
            {
                case( SYS_WINMETAFILE ):
                case( SYS_WNTMETAFILE ): nCvtType = CVT_WMF; break;
                case( SYS_OS2METAFILE ): nCvtType = CVT_MET; break;
                case( SYS_MACMETAFILE ): nCvtType = CVT_PCT; break;

                default:
                    nCvtType = CVT_UNKNOWN;
                break;
            }

            if( nType && GraphicConverter::Import( rIStm, aSysGraphic, nCvtType ) == ERRCODE_NONE )
            {
                *this = ImpGraphic( aSysGraphic.GetGDIMetaFile() );
                bRet = ( rIStm.GetError() == 0UL );
            }
            else
                meType = GRAPHIC_DEFAULT;
        }

        if( bRet )
        {
            ImplSetPrefMapMode( aMapMode );
            ImplSetPrefSize( aSize );
        }
    }
    else
        bRet = TRUE;

    rIStm.SetNumberFormatInt( nOldFormat );

    return bRet;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplWriteEmbedded( SvStream& rOStm )
{
    BOOL bRet = FALSE;

    if( ( meType != GRAPHIC_NONE ) && ( meType != GRAPHIC_DEFAULT ) && !ImplIsSwapOut() )
    {
        const MapMode   aMapMode( ImplGetPrefMapMode() );
        const Size      aSize( ImplGetPrefSize() );
        const USHORT    nOldFormat = rOStm.GetNumberFormatInt();
        const ULONG     nStmPos1 = rOStm.Tell();
        ULONG           nDataFieldPos;

        rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

        // write correct version ( old style/new style header )
        if( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_50 )
        {
            // write ID for new format (5.0)
            rOStm << GRAPHIC_FORMAT_50;

            // write new style header
            VersionCompat* pCompat = new VersionCompat( rOStm, STREAM_WRITE, 1 );

            rOStm << (long) meType;

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm << (long) 0;

            rOStm << aSize;
            rOStm << aMapMode;

            delete pCompat;
        }
        else
        {
            // write old style (<=4.0) header
            rOStm << (long) meType;

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm << (long) 0;

            rOStm << (long) aSize.Width();
            rOStm << (long) aSize.Height();
            rOStm << (long) aMapMode.GetMapUnit();
            rOStm << (long) aMapMode.GetScaleX().GetNumerator();
            rOStm << (long) aMapMode.GetScaleX().GetDenominator();
            rOStm << (long) aMapMode.GetScaleY().GetNumerator();
            rOStm << (long) aMapMode.GetScaleY().GetDenominator();
            rOStm << (long) aMapMode.GetOrigin().X();
            rOStm << (long) aMapMode.GetOrigin().Y();
        }

        // write data block
        if( !rOStm.GetError() )
        {
            const ULONG nDataStart = rOStm.Tell();

            if( ImplIsSupportedGraphic() )
                rOStm << *this;

            if( !rOStm.GetError() )
            {
                const ULONG nStmPos2 = rOStm.Tell();
                rOStm.Seek( nDataFieldPos );
                rOStm << (long) ( nStmPos2 - nDataStart );
                rOStm.Seek( nStmPos2 );
                bRet = TRUE;
            }
        }

        rOStm.SetNumberFormatInt( nOldFormat );
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplSwapOut()
{
    BOOL bRet = FALSE;

    if( !ImplIsSwapOut() )
    {
        if( !maDocFileName.Len() )
        {
            const String aTmpName( TempFile::CreateTempName() );

            if( aTmpName.Len() )
            {
                SvFileStream aOStm( aTmpName, STREAM_WRITE | STREAM_SHARE_DENYWRITE );

                aOStm.SetVersion( SOFFICE_FILEFORMAT_NOW );
                aOStm.SetCompressMode( COMPRESSMODE_NATIVE );

                if( ( bRet = ImplSwapOut( &aOStm ) ) == TRUE )
                {
                    mpSwapFile = new ImpSwapFile;
                    mpSwapFile->nRefCount = 1;
                    mpSwapFile->aSwapFileName = aTmpName;
                }
                else
                {
                    try
                    {
                        ::ucb::Content aCnt( INetURLObject( aTmpName, INET_PROT_FILE ).GetMainURL(),
                                             ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

                        aCnt.executeCommand( ::rtl::OUString::createFromAscii( "delete" ),
                                             ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
                    }
                    catch( ::com::sun::star::ucb::CommandAbortedException& )
                    {
                        DBG_ERRORFILE( "CommandAbortedException" );
                    }
                    catch( ... )
                    {
                        DBG_ERRORFILE( "Any other exception" );
                    }
                }
            }
        }
        else
        {
            ImplClearGraphics( TRUE );
            bRet = mbSwapOut = TRUE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplSwapOut( SvStream* pOStm )
{
    BOOL bRet = FALSE;

    if( pOStm )
    {
        pOStm->SetBufferSize( GRAPHIC_STREAMBUFSIZE );

        if( !pOStm->GetError() && ImplWriteEmbedded( *pOStm ) )
        {
            pOStm->Flush();

            if( !pOStm->GetError() )
            {
                ImplClearGraphics( TRUE );
                bRet = mbSwapOut = TRUE;
            }
        }
    }
    else
    {
        ImplClearGraphics( TRUE );
        bRet = mbSwapOut = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplSwapIn()
{
    BOOL bRet = FALSE;

    if( ImplIsSwapOut() )
    {
        const String    aFileName = ( mpSwapFile ? mpSwapFile->aSwapFileName : maDocFileName );
        SvFileStream    aIStm( aFileName, STREAM_READ | STREAM_SHARE_DENYWRITE );

        aIStm.SetVersion( SOFFICE_FILEFORMAT_NOW );
        aIStm.SetCompressMode( COMPRESSMODE_NATIVE );

        if( !mpSwapFile )
            aIStm.Seek( mnDocFilePos );

        bRet = ImplSwapIn( &aIStm );
        aIStm.Close();

        if( mpSwapFile )
        {
            if( mpSwapFile->nRefCount > 1 )
                mpSwapFile->nRefCount--;
            else
            {
                try
                {
                    ::ucb::Content aCnt( INetURLObject( mpSwapFile->aSwapFileName, INET_PROT_FILE ).GetMainURL(),
                                         ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

                    aCnt.executeCommand( ::rtl::OUString::createFromAscii( "delete" ),
                                         ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
                }
                catch( ::com::sun::star::ucb::CommandAbortedException& )
                {
                    DBG_ERRORFILE( "CommandAbortedException" );
                }
                catch( ... )
                {
                    DBG_ERRORFILE( "Any other exception" );
                }

                delete mpSwapFile;
            }

            mpSwapFile = NULL;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplSwapIn( SvStream* pIStm )
{
    BOOL bRet = FALSE;

    if( pIStm )
    {
        pIStm->SetBufferSize( GRAPHIC_STREAMBUFSIZE );

        if( !pIStm->GetError() )
        {
            mbSwapUnderway = TRUE;
            bRet = ImplReadEmbedded( *pIStm );
            mbSwapUnderway = FALSE;

            if( !bRet )
                ImplClear();
            else
                mbSwapOut = FALSE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplIsSwapOut() const
{
    return mbSwapOut;
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplSetLink( const GfxLink& rGfxLink )
{
    delete mpGfxLink;
    mpGfxLink = new GfxLink( rGfxLink );

    if( mpGfxLink->IsNative() )
        mpGfxLink->SwapOut();
}

// ------------------------------------------------------------------------

GfxLink ImpGraphic::ImplGetLink()
{
    return( mpGfxLink ? *mpGfxLink : GfxLink() );
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplIsLink() const
{
    return ( mpGfxLink != NULL ) ? TRUE : FALSE;
}

// ------------------------------------------------------------------------

ULONG ImpGraphic::ImplGetChecksum() const
{
    ULONG nRet = 0;

    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() )
    {
        switch( meType )
        {
            case( GRAPHIC_DEFAULT ):
            break;

            case( GRAPHIC_BITMAP ):
            {
                if( mpAnimation )
                    nRet = mpAnimation->GetChecksum();
                else
                    nRet = maEx.GetChecksum();
            }
            break;

            default:
                nRet = maMetaFile.GetChecksum();
            break;
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplCopy() const
{
    DBG_ERROR( "Missing implementation!" );
    return FALSE;
}

// ------------------------------------------------------------------------

BOOL ImpGraphic::ImplPaste()
{
    DBG_ERROR( "Missing implementation!" );
    return FALSE;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, ImpGraphic& rImpGraphic )
{
    if( !rIStm.GetError() )
    {
        const ULONG nStmPos1 = rIStm.Tell();
        ULONG       nTmp;

        if ( !rImpGraphic.mbSwapUnderway )
            rImpGraphic.ImplClear();

        // read Id
        rIStm >> nTmp;

        if( NATIVE_FORMAT_50 == nTmp )
        {
            Graphic         aGraphic;
            GfxLink         aLink;
            VersionCompat*  pCompat;

            // read compat info
            pCompat = new VersionCompat( rIStm, STREAM_READ );
            delete pCompat;

            rIStm >> aLink;

            // set dummy link to avoid creation of additional link after filtering;
            // we set a default link to avoid unnecessary swapping of native data
            aGraphic.SetLink( GfxLink() );

            if( !rIStm.GetError() && aLink.LoadNative( aGraphic ) )
            {
                // set link only, if no other link was set
                const BOOL bSetLink = ( rImpGraphic.mpGfxLink == NULL );

                // assign graphic
                rImpGraphic = *aGraphic.ImplGetImpGraphic();

                if( bSetLink )
                    rImpGraphic.ImplSetLink( aLink );
            }
            else
            {
                rIStm.Seek( nStmPos1 );
                rIStm.SetError( ERRCODE_IO_WRONGFORMAT );
            }
        }
        else
        {
            BitmapEx        aBmpEx;
            const USHORT    nOldFormat = rIStm.GetNumberFormatInt();

            rIStm.SeekRel( -4 );
            rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
            rIStm >> aBmpEx;

            if( !rIStm.GetError() )
            {
                UINT32  nMagic1, nMagic2;
                ULONG   nActPos = rIStm.Tell();

                rIStm >> nMagic1 >> nMagic2;
                rIStm.Seek( nActPos );

                rImpGraphic = ImpGraphic( aBmpEx );

                if( ( 0x5344414e == nMagic1 ) && ( 0x494d4931 == nMagic2 ) && !rIStm.GetError() )
                {
                    delete rImpGraphic.mpAnimation;
                    rImpGraphic.mpAnimation = new Animation;
                    rIStm >> *rImpGraphic.mpAnimation;
                }
            }
            else
            {
                GDIMetaFile aMtf;

                rIStm.Seek( nStmPos1 );
                rIStm.ResetError();
                rIStm >> aMtf;

                if( !rIStm.GetError() )
                    rImpGraphic = aMtf;
                else
                    rIStm.Seek( nStmPos1 );
            }

            rIStm.SetNumberFormatInt( nOldFormat );
        }
    }

    return rIStm;
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const ImpGraphic& rImpGraphic )
{
    if( !rOStm.GetError() )
    {
        if( !rImpGraphic.ImplIsSwapOut() )
        {
            if( ( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_50 ) &&
                ( rOStm.GetCompressMode() & COMPRESSMODE_NATIVE ) &&
                rImpGraphic.mpGfxLink && rImpGraphic.mpGfxLink->IsNative() )
            {
                VersionCompat* pCompat;

                // native format
                rOStm << NATIVE_FORMAT_50;

                // write compat info
                pCompat = new VersionCompat( rOStm, STREAM_WRITE, 1 );
                delete pCompat;

                rOStm << *rImpGraphic.mpGfxLink;
            }
            else
            {
                // own format
                const USHORT nOldFormat = rOStm.GetNumberFormatInt();
                rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

                switch( rImpGraphic.ImplGetType() )
                {
                    case( GRAPHIC_NONE ):
                    case( GRAPHIC_DEFAULT ):
                    break;

                    case GRAPHIC_BITMAP:
                    {
                        if ( rImpGraphic.ImplIsAnimated() )
                            rOStm << *rImpGraphic.mpAnimation;
                        else
                            rOStm << rImpGraphic.maEx;
                    }
                    break;

                    default:
                    {
                        if( rImpGraphic.ImplIsSupportedGraphic() )
                            rOStm << rImpGraphic.maMetaFile;
                    }
                    break;
                }

                rOStm.SetNumberFormatInt( nOldFormat );
            }
        }
        else
             rOStm.SetError( SVSTREAM_GENERALERROR );
    }

    return rOStm;
}
