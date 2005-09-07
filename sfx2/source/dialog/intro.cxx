/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intro.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:19:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef GCC
#pragma hdrstop
#endif

#include "intro.hxx"

#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <svtools/pathoptions.hxx>
#include <unotools/configmgr.hxx>
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#include "sfxuno.hxx"

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


