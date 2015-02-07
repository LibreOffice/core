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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <tools/fract.hxx>
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
#include <vcl/dibtools.hxx>
#include <boost/scoped_ptr.hpp>

#define GRAPHIC_MTFTOBMP_MAXEXT     2048
#define GRAPHIC_STREAMBUFSIZE       8192UL

#define SYS_WINMETAFILE             0x00000003L
#define SYS_WNTMETAFILE             0x00000004L
#define SYS_OS2METAFILE             0x00000005L
#define SYS_MACMETAFILE             0x00000006L

#define GRAPHIC_FORMAT_50           static_cast<sal_uInt32>(COMPAT_FORMAT( 'G', 'R', 'F', '5' ))
#define NATIVE_FORMAT_50            static_cast<sal_uInt32>(COMPAT_FORMAT( 'N', 'A', 'T', '5' ))

struct ImpSwapFile
{
    INetURLObject   aSwapURL;
    sal_uLong           nRefCount;
};

class ReaderData
{
public:
    Size    maPreviewSize;
};

GraphicReader::~GraphicReader()
{
    delete mpReaderData;
}

void GraphicReader::DisablePreviewMode()
{
    if( mpReaderData )
        mpReaderData->maPreviewSize = Size( 0, 0 );
}

void GraphicReader::SetPreviewSize( const Size& rSize )
{
    if( !mpReaderData )
        mpReaderData = new ReaderData;
    mpReaderData->maPreviewSize = rSize;
}

Size GraphicReader::GetPreviewSize() const
{
    Size aSize( 0, 0 );
    if( mpReaderData )
        aSize = mpReaderData->maPreviewSize;
    return aSize;
}

ImpGraphic::ImpGraphic() :
        mpAnimation     ( NULL ),
        mpContext       ( NULL ),
        mpSwapFile      ( NULL ),
        mpGfxLink       ( NULL ),
        meType          ( GRAPHIC_NONE ),
        mnDocFilePos    ( 0UL ),
        mnSizeBytes     ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( false ),
        mbSwapUnderway  ( false )
{
}

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
        mbSwapUnderway  ( false )
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

    maSvgData = rImpGraphic.maSvgData;
}

ImpGraphic::ImpGraphic( const Bitmap& rBitmap ) :
        maEx            ( rBitmap ),
        mpAnimation     ( NULL ),
        mpContext       ( NULL ),
        mpSwapFile      ( NULL ),
        mpGfxLink       ( NULL ),
        meType          ( !rBitmap.IsEmpty() ? GRAPHIC_BITMAP : GRAPHIC_NONE ),
        mnDocFilePos    ( 0UL ),
        mnSizeBytes     ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( false ),
        mbSwapUnderway  ( false )
{
}

ImpGraphic::ImpGraphic( const BitmapEx& rBitmapEx ) :
        maEx            ( rBitmapEx ),
        mpAnimation     ( NULL ),
        mpContext       ( NULL ),
        mpSwapFile      ( NULL ),
        mpGfxLink       ( NULL ),
        meType          ( !rBitmapEx.IsEmpty() ? GRAPHIC_BITMAP : GRAPHIC_NONE ),
        mnDocFilePos    ( 0UL ),
        mnSizeBytes     ( 0UL ),
        mnRefCount      ( 1UL ),
        mbSwapOut       ( false ),
        mbSwapUnderway  ( false )
{
}

ImpGraphic::ImpGraphic(const SvgDataPtr& rSvgDataPtr)
:   mpAnimation( NULL ),
    mpContext( NULL ),
    mpSwapFile( NULL ),
    mpGfxLink( NULL ),
    meType( rSvgDataPtr.get() ? GRAPHIC_BITMAP : GRAPHIC_NONE ),
    mnDocFilePos( 0UL ),
    mnSizeBytes( 0UL ),
    mnRefCount( 1UL ),
    mbSwapOut( false ),
    mbSwapUnderway( false ),
    maSvgData(rSvgDataPtr)
{
}

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
        mbSwapOut       ( false ),
        mbSwapUnderway  ( false )
{
}

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
        mbSwapOut       ( false ),
        mbSwapUnderway  ( false )
{
}

ImpGraphic::~ImpGraphic()
{
    ImplClear();

    if( reinterpret_cast<sal_uLong>(mpContext) > 1UL )
        delete mpContext;
}

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

        maSvgData = rImpGraphic.maSvgData;
    }

    return *this;
}

bool ImpGraphic::operator==( const ImpGraphic& rImpGraphic ) const
{
    bool bRet = false;

    if( this == &rImpGraphic )
        bRet = true;
    else if( !ImplIsSwapOut() && ( rImpGraphic.meType == meType ) )
    {
        switch( meType )
        {
            case( GRAPHIC_NONE ):
                bRet = true;
            break;

            case( GRAPHIC_GDIMETAFILE ):
            {
                if( rImpGraphic.maMetaFile == maMetaFile )
                    bRet = true;
            }
            break;

            case( GRAPHIC_BITMAP ):
            {
                if(maSvgData.get())
                {
                    if(maSvgData == rImpGraphic.maSvgData)
                    {
                        bRet = true;
                    }
                    else if(rImpGraphic.maSvgData)
                    {
                        if(maSvgData->getSvgDataArrayLength() == rImpGraphic.maSvgData->getSvgDataArrayLength())
                        {
                            if(0 == memcmp(
                                maSvgData->getSvgDataArray().get(),
                                rImpGraphic.maSvgData->getSvgDataArray().get(),
                                maSvgData->getSvgDataArrayLength()))
                            {
                                bRet = true;
                            }
                        }
                    }
                }
                else if( mpAnimation )
                {
                    if( rImpGraphic.mpAnimation && ( *rImpGraphic.mpAnimation == *mpAnimation ) )
                        bRet = true;
                }
                else if( !rImpGraphic.mpAnimation && ( rImpGraphic.maEx == maEx ) )
                {
                    bRet = true;
                }
            }
            break;

            default:
            break;
        }
    }

    return bRet;
}

void ImpGraphic::ImplClearGraphics( bool bCreateSwapInfo )
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

    maSvgData.reset();
}

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

                aCnt.executeCommand( OUString("delete"),
                                     ::com::sun::star::uno::makeAny( true ) );
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

    mbSwapOut = false;
    mnDocFilePos = 0UL;
    maDocFileURLStr.clear();

    // cleanup
    ImplClearGraphics( false );
    meType = GRAPHIC_NONE;
    mnSizeBytes = 0;
}


void ImpGraphic::ImplSetDefaultType()
{
    ImplClear();
    meType = GRAPHIC_DEFAULT;
}

bool ImpGraphic::ImplIsSupportedGraphic() const
{
    return( meType != GRAPHIC_NONE );
}

bool ImpGraphic::ImplIsTransparent() const
{
    bool bRet(true);

    if( meType == GRAPHIC_BITMAP && !maSvgData.get())
    {
        bRet = ( mpAnimation ? mpAnimation->IsTransparent() : maEx.IsTransparent() );
    }

    return bRet;
}

bool ImpGraphic::ImplIsAlpha() const
{
    bool bRet(false);

    if(maSvgData.get())
    {
        bRet = true;
    }
    else if( meType == GRAPHIC_BITMAP )
    {
        bRet = ( NULL == mpAnimation ) && maEx.IsAlpha();
    }

    return bRet;
}

bool ImpGraphic::ImplIsAnimated() const
{
    return( mpAnimation != NULL );
}

bool ImpGraphic::ImplIsEPS() const
{
    return( ( meType == GRAPHIC_GDIMETAFILE ) &&
            ( maMetaFile.GetActionSize() > 0 ) &&
            ( maMetaFile.GetAction( 0 )->GetType() == META_EPS_ACTION ) );
}

Bitmap ImpGraphic::ImplGetBitmap(const GraphicConversionParameters& rParameters) const
{
    Bitmap aRetBmp;

    if( meType == GRAPHIC_BITMAP )
    {
        if(maSvgData.get() && maEx.IsEmpty())
        {
            // use maEx as local buffer for rendered svg
            const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
        }

        const BitmapEx& rRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maEx );
        const Color     aReplaceColor( COL_WHITE );

        aRetBmp = rRetBmpEx.GetBitmap( &aReplaceColor );

        if(rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
            aRetBmp.Scale(rParameters.getSizePixel());
    }
    else if( ( meType != GRAPHIC_DEFAULT ) && ImplIsSupportedGraphic() )
    {
        if(maEx.IsEmpty())
        {
            // calculate size
            VirtualDevice aVDev;
            Size aDrawSize(aVDev.LogicToPixel(maMetaFile.GetPrefSize(), maMetaFile.GetPrefMapMode()));

            if(rParameters.getSizePixel().Width() && rParameters.getSizePixel().Height())
            {
                // apply given size if exists
                aDrawSize = rParameters.getSizePixel();
            }

            if(aDrawSize.Width() && aDrawSize.Height() && !rParameters.getUnlimitedSize()
                && (aDrawSize.Width() > GRAPHIC_MTFTOBMP_MAXEXT || aDrawSize.Height() > GRAPHIC_MTFTOBMP_MAXEXT))
            {
                // limit bitmap size to a maximum of GRAPHIC_MTFTOBMP_MAXEXT x GRAPHIC_MTFTOBMP_MAXEXT
                double fWH((double)aDrawSize.Width() / (double)aDrawSize.Height());

                if(fWH <= 1.0)
                {
                    aDrawSize.setWidth(basegfx::fround(GRAPHIC_MTFTOBMP_MAXEXT * fWH));
                    aDrawSize.setHeight(GRAPHIC_MTFTOBMP_MAXEXT);
                }
                else
                {
                    aDrawSize.setWidth(GRAPHIC_MTFTOBMP_MAXEXT);
                    aDrawSize.setHeight(basegfx::fround(GRAPHIC_MTFTOBMP_MAXEXT / fWH));
                }
            }

            // calculate pixel size. Normally, it's the same as aDrawSize, but may
            // need to be extended when hairlines are on the right or bottom edge
            Size aPixelSize(aDrawSize);

            if(GRAPHIC_GDIMETAFILE == ImplGetType())
            {
                // get hairline and full bound rect
                Rectangle aHairlineRect;
                const Rectangle aRect(maMetaFile.GetBoundRect(aVDev, &aHairlineRect));

                if(!aRect.IsEmpty() && !aHairlineRect.IsEmpty())
                {
                    // expand if needed to allow bottom and right hairlines to be added
                    if(aRect.Right() == aHairlineRect.Right())
                    {
                        aPixelSize.setWidth(aPixelSize.getWidth() + 1);
                    }

                    if(aRect.Bottom() == aHairlineRect.Bottom())
                    {
                        aPixelSize.setHeight(aPixelSize.getHeight() + 1);
                    }
                }
            }

            if(aVDev.SetOutputSizePixel(aPixelSize))
            {
                if(rParameters.getAntiAliase())
                {
                    aVDev.SetAntialiasing(aVDev.GetAntialiasing() | ANTIALIASING_ENABLE_B2DDRAW);
                }

                if(rParameters.getSnapHorVerLines())
                {
                    aVDev.SetAntialiasing(aVDev.GetAntialiasing() | ANTIALIASING_PIXELSNAPHAIRLINE);
                }

                ImplDraw( &aVDev, Point(), aDrawSize );

                // use maEx as local buffer for rendered metafile
                const_cast< ImpGraphic* >(this)->maEx = aVDev.GetBitmap( Point(), aVDev.GetOutputSizePixel() );
            }
        }

        aRetBmp = maEx.GetBitmap();
    }

    if( !!aRetBmp )
    {
        aRetBmp.SetPrefMapMode( ImplGetPrefMapMode() );
        aRetBmp.SetPrefSize( ImplGetPrefSize() );
    }

    return aRetBmp;
}

BitmapEx ImpGraphic::ImplGetBitmapEx(const GraphicConversionParameters& rParameters) const
{
    BitmapEx aRetBmpEx;

    if( meType == GRAPHIC_BITMAP )
    {
        if(maSvgData.get() && maEx.IsEmpty())
        {
            // use maEx as local buffer for rendered svg
            const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
        }

        aRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maEx );

        if(rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
        {
            aRetBmpEx.Scale(
                rParameters.getSizePixel(),
                rParameters.getScaleHighQuality() ? BMP_SCALE_INTERPOLATE : BMP_SCALE_FAST);
        }
    }
    else if( ( meType != GRAPHIC_DEFAULT ) && ImplIsSupportedGraphic() )
    {
        if(maEx.IsEmpty())
        {
            const ImpGraphic aMonoMask( maMetaFile.GetMonochromeMtf( COL_BLACK ) );

            // use maEx as local buffer for rendered metafile
            const_cast< ImpGraphic* >(this)->maEx = BitmapEx(ImplGetBitmap(rParameters), aMonoMask.ImplGetBitmap(rParameters));
        }

        aRetBmpEx = maEx;
    }

    return aRetBmpEx;
}

Animation ImpGraphic::ImplGetAnimation() const
{
    Animation aAnimation;

    if( mpAnimation )
        aAnimation = *mpAnimation;

    return aAnimation;
}

const GDIMetaFile& ImpGraphic::ImplGetGDIMetaFile() const
{
    if (GRAPHIC_BITMAP == meType && !maMetaFile.GetActionSize())
    {
        // #i119735#
        // Use the local maMetaFile as container for a metafile-representation
        // of the bitmap graphic. This will be done only once, thus be buffered.
        // I checked all usages of maMetaFile, it is only used when type is not
        // GRAPHIC_BITMAP. In operator= it will get copied, thus buffering will
        // survive copying (change this if not wanted)
        ImpGraphic* pThat = const_cast< ImpGraphic* >(this);

        if(maSvgData.get() && !maEx)
        {
            // use maEx as local buffer for rendered svg
            pThat->maEx = maSvgData->getReplacement();
        }

        // #123983# directly create a metafile with the same PrefSize and PrefMapMode
        // the bitmap has, this will be an always correct metafile
        if(maEx.IsTransparent())
        {
            pThat->maMetaFile.AddAction(new MetaBmpExScaleAction(Point(), maEx.GetPrefSize(), maEx));
        }
        else
        {
            pThat->maMetaFile.AddAction(new MetaBmpScaleAction(Point(), maEx.GetPrefSize(), maEx.GetBitmap()));
        }

        pThat->maMetaFile.Stop();
        pThat->maMetaFile.WindStart();
        pThat->maMetaFile.SetPrefSize(maEx.GetPrefSize());
        pThat->maMetaFile.SetPrefMapMode(maEx.GetPrefMapMode());
    }

    return maMetaFile;
}

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
                if(maSvgData.get() && maEx.IsEmpty())
                {
                    // svg not yet buffered in maEx, return size derived from range
                    const basegfx::B2DRange& rRange = maSvgData->getRange();

                    aSize = Size(basegfx::fround(rRange.getWidth()), basegfx::fround(rRange.getHeight()));
                }
                else
                {
                    aSize = maEx.GetPrefSize();

                    if( !aSize.Width() || !aSize.Height() )
                    {
                        aSize = maEx.GetSizePixel();
                    }
                }
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

void ImpGraphic::ImplSetPrefSize( const Size& rPrefSize )
{
    switch( meType )
    {
        case( GRAPHIC_NONE ):
        case( GRAPHIC_DEFAULT ):
        break;

        case( GRAPHIC_BITMAP ):
        {
            //UUUU used when importing a writer FlyFrame with SVG as graphic, added conversion
            // to allow setting the PrefSize at the BitmapEx to hold it
            if(maSvgData.get() && maEx.IsEmpty())
            {
                // use maEx as local buffer for rendered svg
                const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
            }

            // #108077# Push through pref size to animation object,
            // will be lost on copy otherwise
            if( ImplIsAnimated() )
            {
                const_cast< BitmapEx& >(mpAnimation->GetBitmapEx()).SetPrefSize( rPrefSize );
            }

            maEx.SetPrefSize( rPrefSize );
        }
        break;

        default:
        {
            if( ImplIsSupportedGraphic() )
                maMetaFile.SetPrefSize( rPrefSize );
        }
        break;
    }
}

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
                if(maSvgData.get() && maEx.IsEmpty())
                {
                    // svg not yet buffered in maEx, return default PrefMapMode
                    aMapMode = MapMode(MAP_100TH_MM);
                }
                else
                {
                    const Size aSize( maEx.GetPrefSize() );

                    if ( aSize.Width() && aSize.Height() )
                        aMapMode = maEx.GetPrefMapMode();
                }
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

void ImpGraphic::ImplSetPrefMapMode( const MapMode& rPrefMapMode )
{
    switch( meType )
    {
        case( GRAPHIC_NONE ):
        case( GRAPHIC_DEFAULT ):
        break;

        case( GRAPHIC_BITMAP ):
        {
            if(maSvgData.get())
            {
                // ignore for Svg. If this is really used (except the grfcache)
                // it can be extended by using maEx as buffer for maSvgData->getReplacement()
            }
            else
            {
                // #108077# Push through pref mapmode to animation object,
                // will be lost on copy otherwise
                if( ImplIsAnimated() )
                {
                    const_cast< BitmapEx& >(mpAnimation->GetBitmapEx()).SetPrefMapMode( rPrefMapMode );
                }

                maEx.SetPrefMapMode( rPrefMapMode );
            }
        }
        break;

        default:
        {
            if( ImplIsSupportedGraphic() )
                maMetaFile.SetPrefMapMode( rPrefMapMode );
        }
        break;
    }
}

sal_uLong ImpGraphic::ImplGetSizeBytes() const
{
    if( 0 == mnSizeBytes )
    {
        if( meType == GRAPHIC_BITMAP )
        {
            if(maSvgData.get())
            {
                mnSizeBytes = maSvgData->getSvgDataArrayLength();
            }
            else
            {
                mnSizeBytes = mpAnimation ? mpAnimation->GetSizeBytes() : maEx.GetSizeBytes();
            }
        }
        else if( meType == GRAPHIC_GDIMETAFILE )
        {
            mnSizeBytes = maMetaFile.GetSizeBytes();
        }
    }

    return( mnSizeBytes );
}

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
                if(maSvgData.get() && !maEx)
                {
                    // use maEx as local buffer for rendered svg
                    const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
                }

                if ( mpAnimation )
                {
                    mpAnimation->Draw( pOutDev, rDestPt );
                }
                else
                {
                    maEx.Draw( pOutDev, rDestPt );
                }
            }
            break;

            default:
                ImplDraw( pOutDev, rDestPt, maMetaFile.GetPrefSize() );
            break;
        }
    }
}

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
                if(maSvgData.get() && maEx.IsEmpty())
                {
                    // use maEx as local buffer for rendered svg
                    const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
                }

                if( mpAnimation )
                {
                    mpAnimation->Draw( pOutDev, rDestPt, rDestSize );
                }
                else
                {
                    maEx.Draw( pOutDev, rDestPt, rDestSize );
                }
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

void ImpGraphic::ImplStartAnimation( OutputDevice* pOutDev, const Point& rDestPt,
                                     const Size& rDestSize, long nExtraData,
                                     OutputDevice* pFirstFrameOutDev )
{
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() && mpAnimation )
        mpAnimation->Start( pOutDev, rDestPt, rDestSize, nExtraData, pFirstFrameOutDev );
}

void ImpGraphic::ImplStopAnimation( OutputDevice* pOutDev, long nExtraData )
{
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() && mpAnimation )
        mpAnimation->Stop( pOutDev, nExtraData );
}

void ImpGraphic::ImplSetAnimationNotifyHdl( const Link& rLink )
{
    if( mpAnimation )
        mpAnimation->SetNotifyHdl( rLink );
}

Link ImpGraphic::ImplGetAnimationNotifyHdl() const
{
    Link aLink;

    if( mpAnimation )
        aLink = mpAnimation->GetNotifyHdl();

    return aLink;
}

sal_uLong ImpGraphic::ImplGetAnimationLoopCount() const
{
    return( mpAnimation ? mpAnimation->GetLoopCount() : 0UL );
}


void ImpGraphic::ImplSetContext( GraphicReader* pReader )
{
    mpContext = pReader;
}

void ImpGraphic::ImplSetDocFileName( const OUString& rName, sal_uLong nFilePos )
{
    const INetURLObject aURL( rName );

    DBG_ASSERT( rName.isEmpty() || ( aURL.GetProtocol() != INET_PROT_NOT_VALID ), "Graphic::SetDocFileName(...): invalid URL" );

    maDocFileURLStr = aURL.GetMainURL( INetURLObject::NO_DECODE );
    mnDocFilePos = nFilePos;
}

const OUString& ImpGraphic::ImplGetDocFileName() const
{
    return maDocFileURLStr;
}


bool ImpGraphic::ImplReadEmbedded( SvStream& rIStm )
{
    MapMode         aMapMode;
    Size            aSize;
    sal_uInt32      nId;
    sal_Int32       nType;
    sal_Int32       nLen;
    const sal_uInt16    nOldFormat = rIStm.GetNumberFormatInt();
    bool            bRet = false;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rIStm.ReadUInt32( nId );

    // check version
    if( GRAPHIC_FORMAT_50 == nId )
    {
        // read new style header
        VersionCompat* pCompat = new VersionCompat( rIStm, STREAM_READ );

        rIStm.ReadInt32( nType );
        rIStm.ReadInt32( nLen );
        ReadPair( rIStm, aSize );
        ReadMapMode( rIStm, aMapMode );

        delete pCompat;
    }
    else
    {
        // read old style header
        sal_Int32 nWidth, nHeight;
        sal_Int32 nMapMode, nScaleNumX, nScaleDenomX;
        sal_Int32 nScaleNumY, nScaleDenomY, nOffsX, nOffsY;

        rIStm.SeekRel( -4L );

        rIStm.ReadInt32( nType ).ReadInt32( nLen ).ReadInt32( nWidth ).ReadInt32( nHeight );
        rIStm.ReadInt32( nMapMode ).ReadInt32( nScaleNumX ).ReadInt32( nScaleDenomX ).ReadInt32( nScaleNumY );
        rIStm.ReadInt32( nScaleDenomY ).ReadInt32( nOffsX ).ReadInt32( nOffsY );

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

    meType = (GraphicType) nType;

    if( meType )
    {
        if( meType == GRAPHIC_BITMAP )
        {
            if(maSvgData.get() && maEx.IsEmpty())
            {
                // use maEx as local buffer for rendered svg
                maEx = maSvgData->getReplacement();
            }

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

        if( meType == GRAPHIC_BITMAP || meType == GRAPHIC_GDIMETAFILE )
        {
            ReadImpGraphic( rIStm, *this );
            bRet = ( rIStm.GetError() == 0UL );
        }
        else if( sal::static_int_cast<sal_uLong>(meType) >= SYS_WINMETAFILE
                 && sal::static_int_cast<sal_uLong>(meType) <= SYS_MACMETAFILE )
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
        bRet = true;

    rIStm.SetNumberFormatInt( nOldFormat );

    return bRet;
}

bool ImpGraphic::ImplWriteEmbedded( SvStream& rOStm )
{
    bool bRet = false;

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
            rOStm.WriteUInt32( GRAPHIC_FORMAT_50 );

            // write new style header
            VersionCompat* pCompat = new VersionCompat( rOStm, STREAM_WRITE, 1 );

            rOStm.WriteInt32( meType );

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm.WriteInt32( 0 );

            WritePair( rOStm, aSize );
            WriteMapMode( rOStm, aMapMode );

            delete pCompat;
        }
        else
        {
            // write old style (<=4.0) header
            rOStm.WriteInt32( meType );

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm.WriteInt32( 0 );
            rOStm.WriteInt32( aSize.Width() );
            rOStm.WriteInt32( aSize.Height() );
            rOStm.WriteInt32( aMapMode.GetMapUnit() );
            rOStm.WriteInt32( aMapMode.GetScaleX().GetNumerator() );
            rOStm.WriteInt32( aMapMode.GetScaleX().GetDenominator() );
            rOStm.WriteInt32( aMapMode.GetScaleY().GetNumerator() );
            rOStm.WriteInt32( aMapMode.GetScaleY().GetDenominator() );
            rOStm.WriteInt32( aMapMode.GetOrigin().X() );
            rOStm.WriteInt32( aMapMode.GetOrigin().Y() );
        }

        // write data block
        if( !rOStm.GetError() )
        {
            const sal_uLong nDataStart = rOStm.Tell();

            if( ImplIsSupportedGraphic() )
                WriteImpGraphic( rOStm, *this );

            if( !rOStm.GetError() )
            {
                const sal_uLong nStmPos2 = rOStm.Tell();
                rOStm.Seek( nDataFieldPos );
                rOStm.WriteInt32( nStmPos2 - nDataStart );
                rOStm.Seek( nStmPos2 );
                bRet = true;
            }
        }

        rOStm.SetNumberFormatInt( nOldFormat );
    }

    return bRet;
}

bool ImpGraphic::ImplSwapOut()
{
    bool bRet = false;

    if( !ImplIsSwapOut() )
    {
        if (maDocFileURLStr.isEmpty())
        {
            ::utl::TempFile     aTempFile;
            const INetURLObject aTmpURL( aTempFile.GetURL() );

            if( !aTmpURL.GetMainURL( INetURLObject::NO_DECODE ).isEmpty() )
            {
                boost::scoped_ptr<SvStream> pOStm;
                try
                {
                    pOStm.reset(::utl::UcbStreamHelper::CreateStream( aTmpURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READWRITE | STREAM_SHARE_DENYWRITE ));
                }
                catch( const ::com::sun::star::uno::Exception& )
                {
                }
                if( pOStm )
                {
                    pOStm->SetVersion( SOFFICE_FILEFORMAT_50 );
                    pOStm->SetCompressMode( COMPRESSMODE_NATIVE );

                    if( ( bRet = ImplSwapOut( pOStm.get() ) ) )
                    {
                        mpSwapFile = new ImpSwapFile;
                        mpSwapFile->nRefCount = 1;
                        mpSwapFile->aSwapURL = aTmpURL;
                    }
                    else
                    {
                        pOStm.reset();

                        try
                        {
                            ::ucbhelper::Content aCnt( aTmpURL.GetMainURL( INetURLObject::NO_DECODE ),
                                                 ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                                                 comphelper::getProcessComponentContext() );

                            aCnt.executeCommand( OUString("delete"),
                                                 ::com::sun::star::uno::makeAny( true ) );
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
            }
        }
        else
        {
            ImplClearGraphics( true );
            bRet = mbSwapOut = true;
        }
    }

    return bRet;
}

void ImpGraphic::ImplSwapOutAsLink()
{
    ImplClearGraphics( true );
    mbSwapOut = true;
}

bool ImpGraphic::ImplSwapOut( SvStream* pOStm )
{
    bool bRet = false;

    if( pOStm )
    {
        pOStm->SetBufferSize( GRAPHIC_STREAMBUFSIZE );

        if( !pOStm->GetError() && ImplWriteEmbedded( *pOStm ) )
        {
            pOStm->Flush();

            if( !pOStm->GetError() )
            {
                ImplClearGraphics( true );
                bRet = mbSwapOut = true;
            }
        }
    }
    else
    {
        SAL_WARN("vcl.gdi", "Graphic SwapOut: No stream for swap out!");
    }

    return bRet;
}

bool ImpGraphic::ImplSwapIn()
{
    bool bRet = false;

    if( ImplIsSwapOut() )
    {
        OUString aSwapURL;

        if( mpSwapFile )
            aSwapURL = mpSwapFile->aSwapURL.GetMainURL( INetURLObject::NO_DECODE );
        else
            aSwapURL = maDocFileURLStr;

        if( !aSwapURL.isEmpty() )
        {
            boost::scoped_ptr<SvStream> pIStm;
            try
            {
                pIStm.reset(::utl::UcbStreamHelper::CreateStream( aSwapURL, STREAM_READWRITE | STREAM_SHARE_DENYWRITE ));
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

                bRet = ImplSwapIn( pIStm.get() );
                pIStm.reset();

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

                            aCnt.executeCommand( OUString("delete"),
                                                 ::com::sun::star::uno::makeAny( true ) );
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

bool ImpGraphic::ImplSwapIn( SvStream* pIStm )
{
    bool bRet = false;

    if( pIStm )
    {
        pIStm->SetBufferSize( GRAPHIC_STREAMBUFSIZE );

        if( !pIStm->GetError() )
        {
            mbSwapUnderway = true;
            bRet = ImplReadEmbedded( *pIStm );
            mbSwapUnderway = false;

            if( !bRet )
                ImplClear();
            else
                mbSwapOut = false;
        }
    }

    return bRet;
}


void ImpGraphic::ImplSetLink( const GfxLink& rGfxLink )
{
    delete mpGfxLink;
    mpGfxLink = new GfxLink( rGfxLink );

    if( mpGfxLink->IsNative() )
        mpGfxLink->SwapOut();
}

GfxLink ImpGraphic::ImplGetLink()
{
    return( mpGfxLink ? *mpGfxLink : GfxLink() );
}

bool ImpGraphic::ImplIsLink() const
{
    return ( mpGfxLink != NULL );
}

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
                if(maSvgData.get() && maEx.IsEmpty())
                {
                    // use maEx as local buffer for rendered svg
                    const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
                }

                if( mpAnimation )
                {
                    nRet = mpAnimation->GetChecksum();
                }
                else
                {
                    nRet = maEx.GetChecksum();
                }
            }
            break;

            default:
                nRet = maMetaFile.GetChecksum();
            break;
        }
    }

    return nRet;
}

bool ImpGraphic::ImplExportNative( SvStream& rOStm ) const
{
    bool bResult = false;

    if( !rOStm.GetError() )
    {
        if( !ImplIsSwapOut() )
        {
            if( mpGfxLink && mpGfxLink->IsNative() )
                bResult = mpGfxLink->ExportNative( rOStm );
            else
            {
                WriteImpGraphic( rOStm, *this );
                bResult = ( rOStm.GetError() == ERRCODE_NONE );
            }
        }
        else
             rOStm.SetError( SVSTREAM_GENERALERROR );
    }

    return bResult;
}


SvStream& ReadImpGraphic( SvStream& rIStm, ImpGraphic& rImpGraphic )
{
    if( !rIStm.GetError() )
    {
        const sal_uLong nStmPos1 = rIStm.Tell();
        sal_uInt32 nTmp;

        if ( !rImpGraphic.mbSwapUnderway )
            rImpGraphic.ImplClear();

        // read Id
        rIStm.ReadUInt32( nTmp );

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

                ReadGfxLink( rIStm, aLink );

                // set dummy link to avoid creation of additional link after filtering;
                // we set a default link to avoid unnecessary swapping of native data
                aGraphic.SetLink( GfxLink() );

                if( !rIStm.GetError() && aLink.LoadNative( aGraphic ) )
                {
                    // set link only, if no other link was set
                    const bool bSetLink = ( rImpGraphic.mpGfxLink == NULL );

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
                ReadDIBBitmapEx(aBmpEx, rIStm);

                if( !rIStm.GetError() )
                {
                    sal_uInt32  nMagic1(0), nMagic2(0);
                    sal_uLong   nActPos = rIStm.Tell();

                    rIStm.ReadUInt32( nMagic1 ).ReadUInt32( nMagic2 );
                    rIStm.Seek( nActPos );

                    rImpGraphic = ImpGraphic( aBmpEx );

                    if( !rIStm.GetError() && ( 0x5344414e == nMagic1 ) && ( 0x494d4931 == nMagic2 ) )
                    {
                        delete rImpGraphic.mpAnimation;
                        rImpGraphic.mpAnimation = new Animation;
                        ReadAnimation( rIStm, *rImpGraphic.mpAnimation );

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
                    ReadGDIMetaFile( rIStm, aMtf );

                    if( !rIStm.GetError() )
                    {
                        rImpGraphic = aMtf;
                    }
                    else
                    {
                        // try to stream in Svg defining data (length, byte array and evtl. path)
                        // See below (operator<<) for more information
                        const sal_uInt32 nSvgMagic((sal_uInt32('s') << 24) | (sal_uInt32('v') << 16) | (sal_uInt32('g') << 8) | sal_uInt32('0'));
                        sal_uInt32 nMagic;
                        rIStm.Seek(nStmPos1);
                        rIStm.ResetError();
                        rIStm.ReadUInt32( nMagic );

                        if(nSvgMagic == nMagic)
                        {
                            sal_uInt32 mnSvgDataArrayLength(0);
                            rIStm.ReadUInt32( mnSvgDataArrayLength );

                            if(mnSvgDataArrayLength)
                            {
                                SvgDataArray aNewData(new sal_uInt8[mnSvgDataArrayLength]);
                                OUString aPath;

                                rIStm.Read(aNewData.get(), mnSvgDataArrayLength);
                                aPath = rIStm.ReadUniOrByteString(rIStm.GetStreamCharSet());

                                if(!rIStm.GetError())
                                {
                                    SvgDataPtr aSvgDataPtr(
                                        new SvgData(
                                            aNewData,
                                            mnSvgDataArrayLength,
                                            OUString(aPath)));

                                    rImpGraphic = aSvgDataPtr;
                                }
                            }
                        }

                        rIStm.Seek(nStmPos1);
                    }
                }

                rIStm.SetNumberFormatInt( nOldFormat );
            }
        }
    }

    return rIStm;
}

SvStream& WriteImpGraphic( SvStream& rOStm, const ImpGraphic& rImpGraphic )
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
                rOStm.WriteUInt32( NATIVE_FORMAT_50 );

                // write compat info
                pCompat = new VersionCompat( rOStm, STREAM_WRITE, 1 );
                delete pCompat;

                rImpGraphic.mpGfxLink->SetPrefMapMode( rImpGraphic.ImplGetPrefMapMode() );
                rImpGraphic.mpGfxLink->SetPrefSize( rImpGraphic.ImplGetPrefSize() );
                WriteGfxLink( rOStm, *rImpGraphic.mpGfxLink );
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
                        if(rImpGraphic.getSvgData().get())
                        {
                            // stream out Svg defining data (length, byte array and evtl. path)
                            // this is used e.g. in swapping out graphic data and in transporting it over UNO API
                            // as sequence of bytes, but AFAIK not written anywhere to any kind of file, so it should be
                            // no problem to extend it; only used at runtime
                            const sal_uInt32 nSvgMagic((sal_uInt32('s') << 24) | (sal_uInt32('v') << 16) | (sal_uInt32('g') << 8) | sal_uInt32('0'));

                            rOStm.WriteUInt32( nSvgMagic );
                            rOStm.WriteUInt32( rImpGraphic.getSvgData()->getSvgDataArrayLength() );
                            rOStm.Write(rImpGraphic.getSvgData()->getSvgDataArray().get(), rImpGraphic.getSvgData()->getSvgDataArrayLength());
                            rOStm.WriteUniOrByteString(rImpGraphic.getSvgData()->getPath(),
                                                       rOStm.GetStreamCharSet());
                        }
                        else if( rImpGraphic.ImplIsAnimated())
                        {
                            WriteAnimation( rOStm, *rImpGraphic.mpAnimation );
                        }
                        else
                        {
                            WriteDIBBitmapEx(rImpGraphic.maEx, rOStm);
                        }
                    }
                    break;

                    default:
                    {
                        if( rImpGraphic.ImplIsSupportedGraphic() )
                            WriteGDIMetaFile( rOStm, rImpGraphic.maMetaFile );
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
