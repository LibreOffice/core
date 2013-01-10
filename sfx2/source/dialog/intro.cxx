/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#ifndef GCC
#endif

#include "intro.hxx"

#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/uno/Any.h>
#include <sfx2/sfxuno.hxx>
#include <vcl/dibtools.hxx>

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
    {
        ReadDIB(aIntroBmp, aStrm, true);
    }

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


