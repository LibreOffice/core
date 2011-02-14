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
#include "precompiled_svx.hxx"

#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>

#include <comphelper/classids.hxx>
#include <unotools/pathoptions.hxx>

#include <tools/rcid.h>
#include <tools/vcompat.hxx>
#include <vcl/virdev.hxx>
#include <svl/itempool.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmview.hxx>
#include <svx/fmpage.hxx>
#include "gallery.hrc"
#include "svx/galmisc.hxx"
#include "galobj.hxx"
#include <vcl/salbtype.hxx>     // FRound
#include <vcl/svapp.hxx>

#include "gallerydrawmodel.hxx"

using namespace ::com::sun::star;

// -------------
// - SgaObject -
// -------------

SgaObject::SgaObject() :
        bIsValid    ( sal_False ),
        bIsThumbBmp ( sal_True )
{
}

// ------------------------------------------------------------------------

sal_Bool SgaObject::CreateThumb( const Graphic& rGraphic )
{
    sal_Bool bRet = sal_False;

    if( rGraphic.GetType() == GRAPHIC_BITMAP )
    {
        BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
        Size        aBmpSize( aBmpEx.GetSizePixel() );

        if( aBmpSize.Width() && aBmpSize.Height() )
        {
            const Color aWhite( COL_WHITE );

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

                    aBmpEx.SetSizePixel( aBmpSize );
                }
            }

            aThumbBmp = aBmpEx.GetBitmap( &aWhite );

            if( ( aBmpSize.Width() <= S_THUMB ) && ( aBmpSize.Height() <= S_THUMB ) )
            {
                aThumbBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
                bRet = sal_True;
            }
            else
            {
                const float fFactor  = (float) aBmpSize.Width() / aBmpSize.Height();
                const Size  aNewSize( Max( (long) (fFactor < 1. ? S_THUMB * fFactor : S_THUMB), 8L ),
                                      Max( (long) (fFactor < 1. ? S_THUMB : S_THUMB / fFactor), 8L ) );

                if( aThumbBmp.Scale( (double) aNewSize.Width() / aBmpSize.Width(),
                                     (double) aNewSize.Height() / aBmpSize.Height(), BMP_SCALE_INTERPOLATE ) )
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

        const GraphicConversionParameters aParameters(aSize);
        aThumbBmp = rGraphic.GetBitmap(aParameters);

        if( !aThumbBmp.IsEmpty() )
        {
            aThumbBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
            bRet = sal_True;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

void SgaObject::WriteData( SvStream& rOut, const String& rDestDir ) const
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

        rOut << aThumbBmp;

        rOut.SetVersion( nOldVersion );
        rOut.SetCompressMode( nOldCompressMode );
    }
    else
        rOut << aThumbMtf;

    String aURLWithoutDestDir = String(aURL.GetMainURL( INetURLObject::NO_DECODE ));
    aURLWithoutDestDir.SearchAndReplace(rDestDir, String());
    rOut << ByteString( aURLWithoutDestDir, RTL_TEXTENCODING_UTF8 );
}

// ------------------------------------------------------------------------

void SgaObject::ReadData(SvStream& rIn, sal_uInt16& rReadVersion )
{
    ByteString  aTmpStr;
    sal_uInt32      nTmp32;
    sal_uInt16      nTmp16;

    rIn >> nTmp32 >> nTmp16 >> rReadVersion >> nTmp16 >> bIsThumbBmp;

    if( bIsThumbBmp )
        rIn >> aThumbBmp;
    else
        rIn >> aThumbMtf;

    rIn >> aTmpStr; aURL = INetURLObject( String( aTmpStr.GetBuffer(), RTL_TEXTENCODING_UTF8 ) );
}

// ------------------------------------------------------------------------

const String SgaObject::GetTitle() const
{
    String aReturnValue( aTitle );
    if ( !getenv( "GALLERY_SHOW_PRIVATE_TITLE" ) )
    {
        if ( aReturnValue.GetTokenCount( ':' ) == 3 )
        {
            String      aPrivateInd  ( aReturnValue.GetToken( 0, ':' ) );
            String      aResourceName( aReturnValue.GetToken( 1, ':' ) );
            sal_Int32   nResId       ( aReturnValue.GetToken( 2, ':' ).ToInt32() );
            if ( aReturnValue.GetToken( 0, ':' ).EqualsAscii( "private" ) &&
                aResourceName.Len() && ( nResId > 0 ) && ( nResId < 0x10000 ) )
            {
                ByteString aMgrName( aResourceName, RTL_TEXTENCODING_UTF8 );
                ResMgr* pResMgr = ResMgr::CreateResMgr( aMgrName.GetBuffer(),
                            Application::GetSettings().GetUILocale() );
                if ( pResMgr )
                {
                    ResId aResId( (sal_uInt16)nResId, *pResMgr );
                    aResId.SetRT( RSC_STRING );
                    if ( pResMgr->IsAvailable( aResId ) )
                    {
                        aReturnValue = String( aResId );
                    }
                    delete pResMgr;
                }
            }
        }
    }
    return aReturnValue;
}

// ------------------------------------------------------------------------

void SgaObject::SetTitle( const String& rTitle )
{
    aTitle = rTitle;
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOut, const SgaObject& rObj )
{
    rObj.WriteData( rOut, String() );
    return rOut;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIn, SgaObject& rObj )
{
    sal_uInt16 nReadVersion;

    rObj.ReadData( rIn, nReadVersion );
    rObj.bIsValid = ( rIn.GetError() == ERRCODE_NONE );

    return rIn;
}

// ----------------
// - SgaObjectBmp -
// ----------------

SgaObjectBmp::SgaObjectBmp()
{
}

// ------------------------------------------------------------------------

SgaObjectBmp::SgaObjectBmp( const INetURLObject& rURL )
{
    Graphic aGraphic;
    String  aFilter;

    if ( SGA_IMPORT_NONE != GalleryGraphicImport( rURL, aGraphic, aFilter ) )
        Init( aGraphic, rURL );
}

// ------------------------------------------------------------------------

SgaObjectBmp::SgaObjectBmp( const Graphic& rGraphic, const INetURLObject& rURL, const String& )
{
    if( FileExists( rURL ) )
        Init( rGraphic, rURL );
}

// ------------------------------------------------------------------------

void SgaObjectBmp::Init( const Graphic& rGraphic, const INetURLObject& rURL )
{
    aURL = rURL;
    bIsValid = CreateThumb( rGraphic );
}

// ------------------------------------------------------------------------

void SgaObjectBmp::WriteData( SvStream& rOut, const String& rDestDir ) const
{
    String  aDummyStr;
    char    aDummy[ 10 ];

    // Version setzen
    SgaObject::WriteData( rOut, rDestDir );
    rOut.Write( aDummy, 10 );
    rOut << ByteString( aDummyStr, RTL_TEXTENCODING_UTF8 ) << ByteString( aTitle, RTL_TEXTENCODING_UTF8 );
}

// ------------------------------------------------------------------------

void SgaObjectBmp::ReadData( SvStream& rIn, sal_uInt16& rReadVersion )
{
    ByteString aTmpStr;

    SgaObject::ReadData( rIn, rReadVersion );
    rIn.SeekRel( 10 ); // 16, 16, 32, 16
    rIn >> aTmpStr; // dummy

    if( rReadVersion >= 5 )
    {
        rIn >> aTmpStr; aTitle = String( aTmpStr.GetBuffer(), RTL_TEXTENCODING_UTF8 );
    }
}

// ------------------
// - SgaObjectSound -
// ------------------

SgaObjectSound::SgaObjectSound() :
    eSoundType( SOUND_STANDARD )
{
}

// ------------------------------------------------------------------------

SgaObjectSound::SgaObjectSound( const INetURLObject& rURL ) :
    eSoundType( SOUND_STANDARD )
{
    if( FileExists( rURL ) )
    {
        aURL = rURL;
        aThumbBmp = Bitmap( Size( 1, 1 ), 1 );
        bIsValid = sal_True;
    }
    else
        bIsValid = sal_False;
}

// ------------------------------------------------------------------------

SgaObjectSound::~SgaObjectSound()
{
}

// ------------------------------------------------------------------------

Bitmap SgaObjectSound::GetThumbBmp() const
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

    const BitmapEx  aBmpEx( GAL_RESID( nId ) );
    const Color     aTransColor( COL_WHITE );

    return aBmpEx.GetBitmap( &aTransColor );
}

// ------------------------------------------------------------------------

void SgaObjectSound::WriteData( SvStream& rOut, const String& rDestDir ) const
{
    SgaObject::WriteData( rOut, rDestDir );
    rOut << (sal_uInt16) eSoundType << ByteString( aTitle, RTL_TEXTENCODING_UTF8 );
}

// ------------------------------------------------------------------------

void SgaObjectSound::ReadData( SvStream& rIn, sal_uInt16& rReadVersion )
{
    SgaObject::ReadData( rIn, rReadVersion );

    if( rReadVersion >= 5 )
    {
        ByteString  aTmpStr;
        sal_uInt16      nTmp16;

        rIn >> nTmp16; eSoundType = (GalSoundType) nTmp16;

        if( rReadVersion >= 6 )
        {
            rIn >> aTmpStr; aTitle = String( aTmpStr.GetBuffer(), RTL_TEXTENCODING_UTF8 );
        }
    }
}

// -----------------
// - SgaObjectAnim -
// -----------------

SgaObjectAnim::SgaObjectAnim()
{
}

// ------------------------------------------------------------------------

SgaObjectAnim::SgaObjectAnim( const Graphic& rGraphic,
                              const INetURLObject& rURL,
                              const String& )
{
    aURL = rURL;
    bIsValid = CreateThumb( rGraphic );
}

// -----------------
// - SgaObjectINet -
// -----------------

SgaObjectINet::SgaObjectINet()
{
}

// ------------------------------------------------------------------------

SgaObjectINet::SgaObjectINet( const Graphic& rGraphic, const INetURLObject& rURL, const String& rFormatName ) :
            SgaObjectAnim   ( rGraphic, rURL, rFormatName )
{
}

// -------------------
// - SgaObjectSvDraw -
// -------------------

SgaObjectSvDraw::SgaObjectSvDraw()
{
}

// ------------------------------------------------------------------------

SgaObjectSvDraw::SgaObjectSvDraw( const FmFormModel& rModel, const INetURLObject& rURL )
{
    aURL = rURL;
    bIsValid = CreateThumb( rModel );
}

// ------------------------------------------------------------------------

SvxGalleryDrawModel::SvxGalleryDrawModel()
: mpFormModel( 0 )
{
    const String sFactoryURL(RTL_CONSTASCII_USTRINGPARAM("sdraw"));

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

// ------------------------------------------------------------------------

SvxGalleryDrawModel::~SvxGalleryDrawModel()
{
    if( mxDoc.Is() )
        mxDoc->DoClose();
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

sal_Bool SgaObjectSvDraw::CreateThumb( const FmFormModel& rModel )
{
    Graphic     aGraphic;
    ImageMap    aImageMap;
    sal_Bool        bRet = sal_False;

    if ( CreateIMapGraphic( rModel, aGraphic, aImageMap ) )
        bRet = SgaObject::CreateThumb( aGraphic );
    else
    {
        VirtualDevice aVDev;

        aVDev.SetOutputSizePixel( Size( S_THUMB*2, S_THUMB*2 ) );

        bRet = DrawCentered( &aVDev, rModel );
        if( bRet )
        {
            aThumbBmp = aVDev.GetBitmap( Point(), aVDev.GetOutputSizePixel() );

            Size aMS( 2, 2 );
            BmpFilterParam aParam( aMS );
            aThumbBmp.Filter( BMP_FILTER_MOSAIC, &aParam );
            aThumbBmp.Scale( Size( S_THUMB, S_THUMB ) );

            aThumbBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool SgaObjectSvDraw::DrawCentered( OutputDevice* pOut, const FmFormModel& rModel )
{
    const FmFormPage*   pPage = static_cast< const FmFormPage* >( rModel.GetPage( 0 ) );
    sal_Bool                bRet = sal_False;

    if( pOut && pPage )
    {
        const Rectangle aObjRect( pPage->GetAllObjBoundRect() );
        const Size      aOutSizePix( pOut->GetOutputSizePixel() );

        if( aObjRect.GetWidth() && aObjRect.GetHeight() && aOutSizePix.Width() > 2 && aOutSizePix.Height() > 2 )
        {
            FmFormView      aView( const_cast< FmFormModel* >( &rModel ), pOut );
            MapMode         aMap( rModel.GetScaleUnit() );
            Rectangle       aDrawRectPix( Point( 1, 1 ), Size( aOutSizePix.Width() - 2, aOutSizePix.Height() - 2 ) );
            const double    fFactor  = (double) aObjRect.GetWidth() / aObjRect.GetHeight();
            Fraction        aFrac( FRound( fFactor < 1. ? aDrawRectPix.GetWidth() * fFactor : aDrawRectPix.GetWidth() ),
                                   pOut->LogicToPixel( aObjRect.GetSize(), aMap ).Width() );

            aMap.SetScaleX( aFrac );
            aMap.SetScaleY( aFrac );

            const Size aDrawSize( pOut->PixelToLogic( aDrawRectPix.GetSize(), aMap ) );
            Point aOrigin( pOut->PixelToLogic( aDrawRectPix.TopLeft(), aMap ) );

            aOrigin.X() += ( ( aDrawSize.Width() - aObjRect.GetWidth() ) >> 1 ) - aObjRect.Left();
            aOrigin.Y() += ( ( aDrawSize.Height() - aObjRect.GetHeight() ) >> 1 ) - aObjRect.Top();
            aMap.SetOrigin( aOrigin );

            aView.SetPageVisible( sal_False );
            aView.SetBordVisible( sal_False );
            aView.SetGridVisible( sal_False );
            aView.SetHlplVisible( sal_False );
            aView.SetGlueVisible( sal_False );

            pOut->Push();
            pOut->SetMapMode( aMap );
            aView.ShowSdrPage( const_cast< FmFormPage* >( pPage ));
            aView.CompleteRedraw( pOut, Rectangle( pOut->PixelToLogic( Point() ), pOut->GetOutputSize() ) );
            pOut->Pop();

            bRet = sal_True;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

void SgaObjectSvDraw::WriteData( SvStream& rOut, const String& rDestDir ) const
{
    SgaObject::WriteData( rOut, rDestDir );
    rOut << ByteString( aTitle, RTL_TEXTENCODING_UTF8 );
}

// ------------------------------------------------------------------------

void SgaObjectSvDraw::ReadData( SvStream& rIn, sal_uInt16& rReadVersion )
{
    SgaObject::ReadData( rIn, rReadVersion );

    if( rReadVersion >= 5 )
    {
        ByteString aTmpStr;
        rIn >> aTmpStr; aTitle = String( aTmpStr.GetBuffer(), RTL_TEXTENCODING_UTF8 );
    }
}
