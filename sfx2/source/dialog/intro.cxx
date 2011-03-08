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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "intro.hxx"

#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/uno/Any.h>

#include <sfx2/sfxuno.hxx>

// -----------------------------------------------------------------------

void IntroWindow_Impl::Init()
{
    Size aSize = aIntroBmp.GetSizePixel();
    SetOutputSizePixel( aSize );
    Size  aScreenSize( GetDesktopRectPixel().GetSize() );
    Size  aWinSize( GetSizePixel() );
    Point aWinPos( ( aScreenSize.Width()  - aWinSize.Width() )  / 2,
                   ( aScreenSize.Height() - aWinSize.Height() ) / 2  );
    SetPosPixel( aWinPos );

    if ( GetColorCount() >= 16 )
    {
        Show();
        Update();
    }
}

// -----------------------------------------------------------------------

IntroWindow_Impl::IntroWindow_Impl( const Bitmap& rBmp ) :

    WorkWindow( NULL, (WinBits)0 ),

    aIntroBmp( rBmp )

{
    Hide();

    // load bitmap depends on productname ("StarOffice", "StarSuite",...)
    ::com::sun::star::uno::Any aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTNAME );
    rtl::OUString aTmp;
    aRet >>= aTmp;
    String aBmpFileName = aTmp;
    aBmpFileName += String( DEFINE_CONST_UNICODE("_intro.bmp") );
    INetURLObject aObj( SvtPathOptions().GetModulePath(), INET_PROT_FILE );
    aObj.insertName( aBmpFileName );
    SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
    if ( !aStrm.GetError() )
        aStrm >> aIntroBmp;

    Init();
}

// -----------------------------------------------------------------------

IntroWindow_Impl::~IntroWindow_Impl()
{
    Hide();
}

// -----------------------------------------------------------------------

void IntroWindow_Impl::Paint( const Rectangle& )
{
    DrawBitmap( Point(), aIntroBmp );
    Flush();
}

// -----------------------------------------------------------------------

void IntroWindow_Impl::Slide()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
