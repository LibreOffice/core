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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <tools/vcompat.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>

#include <ucbhelper/content.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graph.hxx>
#include <vcl/metaact.hxx>

#include <impgraph.hxx>

#include <com/sun/star/ucb/CommandAbortedException.hpp>

// -----------
// - Defines -
// -----------

#define GRAPHIC_MAXPARTLEN          256000L
#define GRAPHIC_MTFTOBMP_MAXEXT     2048
#define GRAPHIC_STREAMBUFSIZE       8192UL

#define SYS_WINMETAFILE             0x00000003L
#define SYS_WNTMETAFILE             0x00000004L
#define SYS_OS2METAFILE             0x00000005L
#define SYS_MACMETAFILE             0x00000006L

#define GRAPHIC_FORMAT_50           static_cast<sal_uInt32>(COMPAT_FORMAT( 'G', 'R', 'F', '5' ))
#define NATIVE_FORMAT_50            static_cast<sal_uInt32>(COMPAT_FORMAT( 'N', 'A', 'T', '5' ))

// ---------------
// - ImpSwapFile -
// ---------------

struct ImpSwapFile
{
    INetURLObject   aSwapURL;
    sal_uLong           nRefCount;
};

// -----------------
// - Graphicreader -
// -----------------

class ReaderData
{
public:
    Size    maPreviewSize;
};

GraphicReader::~GraphicReader()
{
    delete mpReaderData;
}

// ------------------------------------------------------------------------

void GraphicReader::DisablePreviewMode()
{
    if( mpReaderData )
        mpReaderData->maPreviewSize = Size( 0, 0 );
}

// ------------------------------------------------------------------------

void GraphicReader::SetPreviewSize( const Size& rSize )
{
    if( !mpReaderData )
        mpReaderData = new ReaderData;
    mpReaderData->maPreviewSize = rSize;
}

// ------------------------------------------------------------------------

Size GraphicReader::GetPreviewSize() const
{
    Size aSize( 0, 0 );
    if( mpReaderData )
        aSize = mpReaderData->maPreviewSize;
    return aSize;
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
        mnSizeBytes     ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( sal_False ),
        mbSwapUnderway  ( sal_False )
{
}

// ------------------------------------------------------------------------

ImpGraphic::ImpGraphic( const ImpGraphic& rImpGraphic ) :
        maMetaFile      ( rImpGraphic.maMetaFile ),
        maEx            ( rImpGraphic.maEx ),
        mpContext       ( NULL ),
        mpSwapFile      ( rImpGraphic.mpSwapFile ),
        meType          ( rImpGraphic.meType ),
        maDocFileURLStr ( rImpGraphic.maDocFileURLStr ),
        mnDocFilePos    ( rImpGraphic.mnDocFilePos ),
        mnSizeBytes     ( rImpGraphic.mnSizeBytes ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( rImpGraphic.mbSwapOut ),
        mbSwapUnderway  ( sal_False )
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
        mnSizeBytes     ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( sal_False ),
        mbSwapUnderway  ( sal_False )
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
        mnSizeBytes     ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( sal_False ),
        mbSwapUnderway  ( sal_False )
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
        mnSizeBytes     ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( sal_False ),
        mbSwapUnderway  ( sal_False )
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
        mnSizeBytes     ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( sal_False ),
        mbSwapUnderway  ( sal_False )
{
}

// ------------------------------------------------------------------------

ImpGraphic::~ImpGraphic()
{
    ImplClear();

    if( (sal_uLong) mpContext > 1UL )
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
        mnSizeBytes = rImpGraphic.mnSizeBytes;

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
            maDocFileURLStr = rImpGraphic.maDocFileURLStr;
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

sal_Bool ImpGraphic::operator==( const ImpGraphic& rImpGraphic ) const
{
    sal_Bool bRet = sal_False;

    if( this == &rImpGraphic )
        bRet = sal_True;
    else if( !ImplIsSwapOut() && ( rImpGraphic.meType == meType ) )
    {
        switch( meType )
        {
            case( GRAPHIC_NONE ):
                bRet = sal_True;
            break;

            case( GRAPHIC_GDIMETAFILE ):
            {
                if( rImpGraphic.maMetaFile == maMetaFile )
                    bRet = sal_True;
            }
            break;

            case( GRAPHIC_BITMAP ):
            {
                if( mpAnimation )
                {
                    if( rImpGraphic.mpAnimation && ( *rImpGraphic.mpAnimation == *mpAnimation ) )
                        bRet = sal_True;
                }
                else if( !rImpGraphic.mpAnimation && ( rImpGraphic.maEx == maEx ) )
                    bRet = sal_True;
            }
            break;

            default:
            break;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

void ImpGraphic::ImplClearGraphics( sal_Bool bCreateSwapInfo )
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
                ::ucbhelper::Content aCnt( mpSwapFile->aSwapURL.GetMainURL( INetURLObject::NO_DECODE ),
                                     ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                                     comphelper::getProcessComponentContext() );

                aCnt.executeCommand( ::rtl::OUString("delete"),
                                     ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
            }
            catch( const ::com::sun::star::ucb::ContentCreationException& )
            {
            }
            catch( const ::com::sun::star::uno::RuntimeException& )
            {
            }
            catch( const ::com::sun::star::ucb::CommandAbortedException& )
            {
            }
            catch( const ::com::sun::star::uno::Exception& )
            {
            }

            delete mpSwapFile;
        }

        mpSwapFile = NULL;
    }

    mbSwapOut = sal_False;
    mnDocFilePos = 0UL;
    maDocFileURLStr.Erase();

    // cleanup
    ImplClearGraphics( sal_False );
    meType = GRAPHIC_NONE;
    mnSizeBytes = 0;
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

sal_Bool ImpGraphic::ImplIsSupportedGraphic() const
{
    return( meType != GRAPHIC_NONE );
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplIsTransparent() const
{
    sal_Bool bRet;

    if( meType == GRAPHIC_BITMAP )
        bRet = ( mpAnimation ? mpAnimation->IsTransparent() : maEx.IsTransparent() );
    else
        bRet = sal_True;

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplIsAlpha() const
{
    sal_Bool bRet;

    if( meType == GRAPHIC_BITMAP )
        bRet = ( NULL == mpAnimation ) && maEx.IsAlpha();
    else
        bRet = sal_False;

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplIsAnimated() const
{
    return( mpAnimation != NULL );
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplIsEPS() const
{
    return( ( meType == GRAPHIC_GDIMETAFILE ) &&
            ( maMetaFile.GetActionSize() > 0 ) &&
            ( maMetaFile.GetAction( 0 )->GetType() == META_EPS_ACTION ) );
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplIsRenderGraphic() const
{
    return( ( GRAPHIC_GDIMETAFILE == meType ) &&
            ( 1 == maMetaFile.GetActionSize() ) &&
            ( META_RENDERGRAPHIC_ACTION == maMetaFile.GetAction( 0 )->GetType() ) );
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplHasRenderGraphic() const
{
    sal_Bool bRet = sal_False;

    if( GRAPHIC_GDIMETAFILE == meType )
    {
        GDIMetaFile& rMtf = const_cast< ImpGraphic* >( this )->maMetaFile;

        for( MetaAction* pAct = rMtf.FirstAction(); pAct && !bRet; pAct = rMtf.NextAction() )
        {
            if( META_RENDERGRAPHIC_ACTION == pAct->GetType() )
            {
                bRet = sal_True;
            }
        }

        rMtf.WindStart();
    }

    return( bRet );
}

// ------------------------------------------------------------------------

Bitmap ImpGraphic::ImplGetBitmap(const GraphicConversionParameters& rParameters) const
{
    Bitmap aRetBmp;

    if( meType == GRAPHIC_BITMAP )
    {
        const BitmapEx& rRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maEx );
        const Color     aReplaceColor( COL_WHITE );

        aRetBmp = rRetBmpEx.GetBitmap( &aReplaceColor );

        if(rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
            aRetBmp.Scale(rParameters.getSizePixel());
    }
    else if( ( meType != GRAPHIC_DEFAULT ) && ImplIsSupportedGraphic() )
    {
        // use corner points of graphic to determine the pixel
        // extent of the graphic (rounding errors are possible else)
        VirtualDevice   aVDev;
        const Point     aNullPt;
        const Point     aTLPix( aVDev.LogicToPixel( aNullPt, maMetaFile.GetPrefMapMode() ) );
        const Point     aBRPix( aVDev.LogicToPixel( Point( maMetaFile.GetPrefSize().Width() - 1, maMetaFile.GetPrefSize().Height() - 1 ), maMetaFile.GetPrefMapMode() ) );
        Size            aDrawSize( aVDev.LogicToPixel( maMetaFile.GetPrefSize(), maMetaFile.GetPrefMapMode() ) );
        Size            aSizePix( labs( aBRPix.X() - aTLPix.X() ) + 1, labs( aBRPix.Y() - aTLPix.Y() ) + 1 );

        if(rParameters.getSizePixel().Width() && rParameters.getSizePixel().Height())
        {
            aDrawSize.Width() = FRound((double)rParameters.getSizePixel().Width() *
                (double)aDrawSize.Width() / (double)aSizePix.Width());
            aDrawSize.Height() = FRound((double)rParameters.getSizePixel().Height() *
                (double)aDrawSize.Height() / (double)aSizePix.Height());

            aSizePix = rParameters.getSizePixel();
        }

        if( aSizePix.Width() && aSizePix.Height() && !rParameters.getUnlimitedSize()
            && (aSizePix.Width() > GRAPHIC_MTFTOBMP_MAXEXT || aSizePix.Height() > GRAPHIC_MTFTOBMP_MAXEXT))
        {
            const Size  aOldSizePix( aSizePix );
            double      fWH = (double) aSizePix.Width() / aSizePix.Height();

            if( fWH <= 1.0 )
                aSizePix.Width() = FRound( GRAPHIC_MTFTOBMP_MAXEXT * fWH ), aSizePix.Height() = GRAPHIC_MTFTOBMP_MAXEXT;
            else
                aSizePix.Width() = GRAPHIC_MTFTOBMP_MAXEXT, aSizePix.Height() = FRound(  GRAPHIC_MTFTOBMP_MAXEXT / fWH );

            aDrawSize.Width() = FRound( ( (double) aDrawSize.Width() * aSizePix.Width() ) / aOldSizePix.Width() );
            aDrawSize.Height() = FRound( ( (double) aDrawSize.Height() * aSizePix.Height() ) / aOldSizePix.Height() );
        }

        if( aVDev.SetOutputSizePixel( aSizePix ) )
        {
            if(rParameters.getAntiAliase())
            {
                aVDev.SetAntialiasing(aVDev.GetAntialiasing() | ANTIALIASING_ENABLE_B2DDRAW);
            }

            if(rParameters.getSnapHorVerLines())
            {
                aVDev.SetAntialiasing(aVDev.GetAntialiasing() | ANTIALIASING_PIXELSNAPHAIRLINE);
            }

            ImplDraw( &aVDev, aNullPt, aDrawSize );
            aRetBmp =  aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() );
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

BitmapEx ImpGraphic::ImplGetBitmapEx(const GraphicConversionParameters& rParameters) const
{
    BitmapEx aRetBmpEx;

    if( meType == GRAPHIC_BITMAP )
    {
        aRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maEx );

        if(rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
            aRetBmpEx.Scale(rParameters.getSizePixel());
    }
    else if( ( meType != GRAPHIC_DEFAULT ) && ImplIsSupportedGraphic() )
    {
        const ImpGraphic aMonoMask( maMetaFile.GetMonochromeMtf( COL_BLACK ) );
        aRetBmpEx = BitmapEx(ImplGetBitmap(rParameters), aMonoMask.ImplGetBitmap(rParameters));
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

::vcl::RenderGraphic ImpGraphic::ImplGetRenderGraphic() const
{
    ::vcl::RenderGraphic aRet;

    if( ImplIsRenderGraphic() )
        aRet = static_cast< MetaRenderGraphicAction* >( maMetaFile.GetAction( 0 ) )->GetRenderGraphic();

    return( aRet );
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
            // #108077# Push through pref size to animation object,
            // will be lost on copy otherwise
            if( ImplIsAnimated() )
                const_cast< BitmapEx& >(mpAnimation->GetBitmapEx()).SetPrefSize( rPrefSize );

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
            // #108077# Push through pref mapmode to animation object,
            // will be lost on copy otherwise
            if( ImplIsAnimated() )
                const_cast< BitmapEx& >(mpAnimation->GetBitmapEx()).SetPrefMapMode( rPrefMapMode );

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

sal_uLong ImpGraphic::ImplGetSizeBytes() const
{
    if( 0 == mnSizeBytes )
    {
        if( meType == GRAPHIC_BITMAP )
        {
            mnSizeBytes = mpAnimation ? mpAnimation->GetSizeBytes() : maEx.GetSizeBytes();
        }
        else if( meType == GRAPHIC_GDIMETAFILE )
        {
            mnSizeBytes = maMetaFile.GetSizeBytes();
        }
    }

    return( mnSizeBytes );
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

sal_uLong ImpGraphic::ImplGetAnimationLoopCount() const
{
    return( mpAnimation ? mpAnimation->GetLoopCount() : 0UL );
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

void ImpGraphic::ImplSetDocFileName( const String& rName, sal_uLong nFilePos )
{
    const INetURLObject aURL( rName );

    DBG_ASSERT( !rName.Len() || ( aURL.GetProtocol() != INET_PROT_NOT_VALID ), "Graphic::SetDocFileName(...): invalid URL" );

    maDocFileURLStr = aURL.GetMainURL( INetURLObject::NO_DECODE );
    mnDocFilePos = nFilePos;
}

// ------------------------------------------------------------------------

const String& ImpGraphic::ImplGetDocFileName() const
{
    return maDocFileURLStr;
}

// ------------------------------------------------------------------------

sal_uLong ImpGraphic::ImplGetDocFilePos() const
{
    return mnDocFilePos;
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplReadEmbedded( SvStream& rIStm, sal_Bool bSwap )
{
    MapMode         aMapMode;
    Size            aSize;
    const sal_uLong     nStartPos = rIStm.Tell();
    sal_uInt32      nId;
    sal_uLong           nHeaderLen;
    //#fdo39428 SvStream no longer supports operator>>(long&)
    sal_Int32       nType;
    sal_Int32       nLen;
    const sal_uInt16    nOldFormat = rIStm.GetNumberFormatInt();
    sal_Bool            bRet = sal_False;

    if( !mbSwapUnderway )
    {
        const String        aTempURLStr( maDocFileURLStr );
        const sal_uLong         nTempPos = mnDocFilePos;

        ImplClear();

        maDocFileURLStr = aTempURLStr;
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
        //#fdo39428 SvStream no longer supports operator>>(long&)
        sal_Int32 nWidth, nHeight;
        sal_Int32 nMapMode, nScaleNumX, nScaleDenomX;
        sal_Int32 nScaleNumY, nScaleDenomY, nOffsX, nOffsY;

        rIStm.SeekRel( -4L );

        rIStm >> nType >> nLen >> nWidth >> nHeight;
        rIStm >> nMapMode >> nScaleNumX >> nScaleDenomX >> nScaleNumY;
        rIStm >> nScaleDenomY >> nOffsX >> nOffsY;

        // swapped
        if( nType > 100L )
        {
            nType = OSL_SWAPDWORD( nType );
            nLen = OSL_SWAPDWORD( nLen );
            nWidth = OSL_SWAPDWORD( nWidth );
            nHeight = OSL_SWAPDWORD( nHeight );
            nMapMode = OSL_SWAPDWORD( nMapMode );
            nScaleNumX = OSL_SWAPDWORD( nScaleNumX );
            nScaleDenomX = OSL_SWAPDWORD( nScaleDenomX );
            nScaleNumY = OSL_SWAPDWORD( nScaleNumY );
            nScaleDenomY = OSL_SWAPDWORD( nScaleDenomY );
            nOffsX = OSL_SWAPDWORD( nOffsX );
            nOffsY = OSL_SWAPDWORD( nOffsY );
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
            if( maDocFileURLStr.Len() )
            {
                rIStm.Seek( nStartPos + nHeaderLen + nLen );
                bRet = mbSwapOut = sal_True;
            }
            else
            {
                ::utl::TempFile     aTempFile;
                const INetURLObject aTmpURL( aTempFile.GetURL() );

                if( !aTmpURL.GetMainURL( INetURLObject::NO_DECODE ).isEmpty() )
                {
                    SvStream* pOStm = NULL;
                    try
                    {
                        pOStm = ::utl::UcbStreamHelper::CreateStream( aTmpURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READWRITE | STREAM_SHARE_DENYWRITE );
                    }
                    catch( const ::com::sun::star::uno::Exception& )
                    {
                    }

                    if( pOStm )
                    {
                        sal_uLong   nFullLen = nHeaderLen + nLen;
                        sal_uLong   nPartLen = Min( nFullLen, (sal_uLong) GRAPHIC_MAXPARTLEN );
                        sal_uInt8*  pBuffer = (sal_uInt8*) rtl_allocateMemory( nPartLen );

                          pOStm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

                        if( pBuffer )
                        {
                            rIStm.Seek( nStartPos );

                            while( nFullLen )
                            {
                                rIStm.Read( (char*) pBuffer, nPartLen );
                                pOStm->Write( (char*) pBuffer, nPartLen );

                                nFullLen -= nPartLen;

                                if( nFullLen < GRAPHIC_MAXPARTLEN )
                                    nPartLen = nFullLen;
                            }

                            rtl_freeMemory( pBuffer );
                            sal_uLong nReadErr = rIStm.GetError(), nWriteErr = pOStm->GetError();
                            delete pOStm, pOStm = NULL;

                            if( !nReadErr && !nWriteErr )
                            {
                                bRet = mbSwapOut = sal_True;
                                mpSwapFile = new ImpSwapFile;
                                mpSwapFile->nRefCount = 1;
                                mpSwapFile->aSwapURL = aTmpURL;
                            }
                            else
                            {
                                try
                                {
                                    ::ucbhelper::Content aCnt( aTmpURL.GetMainURL( INetURLObject::NO_DECODE ),
                                                         ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                                                         comphelper::getProcessComponentContext() );

                                    aCnt.executeCommand( ::rtl::OUString("delete"),
                                                         ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
                                }
                                catch( const ::com::sun::star::ucb::ContentCreationException& )
                                {
                                }
                                catch( const ::com::sun::star::uno::RuntimeException& )
                                {
                                }
                                catch( const ::com::sun::star::ucb::CommandAbortedException& )
                                {
                                }
                                catch( const ::com::sun::star::uno::Exception& )
                                {
                                }
                            }
                        }

                        delete pOStm;
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
            sal_uLong   nCvtType;

            switch( sal::static_int_cast<sal_uLong>(meType) )
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
        bRet = sal_True;

    rIStm.SetNumberFormatInt( nOldFormat );

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplWriteEmbedded( SvStream& rOStm )
{
    sal_Bool bRet = sal_False;

    if( ( meType != GRAPHIC_NONE ) && ( meType != GRAPHIC_DEFAULT ) && !ImplIsSwapOut() )
    {
        const MapMode   aMapMode( ImplGetPrefMapMode() );
        const Size      aSize( ImplGetPrefSize() );
        const sal_uInt16    nOldFormat = rOStm.GetNumberFormatInt();
        sal_uLong           nDataFieldPos;

        rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

        // write correct version ( old style/new style header )
        if( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_50 )
        {
            // write ID for new format (5.0)
            rOStm << GRAPHIC_FORMAT_50;

            // write new style header
            VersionCompat* pCompat = new VersionCompat( rOStm, STREAM_WRITE, 1 );

            //#fdo39428 SvStream no longer supports operator<<(long)
            rOStm << sal::static_int_cast<sal_Int32>(meType);

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm << (sal_Int32) 0;

            rOStm << aSize;
            rOStm << aMapMode;

            delete pCompat;
        }
        else
        {
            // write old style (<=4.0) header
            rOStm << (sal_Int32) meType;

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm << (sal_Int32) 0;
            //#fdo39428 SvStream no longer supports operator<<(long)
            rOStm << sal::static_int_cast<sal_Int32>(aSize.Width());
            rOStm << sal::static_int_cast<sal_Int32>(aSize.Height());
            rOStm << sal::static_int_cast<sal_Int32>(aMapMode.GetMapUnit());
            rOStm << sal::static_int_cast<sal_Int32>(aMapMode.GetScaleX().GetNumerator());
            rOStm << sal::static_int_cast<sal_Int32>(aMapMode.GetScaleX().GetDenominator());
            rOStm << sal::static_int_cast<sal_Int32>(aMapMode.GetScaleY().GetNumerator());
            rOStm << sal::static_int_cast<sal_Int32>(aMapMode.GetScaleY().GetDenominator());
            rOStm << sal::static_int_cast<sal_Int32>(aMapMode.GetOrigin().X());
            rOStm << sal::static_int_cast<sal_Int32>(aMapMode.GetOrigin().Y());
        }

        // write data block
        if( !rOStm.GetError() )
        {
            const sal_uLong nDataStart = rOStm.Tell();

            if( ImplIsSupportedGraphic() )
                rOStm << *this;

            if( !rOStm.GetError() )
            {
                const sal_uLong nStmPos2 = rOStm.Tell();
                rOStm.Seek( nDataFieldPos );
                //fdo39428 SvStream no longer supports operator<<(long)
                rOStm << sal::static_int_cast<sal_Int32>(nStmPos2 - nDataStart);
                rOStm.Seek( nStmPos2 );
                bRet = sal_True;
            }
        }

        rOStm.SetNumberFormatInt( nOldFormat );
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplSwapOut()
{
    sal_Bool bRet = sal_False;

    if( !ImplIsSwapOut() )
    {
        if( !maDocFileURLStr.Len() )
        {
            ::utl::TempFile     aTempFile;
            const INetURLObject aTmpURL( aTempFile.GetURL() );

            if( !aTmpURL.GetMainURL( INetURLObject::NO_DECODE ).isEmpty() )
            {
                SvStream* pOStm = NULL;
                try
                {
                    pOStm = ::utl::UcbStreamHelper::CreateStream( aTmpURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READWRITE | STREAM_SHARE_DENYWRITE );
                }
                catch( const ::com::sun::star::uno::Exception& )
                {
                }
                if( pOStm )
                {
                    pOStm->SetVersion( SOFFICE_FILEFORMAT_50 );
                    pOStm->SetCompressMode( COMPRESSMODE_NATIVE );

                    if( ( bRet = ImplSwapOut( pOStm ) ) == sal_True )
                    {
                        mpSwapFile = new ImpSwapFile;
                        mpSwapFile->nRefCount = 1;
                        mpSwapFile->aSwapURL = aTmpURL;
                    }
                    else
                    {
                        delete pOStm, pOStm = NULL;

                        try
                        {
                            ::ucbhelper::Content aCnt( aTmpURL.GetMainURL( INetURLObject::NO_DECODE ),
                                                 ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                                                 comphelper::getProcessComponentContext() );

                            aCnt.executeCommand( ::rtl::OUString("delete"),
                                                 ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
                        }
                        catch( const ::com::sun::star::ucb::ContentCreationException& )
                        {
                        }
                        catch( const ::com::sun::star::uno::RuntimeException& )
                        {
                        }
                        catch( const ::com::sun::star::ucb::CommandAbortedException& )
                        {
                        }
                        catch( const ::com::sun::star::uno::Exception& )
                        {
                        }
                    }

                    delete pOStm;
                }
            }
        }
        else
        {
            ImplClearGraphics( sal_True );
            bRet = mbSwapOut = sal_True;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplSwapOut( SvStream* pOStm )
{
    sal_Bool bRet = sal_False;

    if( pOStm )
    {
        pOStm->SetBufferSize( GRAPHIC_STREAMBUFSIZE );

        if( !pOStm->GetError() && ImplWriteEmbedded( *pOStm ) )
        {
            pOStm->Flush();

            if( !pOStm->GetError() )
            {
                ImplClearGraphics( sal_True );
                bRet = mbSwapOut = sal_True;
            }
        }
    }
    else
    {
        ImplClearGraphics( sal_True );
        bRet = mbSwapOut = sal_True;
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplSwapIn()
{
    sal_Bool bRet = sal_False;

    if( ImplIsSwapOut() )
    {
        String aSwapURL;

        if( mpSwapFile )
            aSwapURL = mpSwapFile->aSwapURL.GetMainURL( INetURLObject::NO_DECODE );
        else
            aSwapURL = maDocFileURLStr;

        if( aSwapURL.Len() )
        {
            SvStream* pIStm = NULL;
            try
            {
                pIStm = ::utl::UcbStreamHelper::CreateStream( aSwapURL, STREAM_READWRITE | STREAM_SHARE_DENYWRITE );
            }
            catch( const ::com::sun::star::uno::Exception& )
            {
            }

            if( pIStm )
            {
                pIStm->SetVersion( SOFFICE_FILEFORMAT_50 );
                pIStm->SetCompressMode( COMPRESSMODE_NATIVE );

                if( !mpSwapFile )
                    pIStm->Seek( mnDocFilePos );

                bRet = ImplSwapIn( pIStm );
                delete pIStm;

                if( mpSwapFile )
                {
                    if( mpSwapFile->nRefCount > 1 )
                        mpSwapFile->nRefCount--;
                    else
                    {
                        try
                        {
                            ::ucbhelper::Content aCnt( aSwapURL,
                                                 ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                                                 comphelper::getProcessComponentContext() );

                            aCnt.executeCommand( ::rtl::OUString("delete"),
                                                 ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
                        }
                        catch( const ::com::sun::star::ucb::ContentCreationException& )
                        {
                        }
                        catch( const ::com::sun::star::uno::RuntimeException& )
                        {
                        }
                        catch( const ::com::sun::star::ucb::CommandAbortedException& )
                        {
                        }
                        catch( const ::com::sun::star::uno::Exception& )
                        {
                        }

                        delete mpSwapFile;
                    }

                    mpSwapFile = NULL;
                }
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplSwapIn( SvStream* pIStm )
{
    sal_Bool bRet = sal_False;

    if( pIStm )
    {
        pIStm->SetBufferSize( GRAPHIC_STREAMBUFSIZE );

        if( !pIStm->GetError() )
        {
            mbSwapUnderway = sal_True;
            bRet = ImplReadEmbedded( *pIStm );
            mbSwapUnderway = sal_False;

            if( !bRet )
                ImplClear();
            else
                mbSwapOut = sal_False;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool ImpGraphic::ImplIsSwapOut() const
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

sal_Bool ImpGraphic::ImplIsLink() const
{
    return ( mpGfxLink != NULL ) ? sal_True : sal_False;
}

// ------------------------------------------------------------------------

sal_uLong ImpGraphic::ImplGetChecksum() const
{
    sal_uLong nRet = 0;

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

sal_Bool ImpGraphic::ImplExportNative( SvStream& rOStm ) const
{
    sal_Bool bResult = sal_False;

    if( !rOStm.GetError() )
    {
        if( !ImplIsSwapOut() )
        {
            if( mpGfxLink && mpGfxLink->IsNative() )
                bResult = mpGfxLink->ExportNative( rOStm );
            else
            {
                rOStm << *this;
                bResult = ( rOStm.GetError() == ERRCODE_NONE );
            }
        }
        else
             rOStm.SetError( SVSTREAM_GENERALERROR );
    }

    return bResult;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, ImpGraphic& rImpGraphic )
{
    if( !rIStm.GetError() )
    {
        const sal_uLong nStmPos1 = rIStm.Tell();
        sal_uInt32 nTmp;

        if ( !rImpGraphic.mbSwapUnderway )
            rImpGraphic.ImplClear();

        // read Id
        rIStm >> nTmp;

        // if there is no more data, avoid further expensive
        // reading which will create VDevs and other stuff, just to
        // read nothing. CAUTION: Eof is only true AFTER reading another
        // byte, a speciality of SvMemoryStream (!)
        if(!rIStm.GetError() && !rIStm.IsEof())
        {
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
                    const sal_Bool bSetLink = ( rImpGraphic.mpGfxLink == NULL );

                    // assign graphic
                    rImpGraphic = *aGraphic.ImplGetImpGraphic();

                    if( aLink.IsPrefMapModeValid() )
                        rImpGraphic.ImplSetPrefMapMode( aLink.GetPrefMapMode() );

                    if( aLink.IsPrefSizeValid() )
                        rImpGraphic.ImplSetPrefSize( aLink.GetPrefSize() );

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
                const sal_uInt16    nOldFormat = rIStm.GetNumberFormatInt();

                rIStm.SeekRel( -4 );
                rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
                rIStm >> aBmpEx;

                if( !rIStm.GetError() )
                {
                    sal_uInt32  nMagic1(0), nMagic2(0);
                    sal_uLong   nActPos = rIStm.Tell();

                    rIStm >> nMagic1 >> nMagic2;
                    rIStm.Seek( nActPos );

                    rImpGraphic = ImpGraphic( aBmpEx );

                    if( !rIStm.GetError() && ( 0x5344414e == nMagic1 ) && ( 0x494d4931 == nMagic2 ) )
                    {
                        delete rImpGraphic.mpAnimation;
                        rImpGraphic.mpAnimation = new Animation;
                        rIStm >> *rImpGraphic.mpAnimation;

                        // #108077# manually set loaded BmpEx to Animation
                        // (which skips loading its BmpEx if already done)
                        rImpGraphic.mpAnimation->SetBitmapEx(aBmpEx);
                    }
                    else
                        rIStm.ResetError();
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

                rImpGraphic.mpGfxLink->SetPrefMapMode( rImpGraphic.ImplGetPrefMapMode() );
                rImpGraphic.mpGfxLink->SetPrefSize( rImpGraphic.ImplGetPrefSize() );
                rOStm << *rImpGraphic.mpGfxLink;
            }
            else
            {
                // own format
                const sal_uInt16 nOldFormat = rOStm.GetNumberFormatInt();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
