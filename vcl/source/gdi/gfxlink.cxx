/*************************************************************************
 *
 *  $RCSfile: gfxlink.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-07 17:09:05 $
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

#include <tools/vcompat.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/content.hxx>
#include "graph.hxx"
#include "gfxlink.hxx"
#include "cvtgrf.hxx"

#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif

// -----------
// - GfxLink -
// -----------

GfxLink::GfxLink() :
    meType      ( GFX_LINK_TYPE_NONE ),
    mnBufSize   ( 0 ),
    mpBuf       ( NULL ),
    mpSwap      ( NULL ),
    mnUserId    ( 0UL )
{
}

// ------------------------------------------------------------------------

GfxLink::GfxLink( const GfxLink& rGfxLink )
{
    ImplCopy( rGfxLink );
}

// ------------------------------------------------------------------------

GfxLink::GfxLink( BYTE* pBuf, ULONG nSize, GfxLinkType nType, BOOL bOwns )
{
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

void GfxLink::ImplCopy( const GfxLink& rGfxLink )
{
    mnBufSize = rGfxLink.mnBufSize;
    meType = rGfxLink.meType;
    mpBuf = rGfxLink.mpBuf;
    mpSwap = rGfxLink.mpSwap;
    mnUserId = rGfxLink.mnUserId;

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

ULONG GfxLink::GetDataSize() const
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
        else if( !( --mpBuf->mnRefCount ) )
            delete mpBuf;

        mpBuf = NULL;
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

SvStream& operator<<( SvStream& rOStream, const GfxLink& rGfxLink )
{
    VersionCompat* pCompat = new VersionCompat( rOStream, STREAM_WRITE, 1 );

    rOStream << (UINT16) rGfxLink.GetType() << rGfxLink.GetDataSize() << rGfxLink.GetUserId();

    delete pCompat;

    if( rGfxLink.GetDataSize() )
    {
        if( rGfxLink.IsSwappedOut() )
            rGfxLink.mpSwap->WriteTo( rOStream );
        else
            rOStream.Write( rGfxLink.GetData(), rGfxLink.GetDataSize() );
    }

    return rOStream;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, GfxLink& rGfxLink)
{
    ULONG           nSize;
    ULONG           nUserId;
    UINT16          nType;
    BYTE*           pBuf;
    VersionCompat*  pCompat = new VersionCompat( rIStream, STREAM_READ );

    rIStream >> nType >> nSize >> nUserId;

    delete pCompat;

    pBuf = new BYTE[ nSize ];
    rIStream.Read( pBuf, nSize );

    rGfxLink = GfxLink( pBuf, nSize, (GfxLinkType) nType, TRUE );
    rGfxLink.SetUserId( nUserId );

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

        maURL = aTempFile.GetURL();

        if( maURL.GetMainURL().Len() )
        {
            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( maURL.GetMainURL(), STREAM_READWRITE | STREAM_SHARE_DENYWRITE );

            if( pOStm )
            {
                pOStm->Write( pData, mnDataSize );
                sal_Bool bError = ( ERRCODE_NONE != pOStm->GetError() );
                delete pOStm;

                if( bError )
                {
                    try
                    {
                        ::ucb::Content aCnt( maURL.GetMainURL(),
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
            ::ucb::Content aCnt( maURL.GetMainURL(),
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

// ------------------------------------------------------------------------

BYTE* ImpSwap::GetData() const
{
    BYTE* pData;

    if( IsSwapped() )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( maURL.GetMainURL(), STREAM_READWRITE );

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
