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

#include <string.h>

#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>
#include <rtl/ustring.h>

#include <vcl/outdev.hxx>

//  ----------------------------------------------------
//  class VCLXFont
//  ----------------------------------------------------
VCLXFont::VCLXFont()
{
    mpFontMetric = NULL;
}

VCLXFont::~VCLXFont()
{
    delete mpFontMetric;
}

void VCLXFont::Init( ::com::sun::star::awt::XDevice& rxDev, const Font& rFont )
{
    mxDevice = &rxDev;

    delete mpFontMetric;
    mpFontMetric = NULL;

    maFont = rFont;
}

sal_Bool VCLXFont::ImplAssertValidFontMetric()
{
    if ( !mpFontMetric && mxDevice.is() )
    {
        OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
        if ( pOutDev )
        {
            Font aOldFont = pOutDev->GetFont();
            pOutDev->SetFont( maFont );
            mpFontMetric = new FontMetric( pOutDev->GetFontMetric() );
            pOutDev->SetFont( aOldFont );
        }
    }
    return mpFontMetric ? sal_True : sal_False;
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXFont::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XFont* >(this)),
                                        (static_cast< ::com::sun::star::awt::XFont2* >(this)),
                                        (static_cast< ::com::sun::star::lang::XUnoTunnel* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXFont )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFont )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont2>* ) NULL )
IMPL_XTYPEPROVIDER_END


::com::sun::star::awt::FontDescriptor VCLXFont::getFontDescriptor(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return VCLUnoHelper::CreateFontDescriptor( maFont );

}

::com::sun::star::awt::SimpleFontMetric VCLXFont::getFontMetric(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::awt::SimpleFontMetric aFM;
    if ( ImplAssertValidFontMetric() )
        aFM = VCLUnoHelper::CreateFontMetric( *mpFontMetric );
    return aFM;
}

sal_Int16 VCLXFont::getCharWidth( sal_Unicode c ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int16 nRet = -1;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );

        nRet = sal::static_int_cast< sal_Int16 >(
            pOutDev->GetTextWidth( rtl::OUString(c) ));

        pOutDev->SetFont( aOldFont );
    }
    return nRet;
}

::com::sun::star::uno::Sequence< sal_Int16 > VCLXFont::getCharWidths( sal_Unicode nFirst, sal_Unicode nLast ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence<sal_Int16> aSeq;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );

        sal_Int16 nCount = nLast-nFirst + 1;
        aSeq = ::com::sun::star::uno::Sequence<sal_Int16>( nCount );
        for ( sal_uInt16 n = 0; n < nCount; n++ )
        {
            aSeq.getArray()[n] = sal::static_int_cast< sal_Int16 >(
                pOutDev->GetTextWidth(
                    rtl::OUString(static_cast< sal_Unicode >(nFirst+n)) ));
        }

        pOutDev->SetFont( aOldFont );
    }
    return aSeq;
}

sal_Int32 VCLXFont::getStringWidth( const ::rtl::OUString& str ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nRet = -1;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );
        nRet = pOutDev->GetTextWidth( str );
        pOutDev->SetFont( aOldFont );
    }
    return nRet;
}

sal_Int32 VCLXFont::getStringWidthArray( const ::rtl::OUString& str, ::com::sun::star::uno::Sequence< sal_Int32 >& rDXArray ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nRet = -1;
    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );
        rDXArray = ::com::sun::star::uno::Sequence<sal_Int32>( str.getLength() );
        nRet = pOutDev->GetTextArray( str, rDXArray.getArray() );
        pOutDev->SetFont( aOldFont );
    }
    return nRet;
}

void VCLXFont::getKernPairs( ::com::sun::star::uno::Sequence< sal_Unicode >& rnChars1, ::com::sun::star::uno::Sequence< sal_Unicode >& rnChars2, ::com::sun::star::uno::Sequence< sal_Int16 >& rnKerns ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if( pOutDev )
    {
        Font aOldFont = pOutDev->GetFont();
        pOutDev->SetFont( maFont );

        sal_uLong nPairs = pOutDev->GetKerningPairCount();
        if ( nPairs )
        {
            KerningPair* pData = new KerningPair[ nPairs ];
            pOutDev->GetKerningPairs( nPairs, pData );

            rnChars1 = ::com::sun::star::uno::Sequence<sal_Unicode>( nPairs );
            rnChars2 = ::com::sun::star::uno::Sequence<sal_Unicode>( nPairs );
            rnKerns = ::com::sun::star::uno::Sequence<sal_Int16>( nPairs );

            sal_Unicode* pChars1 = rnChars1.getArray();
            sal_Unicode* pChars2 = rnChars2.getArray();
            sal_Int16* pKerns = rnKerns.getArray();

            for ( sal_uLong n = 0; n < nPairs; n++ )
            {
                pChars1[n] = pData[n].nChar1;
                pChars2[n] = pData[n].nChar2;
                pKerns[n] = sal::static_int_cast< sal_Int16 >(pData[n].nKern);
            }


            delete[] pData;
        }
        pOutDev->SetFont( aOldFont );
    }
}

// ::com::sun::star::awt::XFont2
sal_Bool VCLXFont::hasGlyphs( const ::rtl::OUString& aText )
    throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( mxDevice );
    if ( pOutDev )
    {
        String aStr( aText );
        if ( pOutDev->HasGlyphs( maFont, aStr, 0, aStr.Len() ) == STRING_LEN )
        {
            return sal_True;
        }
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
