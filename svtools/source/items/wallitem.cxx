/*************************************************************************
 *
 *  $RCSfile: wallitem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-11 07:31:01 $
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

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HPP_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/string.hxx>

#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _STREAM_HXX //autogen wg. SvMemoryStream
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
#ifndef _SV_CVTGRF_HXX //autogen wg. GraphicConverter
#include <vcl/cvtgrf.hxx>
#endif

#include "wallitem.hxx"
#include "cntwall.hxx"

#define WALLITEMFLAG_ERROR      0x0001
#define WALLITEMFLAG_DITHERED   0x0002


DBG_NAME( SfxWallpaperItem );
TYPEINIT1( SfxWallpaperItem, SfxPoolItem );

// -----------------------------------------------------------------------

class WallpaperSink_Impl: public cppu::OWeakObject,
                          public com::sun::star::io::XOutputStream
{
    com::sun::star::uno::Reference<
        com::sun::star::io::XActiveDataSource > _aSource;
    SvMemoryStream  *_pStream;
    WallpaperLoader *_pWallpaper;

public:

    WallpaperSink_Impl();
    virtual ~WallpaperSink_Impl();

    void RequestData( WallpaperLoader *pWallpaper, const String& rURL );
    void Release();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type& aType )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    acquire()
        throw(::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    release()
        throw(::com::sun::star::uno::RuntimeException );

    // XOutputStream
    virtual void SAL_CALL
    writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    flush()
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    closeOutput()
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );
};

// -----------------------------------------------------------------------

//static
void SfxBrushItemLink::Set( SfxBrushItemLink* pLink )
{
    SfxBrushItemLink** ppLink =  (SfxBrushItemLink**)GetAppData(SHL_BRUSHITEM);
    if( !*ppLink )
        *ppLink = pLink;
    else
        delete pLink;
}

// -----------------------------------------------------------------------

SfxWallpaperItem::SfxWallpaperItem( USHORT nWhich ) :
    SfxPoolItem( nWhich ), _nFlags(0)
{
    DBG_CTOR( SfxWallpaperItem, 0 );
}

SfxWallpaperItem::SfxWallpaperItem( USHORT nWhich, SvStream& rStream, USHORT nVersion )
    : SfxPoolItem( nWhich )
{
    DBG_CTOR( SfxWallpaperItem, 0 );
    _nFlags = 0;
    rStream >> _aWallpaper;
    readByteString(rStream, _aURL);
    readByteString(rStream, _aFilter);
}
// -----------------------------------------------------------------------

SfxWallpaperItem::SfxWallpaperItem( const SfxWallpaperItem& rItem ) :
    SfxPoolItem( rItem ),
    _aWallpaper( rItem._aWallpaper ),
    _aURL( rItem._aURL ),
    _nFlags( rItem._nFlags )
{
    DBG_CTOR( SfxWallpaperItem, 0 );
}

// -----------------------------------------------------------------------

SfxWallpaperItem::SfxWallpaperItem( USHORT nWhich, const Wallpaper& rPaper ) :
    SfxPoolItem( nWhich ), _nFlags(0), _aWallpaper( rPaper )
{
    DBG_CTOR( SfxWallpaperItem, 0 );
}

// -----------------------------------------------------------------------

SfxWallpaperItem::~SfxWallpaperItem()
{
    DBG_DTOR(SfxWallpaperItem, 0);
}

// -----------------------------------------------------------------------

int SfxWallpaperItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS( SfxWallpaperItem, 0 );
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    const SfxWallpaperItem& rWallItem = (const SfxWallpaperItem&)rItem;
    if( rWallItem._aURL != _aURL )
        return 0;
    if( !_aURL.Len() )
    {
        if( rWallItem._aWallpaper == _aWallpaper )
            return TRUE;
        return 0;
    }
    else
    {
        // Wenn eine URL gesetzt ist, sind die Items auch dann gleich, wenn
        // das eine Item bereits die Bitmap geladen hat und das andere nicht.
        // Deshalb kann der Wallpaper-Vergleichsoperator nicht benutzt werden.
        if( rWallItem._aWallpaper.GetColor() != _aWallpaper.GetColor() )
            return 0;
        if( rWallItem._aWallpaper.GetStyle() != _aWallpaper.GetStyle() )
            return 0;
        if( rWallItem._aWallpaper.GetGradient() != _aWallpaper.GetGradient() )
            return 0;
        if( rWallItem._aWallpaper.GetRect() != _aWallpaper.GetRect() )
            return 0;
    }
    return TRUE;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxWallpaperItem::Create( SvStream& rStream, USHORT nVersion) const
{
    DBG_CHKTHIS(SfxWallpaperItem, 0);
    return new SfxWallpaperItem( Which(), rStream, nVersion );
}

// -----------------------------------------------------------------------

SvStream& SfxWallpaperItem::Store( SvStream& rStream, USHORT ) const
{
    DBG_CHKTHIS( SfxWallpaperItem, 0 );
    BitmapEx aBmpTemp;
    if( _aURL.Len() )
    {
        // Bei gesetzter URL die Grafik als Link speichern, und deshalb
        // temporaer in der Wallpaper zuruecksetzen.
        aBmpTemp = _aWallpaper.GetBitmap();
        ((SfxWallpaperItem*)this)->_aWallpaper.SetBitmap();
    }
    rStream << _aWallpaper;
    writeByteString(rStream, _aURL);
    writeByteString(rStream, _aFilter);
    if( _aURL.Len() )
        ((SfxWallpaperItem*)this)->_aWallpaper.SetBitmap( aBmpTemp );
    return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxWallpaperItem::Clone( SfxItemPool* ) const
{
    DBG_CHKTHIS( SfxWallpaperItem, 0 );
    return new SfxWallpaperItem( *this );
}

// -----------------------------------------------------------------------

BOOL SfxWallpaperItem::QueryValue( com::sun::star::uno::Any& rVal,
                                   BYTE nMemberId ) const
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL SfxWallpaperItem::PutValue( const com::sun::star::uno::Any& rVal,
                                 BYTE nMemberId )
{
    return FALSE;
}

// -----------------------------------------------------------------------

void SfxWallpaperItem::SetBitmapURL( const XubString& rURL, const String& rFilter )
{
    DBG_CHKTHIS(SfxWallpaperItem, 0);
    if( rURL != _aURL || rFilter != _aFilter )
    {
        _aURL = rURL;
        _aFilter = rFilter;
        _nFlags = 0;
        _aWallpaper.SetBitmap();
    }
}

// -----------------------------------------------------------------------

const Wallpaper& SfxWallpaperItem::GetWallpaper( BOOL bNeedsBitmap ) const
{
    DBG_CHKTHIS(SfxWallpaperItem, 0);
    if( bNeedsBitmap && !IsDownloaded() )
        ((SfxWallpaperItem*)this)->Download();
    if( !(_nFlags & WALLITEMFLAG_DITHERED ))
    {
        if( _aWallpaper.IsBitmap())
        {
            BitmapEx aBmp( _aWallpaper.GetBitmap() );
            OutputDevice* pOutDev = Application::GetDefaultDevice();

            if ( pOutDev->GetBitCount() <= 8 && aBmp.GetBitmap().GetBitCount() >= 8 )
            {
                aBmp.Dither( BMP_DITHER_MATRIX );
                aBmp = BitmapEx( aBmp.GetBitmap().CreateDisplayBitmap( pOutDev ), aBmp.GetMask() );
                ( (SfxWallpaperItem*)this )->_aWallpaper.SetBitmap( aBmp );
            }
        }
        ( (SfxWallpaperItem*)this )->_nFlags |= WALLITEMFLAG_DITHERED;
    }
    return _aWallpaper;
}

void SfxWallpaperItem::SetGraphic( const Graphic& rGraphic, BOOL bKeepLink )
{
    DBG_CHKTHIS(SfxWallpaperItem, 0);
    BitmapEx aBmp( rGraphic.GetBitmapEx() );
    _aWallpaper.SetBitmap( aBmp );
    if( !bKeepLink )
    {
        _aURL.Erase();
        _aFilter.Erase();
    }
    _nFlags &= ~( WALLITEMFLAG_ERROR | WALLITEMFLAG_DITHERED );
}

BOOL SfxWallpaperItem::IsDownloaded() const
{
    if( _nFlags & WALLITEMFLAG_ERROR )
        return TRUE;  // Endlos-Downloads verhindern
    return !_aURL.Len() || _aWallpaper.IsBitmap();
}

void SfxWallpaperItem::Download()
{
    if( _aURL.Len() )
    {
        SfxBrushItemLink* pLink = SfxBrushItemLink::Get();
        if( pLink )
        {
            Graphic aGraphic( pLink->GetGraphic( _aURL, _aFilter ) );
            if( aGraphic.GetType() != GRAPHIC_NONE )
            {
                BitmapEx aBmp( aGraphic.GetBitmapEx() );
                _aWallpaper.SetBitmap( aBmp );
                _nFlags &= ~WALLITEMFLAG_ERROR;
            }
            else
                _nFlags |= WALLITEMFLAG_ERROR;
        }
    }
    _nFlags &= ~WALLITEMFLAG_DITHERED;
}

//============================================================================
class WallpaperLoader_Impl
{
public:

    Link                _aDoneLink;
    SfxWallpaperItem    _aWallpaper;
    com::sun::star::uno::Reference<
        com::sun::star::io::XOutputStream > _xOutputStream;
    WallpaperSink_Impl* _pWallpaperSink;
    BOOL                _bDone;

    WallpaperLoader_Impl( const SfxWallpaperItem& rItem );
};

WallpaperLoader_Impl::WallpaperLoader_Impl( const SfxWallpaperItem& rWallpaper )
    : _aWallpaper( rWallpaper )
{
    _pWallpaperSink = NULL;
    _bDone = FALSE;
}

WallpaperLoader::WallpaperLoader( const CntWallpaperItem& rWallpaper )
{
    SfxWallpaperItem aSfxWallpaper( rWallpaper.Which() );

    aSfxWallpaper.SetBitmapURL( rWallpaper.GetBitmapURL(), String() );
    aSfxWallpaper.SetColor( rWallpaper.GetColor() );
    aSfxWallpaper.SetStyle( (WallpaperStyle) rWallpaper.GetStyle() );

    pImp = new WallpaperLoader_Impl( aSfxWallpaper );
}

WallpaperLoader::~WallpaperLoader()
{
    if ( !pImp->_bDone && pImp->_pWallpaperSink )
    {
        pImp->_pWallpaperSink->Release();
        pImp->_pWallpaperSink = NULL;
    }
    delete pImp;
}

void WallpaperLoader::RequestData( const Link& rDoneLink )
{
    WallpaperLoaderRef aRef( this );

    pImp->_aDoneLink = rDoneLink;

    if ( pImp->_aWallpaper.GetBitmapURL().Len() )
    {
        WallpaperSink_Impl *pStream = new WallpaperSink_Impl();
        pImp->_xOutputStream = pStream;
        pImp->_pWallpaperSink = pStream;
        pStream->RequestData( this, pImp->_aWallpaper.GetBitmapURL() );
    }
    else
        Done();
}

const Wallpaper& WallpaperLoader::GetWallpaper() const
{
    return pImp->_aWallpaper.GetWallpaper();
}

void WallpaperLoader::Done()
{
    WallpaperLoaderRef aRef( this );

    pImp->_bDone = TRUE;

    if ( pImp->_aDoneLink.IsSet() )
        pImp->_aDoneLink.Call( this );
}

void WallpaperLoader::SetBitmap( SvStream *pStream )
{
    WallpaperLoaderRef aRef( this );

    if( pStream && ! pStream->GetError() )
    {
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        Graphic aGraphic;
        GraphicConverter::Import( *pStream, aGraphic );

        if( aGraphic.GetType() != GRAPHIC_NONE )
        {
            pImp->_aWallpaper.SetGraphic( aGraphic, TRUE );
        }
    }

    Done();
}


//============================================================================

WallpaperSink_Impl::WallpaperSink_Impl()
{
    _pStream = NULL;
    _pWallpaper = NULL;
}

//--------------------------------------------------------------------------------------------------
// virtual
WallpaperSink_Impl::~WallpaperSink_Impl()
{
    delete _pStream;
}

//--------------------------------------------------------------------------------------------------
void WallpaperSink_Impl::RequestData( WallpaperLoader *pWallpaper,
                                      const String& rURL )
{
    _pWallpaper = pWallpaper;

    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > xMgr(
            utl::getProcessServiceFactory() );
    if ( !xMgr.is() )
        return;

    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > xFact(
                xMgr->createInstance(
                    rtl::OUString::createFromAscii(
                         "com.sun.star.frame.DataSourceFactory" ) ),
                com::sun::star::uno::UNO_QUERY );
    if ( xFact.is() )
    {
        INetURLObject aURL( rURL );
        com::sun::star::uno::Sequence< com::sun::star::uno::Any > aArgs( 1 );
        aArgs.getArray()[ 0 ] <<= rtl::OUString( aURL.GetMainURL() );

        String aProt( INetURLObject::GetScheme( aURL.GetProtocol() ) );
        _aSource = com::sun::star::uno::Reference<
                        com::sun::star::io::XActiveDataSource >(
                            xFact->createInstanceWithArguments(
                                aProt, aArgs ),
                            com::sun::star::uno::UNO_QUERY );
        if ( _aSource.is() )
        {
            _aSource->setOutputStream( this );
            com::sun::star::uno::Reference<
                com::sun::star::io::XActiveDataControl > aControl(
                    _aSource, com::sun::star::uno::UNO_QUERY );
            if ( aControl.is() )
                aControl->start();
        }
    }
}

//--------------------------------------------------------------------------------------------------
void WallpaperSink_Impl::Release()
{
    com::sun::star::uno::Reference<
                        com::sun::star::io::XOutputStream > aRef( this );

    _pWallpaper = NULL;

    if ( _aSource.is() )
    {
        com::sun::star::uno::Reference<
            com::sun::star::io::XActiveDataControl > aControl(
                _aSource, com::sun::star::uno::UNO_QUERY );
        if ( aControl.is() )
            aControl->terminate();

        _aSource = com::sun::star::uno::Reference<
                                com::sun::star::io::XActiveDataSource >();
    }
}

//--------------------------------------------------------------------------------------------------
// virtual
com::sun::star::uno::Any SAL_CALL WallpaperSink_Impl::
    queryInterface( const com::sun::star::uno::Type& aType )
        throw( com::sun::star::uno::RuntimeException )
{
    com::sun::star::uno::Any aRet
        = cppu::queryInterface( aType,
            static_cast< com::sun::star::io::XOutputStream * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( aType );
}

//--------------------------------------------------------------------------------------------------
// virtual
void SAL_CALL WallpaperSink_Impl::acquire()
    throw(::com::sun::star::uno::RuntimeException )
{
    OWeakObject::acquire();
}

//--------------------------------------------------------------------------------------------------
// virtual
void SAL_CALL WallpaperSink_Impl::release()
    throw(::com::sun::star::uno::RuntimeException )
{
    OWeakObject::release();
}

//--------------------------------------------------------------------------------------------------
// virtual
void SAL_CALL WallpaperSink_Impl::writeBytes(
                    const com::sun::star::uno::Sequence< sal_Int8 >& aData )
    throw( com::sun::star::io::NotConnectedException,
           com::sun::star::io::BufferSizeExceededException,
           com::sun::star::io::IOException,
           com::sun::star::uno::RuntimeException )
{
    if ( !_pStream )
        _pStream = new SvMemoryStream( aData.getLength(), 4096 );

    _pStream->Seek( STREAM_SEEK_TO_END );
    _pStream->Write( aData.getConstArray(), aData.getLength() );
}

//--------------------------------------------------------------------------------------------------
// virtual
void SAL_CALL WallpaperSink_Impl::flush()
    throw( com::sun::star::io::NotConnectedException,
           com::sun::star::io::BufferSizeExceededException,
           com::sun::star::io::IOException,
            com::sun::star::uno::RuntimeException )
{
}

//--------------------------------------------------------------------------------------------------
// virtual
void SAL_CALL WallpaperSink_Impl::closeOutput()
    throw( com::sun::star::io::NotConnectedException,
           com::sun::star::io::BufferSizeExceededException,
           com::sun::star::io::IOException,
           com::sun::star::uno::RuntimeException )
{
    if ( _pWallpaper )
        _pWallpaper->SetBitmap( _pStream );

    _aSource = com::sun::star::uno::Reference<
                                com::sun::star::io::XActiveDataSource >();
}
