/*************************************************************************
 *
 *  $RCSfile: galobj.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:19 $
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

#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <tools/vcompat.hxx>
#include <vcl/virdev.hxx>
#include <svtools/itempool.hxx>
#include "fmmodel.hxx"
#include "fmview.hxx"
#include "gallery.hrc"
#include "galmisc.hxx"
#include "galobj.hxx"

// -----------
// - Defines -
// -----------

#define S_THUMB 80

// -------------
// - SgaObject -
// -------------

SgaObject::SgaObject() :
        bIsValid    ( FALSE ),
        bIsThumbBmp ( TRUE )
{
}

// ------------------------------------------------------------------------

BOOL SgaObject::CreateThumb( const Bitmap& rBitmap )
{
    const Size  aBmpSize( rBitmap.GetSizePixel() );
    BOOL        bRet = FALSE;

    if( aBmpSize.Width() && aBmpSize.Height() )
    {
        const float fFactor  = (float) aBmpSize.Width() / aBmpSize.Height();
        const Size  aNewSize( Max( (long) (fFactor < 1. ? S_THUMB * fFactor : S_THUMB), 8L ),
                              Max( (long) (fFactor < 1. ? S_THUMB : S_THUMB / fFactor), 8L ) );

        aThumbBmp = rBitmap;

        if( aThumbBmp.Scale( (double) aNewSize.Width() / aBmpSize.Width(),
                             (double) aNewSize.Height() / aBmpSize.Height(), BMP_SCALE_INTERPOLATE ) )
        {
            aThumbBmp.Dither( BMP_DITHER_FLOYD );
            bRet = TRUE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL SgaObject::CreateThumbMetaFile( const Graphic& rGraphic )
{
    const Size  aMtfSize( rGraphic.GetPrefSize() );
    BOOL        bRet = FALSE;

    if( aMtfSize.Width() && aMtfSize.Height() )
    {
        VirtualDevice*  pVDev = new VirtualDevice;
        Size            aVSize( S_THUMB, S_THUMB );
        Point           aVPos;
        Size            aLogSize;
        const double    fFactor  = (double) aMtfSize.Width() / aMtfSize.Height();
        const Size      aNewSize((USHORT)(fFactor < 1. ? S_THUMB * fFactor : S_THUMB),
                                 (USHORT)(fFactor < 1. ? S_THUMB : S_THUMB / fFactor));

        pVDev->SetOutputSizePixel( aNewSize );
        rGraphic.Draw( pVDev, aVPos, aNewSize );

        aThumbBmp = pVDev->GetBitmap( aVPos, aNewSize );
        delete pVDev;

        if( !!aThumbBmp )
        {
            aThumbBmp.Dither( BMP_DITHER_FLOYD );
            bRet = TRUE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

void SgaObject::WriteData( SvStream& rOut ) const
{
    static const UINT32 nInventor = COMPAT_FORMAT( 'S', 'G', 'A', '3' );

    rOut << nInventor << (UINT16) 0x0004 << GetVersion() << (UINT16) GetObjKind();
    rOut << bIsThumbBmp;

    if( bIsThumbBmp )
    {
        const USHORT    nOldCompressMode = rOut.GetCompressMode();
        const ULONG     nOldVersion = rOut.GetVersion();

        rOut.SetCompressMode( COMPRESSMODE_ZBITMAP );
        rOut.SetVersion( SOFFICE_FILEFORMAT_NOW );

        rOut << aThumbBmp;

        rOut.SetVersion( nOldVersion );
        rOut.SetCompressMode( nOldCompressMode );
    }
    else
        rOut << aThumbMtf;

    rOut << ByteString( aPath, RTL_TEXTENCODING_UTF8 );
}

// ------------------------------------------------------------------------

void SgaObject::ReadData(SvStream& rIn, UINT16& rReadVersion )
{
    ByteString  aTmpStr;
    UINT32      nTmp32;
    UINT16      nTmp16;

    rIn >> nTmp32 >> nTmp16 >> rReadVersion >> nTmp16 >> bIsThumbBmp;

    if( bIsThumbBmp )
        rIn >> aThumbBmp;
    else
        rIn >> aThumbMtf;

    rIn >> aTmpStr; aPath = String( aTmpStr.GetBuffer(), RTL_TEXTENCODING_UTF8 );
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOut, const SgaObject& rObj )
{
    rObj.WriteData( rOut );
    return rOut;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIn, SgaObject& rObj )
{
    UINT16 nReadVersion;

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

SgaObjectBmp::SgaObjectBmp( const String& rFile )
{
    Graphic aGraphic;
    String  aStr;

    aGraphic.SetLink( GfxLink() );

    if ( SGA_IMPORT_NONE != SGAImport( rFile, aGraphic, aStr ) )
        Init( aGraphic, rFile );
}

// ------------------------------------------------------------------------

SgaObjectBmp::SgaObjectBmp( const Graphic& rGraphic, const String& rFile, const String& rFormat )
{
    const INetURLObject aURL( rFile, INET_PROT_FILE );

    if( FileExists( aURL ) )
        Init( rGraphic, rFile );
}

// ------------------------------------------------------------------------

void SgaObjectBmp::Init( const Graphic& rGraphic, const String& rFile )
{
    aPath = rFile;

    if ( rGraphic.GetType() == GRAPHIC_BITMAP )
        bIsValid = CreateThumb( rGraphic.GetBitmap() );
    else if( rGraphic.GetType() != GRAPHIC_NONE )
        bIsValid = CreateThumbMetaFile( rGraphic );
    else
        bIsValid = FALSE;
}

// ------------------------------------------------------------------------

void SgaObjectBmp::WriteData( SvStream& rOut ) const
{
    String  aDummyStr;
    char    aDummy[ 10 ];

    // Version setzen
    SgaObject::WriteData( rOut );
    rOut.Write( aDummy, 10 );
    rOut << ByteString( aDummyStr, RTL_TEXTENCODING_UTF8 ) << ByteString( aTitle, RTL_TEXTENCODING_UTF8 );
}

// ------------------------------------------------------------------------

void SgaObjectBmp::ReadData( SvStream& rIn, UINT16& rReadVersion )
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

SgaObjectSound::SgaObjectSound( const String& rFile ) :
    eSoundType( SOUND_STANDARD )
{
    const INetURLObject aURL( rFile, INET_PROT_FILE );

    if( FileExists( aURL ) )
    {
        aPath = rFile;
        aThumbBmp = Bitmap( Size( 1, 1 ), 1 );
        bIsValid = TRUE;
    }
    else
        bIsValid = FALSE;
}

// ------------------------------------------------------------------------

SgaObjectSound::~SgaObjectSound()
{
}

// ------------------------------------------------------------------------

Bitmap SgaObjectSound::GetThumbBmp() const
{
    USHORT nId;

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
            nId = RID_SVXBMP_GALLERY_SOUND_0;
        break;
    }

    return Bitmap( GAL_RESID( nId ) );
}

// ------------------------------------------------------------------------

void SgaObjectSound::WriteData( SvStream& rOut ) const
{
    SgaObject::WriteData( rOut );
    rOut << (UINT16) eSoundType << ByteString( aTitle, RTL_TEXTENCODING_UTF8 );
}

// ------------------------------------------------------------------------

void SgaObjectSound::ReadData( SvStream& rIn, UINT16& rReadVersion )
{
    SgaObject::ReadData( rIn, rReadVersion );

    if( rReadVersion >= 5 )
    {
        ByteString  aTmpStr;
        UINT16      nTmp16;

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

SgaObjectAnim::SgaObjectAnim( const Graphic& rGraphic, const String& rFile, const String& rFormatName )
{
    aPath = rFile;
    bIsValid = CreateThumb( rGraphic.GetBitmap() );
}

// -----------------
// - SgaObjectINet -
// -----------------

SgaObjectINet::SgaObjectINet()
{
}

// ------------------------------------------------------------------------

SgaObjectINet::SgaObjectINet( const Graphic& rGraphic, const String& rFile, const String& rFormatName ) :
            SgaObjectAnim   ( rGraphic, rFile, rFormatName )
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
    aPath = rURL.GetBase();
    bIsValid = CreateThumb( rModel );
}

// ------------------------------------------------------------------------

SgaObjectSvDraw::SgaObjectSvDraw( SvStream& rIStm, const INetURLObject& rURL )
{
    FmFormModel aModel;

    aModel.GetItemPool().FreezeIdRanges();

    if( SGASvDrawImport( rIStm, aModel ) )
    {
        aPath = rURL.GetBase();
        bIsValid = CreateThumb( aModel );
    }
}

// ------------------------------------------------------------------------

BOOL SgaObjectSvDraw::CreateThumb( const FmFormModel& rModel )
{
    Graphic     aGraphic;
    ImageMap    aImageMap;
    BOOL        bRet = FALSE;

    // Falls das Draw-Objekt nur eine Graphik enthaelt,
    // erzeugen wir mit dieser den Thumb
    if ( CreateIMapGraphic( rModel, aGraphic, aImageMap ) )
        bRet = SgaObject::CreateThumb( aGraphic.GetBitmap() );
    else
    {
        VirtualDevice   aVDev;
        GDIMetaFile     aMtf;

        aVDev.SetMapMode( MapMode( MAP_100TH_MM ) );
        FmFormView aView( &(FmFormModel&) rModel, &aVDev );

        aView.SetMarkHdlHidden( TRUE );
        aView.ShowPagePgNum( 0, Point() );
        aView.MarkAll();
        aThumbMtf = aView.GetAllMarkedMetaFile();

        const Graphic   aGraphic( aThumbMtf );
        const Size      aMtfSize( aGraphic.GetPrefSize() );
        Point           aVPos;
        Size            aLogSize;
        const double    fFactor  = (double) aMtfSize.Width() / aMtfSize.Height();
        Size            aNewSize( (USHORT) (fFactor < 1. ? S_THUMB * fFactor : S_THUMB ),
                                  (USHORT) (fFactor < 1. ? S_THUMB : S_THUMB / fFactor ) );

        aThumbMtf = GDIMetaFile();
        aVDev.SetOutputSizePixel( aNewSize );
        aVDev.SetMapMode( MapMode() );
        aGraphic.Draw( &aVDev, aVPos, aNewSize );
        aThumbBmp = aVDev.GetBitmap( aVPos, aNewSize );
        aThumbBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
        bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

void SgaObjectSvDraw::WriteData( SvStream& rOut ) const
{
    SgaObject::WriteData( rOut );
    rOut << ByteString( aTitle, RTL_TEXTENCODING_UTF8 );
}

// ------------------------------------------------------------------------

void SgaObjectSvDraw::ReadData( SvStream& rIn, UINT16& rReadVersion )
{
    SgaObject::ReadData( rIn, rReadVersion );

    if( rReadVersion >= 5 )
    {
        ByteString aTmpStr;
        rIn >> aTmpStr; aTitle = String( aTmpStr.GetBuffer(), RTL_TEXTENCODING_UTF8 );
    }
}
