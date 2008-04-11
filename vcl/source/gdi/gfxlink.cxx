/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gfxlink.cxx,v $
 * $Revision: 1.19 $
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
#include "precompiled_vcl.hxx"

#include <tools/vcompat.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/graph.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/cvtgrf.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>

// -----------
// - GfxLink -
// -----------

GfxLink::GfxLink() :
    meType      ( GFX_LINK_TYPE_NONE ),
    mpBuf       ( NULL ),
    mpSwap      ( NULL ),
    mnBufSize   ( 0 ),
    mnUserId    ( 0UL ),
    mpImpData   ( new ImpGfxLink )
{
}

// ------------------------------------------------------------------------

GfxLink::GfxLink( const GfxLink& rGfxLink ) :
    mpImpData( new ImpGfxLink )
{
    ImplCopy( rGfxLink );
}

// ------------------------------------------------------------------------

GfxLink::GfxLink( BYTE* pBuf, sal_uInt32 nSize, GfxLinkType nType, BOOL bOwns ) :
    mpImpData( new ImpGfxLink )
{
    DBG_ASSERT( (pBuf != NULL && nSize) || (!bOwns && nSize == 0),
                "GfxLink::GfxLink(): empty/NULL buffer given" );

    meType = nType;
    mnBufSize = nSize;
    mpSwap = NULL;
    mnUserId = 0UL;

    if( bOwns )
        mpBuf = new ImpBuffer( pBuf );
    else if( nSize )
    {
        mpBuf = new ImpBuffer( nSize );
        memcpy( mpBuf->mpBuffer, pBuf, nSize );
    }
    else
        mpBuf = NULL;
}

// ------------------------------------------------------------------------

GfxLink::~GfxLink()
{
    if( mpBuf && !( --mpBuf->mnRefCount ) )
        delete mpBuf;

    if( mpSwap && !( --mpSwap->mnRefCount ) )
        delete mpSwap;

    delete mpImpData;
}

// ------------------------------------------------------------------------

GfxLink& GfxLink::operator=( const GfxLink& rGfxLink )
{
    if( &rGfxLink != this )
    {
        if ( mpBuf && !( --mpBuf->mnRefCount ) )
            delete mpBuf;

        if( mpSwap && !( --mpSwap->mnRefCount ) )
            delete mpSwap;

        ImplCopy( rGfxLink );
    }

    return *this;
}

// ------------------------------------------------------------------------

sal_Bool GfxLink::IsEqual( const GfxLink& rGfxLink ) const
{
    sal_Bool bIsEqual = sal_False;

    if ( ( mnBufSize == rGfxLink.mnBufSize ) && ( meType == rGfxLink.meType ) )
    {
        const sal_uInt8* pSource = GetData();
        const sal_uInt8* pDest = rGfxLink.GetData();
        sal_uInt32 nSourceSize = GetDataSize();
        sal_uInt32 nDestSize = rGfxLink.GetDataSize();
        if ( pSource && pDest && ( nSourceSize == nDestSize ) )
        {
            bIsEqual = memcmp( pSource, pDest, nSourceSize ) == 0;
        }
        else if ( ( pSource == 0 ) && ( pDest == 0 ) )
            bIsEqual = sal_True;
    }
    return bIsEqual;
}

// ------------------------------------------------------------------------

void GfxLink::ImplCopy( const GfxLink& rGfxLink )
{
    mnBufSize = rGfxLink.mnBufSize;
    meType = rGfxLink.meType;
    mpBuf = rGfxLink.mpBuf;
    mpSwap = rGfxLink.mpSwap;
    mnUserId = rGfxLink.mnUserId;
    *mpImpData = *rGfxLink.mpImpData;

    if( mpBuf )
        mpBuf->mnRefCount++;

    if( mpSwap )
        mpSwap->mnRefCount++;
}

// ------------------------------------------------------------------------

GfxLinkType GfxLink::GetType() const
{
    return meType;
}

// ------------------------------------------------------------------------

BOOL GfxLink::IsNative() const
{
    return( meType >= GFX_LINK_FIRST_NATIVE_ID && meType <= GFX_LINK_LAST_NATIVE_ID );
}

// ------------------------------------------------------------------------

sal_uInt32 GfxLink::GetDataSize() const
{
    return mnBufSize;
}

// ------------------------------------------------------------------------

const BYTE* GfxLink::GetData() const
{
    if( IsSwappedOut() )
        ( (GfxLink*) this )->SwapIn();

    return( mpBuf ? mpBuf->mpBuffer : NULL );
}

// ------------------------------------------------------------------------

const Size& GfxLink::GetPrefSize() const
{
    return mpImpData->maPrefSize;
}

// ------------------------------------------------------------------------

void GfxLink::SetPrefSize( const Size& rPrefSize )
{
    mpImpData->maPrefSize = rPrefSize;
    mpImpData->mbPrefSizeValid = true;
}

// ------------------------------------------------------------------------

bool GfxLink::IsPrefSizeValid()
{
    return mpImpData->mbPrefSizeValid;
}

// ------------------------------------------------------------------------

const MapMode& GfxLink::GetPrefMapMode() const
{
    return mpImpData->maPrefMapMode;
}

// ------------------------------------------------------------------------

void GfxLink::SetPrefMapMode( const MapMode& rPrefMapMode )
{
    mpImpData->maPrefMapMode = rPrefMapMode;
    mpImpData->mbPrefMapModeValid = true;
}

// ------------------------------------------------------------------------

bool GfxLink::IsPrefMapModeValid()
{
    return mpImpData->mbPrefMapModeValid;
}

// ------------------------------------------------------------------------

BOOL GfxLink::LoadNative( Graphic& rGraphic )
{
    BOOL bRet = FALSE;

    if( IsNative() && mnBufSize )
    {
        const BYTE* pData = GetData();

        if( pData )
        {
            SvMemoryStream  aMemStm;
            ULONG           nCvtType;

            aMemStm.SetBuffer( (char*) pData, mnBufSize, FALSE, mnBufSize );

            switch( meType )
            {
                case( GFX_LINK_TYPE_NATIVE_GIF ): nCvtType = CVT_GIF; break;
                case( GFX_LINK_TYPE_NATIVE_JPG ): nCvtType = CVT_JPG; break;
                case( GFX_LINK_TYPE_NATIVE_PNG ): nCvtType = CVT_PNG; break;
                case( GFX_LINK_TYPE_NATIVE_TIF ): nCvtType = CVT_TIF; break;
                case( GFX_LINK_TYPE_NATIVE_WMF ): nCvtType = CVT_WMF; break;
                case( GFX_LINK_TYPE_NATIVE_MET ): nCvtType = CVT_MET; break;
                case( GFX_LINK_TYPE_NATIVE_PCT ): nCvtType = CVT_PCT; break;

                default: nCvtType = CVT_UNKNOWN; break;
            }

            if( nCvtType && ( GraphicConverter::Import( aMemStm, rGraphic, nCvtType ) == ERRCODE_NONE ) )
                bRet = TRUE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GfxLink::SwapOut()
{
    if( !IsSwappedOut() && mpBuf )
    {
        mpSwap = new ImpSwap( mpBuf->mpBuffer, mnBufSize );

        if( !mpSwap->IsSwapped() )
        {
            delete mpSwap;
            mpSwap = NULL;
        }
        else
        {
            if( !( --mpBuf->mnRefCount ) )
                delete mpBuf;

            mpBuf = NULL;
        }
    }
}

// ------------------------------------------------------------------------

void GfxLink::SwapIn()
{
    if( IsSwappedOut() )
    {
        mpBuf = new ImpBuffer( mpSwap->GetData() );

        if( !( --mpSwap->mnRefCount ) )
            delete mpSwap;

        mpSwap = NULL;
    }
}

// ------------------------------------------------------------------------

BOOL GfxLink::ExportNative( SvStream& rOStream ) const
{
    if( GetDataSize() )
    {
        if( IsSwappedOut() )
            mpSwap->WriteTo( rOStream );
        else if( GetData() )
            rOStream.Write( GetData(), GetDataSize() );
    }

    return ( rOStream.GetError() == ERRCODE_NONE );
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const GfxLink& rGfxLink )
{
    VersionCompat* pCompat = new VersionCompat( rOStream, STREAM_WRITE, 2 );

    // Version 1
    rOStream << (UINT16) rGfxLink.GetType() << rGfxLink.GetDataSize() << rGfxLink.GetUserId();

    // Version 2
    rOStream << rGfxLink.GetPrefSize() << rGfxLink.GetPrefMapMode();

    delete pCompat;

    if( rGfxLink.GetDataSize() )
    {
        if( rGfxLink.IsSwappedOut() )
            rGfxLink.mpSwap->WriteTo( rOStream );
        else if( rGfxLink.GetData() )
            rOStream.Write( rGfxLink.GetData(), rGfxLink.GetDataSize() );
    }

    return rOStream;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, GfxLink& rGfxLink)
{
    Size            aSize;
    MapMode         aMapMode;
    sal_uInt32      nSize;
    sal_uInt32      nUserId;
    UINT16          nType;
    BYTE*           pBuf;
    bool            bMapAndSizeValid( false );
    VersionCompat*  pCompat = new VersionCompat( rIStream, STREAM_READ );

    // Version 1
    rIStream >> nType >> nSize >> nUserId;

    if( pCompat->GetVersion() >= 2 )
    {
        rIStream >> aSize >> aMapMode;
        bMapAndSizeValid = true;
    }

    delete pCompat;

    pBuf = new BYTE[ nSize ];
    rIStream.Read( pBuf, nSize );

    rGfxLink = GfxLink( pBuf, nSize, (GfxLinkType) nType, TRUE );
    rGfxLink.SetUserId( nUserId );

    if( bMapAndSizeValid )
    {
        rGfxLink.SetPrefSize( aSize );
        rGfxLink.SetPrefMapMode( aMapMode );
    }

    return rIStream;
}

// -----------
// - ImpSwap -
// -----------

ImpSwap::ImpSwap( BYTE* pData, ULONG nDataSize ) :
            mnDataSize( nDataSize ),
            mnRefCount( 1UL )
{
    if( pData && mnDataSize )
    {
        ::utl::TempFile aTempFile;

        maURL = INetURLObject(aTempFile.GetURL());

        if( maURL.GetMainURL( INetURLObject::NO_DECODE ).getLength() )
        {
            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( maURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READWRITE | STREAM_SHARE_DENYWRITE );

            if( pOStm )
            {
                pOStm->Write( pData, mnDataSize );
                sal_Bool bError = ( ERRCODE_NONE != pOStm->GetError() );
                delete pOStm;

                if( bError )
                {
                    try
                    {
                        ::ucbhelper::Content aCnt( maURL.GetMainURL( INetURLObject::NO_DECODE ),
                                             ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

                        aCnt.executeCommand( ::rtl::OUString::createFromAscii( "delete" ),
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

                    maURL = INetURLObject();
                }
            }
        }
    }
}

// ------------------------------------------------------------------------

ImpSwap::~ImpSwap()
{
    if( IsSwapped() )
    {
        try
        {
            ::ucbhelper::Content aCnt( maURL.GetMainURL( INetURLObject::NO_DECODE ),
                                 ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

            aCnt.executeCommand( ::rtl::OUString::createFromAscii( "delete" ),
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

// ------------------------------------------------------------------------

BYTE* ImpSwap::GetData() const
{
    BYTE* pData;

    if( IsSwapped() )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( maURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READWRITE );

        if( pIStm )
        {
            pData = new BYTE[ mnDataSize ];
            pIStm->Read( pData, mnDataSize );
            sal_Bool bError = ( ERRCODE_NONE != pIStm->GetError() );
            delete pIStm;

            if( bError )
                delete[] pData, pData = NULL;
        }
        else
            pData = NULL;
    }
    else
        pData = NULL;

    return pData;
}

// ------------------------------------------------------------------------

void ImpSwap::WriteTo( SvStream& rOStm ) const
{
    BYTE* pData = GetData();

    if( pData )
    {
        rOStm.Write( pData, mnDataSize );
        delete[] pData;
    }
}
