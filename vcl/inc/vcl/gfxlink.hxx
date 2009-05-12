/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gfxlink.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_GFXLINK_HXX
#define _SV_GFXLINK_HXX

#include <vcl/dllapi.h>
#include <vcl/sv.h>
#include <vcl/mapmod.hxx>
#include <tools/stream.hxx>

//#if 0 // _SOLAR__PRIVATE
#include <tools/urlobj.hxx>

// -------------
// - ImpBuffer -
// -------------

struct ImpBuffer
{
    ULONG       mnRefCount;
    BYTE*       mpBuffer;

                ImpBuffer( ULONG nSize )
                {
                    mnRefCount = 1UL;
                    mpBuffer = nSize ? new BYTE[ nSize ] : NULL;
                }

                ImpBuffer( BYTE* pBuf ) { mnRefCount = 1UL; mpBuffer = pBuf; }

                ~ImpBuffer() { delete[] mpBuffer; }
};

// -----------
// - ImpSwap -
// -----------

struct ImpSwap
{
    INetURLObject   maURL;
    ULONG           mnDataSize;
    ULONG           mnRefCount;

                    ImpSwap( BYTE* pData, ULONG nDataSize );
                    ~ImpSwap();

    BYTE*           GetData() const;

    BOOL            IsSwapped() const { return maURL.GetMainURL( INetURLObject::NO_DECODE ).getLength() > 0; }

    void            WriteTo( SvStream& rOStm ) const;
};

// --------------
// - ImpGfxLink -
// --------------

struct ImpGfxLink
{
    MapMode         maPrefMapMode;
    Size            maPrefSize;
    bool            mbPrefMapModeValid;
    bool            mbPrefSizeValid;

    ImpGfxLink() :
        maPrefMapMode(),
        maPrefSize(),
        mbPrefMapModeValid( false ),
        mbPrefSizeValid( false )
    {}
};

//#endif // __PRIVATE

// ---------------
// - GfxLinkType -
// ---------------

enum GfxLinkType
{
    GFX_LINK_TYPE_NONE          = 0,
    GFX_LINK_TYPE_EPS_BUFFER    = 1,
    GFX_LINK_TYPE_NATIVE_GIF    = 2,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_JPG    = 3,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_PNG    = 4,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_TIF    = 5,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_WMF    = 6,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_MET    = 7,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_PCT    = 8,    // Don't forget to update the following defines
    GFX_LINK_TYPE_USER          = 0xffff
};

#define GFX_LINK_FIRST_NATIVE_ID    GFX_LINK_TYPE_NATIVE_GIF
#define GFX_LINK_LAST_NATIVE_ID     GFX_LINK_TYPE_NATIVE_PCT

// -----------
// - GfxLink -
// -----------

struct ImpBuffer;
struct ImpSwap;
struct ImpGfxLink;
class Graphic;

class VCL_DLLPUBLIC GfxLink
{
private:

    GfxLinkType         meType;
    ImpBuffer*          mpBuf;
    ImpSwap*            mpSwap;
    sal_uInt32          mnBufSize;
    sal_uInt32          mnUserId;
    ImpGfxLink*         mpImpData;
    ULONG               mnExtra2;

    SAL_DLLPRIVATE void ImplCopy( const GfxLink& rGfxLink );

public:
                        GfxLink();
                        GfxLink( const GfxLink& );
                        GfxLink( const String& rPath, GfxLinkType nType );
                        GfxLink( BYTE* pBuf, sal_uInt32 nBufSize, GfxLinkType nType, BOOL bOwns );
                        ~GfxLink();

    GfxLink&            operator=( const GfxLink& );
    sal_Bool            IsEqual( const GfxLink& ) const;

    GfxLinkType         GetType() const;

    void                SetUserId( sal_uInt32 nUserId ) { mnUserId = nUserId; }
    sal_uInt32          GetUserId() const { return mnUserId; }

    sal_uInt32          GetDataSize() const;
    void                SetData( BYTE* pBuf, sal_uInt32 nSize, GfxLinkType nType, BOOL bOwns );
    const BYTE*         GetData() const;

    const Size&         GetPrefSize() const;
    void                SetPrefSize( const Size& rPrefSize );
    bool                IsPrefSizeValid();

    const MapMode&      GetPrefMapMode() const;
    void                SetPrefMapMode( const MapMode& rPrefMapMode );
    bool                IsPrefMapModeValid();

    BOOL                IsNative() const;
    BOOL                IsUser() const { return( GFX_LINK_TYPE_USER == meType ); }

    BOOL                LoadNative( Graphic& rGraphic );

    BOOL                ExportNative( SvStream& rOStream ) const;

    void                SwapOut();
    void                SwapIn();
    BOOL                IsSwappedOut() const { return( mpSwap != NULL ); }

public:

    friend VCL_DLLPUBLIC SvStream&  operator<<( SvStream& rOStream, const GfxLink& rGfxLink );
    friend VCL_DLLPUBLIC SvStream&  operator>>( SvStream& rIStream, GfxLink& rGfxLink );
};

#endif
