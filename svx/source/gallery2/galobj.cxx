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


#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/string.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/rcid.h>
#include <tools/vcompat.hxx>
#include <tools/helpers.hxx>
#include <vcl/virdev.hxx>
#include <svl/itempool.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmview.hxx>
#include <svx/fmpage.hxx>
#include "gallery.hrc"
#include "svx/galmisc.hxx"
#include "galobj.hxx"
#include <vcl/svapp.hxx>
#include <vcl/dibtools.hxx>
#include "gallerydrawmodel.hxx"

using namespace ::com::sun::star;

SgaObject::SgaObject()
:   bIsValid    ( sal_False ),
    bIsThumbBmp ( sal_True )
{
}

BitmapEx SgaObject::createPreviewBitmapEx(const Size& rSizePixel) const
{
    BitmapEx aRetval;

    if(rSizePixel.Width() && rSizePixel.Height())
    {
        if(SGA_OBJ_SOUND == GetObjKind())
        {
            aRetval = GAL_RES(RID_SVXBMP_GALLERY_MEDIA);
        }
        else if(IsThumbBitmap())
        {
            aRetval = GetThumbBmp();
        }
        else
        {
            const Graphic aGraphic(GetThumbMtf());

            aRetval = aGraphic.GetBitmapEx();
        }

        if(!aRetval.IsEmpty())
        {
            const Size aCurrentSizePixel(aRetval.GetSizePixel());
            const double fScaleX((double)rSizePixel.Width() / (double)aCurrentSizePixel.Width());
            const double fScaleY((double)rSizePixel.Height() / (double)aCurrentSizePixel.Height());
            const double fScale(std::min(fScaleX, fScaleY));

            // only scale when need to decrease, no need to make bigger as original. Also
            // prevent scaling close to 1.0 which is not needed for pixel graphics
            if(fScale < 1.0 && fabs(1.0 - fScale) > 0.005)
            {
                static sal_uInt32 nScaleFlag = BMP_SCALE_BESTQUALITY;

                aRetval.Scale(fScale, fScale, nScaleFlag);
            }
        }
    }

    return aRetval;
}

sal_Bool SgaObject::CreateThumb( const Graphic& rGraphic )
{
    sal_Bool bRet = sal_False;

    if( rGraphic.GetType() == GRAPHIC_BITMAP )
    {
        BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
        Size        aBmpSize( aBmpEx.GetSizePixel() );

        if( aBmpSize.Width() && aBmpSize.Height() )
        {
            if( aBmpEx.GetPrefMapMode().GetMapUnit() != MAP_PIXEL &&
                aBmpEx.GetPrefSize().Width() > 0 &&
                aBmpEx.GetPrefSize().Height() > 0 )
            {
                Size aLogSize( OutputDevice::LogicToLogic( aBmpEx.GetPrefSize(), aBmpEx.GetPrefMapMode(), MAP_100TH_MM ) );

                if( aLogSize.Width() > 0 && aLogSize.Height() > 0 )
                {
                    double  fFactorLog = static_cast< double >( aLogSize.Width() ) / aLogSize.Height();
                    double  fFactorPix = static_cast< double >( aBmpSize.Width() ) / aBmpSize.Height();

                    if( fFactorPix > fFactorLog )
                        aBmpSize.Width() = FRound( aBmpSize.Height() * fFactorLog );
                    else
                        aBmpSize.Height() = FRound( aBmpSize.Width() / fFactorLog );

                    aBmpEx.SetSizePixel( aBmpSize, BMP_SCALE_BESTQUALITY );
                }
            }

            // take over BitmapEx
            aThumbBmp = aBmpEx;

            if( ( aBmpSize.Width() <= S_THUMB ) && ( aBmpSize.Height() <= S_THUMB ) )
            {
                aThumbBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
                bRet = sal_True;
            }
            else
            {
                const float fFactor  = (float) aBmpSize.Width() / aBmpSize.Height();
                const Size  aNewSize( std::max( (long) (fFactor < 1. ? S_THUMB * fFactor : S_THUMB), 8L ),
                                      std::max( (long) (fFactor < 1. ? S_THUMB : S_THUMB / fFactor), 8L ) );
                if(aThumbBmp.Scale(
                    (double) aNewSize.Width() / aBmpSize.Width(),
                    (double) aNewSize.Height() / aBmpSize.Height(),
                    BMP_SCALE_BESTQUALITY ) )
                {
                    aThumbBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
                    bRet = sal_True;
                }
            }
        }
    }
    else if( rGraphic.GetType() == GRAPHIC_GDIMETAFILE )
    {
        const Size aPrefSize( rGraphic.GetPrefSize() );
        const double fFactor  = (double)aPrefSize.Width() / (double)aPrefSize.Height();
        Size aSize( S_THUMB, S_THUMB );
        if ( fFactor < 1.0 )
            aSize.Width() = (sal_Int32)( S_THUMB * fFactor );
        else
            aSize.Height() = (sal_Int32)( S_THUMB / fFactor );

        const GraphicConversionParameters aParameters(aSize, false, true, true /*TODO: extra ", true" post-#i121194#*/);
        aThumbBmp = rGraphic.GetBitmapEx(aParameters);

        if( !aThumbBmp.IsEmpty() )
        {
            aThumbBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
            bRet = sal_True;
        }
    }

    return bRet;
}

void SgaObject::WriteData( SvStream& rOut, const OUString& rDestDir ) const
{
    static const sal_uInt32 nInventor = COMPAT_FORMAT( 'S', 'G', 'A', '3' );

    rOut << nInventor << (sal_uInt16) 0x0004 << GetVersion() << (sal_uInt16) GetObjKind();
    rOut << bIsThumbBmp;

    if( bIsThumbBmp )
    {
        const sal_uInt16    nOldCompressMode = rOut.GetCompressMode();
        const sal_uIntPtr       nOldVersion = rOut.GetVersion();

        rOut.SetCompressMode( COMPRESSMODE_ZBITMAP );
        rOut.SetVersion( SOFFICE_FILEFORMAT_50 );

        WriteDIBBitmapEx(aThumbBmp, rOut);

        rOut.SetVersion( nOldVersion );
        rOut.SetCompressMode( nOldCompressMode );
    }
    else
        rOut << aThumbMtf;

    OUString aURLWithoutDestDir = aURL.GetMainURL( INetURLObject::NO_DECODE );
    aURLWithoutDestDir = aURLWithoutDestDir.replaceFirst(rDestDir, "");
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, aURLWithoutDestDir, RTL_TEXTENCODING_UTF8);
}

void SgaObject::ReadData(SvStream& rIn, sal_uInt16& rReadVersion )
{
    sal_uInt32      nTmp32;
    sal_uInt16      nTmp16;

    rIn >> nTmp32 >> nTmp16 >> rReadVersion >> nTmp16 >> bIsThumbBmp;

    if( bIsThumbBmp )
    {
        ReadDIBBitmapEx(aThumbBmp, rIn);
    }
    else
    {
        rIn >> aThumbMtf;
    }

    OUString aTmpStr = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn, RTL_TEXTENCODING_UTF8);
    aURL = INetURLObject(aTmpStr);
}

const OUString SgaObject::GetTitle() const
{
    OUString aReturnValue( aTitle );
    if ( !getenv( "GALLERY_SHOW_PRIVATE_TITLE" ) )
    {
        if ( comphelper::string::getTokenCount(aReturnValue, ':') == 3 )
        {
            OUString    aPrivateInd  ( aReturnValue.getToken( 0, ':' ) );
            OUString    aResourceName( aReturnValue.getToken( 1, ':' ) );
            sal_Int32   nResId       ( aReturnValue.getToken( 2, ':' ).toInt32() );
            if ( aPrivateInd == "private" &&
                !aResourceName.isEmpty() && ( nResId > 0 ) && ( nResId < 0x10000 ) )
            {
                OString aMgrName(OUStringToOString(aResourceName, RTL_TEXTENCODING_UTF8));
                ResMgr* pResMgr = ResMgr::CreateResMgr( aMgrName.getStr(),
                            Application::GetSettings().GetUILanguageTag() );
                if ( pResMgr )
                {
                    ResId aResId( (sal_uInt16)nResId, *pResMgr );
                    aResId.SetRT( RSC_STRING );
                    if ( pResMgr->IsAvailable( aResId ) )
                    {
                        aReturnValue = aResId.toString();
                    }
                    delete pResMgr;
                }
            }
        }
    }
    return aReturnValue;
}

void SgaObject::SetTitle( const OUString& rTitle )
{
    aTitle = rTitle;
}

SvStream& operator<<( SvStream& rOut, const SgaObject& rObj )
{
    rObj.WriteData( rOut, "" );
    return rOut;
}

SvStream& operator>>( SvStream& rIn, SgaObject& rObj )
{
    sal_uInt16 nReadVersion;

    rObj.ReadData( rIn, nReadVersion );
    rObj.bIsValid = ( rIn.GetError() == ERRCODE_NONE );

    return rIn;
}

SgaObjectBmp::SgaObjectBmp()
{
}

SgaObjectBmp::SgaObjectBmp( const INetURLObject& rURL )
{
    Graphic aGraphic;
    OUString  aFilter;

    if ( SGA_IMPORT_NONE != GalleryGraphicImport( rURL, aGraphic, aFilter ) )
        Init( aGraphic, rURL );
}

SgaObjectBmp::SgaObjectBmp( const Graphic& rGraphic, const INetURLObject& rURL, const OUString& )
{
    if( FileExists( rURL ) )
        Init( rGraphic, rURL );
}

void SgaObjectBmp::Init( const Graphic& rGraphic, const INetURLObject& rURL )
{
    aURL = rURL;
    bIsValid = CreateThumb( rGraphic );
}

void SgaObjectBmp::WriteData( SvStream& rOut, const OUString& rDestDir ) const
{
    // Set version
    SgaObject::WriteData( rOut, rDestDir );
    char aDummy[ 10 ];
    rOut.Write( aDummy, 10 );
    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rOut, OString()); //dummy
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, aTitle, RTL_TEXTENCODING_UTF8);
}

void SgaObjectBmp::ReadData( SvStream& rIn, sal_uInt16& rReadVersion )
{

    SgaObject::ReadData( rIn, rReadVersion );
    rIn.SeekRel( 10 ); // 16, 16, 32, 16
    read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rIn); //dummy

    if( rReadVersion >= 5 )
        aTitle = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn, RTL_TEXTENCODING_UTF8);
}

DBG_NAME(SgaObjectSound)

SgaObjectSound::SgaObjectSound() :
    eSoundType( SOUND_STANDARD )
{
    DBG_CTOR(SgaObjectSound,NULL);
}

SgaObjectSound::SgaObjectSound( const INetURLObject& rURL ) :
    eSoundType( SOUND_STANDARD )
{
    DBG_CTOR(SgaObjectSound,NULL);

    if( FileExists( rURL ) )
    {
        aURL = rURL;
        aThumbBmp = Bitmap( Size( 1, 1 ), 1 );
        bIsValid = sal_True;
    }
    else
        bIsValid = sal_False;
}

SgaObjectSound::~SgaObjectSound()
{
    DBG_DTOR(SgaObjectSound,NULL);
}

BitmapEx SgaObjectSound::GetThumbBmp() const
{
    sal_uInt16 nId;

    switch( eSoundType )
    {
        case( SOUND_COMPUTER ): nId = RID_SVXBMP_GALLERY_SOUND_1; break;
        case( SOUND_MISC ): nId = RID_SVXBMP_GALLERY_SOUND_2; break;
        case( SOUND_MUSIC ): nId = RID_SVXBMP_GALLERY_SOUND_3; break;
        case( SOUND_NATURE ): nId = RID_SVXBMP_GALLERY_SOUND_4; break;
        case( SOUND_SPEECH ): nId = RID_SVXBMP_GALLERY_SOUND_5; break;
        case( SOUND_TECHNIC ): nId = RID_SVXBMP_GALLERY_SOUND_6; break;
        case( SOUND_ANIMAL ): nId = RID_SVXBMP_GALLERY_SOUND_7; break;

        // standard
        default:
             nId = RID_SVXBMP_GALLERY_MEDIA;
        break;
    }

    const BitmapEx  aBmpEx( GAL_RES( nId ) );

    return aBmpEx;
}

void SgaObjectSound::WriteData( SvStream& rOut, const OUString& rDestDir ) const
{
    SgaObject::WriteData( rOut, rDestDir );
    rOut << (sal_uInt16) eSoundType;
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, aTitle, RTL_TEXTENCODING_UTF8);
}

void SgaObjectSound::ReadData( SvStream& rIn, sal_uInt16& rReadVersion )
{
    SgaObject::ReadData( rIn, rReadVersion );

    if( rReadVersion >= 5 )
    {
        sal_uInt16      nTmp16;

        rIn >> nTmp16; eSoundType = (GalSoundType) nTmp16;

        if( rReadVersion >= 6 )
            aTitle = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn, RTL_TEXTENCODING_UTF8);
    }
}

SgaObjectAnim::SgaObjectAnim()
{
}

SgaObjectAnim::SgaObjectAnim( const Graphic& rGraphic,
                              const INetURLObject& rURL,
                              const OUString& )
{
    aURL = rURL;
    bIsValid = CreateThumb( rGraphic );
}

SgaObjectINet::SgaObjectINet()
{
}

SgaObjectINet::SgaObjectINet( const Graphic& rGraphic, const INetURLObject& rURL, const OUString& rFormatName ) :
            SgaObjectAnim   ( rGraphic, rURL, rFormatName )
{
}

SgaObjectSvDraw::SgaObjectSvDraw()
{
}

SgaObjectSvDraw::SgaObjectSvDraw( const FmFormModel& rModel, const INetURLObject& rURL )
{
    aURL = rURL;
    bIsValid = CreateThumb( rModel );
}

DBG_NAME(SvxGalleryDrawModel)

SvxGalleryDrawModel::SvxGalleryDrawModel()
: mpFormModel( 0 )
{
    DBG_CTOR(SvxGalleryDrawModel,NULL);

    const OUString sFactoryURL("sdraw");

    mxDoc = SfxObjectShell::CreateObjectByFactoryName( sFactoryURL );

    if( mxDoc.Is() )
    {
        mxDoc->DoInitNew(0);

        uno::Reference< lang::XUnoTunnel > xTunnel( mxDoc->GetModel(), uno::UNO_QUERY );
        if( xTunnel.is() )
        {
            mpFormModel = dynamic_cast< FmFormModel* >(
                reinterpret_cast<SdrModel*>(xTunnel->getSomething(SdrModel::getUnoTunnelImplementationId())));
            if( mpFormModel )
            {
                mpFormModel->InsertPage( mpFormModel->AllocPage( false ) );
            }
        }
    }
}

SvxGalleryDrawModel::~SvxGalleryDrawModel()
{
    if( mxDoc.Is() )
        mxDoc->DoClose();

    DBG_DTOR(SvxGalleryDrawModel,NULL);
}

SgaObjectSvDraw::SgaObjectSvDraw( SvStream& rIStm, const INetURLObject& rURL )
{
    SvxGalleryDrawModel aModel;

    if( aModel.GetModel() )
    {
        if( GallerySvDrawImport( rIStm, *aModel.GetModel()  ) )
        {
            aURL = rURL;
            bIsValid = CreateThumb( *aModel.GetModel()  );
        }
    }
}

sal_Bool SgaObjectSvDraw::CreateThumb( const FmFormModel& rModel )
{
    Graphic     aGraphic;
    ImageMap    aImageMap;
    sal_Bool        bRet = sal_False;

    if ( CreateIMapGraphic( rModel, aGraphic, aImageMap ) )
    {
        bRet = SgaObject::CreateThumb( aGraphic );
    }
    else
    {
        const FmFormPage* pPage = static_cast< const FmFormPage* >(rModel.GetPage(0));

        if(pPage)
        {
            const Rectangle aObjRect(pPage->GetAllObjBoundRect());

            if(aObjRect.GetWidth() && aObjRect.GetHeight())
            {
                VirtualDevice aVDev;
                FmFormView aView(const_cast< FmFormModel* >(&rModel), &aVDev);

                aView.ShowSdrPage(const_cast< FmFormPage* >(pPage));
                aView.MarkAllObj();
                aThumbBmp = aView.GetMarkedObjBitmapEx();

                const Size aDiscreteSize(aThumbBmp.GetSizePixel());

                if(aDiscreteSize.Width() && aDiscreteSize.Height())
                {
                    sal_uInt32 nTargetSizeX(S_THUMB);
                    sal_uInt32 nTargetSizeY(S_THUMB);

                    if(aDiscreteSize.Width() > aDiscreteSize.Height())
                    {
                        nTargetSizeY = (aDiscreteSize.Height() * nTargetSizeX) / aDiscreteSize.Width();
                    }
                    else
                    {
                        nTargetSizeX = (aDiscreteSize.Width() * nTargetSizeY) / aDiscreteSize.Height();
                    }

                    if(!!aThumbBmp)
                    {
                        aThumbBmp.Scale(Size(nTargetSizeX, nTargetSizeY), BMP_SCALE_BESTQUALITY);
                        aThumbBmp.Convert(BMP_CONVERSION_8BIT_COLORS);
                        bRet = true;
                    }
                }
            }
        }
    }

    return bRet;
}

void SgaObjectSvDraw::WriteData( SvStream& rOut, const OUString& rDestDir ) const
{
    SgaObject::WriteData( rOut, rDestDir );
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, aTitle, RTL_TEXTENCODING_UTF8);
}

void SgaObjectSvDraw::ReadData( SvStream& rIn, sal_uInt16& rReadVersion )
{
    SgaObject::ReadData( rIn, rReadVersion );

    if( rReadVersion >= 5 )
        aTitle = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn, RTL_TEXTENCODING_UTF8);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
